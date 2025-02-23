# $Id$
# Makefile.am of tdebase/tdeioslave/http

INCLUDES=	$(all_includes) 

####### Files

check_PROGRAMS = kcookiejartest

kcookiejartest_SOURCES = kcookiejartest.cpp 
kcookiejartest_LDADD   = $(LIB_TDEIO)
kcookiejartest_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor

check-local: kcookiejartest
	./kcookiejartest $(srcdir)/cookie.test
	./kcookiejartest $(srcdir)/cookie_rfc.test
	./kcookiejartest $(srcdir)/cookie_saving.test
	./kcookiejartest $(srcdir)/cookie_settings.test
