#include "tinybox/keyboard.h"
#include "tinybox/console.h"

#include <stdio.h>
#include <stdlib.h>

#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_seat.h>

#include <xkbcommon/xkbcommon.h>

static bool handle_keybinding(struct tbx_server *server, xkb_keysym_t sym,
                              uint32_t modifiers) {

  switch (sym) {
  case XKB_KEY_z:
    console_dump();
    break;

  case XKB_KEY_Escape:
    wl_display_terminate(server->wl_display);
    break;

  default:
    return false;
  }

  return true;
}

static void keyboard_handle_modifiers(struct wl_listener *listener,
                                      void *data) {
  /* This event is raised when a modifier key, such as shift or alt, is
   * pressed. We simply communicate this to the client. */
  struct tbx_keyboard *keyboard =
      wl_container_of(listener, keyboard, modifiers);
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

static void keyboard_handle_key(struct wl_listener *listener, void *data) {
  /* This event is raised when a key is pressed or released. */
  struct tbx_keyboard *keyboard = wl_container_of(listener, keyboard, key);
  struct tbx_server *server = keyboard->server;
  struct wlr_event_keyboard_key *event = data;
  struct wlr_seat *seat = server->seat->seat;

  server->seat->last_keyboard = keyboard;

  /* Translate libinput keycode -> xkbcommon */
  uint32_t keycode = event->keycode + 8;
  /* Get a list of keysyms based on the keymap for this keyboard */
  const xkb_keysym_t *syms;
  int nsyms = xkb_state_key_get_syms(keyboard->device->keyboard->xkb_state,
                                     keycode, &syms);

  bool handled = false;
  uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->device->keyboard);

  if ((modifiers & WLR_MODIFIER_ALT) && event->state == WLR_KEY_PRESSED) {
    /* If alt is held down and this button was _pressed_, we attempt to
     * process it as a compositor keybinding. */
    for (int i = 0; i < nsyms; i++) {
      handled = handle_keybinding(server, syms[i], modifiers);
    }
  }

  if (!handled) {
    /* Otherwise, we pass it along to the client. */
    wlr_seat_set_keyboard(seat, keyboard->device);
    wlr_seat_keyboard_notify_key(seat, event->time_msec, event->keycode,
                                 event->state);
  }
}

void keyboard_attach(struct tbx_server *server,
                     struct wlr_input_device *device) {
  struct tbx_keyboard *keyboard = calloc(1, sizeof(struct tbx_keyboard));
  keyboard->server = server;
  keyboard->device = device;

  /* We need to prepare an XKB keymap and assign it to the keyboard. This
   * assumes the defaults (e.g. layout = "us"). */
  struct xkb_rule_names rules = {0};
  struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  struct xkb_keymap *keymap =
      xkb_map_new_from_names(context, &rules, XKB_KEYMAP_COMPILE_NO_FLAGS);

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