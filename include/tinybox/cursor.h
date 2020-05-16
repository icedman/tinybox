#ifndef TINYBOX_CURSOR_H
#define TINYBOX_CURSOR_H

#include "tinybox/server.h"
#include <wlr/render/wlr_renderer.h>

struct tbx_server;
struct wlr_cursor;
struct wlr_input_device;

enum tbx_cursor_mode {
    TBX_CURSOR_PASSTHROUGH,
    TBX_CURSOR_MOVE,
    TBX_CURSOR_RESIZE,
    TBX_CURSOR_SWIPE_WORKSPACE
};

struct tbx_cursor {
    enum tbx_cursor_mode mode;
    struct wlr_cursor* cursor;
    struct wlr_xcursor_manager* xcursor_manager;

    struct wl_listener cursor_motion;
    struct wl_listener cursor_motion_absolute;
    struct wl_listener cursor_button;
    struct wl_listener cursor_axis;
    struct wl_listener cursor_frame;
    struct wl_listener cursor_swipe_begin;
    struct wl_listener cursor_swipe_update;
    struct wl_listener cursor_swipe_end;

    double swipe_begin_x, swipe_begin_y;
    double swipe_x, swipe_y;
    int swipe_fingers;

    struct tbx_view* grab_view;
    double grab_x, grab_y;
    struct wlr_box grab_box;
    uint32_t resize_edges;

    struct tbx_server* server;
};

bool cursor_setup(struct tbx_server* server);
void cursor_attach(struct tbx_server* server, struct wlr_input_device* device);

#endif // TINYBOX_CURSOR_Hcursor_init