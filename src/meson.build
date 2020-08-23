vala_sources = files(
    'image-suffix-list.vala',
    'file-list.vala',
    'find-file.vala',
    'preimage.vala',
)

vala_lib = static_library('riiv',
    vala_sources,
    dependencies: [
        gee_dependency,
        gio_dependency,
        glib_dependency,
        gnome_games_dependency,
        gsound_dependency,
        gtk_dependency,
        gdk_dependency,
        rsvg_dependency
    ]
)

riiv_dependency = declare_dependency(
    link_with: vala_lib,
)

sources = files(
    'cursors.c',
    'game.c',
    'gameconfig.c',
    'games-controls.c',
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
        riiv_dependency,
        gee_dependency,
        gio_dependency,
        glib_dependency,
        gnome_games_dependency,
        gsound_dependency,
        gtk_dependency,
        gdk_dependency,
        rsvg_dependency
    ],
    c_args: [
        '-DHAVE_CONFIG_H',
        '-DGETTEXT_PACKAGE="@0@"'.format (meson.project_name())
    ],
    install_dir: get_option('bindir'),
    install: true
)
