#define _POSIX_C_SOURCE 200112L

#include "tinybox/server.h"
#include "tinybox/view.h"
#include "tinybox/xwayland.h"

#include <float.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>

static void xwayland_get_constraints(struct tbx_view* view, double* min_width,
    double* max_width, double* min_height, double* max_height)
{
    struct wlr_xwayland_surface* surface = view->xwayland_surface;
    struct wlr_xwayland_surface_size_hints* size_hints = surface->size_hints;

    if (size_hints == NULL) {
        *min_width = DBL_MIN;
        *max_width = DBL_MAX;
        *min_height = DBL_MIN;
        *max_height = DBL_MAX;
        return;
    }

    *min_width = size_hints->min_width > 0 ? size_hints->min_width : DBL_MIN;
    *max_width = size_hints->max_width > 0 ? size_hints->max_width : DBL_MAX;
    *min_height = size_hints->min_height > 0 ? size_hints->min_height : DBL_MIN;
    *max_height = size_hints->max_height > 0 ? size_hints->max_height : DBL_MAX;
}

static void xwayland_get_geometry(struct tbx_view *view, struct wlr_box *box) {
    box->x = box->y = 0;
    if (view->surface) {
        box->width = view->surface->current.width;
        box->height = view->surface->current.height;
    } else {
        box->width = 0;
        box->height = 0;
    }
}

static void xwayland_set_activated(struct tbx_view* view, bool activated)
{
    struct wlr_seat* seat = view->server->seat->seat;
    struct wlr_xwayland* xwayland = view->server->xwayland_shell->wlr_xwayland;

    if (activated) {
        wlr_xwayland_set_seat(xwayland, seat);

        struct wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);
        wlr_seat_keyboard_notify_enter(seat, view->surface, keyboard->keycodes,
            keyboard->num_keycodes,
            &keyboard->modifiers);
    }

    wlr_xwayland_surface_activate(view->xwayland_surface, true);
}

static void xwaylang_set_fullscreen(struct tbx_view* view, bool fullscreen)
{
}

static const char* xwayland_get_string_prop(struct tbx_view* view, enum tbx_view_prop prop)
{
    switch (prop) {
    case VIEW_PROP_TITLE:
        return view->xwayland_surface->title;
    // case VIEW_PROP_CLASS:
    // return view->wlr_xwayland_surface->class;
    // case VIEW_PROP_INSTANCE:
    //   return view->wlr_xwayland_surface->instance;
    // case VIEW_PROP_WINDOW_ROLE:
    //   return view->wlr_xwayland_surface->role;
    default:
        return NULL;
    }
}

uint32_t xwayland_get_int_prop(struct tbx_view* view, enum tbx_view_prop prop)
{
    return 0;
}

static uint32_t xwayland_view_configure(struct tbx_view* view, double lx, double ly,
    int width, int height)
{
    wlr_xwayland_surface_configure(view->xwayland_surface, 0,
        0, width, height);

    view->x = lx;
    view->y = ly;
    // view->width = width;
    // view->height = height;

    view->request_box.x = lx;
    view->request_box.y = ly;
    view->request_box.width = width;
    view->request_box.height = height;
    return 0;
}

static void xwayland_close(struct tbx_view* view)
{
}

static void xwayland_close_popups(struct tbx_view* view)
{
}

static void xwayland_destroy(struct tbx_view* view)
{
    view_destroy(view);
}

static struct tbx_view_interface xwayland_view_interface = {
    .get_constraints = xwayland_get_constraints,
    .get_geometry = xwayland_get_geometry,
    .get_string_prop = xwayland_get_string_prop,
    .get_int_prop = xwayland_get_int_prop,
    .configure = xwayland_view_configure,
    .set_activated = xwayland_set_activated,
    .set_fullscreen = xwaylang_set_fullscreen,
    .close = xwayland_close,
    .close_popups = xwayland_close_popups,
    .destroy = xwayland_destroy
};

static void xwayland_surface_map(struct wl_listener* listener, void* data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */

    struct wlr_xwayland_surface* xsurface = data;

    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, map);
    struct tbx_view* view = &xwayland_view->view;

    view->mapped = true;
    view->title_dirty = true;
    view->surface = xsurface->surface;
    view->xwayland_surface = xsurface;

    view_set_focus(view, view->surface);
}

