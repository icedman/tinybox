#include "tinybox/server.h"
#include "tinybox/shell.h"
#include "tinybox/view.h"

#include <float.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>

/*
/
/ void (*for_each_surface)(struct tbx_view *view,
//     wlr_surface_iterator_func_t iterator, void *user_data);
// void (*for_each_popup)(struct tbx_view *view,
//     wlr_surface_iterator_func_t iterator, void *user_data);
// bool (*is_transient_for)(struct tbx_view *child,
//         struct tbx_view *ancestor);
*/

static void xdg_get_constraints(struct tbx_view* view, double* min_width,
    double* max_width, double* min_height, double* max_height)
{
    struct wlr_xdg_toplevel_state* state = &view->xdg_surface->toplevel->current;
    *min_width = state->min_width > 0 ? state->min_width : DBL_MIN;
    *max_width = state->max_width > 0 ? state->max_width : DBL_MAX;
    *min_height = state->min_height > 0 ? state->min_height : DBL_MIN;
    *max_height = state->max_height > 0 ? state->max_height : DBL_MAX;
}

static void xdg_get_geometry(struct tbx_view* view, struct wlr_box* box)
{
    wlr_xdg_surface_get_geometry(view->xdg_surface, box);
}

static void xdg_set_activated(struct tbx_view* view, bool activated)
{
    struct wlr_seat* seat = view->server->seat->seat;
    struct wlr_keyboard* keyboard = wlr_seat_get_keyboard(seat);

    if (!view->xdg_surface || !view->xdg_surface->surface) {
        return;
    }

    /*
     * Tell the seat to have the keyboard enter this surface. wlroots will keep
     * track of this and automatically send key events to the appropriate
     * clients without additional work on your part.
     */
    if (activated) {
        wlr_seat_keyboard_notify_enter(seat, view->xdg_surface->surface,
            keyboard->keycodes, keyboard->num_keycodes,
            &keyboard->modifiers);
    }

    wlr_xdg_toplevel_set_activated(view->xdg_surface, activated);
}

static void xdg_set_fullscreen(struct tbx_view* view, bool fullscreen)
{
}

static const char* xdg_get_string_prop(struct tbx_view* view, enum tbx_view_prop prop)
{
    switch (prop) {
    case VIEW_PROP_TITLE:
        return view->xdg_surface->toplevel->title;
    case VIEW_PROP_APP_ID:
        return view->xdg_surface->toplevel->app_id;
    default:
        return NULL;
    }
}

uint32_t xdg_get_int_prop(struct tbx_view* view, enum tbx_view_prop prop)
{
    return 0;
}

static uint32_t xdg_view_configure(struct tbx_view* view, double lx, double ly,
    int width, int height)
{
    struct wlr_box geo_box;
    wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);
    view->x = lx - geo_box.x;
    view->y = ly - geo_box.y;

    wlr_xdg_toplevel_set_size(view->xdg_surface, width, height);
    view->request_box.x = view->x;
    view->request_box.y = view->y;
    view->request_box.width = width;
    view->request_box.height = height;
    return 0;
}

static void xdg_close(struct tbx_view* view)
{
}

static void xdg_close_popups(struct tbx_view* view)
{
}

static void xdg_destroy(struct tbx_view* view)
{
    view_destroy(view);
}

static struct tbx_view_interface xdg_view_interface = {
    .get_constraints = xdg_get_constraints,
    .get_geometry = xdg_get_geometry,
    .get_string_prop = xdg_get_string_prop,
    .get_int_prop = xdg_get_int_prop,
    .configure = xdg_view_configure,
    .set_activated = xdg_set_activated,
    .set_fullscreen = xdg_set_fullscreen,
    .close = xdg_close,
    .close_popups = xdg_close_popups,
    .destroy = xdg_destroy
};

static void xdg_surface_map(struct wl_listener* listener, void* data)
{
    /* Called when the surface is mapped, or ready to display on-screen. */
    struct tbx_xdg_shell_view* xdg_shell_view = wl_container_of(listener, xdg_shell_view, map);
    struct tbx_view* view = &xdg_shell_view->view;
    view->mapped = true;
    view->title_dirty = true;
    view->surface = (struct wlr_surface*)view->xdg_surface;

    view_set_focus(view, view->xdg_surface->surface);
}

static void xdg_surface_unmap(struct wl_listener* listener, void* data)
{
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct tbx_xdg_shell_view* xdg_shell_view = wl_container_of(listener, xdg_shell_view, unmap);
    struct tbx_view* view = &xdg_shell_view->view;
    view->surface = NULL;
    view->mapped = false;
}

static void xdg_surface_destroy(struct wl_listener* listener, void* data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    struct tbx_xdg_shell_view* xdg_shell_view = wl_container_of(listener, xdg_shell_view, destroy);
    struct tbx_view* view = &xdg_shell_view->view;
    view->interface->destroy(view);
}

