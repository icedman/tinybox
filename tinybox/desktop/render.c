#define _POSIX_C_SOURCE 200112L

#include "tinybox/tbx_server.h"
#include "tinybox/tbx_output.h"
#include "tinybox/util.h"

#include <wlr/render/gles2.h>
#include <wlr/render/wlr_renderer.h>
#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>

#include "tinybox/cairo.h"
#include "tinybox/pango.h"

#include <stdarg.h>
