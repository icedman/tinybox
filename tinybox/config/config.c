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

#include <wlr/types/wlr_keyboard.h>

char *get_dictionary_value(struct tbx_server *server, char *name) {
  if (!name) {
    return name;
  }

  struct tbx_config_dictionary *entry;
  wl_list_for_each(entry, &server->config.dictionary, link) {
    if (strcmp(name, entry->identifier) == 0) {
      return entry->value;
    }
  }

  return name;
}

void load_config(struct tbx_server *server, char *path) {

  char *expanded = calloc((strlen(path) + 1), sizeof(char));
  strcpy(expanded, path);
  expand_path(&expanded);

  FILE *f = fopen(expanded, "r");
  if (!f) {
    free(expanded);
    return;
  }

  free(expanded);

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
  struct tbx_config *config = &server->config;
  config->server = server;
  wl_list_init(&config->dictionary);
  wl_list_init(&config->input);
  wl_list_init(&config->layout);
  wl_list_init(&config->keybinding);

  // defaults
  config->workspaces = 4;
  config->swipe_threshold = 80;
  config->animate = false;
  config->super_key = WLR_MODIFIER_ALT;

  return true;
}