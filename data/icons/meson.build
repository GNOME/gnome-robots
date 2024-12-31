theme_dir = join_paths(datadir, 'icons', 'hicolor')
pkg_icon_dir = join_paths(pkgdatadir, 'icons', 'hicolor')

install_data(
    [
        'hicolor/teleport-random.png',
        'hicolor/teleport.png'
    ],
    install_dir: join_paths(theme_dir, '24x24', 'actions')
)
install_data(
    'hicolor/symbolic/@0@-symbolic.svg'.format(application_id),
    install_dir: join_paths(theme_dir, 'symbolic', 'apps')
)
install_data(
    'hicolor/scalable/@0@.svg'.format(application_id),
    install_dir: join_paths(theme_dir, 'scalable', 'apps')
)

install_data(
    [
    	'hicolor/symbolic/gamepad2-symbolic.svg',
    	'hicolor/symbolic/brush-monitor-symbolic.svg',
        'hicolor/symbolic/keyboard-shortcuts-symbolic.svg',
    ],
    install_dir: join_paths(pkg_icon_dir, 'scalable', 'categories')
)
meson.add_install_script('gtk4-update-icon-cache', '-qtf', pkg_icon_dir, skip_if_destdir: true)
