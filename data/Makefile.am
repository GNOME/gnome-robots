SUBDIRS = icons

gsettings_in_file = org.gnome.gnobots2.gschema.xml.in
gsettings_SCHEMAS = $(gsettings_in_file:.xml.in=.xml)
@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@

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

pixmapdir = $(datadir)/gnobots2/pixmaps
pixmap_DATA = yahoo.png \
              aieee.png \
              splat.png

soundsdir = $(datadir)/gnobots2/sounds
sounds_DATA = \
	die.ogg \
	splat.ogg \
	teleport.ogg \
	victory.ogg \
	yahoo.ogg

themedir = $(datadir)/gnobots2/themes
theme_DATA = gnomes.png \
             robots.svg \
             boo.svg \
             ufo.svg

configdir = $(datadir)/gnobots2/games
config_DATA = robots2.cfg \
              robots2_easy.cfg \
              classic_robots.cfg \
              robots_with_safe_teleport.cfg \
              nightmare.cfg

Gamesdir = $(datadir)/applications
Games_in_files = gnobots2.desktop.in.in
Games_DATA = $(Games_in_files:.desktop.in.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

install-data-local:
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

EXTRA_DIST = $(cursor_files) \
             $(pixmap_DATA) \
             $(sounds_DATA) \
             $(theme_DATA) \
             $(Games_DATA) \
             $(config_DATA) \
             $(gsettings_in_file) \
             $(man_MANS)

CLEANFILES = $(Games_DATA) $(gsettings_SCHEMAS)
DISTCLEANFILES = $(Games_DATA) $(gsettings_SCHEMAS)

-include $(top_srcdir)/git.mk
