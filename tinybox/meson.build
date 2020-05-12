tinybox_src = files(
  'main.c',
  'stringop.c',
  'input/cursor.c',
  'input/keyboard.c',
  'input/seat.c',
  'input/libinput.c',
  'desktop/view.c',
  'decoration/decoration.c',
  'output/output.c',
  'output/render.c',
  'server/server.c',
  'shell/xdg_shell.c',
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
