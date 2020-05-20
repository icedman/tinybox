#ifndef TINYBOX_VIEW_H
#define TINYBOX_VIEW_H

#include "tinybox/server.h"

struct tbx_view;
struct tbx_output;

enum tbx_view_hotspot {
    HS_EDGE_TOP_LEFT,
    HS_EDGE_TOP_RIGHT,
    HS_EDGE_BOTTOM_LEFT,
    HS_EDGE_BOTTOM_RIGHT,
    HS_EDGE_TOP,
    HS_EDGE_BOTTOM,
    HS_EDGE_LEFT,
    HS_EDGE_RIGHT,
    HS_TITLEBAR,
    HS_HANDLE,
    HS_COUNT,
    HS_NONE = -1
};

enum tbx_view_prop {
    VIEW_PROP_TITLE,
    VIEW_PROP_APP_ID,
    VIEW_PROP_CLASS,
    VIEW_PROP_INSTANCE,
    VIEW_PROP_WINDOW_TYPE,
    VIEW_PROP_WINDOW_ROLE,
    // #if HAVE_XWAYLAND
    VIEW_PROP_X11_WINDOW_ID,
    VIEW_PROP_X11_PARENT_ID,
    // #endif
};

enum tbx_view_type {
    VIEW_TYPE_XDG,
    // #if HAVE_XWAYLAND
    VIEW_TYPE_XWAYLAND
    // #endif
};

struct tbx_view_interface {
    void (*get_constraints)(struct tbx_view* view, double* min_width,
        double* max_width, double* min_height, double* max_height);
    void (*get_geometry)(struct tbx_view* view, struct wlr_box*);
    const char* (*get_string_prop)(struct tbx_view* view,
        enum tbx_view_prop prop);
    uint32_t (*get_int_prop)(struct tbx_view* view, enum tbx_view_prop prop);
    uint32_t (*configure)(struct tbx_view* view, double lx, double ly,
        int width, int height);
    void (*set_activated)(struct tbx_view* view, bool activated);
    void (*set_fullscreen)(struct tbx_view* view, bool fullscreen);
    // void (*for_each_surface)(struct tbx_view *view,
    //     wlr_surface_iterator_func_t iterator, void *user_data);
    // void (*for_each_popup)(struct tbx_view *view,
    //     wlr_surface_iterator_func_t iterator, void *user_data);
    bool (*is_transient_for)(struct tbx_view* child,
        struct tbx_view* ancestor);
    void (*close)(struct tbx_view* view);
    void (*close_popups)(struct tbx_view* view);
    void (*destroy)(struct tbx_view* view);
};

struct tbx_view {
    struct wl_list link;
    struct tbx_server* server;

    enum tbx_view_type view_type;
    struct tbx_view_interface* interface;

    struct wlr_surface* surface; // NULL for unmapped views
    struct wlr_xdg_surface* xdg_surface;
    struct wlr_xwayland_surface* xwayland_surface;

    bool mapped;
    int x, y;
    int lx, ly; // last x, y for damage?

    // xwayland
    int width, height;
    bool override_redirect;

    bool shaded;
    bool csd;
    bool fullscreen;

    struct {
        int x;
        int y;
        int width;
        int height;
    } restore;

    struct tbx_view* parent;

    // animate
    bool wsv_animate;
    double wsv_anim_x;
    double wsv_anim_y;

    // title
    struct wlr_texture* title;
    struct wlr_texture* title_unfocused;
    struct wlr_box title_box;
    bool title_dirty;

    // hotspots
    struct wlr_box hotspots[HS_COUNT];
    enum tbx_view_hotspot hotspot;
    uint32_t hotspot_edges;

    struct wlr_box request_box;
    int request_wait;

    // workspace
    int workspace;

    // damage tracking
    struct wlr_box damage;
};

struct tbx_xdg_shell_view {
    struct tbx_view view;

    struct wl_listener commit;

    struct wl_listener _first;
    struct wl_listener request_move;
    struct wl_listener request_resize;
    // struct wl_listener request_maximize;
    struct wl_listener request_fullscreen;
    struct wl_listener set_title;
    // struct wl_listener set_app_id;
    // struct wl_listener new_popup;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
};

// #if HAVE_XWAYLAND
struct tbx_xwayland_view {
    struct tbx_view view;

    struct wl_listener commit;

    struct wl_listener _first;
    // struct wl_listener request_move;
    // struct wl_listener request_resize;
    // struct wl_listener request_maximize;
    struct wl_listener request_configure;
    struct wl_listener request_fullscreen;
    // struct wl_listener request_activate;
    struct wl_listener set_title;
    // struct wl_listener set_class;
    // struct wl_listener set_role;
    // struct wl_listener set_window_type;
    // struct wl_listener set_hints;
    // struct wl_listener set_decorations;
    struct wl_listener map;
    struct wl_listener unmap;
    struct wl_listener destroy;
};
// #endif

struct tbx_view* view_from_surface(struct tbx_server* server, struct wlr_surface* surface);

struct tbx_view* desktop_view_at(struct tbx_server* server, double lx,
    double ly, struct wlr_surface** surface,
    double* sx, double* sy);

bool view_at(struct tbx_view* view, double lx, double ly,
    struct wlr_surface** surface, double* sx, double* sy);

void view_set_focus(struct tbx_view* view, struct wlr_surface* surface);

void view_move_to_center(struct tbx_view* view, struct tbx_output* output);

bool view_hotspot_at(struct tbx_view* view, double lx, double ly,
    struct wlr_surface** surface, double* sx, double* sy);

int view_is_visible(struct tbx_view* view, struct tbx_output* output);

void view_send_to_workspace(struct tbx_view* view, int id,
    bool animate);

struct tbx_output* view_get_preferred_output(struct tbx_view* view);

void view_setup(struct tbx_view* view);

void view_destroy(struct tbx_view* view);

void view_damage(struct tbx_view* view);

#endif // TINYBOX_VIEW_H