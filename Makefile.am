
schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = gnobots2.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

pixmapdir=$(datadir)/pixmaps/gnobots2
configdir=$(datadir)/gnobots2
soundlistdir=$(sysconfdir)/sound/events
sounddir=$(datadir)/sounds/gnobots2
Gamesdir=$(datadir)/applications

SUBDIRS = help

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-DDATADIR=\""$(datadir)"\"		   \
	-DPIXMAPDIR=\""$(pixmapdir)"\"		\
	-I$(top_srcdir)/libgames-support   	        \
	$(GNOME_GAMES_CFLAGS)

bin_PROGRAMS = gnobots2

gnobots2_SOURCES = \
	gnobots.c \
	gnobots.h \
	statusbar.c \
	statusbar.h \
	gameconfig.c \
	gameconfig.h \
	menu.c \
	menu.h \
	properties.c \
	properties.h \
	graphics.c \
	graphics.h \
	keyboard.c \
	keyboard.h \
	keylabels.h \
	game.c \
	game.h \
	sound.c \
	sound.h \
	cursors.c \
	cursors.h \
	gbdefs.h

gnobots2_LDADD = \
	../libgames-support/libgames-support.la \
	$(GNOME_GAMES_LIBS) \
	$(INTLLIBS)

pixmap_DATA = gnomes.png \
	      yahoo.png \
	      aieee.png \
	      splat.png \
	      robots.svg \
	      boo.svg \
              ufo.svg

config_DATA = robots2.cfg \
              robots2_easy.cfg \
	      classic_robots.cfg \
              robots_with_safe_teleport.cfg \
              nightmare.cfg

soundlist_in_files = gnobots2.soundlist.in
soundlist_DATA = $(soundlist_in_files:.soundlist.in=.soundlist)
@INTLTOOL_SOUNDLIST_RULE@

sound_DATA = bad.wav \
	     die.wav \
             splat.wav \
             teleport.wav \
             victory.wav \
             yahoo.wav

Games_in_files = gnobots2.desktop.in
Games_DATA = $(Games_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

EXTRA_DIST = README \
             AUTHORS \
             gnobots2.desktop.in \
             gnobots2.soundlist.in \
	     $(icon_DATA) \
	     $(svgicon_DATA) \
             $(pixmap_DATA) \
	     $(sound_DATA) \
	     $(Games_DATA) \
	     $(soundlist_DATA) \
	     $(config_DATA) \
             $(schema_in_files)

icondir = $(datadir)/icons/hicolor/48x48/apps
icon_DATA = gnome-robots.png
svgicondir = $(datadir)/icons/hicolor/scalable/apps
svgicon_DATA = gnome-robots.svg

install-data-local:
if GCONF_SCHEMAS_INSTALL
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(top_builddir)/gnobots2/$(schema_DATA)
endif
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-for cfn in $(config_DATA); do \
	    cfn2=`echo $$cfn | sed -e 's/\.cfg//'`; \
            fname1=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2.scores; \
            fname2=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2-safe.scores; \
            fname3=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2-super-safe.scores; \
	    touch $$fname1; chmod 664 $$fname1; chown $(scores_user):$(scores_group) $$fname1; \
	    touch $$fname2; chmod 664 $$fname2; chown $(scores_user):$(scores_group) $$fname2; \
	    touch $$fname3; chmod 664 $$fname3; chown $(scores_user):$(scores_group) $$fname3; \
	    done
	-if test "x$(setgid)" = "xtrue"; then chgrp $(scores_group) $(DESTDIR)$(bindir)/gnobots2 && chmod 2555 $(DESTDIR)$(bindir)/gnobots2 ; fi

DISTCLEANFILES = $(Games_DATA) $(soundlist_DATA) $(schema_DATA)
