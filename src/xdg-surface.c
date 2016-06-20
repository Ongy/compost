#include <libweston-0/compositor.h>
#include <wayland-client-core.h>
#include <wayland-util.h>
#include <assert.h>
#include <limits.h>

#include "xdg-shell-server-protocol.h"
#include "xdg-surface.h"
#include "xdg-shell.h"
#include "compost.h"

const struct xdg_surface_interface xdg_surface_implementation;

void
add_compost_xdg_surface_to_list(struct wl_list *list,
                                struct compost_xdg_surface *surface)
{
	(void) list; (void) surface;
	wl_list_insert(list, &surface->link);
}

static void
xdg_surface_move(struct wl_client *client, struct wl_resource *resource,
                 struct wl_resource *seat_resource, uint32_t serial)
{
	(void) client; (void) resource; (void) seat_resource; (void) serial;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
shell_surface_configure(struct weston_surface *s, int32_t x, int32_t y)
{
	(void) x; (void) y;
	weston_log("call %s\n", __PRETTY_FUNCTION__);

	if (s->output != NULL)
		weston_output_schedule_repaint(s->output);
}

struct compost_xdg_surface *
weston_to_compost_xdg_surface(struct weston_surface *surface)
{
	if (surface->configure == shell_surface_configure)
		return surface->configure_private;
	return NULL;
}


static void
xdg_surface_delete(struct wl_resource *resource)
{
	struct compost_xdg_surface *surface =
		(struct compost_xdg_surface *)
			wl_resource_get_user_data(resource);
	weston_log("Deleting surface\n");
	surface->output->used -= 1;
	wl_list_remove(&surface->link);
	/* So the poopups don't write into freed memory when removing
	 * themselves from the list
	 */
	wl_list_remove(&surface->popups);

	/* TODO */
	free(surface);
}

static void
xdg_surface_set_parent(struct wl_client *client, struct wl_resource *resource,
                       struct wl_resource *parent_resource)
{
	(void) client; (void) resource; (void) parent_resource;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_set_app_id(struct wl_client *client, struct wl_resource *resource,
                       const char *app_id)
{
	(void) client; (void) resource; (void) app_id;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_set_title(struct wl_client *client,
                          struct wl_resource *resource,
                          const char *title)
{
	(void) client; (void) resource; (void) title;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_show_window_menu(struct wl_client *client,
                             struct wl_resource *surface_resource,
                             struct wl_resource *seat_resource,
                             uint32_t serial,
                             int32_t x,
                             int32_t y)
{
	(void) client; (void) surface_resource; (void) seat_resource;
	(void) serial; (void) x; (void) y;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_resize(struct wl_client *client, struct wl_resource *resource,
                   struct wl_resource *seat_resource, uint32_t serial,
                   uint32_t edges)
{
	(void) client; (void) resource; (void) seat_resource; (void) serial;
	(void) edges;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_ack_configure(struct wl_client *client,
                          struct wl_resource *resource,
                          uint32_t serial)
{
	(void) client; (void) resource; (void) serial;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_set_window_geometry(struct wl_client *client,
                                struct wl_resource *resource,
                                int32_t x,
                                int32_t y,
                                int32_t width,
                                int32_t height)
{
	(void) client; (void) resource; (void) x; (void) y;
	(void) width; (void) height;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_set_maximized(struct wl_client *client,
                          struct wl_resource *resource)
{
	(void) client; (void) resource;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_unset_maximized(struct wl_client *client,
                            struct wl_resource *resource)
{
	(void) client; (void) resource;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_set_fullscreen(struct wl_client *client,
                           struct wl_resource *resource,
			   struct wl_resource *output_resource)
{
	(void) client; (void) resource; (void) output_resource;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_unset_fullscreen(struct wl_client *client,
                             struct wl_resource *resource)
{
	(void) client; (void) resource;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
xdg_surface_set_minimized(struct wl_client *client,
                          struct wl_resource *resource)
{
	(void) client; (void) resource;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

struct compost_xdg_surface *
compost_get_xdg_surface(struct wl_client *client,
                        struct compost_shell *shell,
                        uint32_t id,
                        struct weston_surface *surface)
{
	int min = INT_MAX;
	struct weston_output *output;
	struct compost_xdg_surface *xdg_surface;
	struct wl_array array;
	struct compost_output *out;
	struct compost_output *pos;

	wl_list_for_each(pos, &shell->outputs, link) {
		weston_log("Iterating over output while creating surface: %d\n",
		           pos->used);
		if (pos->used < min) {
			weston_log("Choosing output: %s\n", pos->output->name);
			out = pos;
			min = out->used;
		}
	}

	if (&out->link == &shell->outputs) {
		weston_log("fatal: didn't find a usable output\n");
	}

	output = out->output;
	surface->output = output;

	xdg_surface = malloc(sizeof(*xdg_surface));

	xdg_surface->shell = shell;
	xdg_surface->output = out;
	xdg_surface->id = id;
	xdg_surface->surface = surface;
	wl_list_init(&xdg_surface->popups);

	xdg_surface->resource = wl_resource_create(client,
	                                           &xdg_surface_interface,
	                                           1, id);
	wl_resource_set_implementation(xdg_surface->resource,
	                               &xdg_surface_implementation,
	                               xdg_surface, xdg_surface_delete);

	if (weston_surface_set_role(surface, "xdg_surface",
	                            xdg_surface->resource,
	                            XDG_SHELL_ERROR_ROLE) < 0) {
		weston_log("TODO");
		return NULL;
	}

	surface->configure = &shell_surface_configure;
	surface->configure_private = xdg_surface;

	xdg_surface->view = weston_view_create(surface);
	weston_view_set_position(xdg_surface->view, output->x, output->y);
	surface->timeline.force_refresh = 1;
	weston_layer_entry_insert(&shell->tiling_layer.view_list,
	                          &xdg_surface->view->layer_link);

	out->used += 1;
	wl_array_init(&array);
	wl_array_add(&array, sizeof(uint32_t)*2);
	((uint32_t *)array.data)[0] = XDG_SURFACE_STATE_MAXIMIZED;
	((uint32_t *)array.data)[1] = XDG_SURFACE_STATE_ACTIVATED;
	xdg_surface_send_configure(xdg_surface->resource,
	                           output->width, output->height,
	                           &array, 10);
	wl_array_release(&array);

	weston_view_geometry_dirty(xdg_surface->view);

	return xdg_surface;
}

static void
xdg_surface_destroy(struct wl_client *client,
                    struct wl_resource *resource)
{
	struct compost_xdg_surface *surface;
	(void) client;

	surface = wl_resource_get_user_data(resource);

	weston_log("Destroying surface\n");
	if (!wl_list_empty(&surface->popups)) {
		wl_resource_post_error(resource,
		                       XDG_SHELL_ERROR_DEFUNCT_SURFACES,
		                       "XDG-SURFACE destroyed while popup existed\n");
		weston_log("Child destroyed xdg-surface while children existed\n");
	}


	wl_resource_destroy(resource);
}

const struct xdg_surface_interface xdg_surface_implementation = {
	xdg_surface_destroy,
	xdg_surface_set_parent,
	xdg_surface_set_title,
	xdg_surface_set_app_id,
	xdg_surface_show_window_menu,
	xdg_surface_move,
	xdg_surface_resize,
	xdg_surface_ack_configure,
	xdg_surface_set_window_geometry,
	xdg_surface_set_maximized,
	xdg_surface_unset_maximized,
	xdg_surface_set_fullscreen,
	xdg_surface_unset_fullscreen,
	xdg_surface_set_minimized,
};
