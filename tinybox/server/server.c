#define _POSIX_C_SOURCE 200112L

#include "tinybox/server.h"
#include "tinybox/command.h"
#include "tinybox/console.h"
#include "tinybox/cursor.h"
#include "tinybox/damage.h"
#include "tinybox/decoration.h"
#include "tinybox/output.h"
#include "tinybox/render.h"
#include "tinybox/seat.h"
#include "tinybox/shell.h"
#include "tinybox/workspace.h"

#ifdef HAVE_WAYLAND
#include "tinybox/xwayland.h"
#endif

#include <getopt.h>
#include <stdlib.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>

#include <wlr/util/log.h>

bool
tbx_server_setup(struct tbx_server *server)
{
  server->wl_display = wl_display_create();

  if (!server->wl_display) {
    return false;
  }

  server->backend = wlr_backend_autocreate(server->wl_display);
  if (!server->backend) {
    return false;
  }

  server->suspend_damage_tracking = 0;
  
  server->wl_event_loop = wl_display_get_event_loop(server->wl_display);
  server->renderer = wlr_backend_get_renderer(server->backend);

  wlr_renderer_init_wl_display(server->renderer, server->wl_display);

  server->compositor =
      wlr_compositor_create(server->wl_display, server->renderer);
  if (!server->compositor) {
    return false;
  }

  wlr_data_device_manager_create(server->wl_display);

  wl_list_init(&server->views);

  // load config
  config_setup(server);

  // setup protocols
  output_setup(server);
  xdg_shell_setup(server);
  
  #ifdef HAVE_WAYLAND
  xwayland_shell_setup(server);
  #endif

  decoration_setup(server);

  damage_setup(server);
  cursor_setup(server);
  seat_setup(server);
  console_setup(server);
  command_setup(server);

  // create max of 8 workspaces
  workspace_setup(server);

  console_log("%s %s\n", PACKAGE_NAME, PACKAGE_VERSION);
  return true;
}

bool
tbx_server_start(struct tbx_server *server)
{
  const char *socket = wl_display_add_socket_auto(server->wl_display);
  if (!socket) {
    wlr_backend_destroy(server->backend);
    return false;
  }

  /* Set the WAYLAND_DISPLAY environment variable to our socket and run the
   * startup command if requested. */
  setenv("WAYLAND_DISPLAY", socket, true);

  /* Run the Wayland event loop. This does not return until you exit the
   * compositor. Starting the backend rigged up all of the necessary event
   * loop configuration to listen to libinput events, DRM events, generate
   * frame events at the refresh rate, and so on. */
  wlr_log(WLR_INFO, "Running Wayland compositor on WAYLAND_DISPLAY=%s", socket);

  /* Start the backend. This will enumerate outputs and inputs, become the DRM
   * master, etc */
  if (!wlr_backend_start(server->backend)) {
    tbx_server_terminate(server);
    return false;
  }

  workspace_activate(server, 0, false);
  server->started = true;

  damage_whole(server);

  return true;
}

void
tbx_server_terminate(struct tbx_server *server)
{
  // destroy seats
  // destroy cursor
  // destroy xdg_shell

  texture_cache_destroy();

  wlr_backend_destroy(server->backend);
  wl_display_destroy(server->wl_display);
}
