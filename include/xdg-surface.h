#ifndef _COMPOST_XDG_SURFACE_H_
#define _COMPOST_XDG_SURFACE_H_

#include <wayland-util.h>

struct wl_client;
struct weston_surface;
struct compost_shell;
struct weston_view;
struct compost_shell;
struct compost_output;

struct compost_xdg_surface {
	struct wl_list link;
	struct wl_client *client;
	uint32_t id;
	struct weston_surface *surface;

	struct wl_resource *resource;
	struct weston_view *view;
	struct compost_shell *shell;
	struct compost_output *output;
};


struct compost_xdg_surface *
compost_get_xdg_surface(struct wl_client *client,
                        struct compost_shell *shell,
                        uint32_t id,
                        struct weston_surface *surface);
void
add_compost_xdg_surface_to_list(struct wl_list *, struct compost_xdg_surface *);

#endif /*_COMPOST_XDG_SURFACE_H_*/
