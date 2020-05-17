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


static void xwayland_unmanaged_map(struct wl_listener* listener, void* data)
{
    struct wlr_xwayland_surface* xsurface = data;

    struct tbx_xwayland_unmanaged* view = wl_container_of(listener, view, map);
    // struct wlr_xwayland_surface *xsurface = view->wlr_xwayland_surface;

    view->wlr_xwayland_surface = xsurface;
}

static void xwayland_unmanaged_unmap(struct wl_listener* listener, void* data)
{
    struct tbx_xwayland_unmanaged* view = wl_container_of(listener, view, map);
    view->wlr_xwayland_surface = NULL;
}

static void xwayland_unmanaged_request_configure(struct wl_listener* listener, void* data)
{
    struct tbx_xwayland_unmanaged *surface =
        wl_container_of(listener, surface, request_configure);
    struct wlr_xwayland_surface *xsurface = surface->wlr_xwayland_surface;
    struct wlr_xwayland_surface_configure_event *ev = data;
    wlr_xwayland_surface_configure(xsurface, ev->x, ev->y,
        ev->width, ev->height);
}

static void xwayland_unmanaged_destroy(struct wl_listener* listener, void* data)
{
    struct tbx_xwayland_unmanaged *surface =
        wl_container_of(listener, surface, destroy);
    free(surface);
}

static struct tbx_xwayland_unmanaged *create_xwayland_unmanaged(struct wlr_xwayland_surface* surface)
{
    /* Allocate a tbx_view for this surface */
    struct tbx_xwayland_unmanaged* xwayland_view = calloc(1, sizeof(struct tbx_xwayland_unmanaged));
    xwayland_view->wlr_xwayland_surface = surface;

    // struct tbx_view* view = &xwayland_view->view;
    // view->view_type = VIEW_TYPE_XWAYLAND_UNMANAGED;
    // view->interface = &xwayland_view_interface;
    // view->xwayland_surface = xwayland_surface;
    // view->server = server;

    /* Listen to the various events it can emit */
    xwayland_view->map.notify = xwayland_unmanaged_map;
    wl_signal_add(&surface->events.map, &xwayland_view->map);

    xwayland_view->unmap.notify = xwayland_unmanaged_unmap;
    wl_signal_add(&surface->events.unmap, &xwayland_view->unmap);

    xwayland_view->destroy.notify = xwayland_unmanaged_destroy;
    wl_signal_add(&surface->events.destroy, &xwayland_view->destroy);

    xwayland_view->request_configure.notify = xwayland_unmanaged_request_configure;
    wl_signal_add(&surface->events.request_configure,
        &xwayland_view->request_configure);

    return xwayland_view;
}

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

static void xwayland_get_geometry(struct tbx_view* view, struct wlr_box* box)
{
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

    if (!view->xwayland_surface || !view->surface) {
        return;
    }

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

static void xwayland_surface_destroy(struct wl_listener* listener, void* data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, destroy);
    struct tbx_view* view = &xwayland_view->view;

    view->interface->destroy(view);
}

static void xwayland_surface_map(struct wl_listener* listener, void* data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct wlr_xwayland_surface* xsurface = data;

    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, map);
    struct tbx_view* view = &xwayland_view->view;

    if (xsurface->override_redirect) {
        // This window used not to have the override redirect flag and has it
        // now. Switch to unmanaged.
        xsurface->data = NULL;
        struct tbx_xwayland_unmanaged *unmanaged = create_xwayland_unmanaged(xsurface);
        unmanaged->server = view->server;
        wl_list_insert(&view->server->unmanaged, &unmanaged->link);

        // remove from managed
        xwayland_surface_destroy(&xwayland_view->destroy, view);

        xwayland_unmanaged_map(&unmanaged->map, xsurface);
        return;
    }

    view->mapped = true;
    view->title_dirty = true;
    view->surface = xsurface->surface;
    view->xwayland_surface = xsurface;

    view_set_focus(view, view->surface);
    view_move_to_center(view, NULL);
}

static void xwayland_surface_unmap(struct wl_listener* listener, void* data)
{
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, unmap);
    struct tbx_view* view = &xwayland_view->view;
    view->surface = NULL;
    view->mapped = false;
}

static void xwayland_request_configure(struct wl_listener* listener,
    void* data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, request_configure);
    struct tbx_view* view = &xwayland_view->view;

    struct wlr_xwayland_surface_configure_event* ev = data;
    struct wlr_xwayland_surface* xsurface = view->xwayland_surface;
    
    if (!xsurface->mapped) {
        wlr_xwayland_surface_configure(xsurface, ev->x, ev->y, ev->width, ev->height);
    }

    view->x = ev->x;
    view->y = ev->y;
    view->width = ev->width;
    view->height = ev->height;
}

static void new_xwayland_surface(struct wl_listener* listener, void* data)
{
    struct wlr_xwayland_surface* xwayland_surface = data;

    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
    struct tbx_xwayland_shell* xwayland_shell = wl_container_of(listener, xwayland_shell, new_xwayland_surface);
    struct tbx_server* server = xwayland_shell->server;

    if (xwayland_surface->override_redirect) {
        console_log("new xwayland unmanaged surface");
        
        struct tbx_xwayland_unmanaged *unmanaged = create_xwayland_unmanaged(xwayland_surface);
        unmanaged->server = server;
        wl_list_insert(&server->unmanaged, &unmanaged->link);
        return;
    }

    console_log("new surface");

    /* Allocate a tbx_view for this surface */
    struct tbx_xwayland_view* xwayland_view = calloc(1, sizeof(struct tbx_xwayland_view));
    struct tbx_view* view = &xwayland_view->view;
    view->view_type = VIEW_TYPE_XWAYLAND;
    view->interface = &xwayland_view_interface;

    view->xwayland_surface = xwayland_surface;
    view->server = server;

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

    xwayland_set_activated(view, true);
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