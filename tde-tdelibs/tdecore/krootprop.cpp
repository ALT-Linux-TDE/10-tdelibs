/* This file is part of the KDE libraries
    Copyright (C) 1997 Mark Donohoe (donohoe@kde.org)

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

#include <tqwidget.h>

#include "config.h"
#ifdef TQ_WS_X11 // not needed anyway :-)

#include "krootprop.h"
#include "tdeglobal.h"
#include "tdelocale.h"
#include "kcharsets.h"
#include "tdeapplication.h"
#include <tqtextstream.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>

KRootProp::KRootProp(const TQString& rProp )
{
  atom = 0;
  dirty = false;
  setProp( rProp );
}

KRootProp::~KRootProp()
{
  sync();
  propDict.clear();
}

void KRootProp::sync()
{
  if ( !dirty )
      return;

  TQString propString;
  if ( !propDict.isEmpty() )
  {
    TQMap<TQString,TQString>::Iterator it = propDict.begin();
    TQString keyvalue;

    while ( it != propDict.end() )
    {
      keyvalue = TQString( "%1=%2\n").arg(it.key()).arg(it.data());
      propString += keyvalue;
      ++it;
    }
  }

  XChangeProperty( tqt_xdisplay(), tqt_xrootwin(), atom,
                  XA_STRING, 8, PropModeReplace,
                  (const unsigned char *)propString.utf8().data(),
                  propString.length());
  XFlush( tqt_xdisplay() );
}

void KRootProp::setProp( const TQString& rProp )
{
  Atom type;
  int format;
  unsigned long nitems;
  unsigned long bytes_after;
  long offset;

  // If a property has already been opened write
  // the dictionary back to the root window

  if( atom )
    sync();

  property_ = rProp;
  if( rProp.isEmpty() )
    return;

  atom = XInternAtom( tqt_xdisplay(), rProp.utf8(), False);

  TQString s;
  offset = 0; bytes_after = 1;
  while (bytes_after != 0)
  {
    unsigned char *buf = 0; 
    if (XGetWindowProperty( tqt_xdisplay(), tqt_xrootwin(), atom, offset, 256,
                        False, XA_STRING, &type, &format, &nitems, &bytes_after,
                        &buf) == Success && buf) 
    {
      s += TQString::fromUtf8((const char*)buf);
      offset += nitems/4;
      XFree(buf);
    }
  }

  // Parse through the property string stripping out key value pairs
  // and putting them in the dictionary

  TQString keypair;
  int i=0;
  TQString key;
  TQString value;

  while(s.length() >0 )
  {
    // parse the string for first key-value pair separator '\n'

    i = s.find("\n");
    if(i == -1)
      i = s.length();

    // extract the key-values pair and remove from string

    keypair = s.left(i);
    s.remove(0,i+1);

    // split key and value and add to dictionary

    keypair.simplifyWhiteSpace();

    i = keypair.find( "=" );
    if( i != -1 )
    {
      key = keypair.left( i );
      value = keypair.mid( i+1 );
      propDict.insert( key, value );
    }
  }
}


TQString KRootProp::prop() const
{
    return property_;
}

void KRootProp::destroy()
{
    dirty = false;
    propDict.clear();
    if( atom ) {
	XDeleteProperty( tqt_xdisplay(), tqt_xrootwin(), atom );
	atom = 0;
    }
}

TQString KRootProp::readEntry( const TQString& rKey,
			    const TQString& pDefault ) const
{
  if( propDict.contains( rKey ) )
      return propDict[ rKey ];
  else
      return pDefault;
}

int KRootProp::readNumEntry( const TQString& rKey, int nDefault ) const
{

  TQString aValue = readEntry( rKey );
  if( !aValue.isNull() )
  {
    bool ok;

    int rc = aValue.toInt( &ok );
    if (ok)
      return rc;
  }
  return nDefault;
}


TQFont KRootProp::readFontEntry( const TQString& rKey,
                                const TQFont* pDefault ) const
{
  TQFont aRetFont;
  TQFont aDefFont;

  if (pDefault)
    aDefFont = *pDefault;

  TQString aValue = readEntry( rKey );
  if( aValue.isNull() )
    return aDefFont; // Return default font

  if ( !aRetFont.fromString( aValue ) && pDefault )
    aRetFont = aDefFont;

  return aRetFont;
}


TQColor KRootProp::readColorEntry( const TQString& rKey,
								const TQColor* pDefault ) const
{
  TQColor aRetColor;
  int nRed = 0, nGreen = 0, nBlue = 0;

  if( pDefault )
    aRetColor = *pDefault;

  TQString aValue = readEntry( rKey );
  if( aValue.isNull() )
    return aRetColor;

  // Support #ffffff style color naming.
  // Help ease transistion from legacy KDE setups
  if( aValue.find("#") == 0 ) {
    aRetColor.setNamedColor( aValue );
    return aRetColor;
  }

  // Parse "red,green,blue"
  // find first comma
  int nIndex1 = aValue.find( ',' );
  if( nIndex1 == -1 )
    return aRetColor;
  // find second comma
  int nIndex2 = aValue.find( ',', nIndex1+1 );
  if( nIndex2 == -1 )
    return aRetColor;

  bool bOK;
  nRed = aValue.left( nIndex1 ).toInt( &bOK );
  nGreen = aValue.mid( nIndex1+1,
                       nIndex2-nIndex1-1 ).toInt( &bOK );
  nBlue = aValue.mid( nIndex2+1 ).toInt( &bOK );

  aRetColor.setRgb( nRed, nGreen, nBlue );

  return aRetColor;
}

TQString KRootProp::writeEntry( const TQString& rKey, const TQString& rValue )
{
    dirty = true;
    if ( propDict.contains( rKey ) ) {
	TQString aValue = propDict[ rKey ];
	propDict.replace( rKey, rValue );
	return aValue;
    }
    else {
	propDict.insert( rKey, rValue );
	return TQString::null;
    }
}

TQString KRootProp::writeEntry( const TQString& rKey, int nValue )
{
  TQString aValue;

  aValue.setNum( nValue );

  return writeEntry( rKey, aValue );
}

TQString KRootProp::writeEntry( const TQString& rKey, const TQFont& rFont )
{
  return writeEntry( rKey, TQString(rFont.toString()) );
}

TQString KRootProp::writeEntry( const TQString& rKey, const TQColor& rColor )
{
  TQString aValue = TQString( "%1,%2,%3").arg(rColor.red()).arg(rColor.green()).arg(rColor.blue() );

  return writeEntry( rKey, aValue );
}

TQString KRootProp::removeEntry(const TQString& rKey)
{
    if (propDict.contains(rKey)) {
	dirty = true;
	TQString aValue = propDict[rKey];
	propDict.remove(rKey);
	return aValue;
    } else
	return TQString::null;
}

TQStringList KRootProp::listEntries() const
{
    TQMap<TQString,TQString>::ConstIterator it;
    TQStringList list;

	TQMap<TQString,TQString>::ConstIterator end(propDict.end());
    for (it=propDict.begin(); it!=end; ++it)
	list += it.key();

    return list;
}
#endif
