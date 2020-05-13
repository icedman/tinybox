#define _XOPEN_SOURCE 700 // for realpath

#include "tinybox/config.h"
#include "tinybox/command.h"
#include "tinybox/server.h"

#include "common/stringop.h"
#include "common/util.h"

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void load_config(struct tbx_server *server, char *path) {
  FILE *f = fopen(path, "r");
  if (!f) {
    return;
  }

  struct tbx_command *ctx = server->command;

  char *line = NULL;
  size_t line_size = 0;
  size_t nread;
  while (!feof(f)) {
    nread = getline(&line, &line_size, f);
    if (!nread) {
      continue;
    }

    if (!ctx) {
      ctx = server->command;
    }

    int argc;
    char **argv = split_args(line, &argc);

    ctx = command_execute(ctx, argc, argv);
    free_argv(argc, argv);
  }

  fclose(f);
}

bool config_setup(struct tbx_server *server) {
  server->config.server = server;
  wl_list_init(&server->config.input);
  wl_list_init(&server->config.layout);
  return true;
}