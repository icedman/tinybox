#ifndef TINYBOX_XWAYLNAD_H
#define TINYBOX_XWAYLNAD_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_server;

struct tbx_xwayland_shell {
    struct wlr_xwayland* wlr_xwayland;
    struct wl_listener new_xwayland_surface;
    struct tbx_server* server;
};

bool xwayland_shell_setup(struct tbx_server* server);

#endif // TINYBOX_XWAYLNAD_H