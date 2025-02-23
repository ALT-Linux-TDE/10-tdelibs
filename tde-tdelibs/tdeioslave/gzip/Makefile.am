INCLUDES = -I$(top_srcdir)/tdeio $(all_includes)
AM_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
METASOURCES = AUTO

kde_module_LTLIBRARIES = kgzipfilter.la

kgzipfilter_la_SOURCES = kgzipfilter.cpp
kgzipfilter_la_LIBADD = $(LIB_TDEIO) $(LIBZ) $(LIB_TQT) $(LIB_TDECORE)
kgzipfilter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

kde_services_DATA = kgzipfilter.desktop

