#ifndef TBX_DECORATION_H
#define TBX_DECORATION_H

#include <wlr/types/wlr_server_decoration.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>

struct tbx_server_decoration {
    struct wlr_server_decoration *wlr_server_decoration;
    struct wl_list link;

    struct wl_listener destroy;
    struct wl_listener mode;
};

struct tbx_server_decoration *decoration_from_surface(
    struct wlr_surface *surface);

struct tbx_xdg_decoration {
    struct wlr_xdg_toplevel_decoration_v1 *wlr_xdg_decoration;
    struct wl_list link;

    struct tbx_view *view;

    struct wl_listener destroy;
    struct wl_listener request_mode;
};

struct tbx_xdg_decoration *xdg_decoration_from_surface(
    struct wlr_surface *surface);

void server_decoration_init();
void xdg_decoration_init();

#endif// TBX_DECORATION_H
