#ifndef TBX_CURSOR_H
#define TBX_CURSOR_H

enum tbx_cursor_mode {
  TBX_CURSOR_PASSTHROUGH,
  TBX_CURSOR_MOVE,
  TBX_CURSOR_RESIZE,
};

void init_cursor();

#endif //  TBX_CURSOR_H