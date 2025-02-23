METASOURCES = document.moc view.moc

lib_LTLIBRARIES = libkatepartinterfaces.la

libkatepartinterfaces_la_SOURCES = interfaces.cpp katecmd.cpp
libkatepartinterfaces_la_LIBADD   = $(top_builddir)/interfaces/tdetexteditor/libtdetexteditor.la $(LIB_TQT) $(LIB_TDEUI) $(LIB_TDEPARTS) $(LIB_TDECORE)
libkatepartinterfaces_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined

kateinclude_HEADERS = document.h view.h
kateincludedir = $(includedir)/kate

INCLUDES= -I$(top_srcdir)/interfaces -I$(top_srcdir)/tdeparts -I$(top_srcdir) $(all_includes)
