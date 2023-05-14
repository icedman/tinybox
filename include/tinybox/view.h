#ifndef TINYBOX_VIEW_H
#define TINYBOX_VIEW_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_server;
struct wlr_surface;

struct tbx_view {
  struct wl_list link;
  struct tbx_server *server;
  struct wlr_xdg_toplevel *xdg_toplevel;
  struct wlr_scene_tree *scene_tree;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;
  struct wl_listener request_fullscreen;
};

struct tbx_view *
desktop_view_at(struct tbx_server *server,
    double lx,
    double ly,
    struct wlr_surface **surface,
    double *sx,
    double *sy);

void
focus_view(struct tbx_view *view, struct wlr_surface *surface);

#endif // TINYBOX_VIEW_H