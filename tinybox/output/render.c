
#include "tinybox/render.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/style.h"
#include "tinybox/util.h"
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
