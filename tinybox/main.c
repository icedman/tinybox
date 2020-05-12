#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#include "tinybox/server.h"

struct tbx_server theServer = {0};

int main(int argc, const char **argv) {
    printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);

    if (!tbx_server_setup(&theServer)) {
        return 0;
    }

    if (!tbx_server_run(&theServer)) {
        return 0;
    }
    
    tbx_server_terminate(&theServer);
    return 0;
}