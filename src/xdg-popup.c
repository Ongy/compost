#include <libweston-0/compositor.h>
#include <wayland-client-core.h>
#include <wayland-util.h>
#include <assert.h>

#include "xdg-shell-server-protocol.h"
#include "xdg-popup.h"
#include "compost.h"

struct compost_xdg_popup {
	struct wl_client *client;
	uint32_t id;
	struct weston_surface *surface;

	struct wl_resource *resource;
	struct weston_view *view;

	int32_t ox, oy;
};

static void
xdg_popup_destroy(struct wl_client *client, struct wl_resource *resource)
{
	(void) client;
	wl_resource_destroy(resource);
}

static const struct xdg_popup_interface xdg_popup_implementation = {
	xdg_popup_destroy
};

static void
shell_popup_configure(struct weston_surface *s, int32_t x, int32_t y)
{
	(void) x; (void) y; (void) s;
}

static void
xdg_popup_delete(struct wl_resource *resource)
{
	struct compost_xdg_popup *popup =
		(struct compost_xdg_popup *)
			wl_resource_get_user_data(resource);

	/* TODO */
	free(popup);
}

struct compost_xdg_popup *
xdg_popup(struct wl_client *client, uint32_t id,
          struct weston_surface *surface,
          int32_t x, int32_t y,
	  struct compost_shell *shell)
{
	struct weston_output *output;
	struct compost_xdg_popup *xdg_popup;
	struct compost_output *out;

	/* TODO maybe do something more sane than just using first output */
	weston_log("%s\n", __PRETTY_FUNCTION__);


	wl_list_for_each(out, &shell->outputs, link)
		if (out->output == surface->output)
			break;

	output = out->output;

	xdg_popup = malloc(sizeof(*xdg_popup));

	xdg_popup->client = client;
	xdg_popup->id = id;
	xdg_popup->surface = surface;
	xdg_popup->ox = x;
	xdg_popup->oy = y;

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

	surface->configure = &shell_popup_configure;
	surface->configure_private = xdg_popup;
	surface->output = output;

	xdg_popup->view = weston_view_create(surface);

	weston_view_set_position(xdg_popup->view, x, y);
	surface->timeline.force_refresh = 1;
	weston_layer_entry_insert(&out->default_layer.view_list,
	                          &xdg_popup->view->layer_link);

	weston_view_geometry_dirty(xdg_popup->view);

	return xdg_popup;
}
