/* This file is part of the KDE project
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __k_ctime_factory_h__
#define __k_ctime_factory_h__

#include <tdesycocafactory.h>
#include <tqdict.h>

/**
 * Service group factory for building tdesycoca
 * @internal
 */
class KCTimeInfo : public KSycocaFactory
{
  K_SYCOCAFACTORY( KST_CTimeInfo )
public:
  /**
   * Create factory
   */
  KCTimeInfo();

  virtual ~KCTimeInfo();

  /**
   * Write out header information
   */
  virtual void saveHeader(TQDataStream &str);

  /**
   * Write out data 
   */
  virtual void save(TQDataStream &str);

  KSycocaEntry * createEntry(const TQString &, const char *) { return 0; }
  KSycocaEntry * createEntry(int) { return 0; }
  
  void addCTime(const TQString &path, TQ_UINT32 ctime);

  TQ_UINT32 ctime(const TQString &path);

  void fillCTimeDict(TQDict<TQ_UINT32> &dict);

protected:
  TQDict<TQ_UINT32> ctimeDict;
  int m_dictOffset;
};

#endif
