add_project_arguments(
    ['--vapidir', meson.current_source_dir()],
    language: 'vala',
)

config_header = configure_file(
    output: 'config.h',
    configuration: config_h
)

sources = files(
    'config.vapi',

    'image-suffix-list.vala',
    'themes.vala',
    'theme.vala',
    'preimage.vala',
    'controls.vala',
    'game-config.vala',
    'sound.vala',
    'cursors.vala',
    'properties.vala',
    'graphics.vala',
    'arena.vala',
    'game.vala',
    'game-area.vala',
    'robots.vala',
)

resources = gnome.compile_resources(
    'resources',
    '@0@.gresource.xml'.format(meson.project_name()),
    source_dir: '../'
)

executable(
    meson.project_name(),
    sources + resources,
    config_header,
    dependencies: [
        gee_dependency,
        gio_dependency,
        glib_dependency,
        gnome_games_dependency,
        gsound_dependency,
        gtk_dependency,
        gdk_dependency,
        rsvg_dependency
    ],
    install_dir: get_option('bindir'),
    install: true
)
