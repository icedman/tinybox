#ifndef TINYBOX_COMMAND_H
#define TINYBOX_COMMAND_H

#include "tinybox/server.h"

struct tbx_command;

typedef void tbx_exec(struct tbx_command* context, int argc, char** argv);

enum tbx_command_type {
    TBX_COMMAND,
    TBX_COMMAND_MENU,
    TBX_COMMAND_MENU_ITEM
};

struct tbx_command {
    char* identifier;
    enum tbx_command_type type;
    struct wl_list link;
    tbx_exec* exec;
    tbx_exec* exec_custom;
    struct tbx_server* server;
    struct wl_list commands;
    struct tbx_command* context;

    char* description;
    void* data;
};

void command_setup(struct tbx_server* server);
bool command_check_args(struct tbx_command* context, int argc, int min);
void command_unhandled(struct tbx_command* context, char* cmd);

struct tbx_command* command_execute(struct tbx_command* context, int argc,
    char** argv);
struct tbx_command* register_command(struct tbx_command* context, char* name,
    tbx_exec* exec);

#endif // TINYBOX_COMMAND_H