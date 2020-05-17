#ifndef TINYBOX_WORKSPACE_H
#define TINYBOX_WORKSPACE_H

#include "tinybox/server.h"

#define MAX_WORKSPACES 8

struct tbx_view;

struct tbx_workspace {
    int id;
    char* name;
    char* background;
    struct wlr_texture* background_texture;
    struct wl_list link;

    uint32_t background_color;

    // state
    bool active;
    struct wlr_box box;
};

void workspace_setup(struct tbx_server* server);

void cycle_next_view(struct tbx_server* server);

struct tbx_view* workspace_get_top_view(struct tbx_server* server, int workspace_id);

void workspace_activate(struct tbx_server* server, int id, bool animate);
struct tbx_workspace* get_workspace(struct tbx_server* server, int workspace_id);

#endif // TINYBOX_WORKSPACE_H