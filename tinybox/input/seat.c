#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_seat.h>

#include "tinybox/server.h"
#include "tinybox/cursor.h"
#include "tinybox/keyboard.h"
#include "tinybox/seat.h"
#include "common/stringop.h"

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
    struct tbx_seat *seat = wl_container_of(listener, seat, new_input);
    struct tbx_server *server = seat->server;

    struct wlr_input_device *device = data;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        keyboard_attach(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        cursor_attach(server, device);
        break;
    case WLR_INPUT_DEVICE_TOUCH:
        break;
    default:
        break;
    }

    /* We need to let the wlr_seat know what our capabilities are, which is
     * communiciated to the client. In TinyWL we always have a cursor, even if
     * there are no pointer devices, so we always include that capability. */
    // uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    // if (!wl_list_empty(&server->keyboards)) {
    //   caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    // }
    // wlr_seat_set_capabilities(server->seat, caps);

    struct tbx_input_device tbx_device;
    tbx_device.identifier = input_device_get_identifier(device);
    tbx_device.wlr_device = device;

    printf("%s\n", tbx_device.identifier);
    // reset_libinput_device(&tbx_device);
}

bool seat_setup(struct tbx_server *server)
{
    server->seat = calloc(1, sizeof(struct tbx_seat));
    server->seat->server = server;

    wl_list_init(&server->seat->input_devices);
    wl_list_init(&server->seat->keyboards);

    server->seat->new_input.notify = server_new_input;
    wl_signal_add(&server->backend->events.new_input, &server->seat->new_input);
    server->seat->seat = wlr_seat_create(server->wl_display, "seat0");

    // server->seat.request_cursor.notify = seat_request_cursor;
    // wl_signal_add(&server.seat->events.request_set_cursor, &server.request_cursor);

    // server->seat.request_set_selection.notify = seat_request_set_selection;
    // wl_signal_add(&server.seat->events.request_set_selection, &server.request_set_selection);

    return true;
}

void seat_destroy(struct tbx_seat *seat)
{}