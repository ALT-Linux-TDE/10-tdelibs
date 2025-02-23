INCLUDES = -I$(top_builddir)/tdeabc -I$(top_srcdir)/tdeabc $(all_includes)

# these are the headers for your project
noinst_HEADERS   = resourcefileconfig.h

lib_LTLIBRARIES = libtdeabc_file.la
libtdeabc_file_la_SOURCES = resourcefile.cpp resourcefileconfig.cpp
libtdeabc_file_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libtdeabc_file_la_LIBADD = $(LIB_TDEABC) $(LIB_TQT) $(top_builddir)/tderesources/libtderesources.la $(LIB_TDEFILE) $(LIB_TDECORE) $(LIB_TDEUI)
libtdeabc_file_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h


kde_module_LTLIBRARIES = tdeabc_file.la
tdeabc_file_la_SOURCES = resourcefileplugin.cpp
tdeabc_file_la_LDFLAGS = -module $(KDE_PLUGIN) $(KDE_RPATH) $(all_libraries) -no-undefined
tdeabc_file_la_LIBADD = libtdeabc_file.la $(LIB_TQT) $(LIB_TDECORE)
tdeabc_file_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabc_file.pot

tdeabcincludedir = $(includedir)/tdeabc
tdeabcinclude_HEADERS = resourcefile.h

servicedir = $(kde_servicesdir)/tderesources/tdeabc
service_DATA = file.desktop
