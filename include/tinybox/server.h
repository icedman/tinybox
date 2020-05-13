#ifndef TINYBOX_SERVER_H
#define TINYBOX_SERVER_H

#include <stdbool.h>
#include <wayland-server-core.h>

#include "tinybox/cursor.h"
#include "tinybox/seat.h"
#include "tinybox/style.h"
#include "tinybox/console.h"

struct tbx_server_decoration_manager;

struct tbx_server {
  struct wl_display *wl_display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;

  // output
  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;
  struct tbx_output *main_output;

  // shell
  struct tbx_xdg_shell *xdg_shell;

  // views
  struct wl_list views;

  // workspaces

  // decorations
  struct tbx_decoration_manager *decoration_manager;

  // input
  struct tbx_cursor *cursor;
  struct tbx_seat *seat;

  struct tbx_style style;
  struct tbx_console *console;
};

bool tbx_server_setup(struct tbx_server *server);
bool tbx_server_start(struct tbx_server *server);
void tbx_server_terminate(struct tbx_server *server);

#endif // TINYBOX_SERVER_H