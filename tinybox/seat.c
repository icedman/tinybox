#include <tinybox/server.h>

#include <wayland-server-core.h>
#include <wlr/backend.h>
#include <wlr/render/allocator.h>
#include <wlr/render/wlr_renderer.h>
#include <wlr/types/wlr_compositor.h>
#include <wlr/types/wlr_cursor.h>
#include <wlr/types/wlr_data_device.h>
#include <wlr/types/wlr_input_device.h>
#include <wlr/types/wlr_keyboard.h>
#include <wlr/types/wlr_output.h>
#include <wlr/types/wlr_output_layout.h>
#include <wlr/types/wlr_pointer.h>
#include <wlr/types/wlr_scene.h>
#include <wlr/types/wlr_seat.h>
#include <wlr/types/wlr_subcompositor.h>
#include <wlr/types/wlr_xcursor_manager.h>

#include <xkbcommon/xkbcommon.h>

#include <wlr/types/wlr_xdg_shell.h>

#include <assert.h>
#include <stdlib.h>

void
focus_view(struct tbx_view *view, struct wlr_surface *surface)
{
  /* Note: this function only deals with keyboard focus. */
  if (view == NULL) {
    return;
  }
  struct tbx_server *server = view->server;
  struct wlr_seat *seat = server->seat;
  struct wlr_surface *prev_surface = seat->keyboard_state.focused_surface;
  if (prev_surface == surface) {
    /* Don't re-focus an already focused surface. */
    return;
  }
  if (prev_surface) {
    /*
     * Deactivate the previously focused surface. This lets the client know
     * it no longer has focus and the client will repaint accordingly, e.g.
     * stop displaying a caret.
     */
    struct wlr_xdg_surface *previous = wlr_xdg_surface_try_from_wlr_surface(
        seat->keyboard_state.focused_surface);
    assert(previous != NULL && previous->role == WLR_XDG_SURFACE_ROLE_TOPLEVEL);
    wlr_xdg_toplevel_set_activated(previous->toplevel, false);
  }
  struct wlr_keyboard *keyboard = wlr_seat_get_keyboard(seat);
  /* Move the view to the front */
  wlr_scene_node_raise_to_top(&view->scene_tree->node);
  wl_list_remove(&view->link);
  wl_list_insert(&server->views, &view->link);
  /* Activate the new surface */
  wlr_xdg_toplevel_set_activated(view->xdg_toplevel, true);
  /*
   * Tell the seat to have the keyboard enter this surface. wlroots will keep
   * track of this and automatically send key events to the appropriate
   * clients without additional work on your part.
   */
  if (keyboard != NULL) {
    wlr_seat_keyboard_notify_enter(seat,
        view->xdg_toplevel->base->surface,
        keyboard->keycodes,
        keyboard->num_keycodes,
        &keyboard->modifiers);
  }
}

static void
keyboard_handle_modifiers(struct wl_listener *listener, void *data)
{
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
  wlr_seat_set_keyboard(keyboard->server->seat, keyboard->wlr_keyboard);
  /* Send modifiers to the client. */
  wlr_seat_keyboard_notify_modifiers(
      keyboard->server->seat, &keyboard->wlr_keyboard->modifiers);
}

static bool
handle_keybinding(struct tbx_server *server, xkb_keysym_t sym)
{
  /*
   * Here we handle compositor keybindings. This is when the compositor is
   * processing keys, rather than passing them on to the client for its own
   * processing.
   *
   * This function assumes Alt is held down.
   */
  switch (sym) {
    case XKB_KEY_Escape:
      wl_display_terminate(server->wl_display);
      break;
    case XKB_KEY_F1:
      /* Cycle to the next view */
      if (wl_list_length(&server->views) < 2) {
        break;
      }
      struct tbx_view *next_view =
          wl_container_of(server->views.prev, next_view, link);
      focus_view(next_view, next_view->xdg_toplevel->base->surface);
      break;
    default:
      return false;
  }
  return true;
}

