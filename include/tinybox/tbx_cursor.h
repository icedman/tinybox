#ifndef TBX_CURSOR_H
#define TBX_CURSOR_H

enum tbx_cursor_mode {
  tbx_CURSOR_PASSTHROUGH,
  tbx_CURSOR_MOVE,
  tbx_CURSOR_RESIZE,
};

void init_cursor();

#endif //  TBX_CURSOR_H