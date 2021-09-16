#include "tinybox/damage.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/shell.h"
#include "tinybox/view.h"

#include <float.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

/*
/ void (*for_each_surface)(struct tbx_view *view,
//     wlr_surface_iterator_func_t iterator, void *user_data);
// void (*for_each_popup)(struct tbx_view *view,
//     wlr_surface_iterator_func_t iterator, void *user_data);
*/

static void
xdg_new_popup(struct wl_listener *listener, void *data)
{
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, new_popup);
  // struct tbx_view *view = &xdg_shell_view->view;
  // damage_whole(view->server);
  console_log("new popup!");
}

static void
xdg_get_constraints(struct tbx_view *view,
    double *min_width,
    double *max_width,
    double *min_height,
    double *max_height)
{
  struct wlr_xdg_toplevel_state *state = &view->xdg_surface->toplevel->current;
  *min_width = state->min_width > 0 ? state->min_width : DBL_MIN;
  *max_width = state->max_width > 0 ? state->max_width : DBL_MAX;
  *min_height = state->min_height > 0 ? state->min_height : DBL_MIN;
  *max_height = state->max_height > 0 ? state->max_height : DBL_MAX;
}

static void
xdg_get_geometry(struct tbx_view *view, struct wlr_box *box)
{
  if (view->surface) {
    wlr_xdg_surface_get_geometry(view->xdg_surface, box);
    box->x = box->y = 0;
  } else {
    box->width = 0;
    box->height = 0;
  }
}

static void
xdg_set_activated(struct tbx_view *view, bool activated)
{
  console_log("xdg activated %d\n", view->identifier);
  struct wlr_seat *seat = view->server->seat->seat;
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);

  if (!view->xdg_surface || !view->xdg_surface->surface) {
    return;
  }

  /*
   * Tell the seat to have the keyboard enter this surface. wlroots will keep
   * track of this and automatically send key events to the appropriate
   * clients without additional work on your part.
   */
  if (activated) {
    wlr_seat_keyboard_notify_enter(seat,
        view->xdg_surface->surface,
        keyboard->keycodes,
        keyboard->num_keycodes,
        &keyboard->modifiers);
  }

  wlr_xdg_toplevel_set_activated(view->xdg_surface, activated);
}

static const char *
xdg_get_string_prop(struct tbx_view *view, enum tbx_view_prop prop)
{
  switch (prop) {
    case VIEW_PROP_TITLE:
      return view->xdg_surface->toplevel->title;
    case VIEW_PROP_APP_ID:
      return view->xdg_surface->toplevel->app_id;
    default:
      return NULL;
  }
}

uint32_t
xdg_get_int_prop(struct tbx_view *view, enum tbx_view_prop prop)
{
  return 0;
}

static uint32_t
xdg_view_configure(
    struct tbx_view *view, double lx, double ly, int width, int height)
{
  struct wlr_box box;
  wlr_xdg_surface_get_geometry(view->xdg_surface, &box);

  view->x = lx - box.x;
  view->y = ly - box.y;

  if (width != box.width) {
    view->title_dirty = true;
  }

  wlr_xdg_toplevel_set_size(view->xdg_surface, width, height);
  view->request_box.x = view->x;
  view->request_box.y = view->y;
  view->request_box.width = width;
  view->request_box.height = height;

  damage_whole(view->server);
  return 0;
}

static void
xdg_set_fullscreen(struct tbx_view *view, bool fullscreen)
{
  if (!view->xdg_surface) {
    return;
  }

  console_log("xdg fullscreen %d", fullscreen);

  if (fullscreen) {
    view->fullscreen = fullscreen;
    view->restore.x = view->x;
    view->restore.y = view->y;

    struct wlr_box window_box;
    xdg_get_geometry(view, &window_box);
    view->restore.width = window_box.width;
    view->restore.height = window_box.height;

    // todo get output from view
    struct wlr_box *full_box = wlr_output_layout_get_box(
        view->server->output_layout, view->server->main_output->wlr_output);

    view->x = 0;
    view->y = 0;
    view->width = full_box->width;
    view->height = full_box->height;
    xdg_view_configure(view, 0, 0, full_box->width, full_box->height);
  } else {
    view->fullscreen = fullscreen;
    view->x = view->restore.x;
    view->y = view->restore.y;
    view->width = view->restore.width;
    view->height = view->restore.height;
    xdg_view_configure(view, 0, 0, view->restore.width, view->restore.height);
  }

  // wlr_xdg_furface_set_fullscreen(view->xdg_furface, fullscreen);
}

static void
xdg_close(struct tbx_view *view)
{
  struct wlr_xdg_surface *surface = view->xdg_surface;
  if (surface->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL && surface->toplevel) {
    wlr_xdg_toplevel_send_close(surface);
  }
}

static void
xdg_close_popups(struct tbx_view *view)
{}

static void
xdg_destroy(struct tbx_view *view)
{
  struct tbx_xdg_shell_view *xview = (struct tbx_xdg_shell_view *)view;

  struct wl_listener *l = &xview->_first;
  while (++l) {
    if (!l->link.prev || l == &xview->destroy) {
      break;
    }
    // console_log("xdg unlisten");
    wl_list_remove(&l->link);
  }

  view_destroy(view);
}

