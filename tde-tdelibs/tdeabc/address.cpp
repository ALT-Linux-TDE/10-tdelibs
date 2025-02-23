/*
    This file is part of libtdeabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include "address.h"

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

#include <tqfile.h>

using namespace TDEABC;

TQMap<TQString, TQString> *Address::mISOMap = 0;
static KStaticDeleter< TQMap<TQString, TQString> > isoMapDeleter;

Address::Address() :
  mEmpty( true ), mType( 0 )
{
  mId = TDEApplication::randomString( 10 );
}

Address::Address( int type ) :
  mEmpty( true ), mType( type )
{
  mId = TDEApplication::randomString( 10 );
}

bool Address::operator==( const Address &a ) const
{
  if ( mPostOfficeBox != a.mPostOfficeBox ) return false;
  if ( mExtended != a.mExtended ) return false;
  if ( mStreet != a.mStreet ) return false;
  if ( mLocality != a.mLocality ) return false;
  if ( mRegion != a.mRegion ) return false;
  if ( mPostalCode != a.mPostalCode ) return false;
  if ( mCountry != a.mCountry ) return false;
  if ( mLabel != a.mLabel ) return false;
  
  return true;
}

bool Address::operator!=( const Address &a ) const
{
  return !( a == *this );
}

bool Address::isEmpty() const
{
  if ( mPostOfficeBox.isEmpty() &&
       mExtended.isEmpty() &&
       mStreet.isEmpty() &&
       mLocality.isEmpty() &&
       mRegion.isEmpty() &&
       mPostalCode.isEmpty() &&
       mCountry.isEmpty() &&
       mLabel.isEmpty() ) {
    return true;
  }
  return false;
}

void Address::clear()
{
  *this = Address();
}

void Address::setId( const TQString &id )
{
  mEmpty = false;

  mId = id;
}

TQString Address::id() const
{
  return mId;
}

void Address::setType( int type )
{
  mEmpty = false;

  mType = type;
}

int Address::type() const
{
  return mType;
}

TQString Address::typeLabel() const
{
  TQString label;
  bool first = true;

  const TypeList list = typeList();

  TypeList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it ) {
    if ( ( type() & (*it) ) && ( (*it) != Pref ) ) {
      label.append( ( first ? "" : "/" ) + typeLabel( *it ) );
      if ( first )
        first = false;
    }
  }

  return label;
}

void Address::setPostOfficeBox( const TQString &s )
{
  mEmpty = false;

  mPostOfficeBox = s;
}

TQString Address::postOfficeBox() const
{
  return mPostOfficeBox;
}

TQString Address::postOfficeBoxLabel()
{
  return i18n("Post Office Box");
}


void Address::setExtended( const TQString &s )
{
  mEmpty = false;

  mExtended = s;
}

TQString Address::extended() const
{
  return mExtended;
}

TQString Address::extendedLabel()
{
  return i18n("Extended Address Information");
}


void Address::setStreet( const TQString &s )
{
  mEmpty = false;

  mStreet = s;
}

TQString Address::street() const
{
  return mStreet;
}

TQString Address::streetLabel()
{
  return i18n("Street");
}


void Address::setLocality( const TQString &s )
{
  mEmpty = false;

  mLocality = s;
}

TQString Address::locality() const
{
  return mLocality;
}

TQString Address::localityLabel()
{
  return i18n("Locality");
}


void Address::setRegion( const TQString &s )
{
  mEmpty = false;

  mRegion = s;
}

TQString Address::region() const
{
  return mRegion;
}

TQString Address::regionLabel()
{
  return i18n("Region");
}


void Address::setPostalCode( const TQString &s )
{
  mEmpty = false;

  mPostalCode = s;
}

TQString Address::postalCode() const
{
  return mPostalCode;
}

TQString Address::postalCodeLabel()
{
  return i18n("Postal Code");
}


void Address::setCountry( const TQString &s )
{
  mEmpty = false;

  mCountry = s;
}

TQString Address::country() const
{
  return mCountry;
}

TQString Address::countryLabel()
{
  return i18n("Country");
}


void Address::setLabel( const TQString &s )
{
  mEmpty = false;

  mLabel = s;
}

TQString Address::label() const
{
  return mLabel;
}

TQString Address::labelLabel()
{
  return i18n("Delivery Label");
}

Address::TypeList Address::typeList()
{
  static TypeList list;

  if ( list.isEmpty() )
    list << Dom << Intl << Postal << Parcel << Home << Work << Pref;

  return list;
}

TQString Address::typeLabel( int type )
{
  if ( type & Pref )
    return i18n( "Preferred address", "Preferred" );

  switch ( type ) {
    case Dom:
      return i18n("Domestic");
      break;
    case Intl:
      return i18n("International");
      break;
    case Postal:
      return i18n("Postal");
      break;
    case Parcel:
      return i18n("Parcel");
      break;
    case Home:
      return i18n("Home Address", "Home");
      break;
    case Work:
      return i18n("Work Address", "Work");
      break;
    case Pref:
      return i18n("Preferred Address");
      break;
    default:
      return i18n("Other");
      break;
  }
}

void Address::dump() const
{
  kdDebug(5700) << "  Address {" << endl;
  kdDebug(5700) << "    Id: " << id() << endl;
  kdDebug(5700) << "    Extended: " << extended() << endl;
  kdDebug(5700) << "    Street: " << street() << endl;
  kdDebug(5700) << "    Postal Code: " << postalCode() << endl;
  kdDebug(5700) << "    Locality: " << locality() << endl;
  kdDebug(5700) << "  }" << endl;
}


TQString Address::formattedAddress( const TQString &realName,
                                   const TQString &orgaName ) const
{
  TQString ciso;
  TQString addrTemplate;
  TQString ret;

  // FIXME: first check for iso-country-field and prefer that one
  if ( !country().isEmpty() ) {
    ciso = countryToISO( country() );
  } else {
    // fall back to our own country
    ciso = TDEGlobal::locale()->country();
  }
  KSimpleConfig entry( locate( "locale", 
        TQString( "l10n/" ) + ciso + TQString( "/entry.desktop" ) ) );
  entry.setGroup( "KCM Locale" );

  // decide whether this needs special business address formatting
  if ( orgaName.isEmpty() ) {
    addrTemplate = entry.readEntry( "AddressFormat" );
  } else {
    addrTemplate = entry.readEntry( "BusinessAddressFormat" );
    if ( addrTemplate.isEmpty() )
      addrTemplate = entry.readEntry( "AddressFormat" );
  }

  // in the case there's no format found at all, default to what we've always
  // used:
  if ( addrTemplate.isEmpty() ) {
    kdWarning(5700) << "address format database incomplete "
        << "(no format for locale " << ciso 
        << " found). Using default address formatting." << endl;
    addrTemplate = "%0(%n\\n)%0(%cm\\n)%0(%s\\n)%0(PO BOX %p\\n)%0(%l%w%r)%,%z";
  }

  // scan
  parseAddressTemplateSection( addrTemplate, ret, realName, orgaName );

  // now add the country line if needed (formatting this time according to
  // the rules of our own system country )
  if ( !country().isEmpty() ) {
    KSimpleConfig entry( locate( "locale", TQString( "l10n/" )
          + TDEGlobal::locale()->country() + TQString( "/entry.desktop" ) ) );
    entry.setGroup( "KCM Locale" );
    TQString cpos = entry.readEntry( "AddressCountryPosition" );
    if ( "BELOW" == cpos || cpos.isEmpty() ) {
      ret = ret + "\n\n" + country().upper();
    } else if ( "below" == cpos ) {
      ret = ret + "\n\n" + country();
    } else if ( "ABOVE" == cpos ) {
      ret = country().upper() + "\n\n" + ret;
    } else if ( "above" == cpos ) {
      ret = country() + "\n\n" + ret;
    }
  }
  
  return ret;
}

bool Address::parseAddressTemplateSection( const TQString &tsection, 
    TQString &result, const TQString &realName, const TQString &orgaName ) const
{
  // This method first parses and substitutes any bracketed sections and
  // after that replaces any tags with their values. If a bracketed section
  // or a tag evaluate to zero, they are not just removed but replaced
  // with a placeholder. This is because in the last step conditionals are
  // resolved which depend on information about zero-evaluations.
  result = tsection;
  int stpos = 0;
  bool ret = false;
  
  // first check for brackets that have to be evaluated first 
  int fpos = result.find( KABC_FMTTAG_purgeempty, stpos );
  while ( -1 != fpos ) {
    int bpos1 = fpos + KABC_FMTTAG_purgeempty.length();
    int bpos2;
    // expect opening bracket and find next balanced closing bracket. If 
    // next char is no opening bracket, continue parsing (no valid tag)
    if ( '(' == result[bpos1] ) {
      bpos2 = findBalancedBracket( result, bpos1 );
      if ( -1 != bpos2 ) {
        // we have balanced brackets, recursively parse:
        TQString rplstr;
        bool purge = !parseAddressTemplateSection( result.mid( bpos1+1,
                                                   bpos2-bpos1-1 ), rplstr,
                                                   realName, orgaName );
        if ( purge ) {
          // purge -> remove all
          // replace with !_P_!, so conditional tags work later
          result.replace( fpos, bpos2 - fpos + 1, "!_P_!" );
          // leave stpos as it is
        } else {
          // no purge -> replace with recursively parsed string
          result.replace( fpos, bpos2 - fpos + 1, rplstr );
          ret = true;
          stpos = fpos + rplstr.length();
        }
      } else {
        // unbalanced brackets:  keep on parsing (should not happen 
        // and will result in bad formatting)
        stpos = bpos1; 
      }
    }
    fpos = result.find( KABC_FMTTAG_purgeempty, stpos );
  }

  // after sorting out all purge tags, we just search'n'replace the rest,
  // keeping track of whether at least one tag evaluates to something.
  // The following macro needs TQString for R_FIELD
  // It substitutes !_P_! for empty fields so conditional tags work later
#define REPLTAG(R_TAG,R_FIELD) \
  if ( result.find(R_TAG, false) != -1 ) { \
    TQString rpl = R_FIELD.isEmpty() ? TQString("!_P_!") : R_FIELD; \
    result.replace( R_TAG, rpl ); \
    if ( !R_FIELD.isEmpty() ) { \
      ret = true; \
    } \
  }
  REPLTAG( KABC_FMTTAG_realname, realName );
  REPLTAG( KABC_FMTTAG_REALNAME, realName.upper() );
  REPLTAG( KABC_FMTTAG_company, orgaName );
  REPLTAG( KABC_FMTTAG_COMPANY, orgaName.upper() );
  REPLTAG( KABC_FMTTAG_pobox, postOfficeBox() );
  REPLTAG( KABC_FMTTAG_street, street() );
  REPLTAG( KABC_FMTTAG_STREET, street().upper() );
  REPLTAG( KABC_FMTTAG_zipcode, postalCode() );
  REPLTAG( KABC_FMTTAG_location, locality() );
  REPLTAG( KABC_FMTTAG_LOCATION, locality().upper() );
  REPLTAG( KABC_FMTTAG_region, region() );
  REPLTAG( KABC_FMTTAG_REGION, region().upper() );
  result.replace( KABC_FMTTAG_newline, "\n" );
#undef REPLTAG
 
  // conditional comma 
  fpos = result.find( KABC_FMTTAG_condcomma, 0 );
  while ( -1 != fpos ) {
    TQString str1 = result.mid( fpos - 5, 5 );
    TQString str2 = result.mid( fpos + 2, 5 );
    if ( str1 != "!_P_!" && str2 != "!_P_!" ) {
      result.replace( fpos, 2, ", " );
    } else {
      result.remove( fpos, 2 );
    }
    fpos = result.find( KABC_FMTTAG_condcomma, fpos );
  }
  // conditional whitespace
  fpos = result.find( KABC_FMTTAG_condwhite, 0 );
  while ( -1 != fpos ) {
    TQString str1 = result.mid( fpos - 5, 5 );
    TQString str2 = result.mid( fpos + 2, 5 );
    if ( str1 != "!_P_!" && str2 != "!_P_!" ) {
      result.replace( fpos, 2, " " );
    } else {
      result.remove( fpos, 2 );
    }
    fpos = result.find( KABC_FMTTAG_condwhite, fpos );
  }

  // remove purged:
  result.remove( "!_P_!" );

  return ret;
}

int Address::findBalancedBracket( const TQString &tsection, int pos ) const
{
  int balancecounter = 0;
  for( unsigned int i = pos + 1; i < tsection.length(); i++ ) {
    if ( ')' == tsection[i] && 0 == balancecounter ) {
      // found end of brackets
      return i;
    } else
    if ( '(' == tsection[i] ) {
      // nested brackets
      balancecounter++;
    }
  }
  return -1;
}

TQString Address::countryToISO( const TQString &cname )
{
  // we search a map file for translations from country names to
  // iso codes, storing caching things in a TQMap for faster future 
  // access.
  if ( !mISOMap )
    isoMapDeleter.setObject( mISOMap, new TQMap<TQString, TQString>() );

  TQMap<TQString, TQString>::ConstIterator it;
  it = mISOMap->find( cname );
  if ( it != mISOMap->end() )
    return it.data();

  TQString mapfile = TDEGlobal::dirs()->findResource( "data", 
          TQString::fromLatin1( "tdeabc/countrytransl.map" ) );

  TQFile file( mapfile );
  if ( file.open( IO_ReadOnly ) ) {
    TQTextStream s( &file );
    TQString strbuf = s.readLine();
    while( !strbuf.isEmpty() ) {
      TQStringList countryInfo = TQStringList::split( '\t', strbuf, true );
      if ( countryInfo[ 0 ] == cname ) {
        file.close();
        mISOMap->insert( cname, countryInfo[ 1 ] );
        return countryInfo[ 1 ];
      }
      strbuf = s.readLine();
    }
    file.close();
  }
  
  // fall back to system country
  mISOMap->insert( cname, TDEGlobal::locale()->country() );
  return TDEGlobal::locale()->country();
}

TQString Address::ISOtoCountry( const TQString &ISOname )
{
  // get country name from ISO country code (e.g. "no" -> i18n("Norway"))
  if ( ISOname.simplifyWhiteSpace().isEmpty() )
    return TQString::null;

  TQString mapfile = TDEGlobal::dirs()->findResource( "data", 
          TQString::fromLatin1( "tdeabc/countrytransl.map" ) );

  TQFile file( mapfile );
  if ( file.open( IO_ReadOnly ) ) {
    TQTextStream s( &file );
    TQString searchStr = "\t" + ISOname.simplifyWhiteSpace().lower();
    TQString strbuf = s.readLine();
    int pos;
    while ( !strbuf.isEmpty() ) {
      if ( (pos = strbuf.find( searchStr )) != -1 ) {
        file.close();
        return i18n( strbuf.left( pos ).utf8() );
      }
      strbuf = s.readLine();
    }
    file.close();
  }

  return ISOname;
}

TQDataStream &TDEABC::operator<<( TQDataStream &s, const Address &addr )
{
    return s << addr.mId << addr.mType << addr.mPostOfficeBox <<
	    addr.mExtended << addr.mStreet << addr.mLocality <<
	    addr.mRegion << addr.mPostalCode << addr.mCountry <<
	    addr.mLabel;
}

TQDataStream &TDEABC::operator>>( TQDataStream &s, Address &addr )
{
    s >> addr.mId >> addr.mType >> addr.mPostOfficeBox >> addr.mExtended >>
	    addr.mStreet >> addr.mLocality >> addr.mRegion >>
	    addr.mPostalCode >> addr.mCountry >> addr.mLabel;

    addr.mEmpty = false;

    return s;
}
