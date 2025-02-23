# $Id$
# Makefile.am of tdebase/tdeioslave/http

INCLUDES= -I$(top_srcdir)/tdewallet/client $(all_includes)

####### Files

kde_module_LTLIBRARIES = kded_kpasswdserver.la

kded_kpasswdserver_la_SOURCES = kpasswdserver.cpp kpasswdserver.skel 
kded_kpasswdserver_la_METASOURCES = AUTO
kded_kpasswdserver_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_kpasswdserver_la_LIBADD = $(top_builddir)/tdewallet/client/libtdewalletclient.la $(LIB_TDEIO) $(LIB_KDED) $(LIB_TQT) $(LIB_TDEUI) $(LIB_TDECORE) $(top_builddir)/dcop/libDCOP.la $(LIB_X11)

include $(top_srcdir)/admin/Doxyfile.am
