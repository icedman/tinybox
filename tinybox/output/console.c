#include "tinybox/console.h"
#include "common/cairo.h"
#include "common/util.h"
#include "tinybox/damage.h"
#include "tinybox/output.h"
#include "tinybox/render.h"
#include "tinybox/server.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_matrix.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

#include <GLES2/gl2.h>
#include <cairo/cairo.h>
#include <pango/pangocairo.h>
#include <wlr/render/gles2.h>

static struct tbx_console theConsole = { 0 };

// static cairo_surface_t* console_surface = NULL;

void console_setup(struct tbx_server* server)
{
    server->console = &theConsole;
    server->console->server = server;
    console_clear();
}

void console_clear()
{
    struct tbx_console* console = &theConsole;
    if (!console->server->config.console) {
        return;
    }

    FILE* console_file = fopen("/tmp/tinybox.log", "w");
    if (!console_file) {
        return;
    }
    fclose(console_file);
}

void console_log(const char* format, ...)
{
    struct tbx_console* console = &theConsole;
    if (!console->server->config.console) {
        return;
    }

    char string[512] = "";

    va_list args;
    va_start(args, format);
    vsnprintf(string, 255, format, args);
    va_end(args);

    FILE* console_file = fopen("/tmp/tinybox.log", "a");
    if (!console_file) {
        return;
    }
    char* token = strtok(string, "\n");
    while (token != NULL) {
        fprintf(console_file, token);
        fprintf(console_file, "\n");

        token = strtok(NULL, "\n");
    }
    fclose(console_file);
}

const char* header = "-------------\n%s\n";
void console_dump()
{
    struct tbx_console* console = &theConsole;
    struct tbx_server* server = console->server;

    console_clear();
    console_log("%s\nv%s", PACKAGE_NAME, PACKAGE_VERSION);

    struct tbx_view* view;
    console_log(header, "views");
    wl_list_for_each_reverse(view, &server->views, link)
    {
        const char* title = view->interface->get_string_prop(view, VIEW_PROP_TITLE);
        console_log("v: %s\n", title);
    }

    struct tbx_workspace* workspace;
    console_log(header, "workspaces");
    wl_list_for_each(workspace, &server->workspaces, link)
    {
        char main = ' ';
        if (workspace->id == server->workspace) {
            main = '*';
        }
        console_log("w: %d%c %d %d\n", workspace->id, main, workspace->box.x,
            workspace->box.y);
    }

    struct tbx_output* output;
    console_log(header, "outputs");
    wl_list_for_each(output, &server->outputs, link)
    {
        if (!output->enabled) {
            console_log("idle %d", output->last_frame.tv_nsec / 1000000);
            continue;
        }
        double ox = 0, oy = 0;
        wlr_output_layout_output_coords(server->output_layout, output->wlr_output,
            &ox, &oy);

        struct wlr_box* box = wlr_output_layout_get_box(server->output_layout, output->wlr_output);

        char main = ' ';
        if (output == server->main_output) {
            main = '*';
        }

        console_log("%s%c (%d, %d) - (%d %d) %d", output->wlr_output->name, main,
            (int)ox, (int)oy, (int)box->width, (int)box->height,
            output->last_frame.tv_nsec / 1000000);
    }
}
