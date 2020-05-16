#ifndef TINYBOX_SERVER_H
#define TINYBOX_SERVER_H

#include <stdbool.h>
#include <wayland-server-core.h>

#include "tinybox/config.h"
#include "tinybox/console.h"
#include "tinybox/cursor.h"
#include "tinybox/seat.h"
#include "tinybox/style.h"

struct tbx_server_decoration_manager;
struct tbx_command;

struct tbx_server {
  bool started;

  struct wl_display *wl_display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;
  struct wlr_compositor *compositor;
  struct wl_event_loop *wl_event_loop;

  // output
  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener output_destroy;
  struct wl_listener new_output;
  struct tbx_output *main_output;

  // shell
  struct tbx_xdg_shell *xdg_shell;
  struct tbx_xwayland_shell *xwayland_shell;

  // decorations
  struct tbx_decoration_manager *decoration_manager;

  // views
  struct wl_list views;

  // workspaces
  struct wl_list workspaces;
  int workspace;
  bool ws_animate;
  double ws_anim_x;
  double ws_anim_y;

  // input
  struct tbx_cursor *cursor;
  struct tbx_seat *seat;
  struct tbx_command *command;

  struct tbx_config config;
  struct tbx_style style;
  struct tbx_console *console;
};

bool tbx_server_setup(struct tbx_server *server);
bool tbx_server_start(struct tbx_server *server);
void tbx_server_terminate(struct tbx_server *server);

#endif // TINYBOX_SERVER_H