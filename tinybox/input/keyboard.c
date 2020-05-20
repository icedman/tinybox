#include "tinybox/keyboard.h"
#include "tinybox/command.h"
#include "tinybox/console.h"
#include "tinybox/view.h"
#include "tinybox/workspace.h"

#include <stdio.h>
#include <stdlib.h>

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_xdg_shell.h>

#include <string.h>
#include <strings.h>
#include <xkbcommon/xkbcommon.h>

static struct modifier_key {
    char* name;
    uint32_t mod;
} modifiers[] = {
    { XKB_MOD_NAME_SHIFT, WLR_MODIFIER_SHIFT },
    { XKB_MOD_NAME_CAPS, WLR_MODIFIER_CAPS },
    { XKB_MOD_NAME_CTRL, WLR_MODIFIER_CTRL },
    { "Ctrl", WLR_MODIFIER_CTRL },
    { XKB_MOD_NAME_ALT, WLR_MODIFIER_ALT }, // Mod1
    { "Alt", WLR_MODIFIER_ALT }, // Mod2
    { XKB_MOD_NAME_NUM, WLR_MODIFIER_MOD2 },
    { "Mod3", WLR_MODIFIER_MOD3 },
    { XKB_MOD_NAME_LOGO, WLR_MODIFIER_LOGO }, // Mod4
    { "Mod5", WLR_MODIFIER_MOD5 }, // Mod5
};

static uint32_t get_modifier_mask_by_name(const char* name)
{
    int i;
    for (i = 0; i < (int)(sizeof(modifiers) / sizeof(struct modifier_key)); ++i) {
        if (strcasecmp(modifiers[i].name, name) == 0) {
            return modifiers[i].mod;
        }
    }
    return 0;
}

static bool compare_keys(struct tbx_keys_pressed* kp1, struct tbx_keys_pressed* kp2)
{
    for (int i = 0; i < KP_MAX_PRESSED - KP_RESERVED_SPACE; i++) {
        if (kp1->pressed[i] != kp2->pressed[i]) {
            return false;
        }
        if (kp2->pressed[i] == 0) {
            break;
        }
    }
    return true;
}

void keys_print(struct tbx_keys_pressed* kp)
{
    console_log("------");
    for (int i = 0; i < KP_MAX_PRESSED - KP_RESERVED_SPACE; i++) {
        char c = ' ';
        if (kp->pressed[i] < 250) {
            c = kp->pressed[i];
        }
        console_log("%d %d %c", i, kp->pressed[i], c);
        if (kp->pressed[i] == 0) {
            break;
        }
    }
}

void keys_add_named(struct tbx_keys_pressed* kp, char* name)
{
    if (strlen(name) == 1) {
        keys_add(kp, name[0]);
        return;
    }

    // if (strlen(name) >= 4 && name[1] == 'x') {
    //     name+=2;
    //     char *ptr;
    //     uint32_t uk = strtoul(name, &ptr, 16);
    //     keys_add(kp, uk);
    //     // console_log("%s %d added", name, uk);
    //     return;
    // }

    // modifier
    uint32_t m = get_modifier_mask_by_name(name);
    if (m) {
        keys_add(kp, m);
        // console_log("%s added", name);
        return;
    }

    // common name
    xkb_keysym_t keysym = xkb_keysym_from_name(name, XKB_KEYSYM_CASE_INSENSITIVE);
    if (keysym) {
        keys_add(kp, keysym);
        return;
    }

    keys_add(kp, name[0]);
}

void keys_add_modifiers(struct tbx_keys_pressed* kp, uint32_t mod)
{
    int i;
    for (i = 0; i < (int)(sizeof(modifiers) / sizeof(struct modifier_key)); ++i) {
        if ((modifiers[i].mod & mod)) {
            keys_add(kp, modifiers[i].mod);
        }
    }
}

void keys_add(struct tbx_keys_pressed* kp, uint32_t k)
{
    // modifiers (todo!)
    if (k == 65515 || k == 65513 || k == 65514 || k == 65505 || k == 65507 || k == 65508 || k == 65511) {
        return;
    }

    for (int i = 0; i < KP_MAX_PRESSED - KP_RESERVED_SPACE; i++) {
        if (kp->pressed[i] == 0) {
            kp->pressed[i] = k;
            return;
        }
        if (kp->pressed[i] == k) {
            return;
        }
        if (kp->pressed[i] > k) {
            // push
            memmove(&kp->pressed[i + 1], &kp->pressed[i],
                sizeof(char) * (KP_MAX_PRESSED - KP_RESERVED_SPACE - i + 1));
            kp->pressed[i] = k;
            return;
        }
    }
}

void keys_clear(struct tbx_keys_pressed* kp)
{
    memset(kp, 0, sizeof(struct tbx_keys_pressed));
}

static bool handle_keybinding(struct tbx_server* server,
    struct tbx_keys_pressed* keys)
{

