#    This file is part of the KDE libraries
#    Copyright (C) 2002 George Staikos <staikos@kde.org>

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

INCLUDES= -I$(srcdir)/.. -I$(top_srcdir) -I$(top_srcdir)/tdewallet/backend -I$(top_builddir)/tdewallet/backend -I$(top_srcdir)/tdewallet/client -I$(top_builddir)/tdewallet/client $(all_includes)

kde_module_LTLIBRARIES = kded_tdewalletd.la

kded_tdewalletd_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_tdewalletd_la_LIBADD = $(LIB_TDEIO) ../../../tdewallet/backend/libtdewalletbackend.la ../../../tdewallet/client/libtdewalletclient.la $(LIB_KDED) $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la $(LIB_TDECORE) $(LIB_TDEUI)
kded_tdewalletd_la_SOURCES = tdewalletd.cpp tdewalletd.skel ktimeout.cpp tdewalletwizard.ui kbetterthankdialogbase.ui

METASOURCES = AUTO

noinst_HEADERS = tdewalletd.h ktimeout.h tdewalletwizard.ui.h

services_DATA = tdewalletd.desktop
servicesdir = $(kde_servicesdir)/kded

tdewalletwizard.lo: tdewalletwizard.ui tdewalletwizard.ui.h
kbetterthankdialogbase.lo: kbetterthankdialogbase.ui kbetterthankdialogbase.ui.h
