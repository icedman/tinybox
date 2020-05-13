#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tinybox/server.h"
#include <wlr/util/log.h>

struct tbx_server theServer = {0};

int main(int argc, char **argv) {
  printf("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);

  char *startup_cmd = NULL;
  char *style_path = NULL;

  if (argc > 0) {
    int i;
    for (i = 0; i < argc; i++) {
      if ((!strcmp("--cmd", argv[i]) || !strcmp("-c", argv[i])) && i < argc) {
        startup_cmd = argv[i + 1];
      } else if ((!strcmp("--style", argv[i]) || !strcmp("-s", argv[i])) &&
                 i < argc) {
        style_path = argv[i + 1];
      } else if (argv[i][0] == '-') {
        printf("Usage: %s [--help] [--style STYLE] [--startup CMD]\n", argv[0]);
        return strcmp("--help", argv[i]) != 0 && strcmp("-h", argv[i]) != 0;
      }
    }
  }

  wlr_log_init(WLR_DEBUG, NULL);

  if (!tbx_server_setup(&theServer)) {
    printf("unable to setup server\n");
    return 0;
  }

  if (!tbx_server_start(&theServer)) {
    printf("unable to start\n");
    return 0;
  }

  // read style
  if (style_path) {
    load_style(&theServer, style_path);
  }

  if (startup_cmd) {
    if (fork() == 0) {
      execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
    }
  }

  wl_display_run(theServer.wl_display);

  tbx_server_terminate(&theServer);
  return 0;
}