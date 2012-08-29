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
	game.c \
	game.h \
	sound.c \
	sound.h \
	cursors.c \
	cursors.h \
	gbdefs.h \
	find-file.c \
	find-file.h \
	games-file-list.c \
	games-file-list.h \
	games-fullscreen-action.c \
	games-fullscreen-action.h \
	games-gridframe.c \
	games-gridframe.h \
	games-preimage.c \
	games-preimage.h \
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

gnobots2_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

gnobots2_CFLAGS = \
	-DDATA_DIRECTORY=\"$(datadir)/gnobots2\" \
	-DSOUND_DIRECTORY=\"$(datadir)/gnobots2/sounds\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DICON_THEME_DIRECTORY="\"$(datadir)/icons\"" \
	$(GTK_CFLAGS) \
	$(RSVG_CFLAGS) \
	$(CANBERRA_GTK_CFLAGS) \
	$(AM_CFLAGS)

gnobots2_LDADD = \
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GTK_LIBS) \
	$(RSVG_LIBS) \
	$(CANBERRA_GTK_LIBS) \
	$(INTLLIBS) \
	-lm

install-exec-hook:
	-if test "$(setgid)" = "true"; then \
	  chgrp $(scores_group) $(DESTDIR)$(bindir)/gnobots2 && chmod 2555 $(DESTDIR)$(bindir)/gnobots2 ;\
	fi

-include $(top_srcdir)/git.mk
