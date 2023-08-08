#define _POSIX_C_SOURCE 200112L
#include <tinybox/server.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/util/log.h>

#ifdef HAVE_XWAYLAND

#include <X11/Xlib.h>
#include <wlr/xwayland.h>
#include <xcb/xcb_icccm.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

enum {
  NetWMWindowTypeDialog,
  NetWMWindowTypeSplash,
  NetWMWindowTypeToolbar,
  NetWMWindowTypeUtility,
  NetLast
}; /* EWMH atoms */

static Atom netatom[NetLast];

Atom
getatom(xcb_connection_t *xc, const char *name)
{
  Atom atom = 0;
  xcb_intern_atom_reply_t *reply;
  xcb_intern_atom_cookie_t cookie = xcb_intern_atom(xc, 0, strlen(name), name);
  if ((reply = xcb_intern_atom_reply(xc, cookie, NULL)))
    atom = reply->atom;
  free(reply);

  return atom;
}

void
xwayland_ready(struct wl_listener *listener, void *data)
{
  struct tbx_server *server = wl_container_of(listener, server, xwayland_ready);
  struct wlr_xwayland *xwayland = server->xwayland;

  struct wlr_xcursor *xcursor;
  xcb_connection_t *xc = xcb_connect(xwayland->display_name, NULL);
  int err = xcb_connection_has_error(xc);
  if (err) {
    wlr_log(WLR_INFO,
        "xcb_connect to X server failed with code %d\n. Continuing with "
        "degraded functionality.\n",
        err);
    return;
  }

  /* Collect atoms we are interested in. If getatom returns 0, we will
   * not detect that window type. */
  netatom[NetWMWindowTypeDialog] = getatom(xc, "_NET_WM_WINDOW_TYPE_DIALOG");
  netatom[NetWMWindowTypeSplash] = getatom(xc, "_NET_WM_WINDOW_TYPE_SPLASH");
  netatom[NetWMWindowTypeToolbar] = getatom(xc, "_NET_WM_WINDOW_TYPE_TOOLBAR");
  netatom[NetWMWindowTypeUtility] = getatom(xc, "_NET_WM_WINDOW_TYPE_UTILITY");

  /* assign the one and only seat */
  wlr_xwayland_set_seat(xwayland, server->seat);

  /* Set the default XWayland cursor to match the rest of dwl. */
  if ((xcursor = wlr_xcursor_manager_get_xcursor(
           server->cursor_mgr, "left_ptr", 1))) {
    wlr_xwayland_set_cursor(xwayland,
        xcursor->images[0]->buffer,
        xcursor->images[0]->width * 4,
        xcursor->images[0]->width,
        xcursor->images[0]->height,
        xcursor->images[0]->hotspot_x,
        xcursor->images[0]->hotspot_y);
  }

  xcb_disconnect(xc);
}

static void
xwayland_map(struct wl_listener *listener, void *data)
{
  wlr_log(WLR_INFO, "xwayland_map");

  /* Called when the surface is mapped, or ready to display on-screen. */
  struct tbx_view *view = wl_container_of(listener, view, map);
  struct tbx_server *server = view->server;

  struct wlr_xwayland_surface *xsurface = view->surface.xwayland;

  view->scene_tree =
      wlr_scene_subsurface_tree_create(server->scene_views, xsurface->surface);
  view->scene_tree->node.data = view;
  xsurface->data = view->scene_tree;

  // /* Add it to the list of views. */
  wl_list_insert(&view->server->views, &view->link);
}

static void
xwayland_unmap(struct wl_listener *listener, void *data)
{
  wlr_log(WLR_INFO, "xwayland_unmap");

  /* Called when the surface is unmapped, and should no longer be shown.
   */
  struct tbx_view *view = wl_container_of(listener, view, unmap);

  wl_list_remove(&view->link);
}

