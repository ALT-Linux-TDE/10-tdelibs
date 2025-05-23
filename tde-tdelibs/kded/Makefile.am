#    This file is part of the KDE libraries
#    Copyright (C) 1997 David Faure <faure@kde.org>
#    Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

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

INCLUDES= -I$(srcdir)/.. -I$(top_srcdir) $(all_includes)

lib_LTLIBRARIES =
tdeinit_LTLIBRARIES = kded.la tdebuildsycoca.la

kded_la_LDFLAGS = $(all_libraries) -module -avoid-version
kded_la_LIBADD = $(LIB_TDEIO) $(LIB_TQT) $(LIB_TDECORE) $(top_builddir)/dcop/libDCOP.la $(LIB_X11)
kded_la_SOURCES = kded.cpp kdedmodule.cpp

tdebuildsycoca_la_LDFLAGS = $(all_libraries) -module -avoid-version
tdebuildsycoca_la_LIBADD = $(LIB_TDEIO) $(LIB_TQT) $(top_builddir)/dcop/libDCOP.la $(LIB_TDECORE)
tdebuildsycoca_la_SOURCES = tdebuildsycoca.cpp kbuildservicetypefactory.cpp \
				kbuildservicefactory.cpp \
				kbuildservicegroupfactory.cpp \
				kbuildimageiofactory.cpp \
				kbuildprotocolinfofactory.cpp \
				kctimefactory.cpp \
				vfolder_menu.cpp

bin_PROGRAMS = kdontchangethehostname tde-menu tdemimelist

kdontchangethehostname_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
kdontchangethehostname_LDADD = $(LIB_TDECORE)
kdontchangethehostname_SOURCES = khostname.cpp

tdemimelist_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
tdemimelist_LDADD = ../tdeio/libtdeio.la
tdemimelist_SOURCES = tdemimelist.cpp

kde_menu_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
kde_menu_LDADD = $(LIB_TDEIO)
kde_menu_SOURCES = tde-menu.cpp

METASOURCES = AUTO

include_HEADERS = kdedmodule.h

noinst_HEADERS = tdebuildsycoca.h kbuildservicetypefactory.h \
		kbuildservicefactory.h kbuildservicegroupfactory.h \
		kbuildimageiofactory.h kresourcelist.h \
		kbuildprotocolinfofactory.h

servicetype_DATA = kdedmodule.desktop
servicetypedir = $(kde_servicetypesdir)

xdg_menu_DATA = applications.menu applications.menu-no-kde

update_DATA = kded.upd
updatedir = $(kde_datadir)/tdeconf_update

include $(top_srcdir)/admin/Doxyfile.am

