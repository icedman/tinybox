tinybox_src = files(
  'main.c',

  'common/cairo.c',
  'common/pango.c',
  'common/stringop.c',
  'common/util.c',

  'command/command.c',
  'command/commands/config.c',
  'command/commands/global.c',
  'command/commands/input.c',
  'command/commands/output.c',
  'command/commands/style.c',
  'command/commands/keybindings.c',
  'command/commands/workspace.c',
  'command/commands/menu.c',
  
  'config/config.c',
  'style/style.c',

  'input/cursor.c',
  'input/keyboard.c',
  'input/seat.c',
  'input/libinput.c',
  
  'workspace/arrange.c',
  'workspace/view.c',
  'workspace/workspace.c',

  'output/output.c',
  'output/render.c',
  'output/layout.c',
  'output/console.c',
  'output/damage.c',
  
  'menu/menu.c',
  'menu/render.c',

  'server/server.c',
  'shell/decoration.c',
  'shell/xdg_shell.c',
  'shell/xwayland_shell.c'
  )

tinybox_dep = [
  wayland_server,
  cairo,
  pango,
  pangocairo,
  libinput,
  glesv2,
  wlroots,
  xkbcommon,
  math,
  xcb,
  pixman
  ]

executable(
  'tinybox',
  tinybox_src,
  include_directories: [tinybox_inc],
  dependencies: [tinybox_dep, server_protos],
  install: true,
  )
