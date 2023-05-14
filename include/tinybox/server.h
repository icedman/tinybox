#ifndef TINYBOX_SERVER_H
#define TINYBOX_SERVER_H

#include <stdbool.h>
#include <wayland-server-core.h>
#include <wlr/util/box.h>

#include <tinybox/cursor.h>
#include <tinybox/output.h>
#include <tinybox/seat.h>
#include <tinybox/shell.h>
#include <tinybox/view.h>

struct tbx_server {
  struct wl_display *wl_display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;
  struct wlr_allocator *allocator;
  struct wlr_scene *scene;

  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;

  struct tbx_shell xdg_shell;
  struct tbx_shell xwayland_shell;
  struct wl_list views;

  struct wlr_cursor *cursor;
  struct wlr_xcursor_manager *cursor_mgr;
  struct wl_listener cursor_motion;
  struct wl_listener cursor_motion_absolute;
  struct wl_listener cursor_button;
  struct wl_listener cursor_axis;
  struct wl_listener cursor_frame;

  struct wlr_seat *seat;
  struct wl_listener new_input;
  struct wl_listener request_cursor;
  struct wl_listener request_set_selection;
  struct wl_list keyboards;
  enum tbx_cursor_mode cursor_mode;
  struct tbx_view *grabbed_view;
  bool grabbing;
  double grab_x, grab_y;
  struct wlr_box grab_geobox;
  uint32_t resize_edges;
};

bool
tbx_server_setup(struct tbx_server *server);

bool
tbx_server_start(struct tbx_server *server);

void
tbx_server_shutdown(struct tbx_server *server);

#endif // TINYBOX_SERVER_H