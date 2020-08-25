add_project_arguments(['--vapidir', meson.current_source_dir()], language: 'vala')


config_header = configure_file(
    output: 'config.h',
    configuration: config_h
)

vala_sources = files(
    'config.vapi',
    'main.vapi',

    'image-suffix-list.vala',
    'file-list.vala',
    'find-file.vala',
    'preimage.vala',
    'controls.vala',
    'game-config.vala',
    'sound.vala',
    'cursors.vala',
    'properties.vala',
    'graphics.vala',
)

vala_lib = static_library('riiv',
    config_header,
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
    'game.c',
    'gnome-robots.c',
    'keyboard.c',
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
