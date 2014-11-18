# Makefile.am for smoothslidesaver.
#

AM_CPPFLAGS = -UQT_NO_ASCII_CAST
METASOURCES = AUTO

INCLUDES = $(all_includes)
LDFLAGS = $(all_libraries) $(KDE_RPATH)
LDADD = $(LIB_KDEUI) -lkscreensaver -lm
bin_PROGRAMS = smoothslidesaver.kss

smoothslidesaver_kss_SOURCES = smoothslidesaverui.ui smoothslidesaver.cpp \
		smoothslidewidget.cpp imageloadthread.cpp effect.cpp aboutdialog.ui screenproperties.cpp \
	debug.cpp
smoothslidesaver_kss_LDADD = -lGL $(LIB_QT) $(LIB_KDECORE) $(LDADD) $(LIB_KSYCOCA)

desktop_DATA = smoothslidesaver.desktop
desktopdir = $(kde_appsdir)/System/ScreenSavers

EXTRA_DIST = $(desktop_DATA)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/smoothslidesaver.pot
noinst_HEADERS = imageloadthread.h effect.h screenproperties.h debug.h
