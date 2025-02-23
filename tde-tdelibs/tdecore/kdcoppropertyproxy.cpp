/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>

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

#include "kdcoppropertyproxy.h"

#include <tqstrlist.h>
#include <tqmetaobject.h>
#include <tqvariant.h>
#include <tqcursor.h>
#include <tqbitmap.h>
#include <tqregion.h>
#include <tqpointarray.h>
#include <tqiconset.h>
#include <tqfont.h>
#include <tqimage.h>
#include <tqbrush.h>
#include <tqpalette.h>

#include <ctype.h>
#include <assert.h>

class KDCOPPropertyProxyPrivate
{
public:
  KDCOPPropertyProxyPrivate()
  {
  }
  ~KDCOPPropertyProxyPrivate()
  {
  }

  TQObject *m_object;
};

KDCOPPropertyProxy::KDCOPPropertyProxy( TQObject *object )
{
  d = new KDCOPPropertyProxyPrivate;
  d->m_object = object;
}

KDCOPPropertyProxy::~KDCOPPropertyProxy()
{
  delete d;
}

bool KDCOPPropertyProxy::isPropertyRequest( const TQCString &fun )
{
  return isPropertyRequest( fun, d->m_object );
}

bool KDCOPPropertyProxy::processPropertyRequest( const TQCString &fun, const TQByteArray &data,
                                                 TQCString &replyType, TQByteArray &replyData )
{
  return processPropertyRequest( fun, data, replyType, replyData, d->m_object );
}

TQValueList<TQCString> KDCOPPropertyProxy::functions()
{
  return functions( d->m_object );
}

bool KDCOPPropertyProxy::isPropertyRequest( const TQCString &fun, TQObject *object )
{
  if ( fun == "property(TQCString)" ||
       fun == "setProperty(TQCString,TQVariant)" ||
       fun == "propertyNames(bool)" )
    return true;

  bool set;
  TQCString propName, arg;
  return decodePropertyRequestInternal( fun, object, set, propName, arg );
}

TQValueList<TQCString> KDCOPPropertyProxy::functions( TQObject *object )
{
  TQValueList<TQCString> res;
  res << "TQVariant property(TQCString property)";
  res << "bool setProperty(TQCString name,TQVariant property)";
  res << "TQValueList<TQCString> propertyNames(bool super)";

  TQMetaObject *metaObj = object->metaObject();
  TQStrList properties = metaObj->propertyNames( true );
  TQStrListIterator it( properties );
  for (; it.current(); ++it )
  {
    const TQMetaProperty *metaProp = metaObj->property( metaObj->findProperty( it.current(), true ), true );

    assert( metaProp );

    TQCString name = it.current();
    name.prepend( " " );
    name.prepend( metaProp->type() );
    name.append( "()" );
    res << name;

    if ( metaProp->writable() )
    {
      TQCString setName = it.current();
      setName[ 0 ] = toupper( setName[ 0 ] );
      setName = "void set" + setName + "(" + metaProp->type() + " " + it.current() + ")";
      res << setName;
    }
  }

  return res;
}

#define MARSHAL( type ) \
  case TQVariant::type: \
  { \
    reply << prop.to##type(); \
    break; \
  }

#define DEMARSHAL( type, val ) \
  case TQVariant::type: \
  { \
    val v; \
    stream >> v; \
    prop = TQVariant( v ); \
    break; \
  }

