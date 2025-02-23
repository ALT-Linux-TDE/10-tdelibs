AM_CPPFLAGS = -D_TDEPRINT_COMPILE

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/tdeprint $(all_includes)

kde_module_LTLIBRARIES = tdeprint_lpdunix.la

tdeprint_lpdunix_la_SOURCES = kmlpdunixfactory.cpp kmlpdunixmanager.cpp klpdunixprinterimpl.cpp kmlpdunixuimanager.cpp
tdeprint_lpdunix_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
tdeprint_lpdunix_la_LIBADD =  $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEPRINT)
tdeprint_lpdunix_la_METASOURCES = AUTO

noinst_HEADERS = kmlpdunixmanager.h klpdunixprinterimpl.h kmlpdunixuimanager.h

entry_DATA = lpdunix.print
entrydir = $(kde_datadir)/tdeprint/plugins
