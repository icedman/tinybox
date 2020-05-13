#define _POSIX_C_SOURCE 200112L
#include "tinybox/render.h"

#include "tinybox/output.h"
#include "tinybox/render.h"
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
#include <wlr/types/wlr_xcursor_manager.h>

#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <wlr/render/gles2.h>

/*
static void scissor_output(struct wlr_output *wlr_output,
    struct wlr_box box) {
  struct wlr_renderer *renderer = wlr_backend_get_renderer(wlr_output->backend);

  //assert(renderer);

  int ow, oh;
  wlr_output_transformed_resolution(wlr_output, &ow, &oh);

  enum wl_output_transform transform =
    wlr_output_transform_invert(wlr_output->transform);
  wlr_box_transform(&box, &box, transform, ow, oh);

  wlr_renderer_scissor(renderer, &box);
}
*/

static void grow_box_lrtb(struct wlr_box *box, int l, int r, int t, int b) {
  box->x -= l;
  box->width += l + r;
  box->y -= t;
  box->height += t + b;
}

static void grow_box_vh(struct wlr_box *box, int h, int v) {
  grow_box_lrtb(box, h, h, v, v);
}

static void render_view_decorations(struct wlr_surface *surface, int sx, int sy,
                                    void *data) {

  // renders the decorations and positions hotspots

  /* This function is called for every surface that needs to be rendered. */
  struct render_data *rdata = data;
  struct tbx_view *view = rdata->view;
  struct wlr_output *output = rdata->output;
  struct tbx_style *style = &view->server->style;

  float color[4] = {1, 0, 1, 1};

  float colorDebug1[4] = {0, 1, 1, 1};
  float colorDebug2[4] = {1, 0, 1, 1};
  float colorDebug3[4] = {1, 1, 1, 1};
  float colorDebug4[4] = {1, 0, 0, 1};

  if (colorDebug1[0] || colorDebug2[0] || colorDebug3[0] || colorDebug4[4]) {
    // suppress unused
  }

  struct wlr_box view_geometry;
  struct wlr_box box;

  wlr_xdg_surface_get_geometry((struct wlr_xdg_surface *)surface,
                               &view_geometry);

  /* The view has a position in layout coordinates. If you have two displays,
   * one next to the other, both 1080p, a view on the rightmost display might
   * have layout coordinates of 2000,100. We need to translate that to
   * output-local coordinates, or (2000 - 1920). */
  double ox = 0, oy = 0;
  wlr_output_layout_output_coords(view->server->output_layout, output, &ox,
                                  &oy);

  view_geometry.x += view->x + ox + sx;
  view_geometry.y += view->y + oy + sy;
  // view_geometry.width = surface->current.width;
  // view_geometry.height = surface->current.height;

  // smoothen resize from top & left edges
  if ((view->server->cursor->resize_edges & WLR_EDGE_LEFT ||
      view->server->cursor->resize_edges & WLR_EDGE_TOP) && 
      (view->request_box.width > 20 && view->request_box.height > 20) &&
      (view->request_box.width != box.width ||
      view->request_box.height != box.height)) {
    view_geometry.width = view->request_box.width * output->scale;
    view_geometry.height = view->request_box.height * output->scale;
  }


  int frameWidth = 2;
  int gripWidth = 28;
  int borderWidth = 3;
  int handleWidth = 8;
  int titlebarHeight = 24;

  memset(&view->hotspots, 0, sizeof(struct wlr_box) * HS_COUNT);

  // ----------------------
  // render frame
  // ----------------------
  if (frameWidth > 0) {
    color_to_rgba(color, style->borderColor);

    // left
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= frameWidth;
    box.width = frameWidth;
    render_rect(output, &box, colorDebug2, output->scale);

    // right
    box.x += view_geometry.width + frameWidth;
    render_rect(output, &box, colorDebug2, output->scale);

    // top
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= frameWidth;
    box.y -= frameWidth;
    box.width += (frameWidth * 2);
    box.height = frameWidth;
    render_rect(output, &box, colorDebug2, output->scale);

    // bottom
    box.y += view_geometry.height + frameWidth;
    render_rect(output, &box, colorDebug2, output->scale);
  }

  // ----------------------
  // render titlebar
  // ----------------------
  if (titlebarHeight) {
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= (frameWidth + borderWidth);
    box.width += ((frameWidth + borderWidth) * 2);
    box.y -= (frameWidth + borderWidth + titlebarHeight);
    box.height = titlebarHeight + borderWidth;
    render_rect(output, &box, colorDebug3, output->scale);

    memcpy(&view->hotspots[HS_TITLEBAR], &box, sizeof(struct wlr_box));
  }

  // ----------------------
  // render handle
  // ----------------------
  if (handleWidth) {
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= (frameWidth + borderWidth);
    box.width += ((frameWidth + borderWidth) * 2);
    box.y += view_geometry.height + frameWidth;
    box.height = handleWidth + borderWidth;
    render_rect(output, &box, colorDebug3, output->scale);

    memcpy(&view->hotspots[HS_HANDLE], &box, sizeof(struct wlr_box));

    // grips
    if (gripWidth) {
      box.x = view_geometry.x;
      box.y = view_geometry.y + view_geometry.height + frameWidth + borderWidth;
      box.width = gripWidth;
      box.height = handleWidth;
      render_rect(output, &box, colorDebug4, output->scale);

      memcpy(&view->hotspots[HS_GRIP_LEFT], &box, sizeof(struct wlr_box));
      grow_box_vh(&view->hotspots[HS_GRIP_LEFT], borderWidth, borderWidth);

      box.x += view_geometry.width - gripWidth;
      render_rect(output, &box, colorDebug4, output->scale);      

      memcpy(&view->hotspots[HS_GRIP_RIGHT], &box, sizeof(struct wlr_box));
      grow_box_vh(&view->hotspots[HS_GRIP_RIGHT], borderWidth, borderWidth);
    }
  }

  // ----------------------
  // render borders
  // ----------------------
  if (borderWidth > 0) {
    color_to_rgba(color, style->borderColor);

    // left
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= (frameWidth + borderWidth);
    box.y -= frameWidth;
    box.width = borderWidth;
    box.height += (frameWidth * 2);
    render_rect(output, &box, colorDebug1, output->scale);

    memcpy(&view->hotspots[HS_EDGE_LEFT], &box, sizeof(struct wlr_box));

    // right
    box.x = view_geometry.x + view_geometry.width + frameWidth;
    render_rect(output, &box, colorDebug1, output->scale);

    memcpy(&view->hotspots[HS_EDGE_RIGHT], &box, sizeof(struct wlr_box));

    // grow hotspots with titlebar and handle
    grow_box_lrtb(&view->hotspots[HS_EDGE_LEFT], 0, 0, titlebarHeight, handleWidth);
    grow_box_lrtb(&view->hotspots[HS_EDGE_RIGHT], 0, 0, titlebarHeight, handleWidth);

    // top
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= (frameWidth + borderWidth);
    box.y -= (frameWidth + borderWidth);
    box.width += ((frameWidth + borderWidth) * 2);
    box.height = borderWidth;

    if (titlebarHeight) {
      box.y -= titlebarHeight;
    }

    render_rect(output, &box, colorDebug1, output->scale);
    memcpy(&view->hotspots[HS_EDGE_TOP], &box, sizeof(struct wlr_box));

    // bottom
    box.y = view_geometry.y + view_geometry.height + frameWidth;

    if (handleWidth) {
      box.y += handleWidth;
    }

    render_rect(output, &box, colorDebug1, output->scale);
    memcpy(&view->hotspots[HS_EDGE_BOTTOM], &box, sizeof(struct wlr_box));
  }

  // make hotspot edges more receptive
  if (borderWidth > 0) {
    int hs = 2;
    grow_box_lrtb(&view->hotspots[HS_EDGE_LEFT], hs, 0, 0, 0);
    grow_box_lrtb(&view->hotspots[HS_EDGE_RIGHT], 0, hs, 0, 0);
    grow_box_lrtb(&view->hotspots[HS_EDGE_TOP], 0, 0, hs, 0);
    grow_box_lrtb(&view->hotspots[HS_EDGE_BOTTOM], 0, 0, 0, hs);
    if (gripWidth && handleWidth) {
      grow_box_lrtb(&view->hotspots[HS_GRIP_LEFT], hs, 0, 0, hs);
      grow_box_lrtb(&view->hotspots[HS_GRIP_RIGHT], 0, hs, 0, hs);
    }
  }
}

