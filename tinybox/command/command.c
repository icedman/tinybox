#include "tinybox/command.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <wayland-server-core.h>

void register_config_commands(struct tbx_server *server);
void register_input_commands(struct tbx_server *server);
void register_output_commands(struct tbx_server *server);

static struct tbx_command *context_create(struct tbx_server *server) {
  struct tbx_command *cmd = calloc(1, sizeof(struct tbx_command));

  wl_list_init(&cmd->commands);
  cmd->server = server;
  cmd->name = 0;
  cmd->exec = 0;
  cmd->context = 0;
  return cmd;
}

struct tbx_command *register_command(struct tbx_command *context, char *name,
                                     tbx_exec *exec) {
  struct tbx_command *cmd = context_create(context->server);
  cmd->exec = exec;
  cmd->name = calloc(strlen(name) + 1, sizeof(char));
  cmd->context = context;
  strcpy(cmd->name, name);
  wl_list_insert(&context->commands, &cmd->link);

  return cmd;
}

bool command_check_args(struct tbx_command *context, int argc, int min) {
  if (argc < min) {
    console_log("%s %d arguments expected\n", context->name, min);
    return false;
  }

  return true;
}

void command_unhandled(struct tbx_command *context, char *cmd) {
  printf("%s unhandled\n", cmd);
}

struct tbx_command *command_execute(struct tbx_command *context, int argc,
                                    char **argv) {
  if (argc < 1) {
    // empty line?
    return context;
  }

  if (strcmp("}", argv[0]) == 0) {
    return context->context;
  }

  struct tbx_command *cmd;
  wl_list_for_each(cmd, &context->commands, link) {
    if (strcmp(cmd->name, argv[0]) == 0) {
      if (cmd->exec) {
        cmd->exec(cmd, argc - 1, &argv[1]);
      }
      if (wl_list_length(&cmd->commands) > 0) {
        return cmd;
      }
      break;
    }
  }

  return context;
}

void command_setup(struct tbx_server *server) {
  server->command = context_create(server);
  register_config_commands(server);
  register_input_commands(server);
  register_output_commands(server);
}