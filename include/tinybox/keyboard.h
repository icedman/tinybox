#ifndef TINYBOX_KEYBOARD_H
#define TINYBOX_KEYBOARD_H

#include "tinybox/server.h"

struct tbx_server;
struct wlr_input_device;

struct tbx_keyboard {
  struct wl_list link;
  struct wlr_input_device *device;

  struct wl_listener modifiers;
  struct wl_listener key;

  struct tbx_server *server;
};

void keyboard_attach(struct tbx_server *server,
                     struct wlr_input_device *device);

#endif // TINYBOX_KEYBOARD_H