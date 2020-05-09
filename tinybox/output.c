#define _POSIX_C_SOURCE 200112L

#include "tinybox/tbx_server.h"
#include "tinybox/tbx_output.h"
#include "tinybox/util.h"

#include <wlr/render/gles2.h>
#include <wlr/render/wlr_renderer.h>
#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#include "tinybox/cairo.h"
#include "tinybox/pango.h"

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
struct render_data {
  struct wlr_output *output;
  struct wlr_renderer *renderer;
  struct tbx_view *view;
  struct timespec *when;
};

struct wlr_texture *textCache[16] = {
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL,
  NULL, NULL, NULL, NULL
};

static int lastCacheHash = 0;

enum {
  tx_window_title_focus,
  tx_window_title_unfocus,
  tx_window_label_focus,
  tx_window_label_unfocus,
  tx_window_handle_focus,
  tx_window_handle_unfocus,
  tx_window_grip_focus,
  tx_window_grip_unfocus,
};

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

static void generate_textures(struct wlr_renderer *renderer, bool forced) {

  if (textCache[0] != NULL && !(forced || lastCacheHash != server.style.hash)) {
    return;
  }

  lastCacheHash = server.style.hash;

  float color[4];
  float colorTo[4];
  int flags;

  // titlebar
  color_to_rgba(color, server.style.window_title_focus_color);
  color_to_rgba(colorTo, server.style.window_title_focus_colorTo);
  flags = server.style.window_title_focus;
  generate_texture(renderer, tx_window_title_focus, flags, 512, 16, color, colorTo);

  color_to_rgba(color, server.style.window_title_unfocus_color);
  color_to_rgba(colorTo, server.style.window_title_unfocus_colorTo);
  flags = server.style.window_title_unfocus;
  generate_texture(renderer, tx_window_title_unfocus, flags, 512, 16, color, colorTo);

  // titlebar/label
  color_to_rgba(color, server.style.window_label_focus_color);
  color_to_rgba(colorTo, server.style.window_label_focus_colorTo);
  flags = server.style.window_label_focus;
  generate_texture(renderer, tx_window_label_focus, flags, 512, 16, color, colorTo);

  color_to_rgba(color, server.style.window_label_unfocus_color);
  color_to_rgba(colorTo, server.style.window_label_unfocus_colorTo);
  flags = server.style.window_label_unfocus;
  generate_texture(renderer, tx_window_label_unfocus, flags, 512, 16, color, colorTo);

  // handle
  color_to_rgba(color, server.style.window_handle_focus_color);
  color_to_rgba(colorTo, server.style.window_handle_focus_colorTo);
  flags = server.style.window_handle_focus;
  generate_texture(renderer, tx_window_handle_focus, flags, 512, 16, color, colorTo);

  color_to_rgba(color, server.style.window_handle_unfocus_color);
  color_to_rgba(colorTo, server.style.window_handle_unfocus_colorTo);
  flags = server.style.window_handle_unfocus;
  generate_texture(renderer, tx_window_handle_unfocus, flags, 512, 16, color, colorTo);

  // grip
  color_to_rgba(color, server.style.window_grip_focus_color);
  color_to_rgba(colorTo, server.style.window_grip_focus_colorTo);
  flags = server.style.window_grip_focus;
  generate_texture(renderer, tx_window_grip_focus, flags, 30, 16, color, colorTo);

  color_to_rgba(color, server.style.window_grip_unfocus_color);
  color_to_rgba(colorTo, server.style.window_grip_unfocus_colorTo);
  flags = server.style.window_grip_unfocus;
  generate_texture(renderer, tx_window_grip_unfocus, flags, 30, 16, color, colorTo);
}

//--------------------
// move to view
//--------------------
const char *get_string_prop(struct tbx_view *view,
    enum tbx_view_prop prop) {
  switch (prop) {
  case VIEW_PROP_TITLE:
    return view->xdg_surface->toplevel->title;
  case VIEW_PROP_APP_ID:
    return view->xdg_surface->toplevel->app_id;
  default:
    return NULL;
  }
}

