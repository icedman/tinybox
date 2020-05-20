
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

void menu_show(struct tbx_menu* menu, int x, int y, bool shown)
{
    if (!menu) {
        return;
    }

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
    if (menu->submenu == submenu) {
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

cairo_surface_t* generate_menu_texture(struct tbx_output* tbx_output, struct tbx_menu* menu)
{
    if (menu->menu_image) {
        return menu->menu_image;
    }

    // style
    struct tbx_style* style = &tbx_output->server->style;

    char* font = "monospace 10";
    float color[4] = { 1.0, 1.0, 0, 1.0 };
    float colorTo[4] = { 1.0, 1.0, 0, 1.0 };

    int menu_height = 0;
    int menu_width = 0;

    color_to_rgba(color, style->menu_frame_textColor);

    // generate item textures
    struct tbx_command* cmd;
    wl_list_for_each_reverse(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        int w = 300;
        int h = 32;

        if (!item->item_image) {
            item->item_image = cairo_image_from_text(item->label, &w, &h, font, color, tbx_output->wlr_output->subpixel);
            item->width = w;
            item->height = h;
        }

        item->y = menu_height;
        menu_height += item->height;
        if (menu_width < item->width) {
            menu_width = item->width;
        }
    }

    menu->menu_width = menu_width;
    menu->menu_height = menu_height;

    // create the menu texture
    menu->menu_image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, menu_width, menu_height);
    cairo_t* cx = cairo_create(menu->menu_image);

    // borders

    // title
    color_to_rgba(color, style->menu_frame_color);
    color_to_rgba(colorTo, style->menu_frame_colorTo);
    uint32_t flags = style->menu_frame;

    // background
    draw_gradient_rect(cx, flags, menu_width, menu_height, color, colorTo);

    // render each item
    cairo_save(cx);
    wl_list_for_each_reverse(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        // make all item widths equal
        item->width = menu_width;
        cairo_set_source_surface(cx, item->item_image, 0, 0);
        cairo_translate(cx, 0, item->height);
        cairo_paint(cx);
    }
    cairo_restore(cx);

    cairo_destroy(cx);
    return menu->menu_image;
}

static void render_menu(struct tbx_output* tbx_output, struct tbx_menu* menu)
{
    if (!menu->menu_type == TBX_MENU || !wl_list_length(&menu->items)) {
        return;
    }
    // console_log("m %d %d", menu->x, menu->y);

    struct wlr_output* output = tbx_output->wlr_output;
    struct wlr_renderer* renderer = tbx_output->server->renderer;

    // style
    struct tbx_style* style = &tbx_output->server->style;

    struct wlr_box box;

    float color[4] = { 1, 0, 1, 1 };

    generate_menu_texture(tbx_output, menu);

    box.x = menu->menu_x;
    box.y = menu->menu_y;
    box.width = menu->menu_width;
    box.height = menu->menu_height;

    int borderWidth = 3;

    box.width += (borderWidth * 2);
    box.height += (borderWidth * 2);

    color_to_rgba(color, style->borderColor);
    render_rect(output, &box, color, output->scale);

    box.x += borderWidth;
    box.y += borderWidth;
    box.width -= (borderWidth * 2);
    box.height -= (borderWidth * 2);

    unsigned char* data = cairo_image_surface_get_data(menu->menu_image);
    struct wlr_texture* menu_texture = wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
        cairo_image_surface_get_stride(menu->menu_image), menu->menu_width, menu->menu_height, data);

    render_texture(output, &box, menu_texture, output->scale);

    wlr_texture_destroy(menu_texture);

    // hovered or submenu
    struct tbx_command* submenu;
    wl_list_for_each(submenu, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)submenu;
        if (item != menu->hovered && item != menu->submenu) {
            continue;
        }

        box.x = menu->menu_x + item->x;
        box.y = menu->menu_y + item->y;
        box.width = item->width;
        box.height = item->height;
        color_to_rgba(color, style->borderColor);
        render_rect(output, &box, color, output->scale);
    }
}

static void render_menu_recursive(struct tbx_output* output, struct tbx_menu* menu)
{
    if (!menu->menu_type == TBX_MENU || !menu->shown) {
        return;
    }

    render_menu(output, menu);

    struct tbx_command* submenu;
    wl_list_for_each(submenu, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)submenu;
        if (item->menu_type == TBX_MENU && item->shown) {
            item->menu_x = menu->menu_x + menu->menu_width + item->x;
            item->menu_y = menu->menu_y + item->y;
            render_menu_recursive(output, item);
        }
    }
}

void render_menus(struct tbx_output* output)
{
    struct tbx_server* server = output->server;
    render_menu_recursive(output, server->menu);
}