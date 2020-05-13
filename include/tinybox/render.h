#ifndef TINYBOX_RENDER_H
#define TINYBOX_RENDER_H

struct wlr_output;
struct wlr_renderer;
struct wlr_box;
struct wlr_texture;

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

#endif // TINYBOX_RENDER_H