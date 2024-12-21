# Desktop file
desktop_file = i18n.merge_file(
    input: '@0@.desktop.in'.format(application_id),
    output: '@0@.desktop'.format(application_id),
    po_dir: join_paths(meson.project_source_root(), 'po'),
    type: 'desktop',
    install: true,
    install_dir: join_paths(get_option('datadir'), 'applications')
)

# Validate desktop file
desktop_file_validate = find_program('desktop-file-validate', required: false)
if desktop_file_validate.found()
  test(
    'validate-desktop',
    desktop_file_validate,
    args: [
      desktop_file.full_path()
    ]
  )
endif

# MetaInfo file
metainfo_file = i18n.merge_file(
    input: '@0@.metainfo.xml.in'.format(application_id),
    output: '@0@.metainfo.xml'.format(application_id),
    po_dir: join_paths(meson.project_source_root(), 'po'),
    install: true,
    install_dir: join_paths(get_option('datadir'), 'metainfo')
)

# Validate MetaInfo file
appstreamcli = find_program('appstreamcli', required: false)
if appstreamcli.found()
  test(
    'validate-metainfo', appstreamcli,
    args: [
      'validate', '--no-net', '--explain', metainfo_file.full_path()
    ]
  )
endif

# Install man page
install_man('@0@.6'.format(meson.project_name()))

# Make D-Bus activatable
service_conf = configuration_data()
service_conf.set('bindir', bindir)

service = 'org.gnome.Robots.service'

configure_file(
  input: service + '.in',
  output: service,
  install: true,
  install_dir: join_paths(datadir, 'dbus-1', 'services'),
  configuration: service_conf
)

# Install GSchema file
install_data(
  '@0@.gschema.xml'.format(application_id),
  install_dir : join_paths(get_option('datadir'), 'glib-2.0', 'schemas')
)

install_data(
    [
        'games/classic_robots.cfg',
        'games/nightmare.cfg',
        'games/robots2.cfg',
        'games/robots2_easy.cfg',
        'games/robots_with_safe_teleport.cfg'
    ],
    install_dir: join_paths(pkgdatadir, 'games') 
)

install_data(
    [
	'cursors/cursor-up-left.png',
	'cursors/cursor-up.png',
	'cursors/cursor-up-right.png',
	'cursors/cursor-left.png',
	'cursors/cursor-hold.png',
	'cursors/cursor-right.png',
	'cursors/cursor-down-left.png',
	'cursors/cursor-down.png',
	'cursors/cursor-down-right.png'
    ],
    install_dir: join_paths(pkgdatadir, 'cursors')
)

install_data(
    [
        'pixmaps/aieee.png',
        'pixmaps/splat.png',
        'pixmaps/yahoo.png'
    ],
    install_dir: join_paths(pkgdatadir, 'pixmaps')
)

install_data(
    [
        'sounds/die.ogg',
        'sounds/splat.ogg',
        'sounds/teleport.ogg',
        'sounds/victory.ogg',
        'sounds/yahoo.ogg'
    ],
    install_dir: join_paths(pkgdatadir, 'sounds')
)

install_data(
    [
        'themes/anthills.png',
        'themes/boo.svg',
        'themes/cows.png',
        'themes/eggs.png',
        'themes/gnomes.png',
        'themes/mice.png',
        'themes/robots.svg',
        'themes/spills.png',
        'themes/stormclouds.png',
        'themes/toasters.png',
        'themes/ufo.svg'
    ],
    install_dir: join_paths(pkgdatadir, 'themes')
)
subdir('icons')
