#include "tinybox/tbx_server.h"
#include "tinybox/xdg_shell.h"
#include "tinybox/cairo.h"

static void xdg_surface_map(struct wl_listener *listener, void *data) {
  /* Called when the surface is mapped, or ready to display on-screen. */
  struct tbx_view *view = wl_container_of(listener, view, map);
  view->mapped = true;
  view->surface = (struct wlr_surface*)view->xdg_surface;
  focus_view(view, view->xdg_surface->surface);
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

  if (view->title) {
    wlr_texture_destroy(view->title);
    wlr_texture_destroy(view->title_unfocused);
  }

  wl_list_remove(&view->link);
  free(view);
}

static void begin_interactive(struct tbx_view *view,
    enum tbx_cursor_mode mode, uint32_t edges) {
  /* This function sets up an interactive move or resize operation, where the
   * compositor stops propegating pointer events to clients and instead
   * consumes them itself, to move or resize windows. */
  struct tbx_server *server = view->server;
  struct wlr_surface *focused_surface =
    server->seat->pointer_state.focused_surface;
  if (view->xdg_surface->surface != focused_surface) {
    /* Deny move/resize requests from unfocused clients. */
    return;
  }
  server->grabbed_view = view;
  server->cursor_mode = mode;

  view->workspace = server->active_workspace;
  view->workspace_id = server->active_workspace_id;

  if (mode == TBX_CURSOR_MOVE) {
    server->grab_x = server->cursor->x - view->x;
    server->grab_y = server->cursor->y - view->y;
  } else {
    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);

    double border_x = (view->x + geo_box.x) + ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
    double border_y = (view->y + geo_box.y) + ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
    server->grab_x = server->cursor->x - border_x;
    server->grab_y = server->cursor->y - border_y;

    server->grab_geobox = geo_box;
    server->grab_geobox.x += view->x;
    server->grab_geobox.y += view->y;

    server->resize_edges = edges;
  }
}

