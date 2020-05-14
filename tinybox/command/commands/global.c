#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void exec_exec(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  //-------------------
  // extract command
  //-------------------
  char command_line[512];
  char *ptr = command_line;
  for (int i = 0; i < argc; i++) {
    char *n = argv[i];

    if (n[0] == '$') {
      n = get_dictionary_value(cmd->server, n);
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

  if (fork() == 0) {
    execl("/bin/sh", "/bin/sh", "-c", command_line, (void *)NULL);
  }

  console_log("exec!!!");
}

void register_global_commands(struct tbx_server *server) {
  register_command(server->command, "exec", exec_exec);
}