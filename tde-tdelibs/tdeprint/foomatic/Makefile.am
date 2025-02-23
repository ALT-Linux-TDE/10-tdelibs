INCLUDES = -I$(top_srcdir)/tdeprint $(all_includes)

kde_module_LTLIBRARIES = tdeprint_foomatic.la

tdeprint_foomatic_la_SOURCES = kmfoomaticfactory.cpp \
				  kmfoomaticmanager.cpp \
				  kfoomaticprinterimpl.cpp
tdeprint_foomatic_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
tdeprint_foomatic_la_LIBADD = $(top_builddir)/tdeprint/management/libtdeprint_management.la
tdeprint_foomatic_la_METASOURCES = AUTO

noinst_HEADERS = kmfoomaticmanager.h \
		 kfoomaticprinterimpl.h

bin_PROGRAMS = make_driver_db_foomatic

make_driver_db_foomatic_SOURCES = make_driver_db_foomatic.c

entry_DATA = foomatic.print
entrydir = $(kde_datadir)/tdeprint/plugins
