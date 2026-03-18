project('gnome-robots', ['rust'],
    version: '50.0',
    meson_version: '>= 0.59')

gnome = import('gnome')
i18n = import('i18n')

application_id = 'org.gnome.Robots'

gio_dependency = dependency('gio-2.0', version: '>= 2.86')
glib_dependency = dependency('glib-2.0', version: '>= 2.86')
gtk_dependency = dependency('gtk4', version: '>= 4.20.0')
adw_dependency = dependency('libadwaita-1', version: '>= 1.8')
glycin_dependency = dependency('glycin-2', version: '>= 2.0')
glycin_gtk4_dependency = dependency('glycin-gtk4-2', version: '>= 2.0')

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

config_rs = configuration_data()
config_rs.set_quoted('PACKAGE', meson.project_name())
config_rs.set_quoted('GETTEXT_PACKAGE', meson.project_name())
config_rs.set_quoted('LOCALEDIR', join_paths(get_option('prefix'), get_option('localedir')))
config_rs.set_quoted('VERSION', meson.project_version())
config_rs.set_quoted('DATA_DIRECTORY', pkgdatadir)
config_rs.set_quoted('SOUND_DIRECTORY', join_paths(pkgdatadir, 'sounds'))


subdir('data')
subdir('help')
subdir('po')
subdir('src')

gnome.post_install(
    gtk_update_icon_cache: true,
    glib_compile_schemas: true,
    update_desktop_database: true)
