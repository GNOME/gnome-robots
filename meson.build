project('gnome-robots', 'c',
    version: '3.32.0')

python3 = import('python3')
gnome = import('gnome')
i18n = import('i18n')

application_id = 'org.gnome.Robots'

gtk_dependency = dependency('gtk+-3.0', version: '>= 3.15.0')
glib_dependency = dependency('glib-2.0', version: '>= 2.32')
gio_dependency = dependency('gio-2.0', version: '>= 2.32')
rsvg_dependency = dependency('librsvg-2.0', version: '>= 2.36.2')
gnome_games_dependency = dependency('libgnome-games-support-1')
canberra_gtk3_dependency = dependency('libcanberra-gtk3', version: '>= 0.26')

pkgdatadir = join_paths(get_option('prefix'), get_option('datadir'), meson.project_name())


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
