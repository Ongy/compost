#ifndef _COMPOST_COMPOST_H_
#define _COMPOST_COMPOST_H_
#include <libweston-0/compositor.h>

struct compost_shell {
	struct weston_layer default_layer;
	struct weston_compositor *ec;
} compost_shell;

#endif /*_COMPOST_COMPOST_H_*/
