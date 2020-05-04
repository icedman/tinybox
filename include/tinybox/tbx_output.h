#ifndef TBX_OUTPUT_H
#define TBX_OUTPUT_H

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
};

void init_output();

#endif //  TBX_OUTPUT_H