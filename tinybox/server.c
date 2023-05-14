#define _POSIX_C_SOURCE 200112L

#include <tinybox/server.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/util/log.h>

#include <stdlib.h>
#include <time.h>
#include <unistd.h>

bool
tbx_server_setup(struct tbx_server *server)
{
  wlr_log(WLR_INFO, "Preparing Tinybox server");

  /* The Wayland display is managed by libwayland. It handles accepting
   * clients from the Unix socket, manging Wayland globals, and so on. */
  server->wl_display = wl_display_create();

  /* The backend is a wlroots feature which abstracts the underlying input and
   * output hardware. The autocreate option will choose the most suitable
   * backend based on the current environment, such as opening an X11 window
   * if an X11 server is running. */
  server->backend = wlr_backend_autocreate(server->wl_display, NULL);
  if (server->backend == NULL) {
    wlr_log(WLR_ERROR, "failed to create wlr_backend");
    return false;
  }

  /* Autocreates a renderer, either Pixman, GLES2 or Vulkan for us. The user
   * can also specify a renderer using the WLR_RENDERER env var.
   * The renderer is responsible for defining the various pixel formats it
   * supports for shared memory, this configures that for clients. */
  server->renderer = wlr_renderer_autocreate(server->backend);
  if (server->renderer == NULL) {
    wlr_log(WLR_ERROR, "failed to create wlr_renderer");
    return false;
  }

  wlr_renderer_init_wl_display(server->renderer, server->wl_display);

  /* Autocreates an allocator for us.
   * The allocator is the bridge between the renderer and the backend. It
   * handles the buffer creation, allowing wlroots to render onto the
   * screen */
  server->allocator =
      wlr_allocator_autocreate(server->backend, server->renderer);
  if (server->allocator == NULL) {
    wlr_log(WLR_ERROR, "failed to create wlr_allocator");
    return false;
  }

  /* This creates some hands-off wlroots interfaces. The compositor is
   * necessary for clients to allocate surfaces, the subcompositor allows to
   * assign the role of subsurfaces to surfaces and the data device manager
   * handles the clipboard. Each of these wlroots interfaces has room for you
   * to dig your fingers in and play with their behavior if you want. Note that
   * the clients cannot set the selection directly without compositor approval,
   * see the handling of the request_set_selection event below.*/
  wlr_compositor_create(server->wl_display, 5, server->renderer);
  wlr_subcompositor_create(server->wl_display);
  wlr_data_device_manager_create(server->wl_display);

  tbx_output_setup(server);

  wl_list_init(&server->views);
  tbx_xdg_shell_setup(server);
  // tbx_xwayland_shell_setup(server);

  tbx_cursor_setup(server);
  tbx_seat_setup(server);

  return true;
}

bool
tbx_server_start(struct tbx_server *server)
{
  /* Add a Unix socket to the Wayland display. */
  const char *socket = wl_display_add_socket_auto(server->wl_display);
  if (!socket) {
    return false;
  }

  /* Start the backend. This will enumerate outputs and inputs, become the DRM
   * master, etc */
  if (!wlr_backend_start(server->backend)) {
    return false;
  }

  /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
   * startup command if requested. */
  setenv("WAYLAND_DISPLAY", socket, true);
  if (fork() == 0) {
    execl("/bin/sh", "/bin/sh", "-c", "kitty", (void *)NULL);
  }

  /* Run the Wayland event loop. This does not return until you exit the
   * compositor. Starting the backend rigged up all of the necessary event
   * loop configuration to listen to libinput events, DRM events, generate
   * frame events at the refresh rate, and so on. */
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);

  wl_display_run(server->wl_display);
  return true;
}

void
tbx_server_shutdown(struct tbx_server *server)
{
  if (server->backend) {
    wlr_backend_destroy(server->backend);
  }
  if (server->wl_display) {
    wl_display_destroy(server->wl_display);
  }
}