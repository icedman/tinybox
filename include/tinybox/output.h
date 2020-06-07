#ifndef TINYBOX_OUTPUT_H
#define TINYBOX_OUTPUT_H

#include <time.h>
#include <unistd.h>

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_menu;
struct tbx_view;

struct tbx_output {
    struct wl_list link;

    struct tbx_server* server;

    struct wlr_output* wlr_output;
    struct wl_listener frame;
    struct wl_listener destroy;

    struct wlr_output_damage* damage;

    struct timespec last_frame;
    long run_time;
    bool enabled;
};

bool output_setup(struct tbx_server* server);
void configure_output_layout(struct tbx_output* output);

#endif // TINYBOX_OUTPUT_H