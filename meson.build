project('gnome-robots', ['rust', 'c'],
    version: '40.0')

python3 = import('python3')
gnome = import('gnome')
i18n = import('i18n')

application_id = 'org.gnome.Robots'

add_project_arguments([
        '-include', 'config.h'
    ],
    language: 'c'
)

gee_dependency = dependency('gee-0.8')
gio_dependency = dependency('gio-2.0', version: '>= 2.36')
glib_dependency = dependency('glib-2.0', version: '>= 2.36')
gtk_dependency = dependency('gtk4', version: '>= 4.10.0')
adw_dependency = dependency('libadwaita-1', version: '>= 1.2')
rsvg_dependency = dependency('librsvg-2.0', version: '>= 2.36.2')
gstreamer_dependency = dependency('gstreamer-1.0', version: '>= 1.20')

cargo = find_program('cargo', required: true)
cargo_version = run_command(cargo, '--version', check: true).stdout().strip()
message(cargo_version)
rustc_version = run_command('rustc', '--version', check: true).stdout().strip()
message(rustc_version)

if get_option('profile') == 'debug'
    profile = 'debug'
else
    profile = 'release'
endif

bindir      = join_paths(get_option('prefix'), get_option('bindir'))
datadir     = join_paths(get_option('prefix'), get_option('datadir'))
pkgdatadir  = join_paths(datadir, meson.project_name())

config_h = configuration_data()
config_h.set_quoted('PACKAGE', meson.project_name())
config_h.set_quoted('GETTEXT_PACKAGE', meson.project_name())
config_h.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
config_h.set_quoted('VERSION', meson.project_version())
config_h.set_quoted('DATA_DIRECTORY', pkgdatadir)
config_h.set_quoted('SOUND_DIRECTORY', join_paths(pkgdatadir, 'sounds'))


subdir('data')
subdir('help')
subdir('po')
subdir('src')

meson.add_install_script(python3.find_python().path(), join_paths(meson.source_root(),
                                                                  'build-aux',
                                                                  'meson_post_install.py'))
