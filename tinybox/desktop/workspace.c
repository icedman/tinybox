#include "tinybox/workspace.h"
#include "tinybox/output.h"
#include "tinybox/render.h"
#include "tinybox/view.h"

#include <stdlib.h>
#include <string.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <wlr/render/gles2.h>

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

void workspace_activate(struct tbx_server* server, int id, bool animate)
{
    struct tbx_config* config = &server->config;

    int prev = server->workspace;

    if (id < 0) {
        id = 0;
    }
    if (id >= config->workspaces) {
        id = config->workspaces - 1;
    }
    server->workspace = id;
    // console_log("desktop at ws %d", server->workspace);

    // recompute workspaces
    struct wlr_box* main_box = wlr_output_layout_get_box(
        server->output_layout, server->main_output->wlr_output);

    if (prev != id) {
        // activate top most
        struct tbx_view* view = workspace_get_top_view(server, id);
        if (view) {
            view->interface->set_activated(view, true);
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

struct tbx_view* workspace_get_top_view(struct tbx_server* server, int workspace_id)
{
    struct tbx_view* view;
    wl_list_for_each(view, &server->views, link)
    {
        if (!view->mapped) {
            continue;
        }
        if (view->workspace == workspace_id) {
            return view;
        }
    }
    return NULL;
}

struct tbx_workspace* get_workspace(struct tbx_server* server, int workspace_id)
{
    if (workspace_id >= server->config.workspaces) {
        return NULL;
    }
    struct tbx_workspace* workspace;
    wl_list_for_each(workspace, &server->workspaces, link)
    {
        if (workspace->id == workspace_id) {
            return workspace;
        }
    }
    return NULL;
}

void workspace_cycle_views(struct tbx_server* server, int workspace_id)
{
    /* Cycle to the next view */
    if (wl_list_length(&server->views) < 2) {
        return;
    }

    struct tbx_view* current_view = workspace_get_top_view(server, workspace_id);
    if (!current_view) {
        return;
    }

    // get next
    struct tbx_view* next_view = NULL;
    struct tbx_view* view;
    wl_list_for_each(view, &server->views, link)
    {
        if (!view->mapped || view->parent) {
            continue;
        }
        if (view == current_view) {
            continue;
        }
        if (view->workspace == current_view->workspace) {
            next_view = view;
            break;
        }
    }

    if (!next_view) {
        return;
    }

    view_set_focus(next_view, next_view->surface);

    /* Move the previous view to the end of the list */
    wl_list_remove(&current_view->link);
    wl_list_insert(server->views.prev, &current_view->link);
}

void render_workspace(struct tbx_output* output,
    struct tbx_workspace* workspace)
{
    if (!workspace) {
        return;
    }

    struct tbx_server* server = output->server;
    struct tbx_cursor* cursor = server->cursor;
    // struct wlr_renderer *renderer = server->renderer;

    bool in_main_output = (output == server->main_output);

    struct wlr_texture* texture = 0;
    int texture_id = tx_workspace_1 + workspace->id;

    texture = get_texture_cache(texture_id);
    if (!texture && workspace->background) {
        // generate and try again
        generate_background(output, workspace);
        texture = get_texture_cache(texture_id);
        if (!texture) {
            // invalid image
            workspace->background = 0;
        }
    }

    // fallback default background of workspace_1
    if (!texture) {
        texture = get_texture_cache(tx_workspace_1);
    }

    if (!texture) {
        return;
    }

    struct wlr_box box;
    memcpy(&box, &workspace->box, sizeof(struct wlr_box));

    if (server->config.animate && in_main_output && (cursor->mode == TBX_CURSOR_SWIPE_WORKSPACE || server->ws_animate)) {
        if (server->ws_animate) {
            box.x += server->ws_anim_x;
        } else {
            float d = server->cursor->swipe_x - server->cursor->swipe_begin_x;
            if (d * d > SWIPE_MIN) {
                box.x += d;
            }
        }
    }

    struct wlr_box* layout_box = wlr_output_layout_get_box(
        server->output_layout, output->wlr_output);
    box.width = layout_box->width;
    box.height = layout_box->height;

    render_texture(output->wlr_output, &box, texture, output->wlr_output->scale);
}