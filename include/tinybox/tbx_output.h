#ifndef TBX_OUTPUT_H
#define TBX_OUTPUT_H

enum {
  HS_GRIP_LEFT,
  HS_GRIP_RIGHT,
  HS_EDGE_TOP,
  HS_EDGE_BOTTOM,
  HS_EDGE_LEFT,
  HS_EDGE_RIGHT,
  HS_TITLEBAR,
  HS_HANDLE,
  HS_COUNT
};

enum tbx_view_prop {
  VIEW_PROP_APP_ID,
  VIEW_PROP_TITLE
};

struct tbx_output {
  struct wl_list link;
  struct tbx_server *server;
  struct wlr_output *wlr_output;
  struct wl_listener frame;
  struct timespec last_render;
};

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

  // hotspots
  struct wlr_box hotspots[HS_COUNT];
  int hotspot;
  int hotspot_edges;

  // title
  struct wlr_texture *title;
  struct wlr_texture *title_unfocused;
  struct wlr_box title_box;
  bool title_dirty;

  // decoration
  // struct tbx_xdg_decoration *xdg_decoration;
  bool csd;
};

void output_init();

const char *get_string_prop(struct tbx_view *view,
    enum tbx_view_prop prop);

struct tbx_view *view_from_wlr_surface(struct wlr_surface *wlr_surface);

#endif //  TBX_OUTPUT_H