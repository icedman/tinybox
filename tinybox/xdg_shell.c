#include <tinybox/server.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#include <wlr/types/wlr_xdg_shell.h>

#include <assert.h>
#include <stdlib.h>
#include <float.h>

static void
xdg_set_activated(struct tbx_view *view, struct wlr_surface *surface, bool activated)
{
  /* Note: this function only deals with keyboard focus. */
  if (view == NULL || surface == NULL) {
    return;
  }

  struct tbx_server *server = view->server;
  struct wlr_seat *seat = server->seat;
  struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
  if (prev_surface == surface) {
    /* Don't re-focus an already focused surface. */
    return;
  }
  if (prev_surface) {
    /*
     * Deactivate the previously focused surface. This lets the client know
     * it no longer has focus and the client will repaint accordingly, e.g.
     * stop displaying a caret.
     */
    struct wlr_xdg_surface *previous = wlr_xdg_surface_try_from_wlr_surface(
        seat->keyboard_state.focused_surface);
    assert(previous != NULL && previous->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
    wlr_xdg_toplevel_set_activated(previous->toplevel, !activated);
  }
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
  /* Move the view to the front */
  wlr_scene_node_raise_to_top(&view->scene_tree->node);
  wl_list_remove(&view->link);
  wl_list_insert(&server->views, &view->link);
  /* Activate the new surface */
  wlr_xdg_toplevel_set_activated(view->surface.xdg->toplevel, activated);
  /*
   * Tell the seat to have the keyboard enter this surface. wlroots will keep
   * track of this and automatically send key events to the appropriate
   * clients without additional work on your part.
   */
  if (keyboard != NULL) {
    wlr_seat_keyboard_notify_enter(seat,
        view->surface.xdg->toplevel->base->surface,
        keyboard->keycodes,
        keyboard->num_keycodes,
        &keyboard->modifiers);
  }
}

static void
xdg_get_constraints(struct tbx_view *view,
    double *min_width,
    double *max_width,
    double *min_height,
    double *max_height)
{
  struct wlr_xdg_toplevel_state *state = &view->surface.xdg->toplevel->current;
  *min_width = state->min_width > 0 ? state->min_width : DBL_MIN;
  *max_width = state->max_width > 0 ? state->max_width : DBL_MAX;
  *min_height = state->min_height > 0 ? state->min_height : DBL_MIN;
  *max_height = state->max_height > 0 ? state->max_height : DBL_MAX;
}

static void
xdg_get_geometry(struct tbx_view *view, struct wlr_box *box)
{
  if (view->surface.xdg->toplevel) {
    wlr_xdg_surface_get_geometry(view->surface.xdg, box);
    box->x = box->y = 0;
  } else {
    box->width = 0;
    box->height = 0;
  }
}


static uint32_t
xdg_view_configure(
    struct tbx_view *view, double lx, double ly, int width, int height)
{
  struct wlr_box box;
  wlr_xdg_surface_get_geometry(view->surface.xdg, &box);

  view->x = lx - box.x;
  view->y = ly - box.y;

  if (width != box.width) {
    // view->title_dirty = true;
  }

  wlr_xdg_toplevel_set_size(view->surface.xdg->toplevel, width, height);
  view->request_box.x = view->x;
  view->request_box.y = view->y;
  view->request_box.width = width;
  view->request_box.height = height;
  return 0;
}

static void
xdg_set_fullscreen(struct tbx_view *view, bool fullscreen)
{
  if (!view->surface.xdg) {
    return;
  }

  // console_log("xdg fullscreen %d", fullscreen);

  if (fullscreen) {
    view->fullscreen = fullscreen;
    view->restore_box.x = view->x;
    view->restore_box.y = view->y;

    struct wlr_box window_box;
    xdg_get_geometry(view, &window_box);
    view->restore_box.width = window_box.width;
    view->restore_box.height = window_box.height;

    // todo get output from view
    struct wlr_box full_box;
    wlr_output_layout_get_box(
        view->server->output_layout,
        view->server->main_output->wlr_output,
        &full_box);

    view->x = 0;
    view->y = 0;
    view->width = full_box.width;
    view->height = full_box.height;
    xdg_view_configure(view, 0, 0, full_box.width, full_box.height);
  } else {
    view->fullscreen = fullscreen;
    view->x = view->restore_box.x;
    view->y = view->restore_box.y;
    view->width = view->restore_box.width;
    view->height = view->restore_box.height;
    xdg_view_configure(view, 0, 0, view->restore_box.width, view->restore_box.height);
  }

  // wlr_xdg_furface_set_fullscreen(view->xdg_furface, fullscreen);
}

static void
xdg_close(struct tbx_view *view)
{
  struct wlr_xdg_surface *surface = view->surface.xdg;
  if (surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL && surface->toplevel) {
    wlr_xdg_toplevel_send_close(view->surface.xdg->toplevel);
  }
}

static void
xdg_close_popups(struct tbx_view *view)
{}

static struct tbx_view_interface xdg_view_interface = {
  .set_activated = xdg_set_activated,
  .get_constraints = xdg_get_constraints,
  .get_geometry = xdg_get_geometry,
  .configure = xdg_view_configure,
  .set_fullscreen = xdg_set_fullscreen,
  .close = xdg_close,
  .close_popups = xdg_close_popups
};

static void
xdg_toplevel_map(struct wl_listener *listener, void *data)
{
  /* Called when the surface is mapped, or ready to display on-screen. */
  struct tbx_view *view = wl_container_of(listener, view, map);

  struct wlr_xdg_surface *xdg_surface = view->surface.xdg;
  view->scene_tree = wlr_scene_xdg_surface_create(
      view->server->scene_views, view->surface.xdg->toplevel->base);
  view->scene_tree->node.data = view;
  xdg_surface->data = view->scene_tree;

  /* Add it to the list of views. */
  wl_list_insert(&view->server->views, &view->link);

  view->interface->set_activated(view, view->surface.xdg->toplevel->base->surface, true);
}

static void
xdg_toplevel_unmap(struct wl_listener *listener, void *data)
{
  /* Called when the surface is unmapped, and should no longer be shown.
   */
  struct tbx_view *view = wl_container_of(listener, view, unmap);

  /* Reset the cursor mode if the grabbed view was unmapped. */
  if (view == view->server->grabbed_view) {
    reset_cursor_mode(view->server);
  }

  wl_list_remove(&view->link);
}

static void
xdg_toplevel_destroy(struct wl_listener *listener, void *data)
{
  /* Called when the surface is destroyed and should never be shown again.
   */
  struct tbx_view *view = wl_container_of(listener, view, destroy);

  wl_list_remove(&view->map.link);
  wl_list_remove(&view->unmap.link);
  wl_list_remove(&view->destroy.link);
  wl_list_remove(&view->request_move.link);
  wl_list_remove(&view->request_resize.link);
  wl_list_remove(&view->request_maximize.link);
  wl_list_remove(&view->request_fullscreen.link);

  free(view);
}

static void
begin_interactive(
    struct tbx_view *view, enum tbx_cursor_mode mode, uint32_t edges)
{
  /* This function sets up an interactive move or resize operation, where the
   * compositor stops propegating pointer events to clients and instead
   * consumes them itself, to move or resize windows. */
  struct tbx_server *server = view->server;

  if (!server->grabbing) {
    return;
  }

  struct wlr_surface *focused_surface =
      server->seat->pointer_state.focused_surface;
  if (view->surface.xdg->toplevel->base->surface !=
      wlr_surface_get_root_surface(focused_surface)) {
    /* Deny move/resize requests from unfocused clients. */
    return;
  }
  server->grabbed_view = view;
  server->cursor_mode = mode;

  if (mode == TBX_CURSOR_MOVE) {
    server->grab_x = server->cursor->x - view->scene_tree->node.x;
    server->grab_y = server->cursor->y - view->scene_tree->node.y;
  } else {
    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->surface.xdg->toplevel->base, &geo_box);

    double border_x = (view->scene_tree->node.x + geo_box.x) +
                      ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (view->scene_tree->node.y + geo_box.y) +
                      ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
    server->grab_x = server->cursor->x - border_x;
    server->grab_y = server->cursor->y - border_y;

    server->grab_geobox = geo_box;
    server->grab_geobox.x += view->scene_tree->node.x;
    server->grab_geobox.y += view->scene_tree->node.y;

    server->resize_edges = edges;
  }
}

