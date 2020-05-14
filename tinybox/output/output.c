#define _POSIX_C_SOURCE 200112L
#include "tinybox/render.h"

#include "common/util.h"
#include "tinybox/output.h"
#include "tinybox/render.h"
#include "tinybox/server.h"
#include "tinybox/style.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"
#include "tinybox/cursor.h"

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <wlr/render/gles2.h>

#define ANIM_SPEED 0.75

static void render_view_decorations(struct wlr_surface *surface, int sx, int sy,
                                    void *data) {

  // renders the decorations and positions hotspots

  /* This function is called for every surface that needs to be rendered. */
  struct render_data *rdata = data;
  struct tbx_view *view = rdata->view;
  struct wlr_output *output = rdata->output;
  struct wlr_seat *seat = view->server->seat->seat;
  struct tbx_style *style = &view->server->style;

  bool shaded = view->shaded;

  float color[4] = {1, 0, 1, 1};

  int unfocus_offset = 0;
  if (view->xdg_surface->surface != seat->keyboard_state.focused_surface) {
    unfocus_offset++;
  }

  float colorDebug1[4] = {0, 1, 1, 1};
  float colorDebug2[4] = {1, 0, 1, 1};
  float colorDebug3[4] = {1, 1, 1, 1};
  float colorDebug4[4] = {1, 0, 0, 1};

  if (colorDebug1[0] || colorDebug2[0] || colorDebug3[0] || colorDebug4[4]) {
    // suppress unused
  }

  struct wlr_box view_geometry;
  struct wlr_box box;

  wlr_xdg_surface_get_geometry(view->xdg_surface, &view_geometry);

  double ox = 0, oy = 0;
  wlr_output_layout_output_coords(view->server->output_layout, output, &ox,
                                  &oy);

  ox += rdata->offset_x;
  ox += rdata->offset_y;

  double oox = ox;
  double ooy = oy;

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
  int margin = frameWidth ? frameWidth : borderWidth;

  if (view->title) {
    titlebarHeight = view->title_box.height - 4;
  }

  memset(&view->hotspots, 0, sizeof(struct wlr_box) * HS_COUNT);

  // ----------------------
  // render frame
  // ----------------------
  if (frameWidth > 0 && !shaded) {
    if (!unfocus_offset) {
      color_to_rgba(color, style->window_frame_focusColor);
    } else {
      color_to_rgba(color, style->window_frame_unfocusColor);
    }

    // left
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= frameWidth;
    box.width = frameWidth;
    render_rect(output, &box, color, output->scale);

    // right
    box.x += view_geometry.width + frameWidth;
    render_rect(output, &box, color, output->scale);

    // top
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= frameWidth;
    box.y -= frameWidth;
    box.width += (frameWidth * 2);
    box.height = frameWidth;
    render_rect(output, &box, color, output->scale);

    // bottom
    box.y += view_geometry.height + frameWidth;
    render_rect(output, &box, color, output->scale);
  }

  // ----------------------
  // render titlebar
  // ----------------------
  if (titlebarHeight) {
    color_to_rgba(color, style->borderColor);

    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= (frameWidth + borderWidth);
    box.width += ((frameWidth + borderWidth) * 2);
    box.y -= (frameWidth + borderWidth + titlebarHeight);
    box.height = titlebarHeight + borderWidth;
    render_rect(output, &box, color, output->scale);

    memcpy(&view->hotspots[HS_TITLEBAR], &box, sizeof(struct wlr_box));

    // render the texture
    grow_box_hv(&box, -borderWidth, -borderWidth);
    render_texture(output, &box,
                   get_texture_cache(tx_window_title_focus + unfocus_offset),
                   output->scale);
    // render_rect(output, &box, colorDebug2, output->scale);

    // label
    grow_box_hv(&box, -margin, -margin);
    render_texture(output, &box,
                   get_texture_cache(tx_window_label_focus + unfocus_offset),
                   output->scale);
    // render_rect(output, &box, colorDebug3, output->scale);

    // title
    box.x += margin;
    box.y += margin;

    struct wlr_box sc_box = {
        .x = box.x,
        .y = box.y,
        .width = box.width - (margin * 4),
        .height = box.height,
    };

    box.width = view->title_box.width;
    box.height = view->title_box.height;

    scissor_output(output, sc_box);

    if (!unfocus_offset) {
      render_texture(output, &box, view->title, output->scale);
    } else {
      render_texture(output, &box, view->title_unfocused, output->scale);
    }

    wlr_renderer_scissor(rdata->renderer, NULL);
  }

  // ----------------------
  // render handle
  // ----------------------
  if (handleWidth && !shaded) {
    color_to_rgba(color, style->borderColor);

    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= (frameWidth + borderWidth);
    box.width += ((frameWidth + borderWidth) * 2);
    box.y += view_geometry.height + frameWidth;
    box.height = handleWidth + borderWidth;
    render_rect(output, &box, color, output->scale);
    memcpy(&view->hotspots[HS_HANDLE], &box, sizeof(struct wlr_box));

    // render the texture
    grow_box_hv(&box, -borderWidth, -borderWidth);
    if (gripWidth) {
      grow_box_hv(&box, -(gripWidth + (borderWidth * 2) + 1), 0);
    }

    render_texture(output, &box,
                   get_texture_cache(tx_window_handle_focus + unfocus_offset),
                   output->scale);
    // render_rect(output, &box, colorDebug2, output->scale);

    // grips
    if (gripWidth) {
      box.x = view_geometry.x - frameWidth;
      box.y = view_geometry.y + view_geometry.height + frameWidth + borderWidth;
      box.width = gripWidth + (frameWidth * 2);
      box.height = handleWidth - borderWidth;
      render_texture(output, &box,
                     get_texture_cache(tx_window_grip_focus + unfocus_offset),
                     output->scale);
      // render_rect(output, &box, colorDebug4, output->scale);

      memcpy(&view->hotspots[HS_GRIP_LEFT], &box, sizeof(struct wlr_box));
      grow_box_hv(&view->hotspots[HS_GRIP_LEFT], borderWidth, borderWidth);

      box.x += view_geometry.width - gripWidth;
      render_texture(output, &box,
                     get_texture_cache(tx_window_grip_focus + unfocus_offset),
                     output->scale);
      // render_rect(output, &box, colorDebug4, output->scale);

      memcpy(&view->hotspots[HS_GRIP_RIGHT], &box, sizeof(struct wlr_box));
      grow_box_hv(&view->hotspots[HS_GRIP_RIGHT], borderWidth, borderWidth);
    }
  }

  // ----------------------
  // render borders
  // ----------------------
  if (borderWidth > 0 && !shaded) {
    color_to_rgba(color, style->borderColor);

    // left
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= (frameWidth + borderWidth);
    box.y -= frameWidth;
    box.width = borderWidth;
    box.height += (frameWidth * 2);
    render_rect(output, &box, color, output->scale);

    memcpy(&view->hotspots[HS_EDGE_LEFT], &box, sizeof(struct wlr_box));

    // right
    box.x = view_geometry.x + view_geometry.width + frameWidth;
    render_rect(output, &box, color, output->scale);

    memcpy(&view->hotspots[HS_EDGE_RIGHT], &box, sizeof(struct wlr_box));

    // grow hotspots with titlebar and handle
    grow_box_lrtb(&view->hotspots[HS_EDGE_LEFT], 0, 0, titlebarHeight,
                  handleWidth);
    grow_box_lrtb(&view->hotspots[HS_EDGE_RIGHT], 0, 0, titlebarHeight,
                  handleWidth);

    // top
    memcpy(&box, &view_geometry, sizeof(struct wlr_box));
    box.x -= (frameWidth + borderWidth);
    box.y -= (frameWidth + borderWidth);
    box.width += ((frameWidth + borderWidth) * 2);
    box.height = borderWidth;

    if (titlebarHeight) {
      box.y -= titlebarHeight;
    } else {
      render_rect(output, &box, color, output->scale);
    }

    memcpy(&view->hotspots[HS_EDGE_TOP], &box, sizeof(struct wlr_box));

    // bottom
    box.y = view_geometry.y + view_geometry.height + frameWidth;

    if (handleWidth) {
      box.y += handleWidth;
    } else {
      render_rect(output, &box, color, output->scale);
    }

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

  // adjust hotspots to layout
  for (int i = 0; i < HS_COUNT; i++) {
    view->hotspots[i].x -= oox;
    view->hotspots[i].y -= ooy;
    if (shaded && i != HS_TITLEBAR) {
      view->hotspots[i].width = 0;
      view->hotspots[i].height = 0;
    }
  }
}

static void render_view_content(struct wlr_surface *surface, int sx, int sy,
                                void *data) {
  /* This function is called for every surface that needs to be rendered. */
  struct render_data *rdata = data;
  // struct tbx_workspace *workspace = rdata->workspace;
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
  if (view->request_wait > 0 && view->request_box.x != 0 &&
      view->request_box.y != 0) {
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
  ox += rdata->offset_x;
  ox += rdata->offset_y;

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

static void render_console(struct tbx_output *output) {
  // -----------------------
  // render the console
  // -----------------------
  if (output->server->console->dirty) {
    console_render(output);
  }

  // console
  if (output->server->console->texture) {
    struct wlr_box console_box = {
        .x = 10, .y = 10, .width = CONSOLE_WIDTH, .height = CONSOLE_HEIGHT};
    render_texture(output->wlr_output, &console_box,
                   output->server->console->texture, output->wlr_output->scale);
  }
}

static void render_workspace(struct tbx_output *output) {
  struct wlr_renderer *renderer = output->server->renderer;

  float color[4] = {0.3, 0.3, 0.3, 1.0};
  wlr_renderer_clear(renderer, color);
}

static void output_frame(struct wl_listener *listener, void *data) {
  /* This function is called every time an output is ready to display a frame,
   * generally at the output's refresh rate (e.g. 60Hz). */
  struct tbx_output *output = wl_container_of(listener, output, frame);
  struct tbx_server *server = output->server;
  struct tbx_cursor *cursor = server->cursor;
  struct wlr_renderer *renderer = server->renderer;

  generate_textures(output, false);

  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);

  bool in_main_output = (output == server->main_output);

  // workspace animation
  bool animate = server->config.animate;
  if (in_main_output && (server->ws_animate && animate)) {
    server->ws_anim_x *= ANIM_SPEED;
    server->ws_anim_y *= ANIM_SPEED;
    if ((server->ws_anim_x * server->ws_anim_x) < 10) {
      server->ws_animate = false;
      server->ws_anim_x = 0;
      server->ws_anim_y = 0;
    }
  }

  // uint32_t elapsed = (now.tv_nsec - output->last_frame.tv_nsec)/1000000;
  // output->run_time += elapsed;

  /* wlr_output_attach_render makes the OpenGL context current. */
  if (!wlr_output_attach_render(output->wlr_output, NULL)) {
    return;
  }
  /* The "effective" resolution can change if you rotate your outputs. */
  int width, height;
  wlr_output_effective_resolution(output->wlr_output, &width, &height);
  /* Begin the renderer (calls glViewport and some other GL sanity checks) */
  wlr_renderer_begin(renderer, width, height);

  // render begin
  render_workspace(output);

  if (in_main_output) {
    render_console(output);
  }

  // render all views!
  /* Each subsequent window we render is rendered on top of the last. Because
   * our view list is ordered front-to-back, we iterate over it backwards. */
  struct tbx_view *view;
  wl_list_for_each_reverse(view, &server->views, link) {
    if (!view->mapped || !view->surface) {
      /* An unmapped view should not be rendered. */
      continue;
    }

    double offset_x = 0;
    double offset_y = 0;
    struct tbx_workspace *workspace = 0;

    // workspace logic
    if (in_main_output) {
      double d = 0;

      // view animation
      if (animate && view->wsv_animate) {
        view->wsv_anim_x *= ANIM_SPEED;
        view->wsv_anim_y *= ANIM_SPEED;
        if ((view->wsv_anim_x * view->wsv_anim_x) < 10) {
          view->wsv_animate = false;
          view->wsv_anim_x = 0;
          view->wsv_anim_y = 0;
        }

        d -= view->wsv_anim_x;
      }


      if (animate && (cursor->mode == TBX_CURSOR_SWIPE_WORKSPACE || server->ws_animate)) {

        // is view in main_output
        if (!view_is_visible(output, view)) {
          continue;
        }

        if (server->ws_animate) {
          d += server->ws_anim_x;
        } else {
          d += cursor->swipe_x - cursor->swipe_begin_x;
        }

        workspace = get_workspace(output->server, view->workspace);
        offset_x = workspace->box.x + d;        
      } else {
        if (view->workspace != server->workspace) {
          continue;
        }
      }
    }

    if (view->title_dirty) {
      generate_view_title_texture(output, view);
    }

    //-----------------
    // render the view
    //-----------------

    struct render_data rdata = {
        .output = output->wlr_output,
        .view = view,
        .renderer = renderer,
        .workspace = workspace,
        .in_main_output = in_main_output,
        .offset_x = offset_x,
        .offset_y = offset_y,
        .when = &now,
    };

    // decorations
    if (!view->csd) {
      render_view_decorations((struct wlr_surface *)view->xdg_surface, 0, 0,
                              &rdata);
    }

    // content
    if (!view->shaded) {
      wlr_xdg_surface_for_each_surface(view->xdg_surface, render_view_content,
                                       &rdata);
    }
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

  output->last_frame = now;
}

static void output_remove(void *data) {
  // struct tbx_output *output = data;
  // struct tbx_server *server = output->server;
  // wlr_output_layout_remove(server->output_layout, output->wlr_output);
  // wl_list_remove(&output->link);
  // free(output);

  console_log("remove output");
}

static void output_handle_destroy(struct wl_listener *listener, void *data) {
  struct tbx_output *output = wl_container_of(listener, output, destroy);
  struct tbx_server *server = output->server;

  if (server->main_output == output) {
    server->main_output = 0;
    struct tbx_output *o;
    wl_list_for_each_reverse(o, &output->server->outputs, link) {
      if (o != output) {
        server->main_output = o;
      }
    }
  }

  output->enabled = false;
  wl_event_loop_add_idle(server->wl_event_loop, output_remove, output);
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
  output->enabled = true;

  /* Sets up a listener for the frame notify event. */
  output->frame.notify = output_frame;
  wl_signal_add(&wlr_output->events.frame, &output->frame);
  output->destroy.notify = output_handle_destroy;
  wl_signal_add(&wlr_output->events.destroy, &output->destroy);

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

  configure_output_layout(output);

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