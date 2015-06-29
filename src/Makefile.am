bin_PROGRAMS = gnome-robots

gnome_robots_SOURCES = \
	gnome-robots.c \
	gnome-robots.h \
	gameconfig.c \
	gameconfig.h \
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
	games-preimage.c \
	games-preimage.h \
	cursor-down.h \
	cursor-down-left.h \
	cursor-down-right.h \
	cursor-hold.h \
	cursor-left.h \
	cursor-right.h \
	cursor-up.h \
	cursor-up-left.h \
	cursor-up-right.h \
	$(BUILT_SOURCES)

BUILT_SOURCES = robots-resources.c

robots-resources.c: $(shell $(GLIB_COMPILE_RESOURCES) --generate-dependencies robots.gresource.xml)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) --target=$@ --sourcedir=$(srcdir) --generate-source robots.gresource.xml

gnome_robots_CPPFLAGS = \
	-I$(top_srcdir) \
	$(AM_CPPFLAGS)

gnome_robots_CFLAGS = \
	-DDATA_DIRECTORY=\"$(datadir)/gnome-robots\" \
	-DSOUND_DIRECTORY=\"$(datadir)/gnome-robots/sounds\" \
	-DLOCALEDIR=\"$(datadir)/locale\" \
	-DICON_THEME_DIRECTORY="\"$(datadir)/icons\"" \
	$(ROBOTS_CFLAGS)

gnome_robots_LDADD = \
	$(ROBOTS_LIBS) \
	-lm

MAINTAINERCLEANFILES = robots-resources.c

EXTRA_DIST = robots.gresource.xml

-include $(top_srcdir)/git.mk
