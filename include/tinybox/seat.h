#ifndef TINYBOX_SEAT_H
#define TINYBOX_SEAT_H

struct tbx_input_device {
    struct wl_list link;

    char *identifier;
    struct wlr_input_device *wlr_device;
    struct wl_listener device_destroy;
};

struct tbx_seat {
  struct wlr_seat *seat;
  struct wl_listener new_input;
  struct wl_list input_devices;
  struct wl_list keyboards;
  struct tbx_server *server;
};

bool seat_setup(struct tbx_server *server);
void seat_destroy(struct tbx_seat *seat);

#endif // TINYBOX_SEAT_H