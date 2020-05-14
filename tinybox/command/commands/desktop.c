#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"
#include "tinybox/view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tinybox/workspace.h"

void exec_workspace(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  int w = strtol(argv[0], NULL, 10);
  cmd->data = get_workspace(cmd->server, w);
  if (cmd->server->started) {

    if (strcmp(argv[0], "left") == 0) {
      w = cmd->server->workspace - 1;
    } else if (strcmp(argv[0], "right") == 0) {
      w = cmd->server->workspace + 1;
    }
    
    activate_workspace(cmd->server, w, true);
  }
}

void exec_set_background(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_workspace *workspace = cmd->context->data;
  if (workspace) {

    strip_quotes(argv[0]);

    char *expanded = calloc(1, sizeof(char) + (strlen(argv[0]) + 1));
    strcpy(expanded, argv[0]);
    expand_path(&expanded);

    workspace->background = expanded;
    console_log("set background %s", expanded);
  }
}

void exec_move_window_to_workspace(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 4)) {
    return;
  }

  if (!cmd->server->started) {
    return;
  }

  struct tbx_view *current_view =
      wl_container_of(cmd->server->views.next, current_view, link);

  if (!current_view) {
    return;
  }

  int w = 0;
  if (strcmp(argv[3], "left") == 0) {
    w = current_view->workspace - 1;
  } else if (strcmp(argv[3], "right") == 0) {
    w = current_view->workspace + 1;
  } else {
    w = strtol(argv[3], NULL, 10);
  }

  move_to_workspace(cmd->server, current_view, w, true);
}

void exec_shade_window(struct tbx_command *cmd, int argc, char **argv) {
  // if (!command_check_args(cmd, argc, 0)) {
  //   return;
  // }

  if (!cmd->server->started) {
    return;
  }

  struct tbx_view *current_view =
      wl_container_of(cmd->server->views.next, current_view, link);

  if (!current_view) {
    return;
  }

  bool shade = !current_view->shaded;
  if (argc && strcmp(argv[0], "up") == 0) {
    shade = true;
  }
  if (argc && strcmp(argv[0], "down") == 0) {
    shade = false;  
  }

  current_view->shaded = shade;
}

void register_desktop_commands(struct tbx_server *server) {
  struct tbx_command *wks =
      register_command(server->command, "workspace", exec_workspace);
    register_command(wks, "background", exec_set_background);
  register_command(server->command, "move",
                   exec_move_window_to_workspace);
  register_command(server->command, "shade",
                   exec_shade_window);
}