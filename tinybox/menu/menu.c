
#include "tinybox/menu.h"
#include "common/cairo.h"
#include "common/pango.h"
#include "common/util.h"
#include "tinybox/damage.h"
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
    if (item->menu_type != TBX_MENU_ITEM || !item->execute) {
        return;
    }
    // console_log("pressed %s", item->execute);
    menu_close_all(server);
    struct tbx_command* ctx = server->command;
    command_execute(ctx, item->argc, item->argv);
}

void menu_schedule_close(struct tbx_menu* menu)
{
    menu->to_close = 4;
}

void menu_close(struct tbx_menu* menu)
{
    if (menu->parent && menu->parent->submenu == menu) {
        menu->parent->submenu = NULL;
    }

    struct tbx_view* view = (struct tbx_view*)&menu->view;
    struct tbx_server* server = view->server;

    if (menu->submenu && menu->submenu->shown) {
        menu_close(menu->submenu);
    }

    if (!menu->pinned) {

        console_log("close: %s", menu->label);

        menu->shown = false;
        menu->reversed = false;

        damage_add_view(server, view);

        if (menu == server->menu_hovered) {
            server->menu_hovered = NULL;
        }

        if (menu == server->menu_navigation_grab) {
            if (menu->parent && menu->parent->shown) {
                // console_log("transfer grab to parent %s", menu->parent->label);
                server->menu_navigation_grab = menu->parent;
                server->menu_hovered = menu;
            } else {
                console_log("ungrab: %s", menu->label);
                server->menu_navigation_grab = NULL;
            }
        }
    }
}

void menu_close_all(struct tbx_server* server)
{
    struct tbx_view* view;
    server->menu_hovered = NULL;

    wl_list_for_each(view, &server->menus, link)
    {
        struct tbx_menu_view* menu_view = (struct tbx_menu_view*)view;
        menu_close(menu_view->menu);
    }
}

void menu_show(struct tbx_server* server, struct tbx_menu* menu, int x, int y)
{
    if (!menu || menu->menu_type != TBX_MENU) {
        return;
    }

    server->menu_hovered = NULL;
    menu->hovered = NULL;

    // constraint to output
    struct tbx_view* view = (struct tbx_view*)&menu->view;
    view->x = x;
    view->y = y;
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

    server->menu_navigation_grab = menu;

    if (menu->shown) {
        // probably moving somewhere
        if (menu->submenu) {
            menu_close(menu->submenu);
        }
        return;
    }

    struct tbx_view* _view;
    wl_list_for_each(_view, &server->menus, link)
    {
        struct tbx_menu_view* menu_view = (struct tbx_menu_view*)_view;
        struct tbx_menu* _menu = menu_view->menu;
        if (_menu == menu) {
            // we're already on the list
            return;
        }
    }

    menu->shown = true;

    prerender_menu(server, menu, false);
    damage_add_view(server, view);

    wl_list_insert(&server->menus, &view->link);
    // console_log("show menu %d %d %d", menu->shown, menu->menu_x, menu->menu_y);
}

void menu_show_submenu(struct tbx_server* server, struct tbx_menu* menu, struct tbx_menu* submenu)
{
    if (submenu->menu_type != TBX_MENU) {
        return;
    }

    if (menu->submenu == submenu && submenu->shown) {
        return;
    }

    if (menu->submenu) {
        // hide previous
        menu_close(menu->submenu);
        menu->submenu = NULL;
    }

    struct tbx_command* cmd;
    wl_list_for_each(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        if (item == submenu) {
            menu->submenu = submenu;
            // prerender_menu(server, submenu, false);

            if (!submenu->pinned) {

                int new_x = menu->menu_x;
                submenu->reversed = menu->reversed;

                if (!menu->reversed) {
                    new_x += menu->menu_width + item->x + 3;
                } else {
                    new_x -= submenu->menu_width + 3;
                }

                menu_show(server, submenu,
                    new_x,
                    menu->menu_y + item->y);

                submenu->menu_y -= (item->height + 3);
                if (submenu->reversed != menu->reversed) {
                    menu->reversed = submenu->reversed;
                    menu_show(server, submenu, 0, 0); // hide.. so that it get reversed when displayed
                }
            } else {
                // just transfer grab
                // cmd->server->menu_navigation_grab = submenu;
            }

            break;
        }
    }
}

static struct tbx_menu* menu_at_items(struct tbx_menu* menu, int x, int y)
{
    if (!menu->menu_width || !menu->menu_height) {
        return NULL;
    }

    menu->hovered = NULL;

