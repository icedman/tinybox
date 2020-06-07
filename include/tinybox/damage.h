#ifndef DAMAGE_H
#define DAMAGE_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>

struct wlr_box;
struct tbx_server;
struct tbx_output;
struct tbx_view;

struct tbx_damage {
    struct wl_list link;
    struct wlr_box region;
    int life;
    struct tbx_view *view;
};

void damage_setup(struct tbx_server *server);
void damage_add_box(struct tbx_server *server, struct wlr_box *box, struct tbx_view *view);
void damage_add_view(struct tbx_server *server, struct tbx_view *view);
void damage_add_commit(struct tbx_server *server, struct tbx_view *view);
void damage_whole(struct tbx_server *server);
void damage_update(struct tbx_server *server, struct tbx_output *output);
bool damage_check(struct tbx_server *server, struct wlr_box *box);

#endif // DAMAGE_H