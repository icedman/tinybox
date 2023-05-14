#ifndef TINYBOX_SHELL_H
#define TINYBOX_SHELL_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_server;

struct tbx_shell {
  struct tbx_server *server;
  struct wlr_xdg_shell *wlr_shell;
  struct wl_listener new_shell_surface;
};

bool
tbx_xdg_shell_setup(struct tbx_server *server);
bool
tbx_xwayland_shell_setup(struct tbx_server *server);

#endif // TINYBOX_SHELL_H