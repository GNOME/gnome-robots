sizes = ['16x16', '22x22', '24x24', '32x32', '48x48']
theme_dir = join_paths(get_option('prefix'), get_option('datadir'), 'icons', 'hicolor')
foreach size: sizes
    install_data(
        join_paths('hicolor', size, '@0@.png'.format(application_id)),
        install_dir: join_paths(theme_dir, size, 'apps')
    )
endforeach

install_data(
    [
        'hicolor/teleport-random.png',
        'hicolor/teleport.png'
    ],
    install_dir: join_paths(theme_dir, '24x24', 'actions')
)
install_data(
    'hicolor/scalable/@0@-symbolic.svg'.format(application_id),
    install_dir: join_paths(theme_dir, 'symbolic', 'apps')
)
install_data(
    'hicolor/scalable/@0@.svg'.format(application_id),
    install_dir: join_paths(theme_dir, 'scalable', 'apps')
)