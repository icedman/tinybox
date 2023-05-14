tinybox_src = files(
  'main.c',
  'cursor.c',
  'output.c',
  'server.c',
  'view.c',
  'seat.c',
  'xdg_shell.c',
  'xwayland_shell.c',
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
  c_args: '-Wno-unused-variable'
  )
