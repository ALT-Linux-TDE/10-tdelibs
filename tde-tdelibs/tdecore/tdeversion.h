/* This file is part of the KDE libraries
    Copyright (c) 2002-2005 KDE Team

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef _TDE_VERSION_H_
#define _TDE_VERSION_H_

#include "tdelibs_export.h"

/*
    R <ABI VERSION> . <FEATURE REVISION> . <BUG AND SECURITY PATCH LEVEL>
    Initial release is always R<ABI VERSION>.0.0.
    BUG AND SECURITY PATCH LEVEL is always updated with a point release.
    FEATURE REVISION is updated when new features are introduced or existing
    features are significantly updated.
    A FEATURE REVISION release does NOT include ABI changes.
    A new FEATURE REVISION level always resets the BUG AND SECURITY PATCHLEVEL.
    A new ABI version resets both the FEATURE REVISION and BUG AND SECURITY PATCH LEVEL.
*/
#define TDE_VERSION_STRING "R14.1.3"
#define TDE_VERSION_MAJOR 14
#define TDE_VERSION_MINOR 1
#define TDE_VERSION_RELEASE 3
#define TDE_MAKE_VERSION( a,b,c ) (((a) << 16) | ((b) << 8) | (c))

#define TDE_VERSION \
  TDE_MAKE_VERSION(TDE_VERSION_MAJOR,TDE_VERSION_MINOR,TDE_VERSION_RELEASE)

#define KDE_IS_VERSION(a,b,c) ( TDE_VERSION >= TDE_MAKE_VERSION(a,b,c) )

/**
 * Namespace for general KDE functions.
 */
namespace KDE
{
    /**
     * Returns the encoded number of KDE's version, see the TDE_VERSION macro.
     * In contrary to that macro this function returns the number of the actully
     * installed KDE version, not the number of the KDE version that was
     * installed when the program was compiled.
     * @return the version number, encoded in a single uint
     * @since 3.2
     */
    TDECORE_EXPORT unsigned int version();
    /**
     * Returns the major number of KDE's version, e.g.
     * 3 for KDE 3.1.2. 
     * @return the major version number
     * @since 3.1
     */
    TDECORE_EXPORT unsigned int versionMajor();
    /**
     * Returns the minor number of KDE's version, e.g.
     * 1 for KDE 3.1.2. 
     * @return the minor version number
     * @since 3.1
     */
    TDECORE_EXPORT unsigned int versionMinor();
    /**
     * Returns the release of KDE's version, e.g.
     * 2 for KDE 3.1.2. 
     * @return the release number
     * @since 3.1
     */
    TDECORE_EXPORT unsigned int versionRelease();
    /**
     * Returns the KDE version as string, e.g. "3.1.2".
     * @return the KDE version. You can keep the string forever
     * @since 3.1
     */
    TDECORE_EXPORT const char *versionString();
}

#endif // _TDE_VERSION_H_
