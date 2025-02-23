INCLUDES = -I$(top_builddir)/arts/kde \
           -I$(top_srcdir) -I$(includedir)/arts $(all_includes)

lib_LTLIBRARIES = libartskde.la
libartskde_la_SOURCES = artskde.cpp kioinputstream_impl.cpp kplayobject.cpp \
                        kplayobjectfactory.cpp kartsfloatwatch.cpp kartsdispatcher.cpp \
                        kaudiorecordstream.cpp kaudioplaystream.cpp \
                        kartsserver.cpp kdatarequest_impl.cpp kaudioconverter.cpp \
                        kvideowidget.cpp kplayobjectcreator.cpp \
                        kaudiomanagerplay.cpp
libartskde_la_LIBADD = $(LIB_TDEIO) -lqtmcop -lsoundserver_idl -lartsflow_idl -lmcop $(LIB_TQT) $(LIB_TDECORE) -lkmedia2_idl -lartsflow $(LIB_TDEUI) $(LIB_X11)
libartskde_la_LDFLAGS = $(all_libraries) -no-undefined -version-info 3:0:2
libartskde_la_METASOURCES = AUTO
libartskde_la_COMPILE_FIRST = artskde.h

artskdeincludedir = $(includedir)/arts
artskdeinclude_HEADERS = kplayobject.h kplayobjectfactory.h kartsfloatwatch.h \
                         artskde.h kartsdispatcher.h \
                         kaudiorecordstream.h kaudioplaystream.h \
                         kartsserver.h kvideowidget.h kaudiomanagerplay.h
noinst_HEADERS = kplayobjectcreator.h kplayobjectfactory_p.h

artskde.mcoptype: artskde.h
artskde.mcopclass: artskde.h
MCOPINC = -I$(srcdir)
artskde.cpp artskde.h: $(top_srcdir)/arts/kde/artskde.idl $(MCOPIDL)
	     $(MCOPIDL) -I$(includedir)/arts -t $(MCOPINC) $(top_srcdir)/arts/kde/artskde.idl

DISTCLEANFILES = artskde.cpp artskde.h artskde.mcoptype artskde.mcopclass

check_PROGRAMS = kiotest kiotestslow kconverttest
kiotest_SOURCES = kiotest.cpp
kiotestslow_SOURCES = kiotestslow.cpp
kconverttest_SOURCES = kconverttest.cpp

kiotest_LDADD = libartskde.la -lqtmcop -lkmedia2 -lsoundserver_idl
kiotestslow_LDADD = $(kiotest_LDADD)
kconverttest_LDADD = $(kiotest_LDADD)
