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

void keys_add(struct tbx_keys_pressed* kp, uint32_t k);
void keys_add_named(struct tbx_keys_pressed* kp, char* name);
void keys_add_modifiers(struct tbx_keys_pressed* kp, uint32_t mod);
void keys_clear(struct tbx_keys_pressed* kp);
void keys_print(struct tbx_keys_pressed* kp);

#endif // TINYBOX_KEYBOARD_H