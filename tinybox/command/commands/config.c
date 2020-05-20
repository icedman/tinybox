#include "tinybox/config.h"
#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void exec_set(struct tbx_command* cmd, int argc, char** argv)
{
    if (!command_check_args(cmd, argc, 2)) {
        return;
    }

    cmd->data = 0;
    strip_quotes(argv[0]);
    strip_quotes(argv[1]);

    struct tbx_server* server = cmd->server;

    // find dictionary
    struct tbx_config_dictionary* entry;
    wl_list_for_each(entry, &server->config.dictionary, link)
    {
        if (entry->identifier && strcmp(entry->identifier, argv[0]) == 0) {
            cmd->data = (struct tbx_dictionary_config*)entry;
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

static void exec_workspaces(struct tbx_command* cmd, int argc, char** argv)
{
    if (!command_check_args(cmd, argc, 1)) {
        return;
    }

    struct tbx_config* config = &cmd->server->config;
    config->workspaces = strtol(argv[0], NULL, 10);
}

static void exec_animate(struct tbx_command* cmd, int argc, char** argv)
{
    if (!command_check_args(cmd, argc, 1)) {
        return;
    }

    struct tbx_config* config = &cmd->server->config;
    config->animate = parse_boolean(argv[0], false);
}

static void exec_swipe_threshold(struct tbx_command* cmd, int argc,
    char** argv)
{
    if (!command_check_args(cmd, argc, 1)) {
        return;
    }

    struct tbx_config* config = &cmd->server->config;
    config->swipe_threshold = strtol(argv[0], NULL, 10);
    if (config->swipe_threshold < 0) {
        config->swipe_threshold = 0;
    }
    if (config->swipe_threshold > 500) {
        config->swipe_threshold = 500;
    }
}

static void exec_console(struct tbx_command* cmd, int argc, char** argv)
{
    struct tbx_config* config = &cmd->server->config;
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

void register_config_commands(struct tbx_server* server)
{
    register_command(server->command, "set", exec_set);
    register_command(server->command, "workspaces", exec_workspaces);
    register_command(server->command, "animate", exec_animate);
    register_command(server->command, "swipe_threshold", exec_swipe_threshold);
    register_command(server->command, "console", exec_console);
}