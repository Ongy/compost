#ifndef _COMPOST_XDG_POPUP_H_
#define _COMPOST_XDG_POPUP_H_

struct compost_xdg_popup;
struct wl_client;
struct weston_surface;

struct compost_xdg_popup *
xdg_popup(struct wl_client *client, uint32_t id,
          struct weston_surface *surface,
          int32_t x, int32_t y);

#endif /*_COMPOST_XDG_POPUP_H_*/
