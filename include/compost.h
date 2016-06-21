#ifndef _COMPOST_COMPOST_H_
#define _COMPOST_COMPOST_H_
#include <libweston-0/compositor.h>
#include <wayland-util.h>

const struct weston_pointer_grab_interface compost_pointer_grab_interface;
struct compost_output {
	struct wl_list link;
	int used;
	struct weston_output *output;
};

struct compost_shell {
	struct wl_listener destroy;
	struct wl_listener create_surface;
	struct wl_listener activate;
	struct wl_listener transform;
	struct wl_listener kill;
	struct wl_listener idle;
	struct wl_listener wake;
	struct wl_listener show_input_panel;
	struct wl_listener hide_input_panel;
	struct wl_listener update_input_panel;
	struct wl_listener seat_created;
	struct wl_listener output_created;
	struct wl_listener output_destroyed;
	struct wl_listener output_moved;
	struct wl_listener session;

	struct weston_layer tiling_layer;
	struct wl_list outputs;
	struct weston_compositor *ec;
};


void
compost_activate_binding(struct weston_pointer *pointer);

#endif /*_COMPOST_COMPOST_H_*/
