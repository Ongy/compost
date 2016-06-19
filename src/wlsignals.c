#include <libweston-0/compositor.h>
#include <wayland-server-core.h>

#include "compost.h"

static void
compost_destroy(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: destroy\n");
}

static void
compost_create_surface(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: create_surface\n");
}

static void
compost_activate(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	//weston_log("signal: activate\n");
}

static void
compost_transform(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: transform\n");
}

static void
compost_kill(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: kill\n");
}

static void
compost_idle(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: idle\n");
}

static void
compost_wake(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: wake\n");
}

static void
compost_show_input_panel(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: show_input_panel\n");
}

static void
compost_hide_input_panel(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: hide_input_panel\n");
}

static void
compost_update_input_panel(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: update_input_panel\n");
}

static void
compost_seat_created(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: seat_created\n");
}

static void
set_output_background(struct compost_shell *shell, struct weston_output *out)
{
	struct compost_output *c_out;
	struct weston_compositor *ec = shell->ec;
	struct weston_surface *background;
	struct weston_view *bview;

	c_out = malloc(sizeof(*c_out));
	c_out->used = 0;
	c_out->output = out;

	wl_list_insert(&shell->outputs, &c_out->link);

	weston_layer_init(&c_out->default_layer, &ec->cursor_layer.link);
	background = weston_surface_create(ec);
	weston_surface_set_color(background, 0.5, 0.5, 0.5, 1.0);

	weston_surface_set_size(background, out->width, out->height);
	pixman_region32_fini(&background->opaque);
	pixman_region32_init_rect(&background->opaque,
	                          out->x, out->y, /* position */
	                          out->width, out->height);
	weston_surface_damage(background);

	bview = weston_view_create(background);
	weston_view_set_position(bview, out->x, out->y);
	background->timeline.force_refresh = 1;
	weston_layer_entry_insert(&c_out->default_layer.view_list,
	                          &bview->layer_link);

}

static void
compost_output_created(struct wl_listener *listener, void *data)
{
	struct weston_output *output = (struct weston_output *)data;
	struct compost_shell *shell;

	shell = wl_container_of(listener, shell, output_created);

	weston_log("signal: output_created\n");
	weston_log("Created output: \"%s\" %dx%d+%dx%d\n", output->name,
	           output->width, output->height, output->y, output->x);

	set_output_background(shell, output);
}

static void
compost_output_destroyed(struct wl_listener *listener, void *data)
{
	struct weston_output *output = (struct weston_output *)data;
	(void) listener;
	weston_log("signal: output_destroyed\n");
	weston_log("Destroyed output: \"%s\"\n", output->name);
}

static void
compost_output_moved(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: output_moved\n");
}

static void
compost_session(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: session\n");
}

int
compost_bind_signals(struct weston_compositor *ec, struct compost_shell *shell)
{
	shell->destroy.notify = compost_destroy;
	shell->create_surface.notify = compost_create_surface;
	shell->activate.notify = compost_activate;
	shell->transform.notify = compost_transform;
	shell->kill.notify = compost_kill;
	shell->idle.notify = compost_idle;
	shell->wake.notify = compost_wake;
	shell->show_input_panel.notify = compost_show_input_panel;
	shell->hide_input_panel.notify = compost_hide_input_panel;
	shell->update_input_panel.notify = compost_update_input_panel;
	shell->seat_created.notify = compost_seat_created;
	shell->output_created.notify = compost_output_created;
	shell->output_destroyed.notify = compost_output_destroyed;
	shell->output_moved.notify = compost_output_moved;
	shell->session.notify = compost_session;


	wl_signal_add(&ec->destroy_signal, &shell->destroy);
	wl_signal_add(&ec->create_surface_signal, &shell->create_surface);
	wl_signal_add(&ec->activate_signal, &shell->activate);
	wl_signal_add(&ec->transform_signal, &shell->transform);
	wl_signal_add(&ec->kill_signal, &shell->kill);
	wl_signal_add(&ec->idle_signal, &shell->idle);
	wl_signal_add(&ec->wake_signal, &shell->wake);
	wl_signal_add(&ec->show_input_panel_signal, &shell->show_input_panel);
	wl_signal_add(&ec->hide_input_panel_signal, &shell->hide_input_panel);
	wl_signal_add(&ec->update_input_panel_signal, &shell->update_input_panel);
	wl_signal_add(&ec->seat_created_signal, &shell->seat_created);
	wl_signal_add(&ec->output_created_signal, &shell->output_created);
	wl_signal_add(&ec->output_destroyed_signal, &shell->output_destroyed);
	wl_signal_add(&ec->output_moved_signal, &shell->output_moved);
	wl_signal_add(&ec->session_signal, &shell->session);
	weston_log("bound all signals\n");
	return 0;
}
