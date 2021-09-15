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

char *
config_dictionary_value(struct tbx_server *server, char *name)
{
  if (!name) {
    return name;
  }

  struct tbx_config_dictionary *entry;
  wl_list_for_each (entry, &server->config.dictionary, link) {
    if (strcmp(name, entry->identifier) == 0) {
      return entry->value;
    }
  }

  return name;
}

FILE *
open_file(char *path)
{
  FILE *fp = 0;

  char *expanded = calloc((strlen(path) + 1), sizeof(char));
  strcpy(expanded, path);
  expand_path(&expanded);

  fp = fopen(expanded, "r");
  free(expanded);

  return fp;
}

void
parse_file(struct tbx_command *ctx, FILE *fp)
{
  char *line = NULL;
  size_t line_size = 0;
  size_t nread;
  while (!feof(fp)) {
    nread = getline(&line, &line_size, fp);
    if (!nread) {
      continue;
    }

    if (!ctx) {
      // oops!!
      break;
    }

    int argc;
    char **argv = split_args(line, &argc);
    if (argc && strcmp(argv[0], "include") == 0) {
      FILE *fp2 = open_file(argv[1]);
      if (fp2) {
        parse_file(ctx, fp2);
        fclose(fp2);
      }
    } else {
      ctx = command_execute(ctx, argc, argv);
    }

    free_argv(argc, argv);
  }
}

void
load_config(struct tbx_server *server, char *path)
{
  struct tbx_command *ctx = server->command;

  FILE *fp = open_file(path);
  if (fp) {
    parse_file(ctx, fp);
    fclose(fp);
  }
}

bool
config_setup(struct tbx_server *server)
{
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
  config->track_damages = true;
  config->debug_damages = false;

  return true;
}