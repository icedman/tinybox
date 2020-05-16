#define _POSIX_C_SOURCE 200112L

#include "tinybox/xwayland.h"
#include "tinybox/view.h"
#include "tinybox/server.h"

#include <getopt.h>
#include <stdlib.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>

static void xwayland_surface_map(struct wl_listener *listener, void *data) {
  /* Called when the surface is mapped, or ready to display on-screen. */
  console_log("map");
  struct tbx_view *view = wl_container_of(listener, view, map);
  view->mapped = true;
  view->title_dirty = true;
  view->surface = (struct wlr_surface *)view->xwayland_surface;

  // focus_view(view, view->xdg_surface->surface);
}

static void xwayland_surface_unmap(struct wl_listener *listener, void *data) {
  /* Called when the surface is unmapped, and should no longer be shown. */
  struct tbx_view *view = wl_container_of(listener, view, unmap);
  console_log("unmap");
  view->surface = NULL;
  view->mapped = false;
}

static void xwayland_destroy(struct wl_listener *listener, void *data) {
  /* Called when the surface is destroyed and should never be shown again. */
  struct tbx_view *view = wl_container_of(listener, view, destroy);
  console_log("destroy");
  view_destroy(view);
}

static void new_xwayland_surface(struct wl_listener *listener, void *data) {
  struct wlr_xwayland_surface *xwayland_surface = data;

    if (xwayland_surface->override_redirect) {
        console_log("New xwayland unmanaged surface");
        // create_unmanaged(xsurface);
        return;
    }

  console_log("new surface");

  /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
  struct tbx_xwayland_shell *xwayland_shell =
      wl_container_of(listener, xwayland_shell, new_xwayland_surface);
  struct tbx_server *server = xwayland_shell->server;

  // if (xwayland_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
  //   return;
  // }

  /* Allocate a tbx_view for this surface */
  struct tbx_view *view = calloc(1, sizeof(struct tbx_view));
  view->xwayland_surface = xwayland_surface;
  view->server = server;

  xwayland_shell->create_offset = (xwayland_shell->create_offset + 1) % 8;
  view->x = 4 + (xwayland_shell->create_offset * 40);
  view->y = 32 + (xwayland_shell->create_offset * 40);

  /* Listen to the various events it can emit */
  view->map.notify = xwayland_surface_map;
  wl_signal_add(&xwayland_surface->events.map, &view->map);
  view->unmap.notify = xwayland_surface_unmap;
  wl_signal_add(&xwayland_surface->events.unmap, &view->unmap);
  view->destroy.notify = xwayland_destroy;
  wl_signal_add(&xwayland_surface->events.destroy, &view->destroy);

  /* cotd */
  /*
  struct wlr_xdg_toplevel *toplevel = xdg_surface->toplevel;
  view->request_move.notify = xdg_toplevel_request_move;
  wl_signal_add(&toplevel->events.request_move, &view->request_move);
  view->request_resize.notify = xdg_toplevel_request_resize;
  wl_signal_add(&toplevel->events.request_resize, &view->request_resize);
    */

  /* title */
  // view->set_title.notify = handle_set_title;
  // wl_signal_add(&toplevel->events.set_title, &view->set_title);

  // move to workspace
  view->workspace = server->workspace;

  /* Add it to the list of views. */
  wl_list_insert(&server->views, &view->link);
}

bool xwayland_shell_setup(struct tbx_server *server) {
    server->xwayland_shell = calloc(1, sizeof(struct tbx_xwayland_shell));
    server->xwayland_shell->server = server;
    
    server->xwayland_shell->wlr_xwayland =
        wlr_xwayland_create(server->wl_display, server->compositor, true);
                // config->xwayland == XWAYLAND_MODE_LAZY);
    
    server->xwayland_shell->new_xwayland_surface.notify = new_xwayland_surface;
    wl_signal_add(&server->xwayland_shell->wlr_xwayland->events.new_surface,
        &server->xwayland_shell->new_xwayland_surface);

    /*
    wl_signal_add(&server->xwayland.wlr_xwayland->events.ready,
        &server->xwayland_ready);
    server->xwayland_ready.notify = handle_xwayland_ready;
    */
    
    setenv("DISPLAY", server->xwayland_shell->wlr_xwayland->display_name, true);
    return true;
}