static void render_view_content(struct wlr_surface *surface, int sx, int sy,
                                void *data) {
  /* This function is called for every surface that needs to be rendered. */
  struct render_data *rdata = data;
  struct tbx_view *view = rdata->view;
  struct wlr_output *output = rdata->output;

  /* We first obtain a wlr_texture, which is a GPU resource. wlroots
   * automatically handles negotiating these with the client. The underlying
   * resource could be an opaque handle passed from the client, or the client
   * could have sent a pixel buffer which we copied to the GPU, or a few other
   * means. You don't have to worry about this, wlroots takes care of it. */
  struct wlr_texture *texture = wlr_surface_get_texture(surface);
  if (texture == NULL) {
    return;
  }

  // commit from smooth move request
  if (view->request_wait > 0 &&
     view->request_box.x != 0 &&
     view->request_box.y != 0)
  {
    if (--view->request_wait == 0) {
      view->x = view->request_box.x;
      view->y = view->request_box.y;
      view->request_box.x = 0;
      view->request_box.y = 0;
    }
  }

  /* The view has a position in layout coordinates. If you have two displays,
   * one next to the other, both 1080p, a view on the rightmost display might
   * have layout coordinates of 2000,100. We need to translate that to
   * output-local coordinates, or (2000 - 1920). */
  double ox = 0, oy = 0;
  wlr_output_layout_output_coords(view->server->output_layout, output, &ox,
                                  &oy);
  ox += view->x + sx;
  oy += view->y + sy;

  /* We also have to apply the scale factor for HiDPI outputs. This is only
   * part of the puzzle, TinyWL does not fully support HiDPI. */
  struct wlr_box box = {
      .x = ox * output->scale,
      .y = oy * output->scale,
      .width = surface->current.width * output->scale,
      .height = surface->current.height * output->scale,
  };

  // smoothen resize from top & left edges
  if ((view->server->cursor->resize_edges & WLR_EDGE_LEFT ||
      view->server->cursor->resize_edges & WLR_EDGE_TOP) && 
      (view->request_box.width > 20 && view->request_box.height > 20) &&
      (view->request_box.width != box.width ||
      view->request_box.height != box.height)) {
    box.width = view->request_box.width * output->scale;
    box.height = view->request_box.height * output->scale;
  }

  /*
   * Those familiar with OpenGL are also familiar with the role of matricies
   * in graphics programming. We need to prepare a matrix to render the view
   * with. wlr_matrix_project_box is a helper which takes a box with a desired
   * x, y coordinates, width and height, and an output geometry, then
   * prepares an orthographic projection and multiplies the necessary
   * transforms to produce a model-view-projection matrix.
   *
   * Naturally you can do this any way you like, for example to make a 3D
   * compositor.
   */
  float matrix[9];
  enum wl_output_transform transform =
      wlr_output_transform_invert(surface->current.transform);
  wlr_matrix_project_box(matrix, &box, transform, 0, output->transform_matrix);

  /* This takes our matrix, the texture, and an alpha, and performs the actual
   * rendering on the GPU. */
  wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, 1);

  /* This lets the client know that we've displayed that frame and it can
   * prepare another one now if it likes. */
  wlr_surface_send_frame_done(surface, rdata->when);
}

