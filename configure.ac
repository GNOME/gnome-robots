AC_PREREQ([2.63])
AC_INIT([GNOME Robots], [3.12.0], [https://bugzilla.gnome.org/],
        [gnome-robots], [https://wiki.gnome.org/Apps/Robots])
AM_INIT_AUTOMAKE([1.11 no-dist-gzip dist-xz foreign])
AM_SILENT_RULES([yes])
AM_MAINTAINER_MODE
GNOME_MAINTAINER_MODE_DEFINES
AC_CONFIG_HEADERS([config.h])

AM_PROG_CC_C_O

GLIB_GSETTINGS

dnl ###########################################################################
dnl Dependencies
dnl ###########################################################################

GTK_REQUIRED=3.12.0
GLIB_REQUIRED=2.28
RSVG_REQUIRED=2.32.0
CANBERRA_GTK_REQUIRED=0.26

PKG_CHECK_MODULES(ROBOTS, [
  gtk+-3.0 >= $GTK_REQUIRED
  glib-2.0 >= $GLIB_REQUIRED
  gio-2.0 >= $GLIB_REQUIRED
  librsvg-2.0 >= $RSVG_REQUIRED
  libcanberra-gtk3 >= $CANBERRA_GTK_REQUIRED
])

AC_PATH_PROG([APPDATA_VALIDATE], [appdata-validate], [/bin/true])
AC_PATH_PROG([DESKTOP_FILE_VALIDATE], [desktop-file-validate], [/bin/true])

dnl ###########################################################################
dnl Internationalization
dnl ###########################################################################

IT_PROG_INTLTOOL([0.50])
AC_SUBST(GETTEXT_PACKAGE, gnome-robots)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [The gettext package name])

dnl ###########################################################################
dnl Documentation
dnl ###########################################################################

YELP_HELP_INIT

dnl ###########################################################################
dnl Files to generate
dnl ###########################################################################

AC_CONFIG_FILES([
Makefile
po/Makefile.in
data/Makefile
data/icons/Makefile
data/icons/hicolor/Makefile
data/icons/HighContrast/Makefile
help/Makefile
src/Makefile
])
AC_OUTPUT