    struct tbx_keys_pressed k = {
        .pressed = { WLR_MODIFIER_ALT, XKB_KEY_Escape, 0 }
    };
    if (compare_keys(keys, &k)) {
        wl_display_terminate(server->wl_display);
        return true;
    }

    struct tbx_config_keybinding* entry;
    wl_list_for_each(entry, &server->config.keybinding, link)
    {
        if (compare_keys(entry->keys, keys)) {

            struct tbx_command* ctx = server->command;
            command_execute(ctx, entry->argc, entry->argv);
            // console_log("key binding found! %s", entry->identifier);
            return true;
        }
    }

    return false;
}

static void keyboard_handle_modifiers(struct wl_listener* listener,
    void* data)
{
    /* This event is raised when a modifier key, such as shift or alt, is
   * pressed. We simply communicate this to the client. */
    struct tbx_keyboard* keyboard = wl_container_of(listener, keyboard, modifiers);
    /*
   * A seat can only have one keyboard, but this is a limitation of the
   * Wayland protocol - not wlroots. We assign all connected keyboards to the
   * same seat. You can swap out the underlying wlr_keyboard like this and
   * wlr_seat handles this transparently.
   */
    wlr_seat_set_keyboard(keyboard->server->seat->seat, keyboard->device);
    /* Send modifiers to the client. */
    wlr_seat_keyboard_notify_modifiers(keyboard->server->seat->seat,
        &keyboard->device->keyboard->modifiers);
}

static void keyboard_handle_key(struct wl_listener* listener, void* data)
{
    /* This event is raised when a key is pressed or released. */
    struct tbx_keyboard* keyboard = wl_container_of(listener, keyboard, key);
    struct tbx_server* server = keyboard->server;
    struct wlr_event_keyboard_key* event = data;
    struct wlr_seat* seat = server->seat->seat;

    if (server->menu_navigation_grab) {
        // handle menu navigation
        return;
    }

    // uint32_t super_key = server->config.super_key;

    server->seat->last_keyboard = keyboard;
    struct tbx_keys_pressed* kp = server->seat->keys_pressed;

    /* Translate libinput keycode -> xkbcommon */
    uint32_t keycode = event->keycode + 8;
    /* Get a list of keysyms based on the keymap for this keyboard */

    const xkb_keysym_t* syms;
    // int nsyms = xkb_state_key_get_syms(keyboard->device->keyboard->xkb_state,
    //                                    keycode, &syms);
    xkb_layout_index_t layout_index = xkb_state_key_get_layout(keyboard->device->keyboard->xkb_state, keycode);

    int nsyms = xkb_keymap_key_get_syms_by_level(
        keyboard->device->keyboard->keymap, keycode, layout_index, 0, &syms);

    bool handled = false;
    uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);

    if (!modifiers && event->state == WLR_KEY_PRESSED && kp->pressed[1]) {
        keys_clear(kp);
    }

    if (modifiers && event->state == WLR_KEY_PRESSED) {
        keys_add_modifiers(kp, modifiers);
        for (int i = 0; i < nsyms; i++) {
            uint32_t k = syms[i];
            keys_add(kp, k);
        }
        if (kp->pressed[1]) {
            handled = handle_keybinding(server, kp);
            if (handled) {
                keys_clear(kp);
            }
        }
    }

    if (!handled && nsyms == 1 && event->state == WLR_KEY_PRESSED) {
        struct tbx_keys_pressed k = {
            .pressed = { syms[0], 0 }
        };
        handle_keybinding(server, &k);
    }

    if (!handled) {

        /* Otherwise, we pass it along to the client. */
        wlr_seat_set_keyboard(seat, keyboard->device);
        wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode,
            event->state);
    }
}

void keyboard_attach(struct tbx_server* server,
    struct wlr_input_device* device)
{
    struct tbx_keyboard* keyboard = calloc(1, sizeof(struct tbx_keyboard));
    keyboard->server = server;
    keyboard->device = device;

    /* We need to prepare an XKB keymap and assign it to the keyboard. This
   * assumes the defaults (e.g. layout = "us"). */
    struct xkb_rule_names rules = { 0 };
    struct xkb_context* context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    struct xkb_keymap* keymap = xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);

    wlr_keyboard_set_keymap(device->keyboard, keymap);
    xkb_keymap_unref(keymap);
    xkb_context_unref(context);
    wlr_keyboard_set_repeat_info(device->keyboard, 25, 600);

    /* Here we set up listeners for keyboard events. */
    keyboard->modifiers.notify = keyboard_handle_modifiers;
    wl_signal_add(&device->keyboard->events.modifiers, &keyboard->modifiers);
    keyboard->key.notify = keyboard_handle_key;
    wl_signal_add(&device->keyboard->events.key, &keyboard->key);

    wlr_seat_set_keyboard(server->seat->seat, device);

    /* And add the keyboard to our list of keyboards */
    wl_list_insert(&server->seat->keyboards, &keyboard->link);
}