#ifndef _COMPOST_WLSIGNALS_H_
#define _COMPOST_WLSIGNALS_H_

struct compost_shell;

int
compost_bind_signals(struct weston_compositor *ec,
                     struct compost_shell *shell);

#endif /*_COMPOST_WLSIGNALS_H_*/
