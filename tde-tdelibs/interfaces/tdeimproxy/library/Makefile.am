SUBDIRS = icons

INCLUDES = $(all_includes)

noinst_HEADERS = tdeimproxyiface.h

METASOURCES = AUTO

AM_CPPFLAGS = $(all_includes)

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdeimproxy.pot

tdeimproxyincludedir = $(includedir)
tdeimproxyinclude_HEADERS = tdeimproxy.h tdeimproxyiface.h

lib_LTLIBRARIES =  libtdeimproxy.la
libtdeimproxy_la_LDFLAGS = -no-undefined $(all_libraries)
libtdeimproxy_la_LIBADD = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE) $(top_builddir)/dcop/libDCOP.la $(LIB_TDEUI)
libtdeimproxy_la_SOURCES = tdeimproxy.cpp kimiface.stub tdeimproxyiface.skel

kimiface_DIR = $(top_srcdir)/interfaces/tdeimproxy/interface

include $(top_srcdir)/admin/Doxyfile.am
DOXYGEN_REFERENCES = tdeabc dcop

