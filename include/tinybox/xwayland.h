#ifndef TINYBOX_XWAYLNAD_H
#define TINYBOX_XWAYLNAD_H

#include <stdbool.h>
#include <wayland-server-core.h>

#include <xcb/xproto.h>

enum atom_name {
    NET_WM_WINDOW_TYPE_NORMAL,
    NET_WM_WINDOW_TYPE_DIALOG,
    NET_WM_WINDOW_TYPE_UTILITY,
    NET_WM_WINDOW_TYPE_TOOLBAR,
    NET_WM_WINDOW_TYPE_SPLASH,
    NET_WM_WINDOW_TYPE_MENU,
    NET_WM_WINDOW_TYPE_DROPDOWN_MENU,
    NET_WM_WINDOW_TYPE_POPUP_MENU,
    NET_WM_WINDOW_TYPE_TOOLTIP,
    NET_WM_WINDOW_TYPE_NOTIFICATION,
    NET_WM_STATE_MODAL,
    ATOM_LAST,
};

struct tbx_server;

struct tbx_xwayland_shell {
    struct wlr_xwayland* wlr_xwayland;
    struct wl_listener new_xwayland_surface;
    struct tbx_server* server;

    xcb_atom_t atoms[ATOM_LAST];
};

bool xwayland_shell_setup(struct tbx_server* server);

#endif // TINYBOX_XWAYLNAD_H