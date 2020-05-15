#ifndef TINYBOX_RENDER_H
#define TINYBOX_RENDER_H

#include <stdbool.h>

struct wlr_output;
struct wlr_renderer;
struct wlr_box;
struct wlr_texture;
struct tbx_output;
struct tbx_workspace;

enum {
  tx_window_title_focus,
  tx_window_title_unfocus,
  tx_window_label_focus,
  tx_window_label_unfocus,
  tx_window_handle_focus,
  tx_window_handle_unfocus,
  tx_window_grip_focus,
  tx_window_grip_unfocus,

  tx_workspace_1,
  tx_workspace_2,
  tx_workspace_3,
  tx_workspace_4,
  tx_workspace_5,
  tx_workspace_6,
  tx_workspace_7,
  tx_workspace_8,

  tx_last
};

/* Used to move all of the data necessary to render a surface from the top-level
 * frame handler to the per-surface render function. */
struct render_data {
  struct wlr_output *output;
  struct wlr_renderer *renderer;
  struct tbx_view *view;
  struct tbx_workspace *workspace;
  struct timespec *when;
  bool in_main_output;
  double offset_x;
  double offset_y;
};

void render_rect(struct wlr_output *output, struct wlr_box *box, float color[4],
                 float scale);
void render_texture(struct wlr_output *output, struct wlr_box *box,
                    struct wlr_texture *texture, float scale);
void generate_textures(struct tbx_output *output, bool forced);
void generate_view_title_texture(struct tbx_output *output,
                                 struct tbx_view *view);
void generate_background(struct tbx_output *output,
                         struct tbx_workspace *workspace);

void texture_cache_destroy();
void scissor_output(struct wlr_output *wlr_output, struct wlr_box box);

void grow_box_lrtb(struct wlr_box *box, int l, int r, int t, int b);
void grow_box_hv(struct wlr_box *box, int h, int v);

struct wlr_texture *get_texture_cache(int idx);

#endif // TINYBOX_RENDER_H