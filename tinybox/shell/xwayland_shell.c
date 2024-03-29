#define _POSIX_C_SOURCE 200112L

#include "tinybox/damage.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"
#include "tinybox/xwayland.h"

#include <float.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>

// copy now ~ understand later
static const char *atom_map[ATOM_LAST] = {
  "_NET_WM_WINDOW_TYPE_NORMAL",
  "_NET_WM_WINDOW_TYPE_DIALOG",
  "_NET_WM_WINDOW_TYPE_UTILITY",
  "_NET_WM_WINDOW_TYPE_TOOLBAR",
  "_NET_WM_WINDOW_TYPE_SPLASH",
  "_NET_WM_WINDOW_TYPE_MENU",
  "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
  "_NET_WM_WINDOW_TYPE_POPUP_MENU",
  "_NET_WM_WINDOW_TYPE_TOOLTIP",
  "_NET_WM_WINDOW_TYPE_NOTIFICATION",
  "_NET_WM_STATE_MODAL",
};

static void
xwayland_get_constraints(struct tbx_view *view,
    double *min_width,
    double *max_width,
    double *min_height,
    double *max_height)
{
  struct wlr_xwayland_surface *surface = view->xwayland_surface;
  struct wlr_xwayland_surface_size_hints *size_hints = surface->size_hints;

  if (size_hints == NULL) {
    *min_width = DBL_MIN;
    *max_width = DBL_MAX;
    *min_height = DBL_MIN;
    *max_height = DBL_MAX;
    return;
  }

  *min_width = size_hints->min_width > 0 ? size_hints->min_width : DBL_MIN;
  *max_width = size_hints->max_width > 0 ? size_hints->max_width : DBL_MAX;
  *min_height = size_hints->min_height > 0 ? size_hints->min_height : DBL_MIN;
  *max_height = size_hints->max_height > 0 ? size_hints->max_height : DBL_MAX;
}

static void
xwayland_get_geometry(struct tbx_view *view, struct wlr_box *box)
{
  box->x = box->y = 0;
  if (view->surface) {
    box->width = view->surface->current.width;
    box->height = view->surface->current.height;
  } else {
    box->width = 0;
    box->height = 0;
  }
}

static void
xwayland_set_activated(struct tbx_view *view, bool activated)
{
  console_log("set activated %d\n",
      view->xwayland_surface ? view->xwayland_surface->window_id : 0);

  struct wlr_seat *seat = view->server->seat->seat;
  struct wlr_xwayland *xwayland = view->server->xwayland_shell->wlr_xwayland;

  if (!view->xwayland_surface || !view->surface) {
    return;
  }

  if (activated) {
    wlr_xwayland_set_seat(xwayland, seat);
    struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
    wlr_seat_keyboard_notify_enter(seat,
        view->surface,
        keyboard->keycodes,
        keyboard->num_keycodes,
        &keyboard->modifiers);
  }

  wlr_xwayland_surface_activate(view->xwayland_surface, activated);
}

static void
xwaylan_set_fullscreen(struct tbx_view *view, bool fullscreen)
{
  if (!view->xwayland_surface) {
    return;
  }

  console_log("fullscreen %d", fullscreen);

  if (fullscreen) {
    view->fullscreen = fullscreen;
    view->restore.x = view->x;
    view->restore.y = view->y;

    struct wlr_box window_box;
    xwayland_get_geometry(view, &window_box);
    view->restore.width = window_box.width;
    view->restore.height = window_box.height;

    // todo get output from view
    struct wlr_box *full_box = wlr_output_layout_get_box(
        view->server->output_layout, view->server->main_output->wlr_output);

    view->x = 0;
    view->y = 0;
    view->width = full_box->width;
    view->height = full_box->height;
    wlr_xwayland_surface_configure(
        view->xwayland_surface, 0, 0, full_box->width, full_box->height);
  } else {
    view->fullscreen = fullscreen;
    view->x = view->restore.x;
    view->y = view->restore.y;
    view->width = view->restore.width;
    view->height = view->restore.height;
    wlr_xwayland_surface_configure(view->xwayland_surface,
        0,
        0,
        view->restore.width,
        view->restore.height);
  }

  wlr_xwayland_surface_set_fullscreen(view->xwayland_surface, fullscreen);

  view->server->suspend_damage_tracking += fullscreen ? 1 : - 1;
}

