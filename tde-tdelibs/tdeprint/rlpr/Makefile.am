AM_CPPFLAGS = -D_TDEPRINT_COMPILE

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/tdeprint -I$(top_srcdir)/tdeprint/management $(all_includes)

kde_module_LTLIBRARIES = tdeprint_rlpr.la

tdeprint_rlpr_la_SOURCES = kmrlprfactory.cpp kmrlprmanager.cpp krlprprinterimpl.cpp kmrlpruimanager.cpp \
			      kmwrlpr.cpp kmproprlpr.cpp kmproxywidget.cpp kmconfigproxy.cpp
tdeprint_rlpr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
tdeprint_rlpr_la_LIBADD = $(top_builddir)/tdeprint/management/libtdeprint_management.la $(LIB_TQT) $(LIB_TDEPRINT) $(LIB_TDECORE) $(LIB_TDEUI)
tdeprint_rlpr_la_METASOURCES = AUTO

noinst_HEADERS = kmrlprmanager.h krlprprinterimpl.h kmrlpruimanager.h kmwrlpr.h kmproprlpr.h \
		 kmproxywidget.h kmconfigproxy.h

entry_DATA = rlpr.print
entrydir = $(kde_datadir)/tdeprint/plugins
