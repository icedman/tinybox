#include "tinybox/damage.h"
#include "tinybox/server.h"
#include "tinybox/view.h"

#include "pixman.h"
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/region.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OUTPUT_FPS 30
#define DAMAGE_LIFE (OUTPUT_FPS * 2)
#define DAMAGE_WHOLE (OUTPUT_FPS * 4)
#define MAX_DAMAGE_RECTS 120

static struct tbx_view damage_whole_view = { 0 };
// static int damage_regions = 0;

static bool ab_overlap(struct wlr_box *a, struct wlr_box *b) 
{ 
    struct wlr_box l1 = {
        .x = a->x - a->width/2,
        .y = a->y - a->height/2
    };
    struct wlr_box r1 = {
        .x = a->x + a->width/2,
        .y = a->y + a->height/2
    };

    struct wlr_box l2 = {
        .x = b->x - b->width/2,
        .y = b->y - b->height/2
    };
    struct wlr_box r2 = {
        .x = b->x + b->width/2,
        .y = b->y + b->height/2
    };

    if (l1.x >= r2.x || l2.x >= r1.x) 
        return false; 

    if (l1.y >= r2.y || l2.y >= r1.y) 
        return false; 
  
    return true; 
} 

void damage_setup(struct tbx_server* server)
{
    wl_list_init(&server->damages);
    
    struct wlr_box box = {
        .x = 0,
        .y = 0,
        .width = 1,
        .height = 1
    };
    damage_add_box(server, &box, &damage_whole_view);
    server->damage_whole = DAMAGE_WHOLE * 8; // longer initial damage
}

void damage_add_box(struct tbx_server* server, struct wlr_box* box, struct tbx_view* view)
{
    if (box->width == 0 || box->height == 0) {
        return;
    }

    struct tbx_damage *previous_damage = 0;
    struct tbx_damage *available = 0;
    struct tbx_damage *damage;
    wl_list_for_each(damage, &server->damages, link)
    {
        if (damage->view == view) {
            if (damage->region.x == box->x && damage->region.y == box->y && damage->region.width == box->width && damage->region.height == box->height) {
                damage->life = DAMAGE_LIFE;
                return;
            }

            if (!previous_damage || previous_damage->life < damage->life) {
                previous_damage = damage;
            }
        }
        if (damage->life <= 0) {
            available = damage;
        }
    }

    if (!available && previous_damage) {
        available = previous_damage;
    }

    if (!available) {
        if (wl_list_length(&server->damages) > MAX_DAMAGE_RECTS) {
            return;
        }
        available = calloc(1, sizeof(struct tbx_damage));
        wl_list_insert(&server->damages, &available->link);
    }

    memcpy(&available->region, box, sizeof(struct wlr_box));
    available->life = DAMAGE_LIFE;
    available->view = view;
}

void damage_add_view(struct tbx_server* server, struct tbx_view* view)
{
    struct wlr_box box;
    view_frame(view, &box);
    damage_add_box(server, &box, view);
}

void damage_add_commit(struct tbx_server* server, struct tbx_view* view)
{
    damage_add_view(server, view);
    return;

    #if 0
    struct wlr_box box;
    view->interface->get_geometry(view, &box);
    box.x = view->x;
    box.y = view->y;

    bool added = false;

    if (view && view->surface) {
        pixman_region32_t damage;
        pixman_region32_init(&damage);

        wlr_surface_get_effective_damage(view->surface, &damage);
        if (pixman_region32_not_empty(&damage)) {
            int nrects;
            pixman_box32_t* rects = pixman_region32_rectangles(&damage, &nrects);
            console_log("<< %d\n", nrects);

            // todo check problematic results from this query
            if (nrects > 4) {
                nrects = 0;
            }

            for (int i = 0; i < nrects; ++i) {

                struct wlr_box region = {
                    .x = rects[i].x1 + box.x,
                    .y = rects[i].y1 + box.y,
                    .width = rects[i].x2 - rects[i].x1,
                    .height = rects[i].y2 - rects[i].y1,
                };

                // console_log("%d %d %d %d\n", region.x, region.y, region.width, region.height);
                if (region.x < 0 || region.y < 0 || region.width > 1000 || region.height > 1000) {
                    // invalid?
                    break;
                }

                damage_add_box(server, &region, view);
                added = true;
            }
        }

        pixman_region32_fini(&damage);
    }

    if (!added) {
        damage_add_box(server, &box, view);
    }
    #endif
}

void damage_whole(struct tbx_server* server)
{
    server->damage_whole = DAMAGE_WHOLE;
}

bool damage_update(struct tbx_server* server, struct tbx_output* output, struct wl_list *regions)
{
    if (server->damage_whole > 0) {
        server->damage_whole--;
    }

    wl_list_init(regions);

    // pixman_region32_clear(&server->damage_region);    
    // pixman_box32_t boxes[MAX_DAMAGE_RECTS];
    int count = 0;
    // damage_regions = 0;

    struct tbx_damage *damage;
    wl_list_for_each(damage, &server->damages, link)
    {
        if (damage->view == &damage_whole_view) {
            wl_list_insert(regions, &damage->link2);
            continue;
        }

        if (server->damage_whole > 0) {
            damage->life = 0;
        }

        if (damage->life > 0) {
            damage->life--;
            if (damage->life <= 0) {
                damage->view = 0;
                damage->life = 0;
            }

            wl_list_insert(regions, &damage->link2);
            count++;
        }

        // boxes[count].x1 = damage->region.x;
        // boxes[count].y1 = damage->region.y;
        // if (boxes[count].x1 < 0) {
        //     boxes[count].x1 = 0;
        // }
        // if (boxes[count].y1 < 0) {
        //     boxes[count].y1 = 0;
        // }
        // boxes[count].x2 = boxes[count].x1 + damage->region.width;
        // boxes[count].y2 = boxes[count].y1 + damage->region.height;
        // if (damage->region.width > 2000 || damage->region.height > 2000) {
        //     continue;
        // }
        
        
    }

    // pixman_region32_init_rects(&server->damage_region, boxes, count);
    // damage_regions = count;

    return count > 0;
}

bool damage_check(struct tbx_server* server, struct wlr_box* box)
{
    if (server->damage_whole > 0) {
        return true;
    }

    struct tbx_damage *damage;
    wl_list_for_each(damage, &server->damages, link)
    {
        if (damage->view == &damage_whole_view) {
            continue;
        }

        if (damage->life < 0) {
            continue;
        }

        if (ab_overlap(box, &damage->region)) {
            return true;
        }
    }

    return false;
}

#if 0
bool damage_check(struct tbx_server* server, struct wlr_box* box)
{
    // return true;

    if (server->damage_whole > 0) {
        return true;
    }

    pixman_box32_t output_box = {
            .x1 = box->x,
            .y1 = box->y,
            .x2 = box->x + box->width,
            .y2 = box->y + box->height,
        };

    pixman_region32_t query;
    pixman_region32_init_rects(&query, &output_box, 1);

    bool res = false;
    if (pixman_region32_contains_rectangle(&server->damage_region, &output_box)) {
        res = true;
    }

    if (!res) {
        int nrects;
        pixman_box32_t* rects = pixman_region32_rectangles(&server->damage_region, &nrects);
        console_log("%d", nrects);
        for(int i=0; i<nrects; i++) {
            if (pixman_region32_contains_rectangle(&query, &rects[i])) {
                res = true;
                break;
            }            
        }

    }

    pixman_region32_fini(&query);
    return res;
}
#endif
