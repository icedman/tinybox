#include "tinybox/command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-server-core.h>

// context
void
register_config_commands(struct tbx_server *server);
void
register_style_commands(struct tbx_server *server);
void
register_input_commands(struct tbx_server *server);
void
register_output_commands(struct tbx_server *server);
void
register_keybinding_commands(struct tbx_server *server);
void
register_global_commands(struct tbx_server *server);
void
register_workspace_commands(struct tbx_server *server);
void
register_menu_commands(struct tbx_server *server);

// global command
static char merged_args[512];
const char *
command_merge_args(struct tbx_server *server, int argc, char **argv)
{
  char *ptr = merged_args;
  for (int i = 0; i < argc; i++) {
    char *n = argv[i];

    if (server && n[0] == '$') {
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

  return (const char *)merged_args;
}

static struct tbx_command *
context_create(struct tbx_server *server)
{
  struct tbx_command *cmd = calloc(1, sizeof(struct tbx_command));

  wl_list_init(&cmd->commands);
  cmd->server = server;
  cmd->identifier = 0;
  cmd->exec = 0;
  cmd->context = 0;
  return cmd;
}

struct tbx_command *
register_command(struct tbx_command *context, char *name, tbx_exec *exec)
{
  struct tbx_command *cmd = context_create(context->server);
  cmd->exec = exec;
  cmd->identifier = calloc(strlen(name) + 1, sizeof(char));
  cmd->context = context;
  strcpy(cmd->identifier, name);
  wl_list_insert(&context->commands, &cmd->link);

  return cmd;
}

bool
command_check_args(struct tbx_command *context, int argc, int min)
{
  if (argc < min) {
    console_log("%s %d arguments expected\n", context->identifier, min);
    return false;
  }

  return true;
}

void
command_unhandled(struct tbx_command *context, char *cmd)
{
  printf("%s unhandled\n", cmd);
}

struct tbx_command *
command_execute(struct tbx_command *context, int argc, char **argv)
{
  if (argc < 1) {
    // empty line?
    return context;
  }

  struct tbx_command *cmd;
  wl_list_for_each (cmd, &context->commands, link) {
    if (strcmp(cmd->identifier, argv[0]) == 0) {
      if (cmd->exec) {
        cmd->exec(cmd, argc - 1, &argv[1]);
      }
      if (wl_list_length(&cmd->commands) > 0 || cmd->exec_custom) {
        return cmd;
      }
      break;
    }
  }

  if (context->exec_custom) {
    context->exec_custom(context, argc, argv);
    return context;
  }

  // can't be more elagant than this
  if (strcmp("}", argv[0]) == 0) {
    return context->context;
  }

  return context;
}

void
command_setup(struct tbx_server *server)
{
  server->command = context_create(server);
  register_config_commands(server);
  register_style_commands(server);
  register_input_commands(server);
  register_output_commands(server);
  register_keybinding_commands(server);
  register_global_commands(server);
  register_workspace_commands(server);
  register_menu_commands(server);
}