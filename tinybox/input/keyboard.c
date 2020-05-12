#include "tinybox/server.h"
#include "tinybox/keyboard.h"

#include <stdio.h>
#include <stdlib.h>

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>

static void keyboard_handle_modifiers(struct wl_listener *listener, void *data) {
    printf("modifiers\n");
}

static void keyboard_handle_key(struct wl_listener *listener, void *data) {
    printf("key\n");
}

void keyboard_attach(struct tbx_server *server, struct wlr_input_device *device)
{
  struct tbx_keyboard *keyboard = calloc(1, sizeof(struct tbx_keyboard));
  keyboard->server = server;
  keyboard->device = device;

  /* We need to prepare an XKB keymap and assign it to the keyboard. This
   * assumes the defaults (e.g. layout = "us"). */
  struct xkb_rule_names rules = { 0 };
  struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  struct xkb_keymap *keymap = xkb_map_new_from_names(context, &rules,
    XKB_KEYMAP_COMPILE_NO_FLAGS);

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