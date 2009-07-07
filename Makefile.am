SUBDIRS =

if BUILD_HELP
SUBDIRS += help
endif

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_in_files = gnobots2.schemas.in
schema_DATA = $(schema_in_files:.schemas.in=.schemas)
@INTLTOOL_SCHEMAS_RULE@

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

gnobots2_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

gnobots2_CFLAGS = \
	$(GTK_CFLAGS) \
	$(AM_CFLAGS)

gnobots2_LDADD = \
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GTK_LIBS) \
	$(INTLLIBS)

if HAVE_GNOME
gnobots2_CFLAGS += $(GNOME_CFLAGS)
gnobots2_LDADD += $(GNOME_LIBS)
endif

if HAVE_RSVG
gnobots2_CFLAGS += $(RSVG_CFLAGS)
gnobots2_LDADD += $(RSVG_LIBS)
endif

if WITH_GTHREAD
gnobots2_CFLAGS += $(GHTREAD_CFLAGS)
gnobots2_LDADD += $(GTHREAD_LIBS)
endif

pixmapdir = $(pkgdatadir)/gnobots2/pixmaps
pixmap_DATA = gnomes.png \
              yahoo.png \
              aieee.png \
              splat.png \
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
             $(pixmap_DATA) \
             $(Games_DATA) \
             $(config_DATA) \
             $(schema_in_files)

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

DISTCLEANFILES = $(Games_DATA) $(schema_DATA)
