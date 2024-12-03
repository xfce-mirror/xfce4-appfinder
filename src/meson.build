app_sources = [
  'appfinder-actions.c',
  'appfinder-category-model.c',
  'appfinder-gdbus.c',
  'appfinder-model.c',
  'appfinder-preferences.c',
  'appfinder-window.c',
  'main.c',
]

app_sources += GNOME.compile_resources(
  'appfinder-resources',
  'appfinder.gresource.xml'
)

app_deps = [
  glib,
  gthread,
  gio,
  gtk,
  libxfce4ui,
  libxfce4util,
  garcon,
  libxfconf
]

executable(
  'xfce4-appfinder',
  app_sources,
  dependencies: app_deps,
  include_directories: include_dir,
  install: true
)

install_symlink(
  'xfrun4',
  pointing_to: './xfce4-appfinder',
  install_dir: join_paths(get_option('prefix'), get_option('bindir'))
)
