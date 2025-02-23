INCLUDES = -I$(top_srcdir)/tdeabc -I$(top_builddir)/tdeabc $(all_includes)

# these are the headers for your project
noinst_HEADERS = resourceldaptdeioconfig.h

lib_LTLIBRARIES = libtdeabc_ldaptdeio.la
libtdeabc_ldaptdeio_la_SOURCES = resourceldaptdeio.cpp resourceldaptdeioconfig.cpp
libtdeabc_ldaptdeio_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libtdeabc_ldaptdeio_la_LIBADD	= $(LIB_TDEABC) $(LIB_TDEIO) $(LIB_TQT) $(top_builddir)/tderesources/libtderesources.la $(LIB_TDEUI) $(LIB_TDECORE)
libtdeabc_ldaptdeio_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h

kde_module_LTLIBRARIES = tdeabc_ldaptdeio.la
tdeabc_ldaptdeio_la_SOURCES = resourceldaptdeioplugin.cpp
tdeabc_ldaptdeio_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN)
tdeabc_ldaptdeio_la_LIBADD	= libtdeabc_ldaptdeio.la $(LIB_TQT) $(LIB_TDECORE)

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabc_ldaptdeio.pot

tdeabcincludedir = $(includedir)/tdeabc
tdeabcinclude_HEADERS = resourceldaptdeio.h

servicedir = $(kde_servicesdir)/tderesources/tdeabc
service_DATA = ldaptdeio.desktop

resourceldaptdeioplugin.lo: ../../addressee.h