static void
keyboard_handle_key(struct wl_listener *listener, void *data)
{
  /* This event is raised when a key is pressed or released. */
  struct tbx_keyboard *keyboard = wl_container_of(listener, keyboard, key);
  struct tbx_server *server = keyboard->server;
  struct wlr_keyboard_key_event *event = data;
  struct wlr_seat *seat = server->seat;

  /* Translate libinput keycode -> xkbcommon */
  uint32_t keycode = event->keycode + 8;
  /* Get a list of keysyms based on the keymap for this keyboard */
  const xkb_keysym_t *syms;
  int nsyms =
      xkb_state_key_get_syms(keyboard->wlr_keyboard->xkb_state, keycode, &syms);

  bool handled = false;
  uint32_t modifiers = wlr_keyboard_get_modifiers(keyboard->wlr_keyboard);
  if ((modifiers & WLR_MODIFIER_CTRL) &&
      event->state == WL_KEYBOARD_KEY_STATE_PRESSED) {
    /* If alt is held down and this button was _pressed_, we attempt to
     * process it as a compositor keybinding. */
    for (int i = 0; i < nsyms; i++) {
      handled = handle_keybinding(server, syms[i]);
    }
  }

  if (!handled) {
    /* Otherwise, we pass it along to the client. */
    wlr_seat_set_keyboard(seat, keyboard->wlr_keyboard);
    wlr_seat_keyboard_notify_key(
        seat, event->time_msec, event->keycode, event->state);
  }
}

static void
keyboard_handle_destroy(struct wl_listener *listener, void *data)
{
  /* This event is raised by the keyboard base wlr_input_device to signal
   * the destruction of the wlr_keyboard. It will no longer receive events
   * and should be destroyed.
   */
  struct tbx_keyboard *keyboard = wl_container_of(listener, keyboard, destroy);
  wl_list_remove(&keyboard->modifiers.link);
  wl_list_remove(&keyboard->key.link);
  wl_list_remove(&keyboard->destroy.link);
  wl_list_remove(&keyboard->link);
  free(keyboard);
}

static void
server_new_keyboard(struct tbx_server *server, struct wlr_input_device *device)
{
  struct wlr_keyboard *wlr_keyboard = wlr_keyboard_from_input_device(device);

  struct tbx_keyboard *keyboard = calloc(1, sizeof(struct tbx_keyboard));
  keyboard->server = server;
  keyboard->wlr_keyboard = wlr_keyboard;

  /* We need to prepare an XKB keymap and assign it to the keyboard. This
   * assumes the defaults (e.g. layout = "us"). */
  struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
  struct xkb_keymap *keymap =
      xkb_keymap_new_from_names(context, NULL, XKB_KEYMAP_COMPILE_NO_FLAGS);

  wlr_keyboard_set_keymap(wlr_keyboard, keymap);
  xkb_keymap_unref(keymap);
  xkb_context_unref(context);
  wlr_keyboard_set_repeat_info(wlr_keyboard, 25, 600);

  /* Here we set up listeners for keyboard events. */
  keyboard->modifiers.notify = keyboard_handle_modifiers;
  wl_signal_add(&wlr_keyboard->events.modifiers, &keyboard->modifiers);
  keyboard->key.notify = keyboard_handle_key;
  wl_signal_add(&wlr_keyboard->events.key, &keyboard->key);
  keyboard->destroy.notify = keyboard_handle_destroy;
  wl_signal_add(&device->events.destroy, &keyboard->destroy);

  wlr_seat_set_keyboard(server->seat, keyboard->wlr_keyboard);

  /* And add the keyboard to our list of keyboards */
  wl_list_insert(&server->keyboards, &keyboard->link);
}

static void
server_new_pointer(struct tbx_server *server, struct wlr_input_device *device)
{
  /* We don't do anything special with pointers. All of our pointer handling
   * is proxied through wlr_cursor. On another compositor, you might take this
   * opportunity to do libinput configuration on the device to set
   * acceleration, etc. */
  wlr_cursor_attach_input_device(server->cursor, device);
}

