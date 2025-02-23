# don't change the order of this one. it breaks when openssl is in /usr/include 
# and the distro's KDE is there too, but the current "devel" KDE prefix is 
# somewhere else. Another option for the future (needs testing) would be to
# change SSL_INCLUDES to .../include/openssl and make the source use #include <foo.h>
# instead of #include <openssl/foo.h>
INCLUDES=-I$(top_srcdir)/tdecore/network -I$(top_srcdir)/tdewallet/client $(all_includes) $(SSL_INCLUDES)

noinst_LTLIBRARIES = libkssl.la
KSSLVERSION=	3.0.0
KSSLPATCH=	0
METASOURCES= AUTO

include_HEADERS = \
		kssl.h                   \
		ksslcertificatehome.h    \
		ksslsettings.h           \
		ksslcertificate.h        \
		ksslconnectioninfo.h     \
		ksslcertificatefactory.h \
		ksslcertificatecache.h   \
		ksslpeerinfo.h           \
		ksslinfodlg.h            \
		ksslcertdlg.h            \
		ksslutils.h              \
		kopenssl.h               \
		ksslall.h	 	 \
		ksslpkcs12.h		 \
		ksslcertchain.h		 \
		ksslkeygen.h		 \
		ksslx509v3.h		 \
		ksslsigners.h		 \
		ksslpkcs7.h		 \
		ksslpemcallback.h	 \
		ksslconfig.h		 \
		ksslsession.h		 \
		ksslx509map.h		 \
		ksmimecrypto.h

noinst_HEADERS = ksslcsessioncache.h


libkssl_la_SOURCES	= \
			kssl.cpp                   \
			ksslcertificatehome.cpp    \
			ksslsettings.cpp           \
			ksslcertificate.cpp        \
			ksslconnectioninfo.cpp     \
			ksslcertificatefactory.cpp \
			ksslcertificatecache.cpp   \
			ksslpeerinfo.cpp           \
			ksslinfodlg.cpp            \
			ksslcertdlg.cpp            \
			ksslutils.cpp              \
			kopenssl.cpp		  \
			ksslpkcs12.cpp		  \
			ksslcertchain.cpp	  \
			ksslkeygen.cpp		  \
			ksslx509v3.cpp		  \
			ksslsigners.cpp		  \
			ksslpkcs7.cpp		  \
			ksslpemcallback.cpp	  \
			ksslx509map.cpp		  \
			ksslsession.cpp		  \
			keygenwizard.ui		  \
			keygenwizard2.ui	  \
			ksmimecrypto.cpp		  \
			ksslcsessioncache.cpp


SUBDIRS = kssl

include $(top_srcdir)/admin/Doxyfile.am
