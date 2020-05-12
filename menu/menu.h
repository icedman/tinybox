#ifndef MENU_H
#define MENU_H

#include <GLES2/gl2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wayland-client.h>
#include <wayland-egl.h>
#include <wlr/render/egl.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include "xdg-shell-client-protocol.h"
#include "server-decoration-client-protocol.h"

// struct tinybox_pointer {
//     struct wl_pointer *pointer;
//     uint32_t serial;
//     struct wl_cursor_theme *cursor_theme;
//     struct wl_cursor_image *cursor_image;
//     struct wl_surface *cursor_surface;
//     int x;
//     int y;
// };

// struct tinybox_seat {
//     struct wl_seat *wl_seat;
//     uint32_t wl_name;
//     struct tinybox_menu *menu;
//     struct tinybox_pointer pointer;
//     struct wl_list link;
// };

// struct tinybox_output {
//     char *name;
//     struct wl_output *wl_output;
//     uint32_t wl_name;
//     uint32_t scale;
//     struct tinybox_menu *menu;
//     struct wl_list link;
// };

struct tinybox_window {
    int window_id;
    struct wlr_egl egl;
    struct wl_egl_window *egl_window;
    struct wlr_egl_surface *egl_surface;
    struct wl_display *display;

    struct wl_surface *surface;
    struct xdg_surface *xdg_surface;
    struct xdg_toplevel *xdg_toplevel;
    struct xdg_popup *xdg_popup;
    struct org_kde_kwin_server_decoration *client_decoration;

    int width;
    int height;

    struct wl_list link;
    struct tinybox_menu *menu;
    bool configured;
};

struct tinybox_menu {
    struct wl_display *display;
    struct wl_compositor *compositor;
    struct wl_seat *seat;
    struct xdg_wm_base *wm_base;
    struct org_kde_kwin_server_decoration_manager *server_decoration;
    
    struct wl_list windows;
    struct tinybox_window *root;
};

bool menu_setup(struct tinybox_menu *);
void menu_run(struct tinybox_menu *);
void menu_destroy(struct tinybox_menu *);

#endif // MENU_H