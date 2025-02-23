INCLUDES = -I$(top_srcdir)/tdeio $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kbzip2filter.la

kbzip2filter_la_SOURCES = kbzip2filter.cpp
kbzip2filter_la_LIBADD = $(LIB_TDEIO) $(LIBBZ2) $(LIB_TQT) $(LIB_TDECORE)
kbzip2filter_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

kde_services_DATA = kbzip2filter.desktop

