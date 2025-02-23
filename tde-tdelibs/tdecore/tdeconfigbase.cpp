/*
   This file is part of the KDE libraries
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>
   Copyright (c) 1997 Matthias Kalle Dalheimer <kalle@kde.org>

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

#include <stdlib.h>
#include <string.h>

#include <tqfile.h>
#include <tqdir.h>
#include <tqtextstream.h>

#include <tdeapplication.h>
#include <tdeglobalsettings.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kcharsets.h>

#include "tdeconfigbase.h"
#include "tdeconfigbackend.h"
#include "kdebug.h"
#include "kstandarddirs.h"
#include "kstringhandler.h"

class TDEConfigBase::TDEConfigBasePrivate
{
public:
     TDEConfigBasePrivate() : readDefaults(false) { };

public:
     bool readDefaults;
};

TDEConfigBase::TDEConfigBase()
  : backEnd(0L), bDirty(false), bLocaleInitialized(false),
    bReadOnly(false), bExpand(false), d(0)
{
    setGroup(TQString::null);
}

TDEConfigBase::~TDEConfigBase()
{
    delete d;
}

void TDEConfigBase::setLocale()
{
  bLocaleInitialized = true;

  if (TDEGlobal::locale())
    aLocaleString = TDEGlobal::locale()->language().utf8();
  else
    aLocaleString = TDELocale::defaultLanguage().utf8();
  if (backEnd)
     backEnd->setLocaleString(aLocaleString);
}

TQString TDEConfigBase::locale() const
{
  return TQString::fromUtf8(aLocaleString);
}

void TDEConfigBase::setGroup( const TQString& group )
{
  if ( group.isEmpty() )
    mGroup = "<default>";
  else
    mGroup = group.utf8();
}

void TDEConfigBase::setGroup( const char *pGroup )
{
  setGroup(TQCString(pGroup));
}

void TDEConfigBase::setGroup( const TQCString &group )
{
  if ( group.isEmpty() )
    mGroup = "<default>";
  else
    mGroup = group;
}

TQString TDEConfigBase::group() const {
  return TQString::fromUtf8(mGroup);
}

void TDEConfigBase::setDesktopGroup()
{
  mGroup = "Desktop Entry";
}

bool TDEConfigBase::hasKey(const TQString &key) const
{
   return hasKey(key.utf8().data());
}

bool TDEConfigBase::hasKey(const char *pKey) const
{
  KEntryKey aEntryKey(mGroup, 0);
  aEntryKey.c_key = pKey;
  aEntryKey.bDefault = readDefaults();

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (!entry.mValue.isNull())
       return true;
    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  return !entry.mValue.isNull();
}

bool TDEConfigBase::hasTranslatedKey(const char* pKey) const
{
  KEntryKey aEntryKey(mGroup, 0);
  aEntryKey.c_key = pKey;
  aEntryKey.bDefault = readDefaults();

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (!entry.mValue.isNull())
       return true;
    aEntryKey.bLocal = false;
  }

  return false;
}

bool TDEConfigBase::hasGroup(const TQString &group) const
{
  return internalHasGroup( group.utf8());
}

bool TDEConfigBase::hasGroup(const char *_pGroup) const
{
  return internalHasGroup( TQCString(_pGroup));
}

bool TDEConfigBase::hasGroup(const TQCString &_pGroup) const
{
  return internalHasGroup( _pGroup);
}

bool TDEConfigBase::isImmutable() const
{
  return (getConfigState() != ReadWrite);
}

bool TDEConfigBase::groupIsImmutable(const TQString &group) const
{
  if (getConfigState() != ReadWrite)
     return true;

  KEntryKey groupKey(group.utf8(), 0);
  KEntry entry = lookupData(groupKey);
  return entry.bImmutable;
}

bool TDEConfigBase::entryIsImmutable(const TQString &key) const
{
  if (getConfigState() != ReadWrite)
     return true;

  KEntryKey entryKey(mGroup, 0);
  KEntry aEntryData = lookupData(entryKey); // Group
  if (aEntryData.bImmutable)
    return true;

  TQCString utf8_key = key.utf8();
  entryKey.c_key = utf8_key.data();
  aEntryData = lookupData(entryKey); // Normal entry
  if (aEntryData.bImmutable)
    return true;

  entryKey.bLocal = true;
  aEntryData = lookupData(entryKey); // Localized entry
  return aEntryData.bImmutable;
}


TQString TDEConfigBase::readEntryUntranslated( const TQString& pKey,
                                const TQString& aDefault ) const
{
   return TDEConfigBase::readEntryUntranslated(pKey.utf8().data(), aDefault);
}


TQString TDEConfigBase::readEntryUntranslated( const char *pKey,
                                const TQString& aDefault ) const
{
   TQCString result = readEntryUtf8(pKey);
   if (result.isNull())
      return aDefault;
   return TQString::fromUtf8(result);
}


TQString TDEConfigBase::readEntry( const TQString& pKey,
                                const TQString& aDefault ) const
{
   return TDEConfigBase::readEntry(pKey.utf8().data(), aDefault);
}

TQString TDEConfigBase::readEntry( const char *pKey,
                                const TQString& aDefault ) const
{
  // we need to access _locale instead of the method locale()
  // because calling locale() will create a locale object if it
  // doesn't exist, which requires TDEConfig, which will create a infinite
  // loop, and nobody likes those.
  if (!bLocaleInitialized && TDEGlobal::_locale) {
    // get around const'ness.
    TDEConfigBase *that = const_cast<TDEConfigBase *>(this);
    that->setLocale();
  }

  TQString aValue;

  bool expand = false;
  // construct a localized version of the key
  // try the localized key first
  KEntry aEntryData;
  KEntryKey entryKey(mGroup, 0);
  entryKey.c_key = pKey;
  entryKey.bDefault = readDefaults();
  entryKey.bLocal = true;
  aEntryData = lookupData(entryKey);
  if (!aEntryData.mValue.isNull()) {
    // for GNOME .desktop
    aValue = KStringHandler::from8Bit( aEntryData.mValue.data() );
    expand = aEntryData.bExpand;
  } else {
    entryKey.bLocal = false;
    aEntryData = lookupData(entryKey);
    if (!aEntryData.mValue.isNull()) {
      aValue = TQString::fromUtf8(aEntryData.mValue.data());
      if (aValue.isNull())
      {
        static const TQString &emptyString = TDEGlobal::staticQString("");
        aValue = emptyString;
      }
      expand = aEntryData.bExpand;
    } else {
      aValue = aDefault;
    }
  }

  // only do dollar expansion if so desired
  if( expand || bExpand )
    {
      // check for environment variables and make necessary translations
      int nDollarPos = aValue.find( '$' );

      while( nDollarPos != -1 && (nDollarPos + 1) < static_cast<int>(aValue.length())) {
        // there is at least one $
        if( aValue[nDollarPos+1] != '$' ) {
          uint nEndPos = nDollarPos+1;
          // the next character is no $
          TQString aVarName;
          if (aValue[nEndPos]=='{')
          {
            while ( (nEndPos <= aValue.length()) && (aValue[nEndPos]!='}') )
                nEndPos++;
            nEndPos++;
            aVarName = aValue.mid( nDollarPos+2, nEndPos-nDollarPos-3 );
          }
          else
          {
            while ( nEndPos <= aValue.length() && (aValue[nEndPos].isNumber()
                    || aValue[nEndPos].isLetter() || aValue[nEndPos]=='_' )  )
                nEndPos++;
            aVarName = aValue.mid( nDollarPos+1, nEndPos-nDollarPos-1 );
          }
          const char *pEnv = 0;
          if (!aVarName.isEmpty())
               pEnv = getenv( aVarName.ascii() );
          if (pEnv) {
	    // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
	    // A environment variables may contain values in 8bit
	    // locale cpecified encoding or in UTF8 encoding.
	    aValue.replace( nDollarPos, nEndPos-nDollarPos, KStringHandler::from8Bit( pEnv ) );
          }
          else if (aVarName.length() > 8 && aVarName.startsWith("XDG_") && aVarName.endsWith("_DIR")) {
            TQString result;
            if (aVarName == "XDG_DESKTOP_DIR") {
              result = TDEGlobalSettings::desktopPath();
            }
            else if (aVarName == "XDG_DOCUMENTS_DIR") {
              result = TDEGlobalSettings::documentPath();
            }
            else if (aVarName == "XDG_DOWNLOAD_DIR") {
              result = TDEGlobalSettings::downloadPath();
            }
            else if (aVarName == "XDG_MUSIC_DIR") {
              result = TDEGlobalSettings::musicPath();
            }
            else if (aVarName == "XDG_PICTURES_DIR") {
              result = TDEGlobalSettings::picturesPath();
            }
            else if (aVarName == "XDG_PUBLICSHARE_DIR") {
              result = TDEGlobalSettings::publicSharePath();
            }
            else if (aVarName == "XDG_TEMPLATES_DIR") {
              result = TDEGlobalSettings::templatesPath();
            }
            else if (aVarName == "XDG_VIDEOS_DIR") {
              result = TDEGlobalSettings::videosPath();
            }
            aValue.replace( nDollarPos, nEndPos-nDollarPos, result );
          }
          else {
            aValue.remove( nDollarPos, nEndPos-nDollarPos );
          }
        }
        else {
          // remove one of the dollar signs
          aValue.remove( nDollarPos, 1 );
          nDollarPos++;
        }
        nDollarPos = aValue.find( '$', nDollarPos );
      }
    }

  return aValue;
}

TQCString TDEConfigBase::readEntryUtf8( const char *pKey) const
{
  // We don't try the localized key
  KEntryKey entryKey(mGroup, 0);
  entryKey.bDefault = readDefaults();
  entryKey.c_key = pKey;
  KEntry aEntryData = lookupData(entryKey);
  if (aEntryData.bExpand)
  {
     // We need to do fancy, take the slow route.
     return readEntry(pKey, TQString::null).utf8();
  }
  return aEntryData.mValue;
}

TQVariant TDEConfigBase::readPropertyEntry( const TQString& pKey,
                                          TQVariant::Type type ) const
{
  return readPropertyEntry(pKey.utf8().data(), type);
}

TQVariant TDEConfigBase::readPropertyEntry( const char *pKey,
                                          TQVariant::Type type ) const
{
  TQVariant va;
  if ( !hasKey( pKey ) ) return va;
  (void)va.cast(type);
  return readPropertyEntry(pKey, va);
}

TQVariant TDEConfigBase::readPropertyEntry( const TQString& pKey,
                                         const TQVariant &aDefault ) const
{
  return readPropertyEntry(pKey.utf8().data(), aDefault);
}

TQVariant TDEConfigBase::readPropertyEntry( const char *pKey,
                                         const TQVariant &aDefault ) const
{
  if ( !hasKey( pKey ) ) return aDefault;

  TQVariant tmp = aDefault;

  switch( aDefault.type() )
  {
      case TQVariant::Invalid:
          return TQVariant();
      case TQVariant::String:
          return TQVariant( readEntry( pKey, aDefault.toString() ) );
      case TQVariant::StringList:
          return TQVariant( readListEntry( pKey ) );
      case TQVariant::List: {
          TQStringList strList = readListEntry( pKey );
          TQStringList::ConstIterator it = strList.begin();
          TQStringList::ConstIterator end = strList.end();
          TQValueList<TQVariant> list;

          for (; it != end; ++it ) {
              tmp = *it;
              list.append( tmp );
          }
          return TQVariant( list );
      }
      case TQVariant::Font:
          return TQVariant( readFontEntry( pKey, &tmp.asFont() ) );
      case TQVariant::Point:
          return TQVariant( readPointEntry( pKey, &tmp.asPoint() ) );
      case TQVariant::Rect:
          return TQVariant( readRectEntry( pKey, &tmp.asRect() ) );
      case TQVariant::Size:
          return TQVariant( readSizeEntry( pKey, &tmp.asSize() ) );
      case TQVariant::Color:
          return TQVariant( readColorEntry( pKey, &tmp.asColor() ) );
      case TQVariant::Int:
          return TQVariant( readNumEntry( pKey, aDefault.toInt() ) );
      case TQVariant::UInt:
          return TQVariant( readUnsignedNumEntry( pKey, aDefault.toUInt() ) );
      case TQVariant::LongLong:
          return TQVariant( readNum64Entry( pKey, aDefault.toLongLong() ) );
      case TQVariant::ULongLong:
          return TQVariant( readUnsignedNum64Entry( pKey, aDefault.toULongLong() ) );
      case TQVariant::Bool:
          return TQVariant( readBoolEntry( pKey, aDefault.toBool() ) );
      case TQVariant::Double:
          return TQVariant( readDoubleNumEntry( pKey, aDefault.toDouble() ) );
      case TQVariant::DateTime:
          return TQVariant( readDateTimeEntry( pKey, &tmp.asDateTime() ) );
      case TQVariant::Date:
          return TQVariant(readDateTimeEntry( pKey, &tmp.asDateTime() ).date());

      case TQVariant::Pixmap:
      case TQVariant::Image:
      case TQVariant::Brush:
      case TQVariant::Palette:
      case TQVariant::ColorGroup:
      case TQVariant::Map:
      case TQVariant::IconSet:
      case TQVariant::CString:
      case TQVariant::PointArray:
      case TQVariant::Region:
      case TQVariant::Bitmap:
      case TQVariant::Cursor:
      case TQVariant::SizePolicy:
      case TQVariant::Time:
      case TQVariant::ByteArray:
      case TQVariant::BitArray:
      case TQVariant::KeySequence:
      case TQVariant::Pen:
      {
          break;
      }
  }

  Q_ASSERT( 0 );
  return TQVariant();
}

int TDEConfigBase::readListEntry( const TQString& pKey,
                                TQStrList &list, char sep ) const
{
  return readListEntry(pKey.utf8().data(), list, sep);
}

int TDEConfigBase::readListEntry( const char *pKey,
                                TQStrList &list, char sep ) const
{
  if( !hasKey( pKey ) )
    return 0;

  TQCString str_list = readEntryUtf8( pKey );
  if (str_list.isEmpty())
    return 0;

  list.clear();
  TQCString value = "";
  int len = str_list.length();

  for (int i = 0; i < len; i++) {
    if (str_list[i] != sep && str_list[i] != '\\') {
      value += str_list[i];
      continue;
    }
    if (str_list[i] == '\\') {
      i++;
      if ( i < len )
        value += str_list[i];
      continue;
    }
    // if we fell through to here, we are at a separator.  Append
    // contents of value to the list
    // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
    // A TQStrList may contain values in 8bit locale cpecified
    // encoding
    list.append( value );
    value.truncate(0);
  }

  if ( str_list[len-1] != sep || ( len > 1 && str_list[len-2] == '\\' ) )
    list.append( value );
  return list.count();
}

TQStringList TDEConfigBase::readListEntry( const TQString& pKey, char sep ) const
{
  return readListEntry(pKey.utf8().data(), sep);
}

TQStringList TDEConfigBase::readListEntry( const char *pKey, char sep ) const
{
  static const TQString& emptyString = TDEGlobal::staticQString("");

  TQStringList list;
  if( !hasKey( pKey ) )
    return list;
  TQString str_list = readEntry( pKey );
  if( str_list.isEmpty() )
    return list;
  TQString value(emptyString);
  int len = str_list.length();
 // obviously too big, but faster than letting each += resize the string.
  value.reserve( len );
  for( int i = 0; i < len; i++ )
    {
      if( str_list[i] != sep && str_list[i] != '\\' )
        {
          value += str_list[i];
          continue;
        }
      if( str_list[i] == '\\' )
        {
          i++;
          if ( i < len )
            value += str_list[i];
          continue;
        }
      TQString finalvalue( value );
      finalvalue.squeeze();
      list.append( finalvalue );
      value.truncate( 0 );
    }
  if ( str_list[len-1] != sep || ( len > 1 && str_list[len-2] == '\\' ) )
  {
    value.squeeze();
    list.append( value );
  }
  return list;
}

TQStringList TDEConfigBase::readListEntry( const char* pKey, const TQStringList& aDefault,
		char sep ) const
{
	if ( !hasKey( pKey ) )
		return aDefault;
	else
		return readListEntry( pKey, sep );
}

TQValueList<int> TDEConfigBase::readIntListEntry( const TQString& pKey ) const
{
  return readIntListEntry(pKey.utf8().data());
}

TQValueList<int> TDEConfigBase::readIntListEntry( const char *pKey ) const
{
  TQStringList strlist = readListEntry(pKey);
  TQValueList<int> list;
  TQStringList::ConstIterator end(strlist.end());
  for (TQStringList::ConstIterator it = strlist.begin(); it != end; ++it)
    // I do not check if the toInt failed because I consider the number of items
    // more important than their value
    list << (*it).toInt();

  return list;
}

TQString TDEConfigBase::readPathEntry( const TQString& pKey, const TQString& pDefault ) const
{
  return readPathEntry(pKey.utf8().data(), pDefault);
}

TQString TDEConfigBase::readPathEntry( const char *pKey, const TQString& pDefault ) const
{
  const bool bExpandSave = bExpand;
  bExpand = true;
  TQString aValue = readEntry( pKey, pDefault );
  bExpand = bExpandSave;
  return aValue;
}

TQStringList TDEConfigBase::readPathListEntry( const TQString& pKey, char sep ) const
{
  return readPathListEntry(pKey.utf8().data(), sep);
}

TQStringList TDEConfigBase::readPathListEntry( const char *pKey, char sep ) const
{
  const bool bExpandSave = bExpand;
  bExpand = true;
  TQStringList aValue = readListEntry( pKey, sep );
  bExpand = bExpandSave;
  return aValue;
}

int TDEConfigBase::readNumEntry( const TQString& pKey, int nDefault) const
{
  return readNumEntry(pKey.utf8().data(), nDefault);
}

int TDEConfigBase::readNumEntry( const char *pKey, int nDefault) const
{
  TQCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else if( aValue == "true" || aValue == "on" || aValue == "yes" )
    return 1;
  else
    {
      bool ok;
      int rc = aValue.toInt( &ok );
      return( ok ? rc : nDefault );
    }
}


unsigned int TDEConfigBase::readUnsignedNumEntry( const TQString& pKey, unsigned int nDefault) const
{
  return readUnsignedNumEntry(pKey.utf8().data(), nDefault);
}

unsigned int TDEConfigBase::readUnsignedNumEntry( const char *pKey, unsigned int nDefault) const
{
  TQCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      unsigned int rc = aValue.toUInt( &ok );
      return( ok ? rc : nDefault );
    }
}


long TDEConfigBase::readLongNumEntry( const TQString& pKey, long nDefault) const
{
  return readLongNumEntry(pKey.utf8().data(), nDefault);
}

long TDEConfigBase::readLongNumEntry( const char *pKey, long nDefault) const
{
  TQCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      long rc = aValue.toLong( &ok );
      return( ok ? rc : nDefault );
    }
}


unsigned long TDEConfigBase::readUnsignedLongNumEntry( const TQString& pKey, unsigned long nDefault) const
{
  return readUnsignedLongNumEntry(pKey.utf8().data(), nDefault);
}

unsigned long TDEConfigBase::readUnsignedLongNumEntry( const char *pKey, unsigned long nDefault) const
{
  TQCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      unsigned long rc = aValue.toULong( &ok );
      return( ok ? rc : nDefault );
    }
}

TQ_INT64 TDEConfigBase::readNum64Entry( const TQString& pKey, TQ_INT64 nDefault) const
{
  return readNum64Entry(pKey.utf8().data(), nDefault);
}

TQ_INT64 TDEConfigBase::readNum64Entry( const char *pKey, TQ_INT64 nDefault) const
{
  // Note that TQCString::toLongLong() is missing, we muse use a TQString instead.
  TQString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      TQ_INT64 rc = aValue.toLongLong( &ok );
      return( ok ? rc : nDefault );
    }
}


TQ_UINT64 TDEConfigBase::readUnsignedNum64Entry( const TQString& pKey, TQ_UINT64 nDefault) const
{
  return readUnsignedNum64Entry(pKey.utf8().data(), nDefault);
}

TQ_UINT64 TDEConfigBase::readUnsignedNum64Entry( const char *pKey, TQ_UINT64 nDefault) const
{
  // Note that TQCString::toULongLong() is missing, we muse use a TQString instead.
  TQString aValue = readEntry( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      TQ_UINT64 rc = aValue.toULongLong( &ok );
      return( ok ? rc : nDefault );
    }
}

double TDEConfigBase::readDoubleNumEntry( const TQString& pKey, double nDefault) const
{
  return readDoubleNumEntry(pKey.utf8().data(), nDefault);
}

double TDEConfigBase::readDoubleNumEntry( const char *pKey, double nDefault) const
{
  TQCString aValue = readEntryUtf8( pKey );
  if( aValue.isNull() )
    return nDefault;
  else
    {
      bool ok;
      double rc = aValue.toDouble( &ok );
      return( ok ? rc : nDefault );
    }
}


bool TDEConfigBase::readBoolEntry( const TQString& pKey, bool bDefault ) const
{
   return readBoolEntry(pKey.utf8().data(), bDefault);
}

bool TDEConfigBase::readBoolEntry( const char *pKey, bool bDefault ) const
{
  TQCString aValue = readEntryUtf8( pKey );

  if( aValue.isNull() )
    return bDefault;
  else
    {
      if( aValue == "true" || aValue == "on" || aValue == "yes" || aValue == "1" )
        return true;
      else
        {
          bool bOK;
          int val = aValue.toInt( &bOK );
          if( bOK && val != 0 )
            return true;
          else
            return false;
        }
    }
}

TQFont TDEConfigBase::readFontEntry( const TQString& pKey, const TQFont* pDefault ) const
{
  return readFontEntry(pKey.utf8().data(), pDefault);
}

TQFont TDEConfigBase::readFontEntry( const char *pKey, const TQFont* pDefault ) const
{
  TQFont aRetFont;

  TQString aValue = readEntry( pKey );
  if( !aValue.isNull() ) {
    if ( aValue.contains( ',' ) > 5 ) {
      // KDE3 and upwards entry
      if ( !aRetFont.fromString( aValue ) && pDefault )
        aRetFont = *pDefault;
    }
    else {
      // backward compatibility with older font formats
      // ### remove KDE 3.1 ?
      // find first part (font family)
      int nIndex = aValue.find( ',' );
      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }
      aRetFont.setFamily( aValue.left( nIndex ) );

      // find second part (point size)
      int nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );
      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }

      aRetFont.setPointSize( aValue.mid( nOldIndex+1,
                                         nIndex-nOldIndex-1 ).toInt() );

      // find third part (style hint)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );

      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }

      aRetFont.setStyleHint( (TQFont::StyleHint)aValue.mid( nOldIndex+1, nIndex-nOldIndex-1 ).toUInt() );

      // find fourth part (char set)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );

      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }

      TQString chStr=aValue.mid( nOldIndex+1,
                                nIndex-nOldIndex-1 );
      // find fifth part (weight)
      nOldIndex = nIndex;
      nIndex = aValue.find( ',', nOldIndex+1 );

      if( nIndex == -1 ){
        if( pDefault )
          aRetFont = *pDefault;
        return aRetFont;
      }

      aRetFont.setWeight( aValue.mid( nOldIndex+1,
                                      nIndex-nOldIndex-1 ).toUInt() );

      // find sixth part (font bits)
      uint nFontBits = aValue.right( aValue.length()-nIndex-1 ).toUInt();

      aRetFont.setItalic( nFontBits & 0x01 );
      aRetFont.setUnderline( nFontBits & 0x02 );
      aRetFont.setStrikeOut( nFontBits & 0x04 );
      aRetFont.setFixedPitch( nFontBits & 0x08 );
      aRetFont.setRawMode( nFontBits & 0x20 );
    }
  }
  else
    {
      if( pDefault )
        aRetFont = *pDefault;
    }

  return aRetFont;
}


TQRect TDEConfigBase::readRectEntry( const TQString& pKey, const TQRect* pDefault ) const
{
  return readRectEntry(pKey.utf8().data(), pDefault);
}

TQRect TDEConfigBase::readRectEntry( const char *pKey, const TQRect* pDefault ) const
{
  TQCString aValue = readEntryUtf8(pKey);

  if (!aValue.isEmpty())
  {
    int left, top, width, height;

    if (sscanf(aValue.data(), "%d,%d,%d,%d", &left, &top, &width, &height) == 4)
    {
       return TQRect(left, top, width, height);
    }
  }
  if (pDefault)
    return *pDefault;
  return TQRect();
}


TQPoint TDEConfigBase::readPointEntry( const TQString& pKey,
                                    const TQPoint* pDefault ) const
{
  return readPointEntry(pKey.utf8().data(), pDefault);
}

TQPoint TDEConfigBase::readPointEntry( const char *pKey,
                                    const TQPoint* pDefault ) const
{
  TQCString aValue = readEntryUtf8(pKey);

  if (!aValue.isEmpty())
  {
    int x,y;

    if (sscanf(aValue.data(), "%d,%d", &x, &y) == 2)
    {
       return TQPoint(x,y);
    }
  }
  if (pDefault)
    return *pDefault;
  return TQPoint();
}

TQSize TDEConfigBase::readSizeEntry( const TQString& pKey,
                                  const TQSize* pDefault ) const
{
  return readSizeEntry(pKey.utf8().data(), pDefault);
}

TQSize TDEConfigBase::readSizeEntry( const char *pKey,
                                  const TQSize* pDefault ) const
{
  TQCString aValue = readEntryUtf8(pKey);

  if (!aValue.isEmpty())
  {
    int width,height;

    if (sscanf(aValue.data(), "%d,%d", &width, &height) == 2)
    {
       return TQSize(width, height);
    }
  }
  if (pDefault)
    return *pDefault;
  return TQSize();
}


TQColor TDEConfigBase::readColorEntry( const TQString& pKey,
                                    const TQColor* pDefault ) const
{
  return readColorEntry(pKey.utf8().data(), pDefault);
}

TQColor TDEConfigBase::readColorEntry( const char *pKey,
                                    const TQColor* pDefault ) const
{
  TQColor aRetColor;
  int nRed = 0, nGreen = 0, nBlue = 0;

  TQString aValue = readEntry( pKey );
  if( !aValue.isEmpty() )
    {
      if ( aValue.at(0) == (TQChar)'#' )
        {
          aRetColor.setNamedColor(aValue);
        }
      else
        {

          bool bOK;

          // find first part (red)
          int nIndex = aValue.find( ',' );

          if( nIndex == -1 ){
            // return a sensible default -- Bernd
            if( pDefault )
              aRetColor = *pDefault;
            return aRetColor;
          }

          nRed = aValue.left( nIndex ).toInt( &bOK );

          // find second part (green)
          int nOldIndex = nIndex;
          nIndex = aValue.find( ',', nOldIndex+1 );

          if( nIndex == -1 ){
            // return a sensible default -- Bernd
            if( pDefault )
              aRetColor = *pDefault;
            return aRetColor;
          }
          nGreen = aValue.mid( nOldIndex+1,
                               nIndex-nOldIndex-1 ).toInt( &bOK );

          // find third part (blue)
          nBlue = aValue.right( aValue.length()-nIndex-1 ).toInt( &bOK );

          aRetColor.setRgb( nRed, nGreen, nBlue );
        }
    }
  else {

    if( pDefault )
      aRetColor = *pDefault;
  }

  return aRetColor;
}


TQDateTime TDEConfigBase::readDateTimeEntry( const TQString& pKey,
                                          const TQDateTime* pDefault ) const
{
  return readDateTimeEntry(pKey.utf8().data(), pDefault);
}

// ### currentDateTime() as fallback ? (Harri)
TQDateTime TDEConfigBase::readDateTimeEntry( const char *pKey,
                                          const TQDateTime* pDefault ) const
{
  if( !hasKey( pKey ) )
    {
      if( pDefault )
        return *pDefault;
      else
        return TQDateTime::currentDateTime();
    }

  TQStrList list;
  int count = readListEntry( pKey, list, ',' );
  if( count == 6 ) {
    TQDate date( atoi( list.at( 0 ) ), atoi( list.at( 1 ) ),
                atoi( list.at( 2 ) ) );
    TQTime time( atoi( list.at( 3 ) ), atoi( list.at( 4 ) ),
                atoi( list.at( 5 ) ) );

    return TQDateTime( date, time );
  }

  return TQDateTime::currentDateTime();
}

void TDEConfigBase::writeEntry( const TQString& pKey, const TQString& value,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS )
{
   writeEntry(pKey.utf8().data(), value, bPersistent,  bGlobal, bNLS);
}

void TDEConfigBase::writeEntry( const char *pKey, const TQString& value,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS )
{
   writeEntry(pKey, value, bPersistent,  bGlobal, bNLS, false);
}

void TDEConfigBase::writeEntry( const char *pKey, const TQString& value,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS,
                                 bool bExpand )
{
  // the TDEConfig object is dirty now
  // set this before any IO takes place so that if any derivative
  // classes do caching, they won't try and flush the cache out
  // from under us before we read. A race condition is still
  // possible but minimized.
  if( bPersistent )
    setDirty(true);

  if (!bLocaleInitialized && TDEGlobal::locale())
    setLocale();

  KEntryKey entryKey(mGroup, pKey);
  entryKey.bLocal = bNLS;

  KEntry aEntryData;
  aEntryData.mValue = value.utf8();  // set new value
  aEntryData.bGlobal = bGlobal;
  aEntryData.bNLS = bNLS;
  aEntryData.bExpand = bExpand;

  if (bPersistent)
    aEntryData.bDirty = true;

  // rewrite the new value
  putData(entryKey, aEntryData, true);
}

void TDEConfigBase::writePathEntry( const TQString& pKey, const TQString & path,
                                  bool bPersistent, bool bGlobal,
                                  bool bNLS)
{
   writePathEntry(pKey.utf8().data(), path, bPersistent, bGlobal, bNLS);
}


static bool cleanHomeDirPath( TQString &path, const TQString &homeDir )
{
#ifdef TQ_WS_WIN //safer
   if (!TQDir::convertSeparators(path).startsWith(TQDir::convertSeparators(homeDir)))
        return false;
#else
   if (!path.startsWith(homeDir))
        return false;
#endif

   unsigned int len = homeDir.length();
   // replace by "$HOME" if possible
   if (len && (path.length() == len || path[len] == '/')) {
        path.replace(0, len, TQString::fromLatin1("$HOME"));
        return true;
   } else
        return false;
}

static TQString translatePath( TQString path )
{
   if (path.isEmpty())
       return path;

   // only "our" $HOME should be interpreted
   path.replace('$', "$$");

   bool startsWithFile = path.startsWith("file:", false);

   // return original path, if it refers to another type of URL (e.g. http:/), or
   // if the path is already relative to another directory
   if (((!startsWithFile) && (path[0] != '/')) || (startsWithFile && (path[5] != '/'))) {
	return path;
   }

   if (startsWithFile) {
        path.remove(0,5); // strip leading "file:/" off the string
   }

   // keep only one single '/' at the beginning - needed for cleanHomeDirPath()
   while (path[0] == '/' && path[1] == '/') {
	path.remove(0,1);
   }

   // we can not use TDEGlobal::dirs()->relativeLocation("home", path) here,
   // since it would not recognize paths without a trailing '/'.
   // All of the 3 following functions to return the user's home directory
   // can return different paths. We have to test all them.
   TQString homeDir0 = TQFile::decodeName(getenv("HOME"));
   TQString homeDir1 = TQDir::homeDirPath();
   TQString homeDir2 = TQDir(homeDir1).canonicalPath();
   if (cleanHomeDirPath(path, homeDir0) ||
       cleanHomeDirPath(path, homeDir1) ||
       cleanHomeDirPath(path, homeDir2) ) {
     // kdDebug() << "Path was replaced\n";
   }

   if (startsWithFile)
      path.prepend( "file://" );

   return path;
}

void TDEConfigBase::writePathEntry( const char *pKey, const TQString & path,
                                  bool bPersistent, bool bGlobal,
                                  bool bNLS)
{
   writeEntry(pKey, translatePath(path), bPersistent, bGlobal, bNLS, true);
}

void TDEConfigBase::writePathEntry( const char *pKey, const TQString & path,
                                  bool bPersistent, bool bGlobal,
                                  bool bNLS, bool expand)
{
   writeEntry(pKey, translatePath(path), bPersistent, bGlobal, bNLS, expand);
}

void TDEConfigBase::writePathEntry ( const TQString& pKey, const TQStringList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writePathEntry(pKey.utf8().data(), list, sep, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writePathEntry ( const char *pKey, const TQStringList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, TQString::fromLatin1(""), bPersistent );
      return;
    }
  TQStringList new_list;
  TQStringList::ConstIterator it = list.begin();
  for( ; it != list.end(); ++it )
    {
      TQString value = *it;
      new_list.append( translatePath(value) );
    }
  writeEntry( pKey, new_list, sep, bPersistent, bGlobal, bNLS, true );
}

void TDEConfigBase::deleteEntry( const TQString& pKey,
                                 bool bNLS,
                                 bool bGlobal)
{
   deleteEntry(pKey.utf8().data(), bNLS, bGlobal);
}

void TDEConfigBase::deleteEntry( const char *pKey,
                                 bool bNLS,
                                 bool bGlobal)
{
  // the TDEConfig object is dirty now
  // set this before any IO takes place so that if any derivative
  // classes do caching, they won't try and flush the cache out
  // from under us before we read. A race condition is still
  // possible but minimized.
  setDirty(true);

  if (!bLocaleInitialized && TDEGlobal::locale())
    setLocale();

  KEntryKey entryKey(mGroup, pKey);
  KEntry aEntryData;

  aEntryData.bGlobal = bGlobal;
  aEntryData.bNLS = bNLS;
  aEntryData.bDirty = true;
  aEntryData.bDeleted = true;

  // rewrite the new value
  putData(entryKey, aEntryData, true);
}

bool TDEConfigBase::deleteGroup( const TQString& group, bool bDeep, bool bGlobal )
{
  KEntryMap aEntryMap = internalEntryMap(group);

  if (!bDeep) {
    // Check if it empty
    return aEntryMap.isEmpty();
  }

  bool dirty = false;
  bool checkGroup = true;
  // we want to remove all entries in the group
  KEntryMapIterator aIt;
  for (aIt = aEntryMap.begin(); aIt != aEntryMap.end(); ++aIt)
  {
    if (!aIt.key().mKey.isEmpty() && !aIt.key().bDefault && !(*aIt).bDeleted)
    {
      (*aIt).bDeleted = true;
      (*aIt).bDirty = true;
      (*aIt).bGlobal = bGlobal;
      (*aIt).mValue = 0;
      putData(aIt.key(), *aIt, checkGroup);
      checkGroup = false;
      dirty = true;
    }
  }
  if (dirty)
     setDirty(true);
  return true;
}

void TDEConfigBase::writeEntry ( const TQString& pKey, const TQVariant &prop,
                               bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writeEntry(pKey.utf8().data(), prop, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry ( const char *pKey, const TQVariant &prop,
                               bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  switch( prop.type() )
    {
    case TQVariant::Invalid:
      writeEntry( pKey, "", bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::String:
      writeEntry( pKey, prop.toString(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::StringList:
      writeEntry( pKey, prop.toStringList(), ',', bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::List: {
        TQValueList<TQVariant> list = prop.toList();
        TQValueList<TQVariant>::ConstIterator it = list.begin();
        TQValueList<TQVariant>::ConstIterator end = list.end();
        TQStringList strList;

        for (; it != end; ++it )
            strList.append( (*it).toString() );

        writeEntry( pKey, strList, ',', bPersistent, bGlobal, bNLS );

        return;
    }
    case TQVariant::Font:
      writeEntry( pKey, prop.toFont(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::Point:
      writeEntry( pKey, prop.toPoint(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::Rect:
      writeEntry( pKey, prop.toRect(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::Size:
      writeEntry( pKey, prop.toSize(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::Color:
      writeEntry( pKey, prop.toColor(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::Int:
      writeEntry( pKey, prop.toInt(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::UInt:
      writeEntry( pKey, prop.toUInt(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::LongLong:
      writeEntry( pKey, prop.toLongLong(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::ULongLong:
      writeEntry( pKey, prop.toULongLong(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::Bool:
      writeEntry( pKey, prop.toBool(), bPersistent, bGlobal, bNLS );
      return;
    case TQVariant::Double:
      writeEntry( pKey, prop.toDouble(), bPersistent, bGlobal, 'g', 6, bNLS );
      return;
    case TQVariant::DateTime:
      writeEntry( pKey, prop.toDateTime(), bPersistent, bGlobal, bNLS);
      return;
    case TQVariant::Date:
      writeEntry( pKey, TQDateTime(prop.toDate()), bPersistent, bGlobal, bNLS);
      return;

    case TQVariant::Pixmap:
    case TQVariant::Image:
    case TQVariant::Brush:
    case TQVariant::Palette:
    case TQVariant::ColorGroup:
    case TQVariant::Map:
    case TQVariant::IconSet:
    case TQVariant::CString:
    case TQVariant::PointArray:
    case TQVariant::Region:
    case TQVariant::Bitmap:
    case TQVariant::Cursor:
    case TQVariant::SizePolicy:
    case TQVariant::Time:
    case TQVariant::ByteArray:
    case TQVariant::BitArray:
    case TQVariant::KeySequence:
    case TQVariant::Pen:
      {
        break;
      }
    }

  Q_ASSERT( 0 );
}

void TDEConfigBase::writeEntry ( const TQString& pKey, const TQStrList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writeEntry(pKey.utf8().data(), list, sep, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry ( const char *pKey, const TQStrList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, TQString::fromLatin1(""), bPersistent );
      return;
    }
  TQString str_list;
  TQStrListIterator it( list );
  for( ; it.current(); ++it )
    {
      uint i;
      TQString value;
      // !!! Sergey A. Sukiyazov <corwin@micom.don.ru> !!!
      // A TQStrList may contain values in 8bit locale cpecified
      // encoding or in UTF8 encoding.
      value = KStringHandler::from8Bit(it.current());
      uint strLengh(value.length());
      for( i = 0; i < strLengh; i++ )
        {
          if( value[i] == sep || value[i] == '\\' )
            str_list += '\\';
          str_list += value[i];
        }
      str_list += sep;
    }
  if( str_list.at(str_list.length() - 1) == (TQChar)sep )
    str_list.truncate( str_list.length() -1 );
  writeEntry( pKey, str_list, bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry ( const TQString& pKey, const TQStringList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writeEntry(pKey.utf8().data(), list, sep, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry ( const char *pKey, const TQStringList &list,
                               char sep , bool bPersistent,
                               bool bGlobal, bool bNLS )
{
  writeEntry(pKey, list, sep, bPersistent, bGlobal, bNLS, false);
}

void TDEConfigBase::writeEntry ( const char *pKey, const TQStringList &list,
                               char sep, bool bPersistent,
                               bool bGlobal, bool bNLS, bool bExpand )
{
  if( list.isEmpty() )
    {
      writeEntry( pKey, TQString::fromLatin1(""), bPersistent );
      return;
    }
  TQString str_list;
  str_list.reserve( 4096 );
  TQStringList::ConstIterator it = list.begin();
  for( ; it != list.end(); ++it )
    {
      TQString value = *it;
      uint i;
      uint strLength(value.length());
      for( i = 0; i < strLength; i++ )
        {
          if( value[i] == sep || value[i] == '\\' )
            str_list += '\\';
          str_list += value[i];
        }
      str_list += sep;
    }
  if( str_list.at(str_list.length() - 1) == (TQChar)sep )
    str_list.truncate( str_list.length() -1 );
  writeEntry( pKey, str_list, bPersistent, bGlobal, bNLS, bExpand );
}

void TDEConfigBase::writeEntry ( const TQString& pKey, const TQValueList<int> &list,
                               bool bPersistent, bool bGlobal, bool bNLS )
{
  writeEntry(pKey.utf8().data(), list, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry ( const char *pKey, const TQValueList<int> &list,
                               bool bPersistent, bool bGlobal, bool bNLS )
{
    TQStringList strlist;
    TQValueList<int>::ConstIterator end = list.end();
    for (TQValueList<int>::ConstIterator it = list.begin(); it != end; it++)
        strlist << TQString::number(*it);
    writeEntry(pKey, strlist, ',', bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const TQString& pKey, int nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const char *pKey, int nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, unsigned int nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const char *pKey, unsigned int nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, long nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const char *pKey, long nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, unsigned long nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const char *pKey, unsigned long nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const TQString& pKey, TQ_INT64 nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const char *pKey, TQ_INT64 nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, TQ_UINT64 nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const char *pKey, TQ_UINT64 nValue,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue), bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const TQString& pKey, double nValue,
                                 bool bPersistent, bool bGlobal,
                                 char format, int precision,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue, format, precision),
                     bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const char *pKey, double nValue,
                                 bool bPersistent, bool bGlobal,
                                 char format, int precision,
                                 bool bNLS )
{
  writeEntry( pKey, TQString::number(nValue, format, precision),
                     bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, bool bValue,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS )
{
  writeEntry(pKey.utf8().data(), bValue, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry( const char *pKey, bool bValue,
                                 bool bPersistent,
                                 bool bGlobal,
                                 bool bNLS )
{
  TQString aValue;

  if( bValue )
    aValue = "true";
  else
    aValue = "false";

  writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, const TQFont& rFont,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry(pKey.utf8().data(), rFont, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry( const char *pKey, const TQFont& rFont,
                                 bool bPersistent, bool bGlobal,
                                 bool bNLS )
{
  writeEntry( pKey, TQString(rFont.toString()), bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, const TQRect& rRect,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  writeEntry(pKey.utf8().data(), rRect, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry( const char *pKey, const TQRect& rRect,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  TQStrList list;
  TQCString tempstr;
  list.insert( 0, tempstr.setNum( rRect.left() ) );
  list.insert( 1, tempstr.setNum( rRect.top() ) );
  list.insert( 2, tempstr.setNum( rRect.width() ) );
  list.insert( 3, tempstr.setNum( rRect.height() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, const TQPoint& rPoint,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  writeEntry(pKey.utf8().data(), rPoint, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry( const char *pKey, const TQPoint& rPoint,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  TQStrList list;
  TQCString tempstr;
  list.insert( 0, tempstr.setNum( rPoint.x() ) );
  list.insert( 1, tempstr.setNum( rPoint.y() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}


void TDEConfigBase::writeEntry( const TQString& pKey, const TQSize& rSize,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  writeEntry(pKey.utf8().data(), rSize, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry( const char *pKey, const TQSize& rSize,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  TQStrList list;
  TQCString tempstr;
  list.insert( 0, tempstr.setNum( rSize.width() ) );
  list.insert( 1, tempstr.setNum( rSize.height() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const TQString& pKey, const TQColor& rColor,
                              bool bPersistent,
                              bool bGlobal,
                              bool bNLS  )
{
  writeEntry( pKey.utf8().data(), rColor, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry( const char *pKey, const TQColor& rColor,
                              bool bPersistent,
                              bool bGlobal,
                              bool bNLS  )
{
  TQString aValue;
  if (rColor.isValid())
      aValue.sprintf( "%d,%d,%d", rColor.red(), rColor.green(), rColor.blue() );
  else
      aValue = "invalid";

  writeEntry( pKey, aValue, bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::writeEntry( const TQString& pKey, const TQDateTime& rDateTime,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  writeEntry(pKey.utf8().data(), rDateTime, bPersistent, bGlobal, bNLS);
}

void TDEConfigBase::writeEntry( const char *pKey, const TQDateTime& rDateTime,
                              bool bPersistent, bool bGlobal,
                              bool bNLS )
{
  TQStrList list;
  TQCString tempstr;

  TQTime time = rDateTime.time();
  TQDate date = rDateTime.date();

  list.insert( 0, tempstr.setNum( date.year() ) );
  list.insert( 1, tempstr.setNum( date.month() ) );
  list.insert( 2, tempstr.setNum( date.day() ) );

  list.insert( 3, tempstr.setNum( time.hour() ) );
  list.insert( 4, tempstr.setNum( time.minute() ) );
  list.insert( 5, tempstr.setNum( time.second() ) );

  writeEntry( pKey, list, ',', bPersistent, bGlobal, bNLS );
}

void TDEConfigBase::parseConfigFiles()
{
  if (!bLocaleInitialized && TDEGlobal::_locale) {
    setLocale();
  }
  if (backEnd)
  {
     backEnd->parseConfigFiles();
     bReadOnly = (backEnd->getConfigState() == ReadOnly);
  }
}

void TDEConfigBase::sync()
{
  if (isReadOnly())
    return;

  if (backEnd)
     backEnd->sync();
  if (bDirty)
    rollback();
}

TDEConfigBase::ConfigState TDEConfigBase::getConfigState() const {
    if (backEnd)
       return backEnd->getConfigState();
    return ReadOnly;
}

void TDEConfigBase::rollback( bool /*bDeep = true*/ )
{
  bDirty = false;
}


