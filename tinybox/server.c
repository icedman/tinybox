#define _POSIX_C_SOURCE 200112L
#include <getopt.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "tinybox/tbx_server.h"
#include "tinybox/tbx_decoration.h"
#include "tinybox/console.h"

bool server_create()
{
  /* The Wayland display is managed by libwayland. It handles accepting
   * clients from the Unix socket, manging Wayland globals, and so on. */
  server.wl_display = wl_display_create();

  if (!server.wl_display) {
    return false;
  }

  /* The backend is a wlroots feature which abstracts the underlying input and
   * output hardware. The autocreate option will choose the most suitable
   * backend based on the current environment, such as opening an X11 window
   * if an X11 server is running. The NULL argument here optionally allows you
   * to pass in a custom renderer if wlr_renderer doesn't meet your needs. The
   * backend uses the renderer, for example, to fall back to software cursors
   * if the backend does not support hardware cursors (some older GPUs
   * don't). */
  server.backend = wlr_backend_autocreate(server.wl_display, NULL);

  if (!server.backend) {
    return false;
  }

  /* If we don't provide a renderer, autocreate makes a GLES2 renderer for us.
   * The renderer is responsible for defining the various pixel formats it
   * supports for shared memory, this configures that for clients. */
  server.renderer = wlr_backend_get_renderer(server.backend);
  wlr_renderer_init_wl_display(server.renderer, server.wl_display);

  /* This creates some hands-off wlroots interfaces. The compositor is
   * necessary for clients to allocate surfaces and the data device manager
   * handles the clipboard. Each of these wlroots interfaces has room for you
   * to dig your fingers in and play with their behavior if you want. Note that
   * the clients cannot set the selection directly without compositor approval,
   * see the handling of the request_set_selection event below.*/
  wlr_compositor_create(server.wl_display, server.renderer);
  wlr_data_device_manager_create(server.wl_display);

  server_decoration_init();
  xdg_decoration_init();
  
  output_init();
  xdg_shell_init();
  cursor_init();
  seat_init();

  console_init();

  return true;
}

bool server_start() {  
  /* Add a Unix socket to the Wayland display. */
  const char *socket = wl_display_add_socket_auto(server.wl_display);
  if (!socket) {
    wlr_backend_destroy(server.backend);
    return false;
  }

  /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
   * startup command if requested. */
  setenv("WAYLAND_DISPLAY", socket, true);

  /* Run the Wayland event loop. This does not return until you exit the
   * compositor. Starting the backend rigged up all of the necessary event
   * loop configuration to listen to libinput events, DRM events, generate
   * frame events at the refresh rate, and so on. */
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s",
      socket);

  /* Start the backend. This will enumerate outputs and inputs, become the DRM
   * master, etc */
  if (!wlr_backend_start(server.backend)) {
    wlr_backend_destroy(server.backend);
    wl_display_destroy(server.wl_display);
    return false;
  }

  // server_print();
  return true;
}

void server_destroy() {
  
  /* Once wl_display_run returns, we shut down the server. */
  wl_display_destroy_clients(server.wl_display);
  wl_display_destroy(server.wl_display);

}

const char *header = "-------------\n%s\n";
void server_print() {
    console_clear();
    console_log("%s\nv%s", PACKAGE_NAME, PACKAGE_VERSION);

    // struct tbx_view *view;
    // printf(header_format, "views");
    // wl_list_for_each_reverse(view, &server.views, link) {
    //     console_log("%s\n", view->xdg_surface->toplevel->title);
    // }

    // struct tbx_output *output;
    // console_log(header, "outputs");
    // wl_list_for_each(output, &server.outputs, link) {
    //     double ox = 0, oy = 0;
    //     wlr_output_layout_output_coords(
    //         server.output_layout, output->wlr_output, &ox, &oy);
    //     console_log("%s (%d, %d)", output->wlr_output->name, (int)ox, (int)oy);
    // } 
}
