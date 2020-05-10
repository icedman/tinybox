#include <stdlib.h>
#include "tinybox/tbx_decoration.h"
#include "tinybox/tbx_server.h"
#include "tinybox/console.h"

static void server_decoration_handle_destroy(struct wl_listener *listener,
        void *data) {
    struct tbx_server_decoration *deco =
        wl_container_of(listener, deco, destroy);
    wl_list_remove(&deco->destroy.link);
    wl_list_remove(&deco->mode.link);
    wl_list_remove(&deco->link);
    free(deco);
}

static void server_decoration_handle_mode(struct wl_listener *listener,
        void *data) {
    struct tbx_server_decoration *deco =
        wl_container_of(listener, deco, mode);

    bool csd = deco->wlr_server_decoration->mode ==
            WLR_SERVER_DECORATION_MANAGER_MODE_CLIENT;

    struct wlr_surface *deco_surface = deco->wlr_server_decoration->surface->data;
    struct tbx_view *view;
    wl_list_for_each_reverse(view, &server.views, link) {
        if (view->surface == deco_surface) {
            view->csd = csd;
        }
    } 
    
    // view_update_csd_from_client(view, csd);
    // arrange_container(view->container);
    // transaction_commit_dirty();
}

void handle_server_decoration(struct wl_listener *listener, void *data) {
    struct wlr_server_decoration *wlr_deco = data;
    struct tbx_server_decoration *deco = calloc(1, sizeof(*deco));
    if (deco == NULL) {
        return;
    }
    deco->wlr_server_decoration = wlr_deco;

    wl_signal_add(&wlr_deco->events.destroy, &deco->destroy);
    deco->destroy.notify = server_decoration_handle_destroy;

    wl_signal_add(&wlr_deco->events.mode, &deco->mode);
    deco->mode.notify = server_decoration_handle_mode;

    wl_list_insert(&server.decorations, &deco->link);
    // console_log("server_decoration\n");
}

struct tbx_server_decoration *decoration_from_surface(
        struct wlr_surface *surface) {
    struct tbx_server_decoration *deco;
    wl_list_for_each(deco, &server.decorations, link) {
        if (deco->wlr_server_decoration->surface == surface) {
            return deco;
        }
    }
    return NULL;
}


void server_decoration_init() {
    server.server_decoration_manager =
    wlr_server_decoration_manager_create(server.wl_display);
  wlr_server_decoration_manager_set_default_mode(
    server.server_decoration_manager,
    WLR_SERVER_DECORATION_MANAGER_MODE_SERVER);
  wl_signal_add(&server.server_decoration_manager->events.new_decoration,
    &server.server_decoration);
  server.server_decoration.notify = handle_server_decoration;
  wl_list_init(&server.decorations);
}