static const char *
xwayland_get_string_prop(struct tbx_view *view, enum tbx_view_prop prop)
{
  switch (prop) {
    case VIEW_PROP_TITLE:
      return view->xwayland_surface->title;
    case VIEW_PROP_CLASS:
      return view->xwayland_surface->class;
    case VIEW_PROP_INSTANCE:
      return view->xwayland_surface->instance;
    case VIEW_PROP_WINDOW_ROLE:
      return view->xwayland_surface->role;
    default:
      return NULL;
  }
}

uint32_t
xwayland_get_int_prop(struct tbx_view *view, enum tbx_view_prop prop)
{
  switch (prop) {
    case VIEW_PROP_X11_WINDOW_ID:
      return view->xwayland_surface->window_id;
    case VIEW_PROP_X11_PARENT_ID:
      if (view->xwayland_surface->parent) {
        return view->xwayland_surface->parent->window_id;
      }
      return 0;
    case VIEW_PROP_WINDOW_TYPE:
      if (view->xwayland_surface->window_type_len == 0) {
        return 0;
      }
      return view->xwayland_surface->window_type[0];
    default:
      return 0;
  }
  return 0;
}

static uint32_t
xwayland_view_configure(
    struct tbx_view *view, double lx, double ly, int width, int height)
{
  console_log("configure %d %d %d %d\n", lx, ly, width, height);

  wlr_xwayland_surface_configure(view->xwayland_surface, 0, 0, width, height);

  view->x = lx;
  view->y = ly;

  view->request_box.x = lx;
  view->request_box.y = ly;
  view->request_box.width = width;
  view->request_box.height = height;

  damage_whole(view->server);

  return 0;
}

static void
xwayland_close(struct tbx_view *view)
{
  if (view->xwayland_surface) {
    wlr_xwayland_surface_close(view->xwayland_surface);
  }
}

static void
xwayland_close_popups(struct tbx_view *view)
{}

static void
xwayland_destroy(struct tbx_view *view)
{
  struct tbx_xwayland_view *xview = (struct tbx_xwayland_view *)view;

  struct wl_listener *l = &xview->_first;
  while (++l) {
    if (!l->link.prev || l == &xview->destroy) {
      break;
    }
    wl_list_remove(&l->link);
    // console_log("xway unlisten");
  }

  // free all listeners here!
  view_destroy(view);
}

struct tbx_view_interface xwayland_view_interface = {
  .get_constraints = xwayland_get_constraints,
  .get_geometry = xwayland_get_geometry,
  .get_string_prop = xwayland_get_string_prop,
  .get_int_prop = xwayland_get_int_prop,
  .configure = xwayland_view_configure,
  .set_activated = xwayland_set_activated,
  .set_fullscreen = xwaylan_set_fullscreen,
  .close = xwayland_close,
  .close_popups = xwayland_close_popups,
  .destroy = xwayland_destroy
};

static void
xwayland_surface_commit(struct wl_listener *listener, void *data)
{
  console_log("commit xwayland");
  struct tbx_xwayland_view *xwayland_view =
      wl_container_of(listener, xwayland_view, commit);
  struct tbx_view *view = &xwayland_view->view;
  damage_add_commit(view->server, view);
}

static void
xwayland_surface_destroy(struct wl_listener *listener, void *data)
{
  /* Called when the surface is destroyed and should never be shown again. */
  struct tbx_xwayland_view *xwayland_view =
      wl_container_of(listener, xwayland_view, destroy);
  struct tbx_view *view = &xwayland_view->view;
  view->interface->destroy(view);
}

