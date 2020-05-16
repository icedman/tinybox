#include "tinybox/workspace.h"
#include "tinybox/output.h"
#include "tinybox/view.h"

#include <stdlib.h>
#include <string.h>

#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_shell.h>

static struct tbx_workspace* create_workspace(struct tbx_server* server)
{
    struct tbx_workspace* ws = calloc(1, sizeof(struct tbx_workspace));
    wl_list_insert(&server->workspaces, &ws->link);
    return ws;
}

void workspace_setup(struct tbx_server* server)
{
    struct tbx_config* config = &server->config;
    config->server = server;
    server->workspace = -1;

    wl_list_init(&server->workspaces);
    for (int i = 0; i < MAX_WORKSPACES; i++) {
        struct tbx_workspace* ws = create_workspace(server);
        ws->id = i;
    }

    config->workspaces = config->workspaces;
    // console_log("workspaces %d", config->workspaces);
}

void move_to_workspace(struct tbx_server* server, struct tbx_view* view, int id,
    bool animate)
{
    if (!view) {
        return;
    }
    struct tbx_config* config = &server->config;
    if (id < 0) {
        id = 0;
    }
    if (id >= config->workspaces) {
        id = config->workspaces;
    }

    int prev = view->workspace;
    view->workspace = id;
    // console_log("view at ws %d", view->workspace);

    activate_workspace(server, id, animate);
    focus_view_without_raising(view, view->surface);

    // animate view
    if (animate) {

        // recompute workspaces
        struct wlr_box* main_box = wlr_output_layout_get_box(
            server->output_layout, server->main_output->wlr_output);

        int dir = id - prev;
        view->wsv_animate = true;
        view->wsv_anim_x += dir * main_box->width;
        view->wsv_anim_y = 0;
    }
}

void activate_workspace(struct tbx_server* server, int id, bool animate)
{
    struct tbx_config* config = &server->config;

    int prev = server->workspace;

    if (id < 0) {
        id = 0;
    }
    if (id >= config->workspaces) {
        id = config->workspaces;
    }
    server->workspace = id;
    // console_log("desktop at ws %d", server->workspace);

    // recompute workspaces
    struct wlr_box* main_box = wlr_output_layout_get_box(
        server->output_layout, server->main_output->wlr_output);

    if (prev != id) {
        struct tbx_view* view;
        wl_list_for_each_reverse(view, &server->views, link)
        {
            if (view->workspace == id) {
                focus_view_without_raising(view, view->surface);
                break;
            }
        }

        struct tbx_workspace* workspace;
        wl_list_for_each(workspace, &server->workspaces, link)
        {
            memcpy(&workspace->box, main_box, sizeof(struct wlr_box));
            workspace->box.x = (main_box->width * workspace->id) - (main_box->width * server->workspace);
            workspace->box.y = 0;
            workspace->active = workspace->id == server->workspace;
        }
    }

    if (animate) {
        int dir = id - prev;
        server->ws_animate = true;
        server->ws_anim_x += dir * main_box->width;
        server->ws_anim_y = 0;
    }
}

struct tbx_workspace* get_workspace(struct tbx_server* server, int id)
{
    struct tbx_workspace* workspace;
    wl_list_for_each(workspace, &server->workspaces, link)
    {
        if (workspace->id == id) {
            return workspace;
        }
    }
    return NULL;
}

void cycle_next_view(struct tbx_server* server)
{
    /* Cycle to the next view */
    if (wl_list_length(&server->views) < 2) {
        return;
    }

    struct tbx_view* current_view = wl_container_of(server->views.next, current_view, link);

    struct tbx_view* next_view = wl_container_of(current_view->link.next, next_view, link);

    // implement xwayland_surface!
    focus_view(next_view, next_view->surface);

    /* Move the previous view to the end of the list */
    wl_list_remove(&current_view->link);
    wl_list_insert(server->views.prev, &current_view->link);
}