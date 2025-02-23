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

INCLUDES=  -I$(srcdir)/.. -I$(top_srcdir) -I$(srcdir)/../kssl -I../kssl $(all_includes) $(SSL_INCLUDES)

SUBDIRS = . tdesendbugmail kpac tdesasl kssld tdefile tdewalletd tdentlm

lib_LTLIBRARIES =
tdeinit_LTLIBRARIES = tdeio_uiserver.la
bin_PROGRAMS = tdemailservice tdetelnetservice

observer_DIR = $(top_srcdir)/tdeio/tdeio

tdeio_uiserver_la_SOURCES = uiserver.cpp uiserver.skel observer.stub
tdeio_uiserver_la_LIBADD = $(LIB_TDEIO) $(LIB_TDEUI) $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la $(LIB_TDECORE)
tdeio_uiserver_la_LDFLAGS = $(all_libraries) -module -avoid-version

METASOURCES = AUTO

kde_services_DATA = tdeio_uiserver.desktop

tdemailservice_SOURCES = tdemailservice.cpp
tdemailservice_LDADD = $(LIB_TDECORE)
tdemailservice_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor

tdetelnetservice_SOURCES = tdetelnetservice.cpp
tdetelnetservice_LDADD = $(LIB_TDEUI)
tdetelnetservice_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor

protocol_DATA = tdemailservice.protocol telnet.protocol rlogin.protocol rtsp.protocol ssh.protocol \
		mms.protocol mmst.protocol mmsu.protocol pnm.protocol rtspt.protocol rtspu.protocol
protocoldir = $(kde_servicesdir)

kiomiscdir = $(includedir)/tdeio
kiomisc_HEADERS = uiserver.h


bin_SCRIPTS = fileshareset
install-exec-local:
	@-rm -f $(DESTDIR)$(bindir)/filesharelist
	@$(LN_S) fileshareset $(DESTDIR)$(bindir)/filesharelist
	@(chown root $(DESTDIR)$(bindir)/fileshareset && chmod 4755 $(DESTDIR)$(bindir)/fileshareset) || echo "Was not able to make fileshareset setuid root"

