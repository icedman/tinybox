
#include "tinybox/render.h"
#include "common/cairo.h"
#include "common/pango.h"
#include "common/util.h"
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

static struct wlr_texture* texture_cache[tx_last];

static int lastStyleHash = 0;

void texture_cache_destroy()
{
    for (int idx = 0; idx < tx_last; idx++) {
        if (texture_cache[idx]) {
            wlr_texture_destroy(texture_cache[idx]);
            texture_cache[idx] = NULL;
        }
    }
}

struct wlr_texture* get_texture_cache(int idx)
{
    return texture_cache[idx];
}

static void generate_texture(struct wlr_renderer* renderer, int idx, int flags,
    int w, int h, float color[static 4],
    float colorTo[static 4])
{
    if (flags == 0) {
        return;
    }

    // printf("generate texture %d\n", idx);

    if (texture_cache[idx]) {
        wlr_texture_destroy(texture_cache[idx]);
        texture_cache[idx] = NULL;
    }

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cairo_t* cx = cairo_create(surf);

    draw_gradient_rect(cx, flags, w, h, color, colorTo);

    unsigned char* data = cairo_image_surface_get_data(surf);
    texture_cache[idx] = wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
        cairo_image_surface_get_stride(surf), w, h, data);

    // char fname[255] = "";
    // sprintf(fname, "/tmp/text_%d.png", idx);
    // cairo_surface_write_to_png(surf, fname);

    cairo_destroy(cx);
    cairo_surface_destroy(surf);
};

static void generate_texture_pixmap(struct wlr_renderer* renderer, int idx,
    int w, int h, char* pixmap)
{
    // printf("generate texture %d\n", idx);
    if (!pixmap) {
        return;
    }

    if (texture_cache[idx]) {
        wlr_texture_destroy(texture_cache[idx]);
        texture_cache[idx] = NULL;
    }

    if (!pixmap) {
        return;
    }

    cairo_surface_t* xpm = cairo_image_from_xpm(pixmap);
    if (xpm) {
        return;
    }

    cairo_surface_t* surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    cairo_t* cx = cairo_create(surf);

    console_log("we got ourselves a pixmap");

    /* draw_gradient_rect(cx, flags, w, h, color, colorTo); */

    unsigned char* data = cairo_image_surface_get_data(surf);
    texture_cache[idx] = wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
        cairo_image_surface_get_stride(surf), w, h, data);

    // char fname[255] = "";
    // sprintf(fname, "/tmp/text_%d.png", idx);
    // cairo_surface_write_to_png(surf, fname);

    cairo_destroy(cx);
    cairo_surface_destroy(surf);
};

void generate_textures(struct tbx_output* output, bool forced)
{
    struct tbx_style* style = &output->server->style;
    struct wlr_renderer* renderer = output->server->renderer;

    if (texture_cache[0] != NULL && !(forced || lastStyleHash != style->hash)) {
        return;
    }

    lastStyleHash = style->hash;

    float color[4];
    float colorTo[4];
    int flags;

    // bevels
    float bevelLight[4] = { 1.0, 1.0, 1.0, 0.4 };
    float bevelDark[4] = { 0.0, 0.0, 0.0, 0.4 };
    generate_texture(renderer, tx_bevel_light, sf_solid, 32, 32, bevelLight, bevelLight);
    generate_texture(renderer, tx_bevel_dark, sf_solid, 8, 8, bevelDark, bevelDark);

    // titlebar
    color_to_rgba(color, style->window_title_focus_color);
    color_to_rgba(colorTo, style->window_title_focus_colorTo);
    flags = style->window_title_focus;
    generate_texture(renderer, tx_window_title_focus, flags, 512, 16, color,
        colorTo);
    generate_texture_pixmap(renderer, tx_window_title_focus, 512, 16,
        style->window_title_focus_pixmap);

    color_to_rgba(color, style->window_title_unfocus_color);
    color_to_rgba(colorTo, style->window_title_unfocus_colorTo);
    flags = style->window_title_unfocus;
    generate_texture(renderer, tx_window_title_unfocus, flags, 512, 16, color,
        colorTo);
    generate_texture_pixmap(renderer, tx_window_title_unfocus, 512, 16,
        style->window_title_unfocus_pixmap);

    // titlebar/label
    color_to_rgba(color, style->window_label_focus_color);
    color_to_rgba(colorTo, style->window_label_focus_colorTo);
    flags = style->window_label_focus;
    generate_texture(renderer, tx_window_label_focus, flags, 512, 16, color,
        colorTo);
    generate_texture_pixmap(renderer, tx_window_label_focus, 512, 16,
        style->window_label_focus_pixmap);

    color_to_rgba(color, style->window_label_unfocus_color);
    color_to_rgba(colorTo, style->window_label_unfocus_colorTo);
    flags = style->window_label_unfocus;
    generate_texture(renderer, tx_window_label_unfocus, flags, 512, 16, color,
        colorTo);
    generate_texture_pixmap(renderer, tx_window_label_unfocus, 512, 16,
        style->window_label_unfocus_pixmap);

    // handle
    color_to_rgba(color, style->window_handle_focus_color);
    color_to_rgba(colorTo, style->window_handle_focus_colorTo);
    flags = style->window_handle_focus;
    generate_texture(renderer, tx_window_handle_focus, flags, 512, 16, color,
        colorTo);
    generate_texture_pixmap(renderer, tx_window_handle_focus, 512, 16,
        style->window_handle_focus_pixmap);

    color_to_rgba(color, style->window_handle_unfocus_color);
    color_to_rgba(colorTo, style->window_handle_unfocus_colorTo);
    flags = style->window_handle_unfocus;
    generate_texture(renderer, tx_window_handle_unfocus, flags, 512, 16, color,
        colorTo);
    generate_texture_pixmap(renderer, tx_window_handle_unfocus, 512, 16,
        style->window_handle_unfocus_pixmap);

    // grip
    color_to_rgba(color, style->window_grip_focus_color);
    color_to_rgba(colorTo, style->window_grip_focus_colorTo);
    flags = style->window_grip_focus;
    generate_texture(renderer, tx_window_grip_focus, flags, 30, 16, color,
        colorTo);
    generate_texture_pixmap(renderer, tx_window_grip_focus, 512, 16,
        style->window_grip_focus_pixmap);

    color_to_rgba(color, style->window_grip_unfocus_color);
    color_to_rgba(colorTo, style->window_grip_unfocus_colorTo);
    flags = style->window_grip_unfocus;
    generate_texture(renderer, tx_window_grip_unfocus, flags, 30, 16, color,
        colorTo);
    generate_texture_pixmap(renderer, tx_window_grip_unfocus, 512, 16,
        style->window_grip_unfocus_pixmap);
}

