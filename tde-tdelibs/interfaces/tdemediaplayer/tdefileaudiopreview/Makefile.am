INCLUDES = -I$(top_srcdir)/interfaces/ -I$(top_srcdir)/arts/kde -I$(includedir)/arts $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = tdefileaudiopreview.la

tdefileaudiopreview_la_SOURCES = tdefileaudiopreview.cpp
tdefileaudiopreview_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -module -avoid-version -no-undefined
tdefileaudiopreview_la_LIBADD = $(top_builddir)/interfaces/tdemediaplayer/libtdemediaplayer.la $(top_builddir)/arts/kde/libartskde.la $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE)

noinst_HEADERS = tdefileaudiopreview.h

messages:
	$(XGETTEXT) $(tdefileaudiopreview_la_SOURCES) -o $(podir)/tdefileaudiopreview.pot
