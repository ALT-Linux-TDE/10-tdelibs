#	This file is part of the KDE libraries
#    Copyright (C) 1997 Stephan Kulow (coolo@kde.org)

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU General Public License
#    along with this library; see the file COPYING.  If not, write to
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.


INCLUDES= -I$(srcdir)/../libltdl/ -I$(top_srcdir) -I$(top_srcdir)/tdefx \
    -I$(top_builddir)/tdeio/tdeio -I$(top_srcdir)/tdeio/bookmarks $(all_includes) $(LIBART_CFLAGS)

noinst_LTLIBRARIES  = libtdefile.la

METASOURCES = AUTO

#SUBDIRS = . acl_prop_page

include_HEADERS = tdefiledialog.h kencodingfiledialog.h\
	tdediroperator.h tdefileview.h tdefilefiltercombo.h \
	tdefiledetailview.h kcombiview.h kdiskfreesp.h \
	tdefileiconview.h tderecentdocument.h \
	kurlrequester.h tdefilepreview.h tdefile.h \
	kurlcombobox.h kurlrequesterdlg.h kopenwith.h kpropsdlg.h \
	kicondialog.h kdirsize.h kpreviewwidgetbase.h kimagefilepreview.h tdefilesharedlg.h \
	tdefiletreeview.h tdefiletreeviewitem.h tdefiletreebranch.h \
	kdirselectdialog.h kurlbar.h kpropertiesdialog.h knotifydialog.h \
	kcustommenueditor.h knotifywidgetbase.h

noinst_HEADERS = config-tdefile.h tderecentdirs.h kmetaprops.h \
	tdefilebookmarkhandler.h tdefilemetainfowidget.h kopenwith_p.h \
	tdefilespeedbar.h kpreviewprops.h kacleditwidget.h kacleditwidget_p.h images.h

libtdefile_la_SOURCES = \
	tdefilefiltercombo.cpp \
	tdefileview.cpp tdefileiconview.cpp \
	tderecentdocument.cpp tdefiledialog.cpp tdediroperator.cpp \
	tdefiledetailview.cpp kcombiview.cpp kurlrequester.cpp \
	tdefilepreview.cpp kurlcombobox.cpp kurlrequesterdlg.cpp \
	kopenwith.cpp kpropertiesdialog.cpp kicondialog.cpp kdirsize.cpp \
	tderecentdirs.cpp kdiskfreesp.cpp kimagefilepreview.cpp tdefilesharedlg.cpp \
	kurlbar.cpp kmetaprops.cpp kpreviewprops.cpp \
	tdefiletreeview.cpp tdefiletreeviewitem.cpp tdefiletreebranch.cpp \
	kdirselectdialog.cpp tdefilebookmarkhandler.cpp \
	tdefilemetainfowidget.cpp kcustommenueditor.cpp knotifywidgetbase.ui \
	knotifydialog.cpp tdefilespeedbar.cpp kpreviewwidgetbase.cpp \
	tdefilemetapreview.cpp kpropertiesdesktopbase.ui \
	kpropertiesdesktopadvbase.ui kpropertiesmimetypebase.ui \
	kencodingfiledialog.cpp kacleditwidget.cpp

libtdefile_la_COMPILE_FIRST = $(srcdir)/../tdeio/kdirnotify_stub.h

EXTRA_DIST = NOTES

# convenience lib - no _LDFLAGS or _LIBADD !

servicetype_DATA = kpropsdlgplugin.desktop
servicetypedir = $(kde_servicetypesdir)


#Can't be a module, we need to link to it for readConfig
#kde_module_LTLIBRARIES = libtdefileshare.la
#libtdefileshare_la_SOURCES = tdefileshare.cpp
#libtdefileshare_la_LIBADD = libtdefile.la
#libtdefileshare_la_LDFLAGS = -module $(KDE_PLUGIN)
#kde_services_DATA = tdefilesharepropsplugin.desktop

include $(top_srcdir)/admin/Doxyfile.am