void generate_view_title_texture(struct tbx_output* output,
    struct tbx_view* view)
{
    struct tbx_style* style = &output->server->style;
    struct wlr_renderer* renderer = wlr_backend_get_renderer(output->wlr_output->backend);

    if (view->title) {
        wlr_texture_destroy(view->title);
        wlr_texture_destroy(view->title_unfocused);
        view->title = NULL;
        view->title_unfocused = NULL;
    }

    const char* font = style->font;
    float color[4] = { 1.0, 0.0, 0.0, 1.0 };

    char title[128];
    char appId[64];
    snprintf(title, 128, "%s", view->interface->get_string_prop(view, VIEW_PROP_TITLE));
    snprintf(appId, 64, "%s", view->interface->get_string_prop(view, VIEW_PROP_APP_ID));

    console_log("%s::%s", title, appId);

    if (strlen(title) == 0) {
        view->title_dirty = false;
        return;
    }

    // console_log("%s %s", appId, title);

    int borderWidth = 3;
    int margin = 4;

    int max_title_width = view->hotspots[HS_TITLEBAR].width - (borderWidth + margin) * 2;
    int w = max_title_width;
    int h = 32;

    // printf(">>%d\n", w);

    color_to_rgba(color, style->window_label_focus_textColor);
    cairo_surface_t* title1 = cairo_image_from_text((char*)title,
        &w, &h, (char*)font, color, output->wlr_output->subpixel);

    unsigned char* data = cairo_image_surface_get_data(title1);
    view->title = wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
        cairo_image_surface_get_stride(title1), w, h, data);
    view->title_box.width = w;
    view->title_box.height = h;

    w = max_title_width;
    h = 32;
    color_to_rgba(color, style->window_label_unfocus_textColor);
    cairo_surface_t* title2 = cairo_image_from_text((char*)title,
        &w, &h, (char*)font, color, output->wlr_output->subpixel);

    data = cairo_image_surface_get_data(title2);
    view->title_unfocused = wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
        cairo_image_surface_get_stride(title2), w, h, data);

    view->title_box.width = w;
    view->title_box.height = h;
    view->title_dirty = false;

    // char fname[255] = "";
    // sprintf(fname, "/tmp/text_%s.png", appId);
    // cairo_surface_write_to_png(surf, fname);

    cairo_surface_destroy(title1);
    cairo_surface_destroy(title2);
}

void generate_background(struct tbx_output* output,
    struct tbx_workspace* workspace)
{
    int texture_id = workspace->id + tx_workspace_1;
    if (texture_cache[texture_id]) {
        wlr_texture_destroy(texture_cache[texture_id]);
        texture_cache[texture_id] = NULL;
    }

    struct wlr_renderer* renderer = wlr_backend_get_renderer(output->wlr_output->backend);
    cairo_surface_t* surface = cairo_image_surface_create_from_png(workspace->background);
    if (!surface) {
        console_log("error loading");
        return;
    }

    int w = cairo_image_surface_get_width(surface);
    int h = cairo_image_surface_get_height(surface);