static void
server_new_input(struct wl_listener *listener, void *data)
{
  /* This event is raised by the backend when a new input device becomes
   * available. */
  struct tbx_server *server = wl_container_of(listener, server, new_input);
  struct wlr_input_device *device = data;
  switch (device->type) {
    case WLR_INPUT_DEVICE_KEYBOARD:
      server_new_keyboard(server, device);
      break;
    case WLR_INPUT_DEVICE_POINTER:
      server_new_pointer(server, device);
      break;
    default:
      break;
  }
  /* We need to let the wlr_seat know what our capabilities are, which is
   * communiciated to the client. In TinyWL we always have a cursor, even if
   * there are no pointer devices, so we always include that capability. */
  uint32_t caps = WL_SEAT_CAPABILITY_POINTER;
  if (!wl_list_empty(&server->keyboards)) {
    caps |= WL_SEAT_CAPABILITY_KEYBOARD;
  }
  wlr_seat_set_capabilities(server->seat, caps);
}

static void
seat_request_cursor(struct wl_listener *listener, void *data)
{
  struct tbx_server *server = wl_container_of(listener, server, request_cursor);
  /* This event is raised by the seat when a client provides a cursor image */
  struct wlr_seat_pointer_request_set_cursor_event *event = data;
  struct wlr_seat_client *focused_client =
      server->seat->pointer_state.focused_client;
  /* This can be sent by any client, so we check to make sure this one is
   * actually has pointer focus first. */
  if (focused_client == event->seat_client) {
    /* Once we've vetted the client, we can tell the cursor to use the
     * provided surface as the cursor image. It will set the hardware cursor
     * on the output that it's currently on and continue to do so as the
     * cursor moves between outputs. */
    wlr_cursor_set_surface(
        server->cursor, event->surface, event->hotspot_x, event->hotspot_y);
  }
}

static void
seat_request_set_selection(struct wl_listener *listener, void *data)
{
  /* This event is raised by the seat when a client wants to set the selection,
   * usually when the user copies something. wlroots allows compositors to
   * ignore such requests if they so choose, but in tinywl we always honor
   */
  struct tbx_server *server =
      wl_container_of(listener, server, request_set_selection);
  struct wlr_seat_request_set_selection_event *event = data;
  wlr_seat_set_selection(server->seat, event->source, event->serial);
}

struct tbx_view *
desktop_view_at(struct tbx_server *server,
    double lx,
    double ly,
    struct wlr_surface **surface,
    double *sx,
    double *sy)
{
  /* This returns the topmost node in the scene at the given layout coords.
   * we only care about surface nodes as we are specifically looking for a
   * surface in the surface tree of a tbx_view. */
  struct wlr_scene_node *node =
      wlr_scene_node_at(&server->scene->tree.node, lx, ly, sx, sy);
  if (node == NULL || node->type != WLR_SCENE_NODE_BUFFER) {
    return NULL;
  }
  struct wlr_scene_buffer *scene_buffer = wlr_scene_buffer_from_node(node);
  struct wlr_scene_surface *scene_surface =
      wlr_scene_surface_try_from_buffer(scene_buffer);
  if (!scene_surface) {
    return NULL;
  }

  *surface = scene_surface->surface;
  /* Find the node corresponding to the tbx_view at the root of this
   * surface tree, it is the only one for which we set the data field. */
  struct wlr_scene_tree *tree = node->parent;
  while (tree != NULL && tree->node.data == NULL) {
    tree = tree->node.parent;
  }
  return tree->node.data;
}

bool
tbx_seat_setup(struct tbx_server *server)
{
  /*
   * Configures a seat, which is a single "seat" at which a user sits and
   * operates the computer. This conceptually includes up to one keyboard,
   * pointer, touch, and drawing tablet device. We also rig up a listener to
   * let us know when new input devices are available on the backend.
   */
  wl_list_init(&server->keyboards);
  server->new_input.notify = server_new_input;
  wl_signal_add(&server->backend->events.new_input, &server->new_input);
  server->seat = wlr_seat_create(server->wl_display, "seat0");
  server->request_cursor.notify = seat_request_cursor;
  wl_signal_add(
      &server->seat->events.request_set_cursor, &server->request_cursor);
  server->request_set_selection.notify = seat_request_set_selection;
  wl_signal_add(&server->seat->events.request_set_selection,
      &server->request_set_selection);
  return true;
}