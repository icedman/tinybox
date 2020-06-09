#ifndef DAMAGE_H
#define DAMAGE_H

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>

struct wlr_box;
struct tbx_server;
struct tbx_output;
struct tbx_view;

bool region_overlap(struct wlr_box *a, struct wlr_box *b);
void damage_setup(struct tbx_server *server);

void damage_add_view(struct tbx_server *server, struct tbx_view *view);
void damage_add_commit(struct tbx_server *server, struct tbx_view *view);

void damage_whole(struct tbx_server *server);

#endif // DAMAGE_H