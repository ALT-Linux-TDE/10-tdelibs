## Makefile.am for libtqt-addon

# this is the program that gets installed.  it's name is used for all
# of the other Makefile.am variables
noinst_LTLIBRARIES = libtdecorenetwork.la

# set the include path for X, qt and KDE
INCLUDES = $(all_includes)

# the library search path. 
# convenience lib - no LDFLAGS or LIBADD !
# Note:
#  tdesocketdevice.cpp must appear before any inclusion of tdesocketdevice.h
libtdecorenetwork_la_SOURCES = kresolver.cpp \
		kresolvermanager.cpp \
		kresolverworkerbase.cpp \
		tdesocketaddress.cpp \
		kresolverstandardworkers.cpp \
		kreverseresolver.cpp \
		tdesocketdevice.cpp \
		tdesocketbase.cpp \
		kclientsocketbase.cpp \
		kstreamsocket.cpp \
		kserversocket.cpp \
		kdatagramsocket.cpp \
		khttpproxysocketdevice.cpp \
		ksockssocketdevice.cpp \
		kbufferedsocket.cpp \
		tdesocketbuffer.cpp \
		ksrvresolverworker.cpp

include_HEADERS = kresolver.h \
	kreverseresolver.h \
	tdesocketaddress.h \
	tdesocketbase.h \
	tdesocketdevice.h \
	kclientsocketbase.h \
	kstreamsocket.h \
	kserversocket.h \
	kdatagramsocket.h \
	tdemulticastsocketdevice.h \
	tdemulticastsocket.h \
	knetworkinterface.h \
	khttpproxysocketdevice.h \
	ksockssocketdevice.h \
	kbufferedsocket.h \
	tdeiobuffer.h
noinst_HEADERS = kresolver_p.h \
	kresolverworkerbase.h \
	kresolverstandardworkers_p.h \
	tdesocketbuffer_p.h \
	ksrvresolverworker_p.h \
	syssocket.h

configdir = $(kde_confdir)
config_DATA = ipv6blacklist

# let automoc handle all of the meta source files (moc)
METASOURCES = AUTO