static void
xwayland_destroy(struct wl_listener *listener, void *data)
{
  wlr_log(WLR_INFO, "xwayland_destroy");

  /* Called when the surface is destroyed and should never be shown again.
   */
  struct tbx_view *view = wl_container_of(listener, view, destroy);

  wl_list_remove(&view->map.link);
  wl_list_remove(&view->unmap.link);
  wl_list_remove(&view->destroy.link);
  // wl_list_remove(&view->request_move.link);
  // wl_list_remove(&view->request_resize.link);
  // wl_list_remove(&view->request_maximize.link);
  // wl_list_remove(&view->request_fullscreen.link);

  free(view);
}

static void
xwayland_configure(struct wl_listener *listener, void *data)
{
  struct tbx_view *view = wl_container_of(listener, view, configure);
  struct wlr_xwayland_surface *xsurface = view->surface.xwayland;
  struct wlr_xwayland_surface_configure_event *event = data;

  wlr_log(WLR_INFO, "xwayland_configure");

  // wlr_xwayland_surface_configure(xsurface,
  //     0, 0, event->width, event->height);

  // wlr_scene_node_set_position(&view->scene_tree->node,
  //     event->x * 0,
  //     event->y * 0);

  // fprintf(stderr, "%d %d\n", event->width, event->height);
}

static void
xwayland_activate(struct wl_listener *listener, void *data)
{
  struct tbx_view *view = wl_container_of(listener, view, activate);
  struct wlr_xwayland_surface *xsurface = view->surface.xwayland;

  wlr_log(WLR_INFO, "xwayland_activate");

  wlr_xwayland_surface_activate(xsurface, 1);
}

static void
xwayland_set_hints(struct wl_listener *listener, void *data)
{
  struct tbx_view *view = wl_container_of(listener, view, set_hints);
  struct wlr_xwayland_surface *xsurface = view->surface.xwayland;

  wlr_log(WLR_INFO, "xwayland_set_hints");
}

void
new_xwayland_surface(struct wl_listener *listener, void *data)
{
  struct tbx_server *server =
      wl_container_of(listener, server, new_xwayland_surface);
  struct wlr_xwayland *xwayland = server->xwayland;

  struct wlr_xwayland_surface *xsurface = data;
  wlr_log(WLR_INFO, "new_xwayland_surface");

  /* Allocate a tbx_view for this surface */
  struct tbx_view *view = calloc(1, sizeof(struct tbx_view));
  memset(view, 0, sizeof(struct tbx_view));
  view->server = server;
  view->type =
      xsurface->override_redirect ? TBX_X11_UNMANAGED : TBX_X11_MANAGED;
  view->surface.xwayland = xsurface;

  /* Listen to the various events it can emit */
  view->map.notify = xwayland_map;
  wl_signal_add(&xsurface->events.map, &view->map);
  view->unmap.notify = xwayland_unmap;
  wl_signal_add(&xsurface->events.unmap, &view->unmap);
  view->destroy.notify = xwayland_destroy;
  wl_signal_add(&xsurface->events.destroy, &view->destroy);

  view->configure.notify = xwayland_configure;
  wl_signal_add(&xsurface->events.map, &view->configure);
  view->activate.notify = xwayland_activate;
  wl_signal_add(&xsurface->events.map, &view->activate);
  view->set_hints.notify = xwayland_set_hints;
  wl_signal_add(&xsurface->events.map, &view->set_hints);
}

#endif

bool
tbx_xwayland_setup(struct tbx_server *server)
{
#ifdef HAVE_XWAYLAND
  server->xwayland =
      wlr_xwayland_create(server->wl_display, server->compositor, 1);
  if (server->xwayland) {
    server->xwayland_ready.notify = xwayland_ready;
    wl_signal_add(&server->xwayland->events.ready, &server->xwayland_ready);
    server->new_xwayland_surface.notify = new_xwayland_surface;
    wl_signal_add(
        &server->xwayland->events.new_surface, &server->new_xwayland_surface);
    setenv("DISPLAY", server->xwayland->display_name, 1);
  } else {
    wlr_log(
        WLR_INFO, "failed to setup XWayland X server, continuing without it");
  }
#endif
  return true;
}
