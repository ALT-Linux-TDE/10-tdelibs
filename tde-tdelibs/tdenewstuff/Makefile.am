INCLUDES = $(all_includes)

METASOURCES = AUTO

lib_LTLIBRARIES = libknewstuff.la

libknewstuff_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0
libknewstuff_la_LIBADD  = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEUI)
libknewstuff_la_SOURCES = engine.cpp entry.cpp downloaddialog.cpp \
  uploaddialog.cpp providerdialog.cpp provider.cpp knewstuff.cpp \
  knewstuffgeneric.cpp knewstuffbutton.cpp knewstuffsecure.cpp security.cpp

bin_PROGRAMS = tdehotnewstuff

tdehotnewstuff_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
tdehotnewstuff_LDADD = libknewstuff.la
tdehotnewstuff_SOURCES = tdehotnewstuff.cpp

EXTRA_PROGRAMS = testnewstuff ghns

testnewstuff_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
testnewstuff_LDADD   = libknewstuff.la
testnewstuff_SOURCES = testnewstuff.cpp

ghns_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
ghns_LDADD   = libknewstuff.la
ghns_SOURCES = ghns.cpp

knewstuffdir = $(kde_datadir)/knewstuff
knewstuff_DATA = types

kde_conf_DATA = tdehotnewstuffrc

knsdir = $(includedir)/knewstuff
kns_HEADERS = downloaddialog.h engine.h entry.h knewstuffgeneric.h knewstuff.h providerdialog.h provider.h uploaddialog.h knewstuffbutton.h knewstuffsecure.h
noinst_HEADERS = testnewstuff.h ghns.h security.h

KDE_ICON=AUTO

include ../admin/Doxyfile.am

