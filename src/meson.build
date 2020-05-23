sources = files(
    'cursors.c',
    'find-file.c',
    'game.c',
    'gameconfig.c',
    'games-controls.c',
    'games-file-list.c',
    'games-preimage.c',
    'gnome-robots.c',
    'graphics.c',
    'keyboard.c',
    'properties.c',
    'sound.c'
)
resources = gnome.compile_resources(
    'resources',
    '@0@.gresource.xml'.format(meson.project_name()),
    source_dir: '../'
)

config_header = configure_file(
    output: 'config.h',
    configuration: config_h
)

executable(
    meson.project_name(),
    sources + resources,
    config_header,
    dependencies: [
        gio_dependency,
        glib_dependency,
        gnome_games_dependency,
        gsound_dependency,
        gtk_dependency,
        rsvg_dependency
    ],
    c_args: [
        '-DHAVE_CONFIG_H',
        '-DGETTEXT_PACKAGE="@0@"'.format (meson.project_name())
    ],
    install_dir: get_option('bindir'),
    install: true
)
