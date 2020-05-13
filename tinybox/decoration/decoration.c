#include "tinybox/decoration.h"
#include "tinybox/view.h"

#include <stdlib.h>

#include <wayland-server-core.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_server_decoration.h>
// #include <wlr/types/wlr_xdg_decoration_v1.h>

static void server_decoration_handle_destroy(struct wl_listener *listener,
                                             void *data) {
  struct tbx_server_decoration *deco = wl_container_of(listener, deco, destroy);
  wl_list_remove(&deco->destroy.link);
  wl_list_remove(&deco->mode.link);
  wl_list_remove(&deco->link);
  free(deco);
}

static void server_decoration_handle_mode(struct wl_listener *listener,
                                          void *data) {
  struct tbx_server_decoration *deco = wl_container_of(listener, deco, mode);

  bool csd = deco->wlr_server_decoration->mode ==
             WLR_SERVER_DECORATION_MANAGER_MODE_CLIENT;

  struct wlr_surface *deco_surface = deco->wlr_server_decoration->surface->data;
  struct tbx_server *server = deco->server;

  struct tbx_view *view;
  wl_list_for_each(view, &server->views, link) {
    if (view->surface == deco_surface) {
      view->csd = csd;
    }
  }

  // view_update_csd_from_client(view, csd);
  // arrange_container(view->container);
  // transaction_commit_dirty();
}

static void handle_server_decoration(struct wl_listener *listener, void *data) {
  struct wlr_server_decoration *wlr_deco = data;
  struct tbx_server_decoration *deco = calloc(1, sizeof(*deco));
  if (deco == NULL) {
    return;
  }

  struct tbx_decoration_manager *deco_mgr = wl_container_of(listener, deco_mgr, server_decoration);
  deco->wlr_server_decoration = wlr_deco;
  deco->server = deco_mgr->server;

  wl_signal_add(&wlr_deco->events.destroy, &deco->destroy);
  deco->destroy.notify = server_decoration_handle_destroy;

  wl_signal_add(&wlr_deco->events.mode, &deco->mode);
  deco->mode.notify = server_decoration_handle_mode;

  wl_list_insert(&deco_mgr->decorations, &deco->link);
}

bool decoration_setup(struct tbx_server *server) {
  server->decoration_manager = calloc(1, sizeof(struct tbx_decoration_manager));
  server->decoration_manager->server = server;

  wl_list_init(&server->decoration_manager->decorations);

  server->decoration_manager->server_decoration_manager =
      wlr_server_decoration_manager_create(server->wl_display);

   wlr_server_decoration_manager_set_default_mode(
     server->decoration_manager->server_decoration_manager,
     WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);

  server->decoration_manager->server_decoration.notify = handle_server_decoration;
  wl_signal_add(&server->decoration_manager->server_decoration_manager->events
                     .new_decoration,
                &server->decoration_manager->server_decoration);

  return true;
}
