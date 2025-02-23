INCLUDES = -I$(top_srcdir)/tdeabc -I$(top_builddir)/tdeabc $(all_includes)

# these are the headers for your project
noinst_HEADERS = resourcenetconfig.h

lib_LTLIBRARIES = libtdeabc_net.la
libtdeabc_net_la_SOURCES = resourcenet.cpp resourcenetconfig.cpp
libtdeabc_net_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:0:0 -no-undefined
libtdeabc_net_la_LIBADD	= $(top_builddir)/tdeabc/libtdeabc.la $(LIB_TDEIO)
libtdeabc_net_la_COMPILE_FIRST = $(top_builddir)/tdeabc/addressee.h

kde_module_LTLIBRARIES = tdeabc_net.la
tdeabc_net_la_SOURCES = resourcenetplugin.cpp
tdeabc_net_la_LDFLAGS	= $(all_libraries) -module $(KDE_PLUGIN) $(LIB_TQT) -L../../../tdecore/.libs/ -ltdecore
tdeabc_net_la_LIBADD	= libtdeabc_net.la

METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeabc_net.pot

tdeabcincludedir = $(includedir)/tdeabc
tdeabcinclude_HEADERS = resourcenet.h

servicedir = $(kde_servicesdir)/tderesources/tdeabc
service_DATA = net.desktop

resourcenetplugin.lo: ../../addressee.h
