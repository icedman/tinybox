
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

void menu_execute(struct tbx_server* server, struct tbx_menu *item)
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

cairo_surface_t* generate_item_texture(struct tbx_output* tbx_output, struct tbx_menu* menu, bool hilite) {
    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, menu->width, menu->height);
    cairo_t* cx = cairo_create(surface);

    // style
    struct tbx_style* style = &tbx_output->server->style;

    float color[4] = { 0.0, 0.0, 0, 0.0 };
    float colorTo[4] = { 0.0, 0.0, 0, 0.0 };

    uint32_t flags = 0;
    if (hilite) {
        flags = style->menu_hilite;
        color_to_rgba(color, style->menu_hilite_color);
        color_to_rgba(colorTo, style->menu_hilite_colorTo);
        draw_gradient_rect(cx, flags, menu->width, menu->height, color, colorTo);
    }

    cairo_translate(cx, 4, 2);
    cairo_rectangle(cx, 0, 0, menu->text_image_width, menu->text_image_height);

    // text
    if (hilite) {
        cairo_set_source_surface(cx, menu->text_hilite_image, 0, 0);
    } else {
        cairo_set_source_surface(cx, menu->text_image, 0, 0);
    }

    // cairo_fill(cx);
    cairo_paint(cx);

    cairo_destroy(cx);
    return surface;
}

struct wlr_texture* generate_menu_texture(struct tbx_output* tbx_output, struct tbx_menu* menu)
{
    // style
    struct tbx_style* style = &tbx_output->server->style;

    if (menu->menu_texture) {
        if (menu->lastStyleHash == style->hash) {
            return menu->menu_texture;
        }
        wlr_texture_destroy(menu->menu_texture);
    }

    struct wlr_renderer* renderer = tbx_output->server->renderer;
    menu->lastStyleHash = style->hash;

    char* font = style->font;
    float color[4] = { 1.0, 1.0, 0, 1.0 };
    float colorTo[4] = { 1.0, 1.0, 0, 1.0 };

    int menu_height = 0;
    int menu_width = 0;
    int title_height = 0;

    int tw = 400;
    int th = 32;
    color_to_rgba(color, style->menu_title_textColor);
    char *title = menu->title;
    if (!title) {
        title = menu->label;
    }
    cairo_surface_t *title_text = cairo_image_from_text(title, &tw, &th, font, color, 
        tbx_output->wlr_output->subpixel);

    menu_width = tw + 8;

    color_to_rgba(color, style->menu_frame_textColor);

    // generate item text textures
    struct tbx_command* cmd;
    wl_list_for_each_reverse(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        int w = 300;
        int h = 32;

        item->text_image = cairo_image_from_text(item->label, &w, &h, font, color, tbx_output->wlr_output->subpixel);
        w = 300; h = 32;
        item->text_hilite_image = cairo_image_from_text(item->label, &w, &h, font, color, tbx_output->wlr_output->subpixel);
        item->width = w + 8;
        item->height = h + 4;
        item->text_image_width = w;
        item->text_image_height = h;

        if (title_height == 0) {
            title_height = item->height;
        }

        item->y = menu_height;
        menu_height += item->height;
        if (menu_width < item->width) {
            menu_width = item->width;
        }
    }

    menu_height += title_height;

    menu->menu_width = menu_width;
    menu->menu_height = menu_height;

