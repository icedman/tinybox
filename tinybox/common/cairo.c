#define _XOPEN_SOURCE 700 // for realpath

#include "common/cairo.h"
#include "common/pango.h"
#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/style.h"

#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cairo_set_source_u32(cairo_t* cairo, uint32_t color)
{
    cairo_set_source_rgba(cairo, (color >> (3 * 8) & 0xFF) / 255.0,
        (color >> (2 * 8) & 0xFF) / 255.0,
        (color >> (1 * 8) & 0xFF) / 255.0,
        (color >> (0 * 8) & 0xFF) / 255.0);
}

cairo_subpixel_order_t
to_cairo_subpixel_order(enum wl_output_subpixel subpixel)
{
    switch (subpixel) {
    case WL_OUTPUT_SUBPIXEL_HORIZONTAL_RGB:
        return CAIRO_SUBPIXEL_ORDER_RGB;
    case WL_OUTPUT_SUBPIXEL_HORIZONTAL_BGR:
        return CAIRO_SUBPIXEL_ORDER_BGR;
    case WL_OUTPUT_SUBPIXEL_VERTICAL_RGB:
        return CAIRO_SUBPIXEL_ORDER_VRGB;
    case WL_OUTPUT_SUBPIXEL_VERTICAL_BGR:
        return CAIRO_SUBPIXEL_ORDER_VBGR;
    default:
        return CAIRO_SUBPIXEL_ORDER_DEFAULT;
    }
    return CAIRO_SUBPIXEL_ORDER_DEFAULT;
}

cairo_surface_t* cairo_image_surface_scale(cairo_surface_t* image, int width,
    int height)
{
    int image_width = cairo_image_surface_get_width(image);
    int image_height = cairo_image_surface_get_height(image);

    cairo_surface_t* new = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cairo = cairo_create(new);
    cairo_scale(cairo, (double)width / image_width,
        (double)height / image_height);
    cairo_set_source_surface(cairo, image, 0, 0);

    cairo_paint(cairo);
    cairo_destroy(cairo);
    return new;
}

void draw_gradient_rect_xy(cairo_t* cx, int flags, int x, int y, int w, int h,
    float color[static 4], float colorTo[static 4])
{
    if (flags == 0) {
        cairo_save(cx);
        cairo_set_source_rgba(cx, 0.0, 0.0, 0.0, 0.0);
        cairo_set_operator(cx, CAIRO_OPERATOR_CLEAR);
        cairo_rectangle(cx, 0, 0, w, h);
        cairo_paint(cx);
        cairo_restore(cx);
        return;
    }

    cairo_pattern_t* pat = 0;

    cairo_set_antialias(cx, CAIRO_ANTIALIAS_BEST);

    if (flags & sf_gradient) {
        if (flags & sf_diagonal) {
            pat = cairo_pattern_create_linear(0.0, 0.0, w, h);
        } else if (flags & sf_crossdiagonal) {
            pat = cairo_pattern_create_linear(0.0, h, w, 0.0);
        } else {
            pat = cairo_pattern_create_linear(0.0, 0.0, w, 0.0);
        }
    }

    if (pat) {
        cairo_pattern_add_color_stop_rgba(pat, 0, color[0], color[1], color[2], color[3]);
        cairo_pattern_add_color_stop_rgba(pat, 1.0, colorTo[0], colorTo[1],
            colorTo[2], colorTo[3]);

        cairo_rectangle(cx, x, y, w, h);
        cairo_set_source(cx, pat);
        cairo_fill(cx);

        cairo_pattern_destroy(pat);
    } else {

        cairo_rectangle(cx, 0.0, 0.0, w, h);
        cairo_set_source_rgba(cx, color[0], color[1], color[2], color[3]);
        cairo_fill(cx);
    }
};

void draw_gradient_rect(cairo_t* cx, int flags, int w, int h,
    float color[static 4], float colorTo[static 4])
{
    draw_gradient_rect_xy(cx, flags, 0, 0, w, h, color, colorTo);
}