static void
xwayland_surface_map(struct wl_listener *listener, void *data)
{
  /* Called when the surface is mapped, or ready to display on-screen. */
  struct wlr_xwayland_surface *xsurface = data;

  struct tbx_xwayland_view *xwayland_view =
      wl_container_of(listener, xwayland_view, map);
  struct tbx_view *view = &xwayland_view->view;
  struct tbx_server *server = view->server;

  console_log("map %d", view->xwayland_surface->window_id);

  view->mapped = true;
  view->title_dirty = true;
  view->xwayland_surface = xsurface;
  view->surface = xsurface->surface;

  damage_whole(view->server);

  if (xsurface->surface) {
    xwayland_view->commit.notify = xwayland_surface_commit;
    wl_signal_add(&xsurface->surface->events.commit, &xwayland_view->commit);
  }

  if (view->override_redirect) {
    view->x = xsurface->x;
    view->y = xsurface->y;

    if (!view->parent && view->xwayland_surface->parent) {

      // root
      struct wlr_xwayland_surface *root = view->xwayland_surface->parent;
      while (root->parent) {
        root = root->parent;
      }

      struct tbx_view *ancestor;
      wl_list_for_each (ancestor, &server->views, link) {
        if (root->window_id == ancestor->xwayland_surface->window_id) {
          view->x += ancestor->x;
          view->y += ancestor->y;
          view->parent = ancestor;
          break;
        }
      }
    }

    view_raise(view);
    view_set_focus(view, view->surface);
    return;
  }

  view_set_focus(view, view->surface);

  // always set to zero
  wlr_xwayland_surface_configure(
      view->xwayland_surface, 0, 0, view->width, view->height);

  if (!view->parent) {
    view_move_to_center(view, NULL);
  }
}

static void
xwayland_surface_unmap(struct wl_listener *listener, void *data)
{
  console_log("unmap");
  struct tbx_xwayland_view *xwayland_view =
      wl_container_of(listener, xwayland_view, unmap);
  struct tbx_view *view = &xwayland_view->view;
  view->surface = NULL;
  view->mapped = false;

  damage_whole(view->server);

  if (view->xwayland_surface->surface) {
    wl_list_remove(&xwayland_view->commit.link);
  }
}

static void
xwayland_request_configure(struct wl_listener *listener, void *data)
{
  console_log("request configure");
  struct tbx_xwayland_view *xwayland_view =
      wl_container_of(listener, xwayland_view, request_configure);
  struct tbx_view *view = &xwayland_view->view;

  struct wlr_xwayland_surface_configure_event *ev = data;
  struct wlr_xwayland_surface *xsurface = view->xwayland_surface;

  if (!xsurface->mapped) {
    wlr_xwayland_surface_configure(
        xsurface, ev->x, ev->y, ev->width, ev->height);
  }

  view->x = ev->x;
  view->y = ev->y;
  view->width = ev->width;
  view->height = ev->height;
}

static void
xwayland_request_fullscreen(struct wl_listener *listener, void *data)
{
  struct tbx_xwayland_view *xwayland_view =
      wl_container_of(listener, xwayland_view, request_fullscreen);
  struct tbx_view *view = &xwayland_view->view;
  view->interface->set_fullscreen(view, !view->fullscreen);

  damage_whole(view->server);
}

static void
xwayland_set_title(struct wl_listener *listener, void *data)
{
  struct tbx_xwayland_view *xview = wl_container_of(listener, xview, set_title);
  struct tbx_view *view = &xview->view;
  view->title_dirty = true;

  damage_whole(view->server);
}

static void
xwayland_set_class(struct wl_listener *listener, void *data)
{
  struct tbx_xwayland_view *xview = wl_container_of(listener, xview, set_class);
  struct tbx_view *view = &xview->view;

  console_log("set_class %s", xwayland_get_string_prop(view, VIEW_PROP_CLASS));
}

static void
xwayland_set_role(struct wl_listener *listener, void *data)
{
  struct tbx_xwayland_view *xview = wl_container_of(listener, xview, set_role);
  struct tbx_view *view = &xview->view;

  console_log(
      "set_role %s", xwayland_get_string_prop(view, VIEW_PROP_WINDOW_ROLE));
}

static void
xwayland_set_window_type(struct wl_listener *listener, void *data)
{
  struct tbx_xwayland_view *xview =
      wl_container_of(listener, xview, set_window_type);
  struct tbx_view *view = &xview->view;

  int wt = xwayland_get_int_prop(view, VIEW_PROP_WINDOW_TYPE);
  
  console_log("set_window_type %d", wt);
  // for(size_t i=0; i<view->xwayland_surface->window_type_len; i++) {
  //     console_log("window_type %d", view->xwayland_surface->window_type[i]);
  // }
}

