#include "menu.h"

#define WIDTH 400
#define HEIGHT 600

static struct wlr_egl egl;

static void nop() {}

static int window_id = 0;
struct tinybox_window *window_create(struct tinybox_menu *menu);
void window_destroy(struct tinybox_window *window);

static void draw(struct tinybox_window *window) {
    if (!window->configured) {
        return;
    }

    eglMakeCurrent(egl.display, window->egl_surface, window->egl_surface, egl.context);

    float color[] = {1.0 * ((window->window_id + 1) * 50 /255), 1.0, 0.0, 1.0};

    glViewport(0, 0, window->width, window->height);
    glClearColor(color[0], color[1], color[2], 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(egl.display, window->egl_surface);
}

static void pointer_handle_button(void *data, struct wl_pointer *pointer, uint32_t serial,
                                  uint32_t time, uint32_t button, uint32_t state_w) {
    struct tinybox_menu *menu = data;
    if (state_w == WLR_BUTTON_RELEASED) {
        window_create(menu); // create popup
       // xdg_toplevel_destroy(menu->root->xdg_toplevel);
    }
}

static void pointer_handle_enter(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t serial, struct wl_surface *surface,
                                 wl_fixed_t surface_x, wl_fixed_t surface_y) {
    // This space intentionally left blank
}

static void pointer_handle_leave(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t serial, struct wl_surface *surface) {
    // This space intentionally left blank
}

static void pointer_handle_motion(void *data, struct wl_pointer *wl_pointer,
                                  uint32_t time, wl_fixed_t surface_x, wl_fixed_t surface_y) {
    // This space intentionally left blank
}

static const struct wl_pointer_listener pointer_listener = {
    .enter = pointer_handle_enter,
    .leave = pointer_handle_leave,
    .motion = pointer_handle_motion,
    .button = pointer_handle_button,
    nop,
    nop,
    nop,
    nop,
    nop
};

static void xdg_surface_handle_configure(void *data,
        struct xdg_surface *xdg_surface, uint32_t serial) {
    xdg_surface_ack_configure(xdg_surface, serial);

    struct tinybox_window *window = data;
    wl_egl_window_resize(window->egl_window, window->width, window->height, 0, 0);
    
    window->configured = true;

    printf("configured %d\n", window->window_id);
    draw(window);
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_handle_configure,
};

static void xdg_toplevel_handle_configure(void *data,
        struct xdg_toplevel *xdg_toplevel, int32_t width, int32_t height,
        struct wl_array *states) {
    struct tinybox_window *window = data;
    window->width = width;
    window->height = height;
}

static void xdg_toplevel_handle_close(void *data,
                                      struct xdg_toplevel *xdg_toplevel) {
    struct tinybox_window *window = data;
    window_destroy(window);
}

static const struct xdg_toplevel_listener xdg_toplevel_listener = {
    .configure = xdg_toplevel_handle_configure,
    .close = xdg_toplevel_handle_close,
};

static void xdg_poup_handle_configure(void *data,
              struct xdg_popup *xdg_popup,
              int32_t x,
              int32_t y,
              int32_t width,
              int32_t height)
{
    struct tinybox_window *window = data;
    window->width = width;
    window->height = height;
}

static void xdg_popup_handle_done(void *data,
               struct xdg_popup *xdg_popup)
{
    struct tinybox_window *window = data;
    printf("popup done\n");
    window_destroy(window);
}

static const struct xdg_popup_listener xdg_popup_listener = {
    xdg_poup_handle_configure,
    xdg_popup_handle_done,
    nop
};

static void deco_client_mode(void *data,
                             struct org_kde_kwin_server_decoration *org_kde_kwin_server_decoration,
                             uint32_t mode)
{
    struct tinybox_window *window = data;
    org_kde_kwin_server_decoration_request_mode(window->client_decoration, ORG_KDE_KWIN_SERVER_DECORATION_MODE_CLIENT);
}

static struct org_kde_kwin_server_decoration_listener deco_client_listener = {
    deco_client_mode
};

static void handle_global(void *data, struct wl_registry *registry,
                          uint32_t name, const char *interface, uint32_t version) {

    struct tinybox_menu *menu = data;

    printf("%s\n", interface);

    if (strcmp(interface, wl_compositor_interface.name) == 0) {
        menu->compositor = wl_registry_bind(registry, name, &wl_compositor_interface, 1);
    } else if (strcmp(interface, xdg_wm_base_interface.name) == 0) {
         menu->wm_base = wl_registry_bind(registry, name, &xdg_wm_base_interface, 1);
    } else if (strcmp(interface, wl_seat_interface.name) == 0) {
         menu->seat = wl_registry_bind(registry, name, &wl_seat_interface, version);
    } else if (strcmp(interface, org_kde_kwin_server_decoration_manager_interface.name) == 0) {
         menu->server_decoration = wl_registry_bind(registry, name, &org_kde_kwin_server_decoration_manager_interface, version);
    }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
                                 uint32_t name) {
    // who cares
}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

struct tinybox_window* window_create(struct tinybox_menu *menu) {
    struct tinybox_window *window = calloc(1, sizeof(struct tinybox_window));
    window->window_id = window_id++;
    window->menu = menu;
    window->display = menu->display;

    window->surface = wl_compositor_create_surface(menu->compositor);
    window->xdg_surface = xdg_wm_base_get_xdg_surface(menu->wm_base, window->surface);
    xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener, window);

    if (menu->root && 0) {

        struct xdg_positioner *positioner = xdg_wm_base_create_positioner(menu->wm_base);
        window->xdg_popup = xdg_surface_get_popup(window->xdg_surface, menu->root->xdg_surface, positioner);
        xdg_popup_add_listener(window->xdg_popup, &xdg_popup_listener, window);

    } else {
        window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
    }

    // xdg_popup_interface

    if (window->xdg_toplevel) {
        xdg_toplevel_add_listener(window->xdg_toplevel, &xdg_toplevel_listener, window);
    }

    if (menu->server_decoration) {
        window->client_decoration = org_kde_kwin_server_decoration_manager_create(menu->server_decoration, window->surface);
        org_kde_kwin_server_decoration_add_listener(window->client_decoration, &deco_client_listener, window);
    }

    wl_surface_commit(window->surface);

    window->egl = egl;
    window->egl_window = wl_egl_window_create(window->surface, WIDTH, HEIGHT);
    window->egl_surface = wlr_egl_create_surface(&egl, window->egl_window);
    
    wl_list_insert(&menu->windows, &window->link);

    return window;
}

