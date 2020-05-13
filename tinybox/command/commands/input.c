#include "tinybox/command.h"
#include "tinybox/server.h"
#include "tinybox/seat.h"
#include "tinybox/libinput.h"
#include "common/stringop.h"
#include "common/util.h"

#include <stdio.h>
#include <string.h>
#include <wlr/backend/libinput.h>

/* input/libinput.c */
bool set_tap(struct libinput_device *device, enum libinput_config_tap_state tap);
bool set_natural_scroll(struct libinput_device *d, bool n);

struct libinput_device *get_libinput_device(struct tbx_command *cmd) {
  if (!cmd->context->data) {
    return 0;
  }

  struct tbx_input_device *device = (struct tbx_input_device*)cmd->context->data;
  if (!wlr_input_device_is_libinput(device->wlr_device)) {
    // console_log("not a libinput device %s", device->identifier);
    return 0;
  }

  struct libinput_device *libinput_device =
      wlr_libinput_get_device_handle(device->wlr_device);

  return libinput_device;
}

static void exec_input(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  cmd->data = 0;
  strip_quotes(argv[0]);
  
  // console_log("finding %s", argv[0]);

  struct tbx_input_device *device;
  wl_list_for_each(device, &cmd->server->seat->input_devices, link) {
    // console_log("matching with %s", device->identifier);
  
    if (device->identifier && strcmp(device->identifier, argv[0]) == 0) {
      cmd->data = (void*)device;
      // console_log("found! %s", device->identifier);
      return;
    }
  }
}

static void exec_tap(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  bool enabled = parse_boolean(argv[0], false);
  struct libinput_device *libinput_device = get_libinput_device(cmd);
  if (!libinput_device) {
    return;
  }

  set_tap(libinput_device, enabled ? LIBINPUT_CONFIG_TAP_ENABLED : LIBINPUT_CONFIG_TAP_DISABLED);
  console_log("set_tap=%d", enabled);
}

static void exec_natural_scroll(struct tbx_command *cmd, int argc, char **argv) {
  if (!command_check_args(cmd, argc, 1)) {
    return;
  }

  if (!cmd->context->data) {
    return;
  }

  bool enabled = parse_boolean(argv[0], false);
  struct libinput_device *libinput_device = get_libinput_device(cmd);
  if (!libinput_device) {
    return;
  }

  set_natural_scroll(libinput_device, enabled);
  console_log("set_natural_scroll=%d", enabled);
}

void register_input_commands(struct tbx_server* server)
{
    struct tbx_command *input = register_command(server->command, "input", exec_input);
    register_command(input, "tap", exec_tap);
    register_command(input, "natural_scroll", exec_natural_scroll);
}