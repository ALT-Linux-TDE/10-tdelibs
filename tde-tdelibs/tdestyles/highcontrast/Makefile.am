
#	This file is part of the KDE libraries
#    Copyright (C) 2005 Olaf Schmidt (ojschmidt@kde.org)

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

AUTOMAKE_OPTIONS = foreign

SUBDIRS = config

KDE_CXXFLAGS = -DQT_PLUGIN

INCLUDES = $(all_includes)

stylercdir = $(kde_datadir)/tdestyle/themes
stylerc_DATA = highcontrast.themerc

EXTRA_DIST = $(stylerc_DATA)

noinst_HEADERS = highcontrast.h

kde_style_LTLIBRARIES = highcontrast.la
highcontrast_la_SOURCES = highcontrast.cpp
highcontrast_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
highcontrast_la_LIBADD = ../../tdefx/libtdefx.la $(LIB_TQT)
highcontrast_la_METASOURCES = AUTO

DISTCLEANFILES = $(highcontrast_la_METASOURCES)