static void xdg_toplevel_request_move(
    struct wl_listener *listener, void *data) {
  /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct tbx_view *view = wl_container_of(listener, view, request_move);
  begin_interactive(view, TBX_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(
    struct wl_listener *listener, void *data) {
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
  struct tbx_view *view =
    wl_container_of(listener, view, set_title);
  view->title_dirty = true;
}

int offset = 0;
static void server_new_xdg_surface(struct wl_listener *listener, void *data) {
  /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
  struct tbx_server *server =
    wl_container_of(listener, server, new_xdg_surface);
  struct wlr_xdg_surface *xdg_surface = data;
  if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
    return;
  }

  /* Allocate a tbx_view for this surface */
  struct tbx_view *view = calloc(1, sizeof(struct tbx_view));
  view->server = server;
  view->xdg_surface = xdg_surface;

  view->x = 4 + (offset*40);
  view->y = 32 + (offset*40);
  offset = (offset+1)%8;

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

void focus_view(struct tbx_view *view, struct wlr_surface *surface) {
  /* Note: this function only deals with keyboard focus. */
  if (view == NULL) {
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
    struct wlr_xdg_surface *previous = wlr_xdg_surface_from_wlr_surface(
          seat->keyboard_state.focused_surface);
    wlr_xdg_toplevel_set_activated(previous, false);
  }
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
  /* Move the view to the front */
  wl_list_remove(&view->link);
  wl_list_insert(&server->views, &view->link);
  /* Activate the new surface */
  wlr_xdg_toplevel_set_activated(view->xdg_surface, true);
  /*
   * Tell the seat to have the keyboard enter this surface. wlroots will keep
   * track of this and automatically send key events to the appropriate
   * clients without additional work on your part.
   */
  wlr_seat_keyboard_notify_enter(seat, view->xdg_surface->surface,
    keyboard->keycodes, keyboard->num_keycodes, &keyboard->modifiers);
}

bool view_at(struct tbx_view *view,
    double lx, double ly, struct wlr_surface **surface,
    double *sx, double *sy) {
  /*
   * XDG toplevels may have nested surfaces, such as popup windows for context
   * menus or tooltips. This function tests if any of those are underneath the
   * coordinates lx and ly (in output Layout Coordinates). If so, it sets the
   * surface pointer to that wlr_surface and the sx and sy coordinates to the
   * coordinates relative to that surface's top-left corner.
   */
  double view_sx = lx - view->x;
  double view_sy = ly - view->y;

  double _sx, _sy;
  struct wlr_surface *_surface = NULL;
  _surface = wlr_xdg_surface_surface_at(
      view->xdg_surface, view_sx, view_sy, &_sx, &_sy);

  if (_surface != NULL) {
    *sx = _sx;
    *sy = _sy;
    *surface = _surface;
    return true;
  }

  return false;
}

bool hotspot_at(struct tbx_view *view,
    double lx, double ly, struct wlr_surface **surface,
    double *sx, double *sy) {

  // TODO: check multiple outputs

  const int resizeEdges[] = {
    WLR_EDGE_BOTTOM | WLR_EDGE_LEFT,
    WLR_EDGE_BOTTOM | WLR_EDGE_RIGHT,
    WLR_EDGE_TOP,
    WLR_EDGE_BOTTOM,
    WLR_EDGE_LEFT,
    WLR_EDGE_RIGHT
  };

  view->hotspot = -1;
  view->hotspot_edges = WLR_EDGE_NONE;
  for(int i=0; i<(int)HS_COUNT;i++) {
    struct wlr_box *box = &view->hotspots[i];

    // if (i == HS_TITLEBAR) {
    //   console_log("hs: x:%d y:%d w:%d h:%d", box->x, box->y, box->width, box->height);
    // }

    if (!box->width || !box->height) {
      continue;
    }
    if (lx >= box->x && lx <= box->x + box->width &&
        ly >= box->y && ly <= box->y + box->height) {
      view->hotspot = i;
      if (i<=HS_EDGE_RIGHT) {
        view->hotspot_edges = resizeEdges[i];
      }
      return true;
    }
  }

  return false;
}


struct tbx_view *desktop_view_at(
    struct tbx_server *server, double lx, double ly,
    struct wlr_surface **surface, double *sx, double *sy) {
  /* This iterates over all of our surfaces and attempts to find one under the
   * cursor. This relies on server->views being ordered from top-to-bottom. */
  struct tbx_view *view;



  struct wlr_box *main_box = wlr_output_layout_get_box(
      server->output_layout, server->main_output->wlr_output);

  bool in_main_display = (
      (lx >= main_box->x && lx <= main_box->x + main_box->width) &&
      (ly >= main_box->y && ly <= main_box->y + main_box->height)
    );

  // console_clear();
  // console_log("lx:%d ly:%d", (int)lx,(int)ly);
  // console_log("(%d, %d) - (%d, %d) ",
  //     (int)main_box->x,(int)main_box->y,
  //     (int)main_box->width,(int)main_box->height);

  wl_list_for_each(view, &server->views, link) {

    if (in_main_display && view->workspace != server->active_workspace) {
      continue;
    }

    if (!view->shaded && view_at(view, lx, ly, surface, sx, sy)) {
      view->hotspot = -1;
      server->grabbed_view = NULL;
      return view;
    }

    if (hotspot_at(view, lx, ly, surface, sx, sy)) {
      return view;
    }
      
  }
  return NULL;
}

void xdg_shell_init() {
  /* Set up our list of views and the xdg-shell. The xdg-shell is a Wayland
   * protocol which is used for application windows. For more detail on
   * shells, refer to my article:
   *
   * https://drewdevault.com/2018/07/29/Wayland-shells.html
   */
  wl_list_init(&server.views);
  server.xdg_shell = wlr_xdg_shell_create(server.wl_display);
  server.new_xdg_surface.notify = server_new_xdg_surface;
  wl_signal_add(&server.xdg_shell->events.new_surface,
      &server.new_xdg_surface);

}

