# $Id$
# Makefile.am of tdebase/tdeioslave/http

SUBDIRS = kcookiejar

INCLUDES= -I$(top_srcdir)/interfaces -I$(top_srcdir)/tdeio/httpfilter -I$(top_srcdir)/tdecore/network $(all_includes) $(GSSAPI_INCS)
AM_LDFLAGS = $(all_libraries) $(GSSAPI_RPATH)

####### Files

bin_PROGRAMS=
lib_LTLIBRARIES=
tdeinit_LTLIBRARIES = tdeio_http_cache_cleaner.la
kde_module_LTLIBRARIES = tdeio_http.la

tdeio_http_la_SOURCES = http.cpp
tdeio_http_la_METASOURCES = AUTO
tdeio_http_la_LIBADD = $(LIB_TDEIO) $(top_builddir)/tdeio/httpfilter/libhttpfilter.la $(LIB_TQT) $(LIB_TDECORE) $(LIBZ) $(top_builddir)/dcop/libDCOP.la $(top_builddir)/tdeio/misc/tdentlm/libtdentlm.la
tdeio_http_la_LDFLAGS = $(all_libraries) $(GSSAPI_RPATH) -module $(KDE_PLUGIN) $(GSSAPI_LIBS)

tdeio_http_cache_cleaner_la_SOURCES = http_cache_cleaner.cpp
tdeio_http_cache_cleaner_la_LIBADD  = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE) $(top_builddir)/dcop/libDCOP.la
tdeio_http_cache_cleaner_la_LDFLAGS = -module -avoid-version

noinst_HEADERS = http.h

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = http_cache_cleaner.desktop http.protocol https.protocol \
              webdav.protocol webdavs.protocol

include $(top_srcdir)/admin/Doxyfile.am
