tinybox_src = files(
  'main.c',
  
  'desktop/console.c',
  'desktop/output.c',
  'desktop/seat.c',
  'input/cursor.c',
  'input/keyboard.c',
  
  'cairo.c',
  'server.c',
  'stringop.c',
  'style.c',
  'util.c',
  'pango.c',
  'xdg_shell.c'
  )

tinybox_dep = [
  wayland_server,
  cairo,
  pango,
  pangocairo,
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
