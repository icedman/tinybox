#define _POSIX_C_SOURCE 200809L
#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "tinybox/tbx_server.h"
#include "tinybox/tbx_seat.h"
#include "tinybox/tbx_cursor.h"
#include "tinybox/tbx_keyboard.h"
#include "tinybox/stringop.h"

char *input_device_get_identifier(struct wlr_input_device *device) {
  int vendor = device->vendor;
  int product = device->product;
  char *name = strdup(device->name ? device->name : "");
  strip_whitespace(name);

  char *p = name;
  for (; *p; ++p) {
    if (*p == ' ') {
      *p = '_';
    }
  }

  const char *fmt = "%d:%d:%s";
  int len = snprintf(NULL, 0, fmt, vendor, product, name) + 1;
  char *identifier = malloc(len);
  if (!identifier) {
    // sway_log(SWAY_ERROR, "Unable to allocate unique input device name");
    return NULL;
  }

  snprintf(identifier, len, fmt, vendor, product, name);
  free(name);
  return identifier;
}

static void server_new_input(struct wl_listener *listener, void *data) {
  /* This event is raised by the backend when a new input device becomes
   * available. */
  struct tbx_server *server =
    wl_container_of(listener, server, new_input);
  struct wlr_input_device *device = data;
  switch (device->type) {
  case WLR_INPUT_DEVICE_KEYBOARD:
    keyboard_attach(server, device);
    break;
  case WLR_INPUT_DEVICE_POINTER:
    cursor_attach(server, device);
    break;
  default:
    break;
  }
  /* We need to let the wlr_seat know what our capabilities are, which is
   * communiciated to the client. In TinyWL we always have a cursor, even if
   * there are no pointer devices, so we always include that capability. */
  uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
  if (!wl_list_empty(&server->keyboards)) {
    caps |= WL_SEAT_CAPABILITY_KEYBOARD;
  }
  wlr_seat_set_capabilities(server->seat, caps);

  struct tbx_input_device tbx_device;
  tbx_device.identifier = input_device_get_identifier(device);
  tbx_device.wlr_device = device;
  reset_libinput_device(&tbx_device);  
}

static void seat_request_cursor(struct wl_listener *listener, void *data) {
  struct tbx_server *server = wl_container_of(
      listener, server, request_cursor);
  /* This event is rasied by the seat when a client provides a cursor image */
  struct wlr_seat_pointer_request_set_cursor_event *event = data;
  struct wlr_seat_client *focused_client =
    server->seat->pointer_state.focused_client;
  /* This can be sent by any client, so we check to make sure this one is
   * actually has pointer focus first. */
  if (focused_client == event->seat_client) {
    /* Once we've vetted the client, we can tell the cursor to use the
     * provided surface as the cursor image. It will set the hardware cursor
     * on the output that it's currently on and continue to do so as the
     * cursor moves between outputs. */
    wlr_cursor_set_surface(server->cursor, event->surface,
        event->hotspot_x, event->hotspot_y);
  }
}

static void seat_request_set_selection(struct wl_listener *listener, void *data) {
  /* This event is raised by the seat when a client wants to set the selection,
   * usually when the user copies something. wlroots allows compositors to
   * ignore such requests if they so choose, but in tinywl we always honor
   */
  struct tbx_server *server = wl_container_of(
      listener, server, request_set_selection);
  struct wlr_seat_request_set_selection_event *event = data;
  wlr_seat_set_selection(server->seat, event->source, event->serial);
}

void seat_init() {
  /*
   * Configures a seat, which is a single "seat" at which a user sits and
   * operates the computer. This conceptually includes up to one keyboard,
   * pointer, touch, and drawing tablet device. We also rig up a listener to
   * let us know when new input devices are available on the backend.
   */
  wl_list_init(&server.keyboards);
  server.new_input.notify = server_new_input;
  wl_signal_add(&server.backend->events.new_input, &server.new_input);
  server.seat = wlr_seat_create(server.wl_display, "seat0");
  server.request_cursor.notify = seat_request_cursor;
  wl_signal_add(&server.seat->events.request_set_cursor,
      &server.request_cursor);
  server.request_set_selection.notify = seat_request_set_selection;
  wl_signal_add(&server.seat->events.request_set_selection,
      &server.request_set_selection);
  
}