    if ((x >= menu->menu_x && x <= menu->menu_x + menu->menu_width) && (y >= menu->menu_y && y <= menu->menu_y + menu->menu_height)) {

        struct tbx_view* view = (struct tbx_view*)&menu->view;
        view->hotspot = HS_NONE;

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

                if (menu->submenu && menu->hovered != menu->submenu) {
                    menu_close(menu->submenu);
                }
                return menu;
            }
        }

        // check titlebar
        struct wlr_box* titlebar = &view->hotspots[HS_TITLEBAR];

        if (titlebar && titlebar->width && titlebar->height) {
            if (x >= titlebar->x && x <= titlebar->x + titlebar->width && y >= titlebar->y && y <= titlebar->y + titlebar->height) {
                view->hotspot = HS_TITLEBAR;
                // console_log("hs %d %d", titlebar->width, titlebar->height);
                menu->hovered = NULL;
                return menu;
            }
        }
    }

    return NULL;
}

struct tbx_menu* menu_at(struct tbx_server* server, int x, int y)
{
    if (!server->menu) {
        return NULL;
    }

    server->menu_hovered = NULL;
    struct tbx_view* view;
    wl_list_for_each(view, &server->menus, link)
    {
        struct tbx_menu_view* menu_view = (struct tbx_menu_view*)view;
        struct tbx_menu* menu = menu_view->menu;
        if (menu->shown) {
            struct tbx_menu* res = menu_at_items(menu, x, y);
            if (res) {
                return res;
            }
        }
    }

    return NULL;
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

                if (dir_x == -1) {
                    menu_close(item->parent);
                }

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
            menu_show_submenu(server, item->parent, item);
            return;
        }
        if (dir_x == -1) {
            menu_close(item->parent);
        }
    }
}

void menu_navigation(struct tbx_server* server, uint32_t keycode)
{
    struct tbx_menu* item = server->menu_hovered;
    if (server->menu_navigation_grab) {
        item = server->menu_navigation_grab->hovered;
    }

    switch (keycode) {
    case XKB_KEY_Escape:
        menu_close_all(server);
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
    int borderWidth = 3;

    struct tbx_menu_view* menu_view = (struct tbx_menu_view*)view;
    box->x = box->y = 0;
    box->width = menu_view->menu->menu_width + (borderWidth << 1);
    box->height = menu_view->menu->menu_height + (borderWidth << 1);
}

static uint32_t menu_view_configure(struct tbx_view* view, double lx, double ly,
    int width, int height)
{
    struct tbx_menu_view* menu_view = (struct tbx_menu_view*)view;
    menu_view->menu->menu_x = lx;
    menu_view->menu->menu_y = ly;
    return 0;
}

struct tbx_view_interface menu_view_interface = {
    .get_geometry = menu_get_geometry,
    .configure = menu_view_configure,
};

void menu_show_tooltip(struct tbx_server* server, const char* text)
{
    if (!server->config.show_tooltip) {
        return;
    }

    static char tooltip[64];
    struct tbx_menu* menu;
    if (!server->tooltip) {
        menu = calloc(1, sizeof(struct tbx_menu));
        // struct tbx_command* m = &menu->command;
        menu_setup(server, menu);
        server->tooltip = menu;
    }

    menu = server->tooltip;
    menu->to_close = 30; // close within half a second

    if (!strlen(text)) {
        menu_close(server->tooltip);
    }

    struct tbx_view* view = (struct tbx_view*)&menu->view;
    if (menu->menu_width > 0) {
        damage_add_view(server, view);
    }

    if (text) {
        strcpy(tooltip, text);
        server->tooltip->title = tooltip;
        prerender_menu(server, server->tooltip, true);

        // constraint to output
        struct tbx_output* output = view_get_preferred_output(view);

        struct wlr_box* main_box = wlr_output_layout_get_box(
            server->output_layout, output->wlr_output);

        menu_show(server, server->tooltip,
            main_box->x + main_box->width / 2 - view->width / 2,
            main_box->y + main_box->height / 2 - view->height / 2);
    }
}

void menu_setup(struct tbx_server* server, struct tbx_menu* menu)
{
    menu->view.menu = menu;
    menu->menu_width = 0;
    menu->menu_height = 0;

    struct tbx_view* view = (struct tbx_view*)&menu->view;
    view->server = server;
    view->interface = &menu_view_interface;
    view->surface = NULL;
    view->view_type = VIEW_TYPE_UNKNOWN;

    wl_list_init(&menu->items);
}

void menu_focus(struct tbx_server* server, struct tbx_menu* menu)
{
    struct tbx_view* view = (struct tbx_view*)&menu->view;
    wl_list_remove(&view->link);
    wl_list_insert(&server->menus, &view->link);
}

struct tbx_menu* find_named_menu(struct tbx_server* server, const char* identifier)
{
    struct tbx_menu* menu;
    wl_list_for_each(menu, &server->named_menus, named_link)
    {
        console_log(">>%s", menu->title);
        if (strcmp(menu->label, identifier) == 0) {
            return menu;
        }
    }

    return NULL;
}