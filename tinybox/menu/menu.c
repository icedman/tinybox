
#include "tinybox/menu.h"
#include "common/cairo.h"
#include "common/pango.h"
#include "common/util.h"
#include "tinybox/output.h"
#include "tinybox/render.h"
#include "tinybox/server.h"
#include "tinybox/style.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"

#include <time.h>
#include <unistd.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_shell.h>

#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <wlr/render/gles2.h>

void menu_execute(struct tbx_server* server, struct tbx_menu* item)
{
    if (!item->execute) {
        return;
    }

    console_log("pressed %s", item->execute);

    menu_show(server->menu, 0, 0, false);
    struct tbx_command* ctx = server->command;
    command_execute(ctx, item->argc, item->argv);
}

void menu_show(struct tbx_menu* menu, int x, int y, bool shown)
{
    if (!menu) {
        return;
    }

    if (menu->menu_type != TBX_MENU) {
        return;
    }

    shown = shown | menu->pinned;

    menu->command.server->menu_hovered = NULL;
    menu->hovered = NULL;
    menu->shown = shown;

    if (!shown) {
        menu->reversed = false;
    }

    if (menu->shown && menu->submenu && menu->submenu->shown) {
        menu_show(menu->submenu, 0, 0, false);
    }

    // constraint to output
    struct tbx_view* view = (struct tbx_view*)&menu->view;
    struct tbx_server* server = view->server;
    struct tbx_output* output = view_get_preferred_output(view);

    struct wlr_box* main_box = wlr_output_layout_get_box(
        server->output_layout, output->wlr_output);

    if (x < main_box->x) {
        x = main_box->x;
    }
    if (y < main_box->y) {
        y = main_box->y;
    }
    if (x + menu->menu_width > main_box->x + main_box->width) {
        x = main_box->x + main_box->width - menu->menu_width;
        menu->reversed = true;
    }
    if (y < main_box->y) {
        y = main_box->y;
    }
    if (y + menu->menu_height > main_box->y + main_box->height) {
        y = main_box->y + main_box->height - menu->menu_height;
    }

    if (!menu->pinned) {
        menu->menu_x = x;
        menu->menu_y = y;
    }

    if (menu->shown) {
        menu->command.server->menu_navigation_grab = menu;
    } else {
        menu->hovered = NULL;
        if (menu->submenu) {
            menu_show(menu->submenu, 0, 0, false);
            menu->submenu = NULL;
        }
        menu->command.server->menu_navigation_grab = menu->parent;
    }

    // console_log("show menu %d %d %d", menu->shown, menu->menu_x, menu->menu_y);
}

void menu_show_submenu(struct tbx_menu* menu, struct tbx_menu* submenu)
{
    if (menu->submenu == submenu && submenu->shown) {
        // already showing
        return;
    }

    if (menu->submenu) {
        // hide previous
        menu_show(menu->submenu, 0, 0, false);
    }

    struct tbx_command* cmd;
    wl_list_for_each(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        if (item == submenu) {
            menu->submenu = submenu;

            prerender_menu(cmd->server, submenu);

            if (!submenu->pinned) {
                
                int new_x = menu->menu_x;
                submenu->reversed = menu->reversed;

                if (!menu->reversed) {
                    new_x += menu->menu_width + item->x + 3;
                } else {
                    new_x -= submenu->menu_width + 3;
                }

                menu_show(submenu,
                    new_x,
                    menu->menu_y + item->y, true);
                submenu->menu_y -= (item->height + 3);

                if (submenu->reversed != menu->reversed) {
                    menu->reversed = submenu->reversed;
                    menu_show(submenu, 0, 0, false); // hide.. so that it get reversed when displayed

                }
            }
            return;
        }
    }
}

static struct tbx_menu* menu_at_recursive(struct tbx_menu* menu, int x, int y)
{
    if (!menu->menu_width || !menu->menu_height) {
        return NULL;
    }

    if (menu->shown && (x >= menu->menu_x && x <= menu->menu_x + menu->menu_width) && (y >= menu->menu_y && y <= menu->menu_y + menu->menu_height)) {

        // check titlebar
        struct tbx_view *view = (struct tbx_view*)&menu->view;
        struct wlr_box *titlebar = &view->hotspots[HS_TITLEBAR];
        
        if (titlebar) {
            if (x >= titlebar->x && x <= titlebar->x + titlebar->width && 
                y >= titlebar->y && y <= titlebar->y + titlebar->height) {
                view->hotspot = HS_TITLEBAR;
            menu->hovered = NULL;
            return menu;
            }
        }
    
        // were in menu,
        // check item hovered
        struct tbx_command* cmd;
        int px = menu->menu_x;
        int py = menu->menu_y;
        wl_list_for_each(cmd, &menu->items, link)
        {
            struct tbx_menu* item = (struct tbx_menu*)cmd;
            if ((x >= px + item->x && x <= px + item->x + item->width) && (y >= py + item->y && y <= py + item->y + item->height)) {
                menu->hovered = item;
                cmd->server->menu_hovered = item;
                break;
            }
        }

        return menu;
    }

