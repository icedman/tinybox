#include "tinybox/config.h"
#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

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
    activate_workspace(cmd->server, w, true);
  }
}

void exec_set_background(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_workspace *workspace = cmd->context->data;
  if (workspace) {
    console_log("set background %s", argv[0]);
  }
}

void exec_workspace_left(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 0)) {
    return;
  }

  if (!cmd->server->started) {
    return;
  }
  activate_workspace(cmd->server, cmd->server->workspace - 1, true);
}

void exec_workspace_right(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 0)) {
    return;
  }

  if (!cmd->server->started) {
    return;
  }
  activate_workspace(cmd->server, cmd->server->workspace + 1, true);
}

void register_desktop_commands(struct tbx_server *server) {
  struct tbx_command *wks = register_command(server->command, "workspace", exec_workspace);
  register_command(server->command, "workspace_left", exec_workspace_left);
  register_command(server->command, "workspace_right", exec_workspace_right);
  
  register_command(wks, "background", exec_set_background);
}