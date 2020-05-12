#ifndef TINYBOX_DECORATION_H
#define TINYBOX_DECORATION_H

#include "tinybox/server.h"

#include <stdbool.h>

struct tbx_server;
struct wlr_surface;

struct tbx_server_decoration {
  struct wlr_server_decoration *wlr_server_decoration;
  struct wl_list link;

  struct wl_listener destroy;
  struct wl_listener mode;

  struct tbx_server *server;
};

struct tbx_decoration_manager {
  struct wlr_server_decoration_manager *server_decoration_manager;
  struct wl_listener server_decoration;

  struct wl_list decorations;
  struct tbx_server *server;
};

struct tbx_server_decoration *
decoration_from_surface(struct wlr_surface *surface);

bool decoration_setup(struct tbx_server *server);

#endif // TINYBOX_DECORATION_H