#include "tinybox/view.h"
#include "tinybox/output.h"

#include <stdlib.h>

#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>

const char *get_string_prop(struct tbx_view *view, enum tbx_view_prop prop) {
  if (view->xwayland_surface) {
    switch (prop) {
    case VIEW_PROP_TITLE:
      return view->xwayland_surface->title;
    // case VIEW_PROP_CLASS:
      // return view->wlr_xwayland_surface->class;
    // case VIEW_PROP_INSTANCE:
    //   return view->wlr_xwayland_surface->instance;
    // case VIEW_PROP_WINDOW_ROLE:
    //   return view->wlr_xwayland_surface->role;
    default:
      return NULL;
    }

    return NULL;
  }

  if (view->xdg_surface) {
    switch (prop) {
      case VIEW_PROP_TITLE:
        return view->xdg_surface->toplevel->title;
      case VIEW_PROP_APP_ID:
        return view->xdg_surface->toplevel->app_id;
      default:
        return NULL;
    }
  }
  return NULL;
}

void focus_view(struct tbx_view *view, struct wlr_surface *surface) {
  /* Note: this function only deals with keyboard focus. */
  if (view == NULL) {
    return;
  }

  // implement xwayland_surface!
  if (!view->xdg_surface) {
    return;
  }

  struct tbx_server *server = view->server;
  struct wlr_seat *seat = server->seat->seat;
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
    struct wlr_xdg_surface *previous =
        wlr_xdg_surface_from_wlr_surface(seat->keyboard_state.focused_surface);
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
                                 keyboard->keycodes, keyboard->num_keycodes,
                                 &keyboard->modifiers);
}

void focus_view_without_raising(struct tbx_view *view,
                                struct wlr_surface *surface) {
  /* Note: this function only deals with keyboard focus. */
  if (view == NULL) {
    return;
  }

  // implement xwayland_surface!
  if (!view->xdg_surface) {
    return;
  }

  struct tbx_server *server = view->server;
  struct wlr_seat *seat = server->seat->seat;
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
    struct wlr_xdg_surface *previous =
        wlr_xdg_surface_from_wlr_surface(seat->keyboard_state.focused_surface);
    wlr_xdg_toplevel_set_activated(previous, false);
  }
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

  /* Activate the new surface */
  wlr_xdg_toplevel_set_activated(view->xdg_surface, true);
  /*
   * Tell the seat to have the keyboard enter this surface. wlroots will keep
   * track of this and automatically send key events to the appropriate
   * clients without additional work on your part.
   */
  wlr_seat_keyboard_notify_enter(seat, view->xdg_surface->surface,
                                 keyboard->keycodes, keyboard->num_keycodes,
                                 &keyboard->modifiers);
}

bool view_at(struct tbx_view *view, double lx, double ly,
             struct wlr_surface **surface, double *sx, double *sy) {
    // implement xwayland_surface!
  if (!view->xdg_surface) {
    return false;
  }

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
  _surface = wlr_xdg_surface_surface_at(view->xdg_surface, view_sx, view_sy,
                                        &_sx, &_sy);

  if (_surface != NULL) {
    *sx = _sx;
    *sy = _sy;
    *surface = _surface;
    return true;
  }

  return false;
}

struct tbx_view *desktop_view_at(struct tbx_server *server, double lx,
                                 double ly, struct wlr_surface **surface,
                                 double *sx, double *sy) {
  /* This iterates over all of our surfaces and attempts to find one under the
   * cursor. This relies on server->views being ordered from top-to-bottom. */
  struct tbx_view *view;

  struct wlr_box *main_box = wlr_output_layout_get_box(
      server->output_layout, server->main_output->wlr_output);

  bool in_main_display =
      ((lx >= main_box->x && lx <= main_box->x + main_box->width) &&
       (ly >= main_box->y && ly <= main_box->y + main_box->height));

  wl_list_for_each(view, &server->views, link) {

    if (in_main_display && view->workspace != server->workspace) {
      continue;
    }

    bool shaded = view->shaded;
    if (!shaded && view_at(view, lx, ly, surface, sx, sy)) {
      view->hotspot = HS_NONE;
      return view;
    }

    if (hotspot_at(view, lx, ly, surface, sx, sy)) {
      return view;
    }
  }
  return NULL;
}

bool hotspot_at(struct tbx_view *view, double lx, double ly,
                struct wlr_surface **surface, double *sx, double *sy) {

  const int resizeEdges[] = {WLR_EDGE_BOTTOM | WLR_EDGE_LEFT,
                             WLR_EDGE_BOTTOM | WLR_EDGE_RIGHT,
                             WLR_EDGE_TOP,
                             WLR_EDGE_BOTTOM,
                             WLR_EDGE_LEFT,
                             WLR_EDGE_RIGHT};

  view->hotspot = HS_NONE;
  view->hotspot_edges = WLR_EDGE_NONE;
  for (int i = 0; i < (int)HS_COUNT; i++) {
    struct wlr_box *box = &view->hotspots[i];

    if (!box->width || !box->height) {
      continue;
    }
    if (lx >= box->x && lx <= box->x + box->width && ly >= box->y &&
        ly <= box->y + box->height) {
      view->hotspot = i;
      if (i <= HS_EDGE_RIGHT) {
        view->hotspot_edges = resizeEdges[i];
      }
      return true;
    }
  }

  return false;
}

bool view_is_visible(struct tbx_output *output, struct tbx_view *view) {
    // implement xwayland_surface!
  if (!view->xdg_surface) {
    return false;
  }

  struct wlr_box *box = wlr_output_layout_get_box(view->server->output_layout,
                                                  output->wlr_output);

  struct wlr_box geo;
  wlr_xdg_surface_get_geometry(view->xdg_surface, &geo);

  geo.x += view->x;
  geo.y += view->y;

  // console_log("v:%d %d o:%d %d", geo.x, geo.y, box->x, box->y);

  // top-left
  if (geo.x >= box->x && geo.x <= box->x + box->width && geo.y >= box->y &&
      geo.y <= box->y + box->height) {
    return true;
  }
  // top right
  if (geo.x + geo.width >= box->x && geo.x + geo.width <= box->x + box->width &&
      geo.y >= box->y && geo.y <= box->y + box->height) {
    return true;
  }
  // bottom-left
  if (geo.x >= box->x && geo.x <= box->x + box->width &&
      geo.y + geo.height >= box->y &&
      geo.y + geo.height <= box->y + box->height) {
    return true;
  }
  // bottom right
  if (geo.x + geo.width >= box->x && geo.x + geo.width <= box->x + box->width &&
      geo.y + geo.height >= box->y &&
      geo.y + geo.height <= box->y + box->height) {
    return true;
  }

  return false;
}

void view_destroy(struct tbx_view *view) {
  if (view->title) {
    wlr_texture_destroy(view->title);
    wlr_texture_destroy(view->title_unfocused);
    view->title = NULL;
    view->title_unfocused = NULL;
  }

  wl_list_remove(&view->link);
  free(view);
}

void view_close(struct tbx_view *view) {
    // implement xwayland_surface!
  if (!view->xdg_surface) {
    return;
  }

  struct wlr_xdg_surface *surface = view->xdg_surface;
  if (surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL && surface->toplevel) {
    wlr_xdg_toplevel_send_close(surface);
  }
}