tinybox_src = files(
  'main.c',
  'stringop.c',
  'input/cursor.c',
  'input/keyboard.c',
  'input/seat.c',
  'output/output.c',
  'server/server.c',
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
