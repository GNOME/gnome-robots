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
	-DSOUND_DIRECTORY=\"$(pkgdatadir)/sounds\" \
	$(GTK_CFLAGS) \
	$(CANBERRA_GTK_CFLAGS) \
	$(AM_CFLAGS)

gnobots2_LDADD = \
	$(top_builddir)/libgames-support/libgames-support.la \
	$(GTK_LIBS) \
	$(CANBERRA_GTK_LIBS) \
	$(INTLLIBS) \
	-lm

if HAVE_RSVG
gnobots2_CFLAGS += $(RSVG_CFLAGS)
gnobots2_LDADD += $(RSVG_LIBS)
endif

if WITH_GTHREAD
gnobots2_CFLAGS += $(GHTREAD_CFLAGS)
gnobots2_LDADD += $(GTHREAD_LIBS)
endif

install-exec-hook:
	-if test "$(setgid)" = "true"; then \
	  chgrp $(scores_group) $(DESTDIR)$(bindir)/gnobots2 && chmod 2555 $(DESTDIR)$(bindir)/gnobots2 ;\
	fi

-include $(top_srcdir)/git.mk
