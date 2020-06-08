#define _POSIX_C_SOURCE 200112L

#include "tinybox/damage.h"
#include "tinybox/output.h"
#include "tinybox/server.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"
#include "tinybox/xwayland.h"

#include <float.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>

#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_xcursor_manager.h>
#include <wlr/types/wlr_xdg_shell.h>
#include <wlr/xwayland.h>

// copy now ~ understand later
static const char* atom_map[ATOM_LAST] = {
    "_NET_WM_WINDOW_TYPE_NORMAL",
    "_NET_WM_WINDOW_TYPE_DIALOG",
    "_NET_WM_WINDOW_TYPE_UTILITY",
    "_NET_WM_WINDOW_TYPE_TOOLBAR",
    "_NET_WM_WINDOW_TYPE_SPLASH",
    "_NET_WM_WINDOW_TYPE_MENU",
    "_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
    "_NET_WM_WINDOW_TYPE_POPUP_MENU",
    "_NET_WM_WINDOW_TYPE_TOOLTIP",
    "_NET_WM_WINDOW_TYPE_NOTIFICATION",
    "_NET_WM_STATE_MODAL",
};

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
    console_log("set activated");

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

    wlr_xwayland_surface_activate(view->xwayland_surface, activated);
}

static void xwaylan_set_fullscreen(struct tbx_view* view, bool fullscreen)
{
    if (!view->xwayland_surface) {
        return;
    }

    console_log("fullscreen %d", fullscreen);

    if (fullscreen) {
        view->fullscreen = fullscreen;
        view->restore.x = view->x;
        view->restore.y = view->y;

        struct wlr_box window_box;
        xwayland_get_geometry(view, &window_box);
        view->restore.width = window_box.width;
        view->restore.height = window_box.height;

        // todo get output from view
        struct wlr_box* full_box = wlr_output_layout_get_box(
            view->server->output_layout, view->server->main_output->wlr_output);

        view->x = 0;
        view->y = 0;
        view->width = full_box->width;
        view->height = full_box->height;
        wlr_xwayland_surface_configure(view->xwayland_surface,
            0, 0, full_box->width, full_box->height);
    } else {
        view->fullscreen = fullscreen;
        view->x = view->restore.x;
        view->y = view->restore.y;
        view->width = view->restore.width;
        view->height = view->restore.height;
        wlr_xwayland_surface_configure(view->xwayland_surface, 0,
            0, view->restore.width, view->restore.height);
    }

    wlr_xwayland_surface_set_fullscreen(view->xwayland_surface, fullscreen);
}

static const char* xwayland_get_string_prop(struct tbx_view* view, enum tbx_view_prop prop)
{
    switch (prop) {
    case VIEW_PROP_TITLE:
        return view->xwayland_surface->title;
    case VIEW_PROP_CLASS:
        return view->xwayland_surface->class;
    case VIEW_PROP_INSTANCE:
        return view->xwayland_surface->instance;
    case VIEW_PROP_WINDOW_ROLE:
        return view->xwayland_surface->role;
    default:
        return NULL;
    }
}

uint32_t xwayland_get_int_prop(struct tbx_view* view, enum tbx_view_prop prop)
{
    switch (prop) {
    case VIEW_PROP_X11_WINDOW_ID:
        return view->xwayland_surface->window_id;
    case VIEW_PROP_X11_PARENT_ID:
        if (view->xwayland_surface->parent) {
            return view->xwayland_surface->parent->window_id;
        }
        return 0;
    case VIEW_PROP_WINDOW_TYPE:
        if (view->xwayland_surface->window_type_len == 0) {
            return 0;
        }
        return view->xwayland_surface->window_type[0];
    default:
        return 0;
    }
    return 0;
}

static uint32_t xwayland_view_configure(struct tbx_view* view, double lx, double ly,
    int width, int height)
{
    wlr_xwayland_surface_configure(view->xwayland_surface, 0,
        0, width, height);

    damage_add_view(view->server, view);

    view->x = lx;
    view->y = ly;
    // view->width = width;
    // view->height = height;

    view->request_box.x = lx;
    view->request_box.y = ly;
    view->request_box.width = width;
    view->request_box.height = height;

    damage_add_view(view->server, view);
    return 0;
}

static void xwayland_close(struct tbx_view* view)
{
    if (view->xwayland_surface) {
        wlr_xwayland_surface_close(view->xwayland_surface);
    }
}

