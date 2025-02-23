
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

SUBDIRS=. pics tests
AM_CXXFLAGS = -DQT_PLUGIN

INCLUDES = -I$(top_srcdir)/tdefile $(all_includes)

bin_PROGRAMS = maketdewidgets
maketdewidgets_SOURCES = maketdewidgets.cpp
maketdewidgets_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_TQT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
maketdewidgets_LDADD = $(LIB_TQT) ../tdecore/libtdecore.la 

kde_widget_LTLIBRARIES = tdewidgets.la
tdewidgets_la_LDFLAGS = $(KDE_PLUGIN) -module $(all_libraries)
tdewidgets_la_LIBADD = $(LIB_TDEIO) $(LIB_TDEABC) $(LIB_TQT) $(LIB_TDEUI) $(LIB_TDECORE)
tdewidgets_la_SOURCES = classpreviews.cpp tdewidgets.cpp

tdewidgets.cpp: $(srcdir)/kde.widgets maketdewidgets
	./maketdewidgets -o tdewidgets.cpp $(srcdir)/kde.widgets

CLEANFILES = tdewidgets.cpp