void window_destroy(struct tinybox_window *window) {

    printf("x:%d\n", window->window_id);
    wl_list_remove(&window->link);
    wl_egl_window_destroy(window->egl_window);
    wlr_egl_destroy_surface(&egl, window->egl_surface);

    if (window->menu->root == window) {
        window->menu->root = NULL;
    }

    free(window);
}

bool menu_setup(struct tinybox_menu *menu)
{
    menu->display = wl_display_connect(NULL);
    if (menu->display == NULL) {
        fprintf(stderr, "Failed to create display\n");
        return EXIT_FAILURE;
    }

    wl_list_init(&menu->windows);

    struct wl_registry *registry = wl_display_get_registry(menu->display);
    wl_registry_add_listener(registry, &registry_listener, menu);
    wl_display_roundtrip(menu->display);

    if (menu->compositor == NULL) {
        fprintf(stderr, "wl-compositor not available\n");
        return EXIT_FAILURE;
    }
    if (menu->wm_base == NULL) {
        fprintf(stderr, "xdg-shell not available\n");
        return EXIT_FAILURE;
    }

    struct wl_pointer *pointer = wl_seat_get_pointer(menu->seat);
    wl_pointer_add_listener(pointer, &pointer_listener, menu);

    wlr_egl_init(&egl, EGL_PLATFORM_WAYLAND_EXT, menu->display, NULL,
                 WL_SHM_FORMAT_ARGB8888);

    menu->root = window_create(menu);

    wl_display_roundtrip(menu->display);

    wl_registry_destroy(registry);
    return true;
}

void menu_run(struct tinybox_menu *menu)
{
    while (wl_display_dispatch(menu->display) != -1) {
        struct tinybox_window *window;
        wl_list_for_each(window, &menu->windows, link) {
            draw(window);
        }
        // This space intentionally left blank
    }
}

void menu_destroy(struct tinybox_menu *menu)
{}