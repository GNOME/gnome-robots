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
    'svg-paintable.vala',
    'image.vala',
    'animation.vala',
    'controls.vala',
    'game-config.vala',
    'sound-player.vala',
    'cursors.vala',
    'properties.vala',
    'properties-dialog.vala',
    'graphics.vala',
    'arena.vala',
    'game.vala',
    'assets.vala',
    'game-area.vala',
    'bubble.vala',
    'window-size.vala',
    'scores.vala',
    'application.vala',
    'window.vala',
)

resources = gnome.compile_resources(
    'resources',
    '@0@.gresource.xml'.format(meson.project_name()),
    source_dir: '../'
)

dependencies = [
    gee_dependency,
    gio_dependency,
    glib_dependency,
    gnome_games_dependency,
    gtk_dependency,
    rsvg_dependency,
    adw_dependency,
    gstreamer_dependency
]

executable(
    meson.project_name(),
    sources + resources + ['robots.vala'],
    config_header,
    dependencies: dependencies,
    install_dir: get_option('bindir'),
    install: true
)

test_executable = executable(
    'tests',
    sources + resources + ['test-main.vala'],
    config_header,
    dependencies: dependencies,
)

test('tests', test_executable)
