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


INCLUDES= -I$(srcdir)/../libltdl/ -I$(top_srcdir) -I$(top_srcdir)/tdefx -I$(top_builddir)/tdeio/tdeio $(all_includes)

noinst_LTLIBRARIES  = libkbookmarks.la

METASOURCES = AUTO

# convenience lib - no _LDFLAGS or _LIBADD !

include_HEADERS = \
	kbookmark.h kbookmarkbar.h kbookmarkdrag.h kbookmarkexporter.h \
	kbookmarkimporter.h kbookmarkmanager.h kbookmarkmenu.h kbookmarknotifier.h \
	kbookmarkimporter_crash.h kbookmarkimporter_opera.h kbookmarkimporter_ie.h \
	kbookmarkimporter_ns.h kbookmarkimporter_kde1.h kbookmarkdombuilder.h 
libkbookmarks_la_SOURCES = \
	kbookmark.cpp kbookmarkbar.cpp kbookmarkdrag.cpp kbookmarkexporter.cpp \
	kbookmarkimporter.cpp kbookmarkmanager.cpp kbookmarkmenu.cpp \
	kbookmarkimporter_crash.cpp kbookmarkimporter_opera.cpp kbookmarkimporter_ie.cpp \
	kbookmarkimporter_ns.cpp kbookmarkimporter_kde1.cpp kbookmarkdombuilder.cpp \
	kbookmarkmanager.skel kbookmarknotifier.skel

include $(top_srcdir)/admin/Doxyfile.am
