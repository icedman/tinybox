#define _POSIX_C_SOURCE 200112L
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "tinybox/style_defaults.h"
#include "tinybox/tbx_server.h"
#include "tinybox/cairo.h"

struct tbx_server server;

const char *header_format = "\n==========\n%s\n----------\n";
void server_print() {
    struct tbx_view *view;
    printf(header_format, "views");
    wl_list_for_each_reverse(view, &server.views, link) {
        printf("%s\n", view->xdg_surface->toplevel->title);
    }

    struct tbx_output *output;
    printf(header_format, "outputs");
    wl_list_for_each(output, &server.outputs, link) {
        // double ox = 0, oy = 0;
        // wlr_output_layout_output_coords(
        //     server.output_layout, output->wlr_output, &ox, &oy);
        // printf("%s %f %f\n", output->wlr_output->name, ox, oy);
        printf("%s\n", output->wlr_output->name);
    } 
}

int main(int argc, char *argv[]) {
  printf("tinybox\n");

  wlr_log_init(WLR_DEBUG, NULL);

  char *startup_cmd = NULL;
  char *style_path = NULL;
  if (argc > 0) {
    int i;
    for (i = 0; i < argc; i++) {
      if (!strcmp("--debug", argv[i]) || !strcmp("-v", argv[i]) || !strcmp("--exit", argv[i])) {
        printf("Warning: option %s is currently unimplemented\n", argv[i]);
      } else if ((!strcmp("--cmd", argv[i]) || !strcmp("-c", argv[i])) && i < argc) {
        startup_cmd = argv[i + 1];
      } else if ((!strcmp("--style", argv[i]) || !strcmp("-s", argv[i])) && i < argc) {
        style_path = argv[i + 1];
      } else if (!strcmp("--version", argv[i]) || !strcmp("-V", argv[i])) {
        // printf(PACKAGE_NAME " " PACKAGE_VERSION "\n");
        return 0;
      } else if (argv[i][0] == '-') {
        printf("Usage: %s [--debug] [--exit] [--help] [--style STYLE] [--startup CMD] [--version]\n", argv[0]);
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

  console_init(400, 400);
  
  init_server();
  output_init();
  xdg_shell_init();
  cursor_init();
  seat_init();

  /* Add a Unix socket to the Wayland display. */
  const char *socket = wl_display_add_socket_auto(server.wl_display);
  if (!socket) {
    wlr_backend_destroy(server.backend);
    return 1;
  }

  /* Start the backend. This will enumerate outputs and inputs, become the DRM
   * master, etc */
  if (!wlr_backend_start(server.backend)) {
    wlr_backend_destroy(server.backend);
    wl_display_destroy(server.wl_display);
    return 1;
  }

  /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
   * startup command if requested. */
  setenv("WAYLAND_DISPLAY", socket, true);
  if (startup_cmd) {
    if (fork() == 0) {
      execl("/bin/sh", "/bin/sh", "-c", startup_cmd, (void *)NULL);
    }
  }

  /* Run the Wayland event loop. This does not return until you exit the
   * compositor. Starting the backend rigged up all of the necessary event
   * loop configuration to listen to libinput events, DRM events, generate
   * frame events at the refresh rate, and so on. */
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
      socket);
  wl_display_run(server.wl_display);

  // wlr_server_decoration_manager_destroy(server.server_decoration);
  
  /* Once wl_display_run returns, we shut down the server. */
  wl_display_destroy_clients(server.wl_display);
  wl_display_destroy(server.wl_display);
  return 0;
}
