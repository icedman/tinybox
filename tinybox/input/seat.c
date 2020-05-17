#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_seat.h>

#include <wlr/util/log.h>
#include <xkbcommon/xkbcommon.h>

#include "common/stringop.h"
#include "tinybox/cursor.h"
#include "tinybox/keyboard.h"
#include "tinybox/libinput.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

char* input_device_get_identifier(struct wlr_input_device* device)
{
    int vendor = device->vendor;
    int product = device->product;
    char* name = strdup(device->name ? device->name : "");
    strip_whitespace(name);

    char* p = name;
    for (; *p; ++p) {
        if (*p == ' ') {
            *p = '_';
        }
    }

    const char* fmt = "%d:%d:%s";
    int len = snprintf(NULL, 0, fmt, vendor, product, name) + 1;
    char* identifier = malloc(len);
    if (!identifier) {
        // sway_log(SWAY_ERROR, "Unable to allocate unique input device name");
        return NULL;
    }

    snprintf(identifier, len, fmt, vendor, product, name);
    free(name);
    return identifier;
}

static void server_new_input(struct wl_listener* listener, void* data)
{
    /* This event is raised by the backend when a new input device becomes
   * available. */
    struct tbx_seat* seat = wl_container_of(listener, seat, new_input);
    struct tbx_server* server = seat->server;

    struct wlr_input_device* device = data;
    bool add_to_devices = false;

    switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
        add_to_devices = true;
        keyboard_attach(server, device);
        break;
    case WLR_INPUT_DEVICE_POINTER:
        add_to_devices = true;
        cursor_attach(server, device);
        break;
    case WLR_INPUT_DEVICE_TOUCH:
        add_to_devices = true;
        break;
    default:
        break;
    }

    /* We need to let the wlr_seat know what our capabilities are, which is
   * communiciated to the client. In TinyWL we always have a cursor, even if
   * there are no pointer devices, so we always include that capability. */
    uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
    if (!wl_list_empty(&seat->keyboards)) {
        caps |= WL_SEAT_CAPABILITY_KEYBOARD;
    }
    wlr_seat_set_capabilities(server->seat->seat, caps);

    if (add_to_devices) {
        struct tbx_input_device* input_device = calloc(1, sizeof(struct tbx_input_device));
        input_device->identifier = input_device_get_identifier(device);
        input_device->wlr_device = device;
        input_device->server = server;
        wl_list_insert(&seat->input_devices, &input_device->link);

        reset_libinput_device(input_device);
        configure_libinput_device(input_device);
    }
}

static void seat_request_cursor(struct wl_listener* listener, void* data)
{
    struct tbx_seat* seat = wl_container_of(listener, seat, request_cursor);
    /* This event is rasied by the seat when a client provides a cursor image */
    struct wlr_seat_pointer_request_set_cursor_event* event = data;
    struct wlr_seat_client* focused_client = seat->seat->pointer_state.focused_client;
    /* This can be sent by any client, so we check to make sure this one is
   * actually has pointer focus first. */
    if (focused_client == event->seat_client) {
        /* Once we've vetted the client, we can tell the cursor to use the
     * provided surface as the cursor image. It will set the hardware cursor
     * on the output that it's currently on and continue to do so as the
     * cursor moves between outputs. */

        wlr_cursor_set_surface(seat->server->cursor->cursor, event->surface,
            event->hotspot_x, event->hotspot_y);
    }
}

static void seat_request_set_selection(struct wl_listener* listener,
    void* data)
{
    /* This event is raised by the seat when a client wants to set the selection,
   * usually when the user copies something. wlroots allows compositors to
   * ignore such requests if they so choose, but in tinywl we always honor
   */
    struct tbx_seat* seat = wl_container_of(listener, seat, request_set_selection);
    struct wlr_seat_request_set_selection_event* event = data;
    wlr_seat_set_selection(seat->seat, event->source, event->serial);
}

bool seat_setup(struct tbx_server* server)
{
    server->seat = calloc(1, sizeof(struct tbx_seat));
    server->seat->server = server;
    server->seat->keys_pressed = calloc(1, sizeof(struct tbx_keys_pressed));
    keys_clear(server->seat->keys_pressed);

    wl_list_init(&server->seat->input_devices);
    wl_list_init(&server->seat->keyboards);

    server->seat->new_input.notify = server_new_input;
    wl_signal_add(&server->backend->events.new_input, &server->seat->new_input);
    server->seat->seat = wlr_seat_create(server->wl_display, "seat0");

    server->seat->request_cursor.notify = seat_request_cursor;
    wl_signal_add(&server->seat->seat->events.request_set_cursor,
        &server->seat->request_cursor);

    server->seat->request_set_selection.notify = seat_request_set_selection;
    wl_signal_add(&server->seat->seat->events.request_set_selection,
        &server->seat->request_set_selection);

    return true;
}

void seat_destroy(struct tbx_seat* seat) {}