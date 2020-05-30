#include "tinybox/arrange.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/view.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>

typedef struct tbx_packer_node Node;

static struct wlr_box box_screen;
static struct wl_list boxes;
static int box_id = 0;
static bool split_screen;
static Node root;
static int max_width;

static int margin = 8;

static void resetRoot() {
    memset(&root, 0, sizeof(struct tbx_packer_node));
    root.x = margin;
    root.y = margin;
    root.w = box_screen.width - margin*2;
    root.h = box_screen.height - margin*2;
}

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
        if (block->fit) {
            continue;
        }
        Node* node = findNode(_root, block->w, block->h);
        if (node != NULL) {
            block->fit = splitNode(node, block->w, block->h);
        }
    }
}

#if 0
static void optimizeSizes()
{
    int widths[] = {
        (box_screen.width/2),
        (box_screen.width/3),
        (box_screen.width/3*2),
        0
    };

    struct tbx_packer_node* i;
    max_width = 0;
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
            i->view->interface->configure(i->view, i->x, i->y, i->w, i->h);
        }

        if (max_width < i->w) {
            max_width = i->w;
        }
    }
}
#endif

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

    view_frame(view, &geometry);
    geometry.width += margin/2;
    geometry.height += margin/2;

    Node* ab = createNode(view->x, view->y, geometry.width, geometry.height);
    ab->view = view;
    wl_list_insert(&boxes, &ab->link);
    
    struct tbx_output* output = view_get_preferred_output(view);

    struct wlr_box* main_box = wlr_output_layout_get_box(
        server->output_layout, output->wlr_output);

    memcpy(&box_screen, main_box, sizeof(struct wlr_box));
    resetRoot();

    if (view->width > box_screen.width / 2) {
        split_screen = false;
    }
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

void arrange_begin(struct tbx_server* server, int workspace, int gap, int margin)
{
    box_id = 0;
    split_screen = true;
    max_width = 0;
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

    // optimizeSizes();

    sortNodes();

    if (split_screen) {
        root.w = max_width;
    }
    fitNodes(&root);

    if (split_screen) {
        // second pass
        resetRoot();
        root.x += max_width;
        root.w -= max_width;
        fitNodes(&root);
    }

    struct tbx_packer_node* block;
    wl_list_for_each(block, &boxes, link)
    {
        if (block->fit) {
            block->view->x = block->fit->x + block->view->hotspots[HS_EDGE_LEFT].width;
            block->view->y = block->fit->y + block->view->hotspots[HS_EDGE_TOP].height
            + block->view->hotspots[HS_TITLEBAR].height;
        }
    }
    return true;
}