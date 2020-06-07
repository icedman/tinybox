#include "tinybox/damage.h"
#include "tinybox/server.h"
#include "tinybox/view.h"

#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/region.h>
#include "pixman.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define DAMAGE_LIFE 4;

void damage_setup(struct tbx_server *server)
{
    wl_list_init(&server->damages);
    damage_whole(server);
}

void damage_add_box(struct tbx_server *server, struct wlr_box *box, struct tbx_view *view)
{
    if (wl_list_length(&server->damages) > 20) {
        return;
    }

    struct tbx_damage *damage, *tmp;
    wl_list_for_each_safe(damage, tmp, &server->damages, link) {
        if (damage->view == view) {
            if (damage->region.x == box->x && 
                damage->region.y == box->y &&
                damage->region.width == box->width &&
                damage->region.height == box->height) {
                damage->life = DAMAGE_LIFE;
                return;
            }
        }
    }

    struct tbx_damage *newDamage = calloc(1, sizeof(struct tbx_damage));
    memcpy(&newDamage->region, box, sizeof(struct wlr_box));
    newDamage->life = DAMAGE_LIFE;
    wl_list_insert(&server->damages, &newDamage->link); 
}

void damage_add_view(struct tbx_server *server, struct tbx_view *view)
{
    struct wlr_box box;
    view_frame(view, &box);
    damage_add_box(server, &box, view);
}

void damage_add_commit(struct tbx_server *server, struct tbx_view *view)
{
    // void wlr_surface_get_effective_damage(struct wlr_surface *surface,
    //     pixman_region32_t *damage);
    
    struct wlr_box box;
    view->interface->get_geometry(view, &box);
    box.x = view->x;
    box.y = view->y;
    
    if (view && view->surface) {
        pixman_region32_t damage;
        pixman_region32_init(&damage);
        // pixman_region32_copy(&damage, &view->surface->opaque_region);
        wlr_surface_get_effective_damage(view->surface, &damage);
        
        int nrects;
        pixman_box32_t *rects = pixman_region32_rectangles(&damage, &nrects);
        // console_log("<< %d\n", nrects);
        
        if (nrects > 4) {
            nrects = 0;
        }

        for (int i = 0; i < nrects; ++i) {;
            struct wlr_box region = {
                .x = rects[i].x1 + box.x,
                .y = rects[i].y1 + box.y,
                .width = rects[i].x2 - rects[i].x1,
                .height = rects[i].y2 - rects[i].y1,
            };

            // console_log("%d %d %d %d\n", region.x, region.y, region.width, region.height);

            if (region.x < 0 || region.y < 0 || region.width > 1000 || region.height > 1000) {
                nrects = 0;
                break;
            }

            damage_add_box(server, &region, view);
        }

        pixman_region32_fini(&damage);

        if (nrects) {
            return;
        }
    }

    damage_add_box(server, &box, view);
}

void damage_whole(struct tbx_server *server)
{}

void damage_update(struct tbx_server *server)
{
    struct tbx_damage *damage, *tmp;
    wl_list_for_each_safe(damage, tmp, &server->damages, link) {
        damage->life--;
        if (damage->life <= 0) {
            wl_list_remove(&damage->link);
            free(damage);
        }
    }
}

bool damage_check(struct tbx_server *server, struct wlr_box *box)
{
    return true;
}