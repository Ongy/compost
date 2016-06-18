#include <libweston-0/compositor.h>
#include <libweston-0/compositor-x11.h>
#include <libweston-0/compositor-drm.h>
#include <linux/input.h>

#include <wayland-util.h>
#include <wayland-client-core.h>

#include <string.h>

#include "wlsignals.h"
#include "xdg-shell.h"
#include "compost.h"

struct compost_shell compost_shell;

static void
terminate_binding(struct weston_keyboard *keyboard, uint32_t time,
                  uint32_t key, void *data)
{
	struct weston_compositor *compositor = data;
	(void) keyboard; (void) time; (void) key;

	wl_display_terminate(compositor->wl_display);
}


static int
load_shell(struct weston_compositor *ec)
{
	struct weston_output *out;
	struct weston_surface *background;
	struct weston_view *bview;

	compost_shell.ec = ec;
	weston_layer_init(&compost_shell.default_layer, &ec->cursor_layer.link);
	background = weston_surface_create(ec);
	weston_surface_set_color(background, 0.5, 0.5, 0.5, 1.0);

	out = wl_container_of(&ec->output_list.next, out, link);

	out->width = 1280;
	out->height = 720;
	weston_surface_set_size(background,
	                        out->width, out->height);
	pixman_region32_fini(&background->opaque);
	pixman_region32_init_rect(&background->opaque,
	                          0, 0, /* position */
	                          out->width, out->height);
	weston_surface_damage(background);

	bview = weston_view_create(background);
	weston_view_set_position(bview, 0, 0);
	background->timeline.force_refresh = 1;
	weston_layer_entry_insert(&compost_shell.default_layer.view_list,
	                          &bview->layer_link);

	weston_compositor_add_key_binding(ec, KEY_BACKSPACE,
	                                  MODIFIER_CTRL | MODIFIER_ALT,
	                                  &terminate_binding, ec);
	weston_log("set up default view: %d %d\n", out->width, out->height);
	return 0;
}

//static struct wl_list child_process_list;
static int
compost_log(const char *fmt, va_list ap)
{
	return vfprintf(stderr, fmt, ap);
}

static int
compost_log_continue(const char *fmt, va_list ap)
{
	return vfprintf(stderr, fmt, ap);
}

/*Get into libweston? */
static int
choose_default_backend(void)
{
	int backend = -1;

	// Disable wayland backend for now
/*	if (getenv("WAYLAND_DISPLAY") || getenv("WAYLAND_SOCKET"))
		backend = strdup("wayland-backend.so");
	else*/ if (getenv("DISPLAY"))
		backend = WESTON_BACKEND_X11;
	else
		backend = WESTON_BACKEND_DRM;

	return backend;
}

static int
load_drm_backend(struct weston_compositor *ec)
{
	struct weston_drm_backend_config config;

	memset(&config, 0, sizeof(config));

	config.base.struct_version = WESTON_DRM_BACKEND_CONFIG_VERSION;
	/* This tells libweston to use it's on default values */
	config.base.struct_size = sizeof(struct weston_backend_config);

	return weston_compositor_load_backend(ec, WESTON_BACKEND_DRM,
	                                      &config.base);
}


static int
load_x11_backend(struct weston_compositor *ec)
{
	struct weston_x11_backend_output_config out =
		{ 1280
		, 720
		, "X1"
		, WL_OUTPUT_TRANSFORM_NORMAL
		, 1
		};
	struct weston_x11_backend_config config =
		{ { 0 }
		, 0
		, 0
		, 0
		, 1
		, &out
		};

	config.base.struct_version = WESTON_X11_BACKEND_CONFIG_VERSION;
	/* This tells libweston to use it's on default values */
	config.base.struct_size = sizeof(struct weston_x11_backend_config);

	return weston_compositor_load_backend(ec, WESTON_BACKEND_X11,
	                                      &config.base);
}

static int
load_backend(struct weston_compositor *ec, int backend)
{
	switch (backend) {
	case WESTON_BACKEND_X11:
		return load_x11_backend(ec);
	case WESTON_BACKEND_DRM:
		return load_drm_backend(ec);
		break;
	default:
		weston_log("Cannot handle backend: %d\n", backend);
		return -1;
	}
}

static void
handle_exit(struct weston_compositor *ec)
{
	wl_display_terminate(ec->wl_display);
	weston_log("Exiting\n");
}

int
main(int argc, char **argv)
{
	const char *sock_name;
	int ret = 0;
	struct xkb_rule_names xkb_names = { 0 };
	struct wl_display *display = NULL;
	int backend = 0;
	struct weston_compositor *ec = NULL;

	/* TODO maybe some args parsing? */
	(void) argc; (void) argv;

	weston_log_set_handler(compost_log, compost_log_continue);

	display = wl_display_create();
	sock_name = wl_display_add_socket_auto(display);
	weston_log("socket name = %s\n", sock_name);

	/* set the environment for children */
	setenv("WAYLAND_DISPLAY", sock_name, 1);

	backend = choose_default_backend();

	if((ec = weston_compositor_create(display, NULL)) == NULL) {
		weston_log("fatal: failed to create compositor\n");
		ret = -1;
		goto out;
	}

	/* this has to be called at least once */
	if (weston_compositor_set_xkb_rule_names(ec, &xkb_names) < 0) {
		weston_log("fatal: could not set xkb_names\n");
		ret = -1;
		goto out;
	}

	if (compost_bind_signals(ec) < 0) {
		weston_log("fatal: something went wrong while binding singals\n");
		ret = -1;
		goto out;
	}

	if (load_backend(ec, backend) < 0) {
		weston_log("fatal: failed to create compositor backend\n");
		ret = -1;
		goto out;
	}

	if (load_shell(ec) < 0) {
		weston_log("fatal: failed to load shell\n");
		ret = -1;
		goto out;
	}

	if (compost_bind_xdg_shell(display) < 0) {
		weston_log("fatal: failed to load xdg_shell\n");
		ret = -1;
		goto out;
	}

	ec->exit = handle_exit;

	weston_compositor_wake(ec);
	wl_display_run(display);

	/* Allow for setting return exit code after
	* wl_display_run returns normally. This is
	* useful for devs/testers and automated tests
	* that want to indicate failure status to
	* testing infrastructure above
	*/
	ret = ec->exit_code;

      out:
	if (ec != NULL)
		weston_compositor_destroy(ec);


	return ret;
}
