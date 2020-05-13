#include "tinybox/cursor.h"
#include "tinybox/view.h"

#include <stdlib.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

const char *cursor_images[] = {
    "bottom_left_corner",// HS_EDGE_BOTTOM,
    "bottom_right_corner",// HS_EDGE_BOTTOM,
    "top_side",// HS_EDGE_TOP,
    "bottom_side",// HS_EDGE_BOTTOM,
    "left_side",// HS_EDGE_LEFT,
    "right_side",// HS_EDGE_RIGHT,
    "left_ptr",// HS_TITLEBAR,
    "left_ptr",// HS_HANDLE,
    "left_ptr",// HS_GRIP_LEFT,
    "left_ptr",// HS_GRIP_RIGHT,
    // HS_COUNT
};

static bool begin_interactive_sd(struct tbx_server *server, struct tbx_view *view) {
  /* server side decoration initiated resize or move */

  if (!view) {
    return false;
  }

  struct tbx_cursor *cursor = server->cursor;

  if (view->hotspot_edges != WLR_EDGE_NONE) {
      cursor->mode = TBX_CURSOR_RESIZE;
      cursor->grab_view = view;
      cursor->resize_edges = view->hotspot_edges;
      cursor->grab_x = 0;
      cursor->grab_y = 0;

      // int title_bar_height = 28;
      // int footer_height = server->style.handleWidth + (server->style.borderWidth * 2);
      // if (view->hotspot_edges & WLR_EDGE_TOP) {
      //   cursor->grab_y -= title_bar_height;
      // }
      // if (view->hotspot_edges & WLR_EDGE_BOTTOM) {
      //   cursor->grab_y += footer_height;
      // }

      wlr_xdg_surface_get_geometry(view->xdg_surface, &cursor->grab_box);
      cursor->grab_box.x = view->x;
      cursor->grab_box.y = view->y;
      view->hotspot = -1;
      view->hotspot_edges = WLR_EDGE_NONE;

      // move to active workspace  
      // view->workspace = server->active_workspace;
      // view->workspace_id = server->active_workspace_id;

      return true;
  }

  if (view->hotspot == HS_TITLEBAR) {
      cursor->mode = TBX_CURSOR_MOVE;
      cursor->grab_view = view;
      cursor->grab_x = cursor->cursor->x - view->x;
      cursor->grab_y = cursor->cursor->y - view->y;
      wlr_xdg_surface_get_geometry(view->xdg_surface, &cursor->grab_box);
      cursor->grab_box.x = view->x;
      cursor->grab_box.y = view->y;
      view->hotspot = -1;
      view->hotspot_edges = WLR_EDGE_NONE;

      // // move to active workspace  
      // view->workspace = server->active_workspace;
      // view->workspace_id = server->active_workspace_id;

      return true;
  }

  return false;
}

static void process_cursor_move(struct tbx_server *server, uint32_t time) {
  /* Move the grabbed view to the new position. */
  struct tbx_cursor *cursor = server->cursor;
  cursor->grab_view->x = cursor->cursor->x - cursor->grab_x;
  cursor->grab_view->y = cursor->cursor->y - cursor->grab_y;
}