static void output_frame(struct wl_listener *listener, void *data) {
  /* This function is called every time an output is ready to display a frame,
   * generally at the output's refresh rate (e.g. 60Hz). */
  struct tbx_output *output = wl_container_of(listener, output, frame);
  struct wlr_renderer *renderer = output->server->renderer;

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  /* wlr_output_attach_render makes the OpenGL context current. */
  if (!wlr_output_attach_render(output->wlr_output, NULL)) {
    return;
  }
  /* The "effective" resolution can change if you rotate your outputs. */
  int width, height;
  wlr_output_effective_resolution(output->wlr_output, &width, &height);
  /* Begin the renderer (calls glViewport and some other GL sanity checks) */
  wlr_renderer_begin(renderer, width, height);

  // render
  float color[4] = {0.3, 0.3, 0.3, 1.0};
  wlr_renderer_clear(renderer, color);

  // render all views!
  /* Each subsequent window we render is rendered on top of the last. Because
   * our view list is ordered front-to-back, we iterate over it backwards. */
  struct tbx_view *view;
  wl_list_for_each_reverse(view, &output->server->views, link) {
    if (!view->mapped || !view->surface) {
      /* An unmapped view should not be rendered. */
      continue;
    }

    struct render_data rdata = {
        .output = output->wlr_output,
        .view = view,
        .renderer = renderer,
        .when = &now,
    };

    if (!view->csd) {
      render_view_decorations((struct wlr_surface *)view->xdg_surface, 0, 0,
                              &rdata);
      // wlr_xdg_surface_for_each_surface(view->xdg_surface,
      // render_view_decorations, &rdata);
    }

    wlr_xdg_surface_for_each_surface(view->xdg_surface, render_view_content,
                                     &rdata);
  }

  /* Hardware cursors are rendered by the GPU on a separate plane, and can be
   * moved around without re-rendering what's beneath them - which is more
   * efficient. However, not all hardware supports hardware cursors. For this
   * reason, wlroots provides a software fallback, which we ask it to render
   * here. wlr_cursor handles configuring hardware vs software cursors for you,
   * and this function is a no-op when hardware cursors are in use. */
  wlr_output_render_software_cursors(output->wlr_output, NULL);

  /* Conclude rendering and swap the buffers, showing the final frame
   * on-screen. */
  wlr_renderer_end(renderer);
  wlr_output_commit(output->wlr_output);
}

