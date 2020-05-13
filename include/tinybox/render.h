#ifndef TINYBOX_RENDER_H
#define TINYBOX_RENDER_H

#include <stdbool.h>

struct wlr_output;
struct wlr_renderer;
struct wlr_box;
struct wlr_texture;
struct tbx_output;

enum {
  tx_window_title_focus,
  tx_window_title_unfocus,
  tx_window_label_focus,
  tx_window_label_unfocus,
  tx_window_handle_focus,
  tx_window_handle_unfocus,
  tx_window_grip_focus,
  tx_window_grip_unfocus
};

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
struct render_data {
  struct wlr_output *output;
  struct wlr_renderer *renderer;
  struct tbx_view *view;
  struct timespec *when;
};

void render_rect(struct wlr_output *output, struct wlr_box *box, float color[4],
                 float scale);

void render_texture(struct wlr_output *output, struct wlr_box *box,
                    struct wlr_texture *texture, float scale);

void generate_textures(struct tbx_output *output, bool forced);

struct wlr_texture *get_texture_cache(int idx);

#endif // TINYBOX_RENDER_H