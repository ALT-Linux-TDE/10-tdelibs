
#	This file is part of the KDE libraries
#    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
#			  (C) 1997 Stephan Kulow (coolo@kde.org)

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

AM_CPPFLAGS = -DQT_PLUGIN

INCLUDES = -I$(top_srcdir)/tdefx $(all_includes)
noinst_HEADERS = lightstyle-v2.h lightstyle-v3.h
kde_style_LTLIBRARIES = light.la
light_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
light_la_LIBADD =  $(LIB_TQT) ../../tdefx/libtdefx.la
light_la_SOURCES = light.cpp lightstyle-v2.cpp lightstyle-v3.cpp
light_la_METASOURCES = AUTO
