#define _POSIX_C_SOURCE 200809L
#include <cairo.h>
#include <ctype.h>
#include <fcntl.h>
#include <float.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "common/rgb.h"
#include "common/stringop.h"
#include "common/util.h"
#include "tinybox/style.h"

#define MAKE_COLOR(r, g, b) (r << 24) | (g << 16) | (b << 8) | 0xff;

int wrap(int i, int max) { return ((i % max) + max) % max; }

bool xparse_colorname(const char *spec, uint32_t *color) {
  for (int i = 0;; i++) {
    colorEntry *e = &colorTable[i];
    if (!e->name) {
      break;
    }
    if (strcmp(e->name, spec) == 0) {
      *color = MAKE_COLOR(e->r, e->g, e->b);
      return true;
    }
  }
  return false;
}

char *rpad(char *dest, const char *src, const char pad, const size_t sz) {
  memset(dest, pad, sz);
  dest[sz] = 0x0;
  memcpy(dest, src, strlen(src));
  return dest;
}

bool xparse_color(const char *spec, uint32_t *color) {

  // int red;
  // int green;
  // int blue;

  char spec2[32];
  for (int i = 0; i < 32; i++) {
    char c = spec[i];
    if (c == ':' || c == '/') {
      c = ' ';
    }
    spec2[i] = c;
    if (c == 0) {
      break;
    }
  }

  int argc = 0;
  char **argv = split_args(spec2, &argc);

  if (argc != 4) {
    return false;
  }

  char red[8] = "";
  char green[8] = "";
  char blue[8] = "";

  rpad(red, argv[1], argv[1][0], 2);
  rpad(green, argv[2], argv[2][0], 2);
  rpad(blue, argv[3], argv[3][0], 2);

  *color = MAKE_COLOR((strtol(red, NULL, 16)), (strtol(green, NULL, 16)),
                      (strtol(blue, NULL, 16)));
  return true;
}

bool parse_color(const char *color, uint32_t *result) {
  if (xparse_color(color, result)) {
    return true;
  }

  if (xparse_colorname(color, result)) {
    return true;
  }

  if (color[0] == '#') {
    ++color;
  }
  int len = strlen(color);
  if ((len != 6 && len != 8) || !isxdigit(color[0]) || !isxdigit(color[1])) {
    return false;
  }
  char *ptr;
  uint32_t parsed = strtoul(color, &ptr, 16);
  if (*ptr != '\0') {
    return false;
  }
  *result = len == 6 ? ((parsed << 8) | 0xFF) : parsed;
  return true;
}

void color_to_rgba(float dest[static 4], uint32_t color) {
  dest[0] = ((color >> 24) & 0xff) / 255.0;
  dest[1] = ((color >> 16) & 0xff) / 255.0;
  dest[2] = ((color >> 8) & 0xff) / 255.0;
  dest[3] = (color & 0xff) / 255.0;
}

bool parse_boolean(const char *boolean, bool current) {
  if (strcasecmp(boolean, "1") == 0 || strcasecmp(boolean, "yes") == 0 ||
      strcasecmp(boolean, "on") == 0 || strcasecmp(boolean, "true") == 0 ||
      strcasecmp(boolean, "enable") == 0 ||
      strcasecmp(boolean, "enabled") == 0 ||
      strcasecmp(boolean, "active") == 0) {
    return true;
  } else if (strcasecmp(boolean, "toggle") == 0) {
    return !current;
  }
  // All other values are false to match i3
  return false;
}

float parse_float(const char *value) {
  // errno = 0;
  char *end;
  float flt = strtof(value, &end);
  if (*end) { // } || errno) {
    // sway_log(SWAY_DEBUG, "Invalid float value '%s', defaulting to NAN",
    // value);
    return NAN;
  }
  return flt;
}