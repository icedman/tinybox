#define _POSIX_C_SOURCE 200112L

#include "tinybox/server.h"
#include "tinybox/cursor.h"
#include "tinybox/output.h"

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/util/log.h>

bool tbx_server_setup(struct tbx_server *server)
{
    server->wl_display = wl_display_create();

    if (!server->wl_display) {
        return false;
    }

    server->backend = wlr_backend_autocreate(server->wl_display, NULL);
    if (!server->backend) {
        return false;
    }

    server->renderer = wlr_backend_get_renderer(server->backend);
    wlr_renderer_init_wl_display(server->renderer, server->wl_display);
    wlr_compositor_create(server->wl_display, server->renderer);

    wlr_data_device_manager_create(server->wl_display);

    output_setup(server);
    cursor_setup(server);
    seat_setup(server);
    
    // seats
    // keyboards

    return true;
}

bool tbx_server_run(struct tbx_server *server)
{
  if (!wlr_backend_start(server->backend)) {
    tbx_server_terminate(server);
    return false;
  }

  wl_display_run(server->wl_display);
  return true;
}

void tbx_server_terminate(struct tbx_server *server)
{
  // destroy seats
  // destroy cursor
  
  wlr_backend_destroy(server->backend);
  wl_display_destroy(server->wl_display);  
}