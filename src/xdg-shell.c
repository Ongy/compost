#include <libweston-0/compositor.h>
#include <wayland-client-core.h>
#include <wayland-util.h>
#include <assert.h>

#include "xdg-shell-server-protocol.h"
#include "xdg-shell.h"
#include "xdg-surface.h"
#include "xdg-popup.h"

static struct xdg_shell_interface xdg_shell_implementation;

struct compost_xdg_shell {
	struct compost_shell *shell;

	struct wl_list surfaces;
};


static void
xdg_shell_delete(struct wl_resource *resource)
{
	struct compost_xdg_shell *shell;

	shell = (struct compost_xdg_shell *)
			wl_resource_get_user_data(resource);
	free(shell);
}

static void
bind_xdg_shell(struct wl_client *client, void *data,
               uint32_t version, uint32_t id)
{
	struct wl_resource *resource;
	struct compost_xdg_shell *shell;
	/* as long as it's unstable it's alwasy version 1*/
	assert(version == 1);


	shell = malloc(sizeof(*shell));
	shell->shell = (struct compost_shell *)data;

	wl_list_init(&shell->surfaces);

	resource = wl_resource_create(client, &xdg_shell_interface, 1, id);

	wl_resource_set_implementation(resource, &xdg_shell_implementation,
	                               shell, &xdg_shell_delete);
}

int
compost_bind_xdg_shell(struct wl_display *dpy, struct compost_shell *shell)
{
	wl_global_create(dpy, &xdg_shell_interface, 1, shell, bind_xdg_shell);
	return 0;
}

static void
compost_xdg_destroy(struct wl_client *client, struct wl_resource *resource)
{
	(void) client;
	(void) resource;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
compost_xdg_use_unstable_version(struct wl_client *client,
                                 struct wl_resource *resource,
                                 int32_t version)
{
	(void) client;
	(void) resource;
	(void) version;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static void
compost_xdg_get_xdg_surface(struct wl_client *client,
                            struct wl_resource *resource,
                            uint32_t id,
                            struct wl_resource *surface_resource)
{
	struct compost_xdg_surface *xdg_surface;
	struct compost_xdg_shell *shell;
	struct weston_surface *surface;
	weston_log("call %s\n", __PRETTY_FUNCTION__);

	shell = (struct compost_xdg_shell *)
			wl_resource_get_user_data(resource);
	surface = (struct weston_surface *)
			wl_resource_get_user_data(surface_resource);
	xdg_surface = compost_get_xdg_surface(client, shell->shell, id, surface);

	add_compost_xdg_surface_to_list(&shell->surfaces, xdg_surface);

	weston_log("exit %s\n", __PRETTY_FUNCTION__);
}

static void
compost_xdg_get_xdg_popup(struct wl_client *client,
                          struct wl_resource *resource,
                          uint32_t id,
                          struct wl_resource *surface_resource,
                          struct wl_resource *parent_resource,
                          struct wl_resource *seat_resource,
                          uint32_t serial, int32_t x, int32_t y)
{
	struct weston_surface *surface;
	struct compost_xdg_shell *shell;
	struct compost_xdg_surface *parent;
	(void) seat_resource; (void) serial;

	surface = (struct weston_surface *)
			wl_resource_get_user_data(surface_resource);
	parent = (struct compost_xdg_surface *)
			wl_resource_get_user_data(parent_resource);
	shell = (struct compost_xdg_shell *)
			wl_resource_get_user_data(resource);

	xdg_popup(client, id, surface, x, y, shell->shell, parent);

	weston_log("Created xdg_popup\n");
}

static void
compost_xdg_pong(struct wl_client *client, struct wl_resource *resource,
                 uint32_t serial)
{
	(void) client; (void) resource; (void) serial;
	weston_log("call %s\n", __PRETTY_FUNCTION__);
}

static struct xdg_shell_interface xdg_shell_implementation =
	{ compost_xdg_destroy
	, compost_xdg_use_unstable_version
	, compost_xdg_get_xdg_surface
	, compost_xdg_get_xdg_popup
	, compost_xdg_pong
	};
