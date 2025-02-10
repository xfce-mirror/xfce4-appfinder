appfinder_generated_sources = []
appfinder_generated_sources += gnome.compile_resources(
  'appfinder-preferences-ui',
  'appfinder-preferences-ui.gresource.xml',
  export: false,
  extra_args: [
    '--manual-register',
  ],
)

appfinder_sources = [
  appfinder_generated_sources,
  'appfinder-actions.c',
  'appfinder-actions.h',
  'appfinder-category-model.c',
  'appfinder-category-model.h',
  'appfinder-gdbus.c',
  'appfinder-gdbus.h',
  'appfinder-model.c',
  'appfinder-model.h',
  'appfinder-preferences.c',
  'appfinder-preferences.h',
  'appfinder-private.h',
  'appfinder-window.c',
  'appfinder-window.h',
  'main.c',
]

appfinder = executable(
  'xfce4-appfinder',
  appfinder_sources,
  sources: xfce_revision_h,
  include_directories: [
    include_directories('..'),
  ],
  dependencies: [
    glib,
    gthread,
    gio,
    gtk,
    libxfce4util,
    libxfce4ui,
    garcon,
    xfconf,
  ],
  install: true,
  install_dir: get_option('prefix') / get_option('bindir'),
)

install_symlink(
  'xfrun4',
  install_dir: get_option('prefix') / get_option('bindir'),
  pointing_to: appfinder.name()
)