void TDEConfigBase::setReadDefaults(bool b)
{
  if (!d)
  {
     if (!b) return;
     d = new TDEConfigBasePrivate();
  }

  d->readDefaults = b;
}

bool TDEConfigBase::readDefaults() const
{
  return (d && d->readDefaults);
}

void TDEConfigBase::revertToDefault(const TQString &key)
{
  setDirty(true);

  KEntryKey aEntryKey(mGroup, key.utf8());
  aEntryKey.bDefault = true;

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (entry.mValue.isNull())
        entry.bDeleted = true;

    entry.bDirty = true;
    putData(aEntryKey, entry, true); // Revert
    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  if (entry.mValue.isNull())
     entry.bDeleted = true;
  entry.bDirty = true;
  putData(aEntryKey, entry, true); // Revert
}

bool TDEConfigBase::hasDefault(const TQString &key) const
{
  KEntryKey aEntryKey(mGroup, key.utf8());
  aEntryKey.bDefault = true;

  if (!locale().isNull()) {
    // try the localized key first
    aEntryKey.bLocal = true;
    KEntry entry = lookupData(aEntryKey);
    if (!entry.mValue.isNull())
        return true;

    aEntryKey.bLocal = false;
  }

  // try the non-localized version
  KEntry entry = lookupData(aEntryKey);
  if (!entry.mValue.isNull())
     return true;

  return false;
}



