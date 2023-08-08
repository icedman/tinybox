#include <tinybox/server.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include <wlr/types/wlr_xdg_shell.h>

void
reset_cursor_mode(struct tbx_server *server)
{
  /* Reset the cursor mode to passthrough. */
  server->cursor_mode = TBX_CURSOR_PASSTHROUGH;
  server->grabbed_view = NULL;
}

static void
process_cursor_move(struct tbx_server *server, uint32_t time)
{
  /* Move the grabbed view to the new position. */
  struct tbx_view *view = server->grabbed_view;
  wlr_scene_node_set_position(&view->scene_tree->node,
      server->cursor->x - server->grab_x,
      server->cursor->y - server->grab_y);
}

static void
process_cursor_resize(struct tbx_server *server, uint32_t time)
{
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
  if (view->xdg_toplevel) {
    // view->interface->get_geometry(view, &geo_box);
    wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);
    wlr_scene_node_set_position(
        &view->scene_tree->node, new_left - geo_box.x, new_top - geo_box.y);

    int new_width = new_right - new_left;
    int new_height = new_bottom - new_top;
    wlr_xdg_toplevel_set_size(view->xdg_toplevel, new_width, new_height);
  }
}

static void
process_cursor_motion(struct tbx_server *server, uint32_t time)
{
  /* If the mode is non-passthrough, delegate to those functions. */
  if (server->cursor_mode == TBX_CURSOR_MOVE) {
    process_cursor_move(server, time);
    return;
  } else if (server->cursor_mode == TBX_CURSOR_RESIZE) {
    process_cursor_resize(server, time);
    return;
  }

  /* Otherwise, find the view under the pointer and send the event along. */
  double sx, sy;
  struct wlr_seat *seat = server->seat;
  struct wlr_surface *surface = NULL;
  struct tbx_view *view = desktop_view_at(
      server, server->cursor->x, server->cursor->y, &surface, &sx, &sy);
  if (!view) {
    /* If there's no view under the cursor, set the cursor image to a
     * default. This is what makes the cursor image appear when you move it
     * around the screen, not over any views. */
    wlr_xcursor_manager_set_cursor_image(
        server->cursor_mgr, "default", server->cursor);
  }
  if (surface) {
    /*
     * Send pointer enter and motion events.
     *
     * The enter event gives the surface "pointer focus", which is distinct
     * from keyboard focus. You get pointer focus by moving the pointer over
     * a window.
     *
     * Note that wlroots will avoid sending duplicate enter/motion events if
     * the surface has already has pointer focus or if the client is already
     * aware of the coordinates passed.
     */
    wlr_seat_pointer_notify_enter(seat, surface, sx, sy);
    wlr_seat_pointer_notify_motion(seat, time, sx, sy);
  } else {
    /* Clear pointer focus so future button events and such are not sent to
     * the last client to have the cursor over it. */
    wlr_seat_pointer_clear_focus(seat);
  }
}

static void
server_cursor_motion(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits a _relative_
   * pointer motion event (i.e. a delta) */
  struct tbx_server *server = wl_container_of(listener, server, cursor_motion);
  struct wlr_pointer_motion_event *event = data;
  /* The cursor doesn't move unless we tell it to. The cursor automatically
   * handles constraining the motion to the output layout, as well as any
   * special configuration applied for the specific input device which
   * generated the event. You can pass NULL for the device if you want to move
   * the cursor around without any input. */
  wlr_cursor_move(
      server->cursor, &event->pointer->base, event->delta_x, event->delta_y);
  process_cursor_motion(server, event->time_msec);
}

static void
server_cursor_motion_absolute(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits an _absolute_
   * motion event, from 0..1 on each axis. This happens, for example, when
   * wlroots is running under a Wayland window rather than KMS+DRM, and you
   * move the mouse over the window. You could enter the window from any edge,
   * so we have to warp the mouse there. There is also some hardware which
   * emits these events. */
  struct tbx_server *server =
      wl_container_of(listener, server, cursor_motion_absolute);
  struct wlr_pointer_motion_absolute_event *event = data;
  wlr_cursor_warp_absolute(
      server->cursor, &event->pointer->base, event->x, event->y);
  process_cursor_motion(server, event->time_msec);
}

static void
server_cursor_button(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits a button
   * event. */
  struct tbx_server *server = wl_container_of(listener, server, cursor_button);
  struct wlr_pointer_button_event *event = data;
  /* Notify the client with pointer focus that a button press has occurred */
  wlr_seat_pointer_notify_button(
      server->seat, event->time_msec, event->button, event->state);
  double sx, sy;
  struct wlr_surface *surface = NULL;
  struct tbx_view *view = desktop_view_at(
      server, server->cursor->x, server->cursor->y, &surface, &sx, &sy);
  if (event->state == WLR_BUTTON_RELEASED) {
    server->grabbing = false;
    /* If you released any buttons, we exit interactive move/resize mode. */
    reset_cursor_mode(server);
  } else {
    server->grabbing = true;
    /* Focus that client if the button was _pressed_ */
    if (view) {
      view->interface->set_activated(view, surface, true);
    }
  }
}

static void
server_cursor_axis(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits an axis event,
   * for example when you move the scroll wheel. */
  struct tbx_server *server = wl_container_of(listener, server, cursor_axis);
  struct wlr_pointer_axis_event *event = data;
  /* Notify the client with pointer focus of the axis event. */
  wlr_seat_pointer_notify_axis(server->seat,
      event->time_msec,
      event->orientation,
      event->delta,
      event->delta_discrete,
      event->source);
}

static void
server_cursor_frame(struct wl_listener *listener, void *data)
{
  /* This event is forwarded by the cursor when a pointer emits an frame
   * event. Frame events are sent after regular pointer events to group
   * multiple events together. For instance, two axis events may happen at the
   * same time, in which case a frame event won't be sent in between. */
  struct tbx_server *server = wl_container_of(listener, server, cursor_frame);
  /* Notify the client with pointer focus of the frame event. */
  wlr_seat_pointer_notify_frame(server->seat);
}

bool
tbx_cursor_setup(struct tbx_server *server)
{
  /*
   * Creates a cursor, which is a wlroots utility for tracking the cursor
   * image shown on screen.
   */
  server->cursor = wlr_cursor_create();
  wlr_cursor_attach_output_layout(server->cursor, server->output_layout);

  /* Creates an xcursor manager, another wlroots utility which loads up
   * Xcursor themes to source cursor images from and makes sure that cursor
   * images are available at all scale factors on the screen (necessary for
   * HiDPI support). We add a cursor theme at scale factor 1 to begin with. */
  server->cursor_mgr = wlr_xcursor_manager_create(NULL, 24);
  wlr_xcursor_manager_load(server->cursor_mgr, 1);

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
  server->cursor_mode = TBX_CURSOR_PASSTHROUGH;
  server->cursor_motion.notify = server_cursor_motion;
  wl_signal_add(&server->cursor->events.motion, &server->cursor_motion);
  server->cursor_motion_absolute.notify = server_cursor_motion_absolute;
  wl_signal_add(
      &server->cursor->events.motion_absolute, &server->cursor_motion_absolute);
  server->cursor_button.notify = server_cursor_button;
  wl_signal_add(&server->cursor->events.button, &server->cursor_button);
  server->cursor_axis.notify = server_cursor_axis;
  wl_signal_add(&server->cursor->events.axis, &server->cursor_axis);
  server->cursor_frame.notify = server_cursor_frame;
  wl_signal_add(&server->cursor->events.frame, &server->cursor_frame);
  return true;
}