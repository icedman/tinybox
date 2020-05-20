#include "tinybox/style.h"
#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/config.h"
#include "tinybox/seat.h"
#include "tinybox/server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void exec_style(struct tbx_command* cmd, int argc, char** argv)
{
    if (!command_check_args(cmd, argc, 1)) {
        return;
    }
    
    load_style(cmd->server, argv[0]);

    {
        // save as preferred
        char preferred_style[512];
        sprintf(preferred_style, "# auto-generated\n\nstyle %s\n\n", argv[0]);

        char *path = "~/.tinybox/style";
        char* expanded = calloc(strlen(path) + 1, sizeof(char));
        strcpy(expanded, path);
        expand_path(&expanded);

        FILE *fp = fopen(expanded, "w");
        if (fp) {
            fputs(preferred_style, fp);
            fclose(fp);
        }

        free(expanded);
    }
}

void register_style_commands(struct tbx_server* server)
{
    register_command(server->command, "style", exec_style);
}