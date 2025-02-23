/*
    This file is part of KOrganizer.
    Copyright (c) 2000,2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Waldo Bastian <bastian@kde.org>

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

#include <tqcolor.h>
#include <tqvariant.h>

#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kdebug.h>

#include "kstringhandler.h"

#include "tdeconfigskeleton.h"

void TDEConfigSkeletonItem::readImmutability( TDEConfig *config )
{
  mIsImmutable = config->entryIsImmutable( mKey );
}


TDEConfigSkeleton::ItemString::ItemString( const TQString &group, const TQString &key,
                                    TQString &reference,
                                    const TQString &defaultValue,
                                    Type type )
  : TDEConfigSkeletonGenericItem<TQString>( group, key, reference, defaultValue ),
    mType( type )
{
}

void TDEConfigSkeleton::ItemString::writeConfig( TDEConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else if ( mType == Path )
      config->writePathEntry( mKey, mReference );
    else if ( mType == Password )
      config->writeEntry( mKey, KStringHandler::obscure( mReference ) );
    else
      config->writeEntry( mKey, mReference );
  }
}


void TDEConfigSkeleton::ItemString::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );

  if ( mType == Path )
  {
    mReference = config->readPathEntry( mKey, mDefault );
  }
  else if ( mType == Password ) 
  {
    TQString value = config->readEntry( mKey,
                                       KStringHandler::obscure( mDefault ) );
    mReference = KStringHandler::obscure( value );
  }
  else
  {
    mReference = config->readEntry( mKey, mDefault );
  }

  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemString::setProperty(const TQVariant & p)
{
  mReference = p.toString();
}

TQVariant TDEConfigSkeleton::ItemString::property() const
{
  return TQVariant(mReference);
}

TDEConfigSkeleton::ItemPassword::ItemPassword( const TQString &group, const TQString &key,
                                    TQString &reference,
                                    const TQString &defaultValue)
  : ItemString( group, key, reference, defaultValue, Password )
{
}

TDEConfigSkeleton::ItemPath::ItemPath( const TQString &group, const TQString &key,
                                    TQString &reference,
                                    const TQString &defaultValue)
  : ItemString( group, key, reference, defaultValue, Path )
{
}

TDEConfigSkeleton::ItemProperty::ItemProperty( const TQString &group,
                                        const TQString &key,
                                        TQVariant &reference,
                                        TQVariant defaultValue )
  : TDEConfigSkeletonGenericItem<TQVariant>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemProperty::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readPropertyEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemProperty::setProperty(const TQVariant & p)
{
  mReference = p;
}

TQVariant TDEConfigSkeleton::ItemProperty::property() const
{
  return mReference;
}

TDEConfigSkeleton::ItemBool::ItemBool( const TQString &group, const TQString &key,
                                bool &reference, bool defaultValue )
  : TDEConfigSkeletonGenericItem<bool>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemBool::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readBoolEntry( mKey, mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemBool::setProperty(const TQVariant & p)
{
  mReference = p.toBool();
}

TQVariant TDEConfigSkeleton::ItemBool::property() const
{
  return TQVariant( mReference );
}


TDEConfigSkeleton::ItemInt::ItemInt( const TQString &group, const TQString &key,
                              int &reference, int defaultValue )
  : TDEConfigSkeletonGenericItem<int>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void TDEConfigSkeleton::ItemInt::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = TQMAX(mReference, mMin);
  if (mHasMax)
    mReference = TQMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemInt::setProperty(const TQVariant & p)
{
  mReference = p.toInt();
}

TQVariant TDEConfigSkeleton::ItemInt::property() const
{
  return TQVariant(mReference);
}

TQVariant TDEConfigSkeleton::ItemInt::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant TDEConfigSkeleton::ItemInt::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void TDEConfigSkeleton::ItemInt::setMinValue(int v)
{
  mHasMin = true;
  mMin = v;
}

void TDEConfigSkeleton::ItemInt::setMaxValue(int v)
{
  mHasMax = true;
  mMax = v;
}


TDEConfigSkeleton::ItemInt64::ItemInt64( const TQString &group, const TQString &key,
                              TQ_INT64 &reference, TQ_INT64 defaultValue )
  : TDEConfigSkeletonGenericItem<TQ_INT64>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void TDEConfigSkeleton::ItemInt64::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readNum64Entry( mKey, mDefault );
  if (mHasMin)
    mReference = TQMAX(mReference, mMin);
  if (mHasMax)
    mReference = TQMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemInt64::setProperty(const TQVariant & p)
{
  mReference = p.toLongLong();
}

TQVariant TDEConfigSkeleton::ItemInt64::property() const
{
  return TQVariant(mReference);
}

TQVariant TDEConfigSkeleton::ItemInt64::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant TDEConfigSkeleton::ItemInt64::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void TDEConfigSkeleton::ItemInt64::setMinValue(TQ_INT64 v)
{
  mHasMin = true;
  mMin = v;
}

void TDEConfigSkeleton::ItemInt64::setMaxValue(TQ_INT64 v)
{
  mHasMax = true;
  mMax = v;
}

TDEConfigSkeleton::ItemEnum::ItemEnum( const TQString &group, const TQString &key,
                                     int &reference,
                                     const TQValueList<Choice> &choices,
                                     int defaultValue )
  : ItemInt( group, key, reference, defaultValue ), mChoices( choices )
{
}

void TDEConfigSkeleton::ItemEnum::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  if (!config->hasKey(mKey))
  {
    mReference = mDefault;
  }
  else
  {
    int i = 0;
    mReference = -1;
    TQString tmp = config->readEntry( mKey ).lower();
    for(TQValueList<Choice>::ConstIterator it = mChoices.begin();
        it != mChoices.end(); ++it, ++i)
    {
      if ((*it).name.lower() == tmp)
      {
        mReference = i;
        break;
      }
    }
    if (mReference == -1)
       mReference = config->readNumEntry( mKey, mDefault );
  }
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemEnum::writeConfig( TDEConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else if ((mReference >= 0) && (mReference < (int) mChoices.count()))
      config->writeEntry( mKey, mChoices[mReference].name );
    else
      config->writeEntry( mKey, mReference );
  }
}

TQValueList<TDEConfigSkeleton::ItemEnum::Choice> TDEConfigSkeleton::ItemEnum::choices() const
{
  return mChoices;
}


TDEConfigSkeleton::ItemUInt::ItemUInt( const TQString &group, const TQString &key,
                                unsigned int &reference,
                                unsigned int defaultValue )
  : TDEConfigSkeletonGenericItem<unsigned int>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void TDEConfigSkeleton::ItemUInt::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = TQMAX(mReference, mMin);
  if (mHasMax)
    mReference = TQMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemUInt::setProperty(const TQVariant & p)
{
  mReference = p.toUInt();
}

TQVariant TDEConfigSkeleton::ItemUInt::property() const
{
  return TQVariant(mReference);
}

TQVariant TDEConfigSkeleton::ItemUInt::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant TDEConfigSkeleton::ItemUInt::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void TDEConfigSkeleton::ItemUInt::setMinValue(unsigned int v)
{
  mHasMin = true;
  mMin = v;
}

void TDEConfigSkeleton::ItemUInt::setMaxValue(unsigned int v)
{
  mHasMax = true;
  mMax = v;
}


TDEConfigSkeleton::ItemUInt64::ItemUInt64( const TQString &group, const TQString &key,
                              TQ_UINT64 &reference, TQ_UINT64 defaultValue )
  : TDEConfigSkeletonGenericItem<TQ_UINT64>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void TDEConfigSkeleton::ItemUInt64::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedNum64Entry( mKey, mDefault );
  if (mHasMin)
    mReference = TQMAX(mReference, mMin);
  if (mHasMax)
    mReference = TQMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemUInt64::setProperty(const TQVariant & p)
{
  mReference = p.toULongLong();
}

TQVariant TDEConfigSkeleton::ItemUInt64::property() const
{
  return TQVariant(mReference);
}

TQVariant TDEConfigSkeleton::ItemUInt64::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant TDEConfigSkeleton::ItemUInt64::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void TDEConfigSkeleton::ItemUInt64::setMinValue(TQ_UINT64 v)
{
  mHasMin = true;
  mMin = v;
}

void TDEConfigSkeleton::ItemUInt64::setMaxValue(TQ_UINT64 v)
{
  mHasMax = true;
  mMax = v;
}

TDEConfigSkeleton::ItemLong::ItemLong( const TQString &group, const TQString &key,
                                long &reference, long defaultValue )
  : TDEConfigSkeletonGenericItem<long>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void TDEConfigSkeleton::ItemLong::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readLongNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = TQMAX(mReference, mMin);
  if (mHasMax)
    mReference = TQMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemLong::setProperty(const TQVariant & p)
{
  mReference = p.toLongLong();
}

TQVariant TDEConfigSkeleton::ItemLong::property() const
{
  return TQVariant((TQ_LLONG) mReference);
}

TQVariant TDEConfigSkeleton::ItemLong::minValue() const
{
  if (mHasMin)
    return TQVariant((TQ_LLONG) mMin);
  return TQVariant();
}

TQVariant TDEConfigSkeleton::ItemLong::maxValue() const
{
  if (mHasMax)
    return TQVariant((TQ_LLONG) mMax);
  return TQVariant();
}

void TDEConfigSkeleton::ItemLong::setMinValue(long v)
{
  mHasMin = true;
  mMin = v;
}

void TDEConfigSkeleton::ItemLong::setMaxValue(long v)
{
  mHasMax = true;
  mMax = v;
}


TDEConfigSkeleton::ItemULong::ItemULong( const TQString &group, const TQString &key,
                                  unsigned long &reference,
                                  unsigned long defaultValue )
  : TDEConfigSkeletonGenericItem<unsigned long>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void TDEConfigSkeleton::ItemULong::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readUnsignedLongNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = TQMAX(mReference, mMin);
  if (mHasMax)
    mReference = TQMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemULong::setProperty(const TQVariant & p)
{
  mReference = p.toULongLong();
}

TQVariant TDEConfigSkeleton::ItemULong::property() const
{
  return TQVariant((TQ_ULLONG) mReference);
}

TQVariant TDEConfigSkeleton::ItemULong::minValue() const
{
  if (mHasMin)
    return TQVariant((TQ_ULLONG) mMin);
  return TQVariant();
}

TQVariant TDEConfigSkeleton::ItemULong::maxValue() const
{
  if (mHasMax)
    return TQVariant((TQ_ULLONG) mMax);
  return TQVariant();
}

void TDEConfigSkeleton::ItemULong::setMinValue(unsigned long v)
{
  mHasMin = true;
  mMin = v;
}

void TDEConfigSkeleton::ItemULong::setMaxValue(unsigned long v)
{
  mHasMax = true;
  mMax = v;
}


TDEConfigSkeleton::ItemDouble::ItemDouble( const TQString &group, const TQString &key,
                                    double &reference, double defaultValue )
  : TDEConfigSkeletonGenericItem<double>( group, key, reference, defaultValue )
  ,mHasMin(false), mHasMax(false)
{
}

void TDEConfigSkeleton::ItemDouble::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readDoubleNumEntry( mKey, mDefault );
  if (mHasMin)
    mReference = TQMAX(mReference, mMin);
  if (mHasMax)
    mReference = TQMIN(mReference, mMax);
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemDouble::setProperty(const TQVariant & p)
{
  mReference = p.toDouble();
}

TQVariant TDEConfigSkeleton::ItemDouble::property() const
{
  return TQVariant(mReference);
}

TQVariant TDEConfigSkeleton::ItemDouble::minValue() const
{
  if (mHasMin)
    return TQVariant(mMin);
  return TQVariant();
}

TQVariant TDEConfigSkeleton::ItemDouble::maxValue() const
{
  if (mHasMax)
    return TQVariant(mMax);
  return TQVariant();
}

void TDEConfigSkeleton::ItemDouble::setMinValue(double v)
{
  mHasMin = true;
  mMin = v;
}

void TDEConfigSkeleton::ItemDouble::setMaxValue(double v)
{
  mHasMax = true;
  mMax = v;
}


TDEConfigSkeleton::ItemColor::ItemColor( const TQString &group, const TQString &key,
                                  TQColor &reference,
                                  const TQColor &defaultValue )
  : TDEConfigSkeletonGenericItem<TQColor>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemColor::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readColorEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemColor::setProperty(const TQVariant & p)
{
  mReference = p.toColor();
}

TQVariant TDEConfigSkeleton::ItemColor::property() const
{
  return TQVariant(mReference);
}


TDEConfigSkeleton::ItemFont::ItemFont( const TQString &group, const TQString &key,
                                TQFont &reference,
                                const TQFont &defaultValue )
  : TDEConfigSkeletonGenericItem<TQFont>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemFont::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readFontEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemFont::setProperty(const TQVariant & p)
{
  mReference = p.toFont();
}

TQVariant TDEConfigSkeleton::ItemFont::property() const
{
  return TQVariant(mReference);
}


TDEConfigSkeleton::ItemRect::ItemRect( const TQString &group, const TQString &key,
                                TQRect &reference,
                                const TQRect &defaultValue )
  : TDEConfigSkeletonGenericItem<TQRect>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemRect::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readRectEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemRect::setProperty(const TQVariant & p)
{
  mReference = p.toRect();
}

TQVariant TDEConfigSkeleton::ItemRect::property() const
{
  return TQVariant(mReference);
}


TDEConfigSkeleton::ItemPoint::ItemPoint( const TQString &group, const TQString &key,
                                  TQPoint &reference,
                                  const TQPoint &defaultValue )
  : TDEConfigSkeletonGenericItem<TQPoint>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemPoint::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readPointEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemPoint::setProperty(const TQVariant & p)
{
  mReference = p.toPoint();
}

TQVariant TDEConfigSkeleton::ItemPoint::property() const
{
  return TQVariant(mReference);
}


TDEConfigSkeleton::ItemSize::ItemSize( const TQString &group, const TQString &key,
                                TQSize &reference,
                                const TQSize &defaultValue )
  : TDEConfigSkeletonGenericItem<TQSize>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemSize::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readSizeEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemSize::setProperty(const TQVariant & p)
{
  mReference = p.toSize();
}

TQVariant TDEConfigSkeleton::ItemSize::property() const
{
  return TQVariant(mReference);
}


TDEConfigSkeleton::ItemDateTime::ItemDateTime( const TQString &group, const TQString &key,
                                        TQDateTime &reference,
                                        const TQDateTime &defaultValue )
  : TDEConfigSkeletonGenericItem<TQDateTime>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemDateTime::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  mReference = config->readDateTimeEntry( mKey, &mDefault );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemDateTime::setProperty(const TQVariant & p)
{
  mReference = p.toDateTime();
}

TQVariant TDEConfigSkeleton::ItemDateTime::property() const
{
  return TQVariant(mReference);
}


TDEConfigSkeleton::ItemStringList::ItemStringList( const TQString &group, const TQString &key,
                                            TQStringList &reference,
                                            const TQStringList &defaultValue )
  : TDEConfigSkeletonGenericItem<TQStringList>( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemStringList::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readListEntry( mKey );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemStringList::setProperty(const TQVariant & p)
{
  mReference = p.toStringList();
}

TQVariant TDEConfigSkeleton::ItemStringList::property() const
{
  return TQVariant(mReference);
}


TDEConfigSkeleton::ItemPathList::ItemPathList( const TQString &group, const TQString &key,
                                            TQStringList &reference,
                                            const TQStringList &defaultValue )
  : ItemStringList( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemPathList::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readPathListEntry( mKey );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemPathList::writeConfig( TDEConfig *config )
{
  if ( mReference != mLoadedValue ) // WABA: Is this test needed?
  {
    config->setGroup( mGroup );
    if ((mDefault == mReference) && !config->hasDefault( mKey))
      config->revertToDefault( mKey );
    else {
      TQStringList sl = mReference;
      config->writePathEntry( mKey, sl );
    }
  }
}


TDEConfigSkeleton::ItemIntList::ItemIntList( const TQString &group, const TQString &key,
                                      TQValueList<int> &reference,
                                      const TQValueList<int> &defaultValue )
  : TDEConfigSkeletonGenericItem<TQValueList<int> >( group, key, reference, defaultValue )
{
}

void TDEConfigSkeleton::ItemIntList::readConfig( TDEConfig *config )
{
  config->setGroup( mGroup );
  if ( !config->hasKey( mKey ) )
    mReference = mDefault;
  else
    mReference = config->readIntListEntry( mKey );
  mLoadedValue = mReference;

  readImmutability( config );
}

void TDEConfigSkeleton::ItemIntList::setProperty(const TQVariant &)
{
  // TODO: Not yet supported
}

TQVariant TDEConfigSkeleton::ItemIntList::property() const
{
  // TODO: Not yet supported
  return TQVariant();  
}


TDEConfigSkeleton::TDEConfigSkeleton( const TQString &configname )
  : mCurrentGroup( "No Group" ), mUseDefaults(false)
{
  kdDebug(177) << "Creating TDEConfigSkeleton (" << (void *)this << ")" << endl;

  if ( !configname.isEmpty() )
  {
    mConfig = TDESharedConfig::openConfig( configname );
  }
  else
  {
    mConfig = TDEGlobal::sharedConfig();
  }
}

TDEConfigSkeleton::TDEConfigSkeleton(TDESharedConfig::Ptr config)
  : mCurrentGroup( "No Group" ), mUseDefaults(false)
{
  kdDebug(177) << "Creating TDEConfigSkeleton (" << (void *)this << ")" << endl;
  mConfig = config;
}


TDEConfigSkeleton::~TDEConfigSkeleton()
{
  TDEConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    delete *it;
  }
}

void TDEConfigSkeleton::setCurrentGroup( const TQString &group )
{
  mCurrentGroup = group;
}

TDEConfig *TDEConfigSkeleton::config() const
{
  return mConfig;
}

bool TDEConfigSkeleton::useDefaults(bool b)
{
  if (b == mUseDefaults)
     return mUseDefaults;

  mUseDefaults = b;
  TDEConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->swapDefault();
  }

  usrUseDefaults(b);
  return !mUseDefaults;
}

void TDEConfigSkeleton::setDefaults()
{
  TDEConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it ) {
    (*it)->setDefault();
  }

  usrSetDefaults();
}

void TDEConfigSkeleton::readConfig()
{
  kdDebug(177) << "TDEConfigSkeleton::readConfig()" << endl;
  
  TQString origGroup = mConfig->group();

  mConfig->reparseConfiguration();
  TDEConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->readConfig( mConfig );
  }

  usrReadConfig();
  
  mConfig->setGroup(origGroup);
}

void TDEConfigSkeleton::writeConfig()
{
  kdDebug(177) << "TDEConfigSkeleton::writeConfig()" << endl;

  TQString origGroup = mConfig->group();

  TDEConfigSkeletonItem::List::ConstIterator it;
  for( it = mItems.begin(); it != mItems.end(); ++it )
  {
    (*it)->writeConfig( mConfig );
  }

  usrWriteConfig();

  mConfig->sync();

  readConfig();

  mConfig->setGroup(origGroup);
}

void TDEConfigSkeleton::addItem( TDEConfigSkeletonItem *item, const TQString &name )
{
  item->setName( name.isEmpty() ? item->key() : name );
  mItems.append( item );
  mItemDict.insert( item->name(), item );
  item->readDefault( mConfig );
  item->readConfig( mConfig );
}

TDEConfigSkeleton::ItemString *TDEConfigSkeleton::addItemString( const TQString &name, TQString &reference,
                                     const TQString &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemString *item;
  item = new TDEConfigSkeleton::ItemString( mCurrentGroup, key.isEmpty() ? name : key,
                                          reference, defaultValue,
                                          TDEConfigSkeleton::ItemString::Normal );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemPassword *TDEConfigSkeleton::addItemPassword( const TQString &name, TQString &reference,
                                       const TQString &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemPassword *item;
  item = new TDEConfigSkeleton::ItemPassword( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemPath *TDEConfigSkeleton::addItemPath( const TQString &name, TQString &reference,
                                   const TQString &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemPath *item;
  item = new TDEConfigSkeleton::ItemPath( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemProperty *TDEConfigSkeleton::addItemProperty( const TQString &name, TQVariant &reference,
                                       const TQVariant &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemProperty *item;
  item = new TDEConfigSkeleton::ItemProperty( mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemBool *TDEConfigSkeleton::addItemBool( const TQString &name, bool &reference,
                                   bool defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemBool *item;
  item = new TDEConfigSkeleton::ItemBool( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemInt *TDEConfigSkeleton::addItemInt( const TQString &name, int &reference,
                                  int defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemInt *item;
  item = new TDEConfigSkeleton::ItemInt( mCurrentGroup, key.isNull() ? name : key,
                                       reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemUInt *TDEConfigSkeleton::addItemUInt( const TQString &name, unsigned int &reference,
                                   unsigned int defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemUInt *item;
  item = new TDEConfigSkeleton::ItemUInt( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemInt64 *TDEConfigSkeleton::addItemInt64( const TQString &name, TQ_INT64 &reference,
                                    TQ_INT64 defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemInt64 *item;
  item = new TDEConfigSkeleton::ItemInt64( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemUInt64 *TDEConfigSkeleton::addItemUInt64( const TQString &name, TQ_UINT64 &reference,
                                     TQ_UINT64 defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemUInt64 *item;
  item = new TDEConfigSkeleton::ItemUInt64( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemLong *TDEConfigSkeleton::addItemLong( const TQString &name, long &reference,
                                   long defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemLong *item;
  item = new TDEConfigSkeleton::ItemLong( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemULong *TDEConfigSkeleton::addItemULong( const TQString &name, unsigned long &reference,
                                    unsigned long defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemULong *item;
  item = new TDEConfigSkeleton::ItemULong( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemDouble *TDEConfigSkeleton::addItemDouble( const TQString &name, double &reference,
                                     double defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemDouble *item;
  item = new TDEConfigSkeleton::ItemDouble( mCurrentGroup, key.isNull() ? name : key,
                                          reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemColor *TDEConfigSkeleton::addItemColor( const TQString &name, TQColor &reference,
                                    const TQColor &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemColor *item;
  item = new TDEConfigSkeleton::ItemColor( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemFont *TDEConfigSkeleton::addItemFont( const TQString &name, TQFont &reference,
                                   const TQFont &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemFont *item;
  item = new TDEConfigSkeleton::ItemFont( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemRect *TDEConfigSkeleton::addItemRect( const TQString &name, TQRect &reference,
                                   const TQRect &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemRect *item;
  item = new TDEConfigSkeleton::ItemRect( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemPoint *TDEConfigSkeleton::addItemPoint( const TQString &name, TQPoint &reference,
                                    const TQPoint &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemPoint *item;
  item = new TDEConfigSkeleton::ItemPoint( mCurrentGroup, key.isNull() ? name : key,
                                         reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemSize *TDEConfigSkeleton::addItemSize( const TQString &name, TQSize &reference,
                                   const TQSize &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemSize *item;
  item = new TDEConfigSkeleton::ItemSize( mCurrentGroup, key.isNull() ? name : key,
                                        reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemDateTime *TDEConfigSkeleton::addItemDateTime( const TQString &name, TQDateTime &reference,
                                       const TQDateTime &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemDateTime *item;
  item = new TDEConfigSkeleton::ItemDateTime( mCurrentGroup, key.isNull() ? name : key,
                                            reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemStringList *TDEConfigSkeleton::addItemStringList( const TQString &name, TQStringList &reference,
                                         const TQStringList &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemStringList *item;
  item = new TDEConfigSkeleton::ItemStringList( mCurrentGroup, key.isNull() ? name : key,
                                              reference, defaultValue );
  addItem( item, name );
  return item;
}

TDEConfigSkeleton::ItemIntList *TDEConfigSkeleton::addItemIntList( const TQString &name, TQValueList<int> &reference,
                                      const TQValueList<int> &defaultValue, const TQString &key )
{
  TDEConfigSkeleton::ItemIntList *item;
  item = new TDEConfigSkeleton::ItemIntList( mCurrentGroup, key.isNull() ? name : key,
                                           reference, defaultValue );
  addItem( item, name );
  return item;
}

bool TDEConfigSkeleton::isImmutable(const TQString &name)
{
  TDEConfigSkeletonItem *item = findItem(name);
  return !item || item->isImmutable();
}

TDEConfigSkeletonItem *TDEConfigSkeleton::findItem(const TQString &name)
{
  return mItemDict.find(name);
}