    // check items
    // continue down the menu tree
    struct tbx_command* cmd;
    wl_list_for_each(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        if (item->menu_type == TBX_MENU) {
            struct tbx_menu* hit = menu_at_recursive(item, x, y);
            if (hit) {
                return hit;
            }
        }
    }
    return NULL;
}

struct tbx_menu* menu_at(struct tbx_server* server, int x, int y)
{
    server->menu->hovered = NULL;
    struct tbx_menu* res = menu_at_recursive(server->menu, x, y);
    if (res) {
        server->menu->hovered = res->hovered;
        if (!res->shown) {
            return NULL;
        }
    }
    return res;
}

static void menu_focus_item(struct tbx_server* server, struct tbx_menu* item)
{
    if (!item) {
        return;
    }
    server->menu_hovered = item;
    if (item->parent) {
        item->parent->hovered = item;
    }
}
static void menu_walk(struct tbx_server* server, struct tbx_menu* item, int dir_x, int dir_y)
{
    if (!server->menu_navigation_grab) {
        return;
    }

    // first item
    if (!item) {
        item = server->menu_navigation_grab->hovered;
        if (!item) {
            struct tbx_command* cmd;
            wl_list_for_each_reverse(cmd, &server->menu_navigation_grab->items, link)
            {
                item = (struct tbx_menu*)cmd;
                menu_focus_item(server, item);
                return;
            }
        }
    }

    if (dir_y != 0) {
        struct tbx_menu* prev = NULL;
        struct tbx_command* cmd;
        wl_list_for_each_reverse(cmd, &server->menu_navigation_grab->items, link)
        {
            struct tbx_menu* n = (struct tbx_menu*)cmd;
            if (dir_y == 1 && prev == item) {
                menu_focus_item(server, n);
                return;
            }
            if (dir_y == -1 && n == item) {
                menu_focus_item(server, prev);
                return;
            }
            prev = n;
        }
    }

    if (dir_x != 0) {
        if (dir_x == 1 && wl_list_length(&item->items) > 0) {
            menu_show_submenu(item->parent, item);
            return;
        }
        if (dir_x == -1) {
            menu_show(item->parent, 0, 0, false);
        }
    }
}

void menu_navigation(struct tbx_server* server, uint32_t keycode)
{
    struct tbx_menu* item = server->menu_hovered;

    switch (keycode) {
    case XKB_KEY_Escape:
        menu_show(server->menu, 0, 0, false);
        break;

    case XKB_KEY_space:
    case XKB_KEY_Return:
        menu_execute(server, item);
        break;

    case XKB_KEY_Up:
        menu_walk(server, item, 0, -1);
        break;

    case XKB_KEY_Down:
        menu_walk(server, item, 0, 1);
        break;

    case XKB_KEY_Left:
        menu_walk(server, item, -1, 0);
        break;
    case XKB_KEY_Right:
        menu_walk(server, item, 1, 0);
        break;
    }
}

static void menu_get_geometry(struct tbx_view* view, struct wlr_box* box)
{
    struct tbx_menu_view *menu_view = (struct tbx_menu_view*)view;
    box->x = box->y = 0;
    box->width = menu_view->menu->menu_width;
    box->height = menu_view->menu->menu_height;
}

static uint32_t menu_view_configure(struct tbx_view* view, double lx, double ly,
    int width, int height)
{
    struct tbx_menu_view *menu_view = (struct tbx_menu_view*)view;
    menu_view->menu->menu_x = lx;
    menu_view->menu->menu_y = ly;
    return 0;
}

struct tbx_view_interface menu_view_interface = {
    .get_geometry = menu_get_geometry,
    .configure = menu_view_configure,
};

void menu_setup(struct tbx_server* server, struct tbx_menu *menu)
{
    menu->view.menu = menu;
    
    struct tbx_view *view = (struct tbx_view*)&menu->view;
    view->server = server;
    view->interface = &menu_view_interface;
    view->surface = NULL;
}