static void
xdg_toplevel_request_move(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provided serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct tbx_view *view = wl_container_of(listener, view, request_move);
  begin_interactive(view, TBX_CURSOR_MOVE, 0);
}

static void
xdg_toplevel_request_resize(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to begin an interactive
   * resize, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provided serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct wlr_xdg_toplevel_resize_event *event = data;
  struct tbx_view *view = wl_container_of(listener, view, request_resize);
  begin_interactive(view, TBX_CURSOR_RESIZE, event->edges);
}

static void
xdg_toplevel_request_maximize(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to maximize itself,
   * typically because the user clicked on the maximize button on
   * client-side decorations. tinywl doesn't support maximization, but
   * to conform to xdg-shell protocol we still must send a configure.
   * wlr_xdg_surface_schedule_configure() is used to send an empty reply. */
  struct tbx_view *view = wl_container_of(listener, view, request_maximize);
  wlr_xdg_surface_schedule_configure(view->surface.xdg->toplevel->base);
}

static void
xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data)
{
  /* Just as with request_maximize, we must send a configure here. */
  struct tbx_view *view = wl_container_of(listener, view, request_fullscreen);
  wlr_xdg_surface_schedule_configure(view->surface.xdg->toplevel->base);
}

static void
server_new_xdg_surface(struct wl_listener *listener, void *data)
{
  /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
  struct tbx_server *server =
      wl_container_of(listener, server, new_xdg_surface);
  struct wlr_xdg_surface *xdg_surface = data;

  /* We must add xdg popups to the scene graph so they get rendered. The
   * wlroots scene graph provides a helper for this, but to use it we must
   * provide the proper parent scene node of the xdg popup. To enable this,
   * we always set the user data field of xdg_surfaces to the corresponding
   * scene node. */
  if (xdg_surface->role == WLR_XDG_SURFACE_ROLE_POPUP) {
    struct wlr_xdg_surface *parent =
        wlr_xdg_surface_try_from_wlr_surface(xdg_surface->popup->parent);
    assert(parent != NULL);
    struct wlr_scene_tree *parent_tree = parent->data;
    xdg_surface->data = wlr_scene_xdg_surface_create(parent_tree, xdg_surface);
    return;
  }
  assert(xdg_surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);

  /* Allocate a tbx_view for this surface */
  struct tbx_view *view = calloc(1, sizeof(struct tbx_view));
  memset(view, 0, sizeof(struct tbx_view));
  view->server = server;
  view->type = TBX_XDG_SHELL;
  view->xdg_toplevel = xdg_surface->toplevel;
  view->surface.xdg = xdg_surface;

  view->interface = &xdg_view_interface;

  /* Listen to the various events it can emit */
  view->map.notify = xdg_toplevel_map;
  wl_signal_add(&xdg_surface->events.map, &view->map);
  view->unmap.notify = xdg_toplevel_unmap;
  wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
  view->destroy.notify = xdg_toplevel_destroy;
  wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

  /* cotd */
  struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
  view->request_move.notify = xdg_toplevel_request_move;
  wl_signal_add(&toplevel->events.request_move, &view->request_move);
  view->request_resize.notify = xdg_toplevel_request_resize;
  wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
  view->request_maximize.notify = xdg_toplevel_request_maximize;
  wl_signal_add(&toplevel->events.request_maximize, &view->request_maximize);
  view->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
  wl_signal_add(
      &toplevel->events.request_fullscreen, &view->request_fullscreen);
}

bool
tbx_xdg_shell_setup(struct tbx_server *server)
{
  /* Set up xdg-shell version 3. The xdg-shell is a Wayland protocol which is
   * used for application windows. For more detail on shells, refer to my
   * article:
   *
   * https://drewdevault.com/2018/07/29/Wayland-shells.html
   */
  server->xdg_shell = wlr_xdg_shell_create(server->wl_display, 3);
  server->new_xdg_surface.notify = server_new_xdg_surface;
  wl_signal_add(
      &server->xdg_shell->events.new_surface, &server->new_xdg_surface);

  return true;
}