static void xwayland_close_popups(struct tbx_view* view)
{
}

static void xwayland_destroy(struct tbx_view* view)
{
    struct tbx_xwayland_view* xview = (struct tbx_xwayland_view*)view;

    struct wl_listener* l = &xview->_first;
    while (++l) {
        if (!l->link.prev || l == &xview->destroy) {
            break;
        }
        wl_list_remove(&l->link);
        // console_log("xway unlisten");
    }

    // free all listeners here!
    view_destroy(view);
}

static bool xwayland_is_transient_for(struct tbx_view* child,
    struct tbx_view* ancestor)
{

    struct wlr_xwayland_surface* surface = child->xwayland_surface;
    while (surface) {
        if (surface->parent == ancestor->xwayland_surface) {
            return true;
        }
        surface = surface->parent;
    }
    return false;
}

struct tbx_view_interface xwayland_view_interface = {
    .get_constraints = xwayland_get_constraints,
    .get_geometry = xwayland_get_geometry,
    .get_string_prop = xwayland_get_string_prop,
    .get_int_prop = xwayland_get_int_prop,
    .configure = xwayland_view_configure,
    .set_activated = xwayland_set_activated,
    .set_fullscreen = xwaylan_set_fullscreen,
    .is_transient_for = xwayland_is_transient_for,
    .close = xwayland_close,
    .close_popups = xwayland_close_popups,
    .destroy = xwayland_destroy
};

static struct tbx_view* get_root(struct tbx_view* view)
{
    if (view->parent) {
        return get_root(view->parent);
    }
    return view;
}

static void xwayland_view_set_parent(struct tbx_view* view)
{
    struct tbx_view* ancestor;
    struct wlr_xwayland_surface* xsurface = view->xwayland_surface;
    wl_list_for_each(ancestor, &view->server->views, link)
    {
        if (ancestor->view_type == VIEW_TYPE_XWAYLAND) {
            if (view->interface->is_transient_for(view, ancestor)) {
                view->parent = ancestor;

                struct tbx_view* root = get_root(ancestor);

                view->x = xsurface->x + root->x;
                view->y = xsurface->y + root->y;
                console_log("set parent %d", root->x);
                break;
            }
        }
    }
}

static void xwayland_view_try_set_parent(struct tbx_view* view)
{
    struct tbx_server* server = view->server;
    struct wlr_xwayland_surface* xsurface = view->xwayland_surface;
    uint32_t window_type = xwayland_get_int_prop(view, VIEW_PROP_WINDOW_TYPE);

    if (window_type == 0) {
        return;
    }

    struct tbx_view* _view;
    struct tbx_view* candidate = NULL;
    struct tbx_view* fallback = NULL;
    wl_list_for_each(_view, &server->views, link)
    {
        if (_view->view_type != VIEW_TYPE_XWAYLAND || _view == view) {
            continue;
        }
        int wt = xwayland_get_int_prop(_view, VIEW_PROP_WINDOW_TYPE);

        if (!_view->width || !_view->height) {
            continue;
        }

        double sx, sy;
        struct wlr_surface* surface;
        if (view_at(_view, server->cursor->cursor->x, server->cursor->cursor->y, &surface, &sx, &sy)) {
            if (wt == 0) {
                candidate = _view;
                break;
            } else {
                fallback = _view;
            }
        }
    }

    //-------------------------
    // hacky: adopt a parent?
    //-------------------------
    // properly implement popups
    if (!candidate) {
        struct tbx_view* focused = view_from_surface(server,
            server->seat->seat->keyboard_state.focused_surface);

        if (!focused || focused->view_type != VIEW_TYPE_XWAYLAND) {
            focused = NULL;
            struct tbx_view* ancestor;
            wl_list_for_each(ancestor, &view->server->views, link)
            {
                if (ancestor == view || ancestor->parent || !ancestor->mapped || !ancestor->x || ancestor->width < view->width) {
                    continue;
                }
                if (ancestor->view_type == VIEW_TYPE_XWAYLAND) {
                    focused = ancestor;
                    break;
                }
            }
        }

        while (focused && focused->parent) {
            focused = focused->parent;
        }

        candidate = focused;
    }

    if (!candidate) {
        candidate = fallback;
    }

    if (candidate && xsurface) {
        console_log("adopted %d %d %d", candidate->width, candidate->x, candidate->y);
        view->parent = candidate;

        // // (candidate->xwayland_surface->x) .. when surface coords not yet configured.. it wouldn't be zero
        view->x = xsurface->x + candidate->x - (candidate->xwayland_surface->x);
        view->y = xsurface->y + candidate->y - (candidate->xwayland_surface->y);
    }
}

