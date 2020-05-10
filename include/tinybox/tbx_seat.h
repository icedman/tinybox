#ifndef TBX_SEAT_H
#define TBX_SEAT_H

#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_input_device.h>

struct tbx_input_device {
    char *identifier;
    struct wlr_input_device *wlr_device;
    struct wl_list link;
    struct wl_listener device_destroy;
    bool is_virtual;
};

struct tbx_keyboard {
  struct wl_list link;
  struct tbx_server *server;
  struct wlr_input_device *device;

  struct wl_listener modifiers;
  struct wl_listener key;
};

void seat_init();

void reset_libinput_device(struct tbx_input_device *device);

#endif //  TBX_SEAT_H