TDEConfigGroup::TDEConfigGroup(TDEConfigBase *master, const TQString &group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group.utf8();
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

TDEConfigGroup::TDEConfigGroup(TDEConfigBase *master, const TQCString &group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group;
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

TDEConfigGroup::TDEConfigGroup(TDEConfigBase *master, const char * group)
{
  mMaster = master;
  backEnd = mMaster->backEnd; // Needed for getConfigState()
  bLocaleInitialized = true;
  bReadOnly = mMaster->bReadOnly;
  bExpand = false;
  bDirty = false; // Not used
  mGroup = group;
  aLocaleString = mMaster->aLocaleString;
  setReadDefaults(mMaster->readDefaults());
}

void TDEConfigGroup::deleteGroup(bool bGlobal)
{
  mMaster->deleteGroup(TDEConfigBase::group(), true, bGlobal);
}

bool TDEConfigGroup::groupIsImmutable() const
{
    return mMaster->groupIsImmutable(TDEConfigBase::group());
}

void TDEConfigGroup::setDirty(bool _bDirty)
{
  mMaster->setDirty(_bDirty);
}

void TDEConfigGroup::putData(const KEntryKey &_key, const KEntry &_data, bool _checkGroup)
{
  mMaster->putData(_key, _data, _checkGroup);
}

KEntry TDEConfigGroup::lookupData(const KEntryKey &_key) const
{
  return mMaster->lookupData(_key);
}

void TDEConfigGroup::sync()
{
  mMaster->sync();
}

void TDEConfigBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void TDEConfigGroup::virtual_hook( int id, void* data )
{ TDEConfigBase::virtual_hook( id, data ); }

bool TDEConfigBase::checkConfigFilesWritable(bool warnUser)
{
  if (backEnd)
    return backEnd->checkConfigFilesWritable(warnUser);
  else
    return false;
}

#include "tdeconfigbase.moc"