static void xwayland_surface_unmap(struct wl_listener* listener, void* data)
{
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, unmap);
    struct tbx_view* view = &xwayland_view->view;
    view->surface = NULL;
    view->mapped = false;
}

static void xwayland_surface_destroy(struct wl_listener* listener, void* data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, destroy);
    struct tbx_view* view = &xwayland_view->view;

    view->interface->destroy(view);
}

static void xwayland_request_configure(struct wl_listener* listener,
    void* data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, request_configure);
    struct tbx_view* view = &xwayland_view->view;

    struct wlr_xwayland_surface_configure_event* ev = data;
    struct wlr_xwayland_surface* xsurface = view->xwayland_surface;
    // if (!xsurface->mapped) {
    wlr_xwayland_surface_configure(xsurface, ev->x, ev->y, ev->width, ev->height);
    // return;
    // }

    view->x = ev->x;
    view->y = ev->y;
    view->width = ev->width;
    view->height = ev->height;
}

static void new_xwayland_surface(struct wl_listener* listener, void* data)
{
    struct wlr_xwayland_surface* xwayland_surface = data;

    if (xwayland_surface->override_redirect) {
        console_log("new xwayland unmanaged surface");
        // create_unmanaged(xsurface);
        return;
    }

    console_log("new surface");

    if (!xwayland_surface) {
        console_log("no surface?!!");
    }

    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
    struct tbx_xwayland_shell* xwayland_shell = wl_container_of(listener, xwayland_shell, new_xwayland_surface);
    struct tbx_server* server = xwayland_shell->server;

    // if (xwayland_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
    //   return;
    // }

    /* Allocate a tbx_view for this surface */
    struct tbx_xwayland_view* xwayland_view = calloc(1, sizeof(struct tbx_xwayland_view));
    struct tbx_view* view = &xwayland_view->view;
    view->view_type = VIEW_TYPE_XWAYLAND_SHELL;
    view->interface = &xwayland_view_interface;

    view->xwayland_surface = xwayland_surface;
    view->server = server;

    xwayland_shell->create_offset = (xwayland_shell->create_offset + 1) % 8;
    view->x = 4 + (xwayland_shell->create_offset * 40);
    view->y = 32 + (xwayland_shell->create_offset * 40);

    /* Listen to the various events it can emit */
    xwayland_view->map.notify = xwayland_surface_map;
    wl_signal_add(&xwayland_surface->events.map, &xwayland_view->map);

    xwayland_view->unmap.notify = xwayland_surface_unmap;
    wl_signal_add(&xwayland_surface->events.unmap, &xwayland_view->unmap);

    xwayland_view->destroy.notify = xwayland_surface_destroy;
    wl_signal_add(&xwayland_surface->events.destroy, &xwayland_view->destroy);

    xwayland_view->request_configure.notify = xwayland_request_configure;
    wl_signal_add(&xwayland_surface->events.request_configure,
        &xwayland_view->request_configure);

    // move to workspace
    view->workspace = server->workspace;

    /* Add it to the list of views. */
    wl_list_insert(&server->views, &view->link);
}

bool xwayland_shell_setup(struct tbx_server* server)
{
    server->xwayland_shell = calloc(1, sizeof(struct tbx_xwayland_shell));
    server->xwayland_shell->server = server;

    server->xwayland_shell->wlr_xwayland = wlr_xwayland_create(server->wl_display, server->compositor, true);
    // config->xwayland == XWAYLAND_MODE_LAZY);

    server->xwayland_shell->new_xwayland_surface.notify = new_xwayland_surface;
    wl_signal_add(&server->xwayland_shell->wlr_xwayland->events.new_surface,
        &server->xwayland_shell->new_xwayland_surface);

    /*
  wl_signal_add(&server->xwayland.wlr_xwayland->events.ready,
      &server->xwayland_ready);
  server->xwayland_ready.notify = handle_xwayland_ready;
  */

    setenv("DISPLAY", server->xwayland_shell->wlr_xwayland->display_name, true);
    return true;
}