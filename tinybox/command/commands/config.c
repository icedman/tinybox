#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void exec_workspaces(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->workspaces = strtol(argv[0], NULL, 10);
  // console_log("cfg workspaces %d", config->workspaces);
}

static void exec_animate(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->animate = parse_boolean(argv[0], false);
  // console_log("cfg animate %d", config->animate);
}

static void exec_swipe_threshold(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->swipe_threshold = strtol(argv[0], NULL, 10);
  if (config->swipe_threshold < 0) {
    config->swipe_threshold = 0;
  }
  if (config->swipe_threshold > 200) {
    config->swipe_threshold = 200;
  }
}

void register_config_commands(struct tbx_server *server) {
  register_command(server->command, "workspaces", exec_workspaces);
  register_command(server->command, "animate", exec_animate);
  register_command(server->command, "swipe_threshold", exec_swipe_threshold);
}