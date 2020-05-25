
#include "tinybox/render.h"
#include "common/cairo.h"
#include "common/pango.h"
#include "common/util.h"
#include "tinybox/menu.h"
#include "tinybox/output.h"
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

cairo_surface_t* generate_item_texture(struct tbx_output* tbx_output, struct tbx_menu* menu, bool hilite)
{
    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, menu->width, menu->height);
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

static struct wlr_texture* generate_menu_texture(struct tbx_output* tbx_output, struct tbx_menu* menu)
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

    int borderWidth = 3;
    int menu_height = 0;
    int menu_width = 0;
    int title_height = 0;

    int tw = 400;
    int th = 32;
    color_to_rgba(color, style->menu_title_textColor);
    char* title = menu->title;
    if (!title) {
        title = menu->label;
    }
    cairo_surface_t* title_text = cairo_image_from_text(title, &tw, &th, font, color,
        tbx_output->wlr_output->subpixel);

    menu_width = tw + 8;

    // generate item text textures
    struct tbx_command* cmd;
    wl_list_for_each_reverse(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        int w = 300;
        int h = 32;

        color_to_rgba(color, style->menu_frame_textColor);
        item->text_image = cairo_image_from_text(item->label, &w, &h, font, color, tbx_output->wlr_output->subpixel);
        w = 300;
        h = 32;

        color_to_rgba(color, style->menu_hilite_textColor);
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
    menu_height += borderWidth;

    menu->menu_width = menu_width;
    menu->menu_height = menu_height;

    // create the menu texture
    cairo_surface_t* menu_image = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, menu_width, menu_height);
    cairo_t* cx = cairo_create(menu_image);

    // borders
    // render the title
    uint32_t flags = style->menu_title;
    color_to_rgba(color, style->menu_title_color);
    color_to_rgba(colorTo, style->menu_title_colorTo);

    struct wlr_box title_box = {
        0, 0,
        menu_width,
        title_height
    };
    memcpy(&menu->title_box, &title_box, sizeof(struct wlr_box));
    
    draw_gradient_rect_xy(cx, flags, title_box.x, title_box.y, title_box.width, title_box.height, color, colorTo);

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

    struct wlr_box frame = {
        0, title_height + borderWidth,
        menu_width,
        menu_height - title_height - borderWidth
    };
    memcpy(&menu->frame_box, &frame, sizeof(struct wlr_box));
    draw_gradient_rect_xy(cx, flags, frame.x, frame.y, frame.width, frame.height, color, colorTo);

    int item_offset_y = title_height + borderWidth;

    cairo_save(cx);

    cairo_translate(cx, 0, item_offset_y);
    wl_list_for_each_reverse(cmd, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)cmd;
        // make all item widths equal

        // create item with background
        item->width = menu_width;
        item->y += item_offset_y;

        cairo_surface_t* item_image = generate_item_texture(tbx_output, item, false);
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

    // sync xy
    struct tbx_view *view = (struct tbx_view*)&menu->view;
    view->x = menu->menu_x;
    view->y = menu->menu_y;

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

    struct wlr_box box_frame;
    memcpy(&box_frame, &box, sizeof(struct wlr_box));
    box_frame.x += borderWidth;
    box_frame.y += borderWidth;

    box.width += (borderWidth * 2);
    box.height += (borderWidth * 2);

    color_to_rgba(color, style->borderColor);
    render_rect(output, &box, color, output->scale);

    box.x += borderWidth;
    box.y += borderWidth;
    box.width -= (borderWidth * 2);
    box.height -= (borderWidth * 2);

    render_texture(output, &box, menu->menu_texture, output->scale);

    // add the bevels
    int tflags = style->menu_frame;
    float bevelColor[4] = { 1, 0, 1, 1 };
    memcpy(&box, &menu->frame_box, sizeof(struct wlr_box));
    box.x += menu->menu_x + borderWidth;
    box.y += menu->menu_y + borderWidth;
    if (tflags & sf_raised) {
        render_rect_outline(output, &box, bevelColor, 1, 1, output->scale);
    } else if (tflags & sf_sunken) {
        render_rect_outline(output, &box, bevelColor, 1, -1, output->scale);
    }

    tflags = style->menu_title;
    memcpy(&box, &menu->title_box, sizeof(struct wlr_box));
    box.x += menu->menu_x + borderWidth;
    box.y += menu->menu_y + borderWidth;
    if (tflags & sf_raised) {
        render_rect_outline(output, &box, bevelColor, 1, 1, output->scale);
    } else if (tflags & sf_sunken) {
        render_rect_outline(output, &box, bevelColor, 1, -1, output->scale);
    }

    memcpy(&view->hotspots[HS_TITLEBAR], &box, sizeof(struct wlr_box));

    tflags = style->menu_hilite;

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

        if (tflags & sf_raised) {
            render_rect_outline(output, &box, bevelColor, 1, 1, output->scale);
        } else if (tflags & sf_sunken) {
            render_rect_outline(output, &box, bevelColor, 1, -1, output->scale);
        }
    }
}

static void render_menu_recursive(struct tbx_output* output, struct tbx_menu* menu)
{
    if (!menu->menu_type == TBX_MENU) {
        return;
    }

    if (menu->shown) {
        render_menu(output, menu);
    }

    // int borderWidth = 3;
    struct tbx_command* submenu;
    wl_list_for_each(submenu, &menu->items, link)
    {
        struct tbx_menu* item = (struct tbx_menu*)submenu;
        if (item->menu_type == TBX_MENU) {
            // item->menu_x = menu->menu_x + menu->menu_width + item->x + borderWidth;
            // item->menu_y = menu->menu_y + item->y - item->height - borderWidth;
            render_menu_recursive(output, item);
        }
    }
}

void render_menus(struct tbx_output* output)
{
    struct tbx_server* server = output->server;
    render_menu_recursive(output, server->menu);
}

void prerender_menu(struct tbx_server* server, struct tbx_menu *menu)
{
    generate_menu_texture(server->main_output, menu);

    struct tbx_view *view = (struct tbx_view*)&menu->view;
    view->width = menu->menu_width;
    view->height = menu->menu_height;
}