static void xwayland_surface_commit(struct wl_listener* listener, void* data)
{
    // console_log("commit xwayland");
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, commit);
    struct tbx_view* view = &xwayland_view->view;
    damage_add_commit(view->server, view);
}

static void xwayland_surface_destroy(struct wl_listener* listener, void* data)
{
    /* Called when the surface is destroyed and should never be shown again. */
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, destroy);
    struct tbx_view* view = &xwayland_view->view;
    view->interface->destroy(view);
}

static void xwayland_surface_map(struct wl_listener* listener, void* data)
{
    console_log("map");

    /* Called when the surface is mapped, or ready to display on-screen. */
    struct wlr_xwayland_surface* xsurface = data;

    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, map);
    struct tbx_view* view = &xwayland_view->view;
    // struct tbx_server* server = view->server;

    view->mapped = true;
    view->title_dirty = true;
    view->surface = view->xwayland_surface->surface;

    damage_whole(view->server);

    // view->xwayland_surface = xsurface;
    // view->surface = xsurface->surface;

    xwayland_view->commit.notify = xwayland_surface_commit;
    if (xsurface->surface) {
        wl_signal_add(&xsurface->surface->events.commit, &xwayland_view->commit);
    }

    // if override redirect .. position as requested
    if (view->override_redirect && !view->parent) {

        if (view->xwayland_surface->parent) {
            xwayland_view_set_parent(view);
        } else {
            xwayland_view_try_set_parent(view);
            if (view->parent) {
                return;
            }
        }

        view_set_focus(view, view->surface);
        return;
    }

    view_set_focus(view, view->surface);

    // always set to zero
    wlr_xwayland_surface_configure(view->xwayland_surface, 0,
        0, view->width, view->height);

    if (!view->parent)
        view_move_to_center(view, NULL);
}

static void xwayland_surface_unmap(struct wl_listener* listener, void* data)
{
    /* Called when the surface is unmapped, and should no longer be shown. */
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, unmap);
    struct tbx_view* view = &xwayland_view->view;
    view->surface = NULL;
    view->mapped = false;

    damage_whole(view->server);

    if (view->xwayland_surface->surface) {
        wl_list_remove(&xwayland_view->commit.link);
    }
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

    if (view->parent) {
        view->x += view->parent->x;
        view->y += view->parent->y;
        console_log("conf with parent");
    }
}

static void xwayland_request_fullscreen(struct wl_listener* listener,
    void* data)
{
    struct tbx_xwayland_view* xwayland_view = wl_container_of(listener, xwayland_view, request_fullscreen);
    struct tbx_view* view = &xwayland_view->view;
    view->interface->set_fullscreen(view, !view->fullscreen);
}

static void xwayland_set_title(struct wl_listener* listener, void* data)
{
    struct tbx_xwayland_view* xview = wl_container_of(listener, xview, set_title);
    struct tbx_view* view = &xview->view;
    view->title_dirty = true;
}

static void xwayland_set_class(struct wl_listener* listener, void* data)
{
    struct tbx_xwayland_view* xview = wl_container_of(listener, xview, set_class);
    struct tbx_view* view = &xview->view;

    console_log("set_class %s", xwayland_get_string_prop(view, VIEW_PROP_CLASS));
}

static void xwayland_set_role(struct wl_listener* listener, void* data)
{
    struct tbx_xwayland_view* xview = wl_container_of(listener, xview, set_role);
    struct tbx_view* view = &xview->view;

    console_log("set_role %s", xwayland_get_string_prop(view, VIEW_PROP_WINDOW_ROLE));
}

static void xwayland_set_window_type(struct wl_listener* listener, void* data)
{
    struct tbx_xwayland_view* xview = wl_container_of(listener, xview, set_window_type);
    struct tbx_view* view = &xview->view;

    int wt = xwayland_get_int_prop(view, VIEW_PROP_WINDOW_TYPE);
    console_log("set_window_type %d", wt);

    // for(size_t i=0; i<view->xwayland_surface->window_type_len; i++) {
    //     console_log("window_type %d", view->xwayland_surface->window_type[i]);
    // }
}

