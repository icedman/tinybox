#ifndef TINYBOX_CONSOLE_H
#define TINYBOX_CONSOLE_H

#include <stdbool.h>

#define CONSOLE_LINES 32
#define CONSOLE_WIDTH 800
#define CONSOLE_HEIGHT 600

struct wlr_texture;
struct tbx_server;

struct tbx_console {
  struct tbx_server *server;
};

void
console_setup();
void
console_clear();
void
console_log(const char *format, ...);
void
console_dump();

#endif // TINYBOX_CONSOLE_H