bin_PROGRAMS = gnome-robots

gnome_robots_SOURCES = \
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
	game.c \
	game.h \
	sound.c \
	sound.h \
	cursors.c \
	cursors.h \
	gbdefs.h \
	find-file.c \
	find-file.h \
	games-controls.c \
	games-controls.h \
	games-file-list.c \
	games-file-list.h \
	games-fullscreen-action.c \
	games-fullscreen-action.h \
	games-gridframe.c \
	games-gridframe.h \
	games-preimage.c \
	games-preimage.h \
	games-score.h			\
	games-score.c			\
	games-scores.c			\
	games-scores.h			\
	games-scores-dialog.c	\
	games-scores-dialog.h	\
	games-scores-backend.c	\
	games-scores-backend.h	\
	games-stock.c \
	games-stock.h \
	cursor-down.h \
	cursor-down-left.h \
	cursor-down-right.h \
	cursor-hold.h \
	cursor-left.h \
	cursor-right.h \
	cursor-up.h \
	cursor-up-left.h \
	cursor-up-right.h

if ENABLE_SETGID
gnome_robots_SOURCES +=		\
	games-setgid-io.c		\
	games-setgid-io.h
endif # ENABLE_SETGID

gnome_robots_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

gnome_robots_CFLAGS = \
	-DDATA_DIRECTORY=\"$(datadir)/gnome-robots\" \
	-DSOUND_DIRECTORY=\"$(datadir)/gnome-robots/sounds\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DICON_THEME_DIRECTORY="\"$(datadir)/icons\"" \
	-DSCORESDIR="\"$(scoredir)\""				\
	$(ROBOTS_CFLAGS)

gnome_robots_LDADD = \
	$(ROBOTS_LIBS) \
	-lm

install-exec-hook:
	-if test "$(setgid)" = "true"; then \
	  chgrp $(scores_group) $(DESTDIR)$(bindir)/gnome-robots && chmod 2555 $(DESTDIR)$(bindir)/gnome-robots ;\
	fi

-include $(top_srcdir)/git.mk