static void
new_xwayland_surface(struct wl_listener *listener, void *data)
{
  struct wlr_xwayland_surface *xsurface = data;

  struct tbx_xwayland_shell *xwayland_shell =
      wl_container_of(listener, xwayland_shell, new_xwayland_surface);
  struct tbx_server *server = xwayland_shell->server;

  // console_log("new surface");

  /* Allocate a tbx_view for this surface */
  struct tbx_xwayland_view *xwayland_view =
      calloc(1, sizeof(struct tbx_xwayland_view));
  struct tbx_view *view = &xwayland_view->view;
  view->view_type = VIEW_TYPE_XWAYLAND;
  view->interface = &xwayland_view_interface;

  view->xwayland_surface = xsurface;
  view->server = server;

  /* Listen to the various events it can emit */
  xwayland_view->map.notify = xwayland_surface_map;
  wl_signal_add(&xsurface->events.map, &xwayland_view->map);

  xwayland_view->unmap.notify = xwayland_surface_unmap;
  wl_signal_add(&xsurface->events.unmap, &xwayland_view->unmap);

  xwayland_view->destroy.notify = xwayland_surface_destroy;
  wl_signal_add(&xsurface->events.destroy, &xwayland_view->destroy);

  xwayland_view->request_configure.notify = xwayland_request_configure;
  wl_signal_add(
      &xsurface->events.request_configure, &xwayland_view->request_configure);

  xwayland_view->request_fullscreen.notify = xwayland_request_fullscreen;
  wl_signal_add(
      &xsurface->events.request_fullscreen, &xwayland_view->request_fullscreen);

  xwayland_view->set_title.notify = xwayland_set_title;
  wl_signal_add(&xsurface->events.set_title, &xwayland_view->set_title);

  xwayland_view->set_class.notify = xwayland_set_class;
  wl_signal_add(&xsurface->events.set_class, &xwayland_view->set_class);

  xwayland_view->set_role.notify = xwayland_set_role;
  wl_signal_add(&xsurface->events.set_role, &xwayland_view->set_role);

  xwayland_view->set_window_type.notify = xwayland_set_window_type;
  wl_signal_add(
      &xsurface->events.set_window_type, &xwayland_view->set_window_type);

  // move to workspace
  view->workspace = server->workspace;
  view->x = xsurface->x;
  view->y = xsurface->y;

  if (xsurface->override_redirect) {
    view->csd = true; // implement decoration listener
    view->override_redirect = true;
  }

  /* Add it to the list of views. */
  wl_list_insert(&server->views, &view->link);
  view_setup(view);

  console_log(">new xwayland_view %s\n",
      view->override_redirect ? "override_redirect" : "");
}

void
handle_xwayland_ready(struct wl_listener *listener, void *data)
{
  struct tbx_xwayland_shell *xwayland =
      wl_container_of(listener, xwayland, xwayland_ready);
  // struct tbx_server *server = xwayland->server;

  xcb_connection_t *xcb_conn = xcb_connect(NULL, NULL);
  int err = xcb_connection_has_error(xcb_conn);
  if (err) {
    console_log("XCB connect failed: %d", err);
    return;
  }

  xcb_intern_atom_cookie_t cookies[ATOM_LAST];
  for (size_t i = 0; i < ATOM_LAST; i++) {
    cookies[i] = xcb_intern_atom(xcb_conn, 0, strlen(atom_map[i]), atom_map[i]);
  }
  for (size_t i = 0; i < ATOM_LAST; i++) {
    xcb_generic_error_t *error = NULL;
    xcb_intern_atom_reply_t *reply =
        xcb_intern_atom_reply(xcb_conn, cookies[i], &error);
    if (reply != NULL && error == NULL) {
      xwayland->atoms[i] = reply->atom;
    }
    free(reply);

    if (error != NULL) {
      free(error);
      break;
    }
  }

  xcb_disconnect(xcb_conn);
}

bool
xwayland_shell_setup(struct tbx_server *server)
{
  server->xwayland_shell = calloc(1, sizeof(struct tbx_xwayland_shell));
  server->xwayland_shell->server = server;

  server->xwayland_shell->wlr_xwayland =
      wlr_xwayland_create(server->wl_display, server->compositor, true);
  // config->xwayland == XWAYLAND_MODE_LAZY);

  server->xwayland_shell->new_xwayland_surface.notify = new_xwayland_surface;
  wl_signal_add(&server->xwayland_shell->wlr_xwayland->events.new_surface,
      &server->xwayland_shell->new_xwayland_surface);

  server->xwayland_shell->xwayland_ready.notify = handle_xwayland_ready;
  wl_signal_add(&server->xwayland_shell->wlr_xwayland->events.new_surface,
      &server->xwayland_shell->xwayland_ready);

  setenv("DISPLAY", server->xwayland_shell->wlr_xwayland->display_name, true);
  return true;
}
