#include "tinybox/tbx_server.h"

void init_server()
{
  /* The Wayland display is managed by libwayland. It handles accepting
   * clients from the Unix socket, manging Wayland globals, and so on. */
  server.wl_display = wl_display_create();
  /* The backend is a wlroots feature which abstracts the underlying input and
   * output hardware. The autocreate option will choose the most suitable
   * backend based on the current environment, such as opening an X11 window
   * if an X11 server is running. The NULL argument here optionally allows you
   * to pass in a custom renderer if wlr_renderer doesn't meet your needs. The
   * backend uses the renderer, for example, to fall back to software cursors
   * if the backend does not support hardware cursors (some older GPUs
   * don't). */
  server.backend = wlr_backend_autocreate(server.wl_display, NULL);

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

  server.server_decoration = wlr_server_decoration_manager_create(server.wl_display);

  wlr_server_decoration_manager_set_default_mode(
    server.server_decoration,
    WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);
}