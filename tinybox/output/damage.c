#include "tinybox/damage.h"
#include "tinybox/server.h"

void damage_setup(struct tbx_server *server)
{
    wl_list_init(&server->damages);
    damage_whole(server);
}

void damage_add_box(struct tbx_server *server, struct wlr_box *box)
{}

void damage_add_view(struct tbx_server *server, struct tbx_view *view)
{}

void damage_whole(struct tbx_server *server)
{}

void damage_update(struct tbx_server *server)
{}

bool damage_check(struct tbx_server *server, struct wlr_box *box)
{
    return true;
}