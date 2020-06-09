#define _POSIX_C_SOURCE 200112L
#include "tinybox/render.h"

#include "common/util.h"
#include "tinybox/command.h"
#include "tinybox/cursor.h"
#include "tinybox/damage.h"
#include "tinybox/menu.h"
#include "tinybox/output.h"
#include "tinybox/render.h"
#include "tinybox/server.h"
#include "tinybox/style.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_damage.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/util/region.h>

#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <wlr/render/gles2.h>

static void output_render(struct tbx_output* output);

static void smoothen_geometry_when_resizing(struct tbx_view* view, struct wlr_box* box)
{
    if (view->csd) {
        return;
    }

    // smoothen
    if ((view->server->cursor->resize_edges & WLR_EDGE_LEFT || view->server->cursor->resize_edges & WLR_EDGE_TOP) && (view->request_box.width > 20 && view->request_box.height > 20) && (view->request_box.width != box->width || view->request_box.height != box->height)) {
        box->width = view->request_box.width;
        box->height = view->request_box.height;
        damage_add_view(view->server, view);
    }
}

static void render_view_frame(struct wlr_surface* surface, int sx, int sy,
    void* data)
{
    // renders the decorations and positions hotspots

    /* This function is called for every surface that needs to be rendered. */
    struct render_data* rdata = data;
    struct tbx_view* view = rdata->view;
    struct tbx_output* tbx_output = rdata->output;
    struct wlr_output* output = tbx_output->wlr_output;
    struct wlr_seat* seat = view->server->seat->seat;
    struct tbx_style* style = &view->server->style;

    bool shaded = view->shaded;

    float color[4] = { 1, 0, 1, 1 };
    float bevelColor[4] = { 1.0, 0.0, 1.0, 0.1 };

    int unfocus_offset = 0;
    if (view_from_surface(view->server, seat->keyboard_state.focused_surface) != view) {
        unfocus_offset++;
    }

    struct wlr_box view_geometry;
    struct wlr_box box;

    view->interface->get_geometry(view, &view_geometry);

    double ox = 0, oy = 0;
    wlr_output_layout_output_coords(view->server->output_layout, output, &ox,
        &oy);

    ox += rdata->offset_x;
    ox += rdata->offset_y;

    double oox = ox;
    double ooy = oy;

    view_geometry.x += view->x + ox + sx;
    view_geometry.y += view->y + oy + sy;

    // struct wlr_box region_box;
    // memcpy(&region_box, &view->damage_region->region, sizeof(struct wlr_box));
    // region_box.x -= (ox + sx);
    // region_box.y -= (oy + sy);

    smoothen_geometry_when_resizing(view, &view_geometry);

    // read these from style file
    int frameWidth = 2;
    int gripWidth = 28;
    int borderWidth = 3;
    int handleWidth = 8;
    int titlebarHeight = 24;

    int margin = frameWidth ? frameWidth : borderWidth;

    if (view->title) {
        titlebarHeight = view->title_box.height + (margin + borderWidth) * 2;
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

        memcpy(&box, &view_geometry, sizeof(struct wlr_box));
        grow_box_hv(&box, frameWidth, frameWidth);
        // if (region_overlap(&region_box, &box)) {
        render_rect_outline(tbx_output, &box, color, frameWidth, false, output->scale);
        // }
    }

    bool mini_titlebar = view->server->config.mini_titlebar;
    bool mini_frame = view->server->config.mini_frame;
    if (mini_titlebar) {
        titlebarHeight = handleWidth;
    }
    if (mini_frame) {
        titlebarHeight = handleWidth = 0;
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
        render_rect(tbx_output, &box, color, output->scale);

        memcpy(&view->hotspots[HS_TITLEBAR], &box, sizeof(struct wlr_box));
        memcpy(&view->hotspots[HS_EDGE_TOP_LEFT], &box, sizeof(struct wlr_box));
        memcpy(&view->hotspots[HS_EDGE_TOP_RIGHT], &box, sizeof(struct wlr_box));

        int topCornerSize = 12;
        view->hotspots[HS_EDGE_TOP_LEFT].width = topCornerSize;
        view->hotspots[HS_EDGE_TOP_LEFT].height = topCornerSize;
        view->hotspots[HS_EDGE_TOP_RIGHT].x = box.x + box.width - topCornerSize;
        view->hotspots[HS_EDGE_TOP_RIGHT].width = topCornerSize;
        view->hotspots[HS_EDGE_TOP_RIGHT].height = topCornerSize;

        // render the texture
        grow_box_hv(&box, -borderWidth, -borderWidth);

        render_texture(tbx_output, &box,
            get_texture_cache(tx_window_title_focus + unfocus_offset),
            output->scale);

        // render_rect(tbx_output, &box, colorDebug2, output->scale);

        int tflags = unfocus_offset ? style->window_title_focus : style->window_title_unfocus;
        if (tflags & sf_raised) {
            render_rect_outline(tbx_output, &box, bevelColor, 1, 1, output->scale);
        } else if (tflags & sf_sunken) {
            render_rect_outline(tbx_output, &box, bevelColor, 1, -1, output->scale);
        }

        if (!mini_titlebar) {
            // label
            grow_box_hv(&box, -margin, -margin);

            render_texture(tbx_output, &box,
                get_texture_cache(tx_window_label_focus + unfocus_offset),
                output->scale);
            // render_rect(tbx_output, &box, bevelColor, output->scale);

            int tflags = unfocus_offset ? style->window_label_focus : style->window_label_unfocus;
            if (tflags & sf_raised) {
                render_rect_outline(tbx_output, &box, bevelColor, 1, 1, output->scale);
            } else if (tflags & sf_sunken) {
                render_rect_outline(tbx_output, &box, bevelColor, 1, -1, output->scale);
            }

            // title
            box.x += margin;
            box.y += margin;

            struct wlr_box sc_box = {
                .x = box.x,
                .y = box.y,
                .width = box.width - (margin * 4),
                .height = box.height,
            };

            if (sc_box.x) {
            }

            box.width = view->title_box.width;
            box.height = view->title_box.height;

            if (view->title) {
                // scissor_output(output, sc_box);
                if (!unfocus_offset) {
                    render_texture(tbx_output, &box, view->title, output->scale);
                } else {
                    render_texture(tbx_output, &box, view->title_unfocused, output->scale);
                }
                // wlr_renderer_scissor(rdata->renderer, NULL);
            }
        }
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

        render_rect(tbx_output, &box, color, output->scale);
        memcpy(&view->hotspots[HS_HANDLE], &box, sizeof(struct wlr_box));

        // render the texture
        grow_box_hv(&box, -borderWidth, -borderWidth);
        if (gripWidth) {
            grow_box_hv(&box, -(gripWidth + (borderWidth * 2) + 1), 0);
        }

        render_texture(tbx_output, &box,
            get_texture_cache(tx_window_handle_focus + unfocus_offset),
            output->scale);

        int tflags = unfocus_offset ? style->window_handle_focus : style->window_handle_unfocus;
        if (tflags & sf_raised) {
            render_rect_outline(tbx_output, &box, bevelColor, 1, 1, output->scale);
        } else if (tflags & sf_sunken) {
            render_rect_outline(tbx_output, &box, bevelColor, 1, -1, output->scale);
        }

        // render_rect(tbx_output, &box, colorDebug2, output->scale);

        // grips
        if (gripWidth) {
            box.x = view_geometry.x - frameWidth;
            box.y = view_geometry.y + view_geometry.height + frameWidth + borderWidth;
            box.width = gripWidth + (frameWidth * 2);
            box.height = handleWidth - borderWidth;
            render_texture(tbx_output, &box,
                get_texture_cache(tx_window_grip_focus + unfocus_offset),
                output->scale);
            // render_rect(tbx_output, &box, colorDebug4, output->scale);

            int tflags = unfocus_offset ? style->window_grip_focus : style->window_grip_unfocus;
            if (tflags & sf_raised) {
                render_rect_outline(tbx_output, &box, bevelColor, 1, 1, output->scale);
            } else if (tflags & sf_sunken) {
                render_rect_outline(tbx_output, &box, bevelColor, 1, -1, output->scale);
            }

            memcpy(&view->hotspots[HS_EDGE_BOTTOM_LEFT], &box, sizeof(struct wlr_box));
            grow_box_hv(&view->hotspots[HS_EDGE_BOTTOM_LEFT], borderWidth, borderWidth);

            box.x += view_geometry.width - gripWidth;
            render_texture(tbx_output, &box,
                get_texture_cache(tx_window_grip_focus + unfocus_offset),
                output->scale);
            // render_rect(tbx_output, &box, colorDebug4, output->scale);

            memcpy(&view->hotspots[HS_EDGE_BOTTOM_RIGHT], &box, sizeof(struct wlr_box));
            grow_box_hv(&view->hotspots[HS_EDGE_BOTTOM_RIGHT], borderWidth, borderWidth);

            if (tflags & sf_raised) {
                render_rect_outline(tbx_output, &box, bevelColor, 1, 1, output->scale);
            } else if (tflags & sf_sunken) {
                render_rect_outline(tbx_output, &box, bevelColor, 1, -1, output->scale);
            }
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

        render_rect(tbx_output, &box, color, output->scale);

        memcpy(&view->hotspots[HS_EDGE_LEFT], &box, sizeof(struct wlr_box));

        // right
        box.x = view_geometry.x + view_geometry.width + frameWidth;
        render_rect(tbx_output, &box, color, output->scale);

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
            render_rect(tbx_output, &box, color, output->scale);
        }

        memcpy(&view->hotspots[HS_EDGE_TOP], &box, sizeof(struct wlr_box));

        // bottom
        box.y = view_geometry.y + view_geometry.height + frameWidth;

        if (handleWidth) {
            box.y += handleWidth;
        } else {
            render_rect(tbx_output, &box, color, output->scale);
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
            grow_box_lrtb(&view->hotspots[HS_EDGE_BOTTOM_LEFT], hs, 0, 0, hs);
            grow_box_lrtb(&view->hotspots[HS_EDGE_BOTTOM_RIGHT], 0, hs, 0, hs);
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

static void render_view_content(struct wlr_surface* surface, int sx, int sy,
    void* data)
{
    /* This function is called for every surface that needs to be rendered. */
    struct render_data* rdata = data;
    struct tbx_view* view = rdata->view;
    struct tbx_output* tbx_output = rdata->output;
    struct wlr_output* output = tbx_output->wlr_output;

    view->life++;

    // commit from smooth move request
    if (view->request_wait > 0 && view->request_box.x != 0 && view->request_box.y != 0) {
        damage_add_view(view->server, view);
        if (--view->request_wait == 0 || view->csd) {
            view->x = view->request_box.x;
            view->y = view->request_box.y;
            view->request_box.x = 0;
            view->request_box.y = 0;
            damage_add_view(view->server, view);
        }
    }

    /* We first obtain a wlr_texture, which is a GPU resource. wlroots
   * automatically handles negotiating these with the client. The underlying
   * resource could be an opaque handle passed from the client, or the client
   * could have sent a pixel buffer which we copied to the GPU, or a few other
   * means. You don't have to worry about this, wlroots takes care of it. */

    struct wlr_texture* texture = wlr_surface_get_texture(surface);
    if (texture == NULL) {
        // console_log("!");
        return;
    }

    struct wlr_box view_geometry;
    view->interface->get_geometry(view, &view_geometry);

    if (surface != view->surface) {
        view_geometry.width = surface->current.width;
        view_geometry.height = surface->current.height;
    }

    smoothen_geometry_when_resizing(view, &view_geometry);

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
        .width = view_geometry.width * output->scale,
        .height = view_geometry.height * output->scale
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

    float alpha = 1.0;
    if (view->server->cursor->mode == TBX_CURSOR_RESIZE || view->server->cursor->mode == TBX_CURSOR_MOVE) {
        alpha = view->server->config.move_resize_alpha;
        if (alpha < 0.4) {
            alpha = 0.4;
        }
        if (alpha > 1) {
            alpha = 1;
        }
    }

    float matrix[9];
    enum wl_output_transform transform = wlr_output_transform_invert(surface->current.transform);
    wlr_matrix_project_box(matrix, &box, transform, 0, output->transform_matrix);

    /* This takes our matrix, the texture, and an alpha, and performs the actual
   * rendering on the GPU. */

    for (int i = 0; i < tbx_output->scissors_count; i++) {
        scissor_output(output, tbx_output->scissors[i]);
        wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, alpha);
    }
    wlr_renderer_scissor(rdata->renderer, 0);
    if (!tbx_output->scissors_count) {
        wlr_render_texture_with_matrix(rdata->renderer, texture, matrix, alpha);
    }


    /* This lets the client know that we've displayed that frame and it can
   * prepare another one now if it likes. */
    wlr_surface_send_frame_done(surface, rdata->when);
}

static void output_frame(struct wl_listener* listener, void* data)
{
    struct tbx_output* output = wl_container_of(listener, output, frame);
    output_render(output);
}

static void output_render(struct tbx_output* output)
{
    /* This function is called every time an output is ready to display a frame,
   * generally at the output's refresh rate (e.g. 60Hz). */
    struct tbx_server* server = output->server;
    struct tbx_cursor* cursor = server->cursor;
    struct wlr_renderer* renderer = server->renderer;

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
        damage_whole(server);
    }

    // this keeps things simple for now
    if (cursor->mode == TBX_CURSOR_SWIPE_WORKSPACE
        || cursor->mode == TBX_CURSOR_MOVE
        || cursor->mode == TBX_CURSOR_RESIZE) {
        damage_whole(server);
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

    bool needs_frame;
    pixman_region32_t buffer_damage;
    pixman_region32_init(&buffer_damage);
    if (!wlr_output_damage_attach_render(output->damage, &needs_frame,
            &buffer_damage)) {
        return;
    }

    /* Begin the renderer (calls glViewport and some other GL sanity checks) */
    wlr_renderer_begin(renderer, width, height);

    // render box
    if (server->config.render_damages) {
        float color[4] = { 1.0, 0, 0, 1.0 };
        wlr_renderer_clear(renderer, color);
    }

    if (!pixman_region32_not_empty(&buffer_damage)) {
        // struct tbx_view* view;
        // wl_list_for_each_reverse(view, &server->views, link)
        // {
        //     if (!view->mapped || !view->surface) {
        //         continue;
        //     }
        //     wlr_surface_send_frame_done(view->surface, &now);
        // }

        // Output isn't damaged but needs buffer swap
        goto renderer_end;
    }

    memset(&output->scissors, 0, sizeof(struct wlr_box));
    output->scissors_count = 0;

    double ox = 0, oy = 0;
    wlr_output_layout_output_coords(server->output_layout, output->wlr_output, &ox, &oy);

    float color[4] = { 0.0, 0, 0, 1.0 };
    int nrects;
    pixman_box32_t* rects = pixman_region32_rectangles(&buffer_damage, &nrects);
    for (int i = 1; i < nrects && i < MAX_OUTPUT_SCISSORS; ++i) {
        if ((rects[i].x2 - rects[i].x1 <= 0 || rects[i].y2 - rects[i].y1 <= 0)) {
            continue;
        }
        output->scissors[i].x = rects[i].x1 + ox;
        output->scissors[i].y = rects[i].y1 + oy;
        output->scissors[i].width = rects[i].x2 - rects[i].x1;
        output->scissors[i].height = rects[i].y2 - rects[i].y1;
        output->scissors_count = i + 1;

        // printf("wlr: %d %d %d %d\n",
        //         output->scissors[i].x,
        //         output->scissors[i].y,
        //         output->scissors[i].width, 
        //         output->scissors[i].height
        //     );

        scissor_output(output->wlr_output, output->scissors[i]);
        wlr_renderer_clear(renderer, color);
    }

    //-----------------
    // render workspace backgrounds
    //-----------------

    if (in_main_output && animate && (cursor->mode == TBX_CURSOR_SWIPE_WORKSPACE || server->ws_animate)) {
        render_workspace(output, get_workspace(server, server->workspace - 1));
        render_workspace(output, get_workspace(server, server->workspace + 1));
    }
    render_workspace(output, get_workspace(server, server->workspace));

    if (in_main_output) {
        render_console(output);
    }

    //-----------------
    // render views
    //-----------------
    /* Each subsequent window we render is rendered on top of the last. Because
         * our view list is ordered front-to-back, we iterate over it backwards. */
    struct tbx_view* view;
    wl_list_for_each_reverse(view, &server->views, link)
    {
        if (!view->mapped || !view->surface) {
            /* An unmapped view should not be rendered. */
            continue;
        }

        if (output->scissors_count > 0) {
            struct wlr_box window_box;
            view_frame(view, &window_box);
            window_box.x += ox;
            window_box.y += oy;
            pixman_box32_t window_region = {
                .x1 = window_box.x,
                .y1 = window_box.y,
                .x2 = window_box.x + window_box.width,
                .y2 = window_box.y + window_box.height
            };
            if (!pixman_region32_contains_rectangle(&buffer_damage, &window_region)) {
                // printf("drop!\n");
                wlr_surface_send_frame_done(view->surface, &now);
                continue;
            }
        }

        double offset_x = 0;
        double offset_y = 0;
        struct tbx_workspace* workspace = 0;

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
                if (!view_is_visible(view, output)) {
                    continue;
                }

                if (server->ws_animate) {
                    d += server->ws_anim_x;
                } else {
                    d += cursor->swipe_x - cursor->swipe_begin_x;
                    float d = server->cursor->swipe_x - server->cursor->swipe_begin_x;
                    if (d * d < SWIPE_MIN) {
                        d = 0;
                    }
                }

                workspace = get_workspace(output->server, view->workspace);
                offset_x = workspace->box.x + d;
            } else {
                if (view->workspace != server->workspace) {
                    continue;
                }
            }
        }

        if (view->title_box.width <= 0) {
            view->title_dirty = true;
        }
        if (view->title_dirty) {
            generate_view_title_texture(output, view);
        }

        //-----------------
        // render the view
        //-----------------
        struct render_data rdata = {
            .output = output,
            .view = view,
            .renderer = renderer,
            .workspace = workspace,
            .offset_x = offset_x,
            .offset_y = offset_y,
            .when = &now,
        };

        // decorations
        if (!view->csd && !view->fullscreen) {
            render_view_frame(view->surface, 0, 0, &rdata);
        }

        // content
        if (!view->shaded) {

            if (view->view_type == VIEW_TYPE_XDG) {
                wlr_xdg_surface_for_each_surface(view->xdg_surface, render_view_content,
                    &rdata);
            }

            if (view->view_type == VIEW_TYPE_XWAYLAND) {
                render_view_content(view->surface, 0, 0, &rdata);
            }
        }
    }

renderer_end:

    wlr_renderer_scissor(renderer, 0);

    if (server->config.render_damages) {
        for (int i = 0; i < output->scissors_count; ++i) {
            // printf("wlr: %d %d %d %d\n", region.x, region.y, region.width, region.height);
            float damageColor[4] = { 1.0, 0, 1.00, 1.0 };
            render_rect_outline(output, &output->scissors[i], damageColor, 2, false, output->wlr_output->scale);
            // if (region.x) {}
        }
    }

    //-----------------
    // render menus
    //-----------------
    // todo check against damaged regions
    render_menus(output);

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

    pixman_region32_t frame_damage;
    pixman_region32_init(&frame_damage);

    enum wl_output_transform transform = wlr_output_transform_invert(output->wlr_output->transform);
    wlr_region_transform(&frame_damage, &output->damage->current,
        transform, width, height);

    wlr_output_set_damage(output->wlr_output, &frame_damage);
    pixman_region32_fini(&frame_damage);
    pixman_region32_fini(&buffer_damage);

    wlr_output_set_damage(output->wlr_output, &frame_damage);
    wlr_output_commit(output->wlr_output);

    output->last_frame = now;
}

