#include "tinybox/server.h"
#include "tinybox/cursor.h"

#include <stdlib.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>

static void server_cursor_motion(struct wl_listener *listener, void *data) {
    /* This event is forwarded by the cursor when a pointer emits a _relative_
     * pointer motion event (i.e. a delta) */
    struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_motion);
    // struct tbx_server *server = cursor->server;
    struct wlr_event_pointer_motion *event = data;

    /* The cursor doesn't move unless we tell it to. The cursor automatically
     * handles constraining the motion to the output layout, as well as any
     * special configuration applied for the specific input device which
     * generated the event. You can pass NULL for the device if you want to move
     * the cursor around without any input. */
    wlr_cursor_move(cursor->cursor, event->device,
                    event->delta_x, event->delta_y);

    // process_cursor_motion(server, event->time_msec);
    wlr_xcursor_manager_set_cursor_image(
        cursor->xcursor_manager, "left_ptr", cursor->cursor);
}

static void server_cursor_motion_absolute(struct wl_listener *listener, void *data) {
    struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_motion_absolute);
    // struct tbx_server *server = cursor->server;
  struct wlr_event_pointer_motion_absolute *event = data;
  wlr_cursor_warp_absolute(cursor->cursor, event->device, event->x, event->y);
}

static void server_cursor_button(struct wl_listener *listener, void *data) {
struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_button);
    // struct tbx_server *server = cursor->server;
    if (cursor) {
        // printf("button\n");
    }
}

static void server_cursor_axis(struct wl_listener *listener, void *data) {
struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_axis);
    // struct tbx_server *server = cursor->server;
    if (cursor) {
        // printf("axis\n");
    }
}

static void server_cursor_frame(struct wl_listener *listener, void *data) {
struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_frame);
    // struct tbx_server *server = cursor->server;
    if (cursor) {
        // printf("frame\n");
    }
}

static void server_cursor_swipe_begin(struct wl_listener *listener, void *data) {
struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_swipe_begin);
    // struct tbx_server *server = cursor->server;
    if (cursor) {
        // printf("swipe_begin\n");
    }
}

static void server_cursor_swipe_update(struct wl_listener *listener, void *data) {
struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_swipe_update);
    // struct tbx_server *server = cursor->server;
    if (cursor) {
        // printf("swipe_update\n");
    }
}

static void server_cursor_swipe_end(struct wl_listener *listener, void *data) {
struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_swipe_end);
    // struct tbx_server *server = cursor->server;
    if (cursor) {
        // printf("swipe_end\n");
    }
}

void cursor_attach(struct tbx_server *server, struct wlr_input_device *device)
{
    /* We don't do anything special with pointers. All of our pointer handling
     * is proxied through wlr_cursor. On another compositor, you might take this
     * opportunity to do libinput configuration on the device to set
     * acceleration, etc. */
    wlr_cursor_attach_input_device(server->cursor->cursor, device);
}

bool cursor_setup(struct tbx_server *server) {
    server->cursor = calloc(1, sizeof(struct tbx_cursor));
    server->cursor->server = server;

    server->cursor->cursor = wlr_cursor_create();
    wlr_cursor_attach_output_layout(server->cursor->cursor, server->output_layout);

    server->cursor->xcursor_manager = wlr_xcursor_manager_create(NULL, 24);
    wlr_xcursor_manager_load(server->cursor->xcursor_manager, 1);

    /*
     * wlr_cursor *only* displays an image on screen. It does not move around
     * when the pointer moves. However, we can attach input devices to it, and
     * it will generate aggregate events for all of them. In these events, we
     * can choose how we want to process them, forwarding them to clients and
     * moving the cursor around. More detail on this process is described in my
     * input handling blog post:
     *
     * https://drewdevault.com/2018/07/17/Input-handling-in-wlroots.html
     *
     * And more comments are sprinkled throughout the notify functions above.
     */

    struct tbx_cursor *cursor = server->cursor;
    struct wlr_cursor *wlr_cursor = cursor->cursor;

    cursor->cursor_motion.notify = server_cursor_motion;
    wl_signal_add(&wlr_cursor->events.motion, &cursor->cursor_motion);
    cursor->cursor_motion_absolute.notify = server_cursor_motion_absolute;
    wl_signal_add(&wlr_cursor->events.motion_absolute,
                  &cursor->cursor_motion_absolute);
    cursor->cursor_button.notify = server_cursor_button;
    wl_signal_add(&wlr_cursor->events.button, &cursor->cursor_button);
    cursor->cursor_axis.notify = server_cursor_axis;
    wl_signal_add(&wlr_cursor->events.axis, &cursor->cursor_axis);
    cursor->cursor_frame.notify = server_cursor_frame;
    wl_signal_add(&wlr_cursor->events.frame, &cursor->cursor_frame);

    cursor->cursor_swipe_begin.notify = server_cursor_swipe_begin;
    wl_signal_add(&wlr_cursor->events.swipe_begin, &cursor->cursor_swipe_begin);
    cursor->cursor_swipe_update.notify = server_cursor_swipe_update;
    wl_signal_add(&wlr_cursor->events.swipe_update, &cursor->cursor_swipe_update);
    cursor->cursor_swipe_end.notify = server_cursor_swipe_end;
    wl_signal_add(&wlr_cursor->events.swipe_end, &cursor->cursor_swipe_end);

    return true;
}
