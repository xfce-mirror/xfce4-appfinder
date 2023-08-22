icon_name = app_id.to_lower()

# Icons
install_data(
  'icons' / 'scalable' / '@0@.svg'.format(icon_name),
  install_dir: get_option('datadir') / 'icons' / 'hicolor' / 'scalable' / 'apps'
)

foreach size : ['16x16', '24x24', '32x32', '48x48', '128x128']
  install_data(
    'icons' / size / '@0@.png'.format(icon_name),
    install_dir: get_option('datadir') / 'icons' / 'hicolor' / size / 'apps'
  )
endforeach

# Generic config
# Not needed, we are not using value replacement
conf = configuration_data()
conf.set('app_id', app_id)

# xfce4-appfinder desktop file
desktop_file = I18N.merge_file(
  type: 'desktop',
  input: configure_file(
    input: 'xfce4-appfinder.desktop.in',
    output: 'xfce4-appfinder.desktop.in2',
    configuration: conf,
  ),
  output: 'xfce4-appfinder.desktop',
  po_dir: po_dir,
  install: true,
  install_dir: get_option('datadir') / 'applications'
)

# xfce4-run desktop file
desktop_file = I18N.merge_file(
  type: 'desktop',
  input: configure_file(
    input: 'xfce4-run.desktop.in',
    output: 'xfce4-run.desktop.in2',
    configuration: conf,
  ),
  output: 'xfce4-run.desktop',
  po_dir: po_dir,
  install: true,
  install_dir: get_option('datadir') / 'applications'
)

# Appdata
appstream_file = I18N.merge_file(
  input: configure_file(
    input: 'org.xfce.xfce4-appfinder.appdata.xml.in',
    output: '@0@.appdata.xml.in'.format(app_id),
    configuration: conf,
  ),
  output: 'org.xfce.xfce4-appfinder.appdata.xml',
  po_dir: po_dir,
  install: true,
  install_dir: get_option('datadir') / 'metainfo'
)
