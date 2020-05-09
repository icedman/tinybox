#include <stdint.h>
#include <cairo/cairo.h>
#include "tinybox/cairo.h"
#include "tinybox/style.h"

void cairo_set_source_u32(cairo_t *cairo, uint32_t color) {
    cairo_set_source_rgba(cairo,
            (color >> (3*8) & 0xFF) / 255.0,
            (color >> (2*8) & 0xFF) / 255.0,
            (color >> (1*8) & 0xFF) / 255.0,
            (color >> (0*8) & 0xFF) / 255.0);
}

cairo_subpixel_order_t to_cairo_subpixel_order(enum wl_output_subpixel subpixel) {
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

cairo_surface_t *cairo_image_surface_scale(cairo_surface_t *image,
        int width, int height) {
    int image_width = cairo_image_surface_get_width(image);
    int image_height = cairo_image_surface_get_height(image);

    cairo_surface_t *new =
        cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t *cairo = cairo_create(new);
    cairo_scale(cairo, (double)width / image_width,
            (double)height / image_height);
    cairo_set_source_surface(cairo, image, 0, 0);

    cairo_paint(cairo);
    cairo_destroy(cairo);
    return new;
}

void draw_gradient_rect(cairo_t *cx, int flags, int w, int h, float color[static 4], float colorTo[static 4]) {
    cairo_pattern_t *pat = 0;

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
        cairo_pattern_add_color_stop_rgb(pat, 0, color[0], color[1], color[2]);
        cairo_pattern_add_color_stop_rgb(pat, 1.0, colorTo[0], colorTo[1], colorTo[2]);

        cairo_rectangle (cx, 0.0, 0.0, w, h);
        cairo_set_source(cx, pat);
        cairo_fill (cx);
      
        cairo_pattern_destroy(pat);
    } else {

        cairo_rectangle (cx, 0.0, 0.0, w, h);
        cairo_set_source_rgb(cx, color[0], color[1], color[2]);
        cairo_fill (cx);
    }
};
