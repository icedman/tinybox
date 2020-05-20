#include "tinybox/menu.h"
#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct tbx_menu* create_menu(struct tbx_command* cmd, int argc, char** argv);
struct tbx_menu* create_item(struct tbx_command* cmd, int argc, char** argv);

static void distill(char* str) {
    int l=strlen(str);
    for(int i=0; i<l; i++) {
        if (str[i] == '[' || str[i] == ']' ||
            str[i] == '(' || str[i] == ')' ||
            str[i] == '{' || str[i] == '}') {
            str[i] = '"';
        }
    }
}

static void exec_menu_custom(struct tbx_command* cmd, int _argc, char** _argv)
{
    if (!command_check_args(cmd, _argc, 1)) {
        return;
    }

    char *command_line = (char*)command_merge_args(cmd->server, _argc, _argv);
    distill(command_line);

    int argc;
    char** argv = split_args(command_line, &argc);

    struct tbx_menu* menu_context = cmd->server->menu_context;
    // struct tbx_command *mnu_cmd_ctx = (struct tbx_command*)menu_context;

    strip_quotes(argv[0]);

    if (strcmp("begin", argv[0]) == 0) {
        // we already did this at
        // exec_create_root_menu
        free_argv(argc, argv);
        return;
    }

    if (strcmp("end", argv[0]) == 0) {
        // pop context
        cmd->server->menu_context = cmd->server->menu_context->parent;
        if (!cmd->server->menu_context) {
            // console_log("over pop?");
            cmd->server->menu_context = cmd->server->menu;
        }
        free_argv(argc, argv);
        return;
    }

    // console_log("context %s %s %s", mnu_cmd_ctx->identifier, argv[0], argv[1]);

    struct tbx_menu* menu;
    struct tbx_command* item;
    if (strcmp(argv[0], "submenu") == 0) {
        menu = create_menu(cmd, argc, argv);
        item = &menu->command;
        // push context
        cmd->server->menu_context = menu;
    } else {
        menu = create_item(cmd, argc, argv);
        item = &menu->command;
    }

    if (item) {
        menu->parent = (struct tbx_menu*)menu_context;

        // also create an item
        if (menu_context) {
            // console_log("insert %s %d %d!", parent->identifier, parent->type, TBX_COMMAND_MENU);
            wl_list_insert(&menu_context->items, &item->link);
        }

        // debug only
        if (menu->menu_type == TBX_MENU) {
            // console_log("menu %s", argv[0]);
        }
        if (menu->menu_type == TBX_MENU_ITEM) {
            // console_log("  item %s", argv[0]);
        }
    }

    free_argv(argc, argv);
}

static void exec_create_root_menu(struct tbx_command* cmd, int argc, char** argv)
{
    if (!command_check_args(cmd, argc, 1)) {
        return;
    }

    if (argc == 1 && !cmd->server->menu) {
        cmd->server->menu = create_menu(cmd, argc, argv);
        cmd->server->menu_context = cmd->server->menu;
        wl_list_insert(&cmd->server->menus, &cmd->server->menu->command.link);
        console_log("root %s", cmd->server->menu_context->label);
        return;
    }
}

static void exec_show_menu(struct tbx_command* cmd, int argc, char** argv)
{
    if (!command_check_args(cmd, argc, 2)) {
        return;
    }

    struct tbx_menu* menu = cmd->server->menu;
    int x = 0;
    int y = 0;
    if (argc == 3) {
        // menu_identifier x y
        // find a menu
        x = strtol(argv[1], NULL, 10);
        y = strtol(argv[2], NULL, 10);
    }

    if (argc == 2) {
        // x y
        menu = cmd->server->menu;
        x = strtol(argv[0], NULL, 10);
        y = strtol(argv[1], NULL, 10);
    }

    // show a menu
    if (menu) {
        menu_show(menu, x, y, !menu->shown);
    }
}

static int menu_id = 0;
struct tbx_menu* create_menu(struct tbx_command* cmd, int argc, char** argv)
{
    struct tbx_menu* menu = calloc(1, sizeof(struct tbx_menu));
    struct tbx_command* m = &menu->command;

    distill(argv[0]);
    strip_quotes(argv[0]);

    char* identifier = argv[0];
    if (argc > 1) {
        distill(argv[1]);
        strip_quotes(argv[1]);
        identifier = argv[1];
    }
    m->identifier = calloc(strlen(identifier) + 8, sizeof(char));
    // strcpy(m->identifier, argv[0]);
    sprintf(m->identifier, "%d::%s", menu_id++, identifier);

    menu->label = calloc(strlen(identifier) + 1, sizeof(char));
    strcpy(menu->label, identifier);

    if (argc > 2) {
        strip_quotes(argv[2]);
        menu->title = calloc(strlen(argv[2]) + 8, sizeof(char));
        strcpy(menu->title, argv[2]);
    }

    wl_list_init(&menu->items);

    m->type = TBX_MENU;
    m->server = cmd->server;
    m->context = cmd;
    return menu;
}

struct tbx_menu* create_item(struct tbx_command* cmd, int argc, char** argv)
{
    struct tbx_menu* menu = create_menu(cmd, argc, argv);
    struct tbx_command* m = &menu->command;

    // strip_quotes(argv[0]);
    if (argc > 2) {
        menu->execute = calloc(strlen(argv[0]) + strlen(argv[2]) + 2, sizeof(char));
        sprintf(menu->execute, "%s %s", argv[0], argv[2]);

        strip_quotes(menu->execute);
        menu->argv = split_args(menu->execute, &menu->argc);
        // console_log(">%s %s %s", argv[0], argv[1], argv[2]);
    }

    // convert to menu_item
    m->type = TBX_MENU_ITEM;
    return menu;
}

void register_menu_commands(struct tbx_server* server)
{
    wl_list_init(&server->menus);
    register_command(server->command, "menu", exec_show_menu);

    struct tbx_command* cmd = register_command(server->command, "[begin]", exec_create_root_menu);
    cmd->exec_custom = exec_menu_custom;
}