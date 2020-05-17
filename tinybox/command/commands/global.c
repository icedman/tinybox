#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void view_close(struct tbx_view* view);

void exec_exec(struct tbx_command* cmd, int argc, char** argv)
{
    if (!command_check_args(cmd, argc, 1)) {
        return;
    }

    //-------------------
    // extract command
    //-------------------
    char command_line[512];
    char* ptr = command_line;
    for (int i = 0; i < argc; i++) {
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

    console_log("%s", command_line);

    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", command_line, (void*)NULL);
    }
}

void exec_kill(struct tbx_command* cmd, int argc, char** argv)
{
    struct tbx_view* top = workspace_get_top_view(cmd->server, cmd->server->workspace);
    if (top) {
        view_close(top);
    }
}

void register_global_commands(struct tbx_server* server)
{
    register_command(server->command, "exec", exec_exec);
    register_command(server->command, "kill", exec_kill);
}