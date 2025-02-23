/*
   This file is part of the KDE libraries
   Copyright (C) 2004 Jaroslaw Staniek <js@iidea.pl>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <io.h> /* to avoid #includes */

#define KPATH_SEPARATOR ';'
#define popen _popen
#define pclose _pclose

#define KDE_IMPORT __declspec(dllimport)

#ifdef MAKE_TDECORE_LIB
# define TDECORE_EXPORT TDE_EXPORT
#else
# ifndef TDECORE_EXPORT
#  define TDECORE_EXPORT KDE_IMPORT //for apps and other libs
# endif
#endif

#ifdef MAKE_KDEWIN32_LIB
# define KDEWIN32_EXPORT TDE_EXPORT
#else
# define KDEWIN32_EXPORT KDE_IMPORT
#endif

/* some classes, i.e. KDock* already use EXPORT_* macro: define it too */
#ifdef MAKE_TDEUI_LIB
# define TDEUI_EXPORT TDE_EXPORT
# define EXPORT_DOCKCLASS TDE_EXPORT
#elif KDE_MAKE_LIB
# define TDEUI_EXPORT KDE_IMPORT
# define EXPORT_DOCKCLASS KDE_IMPORT /* for library build export docklass by default */
#else
# define TDEUI_EXPORT
# define EXPORT_DOCKCLASS
#endif

#ifdef MAKE_TDEFX_LIB
# define TDEFX_EXPORT  TDE_EXPORT
#else
# define TDEFX_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_TDEPRINT_LIB
# define TDEPRINT_EXPORT  TDE_EXPORT
#else
# define TDEPRINT_EXPORT KDE_IMPORT
#endif

#ifndef TDEIO_EXPORT
# ifdef MAKE_TDEIO_LIB
#  define TDEIO_EXPORT TDE_EXPORT
# else
#  define TDEIO_EXPORT KDE_IMPORT
# endif
#endif

#ifdef MAKE_DCOP_LIB
# define DCOP_EXPORT TDE_EXPORT
#else
# define DCOP_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_TDEPARTS_LIB
# define TDEPARTS_EXPORT TDE_EXPORT
#else
# define TDEPARTS_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KTEXTEDITOR_LIB
# define KTEXTEDITOR_EXPORT TDE_EXPORT
#else
# define KTEXTEDITOR_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KABC_LIB
# define KABC_EXPORT TDE_EXPORT
#else
# define KABC_EXPORT KDE_IMPORT
#endif


#ifdef MAKE_KVCARD_LIB
# define KVCARD_EXPORT TDE_EXPORT
#else
# define KVCARD_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_TDERESOURCES_LIB
# define TDERESOURCES_EXPORT TDE_EXPORT
#else
# define TDERESOURCES_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_TDESU_LIB
# define TDESU_EXPORT TDE_EXPORT
#else
# define TDESU_EXPORT KDE_IMPORT
#endif

// all TDEStyle libs
#ifdef MAKE_TDESTYLE_LIB
# define TDESTYLE_EXPORT TDE_EXPORT
#else
# define TDESTYLE_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KMDI_LIB
# define KMDI_EXPORT  TDE_EXPORT
#else
# define KMDI_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_TDEUTILS_LIB
# define TDEUTILS_EXPORT  TDE_EXPORT
#else
# define TDEUTILS_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KATEPARTINTERFACES_LIB
# define KATEPARTINTERFACES_EXPORT  TDE_EXPORT
#else
# define KATEPARTINTERFACES_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_KATEPART_LIB
# define KATEPART_EXPORT  TDE_EXPORT
#else
# define KATEPART_EXPORT KDE_IMPORT
#endif

#ifdef MAKE_TDEHTML_LIB
# define TDEHTML_EXPORT TDE_EXPORT
#else
# define TDEHTML_EXPORT KDE_IMPORT
#endif
