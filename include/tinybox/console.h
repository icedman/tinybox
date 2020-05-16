#ifndef TINYBOX_CONSOLE_H
#define TINYBOX_CONSOLE_H

#include <stdbool.h>

#define CONSOLE_LINES 32
#define CONSOLE_WIDTH 800
#define CONSOLE_HEIGHT 600

struct wlr_texture;
struct tbx_server;
struct tbx_output;

struct tbx_console {
    int inputIdx;
    int renderIdx;
    char lines[CONSOLE_LINES][255];
    struct wlr_texture* texture;
    bool dirty;

    struct tbx_server* server;
};

void console_setup();
void console_clear();
void console_log(const char* format, ...);
void console_dump();

void console_render(struct tbx_output* output);

#endif // TINYBOX_CONSOLE_H