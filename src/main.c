#include <libweston-0/compositor.h>
#include <libweston-0/compositor-x11.h>
#include <libweston-0/compositor-drm.h>
#include <linux/input.h>

#include <wayland-util.h>
#include <wayland-client-core.h>
#include <string.h>
#include <signal.h>

#include "wlsignals.h"
#include "xdg-shell.h"
#include "compost.h"

#define __USE_GNU
#include <dlfcn.h>
#undef __USE_GNU
#define UNW_LOCAL_ONLY
#include <libunwind.h>

struct compost_shell compost_shell;

static FILE *log_file = NULL;
static void
print_backtrace(void)
{
	unw_cursor_t cursor;
	unw_context_t context;
	unw_word_t off;
	unw_proc_info_t pip;
	int ret, i = 0;
	char procname[256];
	const char *filename;
	Dl_info dlinfo;

	pip.unwind_info = NULL;
	ret = unw_getcontext(&context);
	if (ret) {
		weston_log("unw_getcontext: %d\n", ret);
		return;
	}

	ret = unw_init_local(&cursor, &context);
	if (ret) {
		weston_log("unw_init_local: %d\n", ret);
		return;
	}

	ret = unw_step(&cursor);
	while (ret > 0) {
		ret = unw_get_proc_info(&cursor, &pip);
		if (ret) {
			weston_log("unw_get_proc_info: %d\n", ret);
			break;
		}

		ret = unw_get_proc_name(&cursor, procname, 256, &off);
		if (ret && ret != -UNW_ENOMEM) {
			if (ret != -UNW_EUNSPEC)
				weston_log("unw_get_proc_name: %d\n", ret);
			procname[0] = '?';
			procname[1] = 0;
		}

		if (dladdr((void *)(pip.start_ip + off), &dlinfo) && dlinfo.dli_fname &&
		    *dlinfo.dli_fname)
			filename = dlinfo.dli_fname;
		else
			filename = "?";

		weston_log("%u: %s (%s%s+0x%x) [%p]\n", i++, filename, procname,
			   ret == -UNW_ENOMEM ? "..." : "", (int)off, (void *)(pip.start_ip + off));

		ret = unw_step(&cursor);
		if (ret < 0)
			weston_log("unw_step: %d\n", ret);
	}
}

static void
terminate_binding(struct weston_keyboard *keyboard, uint32_t time,
                  uint32_t key, void *data)
{
	struct weston_compositor *compositor = data;
	(void) keyboard; (void) time; (void) key;

	wl_display_terminate(compositor->wl_display);
}

static struct weston_compositor *segv_comp;
static void
on_signal(int s, siginfo_t *siginfo, void *context)
{
	(void) s; (void) siginfo; (void) context;
	print_backtrace();

	segv_comp->backend->restore(segv_comp);

	raise(SIGTRAP);
}

static void
catch_signals()
{
	struct sigaction action;

	action.sa_flags = SA_SIGINFO | SA_RESETHAND;
	action.sa_sigaction = on_signal;

	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGABRT, &action, NULL);
}


static int
setup_shell(struct weston_compositor *ec, struct compost_shell *shell)
{
	shell->ec = ec;
	weston_compositor_add_key_binding(ec, KEY_BACKSPACE,
	                                  MODIFIER_CTRL | MODIFIER_ALT,
	                                  &terminate_binding, ec);
	wl_list_init(&shell->outputs);
	return 0;
}

//static struct wl_list child_process_list;
static int
compost_log(const char *fmt, va_list ap)
{
	if (log_file)
		return vfprintf(log_file, fmt, ap);
	else
		return vfprintf(stderr, fmt, ap);
}

static int
compost_log_continue(const char *fmt, va_list ap)
{
	if (log_file)
		return vfprintf(log_file, fmt, ap);
	else
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

static enum weston_drm_backend_output_mode
configure_output(struct weston_compositor *ec, bool use_current_mode,
                 const char *name,
                 struct weston_drm_backend_output_config *output_config)
{
	(void) ec; (void) use_current_mode; (void) name;

	output_config->base.scale = 1;

	return WESTON_DRM_BACKEND_OUTPUT_CURRENT;
}

static void
configure_device(struct weston_compositor *ec, struct libinput_device *device)
{
	(void) ec; (void) device;
}

static int
load_drm_backend(struct weston_compositor *ec)
{
	struct weston_drm_backend_config config;

	memset(&config, 0, sizeof(config));

	log_file = fopen("/tmp/compost.log", "w+");

	config.configure_output = configure_output;
	config.configure_device = configure_device;
	//config.tty = 6;

	config.base.struct_version = WESTON_DRM_BACKEND_CONFIG_VERSION;
	/* This tells libweston to use it's on default values */
	config.base.struct_size = sizeof(struct weston_drm_backend_config);

	return weston_compositor_load_backend(ec, WESTON_BACKEND_DRM,
	                                      &config.base);
}


static int
load_x11_backend(struct weston_compositor *ec)
{
	struct weston_x11_backend_output_config out[] =
		{ { 1280
		  , 720
		  , "X1"
		  , WL_OUTPUT_TRANSFORM_NORMAL
		  , 1
		  },
		  { 1280
		  , 720
		  , "X2"
		  , WL_OUTPUT_TRANSFORM_NORMAL
		  , 1
		  }
		};
	struct weston_x11_backend_config config =
		{ { 0 }
		, 0
		, 0
		, 0
		, 2
		, out
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
	struct compost_shell *shell;
	int ret = 0;
	struct xkb_rule_names xkb_names = { 0 };
	struct wl_display *display = NULL;
	int backend = 0;
	struct weston_compositor *ec = NULL;

	/* TODO maybe some args parsing? */
	(void) argc; (void) argv;

	/* Allocate our shell right in the beginning */
	shell = malloc(sizeof(*shell));

	weston_log_set_handler(compost_log, compost_log_continue);

	catch_signals();

	display = wl_display_create();
	sock_name = wl_display_add_socket_auto(display);
	weston_log("socket name = %s\n", sock_name);

	/* set the environment for children */
	setenv("WAYLAND_DISPLAY", sock_name, 1);

	if((ec = weston_compositor_create(display, NULL)) == NULL) {
		weston_log("fatal: failed to create compositor\n");
		ret = -1;
		goto out;
	}

	segv_comp = ec;

	ec->vt_switching = 1;

	/* this has to be called at least once */
	if (weston_compositor_set_xkb_rule_names(ec, &xkb_names) < 0) {
		weston_log("fatal: could not set xkb_names\n");
		ret = -1;
		goto out;
	}

	if (setup_shell(ec, shell) < 0) {
		weston_log("fatal: failed to load shell\n");
		ret = -1;
		goto out;
	}

	if (compost_bind_signals(ec, shell) < 0) {
		weston_log("fatal: something went wrong while binding singals\n");
		ret = -1;
		goto out;
	}

	backend = choose_default_backend();
	if (load_backend(ec, backend) < 0) {
		weston_log("fatal: failed to create compositor backend\n");
		ret = -1;
		goto out;
	}

	ec->user_data = shell;


	if (compost_bind_xdg_shell(display, shell) < 0) {
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
