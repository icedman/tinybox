#include "tinybox/tbx_server.h"
#include "tinybox/tbx_cursor.h"

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

static void process_cursor_move(struct tbx_server *server, uint32_t time) {
  /* Move the grabbed view to the new position. */
  server->grabbed_view->x = server->cursor->x - server->grab_x;
  server->grabbed_view->y = server->cursor->y - server->grab_y;
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
  struct tbx_view *view = server->grabbed_view;
  double border_x = server->cursor->x - server->grab_x;
  double border_y = server->cursor->y - server->grab_y;
  int new_left = server->grab_geobox.x;
  int new_right = server->grab_geobox.x + server->grab_geobox.width;
  int new_top = server->grab_geobox.y;
  int new_bottom = server->grab_geobox.y + server->grab_geobox.height; 

  if (server->resize_edges & WLR_EDGE_TOP) {
    new_top = border_y;
    if (new_top >= new_bottom) {
      new_top = new_bottom - 1;
    }
  } else if (server->resize_edges & WLR_EDGE_BOTTOM) {
    new_bottom = border_y;
    if (new_bottom <= new_top) {
      new_bottom = new_top + 1;
    }
  }
  if (server->resize_edges & WLR_EDGE_LEFT) {
    new_left = border_x;
    if (new_left >= new_right) {
      new_left = new_right - 1;
    }
  } else if (server->resize_edges & WLR_EDGE_RIGHT) {
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

  view->pending_box.x = view->x;
  view->pending_box.y = view->y;
  view->pending_box.width = new_width;
  view->pending_box.height = new_height;
}

static void process_cursor_motion(struct tbx_server *server, uint32_t time) {
  /* If the mode is non-passthrough, delegate to those functions. */
  if (server->cursor_mode == TBX_CURSOR_MOVE) {
    process_cursor_move(server, time);

        wlr_xcursor_manager_set_cursor_image(
          server->cursor_mgr, "grabbing", server->cursor);

    return;
  } else if (server->cursor_mode == TBX_CURSOR_RESIZE) {
    process_cursor_resize(server, time);
    return;
  }

  /* Otherwise, find the view under the pointer and send the event along. */
  double sx, sy;
  struct wlr_seat *seat = server->seat;
  struct wlr_surface *surface = NULL;
  struct tbx_view *view = desktop_view_at(server,
      server->cursor->x, server->cursor->y, &surface, &sx, &sy);
  if (!view) {
    /* If there's no view under the cursor, set the cursor image to a
     * default. This is what makes the cursor image appear when you move it
     * around the screen, not over any views. */
    wlr_xcursor_manager_set_cursor_image(
        server->cursor_mgr, "left_ptr", server->cursor);
  }

  if (view && view->hotspot != -1 && view->hotspot < HS_COUNT) { 
    // view->hotspot_edges != WLR_EDGE_NONE) {
    wlr_xcursor_manager_set_cursor_image(
        server->cursor_mgr, cursor_images[view->hotspot], server->cursor);
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
  struct tbx_server *server =
    wl_container_of(listener, server, cursor_motion);
  struct wlr_event_pointer_motion *event = data;
  /* The cursor doesn't move unless we tell it to. The cursor automatically
   * handles constraining the motion to the output layout, as well as any
   * special configuration applied for the specific input device which
   * generated the event. You can pass NULL for the device if you want to move
   * the cursor around without any input. */
  wlr_cursor_move(server->cursor, event->device,
      event->delta_x, event->delta_y);
  process_cursor_motion(server, event->time_msec);
}

static void server_cursor_motion_absolute(
    struct wl_listener *listener, void *data) {
  /* This event is forwarded by the cursor when a pointer emits an _absolute_
   * motion event, from 0..1 on each axis. This happens, for example, when
   * wlroots is running under a Wayland window rather than KMS+DRM, and you
   * move the mouse over the window. You could enter the window from any edge,
   * so we have to warp the mouse there. There is also some hardware which
   * emits these events. */
  struct tbx_server *server =
    wl_container_of(listener, server, cursor_motion_absolute);
  struct wlr_event_pointer_motion_absolute *event = data;
  wlr_cursor_warp_absolute(server->cursor, event->device, event->x, event->y);
  process_cursor_motion(server, event->time_msec);
}

static bool begin_resize_or_drag(struct tbx_server *server, struct tbx_view *view) {
  if (!view) {
    return false;
  }
  if (view->hotspot_edges != WLR_EDGE_NONE) {
      int title_bar_height = 28;
      int footer_height = server->style.handleWidth + (server->style.borderWidth * 2);
      server->cursor_mode = TBX_CURSOR_RESIZE;
      server->grabbed_view = view;
      server->resize_edges = view->hotspot_edges;
      server->grab_x = 0;
      server->grab_y = 0;

      if (view->hotspot_edges & WLR_EDGE_TOP) {
        server->grab_y -= title_bar_height;
      }
      if (view->hotspot_edges & WLR_EDGE_BOTTOM) {
        server->grab_y += footer_height;
      }

      wlr_xdg_surface_get_geometry(view->xdg_surface, &server->grab_geobox);
      server->grab_geobox.x = view->x;
      server->grab_geobox.y = view->y;
      view->hotspot = -1;
      view->hotspot_edges = WLR_EDGE_NONE;
      return true;
  }

  if (view->hotspot == HS_TITLEBAR) {
      server->cursor_mode = TBX_CURSOR_MOVE;
      server->grabbed_view = view;
      server->grab_x = server->cursor->x - view->x;
      server->grab_y = server->cursor->y - view->y;
      wlr_xdg_surface_get_geometry(view->xdg_surface, &server->grab_geobox);
      server->grab_geobox.x = view->x;
      server->grab_geobox.y = view->y;
      view->hotspot = -1;
      view->hotspot_edges = WLR_EDGE_NONE;
      return true;
  }

  return false;
}

static void server_cursor_button(struct wl_listener *listener, void *data) {
  /* This event is forwarded by the cursor when a pointer emits a button
   * event. */
  struct tbx_server *server =
    wl_container_of(listener, server, cursor_button);
  struct wlr_event_pointer_button *event = data;
  /* Notify the client with pointer focus that a button press has occurred */
  wlr_seat_pointer_notify_button(server->seat,
      event->time_msec, event->button, event->state);
  double sx, sy;
  struct wlr_surface *surface;
  struct tbx_view *view = desktop_view_at(server,
      server->cursor->x, server->cursor->y, &surface, &sx, &sy);
  if (event->state == WLR_BUTTON_RELEASED) {
    /* If you released any buttons, we exit interactive move/resize mode. */
    server->cursor_mode = TBX_CURSOR_PASSTHROUGH;
    server->resize_edges = WLR_EDGE_NONE;
      wlr_xcursor_manager_set_cursor_image(
        server->cursor_mgr, "left_ptr", server->cursor);
    
  } else {
    /* Focus that client if the button was _pressed_ */
    focus_view(view, surface);
    begin_resize_or_drag(server, view);
  }
}

static void server_cursor_axis(struct wl_listener *listener, void *data) {
  /* This event is forwarded by the cursor when a pointer emits an axis event,
   * for example when you move the scroll wheel. */
  struct tbx_server *server =
    wl_container_of(listener, server, cursor_axis);
  struct wlr_event_pointer_axis *event = data;
  /* Notify the client with pointer focus of the axis event. */
  wlr_seat_pointer_notify_axis(server->seat,
      event->time_msec, event->orientation, event->delta,
      event->delta_discrete, event->source);
}

static void server_cursor_frame(struct wl_listener *listener, void *data) {
  /* This event is forwarded by the cursor when a pointer emits an frame
   * event. Frame events are sent after regular pointer events to group
   * multiple events together. For instance, two axis events may happen at the
   * same time, in which case a frame event won't be sent in between. */
  struct tbx_server *server =
    wl_container_of(listener, server, cursor_frame);
  /* Notify the client with pointer focus of the frame event. */
  wlr_seat_pointer_notify_frame(server->seat);
}

static void server_cursor_swipe_begin(struct wl_listener *listener, void *data) {
  struct tbx_server *server =
    wl_container_of(listener, server, cursor_swipe_begin);

  struct wlr_event_pointer_swipe_begin *event = data;

    double sx, sy;
    struct wlr_surface *surface;
    struct tbx_view *view = desktop_view_at(server,
        server->cursor->x, server->cursor->y, &surface, &sx, &sy);

  if (event->fingers == 3) {
    focus_view(view, surface);

    if (!begin_resize_or_drag(server, view)) {
      server->cursor_mode = TBX_CURSOR_PASSTHROUGH;
      server->resize_edges = WLR_EDGE_NONE;
        wlr_xcursor_manager_set_cursor_image(
          server->cursor_mgr, "left_ptr", server->cursor);

        wlr_seat_pointer_notify_button(server->seat,
          event->time_msec, 0, WLR_BUTTON_PRESSED);
    }
  }
}

static void server_cursor_swipe_update(struct wl_listener *listener, void *data) {
  struct tbx_server *server =
    wl_container_of(listener, server, cursor_swipe_update);

  struct wlr_event_pointer_swipe_update *event = data;

  wlr_cursor_move(server->cursor, event->device,
      event->dx, event->dy);

  process_cursor_motion(server, event->time_msec);
}

static void server_cursor_swipe_end(struct wl_listener *listener, void *data) {
  struct tbx_server *server =
    wl_container_of(listener, server, cursor_swipe_end);

  // if (server) {
    server->cursor_mode = TBX_CURSOR_PASSTHROUGH;
    server->resize_edges = WLR_EDGE_NONE;
    wlr_xcursor_manager_set_cursor_image(
        server->cursor_mgr, "left_ptr", server->cursor);
  // }
}

void cursor_init()
{
  /*
   * Creates a cursor, which is a wlroots utility for tracking the cursor
   * image shown on screen.
   */
  server.cursor = wlr_cursor_create();
  wlr_cursor_attach_output_layout(server.cursor, server.output_layout);

  /* Creates an xcursor manager, another wlroots utility which loads up
   * Xcursor themes to source cursor images from and makes sure that cursor
   * images are available at all scale factors on the screen (necessary for
   * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
  server.cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
  wlr_xcursor_manager_load(server.cursor_mgr, 1);

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
  server.cursor_motion.notify = server_cursor_motion;
  wl_signal_add(&server.cursor->events.motion, &server.cursor_motion);
  server.cursor_motion_absolute.notify = server_cursor_motion_absolute;
  wl_signal_add(&server.cursor->events.motion_absolute,
      &server.cursor_motion_absolute);
  server.cursor_button.notify = server_cursor_button;
  wl_signal_add(&server.cursor->events.button, &server.cursor_button);
  server.cursor_axis.notify = server_cursor_axis;
  wl_signal_add(&server.cursor->events.axis, &server.cursor_axis);
  server.cursor_frame.notify = server_cursor_frame;
  wl_signal_add(&server.cursor->events.frame, &server.cursor_frame);

  server.cursor_swipe_begin.notify = server_cursor_swipe_begin;
  wl_signal_add(&server.cursor->events.swipe_begin, &server.cursor_swipe_begin);
  server.cursor_swipe_update.notify = server_cursor_swipe_update;
  wl_signal_add(&server.cursor->events.swipe_update, &server.cursor_swipe_update);
  server.cursor_swipe_end.notify = server_cursor_swipe_end;
  wl_signal_add(&server.cursor->events.swipe_end, &server.cursor_swipe_end);
}

void cursor_attach(struct tbx_server *server,
    struct wlr_input_device *device) {
  /* We don't do anything special with pointers. All of our pointer handling
   * is proxied through wlr_cursor. On another compositor, you might take this
   * opportunity to do libinput configuration on the device to set
   * acceleration, etc. */
  wlr_cursor_attach_input_device(server->cursor, device);
}
