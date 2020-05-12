#define _POSIX_C_SOURCE 200112L

#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "tinybox/style_defaults.h"
#include "tinybox/tbx_server.h"

struct tbx_server server;

int main(int argc, char *argv[]) {
  
  wlr_log_init(WLR_DEBUG, NULL);

  char *startup_cmd = NULL;
  char *style_path = NULL;
  if (argc > 0) {
    int i;
    for (i = 0; i < argc; i++) {
      if ((!strcmp("--cmd", argv[i]) || !strcmp("-c", argv[i])) && i < argc) {
        startup_cmd = argv[i + 1];
      } else if ((!strcmp("--style", argv[i]) || !strcmp("-s", argv[i])) && i < argc) {
        style_path = argv[i + 1];
      } else if (!strcmp("--version", argv[i]) || !strcmp("-v", argv[i])) {
        printf(PACKAGE_NAME " " PACKAGE_VERSION "\n");
        return 0;
      } else if (argv[i][0] == '-') {
        printf("Usage: %s [--help] --version [--style STYLE] [--startup CMD]\n", argv[0]);
        return strcmp("--help", argv[i]) != 0 && strcmp("-h", argv[i]) != 0;
      }
    }
  }

  // read style
  if (style_path) {
    load_style(&server.style, style_path);
  } else {
    memcpy(&server.style, style_bin, sizeof(struct tbx_style));
  }
  strcpy(server.style.font, "monospace 10");
  
  if (!server_create()) {
    return false;
  }

  if (!server_start()) {
    return false;
  }

  if (startup_cmd) {
    if (fork() == 0) {
      execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
    }
  }

  wl_display_run(server.wl_display);
  
  server_destroy();
  return 0;
}
