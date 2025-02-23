AM_CPPFLAGS = -D_TDEPRINT_COMPILE

SUBDIRS = cupsdconf2

INCLUDES= -I$(top_srcdir) -I$(top_srcdir)/tdeio -I$(top_srcdir)/tdefile -I$(top_srcdir)/tdeprint -I$(top_srcdir)/tdeprint/management -I$(top_srcdir)/tdecore/network $(all_includes)

kde_module_LTLIBRARIES = tdeprint_cups.la

tdeprint_cups_la_SOURCES = cupsinfos.cpp ipprequest.cpp kmcupsfactory.cpp kmcupsjobmanager.cpp \
			      kmcupsmanager.cpp kmcupsuimanager.cpp kmpropbanners.cpp kmwbanners.cpp kmwipp.cpp \
			      kmwippselect.cpp kcupsprinterimpl.cpp kmcupsconfigwidget.cpp \
			      kmconfigcups.cpp kmconfigcupsdir.cpp kmwippprinter.cpp kmwfax.cpp kmwquota.cpp \
			      kmpropquota.cpp kmwusers.cpp kmpropusers.cpp kpschedulepage.cpp image.cpp \
				  imagepreview.cpp imageposition.cpp kpimagepage.cpp \
				  kptextpage.cpp kphpgl2page.cpp cupsaddsmb2.cpp kmwother.cpp ippreportdlg.cpp \
				  kptagspage.cpp
tdeprint_cups_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
tdeprint_cups_la_LIBADD = $(top_builddir)/tdeprint/management/libtdeprint_management.la $(LIB_TDEIO) $(LIB_CUPS) $(LIB_TQT) $(LIB_TDEPRINT) $(LIB_TDECORE) $(LIB_TDEUI)
tdeprint_cups_la_METASOURCES = AUTO

noinst_HEADERS = cupsinfos.h ipprequest.h kmcupsjobmanager.h kmcupsmanager.h kmcupsuimanager.h \
		 kmpropbanners.h kmwbanners.h kmwipp.h kmwippselect.h kcupsprinterimpl.h \
		 kmcupsconfigwidget.h kmconfigcups.h kmconfigcupsdir.h kmwippprinter.h kmwfax.h kmwquota.h \
		 kmpropquota.h kmwusers.h kmpropusers.h kpschedulepage.h imagepreview.h imageposition.h \
		 kpimagepage.h kptextpage.h kphpgl2page.h cupsaddsmb2.h \
		 kmwother.h ippreportdlg.h kptagspage.h

bin_PROGRAMS = make_driver_db_cups cupsdoprint
make_driver_db_cups_SOURCES = make_driver_db_cups.cpp
make_driver_db_cups_LDADD = ../libdriverparse.a $(top_builddir)/tdecore/libtdefakes.la $(LIBDL) $(LIBZ) $(LIB_TQT)
make_driver_db_cups_LDFLAGS = $(all_libraries)

cupsdoprint_SOURCES = cupsdoprint.c
cupsdoprint_LDADD = $(LIB_CUPS) $(top_builddir)/tdecore/libtdefakes.la
cupsdoprint_LDFLAGS = $(all_libraries)

entry_DATA = cups.print
entrydir = $(kde_datadir)/tdeprint/plugins

misc_DATA = preview.png preview-mini.png
miscdir = $(kde_datadir)/tdeprint

EXTRA_DIST = postscript.ppd.gz

install-data-local:
	if test -n "$(cups_modeldir)"; then \
		$(INSTALL_DATA) $(srcdir)/postscript.ppd.gz $(DESTDIR)$(cups_modeldir)/postscript.ppd.gz || true ;\
	fi