static void new_xwayland_surface(struct wl_listener* listener, void* data)
{
    struct wlr_xwayland_surface* xsurface = data;

    struct tbx_xwayland_shell* xwayland_shell = wl_container_of(listener, xwayland_shell, new_xwayland_surface);
    struct tbx_server* server = xwayland_shell->server;

    // console_log("new surface");

    /* Allocate a tbx_view for this surface */
    struct tbx_xwayland_view* xwayland_view = calloc(1, sizeof(struct tbx_xwayland_view));
    struct tbx_view* view = &xwayland_view->view;
    view->view_type = VIEW_TYPE_XWAYLAND;
    view->interface = &xwayland_view_interface;

    view->xwayland_surface = xsurface;
    view->server = server;

    /* Listen to the various events it can emit */
    xwayland_view->map.notify = xwayland_surface_map;
    wl_signal_add(&xsurface->events.map, &xwayland_view->map);

    xwayland_view->unmap.notify = xwayland_surface_unmap;
    wl_signal_add(&xsurface->events.unmap, &xwayland_view->unmap);

    xwayland_view->destroy.notify = xwayland_surface_destroy;
    wl_signal_add(&xsurface->events.destroy, &xwayland_view->destroy);

    xwayland_view->request_configure.notify = xwayland_request_configure;
    wl_signal_add(&xsurface->events.request_configure,
        &xwayland_view->request_configure);

    xwayland_view->request_fullscreen.notify = xwayland_request_fullscreen;
    wl_signal_add(&xsurface->events.request_fullscreen,
        &xwayland_view->request_fullscreen);

    xwayland_view->set_title.notify = xwayland_set_title;
    wl_signal_add(&xsurface->events.set_title,
        &xwayland_view->set_title);

    xwayland_view->set_class.notify = xwayland_set_class;
    wl_signal_add(&xsurface->events.set_class,
        &xwayland_view->set_class);

    xwayland_view->set_role.notify = xwayland_set_role;
    wl_signal_add(&xsurface->events.set_role,
        &xwayland_view->set_role);

    xwayland_view->set_window_type.notify = xwayland_set_window_type;
    wl_signal_add(&xsurface->events.set_window_type,
        &xwayland_view->set_window_type);

    // move to workspace
    view->workspace = server->workspace;
    view->x = xsurface->x;
    view->y = xsurface->y;

    /* Add it to the list of views. */
    if (xsurface->override_redirect) {
        view->csd = true; // implement decoration listener
        view->override_redirect = true;
    }

    wl_list_insert(&server->views, &view->link);
    view_setup(view);
}

void handle_xwayland_ready(struct wl_listener* listener, void* data)
{
    struct tbx_xwayland_shell* xwayland = wl_container_of(listener, xwayland, xwayland_ready);
    // struct tbx_server *server = xwayland->server;

    xcb_connection_t* xcb_conn = xcb_connect(NULL, NULL);
    int err = xcb_connection_has_error(xcb_conn);
    if (err) {
        console_log("XCB connect failed: %d", err);
        return;
    }

    xcb_intern_atom_cookie_t cookies[ATOM_LAST];
    for (size_t i = 0; i < ATOM_LAST; i++) {
        cookies[i] = xcb_intern_atom(xcb_conn, 0, strlen(atom_map[i]), atom_map[i]);
    }
    for (size_t i = 0; i < ATOM_LAST; i++) {
        xcb_generic_error_t* error = NULL;
        xcb_intern_atom_reply_t* reply = xcb_intern_atom_reply(xcb_conn, cookies[i], &error);
        if (reply != NULL && error == NULL) {
            xwayland->atoms[i] = reply->atom;
        }
        free(reply);

        if (error != NULL) {
            // sway_log(SWAY_ERROR, "could not resolve atom %s, X11 error code %d",
            //     atom_map[i], error->error_code);
            free(error);
            break;
        }
    }

    xcb_disconnect(xcb_conn);
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

    server->xwayland_shell->xwayland_ready.notify = handle_xwayland_ready;
    wl_signal_add(&server->xwayland_shell->wlr_xwayland->events.new_surface,
        &server->xwayland_shell->xwayland_ready);

    setenv("DISPLAY", server->xwayland_shell->wlr_xwayland->display_name, true);
    return true;
}