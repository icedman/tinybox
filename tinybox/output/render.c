
#include "tinybox/render.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/style.h"
#include "tinybox/view.h"
#include "common/util.h"
#include "common/cairo.h"

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

static struct wlr_texture *textCache[16] = {
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL
};

static int lastCacheHash = 0;

struct wlr_texture *get_texture_cache(int idx)
{
  return textCache[idx];
}

static void generate_texture(struct wlr_renderer *renderer, int idx, int flags, int w, int h, float color[static 4], float colorTo[static 4]) {
  // printf("generate texture %d\n", idx);

  if (textCache[idx]) {
    wlr_texture_destroy(textCache[idx]);
    textCache[idx] = NULL;
  }

  if (flags == 0) {
    return;
  }
  
  cairo_surface_t *surf = cairo_image_surface_create(
      CAIRO_FORMAT_ARGB32, w, h);
  cairo_t *cx = cairo_create(surf);

  draw_gradient_rect(cx, flags, w, h, color, colorTo);

  unsigned char *data = cairo_image_surface_get_data(surf);
  textCache[idx] = wlr_texture_from_pixels(renderer,
      WL_SHM_FORMAT_ARGB8888,
      cairo_image_surface_get_stride(surf),
      w, h, data);

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
  generate_texture(renderer, tx_window_title_focus, flags, 512, 16, color, colorTo);

  color_to_rgba(color, style->window_title_unfocus_color);
  color_to_rgba(colorTo, style->window_title_unfocus_colorTo);
  flags = style->window_title_unfocus;
  generate_texture(renderer, tx_window_title_unfocus, flags, 512, 16, color, colorTo);

  // titlebar/label
  color_to_rgba(color, style->window_label_focus_color);
  color_to_rgba(colorTo, style->window_label_focus_colorTo);
  flags = style->window_label_focus;
  generate_texture(renderer, tx_window_label_focus, flags, 512, 16, color, colorTo);

  color_to_rgba(color, style->window_label_unfocus_color);
  color_to_rgba(colorTo, style->window_label_unfocus_colorTo);
  flags = style->window_label_unfocus;
  generate_texture(renderer, tx_window_label_unfocus, flags, 512, 16, color, colorTo);

  // handle
  color_to_rgba(color, style->window_handle_focus_color);
  color_to_rgba(colorTo, style->window_handle_focus_colorTo);
  flags = style->window_handle_focus;
  generate_texture(renderer, tx_window_handle_focus, flags, 512, 16, color, colorTo);

  color_to_rgba(color, style->window_handle_unfocus_color);
  color_to_rgba(colorTo, style->window_handle_unfocus_colorTo);
  flags = style->window_handle_unfocus;
  generate_texture(renderer, tx_window_handle_unfocus, flags, 512, 16, color, colorTo);

  // grip
  color_to_rgba(color, style->window_grip_focus_color);
  color_to_rgba(colorTo, style->window_grip_focus_colorTo);
  flags = style->window_grip_focus;
  generate_texture(renderer, tx_window_grip_focus, flags, 30, 16, color, colorTo);

  color_to_rgba(color, style->window_grip_unfocus_color);
  color_to_rgba(colorTo, style->window_grip_unfocus_colorTo);
  flags = style->window_grip_unfocus;
  generate_texture(renderer, tx_window_grip_unfocus, flags, 30, 16, color, colorTo);
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
