
INCLUDES= -I$(srcdir) $(all_includes)

lib_LTLIBRARIES = libtdewalletclient.la

libtdewalletclient_la_LDFLAGS = $(KDE_RPATH) $(all_libraries) -version-info 1:1 -no-undefined
libtdewalletclient_la_LIBADD = $(LIB_TDECORE) $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la
libtdewalletclient_la_SOURCES =   tdewallet.skel \
				tdewallet.cpp

libtdewalletclient_la_METASOURCES = AUTO

include_HEADERS = tdewallet.h tdewallettypes.h

