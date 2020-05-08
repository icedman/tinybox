#ifndef TBX_OUTPUT_H
#define TBX_OUTPUT_H

enum {
  HS_EDGE_TOP,
  HS_EDGE_BOTTOM,
  HS_EDGE_LEFT,
  HS_EDGE_RIGHT,
  HS_TITLEBAR,
  HS_HANDLE,
  HS_GRIP_LEFT,
  HS_GRIP_RIGHT,
  HS_COUNT
};

struct tbx_output {
  struct wl_list link;
  struct tbx_server *server;
  struct wlr_output *wlr_output;
  struct wl_listener frame;
};

struct tbx_view {
  struct wl_list link;
  struct tbx_server *server;
  struct wlr_xdg_surface *xdg_surface;
  struct wl_listener map;
  struct wl_listener unmap;
  struct wl_listener destroy;
  struct wl_listener request_move;
  struct wl_listener request_resize;
  bool mapped;
  int x, y;

  // hotspots
  struct wlr_box hotspots[HS_COUNT];
  int hotspot;
  int hotspot_edges;
};

void init_output();

#endif //  TBX_OUTPUT_H