#    This file is part of the KDE libraries
#    Copyright (C) 2001 Waldo Bastian <bastian@kde.org>

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

INCLUDES= -I../kded $(all_includes)

bin_PROGRAMS = 
lib_LTLIBRARIES =
tdeinit_LTLIBRARIES = tdeconf_update.la

tdeconf_update_la_SOURCES = tdeconf_update.cpp
tdeconf_update_la_LIBADD = $(LIB_TDECORE) $(LIB_TQT)
tdeconf_update_la_LDFLAGS = $(all_libraries) -module -avoid-version

METASOURCES = AUTO

# Internal
# noinst_HEADERS = tdeconf_update.h

