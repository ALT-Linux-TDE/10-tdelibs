####### General stuff

AM_LDFLAGS = $(LDFLAGS_AS_NEEDED)

INCLUDES= -I$(srcdir)/../ -I$(top_srcdir)/tdeio/ -I$(top_srcdir)/libltdl \
	-I$(top_srcdir)/tdefile $(all_includes)
libtdeparts_la_LDFLAGS = $(KDE_MT_LDFLAGS) -version-info 3:0:1 -no-undefined
libtdeparts_la_LIBADD  = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDEUI) $(LIB_TDECORE) $(top_builddir)/dcop/libDCOP.la

check-local:
	(cd tests && $(MAKE) check )

####### Files

lib_LTLIBRARIES = libtdeparts.la

libtdeparts_la_SOURCES = part.cpp plugin.cpp partmanager.cpp mainwindow.cpp dockmainwindow.cpp \
		       event.cpp browserextension.cpp factory.cpp  historyprovider.cpp \
		       browserinterface.cpp browserrun.cpp statusbarextension.cpp

tdepartsincludedir = $(includedir)/tdeparts
tdepartsinclude_HEADERS = part.h plugin.h partmanager.h mainwindow.h dockmainwindow.h event.h \
	browserextension.h factory.h historyprovider.h browserinterface.h genericfactory.h \
	componentfactory.h browserrun.h statusbarextension.h

libtdeparts_la_METASOURCES = AUTO

servicetype_DATA = kpart.desktop krop.desktop krwp.desktop browserview.desktop 
servicetypedir = $(kde_servicetypesdir)

DOXYGEN_REFERENCES = tdecore tdeui tdefx tdeio
include ../admin/Doxyfile.am
