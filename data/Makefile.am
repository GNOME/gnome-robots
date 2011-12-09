schema_in_files = gnobots2.schemas.in
if HAVE_GNOME
schemadir = $(GCONF_SCHEMA_FILE_DIR)
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
endif

@INTLTOOL_SCHEMAS_RULE@

man_MANS = gnobots2.6

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

pixmapdir = $(pkgdatadir)/gnobots2/pixmaps
pixmap_DATA = yahoo.png \
              aieee.png \
              splat.png

themedir = $(pkgdatadir)/gnobots2/themes
theme_DATA = gnomes.png \
             robots.svg \
             boo.svg \
             ufo.svg

configdir = $(pkgdatadir)/gnobots2/games
config_DATA = robots2.cfg \
              robots2_easy.cfg \
              classic_robots.cfg \
              robots_with_safe_teleport.cfg \
              nightmare.cfg

Gamesdir = $(datadir)/applications
Games_in_files = gnobots2.desktop.in.in
Games_DATA = $(Games_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = AUTHORS \
	     $(cursor_files) \
             $(pixmap_DATA) \
             $(theme_DATA) \
             $(Games_DATA) \
             $(config_DATA) \
             $(schema_in_files) \
             $(man_MANS)

install-data-local:
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gnobots2/data/$(schema_DATA)
endif

DISTCLEANFILES = $(Games_DATA) $(schema_DATA)

-include $(top_srcdir)/git.mk
