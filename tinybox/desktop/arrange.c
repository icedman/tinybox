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

typedef struct tbx_packer_node Node;

Node root;

static Node* createNode(int x, int y, int w, int h)
{
    Node* n = calloc(1, sizeof(Node));
    n->id = box_id++;
    n->x = x;
    n->y = y;
    n->w = w;
    n->h = h;
    n->view = NULL;
    n->fit = NULL;
    n->right = NULL;
    n->down = NULL;
    n->used = false;
    return n;
}

static Node* splitNode(Node* n, int w, int h)
{
    n->used = true;
    n->down = createNode(n->x, n->y + h, n->w, n->h - h);
    n->right = createNode(n->x + w, n->y, n->w - w, h);
    return n;
}

static Node* findNode(Node* r, int w, int h)
{
    if (r->used) {
        Node* rr = findNode(r->right, w, h);
        if (rr != NULL) {
            return rr;
        } else {
            return findNode(r->down, w, h);
        }
    } else if ((w <= r->w) && (h <= r->h)) {
        return r;
    }

    return NULL;
}

static void fitNodes(Node *_root)
{
    struct tbx_packer_node* block;
    wl_list_for_each(block, &boxes, link)
    {
        Node* node = findNode(_root, block->w, block->h);
        if (node != NULL) {
            block->fit = splitNode(node, block->w, block->h);
            block->view->x = block->fit->x + 4;
            block->view->y = block->fit->y + 4;
        }
    }
}

static void opitmizeSizes()
{
    int widths[] = {
        (box_screen.width/2),
        (box_screen.width/3),
        (box_screen.width/3*2),
        0
    };

    struct tbx_packer_node* i;
    wl_list_for_each(i, &boxes, link)
    {
        int closestD = 0;
        int closestIdx = -1;
        for(int j=0;;j++) {
            int w = widths[j];
            if (w == 0) {
                break;
            }

            int d = i->w - w;
            d = d * d;
            if (d < closestD || closestD == 0) {
                closestD = d;
                closestIdx = j;
            }
        }

        if (widths[closestIdx] < i->w) {
            i->w = widths[closestIdx];
            i->view->interface->configure(i->view, i->x, i->y,
                i->w - 16,
                i->h - i->view->hotspots[HS_TITLEBAR].height
                     - i->view->hotspots[HS_HANDLE].height
                     - (3 * 4)
                     - 1
                     // - (i->view->server->style.borderWidth * 4)
                     - (i->view->server->style.frameWidth * 2)
                );
        }
    }
}

static void sortNodes()
{
    struct tbx_packer_node* i;
    struct tbx_packer_node* j;

    wl_list_for_each(i, &boxes, link)
    {
        wl_list_for_each(j, &boxes, link)
        {
            float ia = i->w * i->h;
            float ja = j->w * j->h;
            if (ia > ja) {
                // swap
                struct tbx_view* tmp_view = i->view;
                struct wlr_box box;
                box.x = i->x;
                box.y = i->y;
                box.width = i->w;
                box.height = i->h;

                i->view = j->view;
                i->x = j->x;
                i->y = j->y;
                i->w = j->w;
                i->h = j->h;

                j->view = tmp_view;
                j->x = box.x;
                j->y = box.y;
                j->w = box.width;
                j->h = box.height;
            }
        }
    }
}

static void arrange_add_view(struct tbx_server* server, struct tbx_view* view)
{
    struct wlr_box geometry;
    view->interface->get_geometry(view, &geometry);

    Node* ab = createNode(view->x, view->y, geometry.width + 16, geometry.height + 46 + 8);
    ab->view = view;

    wl_list_insert(&boxes, &ab->link);
    
    struct tbx_output* output = view_get_preferred_output(view);

    struct wlr_box* main_box = wlr_output_layout_get_box(
        server->output_layout, output->wlr_output);

    memcpy(&box_screen, main_box, sizeof(struct wlr_box));

    root.x = 8;
    root.y = 38;
    root.w = box_screen.width - 16;
    root.h = box_screen.height - 46;

    center_screen.x = (box_screen.width / 2);
    center_screen.y = (box_screen.height / 2);
}

static void arrange_workspace(struct tbx_server* server, int workspace)
{
    struct tbx_view* view;
    wl_list_for_each(view, &server->views, link)
    {
        if (view->workspace == workspace && view->mapped) {
            arrange_add_view(server, view);
        }
    }
}

static void arrange_update(Node* ab)
{
    if (!ab) {
        return;
    }

    console_log("update %d", ab->id);
    
    float margin = 8;

    // borders
    if (ab->x < margin) {
        ab->x = margin;
    }
    if (ab->y < 30 + margin) {
        ab->y = 30 + margin;
    }
    if (ab->x > box_screen.width - margin) {
        ab->x = box_screen.width - margin;
    }
    if (ab->y > box_screen.height - margin - 8) {
        ab->y = box_screen.height - margin - 8;
    }
    
    if (ab->fit) {
        struct tbx_view *view = ab->fit->view;
        if (view) {
            view->x = ab->x;
            view->y = ab->y;
            console_log(">%d %f %f %f %f", ab->id, ab->x, ab->y);
        }
    }

    arrange_update(ab->right);
    arrange_update(ab->down);
}

void arrange_begin(struct tbx_server* server, int workspace, int gap, int margin)
{
    box_id = 0;
    memset(&root, 0, sizeof(struct tbx_packer_node));
    wl_list_init(&boxes);
    arrange_workspace(server, workspace);
}

void arrange_end(struct tbx_server* server)
{
    Node* ab;
    Node* tmp;
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

    opitmizeSizes();
    sortNodes();
    fitNodes(&root);

    struct tbx_packer_node* block;
    wl_list_for_each(block, &boxes, link)
    {
        if (block->fit) {
            block->x = block->fit->x;
            block->y = block->fit->y;
        }
        arrange_update(block);
    }
    return true;
}