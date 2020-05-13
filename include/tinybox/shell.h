#ifndef TINYBOX_SHELL_H
#define TINYBOX_SHELL_H

#include <stdbool.h>

struct tbx_xdg_shell {
  struct wlr_xdg_shell *wlr_xdg_shell;
  struct wl_listener new_xdg_surface;
  struct tbx_server *server;

  int create_offset;
};

bool xdg_shell_setup(struct tbx_server *server);

#endif // TINYBOX_SHELL_H