static void output_remove(void* data)
{
    // struct tbx_output *output = data;
    // struct tbx_server *server = output->server;
    // wlr_output_layout_remove(server->output_layout, output->wlr_output);
    // wl_list_remove(&output->link);
    // free(output);

    console_log("remove output");
}

static void output_handle_destroy(struct wl_listener* listener, void* data)
{
    struct tbx_output* output = wl_container_of(listener, output, destroy);
    struct tbx_server* server = output->server;

    if (server->main_output == output) {
        server->main_output = 0;
        struct tbx_output* o;
        wl_list_for_each_reverse(o, &output->server->outputs, link)
        {
            if (o != output) {
                server->main_output = o;
            }
        }
    }

    output->enabled = false;
    wl_event_loop_add_idle(server->wl_event_loop, output_remove, output);
}

static void output_damage_handle_frame(struct wl_listener* listener,
    void* data)
{
    struct tbx_output* output = wl_container_of(listener, output, damage_frame);
    output_render(output);
}

static void output_damage_handle_destroy(struct wl_listener* listener,
    void* data)
{
    struct tbx_output* output = wl_container_of(listener, output, damage_destroy);
    // output_destroy(output);
}

static void server_new_output(struct wl_listener* listener, void* data)
{
    /* This event is rasied by the backend when a new output (aka a display or
   * monitor) becomes available. */
    struct tbx_server* server = wl_container_of(listener, server, new_output);
    struct wlr_output* wlr_output = data;

    /* Some backends don't have modes. DRM+KMS does, and we need to set a mode
   * before we can use the output. The mode is a tuple of (width, height,
   * refresh rate), and each monitor supports only a specific set of modes. We
   * just pick the monitor's preferred mode, a more sophisticated compositor
   * would let the user configure it. */
    if (!wl_list_empty(&wlr_output->modes)) {
        struct wlr_output_mode* mode = wlr_output_preferred_mode(wlr_output);
        wlr_output_set_mode(wlr_output, mode);
        wlr_output_enable(wlr_output, true);
        if (!wlr_output_commit(wlr_output)) {
            return;
        }
    }

    /* Allocates and configures our state for this output */
    struct tbx_output* output = calloc(1, sizeof(struct tbx_output));
    output->wlr_output = wlr_output;
    output->server = server;
    output->enabled = true;

    // not for now
    output->damage = wlr_output_damage_create(wlr_output);
    output->damage_frame.notify = output_damage_handle_frame;
    // wl_signal_add(&output->damage->events.frame, &output->damage_frame);
    output->damage_destroy.notify = output_damage_handle_destroy;
    // wl_signal_add(&output->damage->events.destroy, &output->damage_destroy);

    /* Sets up a listener for the frame notify event. */
    output->frame.notify = output_frame;
    wl_signal_add(&wlr_output->events.frame, &output->frame);

    output->destroy.notify = output_handle_destroy;
    wl_signal_add(&wlr_output->events.destroy, &output->destroy);

    damage_whole(server);

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

bool output_setup(struct tbx_server* server)
{
    server->output_layout = wlr_output_layout_create();
    wl_list_init(&server->outputs);
    server->new_output.notify = server_new_output;
    wl_signal_add(&server->backend->events.new_output, &server->new_output);
    return true;
}