static void server_new_output(struct wl_listener *listener, void *data) {
  /* This event is rasied by the backend when a new output (aka a display or
   * monitor) becomes available. */
  struct tbx_server *server = wl_container_of(listener, server, new_output);
  struct wlr_output *wlr_output = data;

  /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
   * before we can use the output. The mode is a tuple of (width, height,
   * refresh rate), and each monitor supports only a specific set of modes. We
   * just pick the monitor's preferred mode, a more sophisticated compositor
   * would let the user configure it. */
  if (!wl_list_empty(&wlr_output->modes)) {
    struct wlr_output_mode *mode = wlr_output_preferred_mode(wlr_output);
    wlr_output_set_mode(wlr_output, mode);
    wlr_output_enable(wlr_output, true);
    if (!wlr_output_commit(wlr_output)) {
      return;
    }
  }

  /* Allocates and configures our state for this output */
  struct tbx_output *output = calloc(1, sizeof(struct tbx_output));
  output->wlr_output = wlr_output;
  output->server = server;

  /* Sets up a listener for the frame notify event. */
  output->frame.notify = output_frame;
  wl_signal_add(&wlr_output->events.frame, &output->frame);

  wl_list_insert(&server->outputs, &output->link);

  /* Adds this to the output layout. The add_auto function arranges outputs
   * from left-to-right in the order they appear. A more sophisticated
   * compositor would let the user configure the arrangement of outputs in the
   * layout.
   *
   * The output layout utility automatically adds a wl_output global to the
   * display, which Wayland clients can see to find out information about the
   * output (such as DPI, scale factor, manufacturer, etc).
   */
  wlr_output_layout_add_auto(server->output_layout, wlr_output);

  if (!server->main_output) {
    server->main_output = output;
  }
}

bool output_setup(struct tbx_server *server) {
  server->output_layout = wlr_output_layout_create();
  wl_list_init(&server->outputs);

  server->new_output.notify = server_new_output;
  wl_signal_add(&server->backend->events.new_output, &server->new_output);
  return true;
}