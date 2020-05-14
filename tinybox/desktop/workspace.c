#include "tinybox/workspace.h"
#include "tinybox/view.h"

#include <stdlib.h>

static struct tbx_workspace *create_workspace(struct tbx_server *server) {
  struct tbx_workspace *ws = calloc(1, sizeof(struct tbx_workspace));
  wl_list_insert(&server->workspaces, &ws->link);
  return ws;
}

void workspace_setup(struct tbx_server *server) {
  struct tbx_config *config = &server->config;
  config->server = server;
  server->workspace = 0;

  wl_list_init(&server->workspaces);
  int i = 0;
  for (; i < MAX_WORKSPACES; i++) {
    struct tbx_workspace *ws = create_workspace(server);
    ws->id = i;
    if (i >= config->workspaces) {
      break;
    }
  }

  config->workspaces = i;
  console_log("workspaces %d", config->workspaces);
}

void move_to_workspace(struct tbx_server *server, struct tbx_view *view, int id)
{
  if (!view) {
    return;
  }
  struct tbx_config *config = &server->config;
  if (id < 0) {
    id = 0;
  }
  if (id >= config->workspaces) {
    id = config->workspaces-1;
  }
  view->workspace = id;
  console_log("view at ws %d", view->workspace);
}

void activate_workspace(struct tbx_server *server, int id) {
  struct tbx_config *config = &server->config;
  if (id < 0) {
    id = 0;
  }
  if (id >= config->workspaces) {
    id = config->workspaces-1;
  }
  server->workspace = id;
  console_log("desktop at ws %d", server->workspace);
}
