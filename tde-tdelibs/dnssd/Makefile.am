# set the include path for X, qt and KDE
INCLUDES = -I$(top_srcdir) $(all_includes) $(AVAHI_CFLAGS)

# these are the headers for your project
noinst_HEADERS = sdevent.h

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO

lib_LTLIBRARIES =  libtdednssd.la

libtdednssd_la_SOURCES = remoteservice.cpp responder.cpp servicebase.cpp \
				settings.kcfgc publicservice.cpp query.cpp domainbrowser.cpp servicebrowser.cpp
dnssdincludedir = $(includedir)/dnssd
dnssdinclude_HEADERS = domainbrowser.h query.h remoteservice.h \
	publicservice.h servicebase.h servicebrowser.h settings.h
libtdednssd_la_LIBADD = ../tdecore/libtdecore.la $(AVAHI_LIBS) $(LIB_TQT)
libtdednssd_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -version-info 1:0

#kde_kcfg_DATA = kcm_tdednssd.kcfg

include ../admin/Doxyfile.am
