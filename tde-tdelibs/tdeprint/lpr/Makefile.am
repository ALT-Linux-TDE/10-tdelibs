AM_CPPFLAGS = -D_TDEPRINT_COMPILE

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/tdeprint -I$(top_srcdir)/tdeprint/management $(all_includes)

kde_module_LTLIBRARIES = tdeprint_lpr.la

tdeprint_lpr_la_SOURCES = printcapreader.cpp lprhandler.cpp \
			     matichandler.cpp klprfactory.cpp kmlprmanager.cpp \
			     kmlpruimanager.cpp lpchelper.cpp lpqhelper.cpp \
			     kmlprjobmanager.cpp matichelper.cpp \
			     klprprinterimpl.cpp printcapentry.cpp \
			     lprsettings.cpp kmconfiglpr.cpp apshandler.cpp \
			     lprngtoolhandler.cpp \
			     editentrydialog.cpp
tdeprint_lpr_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
tdeprint_lpr_la_LIBADD = $(top_builddir)/tdeprint/management/libtdeprint_management.la \
    $(LIB_TQT) $(LIB_TDEPRINT) $(LIB_TDEUI) $(LIB_TDECORE)
tdeprint_lpr_la_METASOURCES = AUTO

noinst_HEADERS = printcapreader.h \
		 matichandler.h kmlprmanager.h \
		 kmlpruimanager.h lpchelper.h lpqhelper.h \
		 kmlprjobmanager.h matichelper.h klprprinterimpl.h \
		 kmconfiglpr.h apshandler.h \
		 lprngtoolhandler.h \
		 editentrydialog.h

tdeprintlpr_HEADERS = printcapentry.h lprhandler.h lprsettings.h
tdeprintlprdir = $(includedir)/tdeprint/lpr

bin_PROGRAMS = make_driver_db_lpr
make_driver_db_lpr_SOURCES = make_driver_db_lpr.c
make_driver_db_lpr_LDFLAGS = $(all_libraries) 
make_driver_db_lpr_LDADD = $(top_builddir)/tdecore/libtdefakes.la ../libdriverparse.a $(LIBDL)

entry_DATA = lpr.print
entrydir = $(kde_datadir)/tdeprint/plugins

miscdata_DATA = apsdriver1 apsdriver2 lprngtooldriver1
miscdatadir = $(kde_datadir)/tdeprint
