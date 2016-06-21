#include <libweston-0/compositor.h>

#include "compost.h"

static void
default_grab_pointer_focus(struct weston_pointer_grab *grab)
{
	struct weston_pointer *pointer = grab->pointer;
	struct weston_view *view;
	wl_fixed_t sx, sy;

	if (pointer->button_count > 0)
		return;

	view = weston_compositor_pick_view(pointer->seat->compositor,
					   pointer->x, pointer->y,
					   &sx, &sy);

	if (pointer->focus != view || pointer->sx != sx || pointer->sy != sy)
		weston_pointer_set_focus(pointer, view, sx, sy);
	compost_activate_binding(pointer);
}

static void
default_grab_pointer_motion(struct weston_pointer_grab *grab, uint32_t time,
			    struct weston_pointer_motion_event *event)
{
	struct weston_pointer *pointer = grab->pointer;
	struct wl_list *resource_list;
	struct wl_resource *resource;
	wl_fixed_t x, y;
	wl_fixed_t old_sx = pointer->sx;
	wl_fixed_t old_sy = pointer->sy;

	if (pointer->focus) {
		weston_pointer_motion_to_abs(pointer, event, &x, &y);
		weston_view_from_global_fixed(pointer->focus, x, y,
					      &pointer->sx, &pointer->sy);
	}

	weston_pointer_move(pointer, event);

	if (pointer->focus_client &&
	    (old_sx != pointer->sx || old_sy != pointer->sy)) {
		resource_list = &pointer->focus_client->pointer_resources;
		wl_resource_for_each(resource, resource_list) {
			wl_pointer_send_motion(resource, time,
					       pointer->sx, pointer->sy);
		}
	}
}

static void
default_grab_pointer_button(struct weston_pointer_grab *grab,
			    uint32_t time, uint32_t button, uint32_t state_w)
{
	struct weston_pointer *pointer = grab->pointer;
	struct weston_compositor *compositor = pointer->seat->compositor;
	struct weston_view *view;
	struct wl_resource *resource;
	uint32_t serial;
	enum wl_pointer_button_state state = state_w;
	struct wl_display *display = compositor->wl_display;
	wl_fixed_t sx, sy;
	struct wl_list *resource_list = NULL;

	if (pointer->focus_client)
		resource_list = &pointer->focus_client->pointer_resources;
	if (resource_list && !wl_list_empty(resource_list)) {
		resource_list = &pointer->focus_client->pointer_resources;
		serial = wl_display_next_serial(display);
		wl_resource_for_each(resource, resource_list)
			wl_pointer_send_button(resource,
					       serial,
					       time,
					       button,
					       state_w);
	}

	if (pointer->button_count == 0 &&
	    state == WL_POINTER_BUTTON_STATE_RELEASED) {
		view = weston_compositor_pick_view(compositor,
						   pointer->x, pointer->y,
						   &sx, &sy);

		weston_pointer_set_focus(pointer, view, sx, sy);
	}
}

static void
default_grab_pointer_axis(struct weston_pointer_grab *grab,
			  uint32_t time,
			  struct weston_pointer_axis_event *event)
{
	weston_pointer_send_axis(grab->pointer, time, event);
}

static void
default_grab_pointer_axis_source(struct weston_pointer_grab *grab,
				 uint32_t source)
{
	weston_pointer_send_axis_source(grab->pointer, source);
}

static void
default_grab_pointer_frame(struct weston_pointer_grab *grab)
{
	weston_pointer_send_frame(grab->pointer);
}

static void
default_grab_pointer_cancel(struct weston_pointer_grab *grab)
{
	(void) grab;
}

const struct weston_pointer_grab_interface
				compost_pointer_grab_interface = {
	default_grab_pointer_focus,
	default_grab_pointer_motion,
	default_grab_pointer_button,
	default_grab_pointer_axis,
	default_grab_pointer_axis_source,
	default_grab_pointer_frame,
	default_grab_pointer_cancel,
};


