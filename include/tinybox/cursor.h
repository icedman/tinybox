#ifndef TINYBOX_CURSOR_H
#define TINYBOX_CURSOR_H

struct tbx_server;
struct wlr_cursor;
struct wlr_input_device;

enum tbx_cursor_mode {
  TBX_CURSOR_PASSTHROUGH,
  TBX_CURSOR_MOVE,
  TBX_CURSOR_RESIZE,
};

struct tbx_cursor {
    enum tbx_cursor_mode mode;
    struct wlr_cursor *cursor;
    struct wlr_xcursor_manager *xcursor_manager;

  struct wl_listener cursor_motion;
  struct wl_listener cursor_motion_absolute;
  struct wl_listener cursor_button;
  struct wl_listener cursor_axis;
  struct wl_listener cursor_frame;
  struct wl_listener cursor_swipe_begin;
  struct wl_listener cursor_swipe_update;
  struct wl_listener cursor_swipe_end;

  struct tbx_server *server;
};

bool cursor_setup(struct tbx_server *server);
void cursor_attach(struct tbx_server *server, struct wlr_input_device *device);

#endif // TINYBOX_CURSOR_Hcursor_init