static void process_cursor_resize(struct tbx_server *server, uint32_t time) {
  /*
   * Resizing the grabbed view can be a little bit complicated, because we
   * could be resizing from any corner or edge. This not only resizes the view
   * on one or two axes, but can also move the view if you resize from the top
   * or left edges (or top-left corner).
   *
   * Note that I took some shortcuts here. In a more fleshed-out compositor,
   * you'd wait for the client to prepare a buffer at the new size, then
   * commit any movement that was prepared.
   */

  struct tbx_cursor *cursor = server->cursor;
  struct tbx_view *view = cursor->grab_view;
  double border_x = cursor->cursor->x - cursor->grab_x;
  double border_y = cursor->cursor->y - cursor->grab_y;
  int new_left = cursor->grab_box.x;
  int new_right = cursor->grab_box.x + cursor->grab_box.width;
  int new_top = cursor->grab_box.y;
  int new_bottom = cursor->grab_box.y + cursor->grab_box.height;

  if (cursor->resize_edges & WLR_EDGE_TOP) {
    new_top = border_y;
    if (new_top >= new_bottom) {
      new_top = new_bottom - 1;
    }
  } else if (cursor->resize_edges & WLR_EDGE_BOTTOM) {
    new_bottom = border_y;
    if (new_bottom <= new_top) {
      new_bottom = new_top + 1;
    }
  }
  if (cursor->resize_edges & WLR_EDGE_LEFT) {
    new_left = border_x;
    if (new_left >= new_right) {
      new_left = new_right - 1;
    }
  } else if (cursor->resize_edges & WLR_EDGE_RIGHT) {
    new_right = border_x;
    if (new_right <= new_left) {
      new_right = new_left + 1;
    }
  }

  struct wlr_box geo_box;
  wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);
  view->x = new_left - geo_box.x;
  view->y = new_top - geo_box.y;

  int new_width = new_right - new_left;
  int new_height = new_bottom - new_top;
  wlr_xdg_toplevel_set_size(view->xdg_surface, new_width, new_height);
}

static void process_cursor_motion(struct tbx_server *server, uint32_t time) {
  struct tbx_cursor *cursor = server->cursor;

  /* If the mode is non-passthrough, delegate to those functions. */
  if (cursor->mode == TBX_CURSOR_MOVE) {
    process_cursor_move(server, time);
    return;
  } else if (cursor->mode == TBX_CURSOR_RESIZE) {
    process_cursor_resize(server, time);
    return;
  }

  /* Otherwise, find the view under the pointer and send the event along. */
  double sx, sy;
  struct wlr_seat *seat = server->seat->seat;
  struct wlr_surface *surface = NULL;
  struct tbx_view *view = desktop_view_at(
      server, cursor->cursor->x, cursor->cursor->y, &surface, &sx, &sy);

  if (!view) {
    /* If there's no view under the cursor, set the cursor image to a
     * default. This is what makes the cursor image appear when you move it
     * around the screen, not over any views. */
    wlr_xcursor_manager_set_cursor_image(cursor->xcursor_manager, "left_ptr",
                                         cursor->cursor);
  }

  if (view && view->hotspot >= 0 && view->hotspot < HS_COUNT) { 
    // view->hotspot_edges != WLR_EDGE_NONE) {
    wlr_xcursor_manager_set_cursor_image(
        server->cursor->xcursor_manager, cursor_images[view->hotspot], cursor->cursor);
  }

  if (surface) {
    bool focus_changed = seat->pointer_state.focused_surface != surface;
    /*
     * "Enter" the surface if necessary. This lets the client know that the
     * cursor has entered one of its surfaces.
     *
     * Note that this gives the surface "pointer focus", which is distinct
     * from keyboard focus. You get pointer focus by moving the pointer over
     * a window.
     */
    wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
    if (!focus_changed) {
      /* The enter event contains coordinates, so we only need to notify
       * on motion if the focus did not change. */
      wlr_seat_pointer_notify_motion(seat, time, sx, sy);
    }
  } else {
    /* Clear pointer focus so future button events and such are not sent to
     * the last client to have the cursor over it. */
    wlr_seat_pointer_clear_focus(seat);
  }
}

static void server_cursor_motion(struct wl_listener *listener, void *data) {
  /* This event is forwarded by the cursor when a pointer emits a _relative_
   * pointer motion event (i.e. a delta) */
  struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_motion);
  struct wlr_event_pointer_motion *event = data;

  /* The cursor doesn't move unless we tell it to. The cursor automatically
   * handles constraining the motion to the output layout, as well as any
   * special configuration applied for the specific input device which
   * generated the event. You can pass NULL for the device if you want to move
   * the cursor around without any input. */
  wlr_cursor_move(cursor->cursor, event->device, event->delta_x,
                  event->delta_y);

  process_cursor_motion(cursor->server, event->time_msec);
}

