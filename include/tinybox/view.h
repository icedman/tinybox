#ifndef TINYBOX_DESKTOP_H
#define TINYBOX_DESKTOP_H

#include "tinybox/server.h"

enum tbx_view_hotspot {
  HS_GRIP_LEFT,
  HS_GRIP_RIGHT,
  HS_EDGE_TOP,
  HS_EDGE_BOTTOM,
  HS_EDGE_LEFT,
  HS_EDGE_RIGHT,
  HS_TITLEBAR,
  HS_HANDLE,
  HS_COUNT,
  HS_NONE = -1
};

enum tbx_view_prop { VIEW_PROP_APP_ID, VIEW_PROP_TITLE };

struct tbx_view {
  struct wl_list link;
  struct tbx_server *server;

  struct wlr_surface *surface; // NULL for unmapped views
  struct wlr_xdg_surface *xdg_surface;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  struct wl_listener set_title;

  bool mapped;
  int x, y;

  bool shaded;
  bool csd;

  // title
  struct wlr_texture *title;
  struct wlr_texture *title_unfocused;
  struct wlr_box title_box;
  bool title_dirty;

  // hotspots
  struct wlr_box hotspots[HS_COUNT];
  enum tbx_view_hotspot hotspot;
  uint32_t hotspot_edges;

  struct wlr_box request_box;
  int request_wait;
};

struct tbx_view *desktop_view_at(struct tbx_server *server, double lx,
                                 double ly, struct wlr_surface **surface,
                                 double *sx, double *sy);

bool view_at(struct tbx_view *view, double lx, double ly,
             struct wlr_surface **surface, double *sx, double *sy);

void focus_view(struct tbx_view *view, struct wlr_surface *surface);

bool hotspot_at(struct tbx_view *view, double lx, double ly,
                struct wlr_surface **surface, double *sx, double *sy);

// create is at xdg_shell
void view_destroy(struct tbx_view *view);

const char *get_string_prop(struct tbx_view *view, enum tbx_view_prop prop);

#endif // TINYBOX_DESKTOP_H