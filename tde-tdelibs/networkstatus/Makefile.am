#SUBDIRS = networkstatustray

METASOURCES = AUTO

INCLUDES = -I$(top_srcdir)/kded -I$(top_srcdir) $(all_includes)

kde_module_LTLIBRARIES = kded_networkstatus.la 
lib_LTLIBRARIES = libnetworkstatus.la libconnectionmanager.la

libnetworkstatus_la_LIBADD = $(LIB_TDECORE)
libnetworkstatus_la_LDFLAGS = $(all_libraries)
libnetworkstatus_la_SOURCES = networkstatuscommon.cpp

libconnectionmanager_la_LIBADD = $(LIB_TDECORE)
libconnectionmanager_la_LDFLAGS = $(all_libraries)
libconnectionmanager_la_SOURCES = connectionmanager.cpp networkstatusindicator.cpp connectionmanager.skel clientiface.stub networkstatusiface.stub

kded_networkstatus_la_SOURCES = networkstatus.cpp networkstatus.skel \
	clientiface.skel serviceiface.skel network.cpp
#kclientiface.skel serviceiface.skel
kded_networkstatus_la_LIBADD = $(LIB_TDECORE) $(LIB_TDEIO) ./libnetworkstatus.la
kded_networkstatus_la_LDFLAGS = $(all_libraries) -module -avoid-version 

servicesdir = $(kde_servicesdir)/kded
services_DATA = networkstatus.desktop

noinst_HEADERS = serviceifaceimpl.h \
	network.h clientifaceimpl.h testservice.h connectionmanager.h

include_HEADERS = serviceiface.h provideriface.h connectionmanager.h networkstatuscommon.h networkstatusindicator.h networkstatusiface.h

bin_PROGRAMS = networkstatustestservice

networkstatustestservice_LDFLAGS = $(all_libraries)
networkstatustestservice_LDADD = libnetworkstatus.la

networkstatustestservice_SOURCES = testservice.cpp provideriface.skel serviceiface.stub
