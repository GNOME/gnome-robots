SUBDIRS = icons

gsettings_SCHEMAS = org.gnome.robots.gschema.xml
@GSETTINGS_RULES@

man_MANS = gnome-robots.6

cursor_files = \
	cursor-down-left.png	\
	cursor-down.png		\
	cursor-down-right.png	\
	cursor-hold.png		\
	cursor-left.png		\
	cursor-right.png	\
	cursor-up-left.png	\
	cursor-up.png		\
	cursor-up-right.png

pixmapdir = $(datadir)/gnome-robots/pixmaps
pixmap_DATA = yahoo.png \
              aieee.png \
              splat.png

soundsdir = $(datadir)/gnome-robots/sounds
sounds_DATA = \
	die.ogg \
	splat.ogg \
	teleport.ogg \
	victory.ogg \
	yahoo.ogg

themedir = $(datadir)/gnome-robots/themes
theme_DATA = anthills.png \
             boo.svg \
             cows.png \
             eggs.png \
             gnomes.png \
             mice.png \
             robots.svg \
             spills.png \
             stormclouds.png \
             toasters.png \
             ufo.svg

configdir = $(datadir)/gnome-robots/games
config_DATA = robots2.cfg \
              robots2_easy.cfg \
              classic_robots.cfg \
              robots_with_safe_teleport.cfg \
              nightmare.cfg

desktopdir = $(datadir)/applications
desktop_in_files = gnobots2.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

appdatadir = $(datadir)/appdata
appdata_in_files = gnome-robots.appdata.xml.in
appdata_DATA = $(appdata_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST = $(cursor_files) \
	     $(appdata_in_files) \
             $(pixmap_DATA) \
             $(sounds_DATA) \
             $(theme_DATA) \
             $(desktop_in_files) \
             $(config_DATA) \
             $(gsettings_SCHEMAS) \
             $(man_MANS)

CLEANFILES = \
	$(appdata_DATA) \
	$(desktop_DATA)

check-local: $(appdata_DATA) $(desktop_DATA)
	$(APPDATA_VALIDATE) $(appdata_DATA)
	$(DESKTOP_FILE_VALIDATE) $(desktop_DATA)

-include $(top_srcdir)/git.mk
