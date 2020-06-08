#include "tinybox/damage.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/view.h"

#include "pixman.h"

#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/region.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

bool region_overlap(struct wlr_box* a, struct wlr_box* b)
{
    struct wlr_box l1 = {
        .x = a->x,
        .y = a->y
    };
    struct wlr_box r1 = {
        .x = a->x + a->width,
        .y = a->y + a->height
    };

    struct wlr_box l2 = {
        .x = b->x,
        .y = b->y
    };
    struct wlr_box r2 = {
        .x = b->x + b->width,
        .y = b->y + b->height
    };

    if (l1.x >= r2.x || l2.x >= r1.x)
        return false;

    if (l1.y >= r2.y || l2.y >= r1.y)
        return false;

    return true;
}

void damage_setup(struct tbx_server* server)
{
}

void damage_add_box(struct tbx_server* server, struct wlr_box* box, struct tbx_view* view)
{
    struct tbx_output* output;
    wl_list_for_each(output, &server->outputs, link)
    {
        wlr_output_damage_add_box(output->damage, box);
    }
}

static void damage_surface(struct tbx_output *output,
        struct wlr_surface *surface, struct wlr_box *_box, bool whole) {

    struct wlr_box box = *_box;

    // scale_box(&box, output->wlr_output->scale);
    
    if (pixman_region32_not_empty(&surface->buffer_damage)) {
        pixman_region32_t damage;
        pixman_region32_init(&damage);
        wlr_surface_get_effective_damage(surface, &damage);
        wlr_region_scale(&damage, &damage, output->wlr_output->scale);
        if (ceil(output->wlr_output->scale) > surface->current.scale) {
            wlr_region_expand(&damage, &damage,
                ceil(output->wlr_output->scale) - surface->current.scale);
        }
        pixman_region32_translate(&damage, box.x, box.y);
        wlr_output_damage_add(output->damage, &damage);
        pixman_region32_fini(&damage);
    }

    if (whole) {
        wlr_output_damage_add_box(output->damage, &box);
    }

    wlr_output_schedule_frame(output->wlr_output);
}

void damage_add_view(struct tbx_server* server, struct tbx_view* view)
{
    struct wlr_box box;
    view_frame(view, &box);

    /*
    if (!view->surface && view->view_type == VIEW_TYPE_UNKNOWN) {
        // damage_add_box(server, &box, view);
        // TODO: menus!
        damage_whole(server);
        return;
    }

    if (view->mapped && view->surface) {
        struct tbx_output* output;
        wl_list_for_each(output, &view->server->outputs, link)
        {
            damage_surface(output, view->surface, &box, false);
        }
    }
    */
    
    damage_whole(server);
}

void damage_add_commit(struct tbx_server* server, struct tbx_view* view)
{
    struct wlr_box box;
    view_frame(view, &box);

    struct tbx_output* output;
    wl_list_for_each(output, &view->server->outputs, link)
    {
        damage_surface(output, view->surface, &box, true);
    }
}

void damage_whole(struct tbx_server* server)
{
    struct tbx_output* output;
    wl_list_for_each(output, &server->outputs, link)
    {
        wlr_output_damage_add_whole(output->damage);
    }
}

bool damage_check(struct tbx_server* server, struct wlr_box* box)
{
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
