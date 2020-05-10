#ifndef CONSOLE_H
#define CONSOLE_H

#define CONSOLE_LINES 24
#define CONSOLE_WIDTH 800
#define CONSOLE_HEIGHT 400

struct tbx_console {
    int inputIdx;
    int renderIdx;
    char lines[CONSOLE_LINES][255];
    struct wlr_texture *texture;
    bool dirty;
};

void console_init();
void console_clear();
void console_log(const char *format, ...) ;

void console_render(struct tbx_output *output);

#endif // CONSOLE_H
