#ifndef _COMPOST_XDG_SHELL_H_
#define _COMPOST_XDG_SHELL_H_

struct wl_display;
struct compost_shell;

int
compost_bind_xdg_shell(struct wl_display *dpy, struct compost_shell *shell);

#endif /*_COMPOST_XDG_SHELL_H_*/
