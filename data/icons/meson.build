theme_dir = join_paths(get_option('prefix'), get_option('datadir'), 'icons', 'hicolor')
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
