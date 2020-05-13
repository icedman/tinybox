
#include "tinybox/render.h"
#include "common/cairo.h"
#include "common/pango.h"
#include "common/util.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/style.h"
#include "tinybox/view.h"

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

static struct wlr_texture *textCache[16] = {NULL, NULL, NULL, NULL, NULL, NULL,
                                            NULL, NULL, NULL, NULL, NULL, NULL,
                                            NULL, NULL, NULL, NULL};

static int lastCacheHash = 0;

struct wlr_texture *get_texture_cache(int idx) {
  return textCache[idx];
}

static void generate_texture(struct wlr_renderer *renderer, int idx, int flags,
                             int w, int h, float color[static 4],
                             float colorTo[static 4]) {
  // printf("generate texture %d\n", idx);

  if (textCache[idx]) {
    wlr_texture_destroy(textCache[idx]);
    textCache[idx] = NULL;
  }

  if (flags == 0) {
    return;
  }

  cairo_surface_t *surf = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
  cairo_t *cx = cairo_create(surf);

  draw_gradient_rect(cx, flags, w, h, color, colorTo);

  unsigned char *data = cairo_image_surface_get_data(surf);
  textCache[idx] =
      wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
                              cairo_image_surface_get_stride(surf), w, h, data);

  // char fname[255] = "";
  // sprintf(fname, "/tmp/text_%d.png", idx);
  // cairo_surface_write_to_png(surf, fname);

  cairo_destroy(cx);
  cairo_surface_destroy(surf);
};

void generate_textures(struct tbx_output *output, bool forced) {
  struct tbx_style *style = &output->server->style;
  struct wlr_renderer *renderer = output->server->renderer;

  if (textCache[0] != NULL && !(forced || lastCacheHash != style->hash)) {
    return;
  }

  lastCacheHash = style->hash;

  float color[4];
  float colorTo[4];
  int flags;

  // titlebar
  color_to_rgba(color, style->window_title_focus_color);
  color_to_rgba(colorTo, style->window_title_focus_colorTo);
  flags = style->window_title_focus;
  generate_texture(renderer, tx_window_title_focus, flags, 512, 16, color,
                   colorTo);

  color_to_rgba(color, style->window_title_unfocus_color);
  color_to_rgba(colorTo, style->window_title_unfocus_colorTo);
  flags = style->window_title_unfocus;
  generate_texture(renderer, tx_window_title_unfocus, flags, 512, 16, color,
                   colorTo);

  // titlebar/label
  color_to_rgba(color, style->window_label_focus_color);
  color_to_rgba(colorTo, style->window_label_focus_colorTo);
  flags = style->window_label_focus;
  generate_texture(renderer, tx_window_label_focus, flags, 512, 16, color,
                   colorTo);

  color_to_rgba(color, style->window_label_unfocus_color);
  color_to_rgba(colorTo, style->window_label_unfocus_colorTo);
  flags = style->window_label_unfocus;
  generate_texture(renderer, tx_window_label_unfocus, flags, 512, 16, color,
                   colorTo);

  // handle
  color_to_rgba(color, style->window_handle_focus_color);
  color_to_rgba(colorTo, style->window_handle_focus_colorTo);
  flags = style->window_handle_focus;
  generate_texture(renderer, tx_window_handle_focus, flags, 512, 16, color,
                   colorTo);

  color_to_rgba(color, style->window_handle_unfocus_color);
  color_to_rgba(colorTo, style->window_handle_unfocus_colorTo);
  flags = style->window_handle_unfocus;
  generate_texture(renderer, tx_window_handle_unfocus, flags, 512, 16, color,
                   colorTo);

  // grip
  color_to_rgba(color, style->window_grip_focus_color);
  color_to_rgba(colorTo, style->window_grip_focus_colorTo);
  flags = style->window_grip_focus;
  generate_texture(renderer, tx_window_grip_focus, flags, 30, 16, color,
                   colorTo);

  color_to_rgba(color, style->window_grip_unfocus_color);
  color_to_rgba(colorTo, style->window_grip_unfocus_colorTo);
  flags = style->window_grip_unfocus;
  generate_texture(renderer, tx_window_grip_unfocus, flags, 30, 16, color,
                   colorTo);
}

