#ifndef TBX_CURSOR_H
#define TBX_CURSOR_H

struct tbx_server;

enum tbx_cursor_mode {
  TBX_CURSOR_PASSTHROUGH,
  TBX_CURSOR_MOVE,
  TBX_CURSOR_RESIZE,
};

void cursor_init();
void cursor_attach(struct tbx_server *server, struct wlr_input_device *device);

#endif //  TBX_CURSOR_H