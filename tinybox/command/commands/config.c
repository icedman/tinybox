#include "tinybox/config.h"
#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void exec_set(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 2)) {
    return;
  }

  cmd->data = 0;
  strip_quotes(argv[0]);
  strip_quotes(argv[1]);

  struct tbx_server *server = cmd->server;

  // find dictionary
  struct tbx_config_dictionary *entry;
  wl_list_for_each(entry, &server->config.dictionary, link) {
    if (entry->identifier && strcmp(entry->identifier, argv[0]) == 0) {
      cmd->data = (struct tbx_dictionary_config *)entry;
      return;
    }
  }

  entry = calloc(1, sizeof(struct tbx_config_dictionary));
  entry->identifier = calloc(strlen(argv[0]) + 1, sizeof(char));
  strcpy(entry->identifier, argv[0]);
  entry->value = calloc(strlen(argv[1]) + 1, sizeof(char));
  strcpy(entry->value, argv[1]);
  entry->type = TBX_CONFIG_DICTIONARY;

  // console_log("set %s :\"%s\"", argv[0], argv[1]);

  cmd->data = entry;
  wl_list_insert(&server->config.dictionary, &entry->link);
}

static void exec_workspaces(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->workspaces = strtol(argv[0], NULL, 10);
}

static void exec_animate(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->animate = parse_boolean(argv[0], false);
}

static void exec_swipe_threshold(struct tbx_command *cmd, int argc,
                                 char **argv) {
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
  register_command(server->command, "set", exec_set);
  register_command(server->command, "workspaces", exec_workspaces);
  register_command(server->command, "animate", exec_animate);
  register_command(server->command, "swipe_threshold", exec_swipe_threshold);
}