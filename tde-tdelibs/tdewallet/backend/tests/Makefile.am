INCLUDES = -I$(top_srcdir)/tdewallet/backend -I$(top_srcdir)/tdewallet/client $(all_includes)

AM_LDFLAGS = $(QT_LDFLAGS) $(X_LDFLAGS) $(KDE_RPATH)

check_PROGRAMS = backendtest testbf testsha

METASOURCES = AUTO

LDADD = ../libtdewalletbackend.la ../../client/libtdewalletclient.la
backendtest_SOURCES = backendtest.cpp

testbf_SOURCES = testbf.cpp

testsha_SOURCES = testsha.cpp