    // create the menu texture
    cairo_surface_t *menu_image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, menu_width, menu_height);
    cairo_t* cx = cairo_create(menu_image);

    // borders
    // render the title
    uint32_t flags = style->menu_title;
    color_to_rgba(color, style->menu_title_color);
    color_to_rgba(colorTo, style->menu_title_colorTo);
    draw_gradient_rect_xy(cx, flags, 0, 0, 
            menu_width, title_height, color, colorTo);

    // draw title text
    if (title_text) {
        cairo_save(cx);
        cairo_translate(cx, 4, 2);
        cairo_set_source_surface(cx, title_text, 0, 0);
        cairo_rectangle(cx, 0, 0, tw, th);
        cairo_fill(cx);
        cairo_restore(cx);
        cairo_surface_destroy(title_text);
    }

    // background
    flags = style->menu_frame;
    color_to_rgba(color, style->menu_frame_color);
    color_to_rgba(colorTo, style->menu_frame_colorTo);
    draw_gradient_rect_xy(cx, flags, 0, title_height, 
            menu_width, menu_height - title_height, color, colorTo);

    int item_offset_y = title_height;

    cairo_save(cx);

    cairo_translate(cx, 0, item_offset_y);
    wl_list_for_each_reverse(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        // make all item widths equal
        
        // create item with background
        item->width = menu_width;
        item->y += item_offset_y;

        cairo_surface_t *item_image = generate_item_texture(tbx_output, item, false);
        cairo_set_source_surface(cx, item_image, 0, 0);
        cairo_translate(cx, 0, item->height);
        cairo_paint(cx);
        cairo_surface_destroy(item_image);

        item_image = generate_item_texture(tbx_output, item, true);

        // save wlr_texture
        if (item->item_texture) {
            wlr_texture_destroy(item->item_texture);
        }

        unsigned char* data = cairo_image_surface_get_data(item_image);
        item->item_texture = wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
            cairo_image_surface_get_stride(item_image), item->width, item->height, data);

        cairo_surface_destroy(item_image);
        cairo_surface_destroy(item->text_image);
        cairo_surface_destroy(item->text_hilite_image);
        
    }
    cairo_restore(cx);
    cairo_destroy(cx);

    unsigned char* data = cairo_image_surface_get_data(menu_image);
    menu->menu_texture = wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
        cairo_image_surface_get_stride(menu_image), menu->menu_width, menu->menu_height, data);

    cairo_surface_destroy(menu_image);
    return menu->menu_texture;
}

static void render_menu(struct tbx_output* tbx_output, struct tbx_menu* menu)
{
    if (!menu->menu_type == TBX_MENU || !wl_list_length(&menu->items)) {
        return;
    }

    double ox = 0, oy = 0;
    wlr_output_layout_output_coords(tbx_output->server->output_layout, tbx_output->wlr_output, &ox,
        &oy);

    // console_log("m %d %d", menu->x, menu->y);

    struct wlr_output* output = tbx_output->wlr_output;
    // struct wlr_renderer* renderer = tbx_output->server->renderer;

    // style
    struct tbx_style* style = &tbx_output->server->style;

    struct wlr_box box;

    float color[4] = { 1, 0, 1, 1 };

    generate_menu_texture(tbx_output, menu);

    box.x = menu->menu_x + ox;
    box.y = menu->menu_y + oy;
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

    render_texture(output, &box, menu->menu_texture, output->scale);

    // hovered or submenu
    struct tbx_command* submenu;
    wl_list_for_each(submenu, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)submenu;
        if (item != menu->hovered && item != menu->submenu) {
            continue;
        }

        box.x = menu->menu_x + item->x + 3 + ox;
        box.y = menu->menu_y + item->y + 3 + oy;
        box.width = item->width;
        box.height = item->height;
        color_to_rgba(color, style->borderColor);
        // render_rect(output, &box, color, output->scale);

        render_texture(output, &box, item->item_texture, output->scale);
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

static void menu_focus_item(struct tbx_server *server, struct tbx_menu *item) {
    if (!item) {
        return;
    }
    server->menu_hovered = item;
    if (item->parent) {
        item->parent->hovered = item;
    }
}
static void menu_walk(struct tbx_server *server, struct tbx_menu *item, int dir_x, int dir_y) {
    if (!server->menu_navigation_grab) {
        return;
    }

    // first item
    if (!item) {
        item = server->menu_navigation_grab->hovered;
        if (!item) {
            struct tbx_command *cmd;
            wl_list_for_each_reverse(cmd, &server->menu_navigation_grab->items, link) {
                item = (struct tbx_menu*)cmd;
                menu_focus_item(server, item);
                return;
            }
        }
    }


    if (dir_y != 0) {
        struct tbx_menu *prev = NULL;
        struct tbx_command *cmd;
        wl_list_for_each_reverse(cmd, &server->menu_navigation_grab->items, link) {
            struct tbx_menu *n = (struct tbx_menu*)cmd;
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

void menu_navigation(struct tbx_server *server, uint32_t keycode)
{
    struct tbx_menu *item = server->menu_hovered;

    switch(keycode) {
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

void render_menus(struct tbx_output* output)
{
    struct tbx_server* server = output->server;
    render_menu_recursive(output, server->menu);
}