## Makefile.am of tdebase/tdeioslave/file

AM_CPPFLAGS = -D_LARGEFILE64_SOURCE

INCLUDES = $(all_includes)

####### Files

kde_module_LTLIBRARIES = tdeio_file.la

tdeio_file_la_SOURCES = file.cpp
tdeio_file_la_LIBADD = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE) $(ACL_LIBS)
tdeio_file_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN) $(top_builddir)/dcop/libDCOP.la
noinst_HEADERS = file.h

fileinclude_HEADERS = file.h
fileincludedir = $(includedir)/tdeio

METASOURCES = AUTO

kdelnkdir = $(kde_servicesdir)
kdelnk_DATA = file.protocol
