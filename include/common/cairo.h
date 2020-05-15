#ifndef TINYBOX_CAIRO_H
#define TINYBOX_CAIRO_H

#include <cairo/cairo.h>
#include <stdint.h>
#include <wayland-client-protocol.h>

void cairo_set_source_u32(cairo_t *cairo, uint32_t color);

cairo_subpixel_order_t
to_cairo_subpixel_order(enum wl_output_subpixel subpixel);

cairo_surface_t *cairo_image_surface_scale(cairo_surface_t *image, int width,
                                           int height);

void draw_gradient_rect(cairo_t *cx, int flags, int w, int h,
                        float color[static 4], float colorTo[static 4]);

cairo_surface_t *cairo_image_from_xpm(char *path);

#endif // TINYBOX_CAIRO_H