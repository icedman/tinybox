#include "tinybox/style.h"
#include "tinybox/style_defaults.h"

#include <string.h>

void load_style(struct tbx_style *style, const char *path) {
  if (!path) {
    memcpy(style, style_bin, sizeof(struct tbx_style));
    strcpy(style->font, "monospace 10");
  }
}