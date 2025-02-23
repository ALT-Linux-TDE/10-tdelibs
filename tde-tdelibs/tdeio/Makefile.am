#    This file is part of the KDE libraries
#    Copyright (C) 1997 Torben Weis (weis@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

AM_CPPFLAGS = -D_LARGEFILE64_SOURCE

INCLUDES=  -I$(srcdir)/.. $(all_includes) $(SSL_INCLUDES)

SUBDIRS = kssl tdeio bookmarks tdefile . pics tests tdeioexec httpfilter misc kpasswdserver

lib_LTLIBRARIES = libtdeio.la

libtdeio_la_SOURCES = dummy.cpp

libtdeio_la_LDFLAGS = -version-info 6:0:2 -no-undefined $(all_libraries) \
	$(KDE_MT_LDFLAGS)
libtdeio_la_LIBADD = kssl/libkssl.la tdeio/libtdeiocore.la \
	tdeio/libtdesycoca.la bookmarks/libkbookmarks.la tdefile/libtdefile.la \
	../tdeui/libtdeui.la ../tdesu/libtdesu.la \
	../tdewallet/client/libtdewalletclient.la \
	$(LIBZ) $(LIBFAM) $(LIBVOLMGT) $(ACL_LIBS) $(LIB_TQT) $(LIB_TDECORE) $(top_builddir)/dcop/libDCOP.la $(LIB_X11)

magicdir = $(kde_confdir)/magic
magic_DATA = tdeio.magic tdeio.magic.mgc

kde_servicetypes_DATA = application.desktop kurifilterplugin.desktop \
                        kcomprfilter.desktop kscan.desktop kdatatool.desktop \
                        tdefileplugin.desktop tdecmodule.desktop

EXTRA_DIST = $(kde_mime_DATA)

update_DATA = tdeioslave.upd
update_SCRIPTS = useragent.pl proxytype.pl
updatedir = $(kde_datadir)/tdeconf_update

servicetype_DATA = renamedlgplugin.desktop
servicetypedir = $(kde_servicetypesdir)

kded_DATA = kpasswdserver.desktop
kdeddir = $(kde_servicesdir)/kded

dataprotocol_DATA = data.protocol
dataprotocoldir = $(kde_servicesdir)

dummy.cpp:
	echo >dummy.cpp

messages:
	$(EXTRACTRC) `find . ../tdeioslave -name "*.rc" -o -name "*.ui"` > rc.cpp
	$(XGETTEXT) `find . ../tdeioslave -name "*.cpp" -o -name "*.h"` -o $(podir)/tdeio.pot
	rm -f rc.cpp

SUFFIXES = .magic .magic.mgc

.magic.magic.mgc:
	(cd `dirname $@` && file -C -m $<)

DOXYGEN_REFERENCES = tdecore tdefx kjs dcop tdeui tdeio/tdeio tdeio/tdefile
DOXYGEN_EXCLUDE = kssl/kssl
include ../admin/Doxyfile.am
