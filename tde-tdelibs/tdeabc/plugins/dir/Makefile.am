INCLUDES = -I$(top_builddir)/tdeabc -I$(top_srcdir)/tdeabc -I$(top_builddir) $(all_includes)

# these are the headers for your project
noinst_HEADERS = resourcedirconfig.h

lib_LTLIBRARIES = libtdeabc_dir.la
libtdeabc_dir_la_SOURCES = resourcedir.cpp resourcedirconfig.cpp
libtdeabc_dir_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined 
libtdeabc_dir_la_LIBADD = $(LIB_TDEABC) $(LIB_TQT) $(top_builddir)/tderesources/libtderesources.la $(LIB_TDECORE) $(LIB_TDEFILE) $(LIB_TDEUI)
libtdeabc_dir_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h

kde_module_LTLIBRARIES = tdeabc_dir.la
tdeabc_dir_la_SOURCES = resourcedirplugin.cpp
tdeabc_dir_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
tdeabc_dir_la_LIBADD = libtdeabc_dir.la $(LIB_TQT) $(LIB_TDECORE)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabc_dir.pot

tdeabcincludedir = $(includedir)/tdeabc
tdeabcinclude_HEADERS = resourcedir.h

servicedir = $(kde_servicesdir)/tderesources/tdeabc
service_DATA = dir.desktop

resourcedirplugin.lo: ../../addressee.h
