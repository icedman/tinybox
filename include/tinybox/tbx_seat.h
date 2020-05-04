#ifndef TBX_SEAT_H
#define TBX_SEAT_H

#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_input_device.h>

struct tbx_keyboard {
  struct wl_list link;
  struct tbx_server *server;
  struct wlr_input_device *device;

  struct wl_listener modifiers;
  struct wl_listener key;
};

void init_seat();

#endif //  TBX_SEAT_H