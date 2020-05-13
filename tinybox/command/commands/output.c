#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void exec_output(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 6)) {
    console_log("expecting: output eDP-1 pos 0 0 res 1920 1080");
    return;
  }

  cmd->data = 0;
  strip_quotes(argv[0]);

  struct tbx_server *server = cmd->server;

  // find layout
  struct tbx_config_layout *entry;
  wl_list_for_each(entry, &server->config.layout, link) {
    if (entry->identifier && strcmp(entry->identifier, argv[0]) == 0) {
      cmd->data = (struct tbx_output_config *)entry;
      return;
    }
  }

  entry = calloc(1, sizeof(struct tbx_config_layout));
  entry->identifier = calloc(1, strlen(argv[0]) + 1);
  strcpy(entry->identifier, argv[0]);

  entry->x = strtol(argv[2], NULL, 10);
  entry->y = strtol(argv[3], NULL, 10);
  entry->width = strtol(argv[5], NULL, 10);
  entry->height = strtol(argv[6], NULL, 10);

  cmd->data = entry;

  wl_list_insert(&server->config.layout, &entry->link);
}

void register_output_commands(struct tbx_server *server) {
  // struct tbx_command *cmd =
  register_command(server->command, "output", exec_output);
}