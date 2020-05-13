#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-server-core.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>

#include "tinybox/output.h"
#include "tinybox/server.h"

static struct tbx_config_layout *
get_output_layout_config(struct tbx_output *output) {

  struct tbx_server *server = output->server;
  struct tbx_config_layout *cfg;
  char *identifier = output->wlr_output->name;
  
  wl_list_for_each(cfg, &server->config.layout, link) {
    if (strcmp(cfg->identifier, identifier) == 0) {
      return cfg;
    }
  }
  return NULL;
}

void configure_output_layout(struct tbx_output *output) {
  struct tbx_config_layout *cfg = get_output_layout_config(output);
  struct tbx_server *server = output->server;
  if (!cfg) {
    wlr_output_layout_add_auto(server->output_layout, output->wlr_output);
    return;
  }

  console_log("apply output layout :%d %d %d %d", cfg->x, cfg->y, cfg->width,
              cfg->height);

  wlr_output_layout_add(server->output_layout, output->wlr_output, cfg->x,
                        cfg->y);
}
