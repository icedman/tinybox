#ifndef TINYBOX_CONFIG_H
#define TINYBOX_CONFIG_H

#include <stdbool.h>
#include <wayland-server-core.h>

struct tbx_keys_press;
struct tbx_server;

enum config_entry_type {
    TBX_CONFIG_DICTIONARY,
    TBX_CONFIG_INPUT,
    TBX_CONFIG_LAYOUT,
    TBX_CONFIG_KEYBINDING
};

struct tbx_config_entry {
    char* identifier;
    enum config_entry_type type;
    struct wl_list link;
};

struct tbx_config_dictionary {
    char* identifier;
    enum config_entry_type type;
    struct wl_list link;

    char* value;
};

struct tbx_config_input {
    char* identifier;
    enum config_entry_type type;
    struct wl_list link;

    int tap;
    int natural_scroll;
};

struct tbx_config_keybinding {
    char* identifier;
    enum config_entry_type type;
    struct wl_list link;

    struct tbx_keys_pressed* keys;
    char* command;
    int argc;
    char** argv;
};

struct tbx_config_layout {
    char* identifier;
    enum config_entry_type type;
    struct wl_list link;

    int x;
    int y;
    int width;
    int height;
};

struct tbx_config {
    struct wl_list dictionary;
    struct wl_list input;
    struct wl_list layout;
    struct wl_list keybinding;

    int workspaces;
    int swipe_threshold;
    bool animate;
    bool console;
    bool mini_titlebar;
    bool mini_frame;
    bool show_tooltip;
    float move_resize_alpha;
    bool render_damages;

    uint32_t super_key;

    struct tbx_server* server;
};

char* config_dictionary_value(struct tbx_server* server, char* name);
bool config_setup(struct tbx_server* server);
void load_config(struct tbx_server* server, char* config);

#endif // TINYBOX_CONFIG_H