void generate_view_title_texture(struct tbx_output *output, struct tbx_view *view)
{
  struct wlr_renderer *renderer = wlr_backend_get_renderer(output->wlr_output->backend);

  if (view->title) {
    wlr_texture_destroy(view->title);
    wlr_texture_destroy(view->title_unfocused);
  }

  const char *font = "monospace 10";

  char title[128];
  char appId[64];
  sprintf(title, "%s", get_string_prop(view, VIEW_PROP_TITLE));
  sprintf(appId, "%s", get_string_prop(view, VIEW_PROP_APP_ID));

  float scale = 1.0f;
  int w = 400;
  int h = 32;

  // We must use a non-nil cairo_t for cairo_set_font_options to work.
  // Therefore, we cannot use cairo_create(NULL).
  cairo_surface_t *dummy_surface = cairo_image_surface_create(
      WL_SHM_FORMAT_ARGB8888, 0, 0);
  cairo_t *c = cairo_create(dummy_surface);
  cairo_set_antialias(c, CAIRO_ANTIALIAS_BEST);
  cairo_font_options_t *fo = cairo_font_options_create();
  cairo_font_options_set_hint_style(fo, CAIRO_HINT_STYLE_FULL);
  if (output->wlr_output->subpixel == WL_OUTPUT_SUBPIXEL_NONE) {
    cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_GRAY);
  } else {
    cairo_font_options_set_antialias(fo, CAIRO_ANTIALIAS_SUBPIXEL);
    
    // cairo.c
    cairo_font_options_set_subpixel_order(fo,
      to_cairo_subpixel_order(output->wlr_output->subpixel));
  }

  float color[4];

  cairo_set_font_options(c, fo);
  get_text_size(c, font, &w, NULL, NULL, scale, true, "%s", title);
  cairo_surface_destroy(dummy_surface);
  cairo_destroy(c);


  cairo_surface_t *surf = cairo_image_surface_create(
      WL_SHM_FORMAT_ARGB8888, w, h);
  cairo_t *cx = cairo_create(surf);

  cairo_set_font_options(cx, fo);
  cairo_font_options_destroy(fo);

  PangoContext *pango = pango_cairo_create_context(cx);
  cairo_move_to(cx, 0, 0);

  color_to_rgba(color, server.style.window_label_focus_textColor);
  cairo_set_source_rgba(cx, color[0], color[1], color[2], color[3]);
  pango_printf(cx, font, scale, true,
      "%s", title);

  unsigned char *data = cairo_image_surface_get_data(surf);
  // int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, w);

  view->title = wlr_texture_from_pixels(renderer,
      WL_SHM_FORMAT_ARGB8888,
      cairo_image_surface_get_stride(surf),
      w, h, data);

  // printf("stride: %d height: %d\n", stride, h);

  // clear

  color_to_rgba(color, server.style.window_label_unfocus_textColor);
  cairo_set_source_rgba(cx, color[0], color[1], color[2], color[3]);
  pango_printf(cx, font, scale, true,
      "%s", title);

  data = cairo_image_surface_get_data(surf);
  view->title_unfocused = wlr_texture_from_pixels(renderer,
      WL_SHM_FORMAT_ARGB8888,
      cairo_image_surface_get_stride(surf),
      w, h, data);

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


static void render_rect(struct wlr_output *output, struct wlr_box *box, float color[4]) {
  struct wlr_renderer *renderer = wlr_backend_get_renderer(output->backend);
  wlr_render_rect(renderer, box, color, output->transform_matrix);
}


