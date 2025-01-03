project('xfce4-appfinder', 'c',
  version: '4.18.0',
  meson_version: '>= 0.61.0',
  license: 'GPLv2'
)

GNOME = import('gnome')
I18N = import('i18n')

if get_option('development')
  app_id = 'org.xfce.Appfinder.Devel'
  name_prefix = '(Nightly) '
  vcs_tag = run_command('git', 'rev-parse', '--short', 'HEAD').stdout().strip()
  version_suffix = '-@0@'.format (vcs_tag)
else
  app_id = 'org.xfce.Appfinder'
  name_prefix = ''
  version_suffix = ''
endif

# Some variables
libexec_dir = join_paths(get_option('prefix'), get_option('libexecdir'))
locale_dir = join_paths(get_option('prefix'), get_option('localedir'))
po_dir = join_paths(meson.project_source_root(), 'po')

# Dependencies
glib_min_version = '2.66.0'
xfce_min_version = '4.18.0'

glib = dependency('glib-2.0', version: '>= ' + glib_min_version)
gthread = dependency('gthread-2.0', version: '>= ' + glib_min_version)
gio = dependency('gio-2.0', version: '>= ' + glib_min_version)
gtk = dependency('gtk+-3.0', version: '>= 3.24.0')
libxfce4ui = dependency('libxfce4ui-2', version: '>= ' + xfce_min_version)
libxfce4util = dependency('libxfce4util-1.0', version: '>= ' + xfce_min_version)
garcon = dependency('garcon-1', version: '>= ' + xfce_min_version)
libxfconf = dependency('libxfconf-0', version: '>= ' + xfce_min_version)

# Configuration
conf = configuration_data()

conf.set_quoted('APP_ID', app_id)
conf.set_quoted('NAME_PREFIX', name_prefix)
conf.set_quoted('GETTEXT_PACKAGE', meson.project_name())
conf.set_quoted('LOCALE_DIR', locale_dir)
conf.set_quoted('VERSION', meson.project_version() + version_suffix)
conf.set10('DEVELOPMENT_BUILD', get_option('development'))

# Autotools compability
conf.set_quoted('PACKAGE_NAME', app_id)
conf.set_quoted('PACKAGE_LOCALE_DIR', locale_dir)
conf.set_quoted('PACKAGE_VERSION', meson.project_version() + version_suffix)
conf.set_quoted('PACKAGE_BUGREPORT', 'https://gitlab.xfce.org/xfce/xfce4-appfinder/')
add_global_arguments('-DHAVE_CONFIG_H=1', language: 'c')

configure_file(output: 'config.h', configuration: conf)
include_dir = include_directories('.')

# Subdirs
subdir('data')
subdir('po')
subdir('src')

GNOME.post_install(
  gtk_update_icon_cache: true,
)

summary('Development', get_option('development'), section: 'Development Build')
