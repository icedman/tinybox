tinybox_src = files(
  'main.c',
  'cursor.c',
  'seat.c',
  'output.c',
  'xdg_shell.c'
  )

tinybox_dep = [
  wayland_server,
  cairo,
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
