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

#ifndef _TDELIBS_EXPORT_H
#define _TDELIBS_EXPORT_H

/* needed for TDE_EXPORT macros */
#include <tdemacros.h>

/* needed, because e.g. Q_OS_UNIX is so frequently used */
#include <tqglobal.h>

#ifdef TQ_WS_WIN
#include <tdelibs_export_win.h>

#else /* Q_OS_UNIX */

/* export statements for unix */
#define TDECORE_EXPORT TDE_EXPORT
#define TDEUI_EXPORT TDE_EXPORT
#define TDEFX_EXPORT TDE_EXPORT
#define TDEPRINT_EXPORT TDE_EXPORT
#define TDEDNSSD_EXPORT TDE_EXPORT
#define TDEIO_EXPORT TDE_EXPORT
#define DCOP_EXPORT TDE_EXPORT
#define TDEPARTS_EXPORT TDE_EXPORT
#define KTEXTEDITOR_EXPORT TDE_EXPORT
#define KABC_EXPORT TDE_EXPORT
#define TDESU_EXPORT TDE_EXPORT
#define KVCARD_EXPORT TDE_EXPORT
#define TDERESOURCES_EXPORT TDE_EXPORT
#define TDESTYLE_EXPORT TDE_EXPORT
#define TDEHTML_EXPORT TDE_EXPORT
#define KMDI_EXPORT TDE_EXPORT
#define TDEUTILS_EXPORT TDE_EXPORT
#define KATEPARTINTERFACES_EXPORT TDE_EXPORT
#define KATEPART_EXPORT TDE_EXPORT
#define KMID_EXPORT TDE_EXPORT
#define TDERANDR_EXPORT TDE_EXPORT
#define TDEIMPROXY_EXPORT TDE_EXPORT
#define KDE_ARTS_EXPORT TDE_EXPORT
#define TDEUNITTEST_EXPORT TDE_EXPORT

#define KPATH_SEPARATOR ':'

#ifndef O_BINARY
#define O_BINARY 0 /* for open() */
#endif

#endif

#endif /*_TDELIBS_EXPORT_H*/

/* workaround for tdecore: stupid moc's grammar doesn't accept two macros
   between 'class' keyword and <classname>: */
#ifdef TDE_DEPRECATED
# ifndef TDECORE_EXPORT_DEPRECATED
#  define TDECORE_EXPORT_DEPRECATED TDE_DEPRECATED TDECORE_EXPORT
# endif
# ifndef TDEIO_EXPORT_DEPRECATED
#  define TDEIO_EXPORT_DEPRECATED TDE_DEPRECATED TDEIO_EXPORT
# endif
# ifndef TDEUI_EXPORT_DEPRECATED
#  define TDEUI_EXPORT_DEPRECATED TDE_DEPRECATED TDEUI_EXPORT
# endif
# ifndef KABC_EXPORT_DEPRECATED
#  define KABC_EXPORT_DEPRECATED TDE_DEPRECATED KABC_EXPORT
# endif
#endif
/* (let's add KDE****_EXPORT_DEPRECATED for other libraries if it's needed) */
