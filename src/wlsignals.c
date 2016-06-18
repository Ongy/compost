#include <libweston-0/compositor.h>
#include <wayland-server-core.h>

static void
compost_destroy(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: destroy\n");
}
static struct wl_listener destroy = { { 0 }, compost_destroy };

static void
compost_create_surface(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: create_surface\n");
}
static struct wl_listener create_surface = { { 0 }, compost_create_surface };

static void
compost_activate(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: activate\n");
}
static struct wl_listener activate = { { 0 }, compost_activate };

static void
compost_transform(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: transform\n");
}
static struct wl_listener transform = { { 0 }, compost_transform };

static void
compost_kill(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: kill\n");
}
static struct wl_listener kill = { { 0 }, compost_kill };

static void
compost_idle(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: idle\n");
}
static struct wl_listener idle = { { 0 }, compost_idle };

static void
compost_wake(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: wake\n");
}
static struct wl_listener wake = { { 0 }, compost_wake };

static void
compost_show_input_panel(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: show_input_panel\n");
}
static struct wl_listener show_input_panel = { { 0 }, compost_show_input_panel };

static void
compost_hide_input_panel(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: hide_input_panel\n");
}
static struct wl_listener hide_input_panel = { { 0 }, compost_hide_input_panel };

static void
compost_update_input_panel(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: update_input_panel\n");
}
static struct wl_listener update_input_panel = { { 0 }, compost_update_input_panel };

static void
compost_seat_created(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: seat_created\n");
}
static struct wl_listener seat_created = { { 0 }, compost_seat_created };

static void
compost_output_created(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: output_created\n");
}
static struct wl_listener output_created = { { 0 }, compost_output_created };

static void
compost_output_destroyed(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: output_destroyed\n");
}
static struct wl_listener output_destroyed = { { 0 }, compost_output_destroyed };

static void
compost_output_moved(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: output_moved\n");
}
static struct wl_listener output_moved = { { 0 }, compost_output_moved };

static void
compost_session(struct wl_listener *listener, void *data)
{
	(void) listener; (void) data;
	weston_log("signal: session\n");
}
static struct wl_listener session = { { 0 }, compost_session };

int
compost_bind_signals(struct weston_compositor *ec)
{
    wl_signal_add(&ec->destroy_signal, &destroy);
    wl_signal_add(&ec->create_surface_signal, &create_surface);
    wl_signal_add(&ec->activate_signal, &activate);
    wl_signal_add(&ec->transform_signal, &transform);
    wl_signal_add(&ec->kill_signal, &kill);
    wl_signal_add(&ec->idle_signal, &idle);
    wl_signal_add(&ec->wake_signal, &wake);
    wl_signal_add(&ec->show_input_panel_signal, &show_input_panel);
    wl_signal_add(&ec->hide_input_panel_signal, &hide_input_panel);
    wl_signal_add(&ec->update_input_panel_signal, &update_input_panel);
    wl_signal_add(&ec->seat_created_signal, &seat_created);
    wl_signal_add(&ec->output_created_signal, &output_created);
    wl_signal_add(&ec->output_destroyed_signal, &output_destroyed);
    wl_signal_add(&ec->output_moved_signal, &output_moved);
    wl_signal_add(&ec->session_signal, &session);
    weston_log("bound all signals\n");
    return 0;
}