bool KDCOPPropertyProxy::processPropertyRequest( const TQCString &fun, const TQByteArray &data,
                                                 TQCString &replyType, TQByteArray &replyData,
                                                 TQObject *object )
{
  if ( fun == "property(TQCString)" )
  {
    TQCString propName;
    TQDataStream stream( data, IO_ReadOnly );
    stream >> propName;

    replyType = "TQVariant";
    TQDataStream reply( replyData, IO_WriteOnly );
    reply << object->property( propName );
    return true;
  }

  if ( fun == "setProperty(TQCString,TQVariant)" )
  {
    TQCString propName;
    TQVariant propValue;
    TQDataStream stream( data, IO_ReadOnly );
    stream >> propName >> propValue;

    replyType = "bool";
    TQDataStream reply( replyData, IO_WriteOnly );
    reply << (TQ_INT8)object->setProperty( propName, propValue );
    return true;
  }

  if ( fun == "propertyNames(bool)" )
  {
    TQ_INT8 b;
    TQDataStream stream( data, IO_ReadOnly );
    stream >> b;

    TQValueList<TQCString> res;
    TQStrList props = object->metaObject()->propertyNames( static_cast<bool>( b ) );
    TQStrListIterator it( props );
    for (; it.current(); ++it )
      res.append( it.current() );

    replyType = "TQValueList<TQCString>";
    TQDataStream reply( replyData, IO_WriteOnly );
    reply << res;
    return true;
  }

  bool set;
  TQCString propName, arg;

  bool res = decodePropertyRequestInternal( fun, object, set, propName, arg );
  if ( !res )
    return false;

  if ( set )
  {
    TQVariant prop;
    TQDataStream stream( data, IO_ReadOnly );

    TQVariant::Type type = TQVariant::nameToType( arg );
    if ( type == TQVariant::Invalid )
      return false;

    typedef TQValueList<TQVariant> ListType;
    typedef TQStringVariantMap MapType;

    switch ( type )
    {
      DEMARSHAL( Cursor, TQCursor )
      DEMARSHAL( Bitmap, TQBitmap )
      DEMARSHAL( PointArray, TQPointArray )
      DEMARSHAL( Region, TQRegion )
      DEMARSHAL( List, ListType )
      DEMARSHAL( Map, MapType )
      DEMARSHAL( String, TQString )
      DEMARSHAL( CString, TQCString )
      DEMARSHAL( StringList, TQStringList )
      DEMARSHAL( Font, TQFont )
      DEMARSHAL( Pixmap, TQPixmap )
      DEMARSHAL( Image, TQImage )
      DEMARSHAL( Brush, TQBrush )
      DEMARSHAL( Point, TQPoint )
      DEMARSHAL( Rect, TQRect )
      DEMARSHAL( Size, TQSize )
      DEMARSHAL( Color, TQColor )
      DEMARSHAL( Palette, TQPalette )
      DEMARSHAL( ColorGroup, TQColorGroup )
      case TQVariant::IconSet:
      {
        TQPixmap val;
        stream >> val;
        prop = TQVariant( TQIconSet( val ) );
      }
      break;
      DEMARSHAL( Int, int )
      DEMARSHAL( UInt, uint )
      case TQVariant::Bool:
      {
        TQ_INT8 v;
        stream >> v;
        prop = TQVariant( static_cast<bool>( v ) );
      }
        break;
      DEMARSHAL( Double, double )
      default:
        return false;
    }

    replyType = "void";
    return object->setProperty( propName, prop );
  }
  else
  {
    TQVariant prop = object->property( propName );

    if ( prop.type() == TQVariant::Invalid )
      return false;

    replyType = prop.typeName();
    TQDataStream reply( replyData, IO_WriteOnly );

    switch ( prop.type() )
    {
      MARSHAL( Cursor )
      MARSHAL( Bitmap )
      MARSHAL( PointArray )
      MARSHAL( Region )
      MARSHAL( List )
      MARSHAL( Map )
      MARSHAL( String )
      MARSHAL( CString )
      MARSHAL( StringList )
      MARSHAL( Font )
      MARSHAL( Pixmap )
      MARSHAL( Image )
      MARSHAL( Brush )
      MARSHAL( Point )
      MARSHAL( Rect )
      MARSHAL( Size )
      MARSHAL( Color )
      MARSHAL( Palette )
      MARSHAL( ColorGroup )
      case TQVariant::IconSet:
        reply << prop.toIconSet().pixmap();
        break;
      MARSHAL( Int )
      MARSHAL( UInt )
      case TQVariant::Bool:
        reply << (TQ_INT8)prop.toBool();
        break;
      MARSHAL( Double )
      default:
        return false;
    }

#undef MARSHAL
#undef DEMARSHAL

    return true;
  }

  return false;
}

bool KDCOPPropertyProxy::decodePropertyRequestInternal( const TQCString &fun, TQObject *object, bool &set,
                                                        TQCString &propName, TQCString &arg )
{
  if ( fun.length() < 3 )
    return false;

  set = false;

  propName = fun;

  if ( propName.left( 3 ) == "set" )
  {
    propName.detach();
    set = true;
    propName = propName.mid( 3 );
    int p1 = propName.find( '(' );

    uint len = propName.length();

    if ( propName[ len - 1 ] != ')' )
      return false;

    arg = propName.mid( p1+1, len - p1 - 2 );
    propName.truncate( p1 );
    propName[ 0 ] = tolower( propName[ 0 ] );
  }
  else
    propName.truncate( propName.length() - 2 );

  if ( !object->metaObject()->propertyNames( true ).contains( propName ) )
    return false;

  return true;
}
