#ifndef TINYBOX_KEYBOARD_H
#define TINYBOX_KEYBOARD_H

#include "tinybox/server.h"

struct tbx_server;
struct wlr_input_device;

#define KP_MAX_PRESSED 8
#define KP_RESERVED_SPACE 2

struct tbx_keys_pressed {
    uint32_t pressed[KP_MAX_PRESSED];
};

struct tbx_keyboard {
    struct wl_list link;
    struct wlr_input_device* device;

    struct wl_listener modifiers;
    struct wl_listener key;

    struct tbx_server* server;
};

void keyboard_attach(struct tbx_server* server,
    struct wlr_input_device* device);

void add_key_by_name(struct tbx_keys_pressed* kp, char* name);
void add_key(struct tbx_keys_pressed* kp, uint32_t k);
void add_modifiers(struct tbx_keys_pressed* kp, uint32_t mod);
void clear_keys(struct tbx_keys_pressed* kp);

void dump_keys(struct tbx_keys_pressed* kp);

uint32_t get_modifier_mask_by_name(const char* name);
bool is_modifier(uint32_t modifier);

#endif // TINYBOX_KEYBOARD_H