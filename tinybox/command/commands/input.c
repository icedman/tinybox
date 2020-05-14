#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void exec_input(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  cmd->data = 0;
  strip_quotes(argv[0]);

  struct tbx_server *server = cmd->server;

  // find input
  struct tbx_config_entry *entry;
  wl_list_for_each(entry, &server->config.input, link) {
    if (entry->identifier && strcmp(entry->identifier, argv[0]) == 0) {
      cmd->data = (struct tbx_input_config *)entry;
      return;
    }
  }

  entry = calloc(1, sizeof(struct tbx_config_input));
  entry->identifier = calloc(1, strlen(argv[0]) + 1);
  strcpy(entry->identifier, argv[0]);
  entry->type = TBX_CONFIG_INPUT;
  cmd->data = entry;

  wl_list_insert(&server->config.input, &entry->link);
}

static void exec_tap(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config_input *entry = cmd->context->data;
  if (entry) {
    entry->tap = parse_boolean(argv[0], false);
  }
}

static void exec_natural_scroll(struct tbx_command *cmd, int argc,
                                char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config_input *entry = cmd->context->data;
  if (entry) {
    entry->natural_scroll = parse_boolean(argv[0], false);
  }
}

void register_input_commands(struct tbx_server *server) {
  struct tbx_command *cmd =
      register_command(server->command, "input", exec_input);
  register_command(cmd, "tap", exec_tap);
  register_command(cmd, "natural_scroll", exec_natural_scroll);
}