/*
    This file is part of libtdeabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
    Copyright (c) 2005 Ingo Kloecker <kloecker@kde.org>

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

#include <tqregexp.h>

#include <ksharedptr.h>
#include <kdebug.h>
#include <tdeapplication.h>
#include <tdelocale.h>

#include "addresseehelper.h"
#include "field.h"
#include "resource.h"
#include "sortmode.h"

#include "addressee.h"

using namespace TDEABC;

static bool matchBinaryPattern( int value, int pattern );

template <class L>
static bool listEquals( const TQValueList<L>&, const TQValueList<L>& );
static bool emailsEquals( const TQStringList&, const TQStringList& );

TDEABC::SortMode *Addressee::mSortMode = 0;

struct Addressee::AddresseeData : public TDEShared
{
  TQString uid;
  TQString uri;
  --VARIABLES--

  PhoneNumber::List phoneNumbers;
  Address::List addresses;
  Key::List keys;
  TQStringList emails;
  TQStringList categories;
  TQStringList custom;

  Resource *resource;

  bool empty    :1;
  bool changed  :1;
};

Addressee::AddresseeData* Addressee::shared_null = 0;

Addressee::AddresseeData* Addressee::makeSharedNull()
{
  Addressee::shared_null = new AddresseeData;
  shared_null->_TDEShared_ref(); //just in case (we should add KSD)
  shared_null->empty = true;
  shared_null->changed = false;
  shared_null->resource = 0;
  return shared_null;
}

Addressee::Addressee()
{
  mData = shared_null ? shared_null : makeSharedNull();
}

Addressee::~Addressee()
{
}

Addressee::Addressee( const Addressee &a )
{
  mData = a.mData;
}

Addressee &Addressee::operator=( const Addressee &a )
{
  if ( this == &a )
    return (*this);

  mData = a.mData;
  return (*this);
}

void Addressee::detach()
{
  if ( mData.data() == shared_null ) {
    mData = new AddresseeData;
    mData->empty = true;
    mData->changed = false;
    mData->resource = 0;
    mData->uid = TDEApplication::randomString( 10 );
    return;
  } else if ( mData.count() == 1 ) return;

  AddresseeData data = *mData;
  mData = new AddresseeData( data );
}

bool Addressee::operator==( const Addressee &a ) const
{
  if ( uid() != a.uid() ) {
    kdDebug(5700) << "uid differs" << endl;
    return false;
  }
  --EQUALSTEST--
  if ( ( mData->url.isValid() || a.mData->url.isValid() ) &&
       ( mData->url != a.mData->url ) ) {
    kdDebug(5700) << "url differs" << endl;
    return false;
  }
  if ( !listEquals( mData->phoneNumbers, a.mData->phoneNumbers ) ) {
    kdDebug(5700) << "phoneNumbers differs" << endl;
    return false;
  }
  if ( !listEquals( mData->addresses, a.mData->addresses ) ) {
    kdDebug(5700) << "addresses differs" << endl;
    return false;
  }
  if ( !listEquals( mData->keys, a.mData->keys ) ) {
    kdDebug(5700) << "keys differs" << endl;
    return false;
  }
  if ( !emailsEquals( mData->emails, a.mData->emails ) ) {
    kdDebug(5700) << "emails differs" << endl;
    return false;
  }
  if ( !listEquals( mData->categories, a.mData->categories ) ) {
    kdDebug(5700) << "categories differs" << endl;
    return false;
  }
  if ( !listEquals( mData->custom, a.mData->custom ) ) {
    kdDebug(5700) << "custom differs" << endl;
    return false;
  }

  return true;
}

bool Addressee::operator!=( const Addressee &a ) const
{
  return !( a == *this );
}

bool Addressee::isEmpty() const
{
  return mData->empty;
}

void Addressee::setUid( const TQString &id )
{
  if ( id == mData->uid ) return;
  detach();
  mData->empty = false;
  mData->uid = id;
}

TQString Addressee::uid() const
{
  return mData->uid;
}

TQString Addressee::uidLabel()
{
  return i18n("Unique Identifier");
}

void Addressee::setUri( const TQString &id )
{
  if ( id == mData->uri ) return;
  detach();
  mData->empty = false;
  mData->uri = id;
}

TQString Addressee::uri() const
{
  return mData->uri;
}

TQString Addressee::uriLabel()
{
  return i18n("Unique Resource Identifier");
}

--DEFINITIONS--

void Addressee::setNameFromString( const TQString &s )
{
  TQString str = s;
  //remove enclosing quotes from string
  if ( str.length() > 1  && s[ 0 ] == '"' && s[ s.length() - 1 ] == '"' )
    str = s.mid( 1, s.length() - 2 );

  setFormattedName( str );
  setName( str );

  // clear all name parts
  setPrefix( TQString() );
  setGivenName( TQString() );
  setAdditionalName( TQString() );
  setFamilyName( TQString() );
  setSuffix( TQString() );

  if ( str.isEmpty() )
    return;

  TQString spaceStr = " ";
  TQString emptyStr = "";
  AddresseeHelper *helper = AddresseeHelper::self();

  int i = str.find( ',' );
  if( i < 0 ) {
    TQStringList parts = TQStringList::split( spaceStr, str );
    int leftOffset = 0;
    int rightOffset = parts.count() - 1;

    TQString suffix;
    while ( rightOffset >= 0 ) {
      if ( helper->containsSuffix( parts[ rightOffset ] ) ) {
        suffix.prepend(parts[ rightOffset ] + (suffix.isEmpty() ? emptyStr : spaceStr));
        rightOffset--;
      } else
        break;
    }
    setSuffix( suffix );

    if ( rightOffset < 0 )
      return;

    TQStringList inclusionList;
    for ( int n = 1; (rightOffset - n >= 0) && (n < 4); ++n ) {
      if ( helper->containsPrefix( parts[ rightOffset - n ].lower() ) ) {
        inclusionList.prepend( parts[ rightOffset - n ] );
      } else
        break;
    }

    if ( !inclusionList.isEmpty() ) {
      setFamilyName( inclusionList.join( " " ) + spaceStr + parts[ rightOffset ] );
      rightOffset -= inclusionList.count();
    } else {
      if ( helper->tradeAsFamilyName() )
        setFamilyName( parts[ rightOffset ] );
      else
        setGivenName( parts[ rightOffset ] );
    }

    TQString prefix;
    while ( leftOffset < rightOffset ) {
      if ( helper->containsTitle( parts[ leftOffset ] ) ) {
        prefix.append( ( prefix.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
        leftOffset++;
      } else
        break;
    }
    setPrefix( prefix );

    if ( leftOffset < rightOffset ) {
      setGivenName( parts[ leftOffset ] );
      leftOffset++;
    }

    TQString additionalName;
    while ( leftOffset < rightOffset ) {
      additionalName.append( ( additionalName.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
      leftOffset++;
    }
    setAdditionalName( additionalName );
  } else {
    TQString part1 = str.left( i );
    TQString part2 = str.mid( i + 1 );

    TQStringList parts = TQStringList::split( spaceStr, part1 );
    int leftOffset = 0;
    int rightOffset = parts.count() - 1;

    if ( parts.count() > 0 ) {

      TQString suffix;
      while ( rightOffset >= 0 ) {
        if ( helper->containsSuffix( parts[ rightOffset ] ) ) {
          suffix.prepend(parts[ rightOffset ] + (suffix.isEmpty() ? emptyStr : spaceStr));
          rightOffset--;
        } else
          break;
      }
      setSuffix( suffix );

      if ( rightOffset - 1 >= 0 && helper->containsPrefix( parts[ rightOffset - 1 ].lower() ) ) {
        setFamilyName( parts[ rightOffset - 1 ] + spaceStr + parts[ rightOffset ] );
        rightOffset--;
      } else
        setFamilyName( parts[ rightOffset ] );

      TQString prefix;
      while ( leftOffset < rightOffset ) {
        if ( helper->containsTitle( parts[ leftOffset ] ) ) {
          prefix.append( ( prefix.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
          leftOffset++;
        } else
          break;
      }
    } else {
      setPrefix( "" );
      setFamilyName( "" );
      setSuffix( "" );
    }

    parts = TQStringList::split( spaceStr, part2 );

    leftOffset = 0;
    rightOffset = parts.count();

    if ( parts.count() > 0 ) {

      TQString prefix;
      while ( leftOffset < rightOffset ) {
        if ( helper->containsTitle( parts[ leftOffset ] ) ) {
          prefix.append( ( prefix.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
          leftOffset++;
        } else
          break;
      }
      setPrefix( prefix );

      if ( leftOffset < rightOffset ) {
        setGivenName( parts[ leftOffset ] );
        leftOffset++;
      }

      TQString additionalName;
      while ( leftOffset < rightOffset ) {
        additionalName.append( ( additionalName.isEmpty() ? emptyStr : spaceStr) + parts[ leftOffset ] );
        leftOffset++;
      }
      setAdditionalName( additionalName );
    } else {
      setGivenName( "" );
      setAdditionalName( "" );
    }
  }
}

TQString Addressee::realName() const
{
  TQString n( formattedName() );
  if ( !n.isEmpty() )
	return n;

  n = assembledName();
  if ( !n.isEmpty() )
	return n;

  n = name();
  if ( !n.isEmpty() )
	return n;

  return organization();
}

TQString Addressee::assembledName() const
{
  TQString name = prefix() + " " + givenName() + " " + additionalName() + " " +
              familyName() + " " + suffix();

  return name.simplifyWhiteSpace();
}

TQString Addressee::fullEmail( const TQString &email ) const
{
  TQString e;
  if ( email.isNull() ) {
    e = preferredEmail();
  } else {
    e = email;
  }
  if ( e.isEmpty() ) return TQString();

  TQString text;
  if ( realName().isEmpty() )
    text = e;
  else {
    TQRegExp needQuotes( "[^ 0-9A-Za-z\\x0080-\\xFFFF]" );
    if ( realName().find( needQuotes ) != -1 ) {
      TQString name = realName();
      name.replace( "\"", "\\\"" );
      text = "\"" + name + "\" <" + e + ">";
    } else
      text = realName() + " <" + e + ">";
  }

  return text;
}

void Addressee::insertEmail( const TQString &email, bool preferred )
{
  if ( email.simplifyWhiteSpace().isEmpty() )
    return;

  detach();
  mData->empty = false;

  TQStringList::Iterator it = mData->emails.find( email );

  if ( it != mData->emails.end() ) {
    if ( !preferred || it == mData->emails.begin() ) return;
    mData->emails.remove( it );
    mData->emails.prepend( email );
  } else {
    if ( preferred ) {
      mData->emails.prepend( email );
    } else {
      mData->emails.append( email );
    }
  }
}

void Addressee::removeEmail( const TQString &email )
{
  detach();

  TQStringList::Iterator it = mData->emails.find( email );
  if ( it == mData->emails.end() ) return;

  mData->emails.remove( it );
}

TQString Addressee::preferredEmail() const
{
  if ( mData->emails.count() == 0 ) return TQString();
  else return mData->emails.first();
}

TQStringList Addressee::emails() const
{
  return mData->emails;
}
void Addressee::setEmails( const TQStringList& emails ) {
  detach();

  mData->emails = emails;
}
void Addressee::insertPhoneNumber( const PhoneNumber &phoneNumber )
{
  detach();
  mData->empty = false;

  PhoneNumber::List::Iterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == phoneNumber.id() ) {
      *it = phoneNumber;
      return;
    }
  }
  if ( !phoneNumber.number().simplifyWhiteSpace().isEmpty() )
    mData->phoneNumbers.append( phoneNumber );
}

void Addressee::removePhoneNumber( const PhoneNumber &phoneNumber )
{
  detach();

  PhoneNumber::List::Iterator it;
  for( it = mData->phoneNumbers.begin(); it != mData->phoneNumbers.end(); ++it ) {
    if ( (*it).id() == phoneNumber.id() ) {
      mData->phoneNumbers.remove( it );
      return;
    }
  }
}

PhoneNumber Addressee::phoneNumber( int type ) const
{
  PhoneNumber phoneNumber( "", type );
  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.constBegin(); it != mData->phoneNumbers.constEnd(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type ) ) {
      if ( (*it).type() & PhoneNumber::Pref )
        return (*it);
      else if ( phoneNumber.number().isEmpty() )
        phoneNumber = (*it);
    }
  }

  return phoneNumber;
}

PhoneNumber::List Addressee::phoneNumbers() const
{
  return mData->phoneNumbers;
}

PhoneNumber::List Addressee::phoneNumbers( int type ) const
{
  PhoneNumber::List list;

  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.constBegin(); it != mData->phoneNumbers.constEnd(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type ) ) {
      list.append( *it );
    }
  }
  return list;
}

PhoneNumber Addressee::findPhoneNumber( const TQString &id ) const
{
  PhoneNumber::List::ConstIterator it;
  for( it = mData->phoneNumbers.constBegin(); it != mData->phoneNumbers.constEnd(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return PhoneNumber();
}

void Addressee::insertKey( const Key &key )
{
  detach();
  mData->empty = false;

  Key::List::Iterator it;
  for( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).id() == key.id() ) {
      *it = key;
      return;
    }
  }
  mData->keys.append( key );
}

void Addressee::removeKey( const Key &key )
{
  detach();

  Key::List::Iterator it;
  for( it = mData->keys.begin(); it != mData->keys.end(); ++it ) {
    if ( (*it).id() == key.id() ) {
      mData->keys.remove( key );
      return;
    }
  }
}

Key Addressee::key( int type, TQString customTypeString ) const
{
  Key::List::ConstIterator it;
  for( it = mData->keys.constBegin(); it != mData->keys.constEnd(); ++it ) {
    if ( (*it).type() == type ) {
      if ( type == Key::Custom ) {
        if ( customTypeString.isEmpty() ) {
          return *it;
        } else {
          if ( (*it).customTypeString() == customTypeString )
            return (*it);
        }
      } else {
        return *it;
      }
    }
  }
  return Key( TQString(), type );
}

void Addressee::setKeys( const Key::List& list )
{
  detach();
  mData->keys = list;
}

Key::List Addressee::keys() const
{
  return mData->keys;
}

Key::List Addressee::keys( int type, TQString customTypeString ) const
{
  Key::List list;

  Key::List::ConstIterator it;
  for( it = mData->keys.constBegin(); it != mData->keys.constEnd(); ++it ) {
    if ( (*it).type() == type ) {
      if ( type == Key::Custom ) {
        if ( customTypeString.isEmpty() ) {
          list.append( *it );
        } else {
          if ( (*it).customTypeString() == customTypeString )
            list.append( *it );
        }
      } else {
        list.append( *it );
      }
    }
  }
  return list;
}

Key Addressee::findKey( const TQString &id ) const
{
  Key::List::ConstIterator it;
  for( it = mData->keys.constBegin(); it != mData->keys.constEnd(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return Key();
}

TQString Addressee::asString() const
{
  return "Smith, agent Smith...";
}

void Addressee::dump() const
{
  kdDebug(5700) << "Addressee {" << endl;

  kdDebug(5700) << "  Uid: '" << uid() << "'" << endl;

  --DEBUG--

  kdDebug(5700) << "  Emails {" << endl;
  const TQStringList e = emails();
  TQStringList::ConstIterator it;
  for( it = e.begin(); it != e.end(); ++it ) {
    kdDebug(5700) << "    " << (*it) << endl;
  }
  kdDebug(5700) << "  }" << endl;

  kdDebug(5700) << "  PhoneNumbers {" << endl;
  const PhoneNumber::List p = phoneNumbers();
  PhoneNumber::List::ConstIterator it2;
  for( it2 = p.begin(); it2 != p.end(); ++it2 ) {
    kdDebug(5700) << "    Type: " << int((*it2).type()) << " Number: " << (*it2).number() << endl;
  }
  kdDebug(5700) << "  }" << endl;

  const Address::List a = addresses();
  Address::List::ConstIterator it3;
  for( it3 = a.begin(); it3 != a.end(); ++it3 ) {
    (*it3).dump();
  }

  kdDebug(5700) << "  Keys {" << endl;
  const Key::List k = keys();
  Key::List::ConstIterator it4;
  for( it4 = k.begin(); it4 != k.end(); ++it4 ) {
    kdDebug(5700) << "    Type: " << int((*it4).type()) <<
                     " Key: " << (*it4).textData() <<
                     " CustomString: " << (*it4).customTypeString() << endl;
  }
  kdDebug(5700) << "  }" << endl;

  kdDebug(5700) << "}" << endl;
}


void Addressee::insertAddress( const Address &address )
{
  if ( address.isEmpty() )
    return;

  detach();
  mData->empty = false;

  Address::List::Iterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == address.id() ) {
      *it = address;
      return;
    }
  }

  mData->addresses.append( address );
}

void Addressee::removeAddress( const Address &address )
{
  detach();

  Address::List::Iterator it;
  for( it = mData->addresses.begin(); it != mData->addresses.end(); ++it ) {
    if ( (*it).id() == address.id() ) {
      mData->addresses.remove( it );
      return;
    }
  }
}

Address Addressee::address( int type ) const
{
  Address address( type );
  Address::List::ConstIterator it;
  for( it = mData->addresses.constBegin(); it != mData->addresses.constEnd(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type ) ) {
      if ( (*it).type() & Address::Pref )
        return (*it);
      else if ( address.isEmpty() )
        address = (*it);
    }
  }

  return address;
}

Address::List Addressee::addresses() const
{
  return mData->addresses;
}

Address::List Addressee::addresses( int type ) const
{
  Address::List list;

  Address::List::ConstIterator it;
  for( it = mData->addresses.constBegin(); it != mData->addresses.constEnd(); ++it ) {
    if ( matchBinaryPattern( (*it).type(), type ) ) {
      list.append( *it );
    }
  }

  return list;
}

Address Addressee::findAddress( const TQString &id ) const
{
  Address::List::ConstIterator it;
  for( it = mData->addresses.constBegin(); it != mData->addresses.constEnd(); ++it ) {
    if ( (*it).id() == id ) {
      return *it;
    }
  }
  return Address();
}

void Addressee::insertCategory( const TQString &c )
{
  detach();
  mData->empty = false;

  if ( mData->categories.findIndex( c ) != -1 ) return;

  mData->categories.append( c );
}

void Addressee::removeCategory( const TQString &c )
{
  detach();

  TQStringList::Iterator it = mData->categories.find( c );
  if ( it == mData->categories.end() ) return;

  mData->categories.remove( it );
}

bool Addressee::hasCategory( const TQString &c ) const
{
  return ( mData->categories.findIndex( c ) != -1 );
}

void Addressee::setCategories( const TQStringList &c )
{
  detach();
  mData->empty = false;

  mData->categories = c;
}

TQStringList Addressee::categories() const
{
  return mData->categories;
}

void Addressee::insertCustom( const TQString &app, const TQString &name,
                              const TQString &value )
{
  if ( value.isEmpty() || name.isEmpty() || app.isEmpty() ) return;

  detach();
  mData->empty = false;

  TQString qualifiedName = app + "-" + name + ":";

  TQStringList::Iterator it;
  for( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      (*it) = qualifiedName + value;
      return;
    }
  }

  mData->custom.append( qualifiedName + value );
}

void Addressee::removeCustom( const TQString &app, const TQString &name)
{
  detach();

  TQString qualifiedName = app + "-" + name + ":";

  TQStringList::Iterator it;
  for( it = mData->custom.begin(); it != mData->custom.end(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      mData->custom.remove( it );
      return;
    }
  }
}

TQString Addressee::custom( const TQString &app, const TQString &name ) const
{
  TQString qualifiedName = app + "-" + name + ":";
  TQString value;

  TQStringList::ConstIterator it;
  for( it = mData->custom.constBegin(); it != mData->custom.constEnd(); ++it ) {
    if ( (*it).startsWith( qualifiedName ) ) {
      value = (*it).mid( (*it).find( ":" ) + 1 );
      break;
    }
  }

  return value;
}

void Addressee::setCustoms( const TQStringList &l )
{
  detach();
  mData->empty = false;

  mData->custom = l;
}

TQStringList Addressee::customs() const
{
  return mData->custom;
}

void Addressee::parseEmailAddress( const TQString &rawEmail, TQString &fullName,
                                   TQString &email)
{
  // This is a simplified version of KPIM::splitAddress().

  fullName = "";
  email = "";
  if ( rawEmail.isEmpty() )
    return; // KPIM::AddressEmpty;

  // The code works on 8-bit strings, so convert the input to UTF-8.
  TQCString address = rawEmail.utf8();

  TQCString displayName;
  TQCString addrSpec;
  TQCString comment;

  // The following is a primitive parser for a mailbox-list (cf. RFC 2822).
  // The purpose is to extract a displayable string from the mailboxes.
  // Comments in the addr-spec are not handled. No error checking is done.

  enum { TopLevel, InComment, InAngleAddress } context = TopLevel;
  bool inQuotedString = false;
  int commentLevel = 0;
  bool stop = false;

  for ( char* p = address.data(); *p && !stop; ++p ) {
    switch ( context ) {
    case TopLevel : {
      switch ( *p ) {
      case '"' : inQuotedString = !inQuotedString;
                 displayName += *p;
                 break;
      case '(' : if ( !inQuotedString ) {
                   context = InComment;
                   commentLevel = 1;
                 }
                 else
                   displayName += *p;
                 break;
      case '<' : if ( !inQuotedString ) {
                   context = InAngleAddress;
                 }
                 else
                   displayName += *p;
                 break;
      case '\\' : // quoted character
                 displayName += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   displayName += *p;
                 else
                   //return KPIM::UnexpectedEnd;
                   goto ABORT_PARSING;
                 break;
      case ',' : if ( !inQuotedString ) {
                   //if ( allowMultipleAddresses )
                   //  stop = true;
                   //else
                   //  return KPIM::UnexpectedComma;
                   goto ABORT_PARSING;
                 }
                 else
                   displayName += *p;
                 break;
      default :  displayName += *p;
      }
      break;
    }
    case InComment : {
      switch ( *p ) {
      case '(' : ++commentLevel;
                 comment += *p;
                 break;
      case ')' : --commentLevel;
                 if ( commentLevel == 0 ) {
                   context = TopLevel;
                   comment += ' '; // separate the text of several comments
                 }
                 else
                   comment += *p;
                 break;
      case '\\' : // quoted character
                 comment += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   comment += *p;
                 else
                   //return KPIM::UnexpectedEnd;
                   goto ABORT_PARSING;
                 break;
      default :  comment += *p;
      }
      break;
    }
    case InAngleAddress : {
      switch ( *p ) {
      case '"' : inQuotedString = !inQuotedString;
                 addrSpec += *p;
                 break;
      case '>' : if ( !inQuotedString ) {
                   context = TopLevel;
                 }
                 else
                   addrSpec += *p;
                 break;
      case '\\' : // quoted character
                 addrSpec += *p;
                 ++p; // skip the '\'
                 if ( *p )
                   addrSpec += *p;
                 else
                   //return KPIM::UnexpectedEnd;
                   goto ABORT_PARSING;
                 break;
      default :  addrSpec += *p;
      }
      break;
    }
    } // switch ( context )
  }

ABORT_PARSING:
  displayName = displayName.stripWhiteSpace();
  comment = comment.stripWhiteSpace();
  addrSpec = addrSpec.stripWhiteSpace();

  fullName = TQString::fromUtf8( displayName );
  email = TQString::fromUtf8( addrSpec );

  // check for errors
  if ( inQuotedString )
    return; // KPIM::UnbalancedQuote;
  if ( context == InComment )
    return; // KPIM::UnbalancedParens;
  if ( context == InAngleAddress )
    return; // KPIM::UnclosedAngleAddr;

  if ( addrSpec.isEmpty() ) {
    if ( displayName.isEmpty() )
      return; // KPIM::NoAddressSpec;
    else {
      //addrSpec = displayName;
      //displayName.truncate( 0 );
      // Address of the form "foo@bar" or "foo@bar (Name)".
      email = fullName;
      fullName = TQString::fromUtf8( comment );
    }
  }

  // Check that we do not have any extra characters on the end of the
  // strings
  unsigned int len = fullName.length();
  if ( fullName[ 0 ] == '"' && fullName[ len - 1 ] == '"' )
    fullName = fullName.mid( 1, len - 2 );
}

void Addressee::setResource( Resource *resource )
{
  detach();
  mData->resource = resource;
}

Resource *Addressee::resource() const
{
  return mData->resource;
}

void Addressee::setChanged( bool value )
{
  detach();
  mData->changed = value;
}

bool Addressee::changed() const
{
  return mData->changed;
}

void Addressee::setSortMode( TDEABC::SortMode *mode )
{
  mSortMode = mode;
}

bool Addressee::operator< ( const Addressee &addr )
{
  if ( !mSortMode )
    return false;
  else
    return mSortMode->lesser( *this, addr );
}

TQDataStream &TDEABC::operator<<( TQDataStream &s, const Addressee &a )
{
  if (!a.mData) return s;

  s << a.uid();

  --STREAMOUT--
  s << a.mData->phoneNumbers;
  s << a.mData->addresses;
  s << a.mData->emails;
  s << a.mData->categories;
  s << a.mData->custom;
  s << a.mData->keys;
  return s;
}

TQDataStream &TDEABC::operator>>( TQDataStream &s, Addressee &a )
{
  if (!a.mData)
    return s;

  a.detach();

  s >> a.mData->uid;

  --STREAMIN--
  s >> a.mData->phoneNumbers;
  s >> a.mData->addresses;
  s >> a.mData->emails;
  s >> a.mData->categories;
  s >> a.mData->custom;
  s >> a.mData->keys;

  a.mData->empty = false;

  return s;
}

bool matchBinaryPattern( int value, int pattern )
{
  /**
    We want to match all telephonnumbers/addresses which have the bits in the
    pattern set. More are allowed.
    if pattern == 0 we have a special handling, then we want only those with
    exactly no bit set.
   */
  if ( pattern == 0 )
    return ( value == 0 );
  else
    return ( pattern == ( pattern & value ) );
}

template <class L>
bool listEquals( const TQValueList<L> &list, const TQValueList<L> &pattern )
{
  if ( list.count() != pattern.count() )
    return false;

  for ( uint i = 0; i < list.count(); ++i )
    if ( pattern.find( list[ i ] ) == pattern.end() )
      return false;

  return true;
}

bool emailsEquals( const TQStringList &list, const TQStringList &pattern )
{
  if ( list.count() != pattern.count() )
    return false;

  if ( list.first() != pattern.first() )
    return false;

  TQStringList::ConstIterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    if ( pattern.find( *it ) == pattern.end() )
      return false;

  return true;
}
