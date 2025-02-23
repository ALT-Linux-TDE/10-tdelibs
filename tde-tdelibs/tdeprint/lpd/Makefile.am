INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/tdeprint -I$(top_srcdir)/tdeprint/management $(all_includes)

kde_module_LTLIBRARIES = tdeprint_lpd.la

tdeprint_lpd_la_SOURCES = kmlpdfactory.cpp kmlpdmanager.cpp klpdprinterimpl.cpp kmlpduimanager.cpp \
			     lpdtools.cpp gschecker.cpp
tdeprint_lpd_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
tdeprint_lpd_la_LIBADD = $(top_builddir)/tdeprint/management/libtdeprint_management.la 
tdeprint_lpd_la_METASOURCES = AUTO

noinst_HEADERS = kmlpdfactory.h kmlpdmanager.h klpdprinterimpl.h kmlpduimanager.h lpdtools.h gschecker.h

bin_PROGRAMS = make_driver_db_lpd
make_driver_db_lpd_SOURCES = make_driver_db_lpd.c
make_driver_db_lpd_LDADD = $(top_builddir)/tdecore/libtdefakes.la

entry_DATA = lpd.print
entrydir = $(kde_datadir)/tdeprint/plugins
