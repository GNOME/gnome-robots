scoredir=$(localstatedir)/games
pixmapdir=$(datadir)/pixmaps/gnobots2
configdir=$(datadir)/gnobots2
soundlistdir=$(sysconfdir)/sound/events
sounddir=$(datadir)/sounds/gnobots2
appicondir=$(datadir)/pixmaps
Gamesdir=$(datadir)/apps/Games

SUBDIRS = help

INCLUDES = 			\
	-DGNOMELOCALEDIR=\""$(datadir)/locale"\" \
	-I$(includedir)				 \
	$(GNOME_INCLUDEDIR)

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
	-$(mkinstalldirs) $(scoredir)
	-for cfn in $(config_DATA); do \
	    cfn2=`echo $$cfn | sed -e 's/\.cfg//'`; \
            fname1=$(scoredir)/gnobots2.$$cfn2.scores; \
            fname2=$(scoredir)/gnobots2.$$cfn2-safe.scores; \
	    touch echo $$fname1; chmod 664 $$fname1; chown games:games $$fname1; \
	    touch echo $$fname2; chmod 664 $$fname2; chown games:games $$fname1; \
	    done
	-chgrp games $(bindir)/gnobots2 && chmod 2111 $(bindir)/gnobots2
