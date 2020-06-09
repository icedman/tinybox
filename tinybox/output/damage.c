#include "tinybox/damage.h"
#include "tinybox/output.h"
#include "tinybox/render.h"
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
        wlr_output_schedule_frame(output->wlr_output);
    }
}

static void damage_add_surface(struct tbx_output* output,
    struct wlr_surface* surface, struct wlr_box* _box, bool whole)
{
    struct wlr_box box = *_box;

    if (!whole) {
        printf("box: %d %d %d %d\n", box.x, box.y, box.width, box.height);

        // scale_box(&box, output->wlr_output->scale);

        if (!(surface->current.committed & WLR_SURFACE_STATE_SURFACE_DAMAGE)) {
            printf("failed validate 0\n");
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
                printf("failed validate 1\n");
            }

            if (!whole) {
                rects = pixman_region32_rectangles(&damage, &nrects);
            }

            // validate again
            if (nrects == 0 ||
                nrects > 20 || 
                !rects) {
                whole = true;
                printf("failed validate 2 %d\n", nrects);
            }

            if (!whole) {
                printf("extents: %d %d %d %d\n",
                    extents->x1, extents->y1,
                    extents->x2 - extents->x1, extents->y2 - extents->y1);

                wlr_region_scale(&damage, &damage, output->wlr_output->scale);
                if (ceil(output->wlr_output->scale) > surface->current.scale) {
                    wlr_region_expand(&damage, &damage,
                        ceil(output->wlr_output->scale) - surface->current.scale);
                }

                pixman_region32_translate(&damage, box.x, box.y);
                wlr_output_damage_add(output->damage, &damage);
                
                /*
                for (int i = 0; i < nrects; ++i) {
                    struct wlr_box region = {
                        .x = rects[i].x1 + box.x,
                        .y = rects[i].y1 + box.y,
                        .width = rects[i].x2 - rects[i].x1,
                        .height = rects[i].y2 - rects[i].y1,
                    };
                    wlr_output_damage_add_box(output->damage, &region);
                }
                */
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

    // view->interface->get_geometry(view, &box);
    // box.x = view->x;
    // box.y = view->y;

    struct tbx_output* output;
    wl_list_for_each(output, &view->server->outputs, link)
    {
        damage_add_surface(output, view->surface, &box, true);
    }
}

void damage_add_commit(struct tbx_server* server, struct tbx_view* view)
{
    if (view->life < 120
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
        wlr_output_damage_add_whole(output->damage);
    }
}

bool damage_check(struct tbx_server* server, struct wlr_box* box)
{
    return false;
}