static struct tbx_view_interface xdg_view_interface = { .get_constraints =
                                                            xdg_get_constraints,
  .get_geometry = xdg_get_geometry,
  .get_string_prop = xdg_get_string_prop,
  .get_int_prop = xdg_get_int_prop,
  .configure = xdg_view_configure,
  .set_activated = xdg_set_activated,
  .set_fullscreen = xdg_set_fullscreen,
  .close = xdg_close,
  .close_popups = xdg_close_popups,
  .destroy = xdg_destroy };

static void
begin_interactive(
    struct tbx_view *view, enum tbx_cursor_mode mode, uint32_t edges)
{
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

static void
xdg_surface_commit(struct wl_listener *listener, void *data)
{
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, commit);
  struct tbx_view *view = &xdg_shell_view->view;
  damage_add_commit(view->server, view);
}

static void
xdg_surface_map(struct wl_listener *listener, void *data)
{
  /* Called when the surface is mapped, or ready to display on-screen. */
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, map);
  struct tbx_view *view = &xdg_shell_view->view;
  view->mapped = true;
  view->title_dirty = true;
  view->surface = (struct wlr_surface *)view->xdg_surface;
  view->hotspot = HS_NONE;

  view_set_focus(view, view->xdg_surface->surface);
  view_move_to_center(view, NULL);

  damage_whole(view->server);

  if (view->xdg_surface->surface) {
    xdg_shell_view->commit.notify = xdg_surface_commit;
    wl_signal_add(
      &view->xdg_surface->surface->events.commit,
      &xdg_shell_view->commit);
  }
}

static void
xdg_surface_unmap(struct wl_listener *listener, void *data)
{
  /* Called when the surface is unmapped, and should no longer be shown. */
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, unmap);
  struct tbx_view *view = &xdg_shell_view->view;
  view->surface = NULL;
  view->mapped = false;

  damage_whole(view->server);

  if (view->xdg_surface->surface) {
    wl_list_remove(&xdg_shell_view->commit.link);
  }
}

static void
xdg_surface_destroy(struct wl_listener *listener, void *data)
{
  /* Called when the surface is destroyed and should never be shown again. */
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, destroy);
  struct tbx_view *view = &xdg_shell_view->view;
  view->interface->destroy(view);
}

static void
xdg_toplevel_request_move(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, request_move);
  struct tbx_view *view = &xdg_shell_view->view;
  begin_interactive(view, TBX_CURSOR_MOVE, 0);
}

static void
xdg_toplevel_request_fullscreen(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, request_move);
  struct tbx_view *view = &xdg_shell_view->view;
  xdg_set_fullscreen(view, !view->fullscreen);
}

static void
xdg_toplevel_request_resize(struct wl_listener *listener, void *data)
{
  /* This event is raised when a client would like to begin an interactive
   * resize, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
  struct wlr_xdg_toplevel_resize_event *event = data;
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, request_resize);
  struct tbx_view *view = &xdg_shell_view->view;
  begin_interactive(view, TBX_CURSOR_RESIZE, event->edges);
}

static void
xdg_set_title(struct wl_listener *listener, void *data)
{
  struct tbx_xdg_shell_view *xdg_shell_view =
      wl_container_of(listener, xdg_shell_view, set_title);
  struct tbx_view *view = &xdg_shell_view->view;
  view->title_dirty = true;
  damage_whole(view->server);
}

static void
server_new_xdg_surface(struct wl_listener *listener, void *data)
{

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
  struct tbx_xdg_shell_view *xdg_shell_view =
      calloc(1, sizeof(struct tbx_xdg_shell_view));
  struct tbx_view *view = &xdg_shell_view->view;
  view->view_type = VIEW_TYPE_XDG;
  view->interface = &xdg_view_interface;

  view->xdg_surface = xdg_surface;
  view->server = server;

  /* Listen to the various events it can emit */
  xdg_shell_view->map.notify = xdg_surface_map;
  wl_signal_add(&xdg_surface->events.map, &xdg_shell_view->map);
  xdg_shell_view->unmap.notify = xdg_surface_unmap;
  wl_signal_add(&xdg_surface->events.unmap, &xdg_shell_view->unmap);
  xdg_shell_view->destroy.notify = xdg_surface_destroy;
  wl_signal_add(&xdg_surface->events.destroy, &xdg_shell_view->destroy);

  struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
  xdg_shell_view->request_move.notify = xdg_toplevel_request_move;
  wl_signal_add(&toplevel->events.request_move, &xdg_shell_view->request_move);
  xdg_shell_view->request_resize.notify = xdg_toplevel_request_resize;
  wl_signal_add(
      &toplevel->events.request_resize, &xdg_shell_view->request_resize);
  xdg_shell_view->request_fullscreen.notify = xdg_toplevel_request_fullscreen;
  wl_signal_add(&toplevel->events.request_fullscreen,
      &xdg_shell_view->request_fullscreen);

  // title
  xdg_shell_view->set_title.notify = xdg_set_title;
  wl_signal_add(&toplevel->events.set_title, &xdg_shell_view->set_title);

  // popup
  xdg_shell_view->new_popup.notify = xdg_new_popup;
  wl_signal_add(&xdg_surface->events.new_popup, &xdg_shell_view->new_popup);

  // move to workspace
  view->workspace = server->workspace;

  xdg_set_activated(view, true);

  /* Add it to the list of views. */
  wl_list_insert(&server->views, &view->link);
  view_setup(view);

  console_log(">new xdg_view %d\n", view->identifier);
}

bool
xdg_shell_setup(struct tbx_server *server)
{
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
  wl_signal_add(&server->xdg_shell->wlr_xdg_shell->events.new_surface,
      &server->xdg_shell->new_xdg_surface);
  return true;
}