cairo_surface_t* cairo_image_from_xpm(char* path)
{
    printf("xpm: %s\n", path);
    FILE* f = fopen(path, "r");

    if (!f) {
        printf("unable to open %s\n", path);
        return 0;
    }

    uint32_t colors[255];
    char* line = NULL;

    int width = 0;
    int height = 0;
    int palette_size = 0;
    int index_size = 0;

    int argc;
    char** argv;

    int read_colors = 0;
    bool read_data = false;

    size_t line_size = 0;
    size_t nread;

    cairo_surface_t* surface = 0;
    int stride;
    unsigned char* data;

    int y = 0;
    while (!feof(f)) {
        nread = getline(&line, &line_size, f);
        if (!nread) {
            continue;
        }
        if (line[0] != '"') {
            continue;
        }

        strip_quotes(line);

        if (!palette_size) {
            // get dimensions
            argv = split_args(line, &argc);
            if (argc) {
                if (argc == 4) {
                    width = strtol(argv[0], NULL, 10);
                    height = strtol(argv[1], NULL, 10);
                    palette_size = strtol(argv[2], NULL, 10);
                    index_size = strtol(argv[3], NULL, 10);

                    read_colors = palette_size;
                }
                free_argv(argc, argv);
            }
            continue;
        }

        if (index_size > 2) {
            palette_size = 0;
            break;
        }

        if (read_colors-- > 0) {
            char color = line[0];

            line[0] = '_';
            argv = split_args(line, &argc);
            if (argc != 3) {
                palette_size = 0;
                break;
            }

            uint32_t _color = 0;
            argv[2][7] = 0;
            parse_color(argv[2], &_color);
            // printf("%c %s <\n", color, argv[2]);
            colors[(int)color] = _color;

            if (read_colors == 0) {
                read_data = true;
            }

            continue;
        }

        if (read_data) {

            if (!surface) {
                printf("create %d %d\n", width, height);
                surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, width * 4,
                    height * 4);
                if (cairo_surface_status(surface) != CAIRO_STATUS_SUCCESS) {
                    printf("error creating surface\n");
                    break;
                }
                cairo_surface_flush(surface);
                stride = cairo_image_surface_get_stride(surface);
                data = cairo_image_surface_get_data(surface);
            }

            if (!data) {
                break;
            }

            printf("%s", line);

            uint32_t* row = (void*)data;
            row += (y * stride / 4);

            for (int i = 0; i < width; i++) {
                char color = line[i * index_size];
                uint32_t _color = colors[(int)color];

                int r = ((_color >> 24) & 0xff);
                int g = ((_color >> 16) & 0xff);
                int b = ((_color >> 8) & 0xff);

                // row[i] = (r << 24) | (g << 16) | (b << 8) | 0xff;;
                row[i] = (r << 16) | (g << 8) | (b << 0); // | 0xff;
            }
            // printf("\n");

            y++;
        }
    }

    fclose(f);

    if (palette_size == 0) {
        printf("unable to parse %s\n", path);
        return 0;
    }

    // char fname[255] = "";
    // sprintf(fname, "%s.png", path);
    // sprintf(fname, "/tmp/xpm.png");

    if (cairo_surface_get_type(surface) != CAIRO_SURFACE_TYPE_IMAGE) {
        printf("fail\n");
    }

    cairo_surface_mark_dirty(surface);
    // cairo_surface_write_to_png(surface, fname);

    printf("%d %d %d %d\n", width, height, palette_size, index_size);
    return surface;
}

// cairo_image_from_xpm("/home/iceman/Developer/wm/tinybox/styles/fb/arch/pixmaps/bullet.xpm");
// cairo_image_from_xpm("/home/iceman/Developer/wm/tinybox/styles/fb/BlueFlux/pixmaps/title_bar.xpm");
// cairo_image_from_xpm("/home/iceman/Developer/wm/tinybox/styles/fb/arch/pixmaps/closefcs.xpm");

cairo_surface_t* cairo_image_from_text(char* text,
    int* width, int* height,
    char* font, float color[static 4], enum wl_output_subpixel subpixel)
{

    float scale = 1.0f;
    int w = 400;
    int h = 32;

    int max_width = *width;
    int max_height = *height;

    // We must use a non-nil cairo_t for cairo_set_font_options to work.
    // Therefore, we cannot use cairo_create(NULL).
    cairo_surface_t* dummy_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 0, 0);
    cairo_t* c = cairo_create(dummy_surface);
    cairo_set_antialias(c, CAIRO_ANTIALIAS_BEST);
    cairo_font_options_t* fo = cairo_font_options_create();
    cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_FULL);

    if (subpixel == WL_OUTPUT_SUBPIXEL_NONE) {
        cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_GRAY);
    } else {
        cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_SUBPIXEL);
        cairo_font_options_set_subpixel_order(
            fo, to_cairo_subpixel_order(subpixel));
    }

    cairo_set_font_options(c, fo);
    get_text_size(c, font, &w, &h, NULL, scale, true, "%s", text);
    cairo_surface_destroy(dummy_surface);
    cairo_destroy(c);

    if (w > max_width) {
        w = max_width;
    }
    if (h > max_height) {
        h = max_height;
    }
    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cairo_t* cx = cairo_create(surf);

    cairo_set_font_options(cx, fo);
    cairo_font_options_destroy(fo);

    PangoContext* pango = pango_cairo_create_context(cx);
    cairo_move_to(cx, 0, 0);

    // color_to_rgba(color, style->window_label_focus_textColor);
    cairo_set_source_rgba(cx, color[0], color[1], color[2], color[3]);
    pango_printf(cx, font, scale, true, "%s", text);

    g_object_unref(pango);
    cairo_destroy(cx);

    *width = w;
    *height = h;
    return surf;
}