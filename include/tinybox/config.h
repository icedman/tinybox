#ifndef TINYBOX_CONFIG_H
#define TINYBOX_CONFIG_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_server;

enum config_entry_type { CONFIG_INPUT, CONFIG_LAYOUT };

struct tbx_config_entry {
  char *identifier;
  enum config_entry_type type;
  struct wl_list link;
};

struct tbx_config_input {
  char *identifier;
  enum config_entry_type type;
  struct wl_list link;

  int tap;
  int natural_scroll;
};

struct tbx_config_layout {
  char *identifier;
  enum config_entry_type type;
  struct wl_list link;

  int x;
  int y;
  int width;
  int height;
};

struct tbx_config {
  struct wl_list input;
  struct wl_list layout;

  int workspaces;
  bool animate;

  struct tbx_server *server;
};

bool config_setup(struct tbx_server *server);
void load_config(struct tbx_server *server, char *config);

#endif // TINYBOX_CONFIG_H