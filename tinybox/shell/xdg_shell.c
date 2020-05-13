#include "tinybox/server.h"
#include "tinybox/shell.h"
#include "tinybox/view.h"

#include <stdlib.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

static void xdg_surface_map(struct wl_listener *listener, void *data) {
  /* Called when the surface is mapped, or ready to display on-screen. */
  struct tbx_view *view = wl_container_of(listener, view, map);
  view->mapped = true;
  view->surface = (struct wlr_surface *)view->xdg_surface;
  // focus_view(view, view->xdg_surface->surface);
}

static void xdg_surface_unmap(struct wl_listener *listener, void *data) {
  /* Called when the surface is unmapped, and should no longer be shown. */
  struct tbx_view *view = wl_container_of(listener, view, unmap);
  view->surface = NULL;
  view->mapped = false;
}

static void xdg_surface_destroy(struct wl_listener *listener, void *data) {
  /* Called when the surface is destroyed and should never be shown again. */
  struct tbx_view *view = wl_container_of(listener, view, destroy);

  // if (view->title) {
  //   wlr_texture_destroy(view->title);
  //   wlr_texture_destroy(view->title_unfocused);
  // }

  wl_list_remove(&view->link);
  free(view);
}

static void begin_interactive(struct tbx_view *view, enum tbx_cursor_mode mode,
                              uint32_t edges) {
  /* This function sets up an interactive move or resize operation, where the
   * compositor stops propegating pointer events to clients and instead
   * consumes them itself, to move or resize windows. */
  struct tbx_server *server = view->server;
  struct wlr_surface *focused_surface =
      server->seat->seat->pointer_state.focused_surface;

  if (view->xdg_surface->surface != focused_surface) {
    /* Deny move/resize requests from unfocused clients. */
    return;
  }

  struct tbx_cursor *cursor = server->cursor;
  cursor->grab_view = view;
  cursor->mode = mode;

  if (mode == TBX_CURSOR_MOVE) {
    cursor->grab_x = cursor->cursor->x - view->x;
    cursor->grab_y = cursor->cursor->y - view->y;

  } else {

    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);

    double border_x =
        (view->x + geo_box.x) + ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (view->y + geo_box.y) +
                      ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
    cursor->grab_x = cursor->cursor->x - border_x;
    cursor->grab_y = cursor->cursor->y - border_y;

    cursor->grab_box = geo_box;
    cursor->grab_box.x += view->x;
    cursor->grab_box.y += view->y;

    cursor->resize_edges = edges;
  }
}

static void xdg_toplevel_request_move(struct wl_listener *listener,
                                      void *data) {
  /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct tbx_view *view = wl_container_of(listener, view, request_move);
  begin_interactive(view, TBX_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(struct wl_listener *listener,
                                        void *data) {
  /* This event is raised when a client would like to begin an interactive
   * resize, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct wlr_xdg_toplevel_resize_event *event = data;
  struct tbx_view *view = wl_container_of(listener, view, request_resize);
  begin_interactive(view, TBX_CURSOR_RESIZE, event->edges);
}

static void handle_set_title(struct wl_listener *listener, void *data) {
  struct tbx_view *view = wl_container_of(listener, view, set_title);

  // view->title_dirty = true;
}

static void server_new_xdg_surface(struct wl_listener *listener, void *data) {

  /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
  struct tbx_xdg_shell *xdg_shell =
      wl_container_of(listener, xdg_shell, new_xdg_surface);
  struct tbx_server *server = xdg_shell->server;

  struct wlr_xdg_surface *xdg_surface = data;
  if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
    return;
  }

  /* Allocate a tbx_view for this surface */
  struct tbx_view *view = calloc(1, sizeof(struct tbx_view));
  view->xdg_surface = xdg_surface;
  view->server = server;

  xdg_shell->create_offset = (xdg_shell->create_offset + 1) % 8;
  view->x = 4 + (xdg_shell->create_offset * 40);
  view->y = 32 + (xdg_shell->create_offset * 40);

  /* Listen to the various events it can emit */
  view->map.notify = xdg_surface_map;
  wl_signal_add(&xdg_surface->events.map, &view->map);
  view->unmap.notify = xdg_surface_unmap;
  wl_signal_add(&xdg_surface->events.unmap, &view->unmap);
  view->destroy.notify = xdg_surface_destroy;
  wl_signal_add(&xdg_surface->events.destroy, &view->destroy);

  /* cotd */
  struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
  view->request_move.notify = xdg_toplevel_request_move;
  wl_signal_add(&toplevel->events.request_move, &view->request_move);
  view->request_resize.notify = xdg_toplevel_request_resize;
  wl_signal_add(&toplevel->events.request_resize, &view->request_resize);

  /* title */
  view->set_title.notify = handle_set_title;
  wl_signal_add(&toplevel->events.set_title, &view->set_title);

  /* Add it to the list of views. */
  wl_list_insert(&server->views, &view->link);
}

bool xdg_shell_setup(struct tbx_server *server) {
  /* Set up our list of views and the xdg-shell. The xdg-shell is a Wayland
   * protocol which is used for application windows. For more detail on
   * shells, refer to my article:
   *
   * https://drewdevault.com/2018/07/29/Wayland-shells.html
   */
  server->xdg_shell = calloc(1, sizeof(struct tbx_xdg_shell));
  server->xdg_shell->server = server;
  server->xdg_shell->wlr_xdg_shell = wlr_xdg_shell_create(server->wl_display);
  server->xdg_shell->new_xdg_surface.notify = server_new_xdg_surface;

  wl_list_init(&server->views);
  wl_signal_add(&server->xdg_shell->wlr_xdg_shell->events.new_surface,
                &server->xdg_shell->new_xdg_surface);

  return true;
}