void generate_view_title_texture(struct tbx_output *output,
                                 struct tbx_view *view) {
  struct tbx_style *style = &output->server->style;
  struct wlr_renderer *renderer =
      wlr_backend_get_renderer(output->wlr_output->backend);

  if (view->title) {
    wlr_texture_destroy(view->title);
    wlr_texture_destroy(view->title_unfocused);
    view->title = NULL;
    view->title_unfocused = NULL;
  }

  const char *font = style->font;

  char title[128];
  char appId[64];
  snprintf(title, 128, "%s", get_string_prop(view, VIEW_PROP_TITLE));
  snprintf(appId, 64, "%s", get_string_prop(view, VIEW_PROP_APP_ID));

  console_log("%s::%s", title, appId);

  if (strlen(title) == 0) {
    view->title_dirty = false;
    return;
  }

  // console_log("%s %s", appId, title);

  float scale = 1.0f;
  int w = 400;
  int h = 32;

  // We must use a non-nil cairo_t for cairo_set_font_options to work.
  // Therefore, we cannot use cairo_create(NULL).
  cairo_surface_t *dummy_surface =
      cairo_image_surface_create(WL_SHM_FORMAT_ARGB8888, 0, 0);
  cairo_t *c = cairo_create(dummy_surface);
  cairo_set_antialias(c, CAIRO_ANTIALIAS_BEST);
  cairo_font_options_t *fo = cairo_font_options_create();
  cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_FULL);
  if (output->wlr_output->subpixel == WL_OUTPUT_SUBPIXEL_NONE) {
    cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_GRAY);
  } else {
    cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_SUBPIXEL);

    // cairo.c
    cairo_font_options_set_subpixel_order(
        fo, to_cairo_subpixel_order(output->wlr_output->subpixel));
  }
  cairo_set_font_options(c, fo);
  get_text_size(c, font, &w, NULL, NULL, scale, true, "%s", title);
  cairo_surface_destroy(dummy_surface);
  cairo_destroy(c);

  float color[4];

  cairo_surface_t *surf =
      cairo_image_surface_create(WL_SHM_FORMAT_ARGB8888, w, h);
  cairo_t *cx = cairo_create(surf);

  cairo_set_font_options(cx, fo);
  cairo_font_options_destroy(fo);

  PangoContext *pango = pango_cairo_create_context(cx);
  cairo_move_to(cx, 0, 0);

  color_to_rgba(color, style->window_label_focus_textColor);
  cairo_set_source_rgba(cx, color[0], color[1], color[2], color[3]);
  pango_printf(cx, font, scale, true, "%s", title);

  unsigned char *data = cairo_image_surface_get_data(surf);

  view->title =
      wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
                              cairo_image_surface_get_stride(surf), w, h, data);

  // clear
  cairo_save(cx);
  cairo_set_source_rgba(cx, 0.0, 0.0, 0.0, 0.0);
  cairo_set_operator(cx, CAIRO_OPERATOR_CLEAR);
  cairo_rectangle(cx, 0, 0, w, h);
  cairo_paint(cx);
  cairo_restore(cx);

  color_to_rgba(color, style->window_label_unfocus_textColor);
  cairo_set_source_rgba(cx, color[0], color[1], color[2], color[3]);
  pango_printf(cx, font, scale, true, "%s", title);

  data = cairo_image_surface_get_data(surf);
  view->title_unfocused =
      wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
                              cairo_image_surface_get_stride(surf), w, h, data);

  view->title_box.width = w;
  view->title_box.height = h;
  view->title_dirty = false;

  // char fname[255] = "";
  // sprintf(fname, "/tmp/text_%s.png", appId);
  // cairo_surface_write_to_png(surf, fname);

  g_object_unref(pango);
  cairo_destroy(cx);
  cairo_surface_destroy(surf);
}

void render_rect(struct wlr_output *output, struct wlr_box *box, float color[4],
                 float scale) {
  struct wlr_renderer *renderer = wlr_backend_get_renderer(output->backend);

  struct wlr_box box_scaled = {
      .x = box->x * scale,
      .y = box->y * scale,
      .width = box->width * scale,
      .height = box->height * scale,
  };

  wlr_render_rect(renderer, &box_scaled, color, output->transform_matrix);
}

void render_texture(struct wlr_output *output, struct wlr_box *box,
                    struct wlr_texture *texture, float scale) {
  if (!texture) {
    return;
  }

  struct wlr_renderer *renderer = wlr_backend_get_renderer(output->backend);

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
                         output->transform_matrix);

  wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0);
}
