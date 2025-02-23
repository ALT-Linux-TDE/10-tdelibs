AM_CPPFLAGS = -DKDE_NO_COMPAT -DTQT_NO_ASCII_CAST $(all_includes)

METASOURCES = AUTO
check_PROGRAMS = tdewalletasync tdewalletsync tdewalletboth

tdewalletasync_SOURCES = tdewalletasync.cpp tdewallettest.cpp
tdewalletasync_LDFLAGS = -no-undefined $(KDE_RPATH) $(all_libraries)
tdewalletasync_LDADD = $(top_builddir)/tdewallet/client/libtdewalletclient.la \
	$(LIB_TDECORE) $(LIB_TQT)

tdewalletsync_SOURCES = tdewalletsync.cpp
tdewalletsync_LDFLAGS = -no-undefined $(KDE_RPATH) $(all_libraries)
tdewalletsync_LDADD = $(top_builddir)/tdewallet/client/libtdewalletclient.la \
	$(LIB_TDECORE) $(LIB_TQT)

tdewalletboth_SOURCES = tdewalletboth.cpp tdewallettest.cpp
tdewalletboth_LDFLAGS = -no-undefined $(KDE_RPATH) $(all_libraries)
tdewalletboth_LDADD = $(top_builddir)/tdewallet/client/libtdewalletclient.la \
	$(LIB_TDECORE) $(LIB_TQT)
INCLUDES = -I$(top_srcdir)/tdewallet/client
