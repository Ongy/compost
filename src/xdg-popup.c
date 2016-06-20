#include <libweston-0/compositor.h>
#include <wayland-client-core.h>
#include <wayland-util.h>
#include <assert.h>

#include "xdg-shell-server-protocol.h"
#include "xdg-popup.h"
#include "xdg-surface.h"
#include "compost.h"

struct compost_xdg_popup {
	struct wl_list link;
	uint32_t id;
	struct compost_xdg_surface *parent;
	struct wl_listener parent_listener;
	struct weston_surface *surface;

	struct wl_resource *resource;
	struct weston_view *view;

	int32_t ox, oy;
};

static void
on_parent_destroy(struct wl_listener *listener, void *data)
{
	struct compost_xdg_popup *popup;
	(void) data;

	popup = wl_container_of(listener, popup, parent_listener);

	popup->parent = NULL;
}

static void
xdg_popup_destroy(struct wl_client *client, struct wl_resource *resource)
{
	struct compost_xdg_popup *popup;
	(void) client;

	popup = wl_resource_get_user_data(resource);

	weston_log("Destroying popup\n");
	if (&popup->link != popup->parent->popups.next) {
		wl_resource_post_error(resource,
		                       XDG_SHELL_ERROR_DEFUNCT_SURFACES,
		                       "XDG-POPUP destroyed that wasn't the latest\n");
		weston_log("Child destroyed xdg-popup while newer existed\n");
	}
	weston_log("%s\n", __PRETTY_FUNCTION__);
	wl_resource_destroy(resource);
}

static const struct xdg_popup_interface xdg_popup_implementation = {
	xdg_popup_destroy
};

static void
compost_popup_configure(struct weston_surface *s, int32_t x, int32_t y)
{
	(void) x; (void) y;
	if (s->output != NULL)
		weston_output_schedule_repaint(s->output);
	weston_log("%s\n", __PRETTY_FUNCTION__);
}

static void
xdg_popup_delete(struct wl_resource *resource)
{
	struct compost_xdg_popup *popup =
		(struct compost_xdg_popup *)
			wl_resource_get_user_data(resource);
	weston_log("%s\n", __PRETTY_FUNCTION__);

	if (popup->parent)
		wl_list_remove(&popup->parent_listener.link);

	wl_list_remove(&popup->link);
	free(popup);
}

struct compost_xdg_popup *
xdg_popup(struct wl_client *client, uint32_t id,
          struct weston_surface *surface,
          int32_t x, int32_t y,
	  struct compost_shell *shell,
	  struct weston_surface *surf)
{
	struct compost_xdg_surface *parent;
	struct compost_xdg_popup *xdg_popup;
	struct compost_output *out;

	parent = (struct compost_xdg_surface *)surf->configure_private;

	weston_log("%s\n", __PRETTY_FUNCTION__);

	wl_list_for_each(out, &shell->outputs, link) {
		if (out->output == surf->output) {
			weston_log("Found fitting output\n");
			break;
		}
	}

	xdg_popup = malloc(sizeof(*xdg_popup));

	xdg_popup->id = id;
	xdg_popup->surface = surface;
	xdg_popup->ox = x;
	xdg_popup->oy = y;
	wl_list_insert(&parent->popups, &xdg_popup->link);
	xdg_popup->parent = parent;
	xdg_popup->parent_listener.notify = on_parent_destroy;
	wl_resource_add_destroy_listener(xdg_popup->parent->resource,
	                                 &xdg_popup->parent_listener);

	xdg_popup->resource = wl_resource_create(client,
	                                         &xdg_surface_interface,
	                                         1, id);
	wl_resource_set_implementation(xdg_popup->resource,
	                               &xdg_popup_implementation,
	                               xdg_popup,
	                               xdg_popup_delete);

	if (weston_surface_set_role(surface, "xdg_popup", xdg_popup->resource,
	                            XDG_SHELL_ERROR_ROLE) < 0) {
		weston_log("TODO\n");
		return NULL;
	}

	surface->configure = &compost_popup_configure;
	surface->configure_private = xdg_popup;
	surface->output = out->output;

	xdg_popup->view = weston_view_create(surface);
	weston_view_set_transform_parent(xdg_popup->view, parent->view);


	weston_view_set_position(xdg_popup->view, x, y);
	surface->timeline.force_refresh = 1;
	weston_layer_entry_insert(&parent->view->layer_link.layer->view_list,
	                          &xdg_popup->view->layer_link);

	weston_view_geometry_dirty(xdg_popup->view);

	return xdg_popup;
}
