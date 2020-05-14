#ifndef TINYBOX_WORKSPACE_H
#define TINYBOX_WORKSPACE_H

#include "tinybox/server.h"

#define MAX_WORKSPACES 8

struct tbx_view;

struct tbx_workspace {
  int id;
  char *name;
  char *background;
  struct wlr_texture *background_texture;
  struct wl_list link;

  uint32_t background_color;

  // state
  bool active;
  int x;
  int y;
};

void workspace_setup(struct tbx_server *server);
void move_to_workspace(struct tbx_server *server, struct tbx_view *view, int id);
void activate_workspace(struct tbx_server *server, int id);

#endif // TINYBOX_WORKSPACE_H