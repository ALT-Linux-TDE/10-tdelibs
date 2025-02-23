/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __k_service_type_factory_h__
#define __k_service_type_factory_h__

#include <assert.h>

#include <tqstringlist.h>
#include <tqvaluevector.h>

#include "tdesycocafactory.h"
#include "kmimetype.h"

class KSycoca;
class KSycocaDict;

class KServiceType;
class KFolderType;
class KDEDesktopMimeType;
class KExecMimeType;

/**
 * @internal
 * A sycoca factory for service types (e.g. mimetypes)
 * It loads the service types from parsing directories (e.g. mimelnk/)
 * but can also create service types from data streams or single config files
 */
class TDEIO_EXPORT KServiceTypeFactory : public KSycocaFactory
{
  K_SYCOCAFACTORY( KST_KServiceTypeFactory )
public:
  /**
   * Create factory
   */
  KServiceTypeFactory();

  virtual ~KServiceTypeFactory();

  /**
   * Not meant to be called at this level
   */
  virtual KSycocaEntry *createEntry(const TQString &, const char *)
    { assert(0); return 0; }

  /**
   * Find a service type in the database file (allocates it)
   * Overloaded by KBuildServiceTypeFactory to return a memory one.
   */
  virtual KServiceType * findServiceTypeByName(const TQString &_name);

  /**
   * Find a the property type of a named property.
   */
  TQVariant::Type findPropertyTypeByName(const TQString &_name);

  /**
   * Find a mimetype from a filename (using the pattern list)
   * @param _filename filename to check.
   * @param match if provided, returns the pattern that matched.
   */
  KMimeType * findFromPattern(const TQString &_filename, TQString *match = 0);

  /**
   * @return all mimetypes
   * Slow and memory consuming, avoid using
   */
  KMimeType::List allMimeTypes();

  /**
   * @return all servicetypes
   * Slow and memory consuming, avoid using
   */
  KServiceType::List allServiceTypes();

  /**
   * @return true if at least one mimetype is present
   * Safety test
   */
  bool checkMimeTypes();

  /**
   * @return the unique servicetype factory, creating it if necessary
   */
  static KServiceTypeFactory * self();

protected:
  virtual KServiceType *createEntry(int offset);

private:
  static KServiceTypeFactory *_self;

protected:
  int m_fastPatternOffset;
  int m_otherPatternOffset;
  TQMap<TQString,int> m_propertyTypeDict;

private:
  TQStringList m_patterns;
  TQValueVector<TQ_INT32> m_pattern_offsets;
protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KServiceTypeFactoryPrivate* d;
};

#endif
