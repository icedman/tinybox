#ifndef TINYBOX_VIEW_H
#define TINYBOX_VIEW_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_server;
struct wlr_surface;

enum tbx_view_type {
  TBX_XDG_SHELL,
  TBX_LAYER_SHELL,
  TBX_X11_MANAGED,
  TBX_X11_UNMANAGED
};


enum tbx_view_prop {
  VIEW_PROP_TITLE,
  VIEW_PROP_APP_ID,
  VIEW_PROP_CLASS,
  VIEW_PROP_INSTANCE,
  VIEW_PROP_WINDOW_TYPE,
  VIEW_PROP_WINDOW_ROLE,
  // #if HAVE_XWAYLAND
  VIEW_PROP_X11_WINDOW_ID,
  VIEW_PROP_X11_PARENT_ID,
  // #endif
};

struct wlr_box;
struct tbx_view;
struct tbx_view_interface {
  void (*get_constraints)(struct tbx_view *view,
      double *min_width,
      double *max_width,
      double *min_height,
      double *max_height);
  void (*get_geometry)(struct tbx_view *view, struct wlr_box *);
  const char *(*get_string_prop)(
      struct tbx_view *view, enum tbx_view_prop prop);
  uint32_t (*get_int_prop)(struct tbx_view *view, enum tbx_view_prop prop);
  uint32_t (*configure)(
      struct tbx_view *view, double lx, double ly, int width, int height);
  void (*set_activated)(struct tbx_view *view, struct wlr_surface *surface, bool activated);
  void (*set_fullscreen)(struct tbx_view *view, bool fullscreen);
  void (*close)(struct tbx_view *view);
  void (*close_popups)(struct tbx_view *view);
  void (*destroy)(struct tbx_view *view);
};

struct tbx_view {
  struct wl_list link;
  struct tbx_server *server;
  enum tbx_view_type type;
  struct wlr_scene_tree *scene_tree;

  double x, y;
  double width, height;
  struct {
    double x, y, width, height;
  } restore_box, request_box;
  bool fullscreen;

  union {
    struct wlr_xdg_surface *xdg;
    struct wlr_xwayland_surface *xwayland;
  } surface;
  struct wlr_xdg_toplevel *xdg_toplevel;

  struct wl_listener _first;
  struct wl_listener commit;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener request_maximize;
  struct wl_listener request_fullscreen;
#ifdef HAVE_XWAYLAND
  struct wl_listener activate;
  struct wl_listener configure;
  struct wl_listener set_hints;
#endif

  struct tbx_view_interface *interface;
};

struct tbx_view *
desktop_view_at(struct tbx_server *server,
    double lx,
    double ly,
    struct wlr_surface **surface,
    double *sx,
    double *sy);

#endif // TINYBOX_VIEW_H