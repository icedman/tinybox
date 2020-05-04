tinybox_src = files(
  'main.c'
  )

tinybox_dep = [
  wayland_server,
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
