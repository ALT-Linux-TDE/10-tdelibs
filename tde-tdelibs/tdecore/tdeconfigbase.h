/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>
   Copyright (c) 2001 Waldo Bastian <bastian@kde.org>

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

#ifndef _TDECONFIGBASE_H
#define _TDECONFIGBASE_H

#include <tqobject.h>
#include <tqcolor.h>
#include <tqfont.h>
#include <tqdatetime.h>
#include <tqstrlist.h>
#include <tqstringlist.h>
#include <tqvariant.h>
#include <tqmap.h>

#include "tdeconfigdata.h"
#include "tdelibs_export.h"

class TDEConfigBackEnd;
class TDEConfigBasePrivate;
class TDEConfigGroup;

/**
 * @short KDE Configuration Management abstract base class
 *
 * This class forms the base for all %KDE configuration. It is an
 * abstract base class, meaning that you cannot directly instantiate
 * objects of this class. Either use TDEConfig (for usual %KDE
 * configuration) or KSimpleConfig (for special needs as in ksamba), or
 * even TDESharedConfig (stores values in shared memory).
 *
 * All configuration entries are key, value pairs.  Each entry also
 * belongs to a specific group of related entries.  All configuration
 * entries that do not explicitly specify which group they are in are
 * in a special group called the default group.
 *
 * If there is a $ character in an entry, TDEConfigBase tries to expand
 * environment variable and uses its value instead of its name. You
 * can avoid this feature by having two consecutive $ characters in
 * your config file which get expanded to one.
 *
 * \note the '=' char is not allowed in keys and the ']' char is not allowed in
 * a group name.
 *
 * @author Kalle Dalheimer <kalle@kde.org>, Preston Brown <pbrown@kde.org>
 * @see TDEGlobal#config()
 * @see TDEConfig
 * @see KSimpleConfig
 * @see TDESharedConfig
 */
class TDECORE_EXPORT TDEConfigBase : public TQObject
{
  TQ_OBJECT
  

  friend class TDEConfigBackEnd;
  friend class TDEConfigINIBackEnd;
  friend class TDEConfigGroup;

public:
  /**
   * Construct a TDEConfigBase object.
   */
  TDEConfigBase();

  /**
   * Destructs the TDEConfigBase object.
   */
  virtual ~TDEConfigBase();

  /**
   * Specifies the group in which keys will be read and written.
   *
   *  Subsequent
   * calls to readEntry() and writeEntry() will be applied only in the
   * activated group.
   *
   * Switch back to the default group by passing a null string.
   * @param group The name of the new group.
   */
  void setGroup( const TQString& group );

  /**
   * Sets the group to the "Desktop Entry" group used for
   * desktop configuration files for applications, mime types, etc.
   */
  void setDesktopGroup();

  /**
   * Returns the name of the group in which we are
   *  searching for keys and from which we are retrieving entries.
   *
   * @return The current group.
   */
  TQString group() const;

  /**
   * Returns true if the specified group is known about.
   *
   * @param group The group to search for.
   * @return true if the group exists.
   */
  bool hasGroup(const TQString &group) const;

  /**
   * Returns a list of groups that are known about.
   *
   * @return The list of groups.
   **/
  virtual TQStringList groupList() const = 0;

