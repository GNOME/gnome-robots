scoredir=$(localstatedir)/games
pixmapdir=$(datadir)/pixmaps/gnobots2
configdir=$(datadir)/gnobots2
soundlistdir=$(sysconfdir)/sound/events
sounddir=$(datadir)/sounds/gnobots2
appicondir=$(datadir)/pixmaps
Gamesdir=$(datadir)/gnome/apps/Games

SUBDIRS = help

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)				 \
	$(GNOME_INCLUDEDIR) $(GDK_PIXBUF_CFLAGS)

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
	gbdefs.h

gnobots2_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS)	\
	$(GDK_PIXBUF_LIBS) \
	$(INTLLIBS)

appicon_DATA = gnome-gnobots2.png

pixmap_DATA = gnomes.png \
              cows.png \
	      eggs.png \
              mice.png \
              robots.png \
              windows.png \
	      yahoo.png \
	      aieee.png \
	      splat.png

config_DATA = robots2.cfg \
              robots2_easy.cfg \
	      classic_robots.cfg \
              robots_with_safe_teleport.cfg \
              nightmare.cfg

soundlist_DATA = gnobots2.soundlist

sound_DATA = bad.wav \
	     die.wav \
             splat.wav \
             teleport.wav \
             victory.wav \
             yahoo.wav

Games_DATA = gnobots2.desktop

EXTRA_DIST = README \
             AUTHORS \
             gnobots2.desktop \
             gnobots2.soundlist \
             $(appicon_DATA) \
             $(pixmap_DATA) \
	     $(sound_DATA) \
	     $(Games_DATA) \
	     $(soundlist_DATA) \
	     $(config_DATA)

install-data-local:
	-$(mkinstalldirs) $(DESTDIR)$(scoredir)
	-for cfn in $(config_DATA); do \
	    cfn2=`echo $$cfn | sed -e 's/\.cfg//'`; \
            fname1=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2.scores; \
            fname2=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2-safe.scores; \
            fname3=$(DESTDIR)$(scoredir)/gnobots2.$$cfn2-super-safe.scores; \
	    touch $$fname1; chmod 664 $$fname1; chown games:games $$fname1; \
	    touch $$fname2; chmod 664 $$fname2; chown games:games $$fname2; \
	    touch $$fname3; chmod 664 $$fname3; chown games:games $$fname3; \
	    done
	-chgrp games $(DESTDIR)$(bindir)/gnobots2 && chmod 2111 $(DESTDIR)$(bindir)/gnobots2
