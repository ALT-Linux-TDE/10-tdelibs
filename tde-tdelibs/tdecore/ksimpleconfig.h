/* This file is part of the KDE libraries
   Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

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

#ifndef _KSIMPLECONFIG_H
#define _KSIMPLECONFIG_H

#include "tdeconfig.h"

class KSimpleConfigPrivate;

/**
 * KDE Configuration entries
 *
 * This is a trivial extension of TDEConfig for applications that need
 * only one configuration file and no default system.
 * A difference with TDEConfig is that when the data in memory is written back
 * it is not merged with what is on disk.
 * Whatever is in memory simply replaces what is on disk entirely.
 *
 * @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
 * @see TDEConfigBase TDEConfig
 * @short KDE Configuration Management class with deletion ability
 */
class TDECORE_EXPORT KSimpleConfig : public TDEConfig
{
  TQ_OBJECT

public:
  /**
   * Construct a KSimpleConfig object and make it either read-write
   * or read-only.
   *
   * @param fileName The file used for saving the config data. Either
   *                  a full path can be specified or just the filename.
   *                  If only a filename is specified, the default
   *                  directory for "config" files is used.
   * @param bReadOnly Whether the object should be read-only.
   */
  KSimpleConfig( const TQString &fileName, bool bReadOnly = false);

  KSimpleConfig(TDEConfigBackEnd *backEnd, bool bReadOnly = false);

  /**
   * Destructor.
   *
   * Writes back any dirty configuration entries.
   */
  virtual ~KSimpleConfig();

  virtual void sync();

private:

  // copy-construction and assignment are not allowed
  KSimpleConfig( const KSimpleConfig& );
  KSimpleConfig& operator= ( const KSimpleConfig& rConfig );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KSimpleConfigPrivate *d;
};

#endif
