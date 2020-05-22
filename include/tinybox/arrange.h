#ifndef TINYBOX_ARRANGE_H
#define TINYBOX_ARRANGE_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_view;
struct tbx_server;

struct tbx_vec {
    float x;
    float y;
};

struct tbx_packer_node {
    struct wl_list link;
    struct tbx_view* view;

    int id;

    float x;
    float y;
    float w;
    float h;

    struct tbx_packer_node* fit;
    struct tbx_packer_node* down;
    struct tbx_packer_node* right;

    bool used;
};

void arrange_begin(struct tbx_server* server, int workspace, int gap, int margin);
void arrange_end(struct tbx_server* server);
bool arrange_run(struct tbx_server* server);

#endif // TINYBOX_ARRANGE_H
