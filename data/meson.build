i18n.merge_file(
    input: 'xfce4-appfinder.desktop.in',
    output: 'xfce4-appfinder.desktop',
    po_dir: '..' / 'po',
    type: 'desktop',
    install: true,
    install_dir: get_option('prefix') / get_option('datadir') / 'applications',
)

i18n.merge_file(
    input: 'xfce4-run.desktop.in',
    output: 'xfce4-run.desktop',
    po_dir: '..' / 'po',
    type: 'desktop',
    install: true,
    install_dir: get_option('prefix') / get_option('datadir') / 'applications',
)

i18n.merge_file(
    input: 'org.xfce.xfce4-appfinder.appdata.xml.in',
    output: 'org.xfce.xfce4-appfinder.appdata.xml',
    po_dir: '..' / 'po',
    type: 'xml',
    install: true,
    install_dir: get_option('prefix') / get_option('datadir') / 'metainfo',
)

subdir('icons')

