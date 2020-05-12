#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include <wlr/util/log.h>
#include "tinybox/server.h"

struct tbx_server theServer = {0};

int main(int argc, const char **argv) {
    printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);

    wlr_log_init(WLR_DEBUG, NULL);

    if (!tbx_server_setup(&theServer)) {
        printf("unable to setup server\n");
        return 0;
    }

    if (!tbx_server_start(&theServer)) {
        printf("unable to start\n");
        return 0;
    }

    if (fork() == 0) {
        execl("/bin/sh", "/bin/sh", "-c", "termite", (char *) NULL);
    }

    wl_display_run(theServer.wl_display);
    
    tbx_server_terminate(&theServer);
    return 0;
}