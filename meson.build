project(
  'tinybox',
  'c',
  version: '0.0.2',
  license: 'MIT',
  meson_version: '>=0.43.0',
  default_options: [
    'c_std=c11',
    'warning_level=2',
    'werror=true',
  ],
)

add_project_arguments(
  [
    '-DWLR_USE_UNSTABLE',
    '-DPACKAGE_NAME="' + meson.project_name() + '"',
    '-DPACKAGE_VERSION="' + meson.project_version() + '"',
    '-Wno-unused-parameter',
    '-Wno-unused-result',
    '-Wno-missing-braces',
    '-Wundef',
    '-Wvla'
  ],
  language: 'c',
)

tinybox_inc = include_directories('include')

cc = meson.get_compiler('c')

math = cc.find_library('m', required : true)

wayland_server = dependency('wayland-server')
wayland_cursor = dependency('wayland-cursor')
wayland_client = dependency('wayland-client')
wayland_egl    = dependency('wayland-egl')
wayland_protos = dependency('wayland-protocols', version: '>=1.17')
xkbcommon      = dependency('xkbcommon')
cairo          = dependency('cairo')
pango          = dependency('pango')
libinput       = dependency('libinput')
pangocairo     = dependency('pangocairo')
glesv2         = dependency('glesv2')
egl            = dependency('egl')
xcb            = dependency('xcb')
pixman         = dependency('pixman-1')

wlroots = dependency('wlroots', version: '>=0.10.0')

#wlroots_version = ['>=0.10.0', '<0.11.0']
#wlroots_proj = subproject(
#  'wlroots',
#  default_options: ['examples=false'],
#  required: false,
#  version: wlroots_version,
#)
#if wlroots_proj.found()
#  wlroots = wlroots_proj.get_variable('wlroots')
#  wlroots_conf = wlroots_proj.get_variable('conf_data')
#  wlroots_has_xwayland = wlroots_conf.get('WLR_HAS_XWAYLAND') == 1
#else
#  wlroots = dependency('wlroots', version: wlroots_version)
#  wlroots_has_xwayland = cc.get_define('WLR_HAS_XWAYLAND', prefix: '#include <wlr/config.h>', dependencies: wlroots) == '1'
#endif

subdir('protocols')
subdir('tinybox')
