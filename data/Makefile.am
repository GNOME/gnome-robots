SUBDIRS = icons

dist_noinst_DATA = \
	app-menu.ui

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
desktop_in_files = gnome-robots.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
$(desktop_DATA): $(desktop_in_files)
	$(AM_V_GEN)$(MSGFMT) --desktop --template $< -d $(top_srcdir)/po -o $@

appstream_in_files = gnome-robots.appdata.xml.in
appstream_XML = $(appstream_in_files:.xml.in=.xml)
$(appstream_XML): $(appstream_in_files)
	$(AM_V_GEN)$(MSGFMT) --xml --template $< -d $(top_srcdir)/po -o $@
@APPSTREAM_XML_RULES@

EXTRA_DIST = $(cursor_files) \
             $(appstream_in_files) \
             $(pixmap_DATA) \
             $(sounds_DATA) \
             $(theme_DATA) \
             $(desktop_in_files) \
             $(config_DATA) \
             $(gsettings_SCHEMAS) \
             $(man_MANS)

CLEANFILES = \
	$(appstream_XML) \
	$(desktop_DATA)

check-local: $(desktop_DATA)
	$(DESKTOP_FILE_VALIDATE) $(desktop_DATA)

DISTCHECK_CONFIGURE_FLAGS = --enable-appdata-check

-include $(top_srcdir)/git.mk
