
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

    menu->command.server->menu_hovered = NULL;
    menu->hovered = NULL;
    menu->shown = shown;
    menu->menu_x = x;
    menu->menu_y = y;

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
            menu_show(submenu,
                menu->menu_x + menu->menu_width + item->x,
                menu->menu_y + item->y, true);
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

struct tbx_menu* menut_at(struct tbx_server* server, int x, int y)
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
