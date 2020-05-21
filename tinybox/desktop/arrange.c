#include "tinybox/arrange.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/view.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>

static struct wlr_box box_screen;
static struct tbx_vec center_screen;
static struct wl_list boxes;
static int box_id = 0;

static bool ab_overlap(struct tbx_arrange_box *a, struct tbx_arrange_box *b) 
{ 
    struct tbx_vec l1 = {
        .x = a->x - a->width/2,
        .y = a->y - a->height/2
    };
    struct tbx_vec r1 = {
        .x = a->x + a->width/2,
        .y = a->y + a->height/2
    };

    struct tbx_vec l2 = {
        .x = b->x - b->width/2,
        .y = b->y - b->height/2
    };
    struct tbx_vec r2 = {
        .x = b->x + b->width/2,
        .y = b->y + b->height/2
    };

    if (l1.x >= r2.x || l2.x >= r1.x) 
        return false; 

    if (l1.y >= r2.y || l2.y >= r1.y) 
        return false; 
  
    return true; 
} 

static void vec_normalize(struct tbx_vec* vec)
{
    float d = sqrt((vec->x * vec->x) + (vec->y * vec->y));
    if (d == 0) {
        d = 0.01;
    }
    vec->x = vec->x / d;
    vec->y = vec->y / d;
    // console_log("%f", d);
}

static float vec_distance(struct tbx_vec* from, struct tbx_vec* to)
{
    float dx = (from->x - to->x);
    float dy = (from->y - to->y);
    return sqrt((dx * dx) + (dy * dy));
}

static void vec_dir_to_xy(struct tbx_vec* from, struct tbx_vec* to, struct tbx_vec* res)
{
    res->x = to->x - from->x;
    res->y = to->y - from->y;
    vec_normalize(res);
}

static void arrange_add_view(struct tbx_server* server, struct tbx_view* view)
{
    struct tbx_arrange_box* ab = calloc(1, sizeof(struct tbx_arrange_box));
    ab->id = box_id++;

    struct wlr_box geometry;
    view->interface->get_geometry(view, &geometry);

    ab->view = view;
    ab->x = view->x + geometry.width / 2;
    ab->y = view->y + geometry.height / 2;
    ab->width = geometry.width;
    ab->height = geometry.height;

    struct tbx_output* output = view_get_preferred_output(view);

    struct wlr_box* main_box = wlr_output_layout_get_box(
        server->output_layout, output->wlr_output);

    memcpy(&box_screen, main_box, sizeof(struct wlr_box));

    center_screen.x = (box_screen.width / 2);
    center_screen.y = (box_screen.height / 2);

    struct tbx_vec to_center;
    struct tbx_vec loc = {
        .x = ab->x,
        .y = ab->y
    };
    vec_dir_to_xy(&center_screen, &loc, &to_center);

    if (ab->x == center_screen.x || ab->y == center_screen.y) {
        ab->x += ab->id;
        ab->y += ab->id;
    }

    wl_list_insert(&boxes, &ab->link);
}

static void arrange_workspace(struct tbx_server* server, int workspace)
{
    struct tbx_view* view;
    wl_list_for_each(view, &server->views, link)
    {
        if (view->workspace == workspace) {
            arrange_add_view(server, view);
        }
    }
}

static void arrange_update(struct tbx_arrange_box* ab)
{
    float mag = 0;
    ab->x += ab->force.x * mag;
    ab->y += ab->force.y * mag;

    float margin = 8;

    // borders
    if (ab->x - (ab->width) / 2 < 0 + margin) {
        ab->x = ab->width/2 + margin;
    }
    if (ab->y - (ab->height) / 2 < 30 + margin) {
        ab->y = ab->height/2 + 30 + margin;
    }
    if (ab->x + (ab->width) / 2 > box_screen.width - margin) {
        ab->x = box_screen.width - (ab->width/2) - margin;
    }
    if (ab->y + (ab->height) / 2 > box_screen.height - margin - 8) {
        ab->y = box_screen.height - (ab->height/2) - margin - 8;
    }

    ab->view->x = ab->x - ab->width / 2;
    ab->view->y = ab->y - ab->height / 2;

    // console_log(">%d %f %f %f %f", ab->id, ab->x, ab->y);
}

static void arrange_repel(struct tbx_arrange_box* ab)
{
    struct tbx_vec force;
    struct tbx_arrange_box* other;
    wl_list_for_each(other, &boxes, link)
    {
        if (other == ab) {
            continue;
        }

        struct tbx_vec a = {
            .x = ab->x,
            .y = ab->y,
        };

        struct tbx_vec b = {
            .x = other->x,
            .y = other->y,
        };

        float d = vec_distance(&a, &b);
        if (d == 0) {
            a.x = ab->id;
        }

        if (ab_overlap(ab, other)) {
            float radx = (ab->width + other->width)/2; 
            float rady = (ab->height + other->height)/2; 

            float dx = sqrt((a.x - b.x)*(a.x - b.x));
            float dy = sqrt((a.y - b.y)*(a.y - b.y));
            
            struct tbx_vec rr;
            if (dx < radx) {
                rr.x = b.x - a.x;
            }
            if (dy < rady) {
                rr.y = b.y - a.y;
            }
            force.x -= rr.x;
            force.y -= rr.y;
            vec_normalize(&force);

            ab->x += force.x * radx * 0.9;
            ab->y += force.y * rady * 0.9;
        }
    }

    ab->force.x += force.x;
    ab->force.y += force.y;
    vec_normalize(&ab->force);
    // memcpy(&ab->force, &force, sizeof(struct tbx_vec));
}

void arrange_begin(struct tbx_server* server, int workspace, int gap, int margin)
{
    box_id = 0;
    wl_list_init(&boxes);
    arrange_workspace(server, workspace);
}

void arrange_end(struct tbx_server* server)
{
    struct tbx_arrange_box* ab;
    struct tbx_arrange_box* tmp;
    wl_list_for_each_safe(ab, tmp, &boxes, link)
    {
        free(ab);
    }
}

bool arrange_run(struct tbx_server* server)
{
    if (wl_list_length(&boxes) < 2) {
        return true;
    }

    for (int i = 0; i < 2000; i++) {
        // console_log("i:%d", i);
        struct tbx_arrange_box* ab;
        wl_list_for_each(ab, &boxes, link)
        {
            arrange_repel(ab);
        }
        wl_list_for_each(ab, &boxes, link)
        {
            arrange_update(ab);
        }
    }
    return true;
}