static void begin_interactive(struct tbx_view* view, enum tbx_cursor_mode mode,
    uint32_t edges)
{
    /* This function sets up an interactive move or resize operation, where the
   * compositor stops propegating pointer events to clients and instead
   * consumes them itself, to move or resize windows. */
    struct tbx_server* server = view->server;
    struct wlr_surface* focused_surface = server->seat->seat->pointer_state.focused_surface;

    if (view->xdg_surface->surface != focused_surface) {
        /* Deny move/resize requests from unfocused clients. */
        return;
    }

    struct tbx_cursor* cursor = server->cursor;
    cursor->grab_view = view;
    cursor->mode = mode;

    if (mode == TBX_CURSOR_MOVE) {
        cursor->grab_x = cursor->cursor->x - view->x;
        cursor->grab_y = cursor->cursor->y - view->y;

    } else {

        struct wlr_box geo_box;
        wlr_xdg_surface_get_geometry(view->xdg_surface, &geo_box);

        double border_x = (view->x + geo_box.x) + ((edges & WLR_EDGE_RIGHT) ? geo_box.width : 0);
        double border_y = (view->y + geo_box.y) + ((edges & WLR_EDGE_BOTTOM) ? geo_box.height : 0);
        cursor->grab_x = cursor->cursor->x - border_x;
        cursor->grab_y = cursor->cursor->y - border_y;

        cursor->grab_box = geo_box;
        cursor->grab_box.x += view->x;
        cursor->grab_box.y += view->y;

        cursor->resize_edges = edges;
    }
}

static void xdg_toplevel_request_move(struct wl_listener* listener,
    void* data)
{
    /* This event is raised when a client would like to begin an interactive
   * move, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
    struct tbx_xdg_shell_view* xdg_shell_view = wl_container_of(listener, xdg_shell_view, request_move);
    struct tbx_view* view = &xdg_shell_view->view;
    begin_interactive(view, TBX_CURSOR_MOVE, 0);
}

static void xdg_toplevel_request_resize(struct wl_listener* listener,
    void* data)
{
    /* This event is raised when a client would like to begin an interactive
   * resize, typically because the user clicked on their client-side
   * decorations. Note that a more sophisticated compositor should check the
   * provied serial against a list of button press serials sent to this
   * client, to prevent the client from requesting this whenever they want. */
    struct wlr_xdg_toplevel_resize_event* event = data;
    struct tbx_xdg_shell_view* xdg_shell_view = wl_container_of(listener, xdg_shell_view, request_resize);
    struct tbx_view* view = &xdg_shell_view->view;
    begin_interactive(view, TBX_CURSOR_RESIZE, event->edges);
}

static void handle_set_title(struct wl_listener* listener, void* data)
{
    struct tbx_xdg_shell_view* xdg_shell_view = wl_container_of(listener, xdg_shell_view, set_title);
    struct tbx_view* view = &xdg_shell_view->view;
    view->title_dirty = true;
}

static void server_new_xdg_surface(struct wl_listener* listener, void* data)
{

    /* This event is raised when wlr_xdg_shell receives a new xdg surface from a
   * client, either a toplevel (application window) or popup. */
    struct tbx_xdg_shell* xdg_shell = wl_container_of(listener, xdg_shell, new_xdg_surface);
    struct tbx_server* server = xdg_shell->server;

    struct wlr_xdg_surface* xdg_surface = data;
    if (xdg_surface->role != WLR_XDG_SURFACE_ROLE_TOPLEVEL) {
        return;
    }

    /* Allocate a tbx_view for this surface */
    struct tbx_xdg_shell_view* xdg_shell_view = calloc(1, sizeof(struct tbx_xdg_shell_view));
    struct tbx_view* view = &xdg_shell_view->view;
    view->view_type = VIEW_TYPE_XDG_SHELL;
    view->interface = &xdg_view_interface;

    view->xdg_surface = xdg_surface;
    view->server = server;

    xdg_shell->create_offset = (xdg_shell->create_offset + 1) % 8;
    view->x = 4 + (xdg_shell->create_offset * 40);
    view->y = 32 + (xdg_shell->create_offset * 40);

    /* Listen to the various events it can emit */
    xdg_shell_view->map.notify = xdg_surface_map;
    wl_signal_add(&xdg_surface->events.map, &xdg_shell_view->map);
    xdg_shell_view->unmap.notify = xdg_surface_unmap;
    wl_signal_add(&xdg_surface->events.unmap, &xdg_shell_view->unmap);
    xdg_shell_view->destroy.notify = xdg_surface_destroy;
    wl_signal_add(&xdg_surface->events.destroy, &xdg_shell_view->destroy);

    /* cotd */
    struct wlr_xdg_toplevel* toplevel = xdg_surface->toplevel;
    xdg_shell_view->request_move.notify = xdg_toplevel_request_move;
    wl_signal_add(&toplevel->events.request_move, &xdg_shell_view->request_move);
    xdg_shell_view->request_resize.notify = xdg_toplevel_request_resize;
    wl_signal_add(&toplevel->events.request_resize, &xdg_shell_view->request_resize);

    /* title */
    xdg_shell_view->set_title.notify = handle_set_title;
    wl_signal_add(&toplevel->events.set_title, &xdg_shell_view->set_title);

    // move to workspace
    view->workspace = server->workspace;

    /* Add it to the list of views. */
    wl_list_insert(&server->views, &view->link);

    xdg_set_activated(view, true);
}

bool xdg_shell_setup(struct tbx_server* server)
{
    /* Set up our list of views and the xdg-shell. The xdg-shell is a Wayland
   * protocol which is used for application windows. For more detail on
   * shells, refer to my article:
   *
   * https://drewdevault.com/2018/07/29/Wayland-shells.html
   */
    server->xdg_shell = calloc(1, sizeof(struct tbx_xdg_shell));
    server->xdg_shell->server = server;
    server->xdg_shell->wlr_xdg_shell = wlr_xdg_shell_create(server->wl_display);
    server->xdg_shell->new_xdg_surface.notify = server_new_xdg_surface;
    wl_signal_add(&server->xdg_shell->wlr_xdg_shell->events.new_surface,
        &server->xdg_shell->new_xdg_surface);

    wl_list_init(&server->views);

    return true;
}