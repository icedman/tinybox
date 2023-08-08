#ifndef TINYBOX_SHELL_H
#define TINYBOX_SHELL_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_server;

bool
tbx_xdg_shell_setup(struct tbx_server *server);
bool
tbx_xwayland_setup(struct tbx_server *server);

#endif // TINYBOX_SHELL_H