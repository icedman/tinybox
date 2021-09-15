#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/keyboard.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_keyboard.h>

static void
exec_bind(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 2)) {
    return;
  }

  cmd->data = 0;
  strip_quotes(argv[0]);

  int l = strlen(argv[0]);
  if (l > 255) {
    return;
  }

  struct tbx_server *server = cmd->server;

  // find keybinding if one already exists (can't replace)
  struct tbx_config_keybinding *entry;
  wl_list_for_each (entry, &server->config.keybinding, link) {
    if (entry->identifier && strcmp(entry->identifier, argv[0]) == 0) {
      cmd->data = (struct tbx_keybinding_config *)entry;
      return;
    }
  }

  entry = calloc(1, sizeof(struct tbx_config_keybinding));
  entry->identifier = calloc(strlen(argv[0]) + 1, sizeof(char));
  strcpy(entry->identifier, argv[0]);
  entry->type = TBX_CONFIG_KEYBINDING;

  //-------------------
  // extract keys
  //-------------------
  char keys[255];
  strcpy(keys, argv[0]);
  for (int i = 0; i < l; i++) {
    if (keys[i] == '+') {
      keys[i] = ' ';
    }
  }

  entry->keys = calloc(1, sizeof(struct tbx_keys_pressed));
  keys_clear(entry->keys);

  int kargc = 0;
  char **kargv = split_args(keys, &kargc);
  for (int i = 0; i < kargc; i++) {
    char *n = kargv[i];

    if (n[0] == '$') {
      n = config_dictionary_value(server, n);
    }
    if (!n) {
      n = argv[i];
    }

    keys_add_named(entry->keys, n);
  }

  free_argv(kargc, kargv);

  // console_log(">> bind %s?", entry->identifier);
  // keys_print(entry->keys);

  //-------------------
  // extract command
  //-------------------
  char command_line[512];
  char *ptr = command_line;
  for (int i = 1; i < argc; i++) {
    char *n = argv[i];

    if (n[0] == '$') {
      n = config_dictionary_value(server, n);
    }
    if (!n) {
      n = argv[i];
    }

    strcpy(ptr, n);
    ptr += strlen(n);
    ptr[0] = ' ';
    ptr[1] = 0;
    ptr++;
  }

  {
    int argc;
    char **argv = split_args(command_line, &argc);
    entry->argc = argc;
    entry->argv = argv;
  }

  cmd->data = entry;
  wl_list_insert(&server->config.keybinding, &entry->link);
}

void
register_keybinding_commands(struct tbx_server *server)
{
  // register_command(server->command, "bind", exec_bind);
  register_command(server->command, "bindsym", exec_bind);
}