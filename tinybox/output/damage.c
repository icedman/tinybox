#include "tinybox/damage.h"
#include "tinybox/output.h"
#include "tinybox/render.h"
#include "tinybox/server.h"
#include "tinybox/view.h"

#include "pixman.h"

#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_surface.h>
#include <wlr/util/region.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIEW_EARLY_LIFE 30

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

static void damage_add_surface(struct tbx_output* output,
    struct wlr_surface* surface, struct wlr_box* _box, bool whole)
{
    if (!output->enabled) {
        return;
    }

    struct wlr_box box;
    memcpy(&box, _box, sizeof(struct wlr_box));

    // double ox = 0, oy = 0;
    // wlr_output_layout_output_coords(output->server->output_layout, output->wlr_output, &ox, &oy);
    // box.x += ox;
    // box.y += oy;

    struct wlr_box *output_box = wlr_output_layout_get_box(output->server->output_layout, output->wlr_output);
    if (!region_overlap(&box, output_box)) {
        return;
    }

    // console_log("whole %d", whole);
    
    if (!whole) {
        // console_log("box: %d %d %d %d (%d %d)\n", box.x, box.y, box.width, box.height, (int)output_box->x, (int)output_box->y);

        // scale_box(&box, output->wlr_output->scale);

        if (!(surface->current.committed & WLR_SURFACE_STATE_SURFACE_DAMAGE)) {
            // console_log("failed validate 0\n");
            whole = true;
        }

        if (!whole && pixman_region32_not_empty(&surface->buffer_damage)) {
            pixman_region32_t damage;
            pixman_region32_init(&damage);
            wlr_surface_get_effective_damage(surface, &damage);

            int nrects = 0;
            pixman_box32_t* rects = 0;
            pixman_box32_t* extents = pixman_region32_extents(&damage);

            float w = extents->x2 - extents->x1;
            float h = extents->y2 - extents->y1;

            // validate
            if (extents->x1 < 0 || extents->y1 < 0 || w > box.width || h > box.height || w <= 0 || h <= 0) {
                whole = true;
                // console_log("failed validate 1\n");
            }

            if (!whole) {
                rects = pixman_region32_rectangles(&damage, &nrects);
            }

            // validate again
            if (nrects == 0 ||
                nrects > 20 || 
                !rects) {
                whole = true;
                // console_log("failed validate 2 %d\n", nrects);
            }

            if (!whole) {
                // console_log("extents: %d %d %d %d\n",
                //     extents->x1, extents->y1,
                //     extents->x2 - extents->x1, extents->y2 - extents->y1);

                wlr_region_scale(&damage, &damage, output->wlr_output->scale);
                if (ceil(output->wlr_output->scale) > surface->current.scale) {
                    wlr_region_expand(&damage, &damage,
                        ceil(output->wlr_output->scale) - surface->current.scale);
                }

                pixman_region32_translate(&damage, box.x - output_box->x, box.y - output_box->y);
                wlr_output_damage_add(output->damage, &damage);
            }

            pixman_region32_fini(&damage);
        }
    }

    if (whole) {
        // console_log("whole!");
        wlr_output_damage_add_box(output->damage, &box);
    }

    wlr_output_schedule_frame(output->wlr_output);
}

void damage_add_view(struct tbx_server* server, struct tbx_view* view)
{
    struct wlr_box box;
    view_frame(view, &box);

    if (view->view_type == VIEW_TYPE_UNKNOWN) {
        // damage_add_box(server, &box, view);
        // TODO: menus!
        damage_whole(server);
        return;
    }

    // todo handle subsurface
    if (view->view_type == VIEW_TYPE_XDG) {
    }

    // view->interface->get_geometry(view, &box);
    // box.x = view->x;
    // box.y = view->y;

    struct tbx_output* output;
    wl_list_for_each(output, &view->server->outputs, link)
    {
        // console_log("add view %d %d", box.x, box.y);
        damage_add_surface(output, view->surface, &box, true);
    }
}

void damage_add_commit(struct tbx_server* server, struct tbx_view* view)
{
    if (view->life < VIEW_EARLY_LIFE
        || view->view_type == VIEW_TYPE_XDG // problematic
        ) {
        // xdg problematic
        damage_add_view(server, view);
        return;
    }

    struct wlr_box box;

    view->interface->get_geometry(view, &box);
    box.x = view->x;
    box.y = view->y;

    struct tbx_output* output;
    wl_list_for_each(output, &view->server->outputs, link)
    {
        damage_add_surface(output, view->surface, &box, false);
    }
}

void damage_whole(struct tbx_server* server)
{
    struct tbx_output* output;
    wl_list_for_each(output, &server->outputs, link)
    {
        if (!output->enabled) {
            continue;
        }
        wlr_output_damage_add_whole(output->damage);
    }
}

bool damage_check(struct tbx_server* server, struct wlr_box* box)
{
    return false;
}
