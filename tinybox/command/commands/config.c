#include "tinybox/config.h"
#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/damage.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
exec_set(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 2)) {
    return;
  }

  cmd->data = 0;
  strip_quotes(argv[0]);
  strip_quotes(argv[1]);

  struct tbx_server *server = cmd->server;

  // find dictionary
  struct tbx_config_dictionary *entry;
  wl_list_for_each (entry, &server->config.dictionary, link) {
    if (entry->identifier && strcmp(entry->identifier, argv[0]) == 0) {
      cmd->data = (struct tbx_dictionary_config *)entry;
      return;
    }
  }

  entry = calloc(1, sizeof(struct tbx_config_dictionary));
  entry->identifier = calloc(strlen(argv[0]) + 1, sizeof(char));
  strcpy(entry->identifier, argv[0]);
  entry->type = TBX_CONFIG_DICTIONARY;

  // console_log("set %s :\"%s\"", argv[0], argv[1]);
#if 0
    char command_line[512];
    char* ptr = command_line;
    for (int i = 1; i < argc; i++) {
        char* n = argv[i];

        if (n[0] == '$') {
            n = config_dictionary_value(cmd->server, n);
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

    // console_log(">>>%s", argv[1]);
    // console_log(">>>%s", command_line);

    entry->value = calloc(strlen(command_line) + 1, sizeof(char));
    strcpy(entry->value, command_line);
#endif

  entry->value = calloc(strlen(argv[1]) + 1, sizeof(char));
  strcpy(entry->value, argv[1]);

  cmd->data = entry;
  wl_list_insert(&server->config.dictionary, &entry->link);
}

static void
exec_workspaces(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->workspaces = strtol(argv[0], NULL, 10);
}

static void
exec_animate(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->animate = parse_boolean(argv[0], false);
}

static void
exec_swipe_threshold(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->swipe_threshold = strtol(argv[0], NULL, 10);
  if (config->swipe_threshold < 0) {
    config->swipe_threshold = 0;
  }
  if (config->swipe_threshold > 500) {
    config->swipe_threshold = 500;
  }
}

static void
exec_move_resize_alpha(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  struct tbx_config *config = &cmd->server->config;
  config->move_resize_alpha = parse_float(argv[0]);
}

static void
exec_track_damages(struct tbx_command *cmd, int argc, char **argv)
{
  struct tbx_config *config = &cmd->server->config;
  if (argc) {
    config->track_damages = parse_boolean(argv[0], false);
  } else {
    config->track_damages = !config->track_damages;
  }
  if (config->track_damages) {
    console_log("tracking damages");
  }

  damage_whole(cmd->server);
}

static void
exec_debug_damages(struct tbx_command *cmd, int argc, char **argv)
{
  struct tbx_config *config = &cmd->server->config;
  if (argc) {
    config->debug_damages = parse_boolean(argv[0], false);
  } else {
    config->debug_damages = !config->debug_damages;
  }
  if (config->debug_damages) {
    console_log("rendering damage rects");
  }

  damage_whole(cmd->server);
}

static void
exec_console(struct tbx_command *cmd, int argc, char **argv)
{
  struct tbx_config *config = &cmd->server->config;
  if (argc) {
    config->console = parse_boolean(argv[0], false);
  } else {
    config->console = !config->console;
  }
  if (config->console) {
    console_clear();
    console_log("console enabled");
  }
}

static void
exec_font(struct tbx_command *cmd, int argc, char **argv)
{
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  strip_quotes(argv[0]);
  // struct tbx_config* config = &cmd->server->config;
  strcpy(cmd->server->style.font, argv[0]);

  console_log(">font %s", argv[0]);
}

static void
exec_show_tooltip(struct tbx_command *cmd, int argc, char **argv)
{
  struct tbx_config *config = &cmd->server->config;
  if (argc) {
    config->show_tooltip = parse_boolean(argv[0], false);
  } else {
    config->show_tooltip = !config->show_tooltip;
  }

  console_log("tooltip: %d", config->show_tooltip);
}

static void
exec_mini_titlebar(struct tbx_command *cmd, int argc, char **argv)
{
  struct tbx_config *config = &cmd->server->config;
  if (argc) {
    config->mini_titlebar = parse_boolean(argv[0], false);
  } else {
    config->mini_titlebar = !config->mini_titlebar;
  }
  damage_whole(cmd->server);
}

static void
exec_mini_frame(struct tbx_command *cmd, int argc, char **argv)
{
  struct tbx_config *config = &cmd->server->config;
  if (argc) {
    config->mini_frame = parse_boolean(argv[0], false);
  } else {
    config->mini_frame = !config->mini_frame;
  }
  damage_whole(cmd->server);
}

void
register_config_commands(struct tbx_server *server)
{
  register_command(server->command, "set", exec_set);
  register_command(server->command, "workspaces", exec_workspaces);
  register_command(server->command, "animate", exec_animate);
  register_command(server->command, "swipe_threshold", exec_swipe_threshold);
  register_command(server->command, "mini_titlebar", exec_mini_titlebar);
  register_command(server->command, "mini_frame", exec_mini_frame);
  register_command(server->command, "console", exec_console);
  register_command(server->command, "show_tooltip", exec_show_tooltip);
  register_command(server->command, "font", exec_font);
  register_command(
      server->command, "move_resize_alpha", exec_move_resize_alpha);
  register_command(server->command, "track_damages", exec_track_damages);
  register_command(server->command, "debug_damages", exec_debug_damages);
}