    console_log("loaded %s %d %d", workspace->background, w, h);

    unsigned char* data = cairo_image_surface_get_data(surface);
    texture_cache[texture_id] = wlr_texture_from_pixels(
        renderer, WL_SHM_FORMAT_ARGB8888, cairo_image_surface_get_stride(surface),
        w, h, data);

    char fname[255] = "";
    sprintf(fname, "/tmp/text_%s.png", workspace->background);
    cairo_surface_write_to_png(surface, fname);

    if (texture_cache[texture_id]) {
        console_log("we have an image");
    }
    cairo_surface_destroy(surface);
}

void render_texture(struct tbx_output* output, struct wlr_box* box,
    struct wlr_texture* texture, float scale)
{
    if (!texture) {
        return;
    }

    struct wlr_renderer* renderer = wlr_backend_get_renderer(output->wlr_output->backend);
    struct wlr_box box_scaled = {
        .x = box->x * scale,
        .y = box->y * scale,
        .width = box->width * scale,
        .height = box->height * scale,
    };

    struct wlr_gles2_texture_attribs attribs;
    wlr_gles2_texture_get_attribs(texture, &attribs);
    glBindTexture(attribs.target, attribs.tex);
    glTexParameteri(attribs.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    float matrix[9];
    wlr_matrix_project_box(matrix, &box_scaled, WL_OUTPUT_TRANSFORM_NORMAL, 0.0,
        output->wlr_output->transform_matrix);

    for(int i=0; i<output->scissors_count; i++) {
        scissor_output(output->wlr_output, output->scissors[i]);
        wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0);
    }

    wlr_renderer_scissor(renderer, 0);
    if (!output->scissors_count) {
        wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0);
    }

    glBindTexture(attribs.target, 0);
}

void render_rect(struct tbx_output* output, struct wlr_box* box, float color[4],
    float scale)
{
    struct wlr_renderer* renderer = wlr_backend_get_renderer(output->wlr_output->backend);

    struct wlr_box box_scaled = {
        .x = box->x * scale,
        .y = box->y * scale,
        .width = box->width * scale,
        .height = box->height * scale,
    };

    for(int i=0; i<output->scissors_count; i++) {
        scissor_output(output->wlr_output, output->scissors[i]);
        wlr_render_rect(renderer, &box_scaled, color, output->wlr_output->transform_matrix);
    }
    wlr_renderer_scissor(renderer, 0);
    if (!output->scissors_count) {
        wlr_render_rect(renderer, &box_scaled, color, output->wlr_output->transform_matrix);
    }
}

void render_rect_outline(struct tbx_output* output, struct wlr_box* box, float color[4],
    float width, int bevel, float scale)
{
    struct wlr_texture* bevelTop;
    struct wlr_texture* bevelBottom;

    if (bevel == 1) {
        bevelTop = get_texture_cache(tx_bevel_light);
        bevelBottom = get_texture_cache(tx_bevel_dark);
    } else {
        bevelBottom = get_texture_cache(tx_bevel_light);
        bevelTop = get_texture_cache(tx_bevel_dark);
    }

    // top
    struct wlr_box edge;
    memcpy(&edge, box, sizeof(struct wlr_box));
    edge.height = width;

    if (bevel) {
        render_texture(output, &edge, bevelTop, scale);
    } else {
        render_rect(output, &edge, color, scale);
    }

    // bottom
    edge.y = edge.y + box->height - width;
    if (bevel) {
        render_texture(output, &edge, bevelBottom, scale);
    } else {
        render_rect(output, &edge, color, scale);
    }

    // left
    memcpy(&edge, box, sizeof(struct wlr_box));
    edge.width = width;
    if (bevel) {
        render_texture(output, &edge, bevelTop, scale);
    } else {
        render_rect(output, &edge, color, scale);
    }

    // right
    edge.x = edge.x + box->width - width;
    if (bevel) {
        render_texture(output, &edge, bevelBottom, scale);
    } else {
        render_rect(output, &edge, color, scale);
    }
}

void scissor_output(struct wlr_output* wlr_output, struct wlr_box box)
{
    struct wlr_renderer* renderer = wlr_backend_get_renderer(wlr_output->backend);

    // assert(renderer);

    int ow, oh;
    wlr_output_transformed_resolution(wlr_output, &ow, &oh);

    enum wl_output_transform transform = wlr_output_transform_invert(wlr_output->transform);
    wlr_box_transform(&box, &box, transform, ow, oh);

    wlr_renderer_scissor(renderer, &box);
}

void grow_box_lrtb(struct wlr_box* box, int l, int r, int t, int b)
{
    box->x -= l;
    box->width += l + r;
    box->y -= t;
    box->height += t + b;
}

void grow_box_hv(struct wlr_box* box, int h, int v)
{
    grow_box_lrtb(box, h, h, v, v);
}