static void server_cursor_motion_absolute(struct wl_listener *listener,
                                          void *data) {
  struct tbx_cursor *cursor =
      wl_container_of(listener, cursor, cursor_motion_absolute);
  struct wlr_event_pointer_motion_absolute *event = data;
  wlr_cursor_warp_absolute(cursor->cursor, event->device, event->x, event->y);

  process_cursor_motion(cursor->server, event->time_msec);
}

static void server_cursor_button(struct wl_listener *listener, void *data) {
  struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_button);
  struct tbx_server *server = cursor->server;
  struct wlr_event_pointer_button *event = data;
  /* Notify the client with pointer focus that a button press has occurred */
  wlr_seat_pointer_notify_button(server->seat->seat, event->time_msec,
                                 event->button, event->state);
  double sx, sy;
  struct wlr_surface *surface;

  struct tbx_view *view = desktop_view_at(
      cursor->server, cursor->cursor->x, cursor->cursor->y, &surface, &sx, &sy);

  if (event->state == WLR_BUTTON_RELEASED) {
    /* If you released any buttons, we exit interactive move/resize mode. */
    cursor->mode = TBX_CURSOR_PASSTHROUGH;
    cursor->resize_edges = WLR_EDGE_NONE;

    wlr_xcursor_manager_set_cursor_image(server->cursor->xcursor_manager,
                                         "left_ptr", server->cursor->cursor);

  } else {
    /* Focus that client if the button was _pressed_ */
    focus_view(view, surface);
    begin_interactive_sd(server, view);
  }
}

static void server_cursor_axis(struct wl_listener *listener, void *data) {
  /* This event is forwarded by the cursor when a pointer emits an axis event,
   * for example when you move the scroll wheel. */

  struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_axis);
  struct tbx_server *server = cursor->server;

  struct wlr_event_pointer_axis *event = data;
  /* Notify the client with pointer focus of the axis event. */
  wlr_seat_pointer_notify_axis(server->seat->seat, event->time_msec,
                               event->orientation, event->delta,
                               event->delta_discrete, event->source);
}

static void server_cursor_frame(struct wl_listener *listener, void *data) {
  /* This event is forwarded by the cursor when a pointer emits an frame
   * event. Frame events are sent after regular pointer events to group
   * multiple events together. For instance, two axis events may happen at the
   * same time, in which case a frame event won't be sent in between. */
  struct tbx_cursor *cursor = wl_container_of(listener, cursor, cursor_frame);
  struct tbx_server *server = cursor->server;
  wlr_seat_pointer_notify_frame(server->seat->seat);
}

static void server_cursor_swipe_begin(struct wl_listener *listener,
                                      void *data) {
  struct tbx_cursor *cursor =
      wl_container_of(listener, cursor, cursor_swipe_begin);
  // struct tbx_server *server = cursor->server;
  if (cursor) {
    // printf("swipe_begin\n");
  }
}

static void server_cursor_swipe_update(struct wl_listener *listener,
                                       void *data) {
  struct tbx_cursor *cursor =
      wl_container_of(listener, cursor, cursor_swipe_update);
  // struct tbx_server *server = cursor->server;
  if (cursor) {
    // printf("swipe_update\n");
  }
}

static void server_cursor_swipe_end(struct wl_listener *listener, void *data) {
  struct tbx_cursor *cursor =
      wl_container_of(listener, cursor, cursor_swipe_end);
  // struct tbx_server *server = cursor->server;
  if (cursor) {
    // printf("swipe_end\n");
  }
}

void cursor_attach(struct tbx_server *server, struct wlr_input_device *device) {
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
  wlr_cursor_attach_output_layout(server->cursor->cursor,
                                  server->output_layout);

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
