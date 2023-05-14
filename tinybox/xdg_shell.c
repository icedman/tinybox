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

static void
xdg_toplevel_map(struct wl_listener *listener, void *data)
{
  /* Called when the surface is mapped, or ready to display on-screen. */
  struct tbx_view *view = wl_container_of(listener, view, map);

  wl_list_insert(&view->server->views, &view->link);

  focus_view(view, view->xdg_toplevel->base->surface);
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
  if (view->xdg_toplevel->base->surface !=
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
    wlr_xdg_surface_get_geometry(view->xdg_toplevel->base, &geo_box);

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
  wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
}

static void
xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data)
{
  /* Just as with request_maximize, we must send a configure here. */
  struct tbx_view *view = wl_container_of(listener, view, request_fullscreen);
  wlr_xdg_surface_schedule_configure(view->xdg_toplevel->base);
}

static void
server_new_xdg_surface(struct wl_listener *listener, void *data)
{
  /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
  struct tbx_shell *shell = wl_container_of(listener, shell, new_shell_surface);
  struct tbx_server *server = shell->server;
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
  view->server = server;
  view->xdg_toplevel = xdg_surface->toplevel;
  view->scene_tree = wlr_scene_xdg_surface_create(
      &view->server->scene->tree, view->xdg_toplevel->base);
  view->scene_tree->node.data = view;
  xdg_surface->data = view->scene_tree;

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
  struct tbx_shell *shell = &server->xdg_shell;
  shell->server = server;

  /* Set up xdg-shell version 3. The xdg-shell is a Wayland protocol which is
   * used for application windows. For more detail on shells, refer to my
   * article:
   *
   * https://drewdevault.com/2018/07/29/Wayland-shells.html
   */
  shell->wlr_shell = wlr_xdg_shell_create(server->wl_display, 3);
  shell->new_shell_surface.notify = server_new_xdg_surface;
  wl_signal_add(
      &shell->wlr_shell->events.new_surface, &shell->new_shell_surface);

  return true;
}