  /**
   * Returns a the current locale.
   *
   * @return A string representing the current locale.
   */
  TQString locale() const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * If you want to read a path, please use readPathEntry().
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be TQString::null if aDefault is null.
   */
   TQString readEntry(const TQString& pKey,
                     const TQString& aDefault = TQString::null ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be TQString::null if aDefault is null.
   */
   TQString readEntry(const char *pKey,
                     const TQString& aDefault = TQString::null ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the given type.
   *
   * Note that only the following TQVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, DateTime and Date.
   * @deprecated
   *
   * @param pKey The key to search for.
   * @return An invalid TQVariant if the key was not found or if the
   * read value cannot be converted to the given TQVariant::Type.
   */
  TQVariant readPropertyEntry( const TQString& pKey, TQVariant::Type ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the given type.
   *
   * Note that only the following TQVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, DateTime and Date.
   *
   * @deprecated
   *
   * @param pKey The key to search for.
   * @return An invalid TQVariant if the key was not found or if the
   * read value cannot be converted to the given TQVariant::Type.
   */
  TQVariant readPropertyEntry( const char *pKey, TQVariant::Type ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the type of the given default value.
   *
   * Note that only the following TQVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, DateTime and Date.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found or
   * if the read value cannot be converted to the TQVariant::Type.
   * @return The value for the key or the default value if the key was not
   *         found.
   */
  TQVariant readPropertyEntry( const TQString& pKey,
                              const TQVariant &aDefault) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The value is treated as if it is of the type of the given default value.
   *
   * Note that only the following TQVariant types are allowed : String,
   * StringList, List, Font, Point, Rect, Size, Color, Int, UInt, Bool,
   * Double, DateTime and Date.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found or
   * if the read value cannot be converted to the TQVariant::Type.
   * @return The value for the key or the default value if the key was not
   *         found.
   */
  TQVariant readPropertyEntry( const char *pKey,
                              const TQVariant &aDefault) const;

  /**
   * Reads a list of strings.
   *
   * @deprecated
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  int readListEntry( const TQString& pKey, TQStrList &list, char sep = ',' ) const;

  /**
   * Reads a list of strings.
   *
   * @deprecated
   *
   * @param pKey The key to search for
   * @param list In this object, the read list will be returned.
   * @param sep  The list separator (default ",")
   * @return The number of entries in the list.
   */
  int readListEntry( const char *pKey, TQStrList &list, char sep = ',' ) const;

  /**
   * Reads a list of strings.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   */
  TQStringList readListEntry( const TQString& pKey, char sep = ',' ) const;

  /**
   * Reads a list of strings.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   */
  TQStringList readListEntry( const char *pKey, char sep = ',' ) const;

  /**
   * Reads a list of strings, but returns a default if the key 
   * did not exist.
   * @param pKey The key to search for.
   * @param aDefault The default value to use if the key does not exist.
   * @param sep The list separator (default is ",").
   * @return The list. Contains @p aDefault if the Key does not exist.
   * @since 3.3
   */
  TQStringList readListEntry( const char* pKey, const TQStringList& aDefault, 
		  char sep = ',' ) const;

  /**
   * Reads a list of Integers.
   *
   * @param pKey The key to search for.
   * @return The list. Empty if the entry does not exist.
   */
  TQValueList<int> readIntListEntry( const TQString& pKey ) const;

  /**
   * Reads a list of Integers.
   *
   * @param pKey The key to search for.
   * @return The list. Empty if the entry does not exist.
   */
  TQValueList<int> readIntListEntry( const char *pKey ) const;

  /**
   * Reads a path.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a path. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be TQString::null if aDefault is null.
   */
  TQString readPathEntry( const TQString& pKey, const TQString & aDefault = TQString::null ) const;

  /**
   * Reads a path.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a path. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key. Can be TQString::null if aDefault is null.
   */
  TQString readPathEntry( const char *pKey, const TQString & aDefault = TQString::null ) const;

  /**
   * Reads a list of string paths.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a list of paths. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   * @since 3.1.3
   */
  TQStringList readPathListEntry( const TQString& pKey, char sep = ',' ) const;

  /**
   * Reads a list of string paths.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a list of paths. This means, dollar expansion is activated
   * for this value, so that e.g. $HOME gets expanded.
   *
   * @param pKey The key to search for.
   * @param sep  The list separator (default is ",").
   * @return The list. Empty if the entry does not exist.
   * @since 3.1.3
   */
  TQStringList readPathListEntry( const char *pKey, char sep = ',' ) const;


  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  int readNumEntry( const TQString& pKey, int nDefault = 0 ) const;

  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  int readNumEntry( const char *pKey, int nDefault = 0 ) const;

  /**
   * Reads an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  unsigned int readUnsignedNumEntry( const TQString& pKey, unsigned int nDefault = 0 ) const;

  /**
   * Reads an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  unsigned int readUnsignedNumEntry( const char *pKey, unsigned int nDefault = 0 ) const;


  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  long readLongNumEntry( const TQString& pKey, long nDefault = 0 ) const;

  /**
   * Reads a numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  long readLongNumEntry( const char *pKey, long nDefault = 0 ) const;

  /**
   * Read an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  unsigned long readUnsignedLongNumEntry( const TQString& pKey, unsigned long nDefault = 0 ) const;

  /**
   * Read an unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  unsigned long readUnsignedLongNumEntry( const char *pKey, unsigned long nDefault = 0 ) const;

  /**
   * Reads a 64-bit numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQ_INT64 readNum64Entry( const TQString& pKey, TQ_INT64 nDefault = 0 ) const;

  /**
   * Reads a 64-bit numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQ_INT64 readNum64Entry( const char *pKey, TQ_INT64 nDefault = 0 ) const;

  /**
   * Read an 64-bit unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQ_UINT64 readUnsignedNum64Entry( const TQString& pKey, TQ_UINT64 nDefault = 0 ) const;

  /**
   * Read an 64-bit unsigned numerical value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQ_UINT64 readUnsignedNum64Entry( const char *pKey, TQ_UINT64 nDefault = 0 ) const;

  /**
   * Reads a floating point value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  double readDoubleNumEntry( const TQString& pKey, double nDefault = 0.0 ) const;

  /**
   * Reads a floating point value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it numerically.
   *
   * @param pKey The key to search for.
   * @param nDefault A default value returned if the key was not found or if
   * the read value cannot be interpreted.
   * @return The value for this key.
   */
  double readDoubleNumEntry( const char *pKey, double nDefault = 0.0 ) const;

  /**
   * Reads a TQFont value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a font object.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null TQFont by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQFont readFontEntry( const TQString& pKey, const TQFont* pDefault = 0L ) const;

  /**
   * Reads a TQFont value.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a font object.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null TQFont by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQFont readFontEntry( const char *pKey, const TQFont* pDefault = 0L ) const;

  /**
   * Reads a boolean entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a boolean value. Currently "on", "yes", "1" and
   * "true" are accepted as true, everything else if false.
   *
   * @param pKey The key to search for
   * @param bDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
  bool readBoolEntry( const TQString& pKey, bool bDefault = false ) const;

  /**
   * Reads a boolean entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a boolean value. Currently "on", "yes", "1" and
   * "true" are accepted as true, everything else if false.
   *
   * @param pKey The key to search for
   * @param bDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
  bool readBoolEntry( const char *pKey, bool bDefault = false ) const;

  /**
   * Reads a TQRect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a TQRect object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null TQRect by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQRect readRectEntry( const TQString& pKey, const TQRect* pDefault = 0L ) const;

  /**
   * Reads a TQRect entry.
   *
   * Read the value of an entry specified by pKey in the current group
   * and interpret it as a TQRect object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null TQRect by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQRect readRectEntry( const char *pKey, const TQRect* pDefault = 0L ) const;

  /**
   * Reads a TQPoint entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a TQPoint object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null TQPoint by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQPoint readPointEntry( const TQString& pKey, const TQPoint* pDefault = 0L ) const;

  /**
   * Reads a TQPoint entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a TQPoint object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null TQPoint by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQPoint readPointEntry( const char *pKey, const TQPoint* pDefault = 0L ) const;

  /**
   * Reads a TQSize entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a TQSize object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null TQSize by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQSize readSizeEntry( const TQString& pKey, const TQSize* pDefault = 0L ) const;

  /**
   * Reads a TQSize entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a TQSize object.
   *
   * @param pKey The key to search for
   * @param pDefault A default value (null TQSize by default) returned if the
   * key was not found or if the read value cannot be interpreted.
   * @return The value for this key.
   */
  TQSize readSizeEntry( const char *pKey, const TQSize* pDefault = 0L ) const;


  /**
   * Reads a TQColor entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a color.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null TQColor by default) returned if the
   * key was not found or if the value cannot be interpreted.
   * @return The value for this key.
   */
  TQColor readColorEntry( const TQString& pKey, const TQColor* pDefault = 0L ) const;

  /**
   * Reads a TQColor entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a color.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value (null TQColor by default) returned if the
   * key was not found or if the value cannot be interpreted.
   * @return The value for this key.
   */
  TQColor readColorEntry( const char *pKey, const TQColor* pDefault = 0L ) const;

  /**
   * Reads a TQDateTime entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a date and time.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value ( currentDateTime() by default)
   * returned if the key was not found or if the read value cannot be
   * interpreted.
   * @return The value for this key.
   */
  TQDateTime readDateTimeEntry( const TQString& pKey, const TQDateTime* pDefault = 0L ) const;

  /**
   * Reads a TQDateTime entry.
   *
   * Read the value of an entry specified by @p pKey in the current group
   * and interpret it as a date and time.
   *
   * @param pKey The key to search for.
   * @param pDefault A default value ( currentDateTime() by default)
   * returned if the key was not found or if the read value cannot be
   * interpreted.
   * @return The value for this key.
   */
  TQDateTime readDateTimeEntry( const char *pKey, const TQDateTime* pDefault = 0L ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The untranslated entry is returned, you normally do not need this.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
   TQString readEntryUntranslated( const TQString& pKey,
                     const TQString& aDefault = TQString::null ) const;

  /**
   * Reads the value of an entry specified by @p pKey in the current group.
   * The untranslated entry is returned, you normally do not need this.
   *
   * @param pKey The key to search for.
   * @param aDefault A default value returned if the key was not found.
   * @return The value for this key.
   */
   TQString readEntryUntranslated( const char *pKey,
                     const TQString& aDefault = TQString::null ) const;

  /**
   * Writes a key/value pair.
   *
   * This is stored in the most specific config file when destroying the
   * config object or when calling sync().
   *
   * If you want to write a path, please use writePathEntry().
   *
   * @param pKey         The key to write.
   * @param pValue       The value to write.
   * @param bPersistent  If @p bPersistent is false, the entry's dirty
   *                     flag will not be set and thus the entry will
   *                     not be written to disk at deletion time.
   * @param bGlobal      If @p bGlobal is true, the pair is not saved to the
   *                     application specific config file, but to the
   *                     global KDE config file.
   * @param bNLS         If @p bNLS is true, the locale tag is added to the key
   *                     when writing it back.
   */
  void writeEntry( const TQString& pKey, const TQString& pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a key/value pair.
   *
   * This is stored in the most specific config file when destroying the
   * config object or when calling sync().
   *
   * @param pKey         The key to write.
   * @param pValue       The value to write.
   * @param bPersistent  If @p bPersistent is false, the entry's dirty
   *                     flag will not be set and thus the entry will
   *                     not be written to disk at deletion time.
   * @param bGlobal      If @p bGlobal is true, the pair is not saved to the
   *                     application specific config file, but to the
   *                     global KDE config file.
   * @param bNLS         If @p bNLS is true, the locale tag is added to the key
   *                     when writing it back.
   */
  void writeEntry( const char *pKey, const TQString& pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * writeEntry() Overridden to accept a property.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The property to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const TQString& pKey, const TQVariant& rValue,
                    bool bPersistent = true, bool bGlobal = false,
                    bool bNLS = false );
  /**
   * writeEntry() Overridden to accept a property.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The property to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const TQVariant& rValue,
                    bool bPersistent = true, bool bGlobal = false,
                    bool bNLS = false );

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param sep  The list separator (default is ",").
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const TQString& pKey, const TQStrList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );
  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param sep  The list separator (default is ",").
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const TQStrList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );

  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param sep  The list separator (default is ",").
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const TQString& pKey, const TQStringList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );
  /**
   * writeEntry() overridden to accept a list of strings.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param sep  The list separator (default is ",").
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const TQStringList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );


 /**
   * writeEntry() overridden to accept a list of Integers.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const TQString& pKey, const TQValueList<int>& rValue,
		   bool bPersistent = true, bool bGlobal = false, bool bNLS = false );
 /**
   * writeEntry() overridden to accept a list of Integers.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writeEntry()
   */
  void writeEntry( const char *pKey, const TQValueList<int>& rValue,
		   bool bPersistent = true, bool bGlobal = false, bool bNLS = false );

  /**
   * Write a (key/value) pair.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling sync().
   *
   *  @param pKey               The key to write.
   *  @param pValue     The value to write.
   *  @param bPersistent        If @p bPersistent is false, the entry's dirty
   *                    flag will not be set and thus the entry will
   *                    not be written to disk at deletion time.
   *  @param bGlobal    If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   *  @param bNLS       If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const TQString& pKey, const char *pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false )
    { writeEntry(pKey, TQString::fromLatin1(pValue), bPersistent, bGlobal, bNLS); }
  /**
   * Write a (key/value) pair.
   *
   * This is stored to the most specific config file when destroying the
   * config object or when calling sync().
   *
   *  @param pKey               The key to write.
   *  @param pValue     The value to write.
   *  @param bPersistent        If @p bPersistent is false, the entry's dirty
   *                    flag will not be set and thus the entry will
   *                    not be written to disk at deletion time.
   *  @param bGlobal    If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   *  @param bNLS       If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const char *pKey, const char *pValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false )
    { writeEntry(pKey, TQString::fromLatin1(pValue), bPersistent, bGlobal, bNLS); }

  /**
   * Write a (key/value) pair.
   * Same as above, but writes a numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const TQString& pKey, int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Write a (key/value) pair.
   * Same as above, but writes a numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const char *pKey, int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const TQString& pKey, unsigned int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *                    application specific config file, but to the
   *                    global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *                    when writing it back.
   */
  void writeEntry( const char *pKey, unsigned int nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but write a long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but write a long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, unsigned long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, unsigned long nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but write a 64-bit numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, TQ_INT64 nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but write a long numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, TQ_INT64 nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned 64-bit numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, TQ_UINT64 nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes an unsigned 64-bit numerical value.
   *
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, TQ_UINT64 nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a floating-point value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param format      @p format determines the format to which the value
   *  is converted. Default is 'g'.
   * @param precision   @p precision sets the precision with which the
   *  value is converted. Default is 6 as in TQString.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, double nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      char format = 'g', int precision = 6,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a floating-point value.
   * @param pKey The key to write.
   * @param nValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param format      @p format determines the format to which the value
   *  is converted. Default is 'g'.
   * @param precision   @p precision sets the precision with which the
   *  value is converted. Default is 6 as in TQString.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, double nValue,
                      bool bPersistent = true, bool bGlobal = false,
                      char format = 'g', int precision = 6,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a boolean value.
   *
   * @param pKey The key to write.
   * @param bValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, bool bValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a boolean value.
   *
   * @param pKey The key to write.
   * @param bValue The value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, bool bValue,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a font value.
   *
   * @param pKey The key to write.
   * @param rFont The font value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, const TQFont& rFont,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a font value.
   *
   * @param pKey The key to write.
   * @param rFont The font value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const TQFont& rFont,
                      bool bPersistent = true, bool bGlobal = false,
                      bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but write a color entry.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rColor The color value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, const TQColor& rColor,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but write a color entry.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rColor The color value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const TQColor& rColor,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a date and time entry.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * @em not returned here!
   *
   * @param pKey The key to write.
   * @param rDateTime The date and time value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, const TQDateTime& rDateTime,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a date and time entry.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * @em not returned here!
   *
   * @param pKey The key to write.
   * @param rDateTime The date and time value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const TQDateTime& rDateTime,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );


  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a rectangle.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The rectangle value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, const TQRect& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a rectangle.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The rectangle value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const TQRect& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a point.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The point value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, const TQPoint& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a point.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The point value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const TQPoint& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a size.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The size value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const TQString& pKey, const TQSize& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );
  /**
   * Writes a (key/value) pair.
   * Same as above, but writes a size.
   *
   * Note: Unlike the other writeEntry() functions, the old value is
   * _not_ returned here!
   *
   * @param pKey The key to write.
   * @param rValue The size value to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writeEntry( const char *pKey, const TQSize& rValue,
                   bool bPersistent = true, bool bGlobal = false,
                   bool bNLS = false );

  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writePathEntry( const TQString& pKey, const TQString & path,
                       bool bPersistent = true, bool bGlobal = false,
                       bool bNLS = false );
  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   */
  void writePathEntry( const char *pKey, const TQString & path,
                       bool bPersistent = true, bool bGlobal = false,
                       bool bNLS = false );

  /**
   * Writes a file path.
   *
   * It is checked whether the path is located under $HOME. If so the
   * path is written out with the user's home-directory replaced with
   * $HOME. The path should be read back with readPathEntry()
   *
   * @param pKey The key to write.
   * @param path The path to write.
   * @param bPersistent If @p bPersistent is false, the entry's dirty
   * flag will not be set and thus the entry will not be written to
   * disk at deletion time.
   * @param bGlobal     If @p bGlobal is true, the pair is not saved to the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the locale tag is added to the key
   *  when writing it back.
   * @param expand      If @p expand is true, environment variables will be
   * expanded on read.
   */
  void writePathEntry( const char *pKey, const TQString & path,
                       bool bPersistent, bool bGlobal,
                       bool bNLS, bool expand );

  /**
   * writePathEntry() overridden to accept a list of paths (strings).
   *
   * It is checked whether the paths are located under $HOME. If so each of
   * the paths are written out with the user's home-directory replaced with
   * $HOME. The paths should be read back with readPathListEntry()
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param sep  The list separator (default is ",").
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writePathEntry()
   * @see  readPathListEntry()
   * @since 3.1.3
   */
  void writePathEntry( const TQString& pKey, const TQStringList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );
  /**
   * writePathEntry() overridden to accept a list of paths (strings).
   *
   * It is checked whether the paths are located under $HOME. If so each of
   * the paths are written out with the user's home-directory replaced with
   * $HOME. The paths should be read back with readPathListEntry()
   *
   * @param pKey The key to write
   * @param rValue The list to write
   * @param sep  The list separator (default is ",").
   * @param bPersistent If @p bPersistent is false, the entry's dirty flag
   *                    will not be set and thus the entry will not be
   *                    written to disk at deletion time.
   * @param bGlobal If @p bGlobal is true, the pair is not saved to the
   *                application specific config file, but to the
   *                global KDE config file.
   * @param bNLS If @p bNLS is true, the locale tag is added to the key
   *             when writing it back.
   *
   * @see  writePathEntry()
   * @see  readPathListEntry()
   * @since 3.1.3
   */
  void writePathEntry( const char *pKey, const TQStringList &rValue,
		   char sep = ',', bool bPersistent = true, bool bGlobal = false, bool bNLS = false );


  /**
   * Deletes the entry specified by @p pKey in the current group.
   *
   * @param pKey The key to delete.
   * @param bGlobal     If @p bGlobal is true, the pair is not removed from the
   *  application specific config file, but to the global KDE config file.
   * @param bNLS        If @p bNLS is true, the key with the locale tag is removed.
   */
   void deleteEntry( const TQString& pKey,
                   bool bNLS = false, bool bGlobal = false);
  /**
   * Deletes the entry specified by @p pKey in the current group.
   *
   * @param pKey The key to delete.
   * @param bGlobal     If @p bGlobal is true, the pair is not removed from the
   *  application specific config file, but from the global KDE config file.
   * @param bNLS        If @p bNLS is true, the key with the locale tag is removed.
   */
   void deleteEntry( const char *pKey,
                   bool bNLS = false, bool bGlobal = false);

  /**
   * Deletes a configuration entry group
   *
   * If the group is not empty and bDeep is false, nothing gets
   * deleted and false is returned.
   * If this group is the current group and it is deleted, the
   * current group is undefined and should be set with setGroup()
   * before the next operation on the configuration object.
   *
   * @param group The name of the group
   * @param bDeep Specify whether non-empty groups should be completely
   *        deleted (including their entries).
   * @param bGlobal     If @p bGlobal is true, the group is not removed from the
   *  application specific config file, but from the global KDE config file.
   * @return If the group is not empty and bDeep is false,
   *         deleteGroup returns false.
   */
  bool deleteGroup( const TQString& group, bool bDeep = true, bool bGlobal = false );


  /**
   * Turns on or off "dollar  expansion" (see TDEConfigBase introduction)
   *  when reading config entries.
   * Dollar sign expansion is initially OFF.
   *
   * @param _bExpand Tf true, dollar expansion is turned on.
   */
  void setDollarExpansion( bool _bExpand = true ) { bExpand = _bExpand; }

  /**
   * Returns whether dollar expansion is on or off.  It is initially OFF.
   *
   * @return true if dollar expansion is on.
   */
  bool isDollarExpansion() const { return bExpand; }

  /**
   * Mark the config object as "clean," i.e. don't write dirty entries
   * at destruction time. If @p bDeep is false, only the global dirty
   * flag of the TDEConfig object gets cleared. If you then call
   * writeEntry() again, the global dirty flag is set again and all
   * dirty entries will be written at a subsequent sync() call.
   *
   * Classes that derive from TDEConfigBase should override this
   * method and implement storage-specific behavior, as well as
   * calling the TDEConfigBase::rollback() explicitly in the initializer.
   *
   * @param bDeep If true, the dirty flags of all entries are cleared,
   *        as well as the global dirty flag.
   */
  virtual void rollback( bool bDeep = true );

  /**
   * Flushes all changes that currently reside only in memory
   * back to disk / permanent storage. Dirty configuration entries are
   * written to the most specific file available.
   *
   * Asks the back end to flush out all pending writes, and then calls
   * rollback().  No changes are made if the object has @p readOnly
   * status.
   *
   * You should call this from your destructor in derivative classes.
   *
   * @see rollback(), #isReadOnly()
   */
  virtual void sync();

  /**
   * Checks whether the config file has any dirty (modified) entries.
   * @return true if the config file has any dirty (modified) entries.
   */
  bool isDirty() const { return bDirty; }

  /**
   * Sets the config object's read-only status.
   *
   * @param _ro If true, the config object will not write out any
   *        changes to disk even if it is destroyed or sync() is called.
   *
   */
  virtual void setReadOnly(bool _ro) { bReadOnly = _ro; }

   /**
    * Returns the read-only status of the config object.
    *
    * @return The read-only status.
    */
  bool isReadOnly() const { return bReadOnly; }

  /**
   * Checks whether the key has an entry in the currently active group.
   * Use this to determine whether a key is not specified for the current
   * group (hasKey() returns false). Keys with null data are considered
   * nonexistent.
   *
   * @param key The key to search for.
   * @return If true, the key is available.
   */
  bool hasKey( const TQString& key ) const;

  /**
   * Returns a map (tree) of entries for all entries in a particular
   * group.  Only the actual entry string is returned, none of the
   * other internal data should be included.
   *
   * @param group A group to get keys from.
   * @return A map of entries in the group specified, indexed by key.
   *         The returned map may be empty if the group is not found.
   * @see   QMap
   */
  virtual TQMap<TQString, TQString> entryMap(const TQString &group) const = 0;

  /**
   * Reparses all configuration files. This is useful for programs
   * that use stand alone graphical configuration tools. The base
   * method implemented here only clears the group list and then
   * appends the default group.
   *
   * Derivative classes should clear any internal data structures and
   * then simply call parseConfigFiles() when implementing this
   * method.
   *
   * @see  parseConfigFiles()
   */
  virtual void reparseConfiguration() = 0;

  /**
   * Checks whether this configuration file can be modified.
   * @return whether changes may be made to this configuration file.
   */
  bool isImmutable() const;

  /**
   * Checks whether it is possible to change the given group.
   * @param group the group to check
   * @return whether changes may be made to @p group in this configuration
   * file.
   */
  bool groupIsImmutable(const TQString &group) const;

  /**
   * Checks whether it is possible to change the given entry.
   * @param key the key to check
   * @return whether the entry @p key may be changed in the current group
   * in this configuration file.
   */
  bool entryIsImmutable(const TQString &key) const;

  /**
   * Possible return values for getConfigState().
   *
   * @see  getConfigState()
   */
  enum ConfigState { NoAccess, ReadOnly, ReadWrite };

  /**
   * Returns the state of the app-config object.
   *
   * Possible return values
   * are NoAccess (the application-specific config file could not be
   * opened neither read-write nor read-only), ReadOnly (the
   * application-specific config file is opened read-only, but not
   * read-write) and ReadWrite (the application-specific config
   * file is opened read-write).
   *
   * @see  ConfigState()
   * @return the state of the app-config object
   */
  ConfigState getConfigState() const;

  /**
   * Check whether the config files are writable.
   * @param warnUser Warn the user if the configuration files are not writable.
   * @return Indicates that all of the configuration files used are writable.
   * @since 3.2
   */
  bool checkConfigFilesWritable(bool warnUser);

  /**
   * When set, all readEntry and readXXXEntry calls return the system
   * wide (default) values instead of the user's preference.
   * This is off by default.
   * @since 3.2
   */
  void setReadDefaults(bool b);

  /**
   * @returns true if all readEntry and readXXXEntry calls return the system
   * wide (default) values instead of the user's preference.
   * @since 3.2
   */
  bool readDefaults() const;

  /**
   * Reverts the entry with key @p key in the current group in the
   * application specific config file to either the system wide (default)
   * value or the value specified in the global KDE config file.
   *
   * To revert entries in the global KDE config file, the global KDE config
   * file should be opened explicitly in a separate config object.
   *
   * @param key The key of the entry to revert.
   * @since 3.2
   */
  void revertToDefault(const TQString &key);

  /**
   * Returns whether a default is specified for an entry in either the
   * system wide configuration file or the global KDE config file.
   *
   * If an application computes a default value at runtime for
   * a certain entry, e.g. like:
   * \code
   * TQColor computedDefault = kapp->palette().color(TQPalette::Active, TQColorGroup::Text)
   * TQColor color = config->readEntry(key, computedDefault);
   * \encode
   *
   * Then it may wish to make the following check before
   * writing back changes:
   * \code
   * if ( (value == computedDefault) && !config->hasDefault(key) )
   *    config->revertToDefault(key)
   * else
   *    config->writeEntry(key, value)
   * \endcode
   *
   * This ensures that as long as the entry is not modified to differ from 
   * the computed default, the application will keep using the computed default
   * and will follow changes the computed default makes over time.
   * @param key The key of the entry to check.
   * @since 3.2
   */
  bool hasDefault(const TQString &key) const;

protected:
  /**
   * Reads the locale and put in the configuration data struct.
   * Note that this should be done in the constructor, but this is not
   * possible due to some mutual dependencies in TDEApplication::init()
   */
  void setLocale();

  /**
   * Sets the global dirty flag of the config object
   *
   * @param _bDirty How to mark the object's dirty status
   */
  virtual void setDirty(bool _bDirty = true) { bDirty = _bDirty; }

  /**
   * Parses all configuration files for a configuration object.
   *
   * The actual parsing is done by the associated TDEConfigBackEnd.
   */
  virtual void parseConfigFiles();

  /**
   * Returns a map (tree) of the entries in the specified group.
   * This may or may not return all entries that belong to the
   * config object.  The only guarantee that you are given is that
   * any entries that are dirty (i.e. modified and not yet written back
   * to the disk) will be contained in the map.  Some derivative
   * classes may choose to return everything.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param pGroup The group to provide a KEntryMap for.
   * @return The map of the entries in the group.
   * @internal
   */
  virtual KEntryMap internalEntryMap( const TQString& pGroup ) const = 0;

  /**
   * Returns a map (tree) of the entries in the tree.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @return A map of the entries in the tree.
   *
   * @internal
   *
   */
  virtual KEntryMap internalEntryMap() const = 0;

  /**
   * Inserts a (key/value) pair into the internal storage mechanism of
   * the configuration object. Classes that derive from TDEConfigBase
   * will need to implement this method in a storage-specific manner.
   *
   * Do not use this function, the implementation / return type are
   * subject to change.
   *
   * @param _key The key to insert.  It contains information both on
   *        the group of the key and the key itself. If the key already
   *        exists, the old value will be replaced.
   * @param _data the KEntry that is to be stored.
   * @param _checkGroup When false, assume that the group already exists.
   * @internal
   */
  virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup = true) = 0;

  /**
   * Looks up an entry in the config object's internal structure.
   * Classes that derive from TDEConfigBase will need to implement this
   * method in a storage-specific manner.
   *
   * Do not use this function, the implementation and return type are
   * subject to change.
   *
   * @param _key The key to look up  It contains information both on
   *        the group of the key and the entry's key itself.
   * @return The KEntry value (data) found for the key.  @p KEntry.aValue
   * will be the null string if nothing was located.
   * @internal
   */
  virtual KEntry lookupData(const KEntryKey &_key) const = 0;

  virtual bool internalHasGroup(const TQCString &group) const = 0;

  /**
   * A back end for loading/saving to disk in a particular format.
   */
  TDEConfigBackEnd *backEnd;
public:
  /**
   * Overloaded public methods:
   */
  void setGroup( const TQCString &pGroup );
  void setGroup( const char *pGroup );
  bool hasGroup(const TQCString &_pGroup) const;
  bool hasGroup(const char *_pGroup) const;
  bool hasKey( const char *pKey ) const;

protected:
  TQCString readEntryUtf8( const char *pKey) const;
  bool hasTranslatedKey( const char *pKey ) const;

  /**
   * The currently selected group. */
  TQCString mGroup;

  /**
   * The locale to retrieve keys under if possible, i.e en_US or fr.  */
  TQCString aLocaleString;

  /**
   * Indicates whether there are any dirty entries in the config object
   * that need to be written back to disk. */
  bool bDirty;

  bool bLocaleInitialized;
  bool bReadOnly;           // currently only used by KSimpleConfig
  mutable bool bExpand;     // whether dollar expansion is used

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class TDEConfigBasePrivate;
  TDEConfigBasePrivate *d;

  void writeEntry( const char *pKey, const TQString &rValue,
    bool bPersistent, bool bGlobal, bool bNLS, bool bExpand );
  void writeEntry( const char *pKey, const TQStringList &rValue,
    char sep, bool bPersistent, bool bGlobal, bool bNLS, bool bExpand );

};

class TDEConfigGroupSaverPrivate;

/**
  * Helper class to facilitate working with TDEConfig / KSimpleConfig
  * groups.
  *
  * Careful programmers always set the group of a
  * TDEConfig KSimpleConfig object to the group they want to read from
  * and set it back to the old one of afterwards. This is usually
  * written as:
  * \code
  *
  * TQString oldgroup config->group();
  * config->setGroup( "TheGroupThatIWant" );
  * ...
  * config->writeEntry( "Blah", "Blubb" );
  *
  * config->setGroup( oldgroup );
  * \endcode
  *
  * In order to facilitate this task, you can use
  * TDEConfigGroupSaver. Simply construct such an object ON THE STACK
  * when you want to switch to a new group. Then, when the object goes
  * out of scope, the group will automatically be restored. If you
  * want to use several different groups within a function or method,
  * you can still use TDEConfigGroupSaver: Simply enclose all work with
  * one group (including the creation of the TDEConfigGroupSaver object)
  * in one block.
  *
  * @deprecated This class is deprecated and will be removed in KDE 4.
  * TDEConfigGroup provides similar functionality in a more object oriented
  * way.
  *
  * @author Matthias Kalle Dalheimer <kalle@kde.org>
  * @see TDEConfigBase, TDEConfig, KSimpleConfig, TDEConfigGroup
  * @short Helper class for easier use of TDEConfig/KSimpleConfig groups
  */

class TDECORE_EXPORT TDEConfigGroupSaver // KDE4 remove
{
public:
  /**
   * Constructor. You pass a pointer to the TDEConfigBase-derived
   * object you want to work with and a string indicating the _new_
   * group.
   *
   * @param config The TDEConfigBase-derived object this
   *               TDEConfigGroupSaver works on.
   * @param group  The new group that the config object should switch to.
   */
  TDEConfigGroupSaver( TDEConfigBase* config, TQString group )
      /* KDE 4 : make the second parameter const TQString & */
      : _config(config), _oldgroup(config->group())
        { _config->setGroup( group ); }

  TDEConfigGroupSaver( TDEConfigBase* config, const char *group )
      : _config(config), _oldgroup(config->group())
        { _config->setGroup( group ); }

  TDEConfigGroupSaver( TDEConfigBase* config, const TQCString &group )
      : _config(config), _oldgroup(config->group())
        { _config->setGroup( group ); }

  ~TDEConfigGroupSaver() { _config->setGroup( _oldgroup ); }

    TDEConfigBase* config() { return _config; };

private:
  TDEConfigBase* _config;
  TQString _oldgroup;

  TDEConfigGroupSaver(const TDEConfigGroupSaver&);
  TDEConfigGroupSaver& operator=(const TDEConfigGroupSaver&);

  TDEConfigGroupSaverPrivate *d;
};

class TDEConfigGroupPrivate;

/**
 * A TDEConfigBase derived class for one specific group in a TDEConfig object.
 */
class TDECORE_EXPORT TDEConfigGroup: public TDEConfigBase
{
public:
   /**
    * Construct a config group corresponding to @p group in @p master.
    * @p group is the group name encoded in UTF-8.
    */
   TDEConfigGroup(TDEConfigBase *master, const TQCString &group);
   /**
    * This is an overloaded constructor provided for convenience.
    * It behaves essentially like the above function.
    *
    * Construct a config group corresponding to @p group in @p master
    */
   TDEConfigGroup(TDEConfigBase *master, const TQString &group);
   /**
    * This is an overloaded constructor provided for convenience.
    * It behaves essentially like the above function.
    *
    * Construct a config group corresponding to @p group in @p master
    * @p group is the group name encoded in UTF-8.
    */
   TDEConfigGroup(TDEConfigBase *master, const char * group);

   /**
    * Delete all entries in the entire group
    * @param bGlobal     If @p bGlobal is true, the entries are not removed
    *        from the application specific config file, but from the global
    *        KDE config file.
    */
   void deleteGroup(bool bGlobal = false);

   /**
   * Checks whether it is possible to change this group.
   * @return whether changes may be made to this group in this configuration
   * file.
   * @since 3.4
   */
  bool groupIsImmutable() const;

   // The following functions are reimplemented:
   virtual void setDirty(bool _bDirty);
   virtual void putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup = true);
   virtual KEntry lookupData(const KEntryKey &_key) const;
   virtual void sync();

private:
   // Hide the following members:
   void setGroup() { }
   void setDesktopGroup() { }
   void group() { }
   void hasGroup() { }
   void setReadOnly(bool) { }
   void isDirty() { }

   // The following members are not used.
   virtual TQStringList groupList() const { return TQStringList(); }
   virtual void rollback(bool) { }
   virtual void reparseConfiguration() { }
   virtual TQMap<TQString, TQString> entryMap(const TQString &) const
    { return TQMap<TQString,TQString>(); }
   virtual KEntryMap internalEntryMap( const TQString&) const
    { return KEntryMap(); }
   virtual KEntryMap internalEntryMap() const
    { return KEntryMap(); }
   virtual bool internalHasGroup(const TQCString &) const
    { return false; }

   void getConfigState() { }

   TDEConfigBase *mMaster;
protected:
   virtual void virtual_hook( int id, void* data );
private:
   TDEConfigGroupPrivate* d;
};

#endif
