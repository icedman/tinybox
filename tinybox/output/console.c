#include "tinybox/console.h"
#include "common/cairo.h"
#include "common/util.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"

#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xdg_shell.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

static struct tbx_console theConsole = {0};

cairo_surface_t *console_surface = NULL;

void console_setup(struct tbx_server *server) {
  server->console = &theConsole;
  server->console->server = server;

  int w = CONSOLE_WIDTH;
  int h = CONSOLE_HEIGHT;

  console_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);

  console_clear();
}

void console_clear() {
  struct tbx_console *console = &theConsole;

  console->inputIdx = 0;
  console->renderIdx = 0;
  console->dirty = true;
  memset(console->lines, 0, sizeof(char) * 255 * CONSOLE_LINES);
}

void console_log(const char *format, ...) {
  struct tbx_console *console = &theConsole;

  char string[255] = "";

  va_list args;
  va_start(args, format);
  vsnprintf(string, 255, format, args);
  va_end(args);

  char *token = strtok(string, "\n");
  while (token != NULL) {
    strcpy(console->lines[console->inputIdx % CONSOLE_LINES], token);
    console->inputIdx++;
    if (console->inputIdx >= CONSOLE_LINES) {
      console->renderIdx = (console->inputIdx + 1) % CONSOLE_LINES;
    }
    token = strtok(NULL, "\n");
  }

  console->dirty = true;
}

const char *header = "-------------\n%s\n";
void console_dump() {
  struct tbx_console *console = &theConsole;
  struct tbx_server *server = console->server;

  console_clear();
  console_log("%s\nv%s", PACKAGE_NAME, PACKAGE_VERSION);

  struct tbx_view *view;
  console_log(header, "views");
  wl_list_for_each_reverse(view, &server->views, link) {
    console_log("v: %s\n", view->xdg_surface->toplevel->title);
  }

  struct tbx_workspace *workspace;
  console_log(header, "workspaces");
  wl_list_for_each(workspace, &server->workspaces, link) {
    char main = ' ';
    if (workspace->id == server->workspace) {
      main = '*';
    }
    console_log("w: %d%c\n", workspace->id, main);
  }

  struct tbx_output *output;
  console_log(header, "outputs");
  wl_list_for_each(output, &server->outputs, link) {
    if (!output->enabled) {
      console_log("idle %d", output->last_frame.tv_nsec / 1000000);
      continue;
    }
    double ox = 0, oy = 0;
    wlr_output_layout_output_coords(server->output_layout, output->wlr_output,
                                    &ox, &oy);

    struct wlr_box *box =
        wlr_output_layout_get_box(server->output_layout, output->wlr_output);

    char main = ' ';
    if (output == server->main_output) {
      main = '*';
    }

    console_log("%s%c (%d, %d) - (%d %d) %d", output->wlr_output->name, main,
                (int)ox, (int)oy, (int)box->width, (int)box->height,
                output->last_frame.tv_nsec / 1000000);
  }
}

void console_render(struct tbx_output *output) {
  struct tbx_console *console = &theConsole;
  struct tbx_server *server = output->server;
  ;
  struct wlr_renderer *renderer = output->server->renderer;

  if (console->texture) {
    wlr_texture_destroy(console->texture);
  }

  // float scale = 1.0f;
  const char *font = output->server->style.font;

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
  cairo_surface_destroy(dummy_surface);
  cairo_destroy(c);

  cairo_t *cx = cairo_create(console_surface);
  cairo_set_font_options(cx, fo);
  cairo_font_options_destroy(fo);

  cairo_save(cx);
  cairo_set_source_rgba(cx, 0.0, 0.0, 0.0, 0.0);
  cairo_set_operator(cx, CAIRO_OPERATOR_CLEAR);
  cairo_rectangle(cx, 0, 0, CONSOLE_WIDTH, CONSOLE_HEIGHT);
  cairo_paint(cx);
  cairo_restore(cx);

  cairo_move_to(cx, 0, 0);

  float color[4];
  color_to_rgba(color, server->style.window_label_focus_textColor);
  cairo_set_source_rgba(cx, color[0], color[1], color[2], color[3]);

  cairo_select_font_face(cx, font, 0, 0);
  cairo_set_font_size(cx, 12);

  for (int i = 0; i < CONSOLE_LINES; i++) {
    int idx = (console->renderIdx + i) % CONSOLE_LINES;
    cairo_move_to(cx, 0, 14 + (14 * i));
    cairo_show_text(cx, console->lines[idx]);
  }

  // char fname[255] = "";
  // sprintf(fname, "/tmp/text_%s.png", appId);
  // cairo_surface_write_to_png(surf, fname);

  unsigned char *data = cairo_image_surface_get_data(console_surface);
  console->texture =
      wlr_texture_from_pixels(renderer, WL_SHM_FORMAT_ARGB8888,
                              cairo_image_surface_get_stride(console_surface),
                              CONSOLE_WIDTH, CONSOLE_HEIGHT, data);

  cairo_destroy(cx);
  console->dirty = false;
}