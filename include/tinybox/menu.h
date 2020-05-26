#ifndef TINYBOX_MENU_H
#define TINYBOX_MENU_H

#include "tinybox/command.h"
#include "tinybox/view.h"
#include <cairo/cairo.h>

struct tbx_command;
struct tbx_output;

enum menu_type {
    TBX_MENU,
    TBX_MENU_ITEM
};

struct tbx_menu;
struct tbx_menu_view {
    struct tbx_view view;
    struct tbx_menu *menu;
};

struct tbx_menu {
    struct tbx_command command;
    enum menu_type menu_type;

    char* title; // menu
    char* label; // item

    bool shown;
    bool pinned;
    bool reversed; // reverse direction right-to-left

    // local coors ~ relative to parent
    int x;
    int y;
    int width;
    int height;

    // world coords
    int menu_x;
    int menu_y;
    int menu_width;
    int menu_height;

    struct wl_list items;
    struct tbx_menu* parent;

    // temporary
    cairo_surface_t* text_image;
    cairo_surface_t* text_hilite_image;
    int text_image_width;
    int text_image_height;

    struct wlr_box title_box;
    struct wlr_box frame_box;
    struct wlr_texture* item_texture;
    struct wlr_texture* menu_texture;
    int lastStyleHash;

    struct tbx_menu* hovered;
    struct tbx_menu* submenu;

    char* execute;
    int argc;
    char** argv;

    struct tbx_menu_view view;
};

struct tbx_menu* menu_at(struct tbx_server* server, int x, int y);
void menu_close(struct tbx_menu* menu);
void menu_close_all(struct tbx_server* server);
void menu_show(struct tbx_menu* menu, int x, int y);
void menu_show_submenu(struct tbx_menu* menu, struct tbx_menu* sub_menu);
void menu_execute(struct tbx_server* server, struct tbx_menu* item);
void menu_navigation(struct tbx_server* server, uint32_t keycode);

void menu_setup(struct tbx_server* server, struct tbx_menu *menu);
void prerender_menu(struct tbx_server* server, struct tbx_menu *menu);
void render_menus(struct tbx_output* output);

#endif // TINYBOX_MENU_H