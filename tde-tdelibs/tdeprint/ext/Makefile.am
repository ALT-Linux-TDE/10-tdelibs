AM_CPPFLAGS = -D_TDEPRINT_COMPILE

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/tdeprint $(all_includes)

kde_module_LTLIBRARIES = tdeprint_ext.la

tdeprint_ext_la_SOURCES = kmextfactory.cpp kmextmanager.cpp kextprinterimpl.cpp kmextuimanager.cpp
tdeprint_ext_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
tdeprint_ext_la_LIBADD = $(LIB_TDEPRINT) $(LIB_TQT) $(LIB_TDECORE)
tdeprint_ext_la_METASOURCES = AUTO

noinst_HEADERS = kmextmanager.h kextprinterimpl.h kmextuimanager.h

entry_DATA = ext.print
entrydir = $(kde_datadir)/tdeprint/plugins
