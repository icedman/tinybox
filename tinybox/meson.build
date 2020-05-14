tinybox_src = files(
  'main.c',

  'common/cairo.c',
  'common/pango.c',
  'common/stringop.c',
  'common/util.c',

  'command/command.c',
  'command/commands/config.c',
  'command/commands/input.c',
  'command/commands/output.c',
  'command/commands/style.c',
  
  'config/config.c',
  'style/style.c',

  'input/cursor.c',
  'input/keyboard.c',
  'input/seat.c',
  'input/libinput.c',
  
  'desktop/view.c',
  'desktop/workspace.c',
  'decoration/decoration.c',

  'output/output.c',
  'output/render.c',
  'output/layout.c',
  'output/console.c',

  'server/server.c',
  'shell/xdg_shell.c'
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
  ]

executable(
  'tinybox',
  tinybox_src,
  include_directories: [tinybox_inc],
  dependencies: [tinybox_dep, server_protos],
  install: true,
  )
