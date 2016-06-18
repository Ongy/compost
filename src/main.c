#include <libweston-0/compositor.h>
#include <libweston-0/compositor-x11.h>
#include <libweston-0/compositor-drm.h>
#include <wayland-client-core.h>

#include <string.h>

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
	struct weston_x11_backend_config config;

	memset(&config, 0, sizeof(config));

	config.base.struct_version = WESTON_X11_BACKEND_CONFIG_VERSION;
	/* This tells libweston to use it's on default values */
	config.base.struct_size = sizeof(struct weston_backend_config);

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

int
main(int argc, char **argv)
{
	struct wl_display *display = NULL;
	int backend = 0;
	struct weston_compositor *ec = NULL;

	/* TODO maybe some args parsing? */
	(void) argc; (void) argv;

	weston_log_set_handler(compost_log, compost_log_continue);

	display = wl_display_create();
	//wl_list_init(&child_process_list);

	backend = choose_default_backend();

	if((ec = weston_compositor_create(display, NULL)) == NULL) {
		weston_log("fatal: failed to create compositor\n");
		goto out;
	}

	if (load_backend(ec, backend) < 0) {
		weston_log("fatal: failed to create compositor backend\n");
		goto out;
	}

	return 0;
      out:

	return -1;
}
