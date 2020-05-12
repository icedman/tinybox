#ifndef TINYBOX_SERVER_H
#define TINYBOX_SERVER_H

#include <wayland-server-core.h>
#include <stdbool.h>

#include "tinybox/cursor.h"
#include "tinybox/seat.h"

struct tbx_server{
  struct wl_display *wl_display;
  struct wlr_backend *backend;
  struct wlr_renderer *renderer;

  // output
  struct wlr_output_layout *output_layout;
  struct wl_list outputs;
  struct wl_listener new_output;
  struct tbx_output *main_output;

  // input
  struct tbx_cursor *cursor;
  struct tbx_seat *seat;
};

bool tbx_server_setup(struct tbx_server *server);
bool tbx_server_run(struct tbx_server *server);
void tbx_server_terminate(struct tbx_server *server);

#endif // TINYBOX_SERVER_H