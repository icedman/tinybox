menu_src = files(
  'main.c',
  'menu.c',
  'render.c'
)
  
menu_dep = [
        cairo,
        client_protos,
        pango,
        pangocairo,
        egl,
        glesv2,
        wlroots,
        wayland_egl,
        wayland_protos,
        wayland_client,
        wayland_cursor
  ]

executable(
  'tinybox_menu',
  menu_src,
  include_directories: [tinybox_inc],
  dependencies: [menu_dep],
  install: true,
  )
