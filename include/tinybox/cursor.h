#ifndef TINYBOX_CURSOR_H
#define TINYBOX_CURSOR_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_server;

enum tbx_cursor_mode {
  TBX_CURSOR_PASSTHROUGH,
  TBX_CURSOR_MOVE,
  TBX_CURSOR_RESIZE,
};

bool
tbx_cursor_setup(struct tbx_server *server);

void
reset_cursor_mode(struct tbx_server *server);

#endif // TINYBOX_CURSOR_H