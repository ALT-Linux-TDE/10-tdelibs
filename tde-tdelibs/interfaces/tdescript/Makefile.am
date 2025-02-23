SUBDIRS = . sample

INCLUDES = -I$(top_srcdir)/tdeio -I$(top_srcdir) -I$(srcdir) -I$(srcdir)/.. $(all_includes)

#lib_LTLIBRARIES = libtdescriptloader.la libtdescript.la
lib_LTLIBRARIES = libtdescript.la
#libtdescriptloader_la_SOURCES = scriptloader.cpp
#libtdescriptloader_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
#libtdescriptloader_la_LIBADD = $(LIB_TDEPARTS)

libtdescript_la_SOURCES = scriptmanager.cpp
libtdescript_la_LDFLAGS = $(all_libraries) $(KDE_RPATH) -no-undefined
libtdescript_la_LIBADD = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE) $(LIB_TDEUI)

tdescriptinclude_HEADERS = scriptinterface.h scriptclientinterface.h scriptmanager.h
tdescriptincludedir = $(includedir)

METASOURCES = AUTO

servicetypedir = $(kde_servicetypesdir)
servicetype_DATA = scriptinterface.desktop