static void render_texture(struct wlr_output *output, struct wlr_box *box, struct wlr_texture *texture) {
  if (!texture) {
    return;
  }
  
  struct wlr_renderer *renderer = wlr_backend_get_renderer(output->backend);

  struct wlr_gles2_texture_attribs attribs;
  wlr_gles2_texture_get_attribs(texture, &attribs);
  glBindTexture(attribs.target, attribs.tex);
  glTexParameteri(attribs.target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  float matrix[9];
  wlr_matrix_project_box(matrix, box,
    WL_OUTPUT_TRANSFORM_NORMAL,
    0.0, output->transform_matrix);

  wlr_render_texture_with_matrix(renderer, texture, matrix, 1.0);
}

static void render_view_frame(struct wlr_surface *surface, int sx, int sy, void *data) {
  /* This function is called for every surface that needs to be rendered. */
  struct render_data *rdata = data;
  struct tbx_view *view = rdata->view;
  struct wlr_output *output = rdata->output;

  struct wlr_renderer *renderer = wlr_backend_get_renderer(output->backend);

  double ox = 0, oy = 0;
  wlr_output_layout_output_coords(
      view->server->output_layout, output, &ox, &oy);
  ox += view->x + sx, oy += view->y + sy;

  int border_thickness = 2;
  int footer_height = 8;
  int title_bar_height = 28;

  struct wlr_box box;
  wlr_xdg_surface_get_geometry((struct wlr_xdg_surface*)surface, &box);

  box.x = (ox - border_thickness) * output->scale;
  box.y = (oy - border_thickness) * output->scale;
  box.width = (box.width + border_thickness * 2) * output->scale;
  box.height = (box.height + border_thickness * 2) * output->scale;

  struct wlr_box base_box;  
  memcpy(&base_box, &box, sizeof(struct wlr_box));

  // struct tbx_view *active_view = server.active_view;
  int focusTextureOffset = !(view->xdg_surface->surface == server.seat->keyboard_state.focused_surface);

  float color[4];
  color_to_rgba(color, server.style.borderColor);
  color[3] = 1.0;

  // --------------
  // borders
  // --------------
  // top
  box.height = border_thickness * output->scale;
  render_rect(output, &box, color);
  memcpy(&view->hotspots[HS_EDGE_TOP], &box, sizeof(struct wlr_box));

  // bottom
  box.y = base_box.y + base_box.height - border_thickness;
  render_rect(output, &box, color);
  memcpy(&view->hotspots[HS_EDGE_BOTTOM], &box, sizeof(struct wlr_box));

  // left
  memcpy(&box, &base_box, sizeof(struct wlr_box));
  box.width = border_thickness;
  render_rect(output, &box, color);
  memcpy(&view->hotspots[HS_EDGE_LEFT], &box, sizeof(struct wlr_box));
  
  // right
  box.x += base_box.width - border_thickness;
  render_rect(output, &box, color);
  memcpy(&view->hotspots[HS_EDGE_RIGHT], &box, sizeof(struct wlr_box));

  // make hotspot edges tolerant
  int hs_thickness = 4;
  view->hotspots[HS_EDGE_TOP].y -= (title_bar_height + hs_thickness - border_thickness);
  view->hotspots[HS_EDGE_TOP].height += hs_thickness;
  // view->hotspots[HS_EDGE_BOTTOM].y += ;
  view->hotspots[HS_EDGE_BOTTOM].height += (footer_height + hs_thickness);
  view->hotspots[HS_EDGE_LEFT].x -= (hs_thickness - border_thickness);
  view->hotspots[HS_EDGE_LEFT].width += hs_thickness;
  view->hotspots[HS_EDGE_RIGHT].width += hs_thickness;

  // --------------
  // titlebar
  // --------------
  memcpy(&box, &base_box, sizeof(struct wlr_box));
  box.y = box.y - title_bar_height;
  box.height = title_bar_height;
  render_rect(output, &box, color);
  memcpy(&view->hotspots[HS_TITLEBAR], &box, sizeof(struct wlr_box));


  box.x += border_thickness;
  box.y += border_thickness;
  box.width -= (border_thickness*2);
  box.height -= (border_thickness*2);
  render_texture(output, &box, textCache[tx_window_title_focus + focusTextureOffset]);
  // label
  box.x += border_thickness;
  box.y += border_thickness;
  box.width -= (border_thickness*2);
  box.height -= (border_thickness*2);
  render_texture(output, &box, textCache[tx_window_label_focus + focusTextureOffset]);

  if (view->title) {
    box.width -= 4;
    scissor_output(output, box);

    box.x += 2;
    box.y += 2;
    box.width = view->title_box.width;
    box.height = view->title_box.height;

    if (!focusTextureOffset) {
      render_texture(output, &box, view->title);
    } else {
      render_texture(output, &box, view->title_unfocused);
    }

    wlr_renderer_scissor(renderer, NULL);
  }

  // handle
  memcpy(&box, &base_box, sizeof(struct wlr_box));
  box.y = box.y + box.height;
  box.height = footer_height + border_thickness;
  render_rect(output, &box, color);
  memcpy(&view->hotspots[HS_HANDLE], &box, sizeof(struct wlr_box));

  box.x += border_thickness;
  box.y += border_thickness;
  box.width -= (border_thickness * 2);
  box.height -= (border_thickness * 2);
  render_texture(output, &box, textCache[tx_window_handle_focus + focusTextureOffset]);
}

static void render_view_content(struct wlr_surface *surface,
    int sx, int sy, void *data) {
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

  /* The view has a position in layout coordinates. If you have two displays,
   * one next to the other, both 1080p, a view on the rightmost display might
   * have layout coordinates of 2000,100. We need to translate that to
   * output-local coordinates, or (2000 - 1920). */
  double ox = 0, oy = 0;
  wlr_output_layout_output_coords(
      view->server->output_layout, output, &ox, &oy);
  ox += view->x + sx, oy += view->y + sy;

  /* We also have to apply the scale factor for HiDPI outputs. This is only
   * part of the puzzle, TinyWL does not fully support HiDPI. */
  struct wlr_box box = {
    .x = ox * output->scale,
    .y = oy * output->scale,
    .width = surface->current.width * output->scale,
    .height = surface->current.height * output->scale,
  };

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
  wlr_matrix_project_box(matrix, &box, transform, 0,
    output->transform_matrix);

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
  struct tbx_output *output =
    wl_container_of(listener, output, frame);
  struct wlr_renderer *renderer = output->server->renderer;

  generate_textures(renderer, false);

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

  float color[4] = {0.3, 0.3, 0.3, 1.0};
  wlr_renderer_clear(renderer, color);

  /* Each subsequent window we render is rendered on top of the last. Because
   * our view list is ordered front-to-back, we iterate over it backwards. */
  struct tbx_view *view;
  wl_list_for_each_reverse(view, &output->server->views, link) {
    if (!view->mapped) {
      /* An unmapped view should not be rendered. */
      continue;
    }
    struct render_data rdata = {
      .output = output->wlr_output,
      .view = view,
      .renderer = renderer,
      .when = &now,
    };
    /* This calls our render_surface function for each surface among the
     * xdg_surface's toplevel and popups. */

    if (view->title_dirty) {
        generate_view_title_texture(output, view);
    }

    render_view_frame((struct wlr_surface *)view->xdg_surface, 0, 0, &rdata);

    // if (view != output->server->grabbed_view)
    wlr_xdg_surface_for_each_surface(view->xdg_surface,
        render_view_content, &rdata);
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

  output->last_render = now;
}

static void server_new_output(struct wl_listener *listener, void *data) {
  /* This event is rasied by the backend when a new output (aka a display or
   * monitor) becomes available. */
  struct tbx_server *server =
    wl_container_of(listener, server, new_output);
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
}

void init_output() {

  /* Creates an output layout, which a wlroots utility for working with an
   * arrangement of screens in a physical layout. */
  server.output_layout = wlr_output_layout_create();

  /* Configure a listener to be notified when new outputs are available on the
   * backend. */
  wl_list_init(&server.outputs);
  server.new_output.notify = server_new_output;
  wl_signal_add(&server.backend->events.new_output, &server.new_output);
}

