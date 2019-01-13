# Desktop file
desktop_file = i18n.merge_file(
    input: '@0@.desktop.in'.format(application_id),
    output: '@0@.desktop'.format(application_id),
    po_dir: join_paths(meson.source_root(), 'po'),
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

# AppData file
appdata_file = i18n.merge_file(
    input: '@0@.appdata.xml.in'.format(application_id),
    output: '@0@.appdata.xml'.format(application_id),
    po_dir: join_paths(meson.source_root(), 'po'),
    install: true,
    install_dir: join_paths(get_option('datadir'), 'metainfo')
)

# Validate AppData file
appstream_util = find_program('appstream-util', required: false)
if appstream_util.found()
  test(
    'validate-appdata', appstream_util,
    args: [
      'validate-relax', '--nonet', appdata_file.full_path()
    ]
  )
endif

# Install man page
install_man('@0@.6'.format(meson.project_name()))

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