#ifndef TINYBOX_SEAT_H
#define TINYBOX_SEAT_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_server;

struct tbx_keyboard {
  struct wl_list link;
  struct tbx_server *server;
  struct wlr_keyboard *wlr_keyboard;

  struct wl_listener modifiers;
  struct wl_listener key;
  struct wl_listener destroy;
};

bool
tbx_seat_setup(struct tbx_server *server);

#endif // TINYBOX_SEAT_H