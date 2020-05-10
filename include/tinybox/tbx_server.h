#ifndef TBX_SERVER_H
#define TBX_SERVER_H

#include <stdlib.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/types/wlr_xdg_decoration_v1.h>
#include <wlr/types/wlr_server_decoration.h>

#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

#include "tinybox/tbx_cursor.h"
#include "tinybox/tbx_output.h"
#include "tinybox/tbx_seat.h"
#include "tinybox/tbx_workspace.h"
#include "tinybox/tbx_server.h"
#include "tinybox/xdg_shell.h"
#include "tinybox/style.h"
#include "tinybox/console.h"

struct tbx_server {
  struct wl_display *wl_display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;

  struct wlr_xdg_shell *xdg_shell;
  struct wl_listener new_xdg_surface;
  struct wl_list views;
  struct wl_listener set_title;

  struct wlr_cursor *cursor;
  struct wlr_xcursor_manager *cursor_manager;
  struct wl_listener cursor_motion;
  struct wl_listener cursor_motion_absolute;
  struct wl_listener cursor_button;
  struct wl_listener cursor_axis;
  struct wl_listener cursor_frame;
  struct wl_listener cursor_swipe_begin;
  struct wl_listener cursor_swipe_update;
  struct wl_listener cursor_swipe_end;
  struct tbx_view *grabbed_view;
  double swipe_begin_x, swipe_begin_y;
  int swipe_fingers;
  double grab_x, grab_y;
  struct wlr_box grab_geobox;
  uint32_t resize_edges;
  enum tbx_cursor_mode cursor_mode;

  struct wlr_seat *seat;
  struct wl_listener new_input;
  struct wl_listener request_cursor;
  struct wl_listener request_set_selection;
  struct wl_list keyboards;
  
  struct wlr_server_decoration_manager *server_decoration_manager;
  struct wl_listener server_decoration;
  struct wl_list decorations; // tbx_server_decoration::link

  struct wlr_xdg_decoration_manager_v1 *xdg_decoration_manager;
  struct wl_listener xdg_decoration;
  struct wl_list xdg_decorations; // tbx_xdg_decoration::link

  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;
  struct tbx_output *main_output;

  struct wl_list workspaces;
  struct tbx_workspace *active_workspace;
  int active_workspace_id;

  struct tbx_style style;
  struct tbx_console console;
};

extern struct tbx_server server;
extern void server_print();

bool server_create();
bool server_start();
void server_destroy();

#endif //  TBX_SERVER_H