#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void exec_bind(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 2)) {
    return;
  }

  cmd->data = 0;
  strip_quotes(argv[0]);

  struct tbx_server *server = cmd->server;

  // find keybinding
  struct tbx_config_keybinding *entry;
  wl_list_for_each(entry, &server->config.keybinding, link) {
    if (entry->identifier && strcmp(entry->identifier, argv[0]) == 0) {
      cmd->data = (struct tbx_keybinding_config *)entry;
      return;
    }
  }

  entry = calloc(1, sizeof(struct tbx_config_keybinding));
  entry->identifier = calloc(1, strlen(argv[0]) + 1);
  strcpy(entry->identifier, argv[0]);
  entry->type = TBX_CONFIG_KEYBINDING;

  // command
  console_log("bind key %s", argv[0]);

  cmd->data = entry;
  wl_list_insert(&server->config.keybinding, &entry->link);
}

void register_keybinding_commands(struct tbx_server *server) {
  // struct tbx_command *cmd =
      register_command(server->command, "bind", exec_bind);
  // register_command(cmd, "keys", exec_keys);
  // register_command(cmd, "command", exec_command);
}