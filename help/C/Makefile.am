## Process this file with automake to produce Makefile.in.

EXTRA_DIST = index.html \
	     topic.dat

gnobots2_helpdir = $(datadir)/gnome/help/gnobots2/C

install-data-local:
	$(mkinstalldirs) $(gnobots2_helpdir)
	$(INSTALL_DATA)  $(srcdir)/index.html $(gnobots2_helpdir)
	$(INSTALL_DATA)  $(srcdir)/topic.dat $(gnobots2_helpdir)

