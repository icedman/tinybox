#ifndef TINYBOX_OUTPUT_H
#define TINYBOX_OUTPUT_H

#include <wayland-server-core.h>
#include <stdbool.h>

struct tbx_output {
  struct wl_list link;

  struct tbx_server *server;

  struct wlr_output *wlr_output;
  struct wl_listener frame;
};

bool output_setup(struct tbx_server *server);

#endif // TINYBOX_OUTPUT_H