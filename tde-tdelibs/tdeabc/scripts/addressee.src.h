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

#ifndef KABC_ADDRESSEE_H
#define KABC_ADDRESSEE_H

#include <tqdatetime.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>

#include <ksharedptr.h>
#include <kurl.h>

#include "address.h"
#include "agent.h"
#include "geo.h"
#include "key.h"
#include "phonenumber.h"
#include "picture.h"
#include "secrecy.h"
#include "sound.h"
#include "timezone.h"

namespace TDEABC {

class Resource;
class Field;
class SortMode;

/**
  @short address book entry

  This class represents an entry in the address book.

  The data of this class is implicitly shared. You can pass this class by value.

  If you need the name of a field for presenting it to the user you should use
  the functions ending in Label(). They return a translated string which can be
  used as label for the corresponding field.

  About the name fields:

  givenName() is the first name and familyName() the last name. In some
  countries the family name comes first, that's the reason for the
  naming. formattedName() is the full name with the correct formatting.
  It is used as an override, when the correct formatting can't be generated
  from the other name fields automatically.

  realName() returns a fully formatted name(). It uses formattedName, if set,
  otherwise it constucts the name from the name fields. As fallback, if
  nothing else is set it uses name().

  name() is the NAME type of RFC2426. It can be used as internal name for the
  data enty, but shouldn't be used for displaying the data to the user.
 */
class KABC_EXPORT Addressee
{
  friend KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Addressee & );
  friend KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Addressee & );

  public:
    typedef TQValueList<Addressee> List;
    typedef TQMap<TQString, Addressee> Map;

    /**
      Construct an empty address book entry.
     */
    Addressee();
    ~Addressee();

    Addressee( const Addressee & );
    Addressee &operator=( const Addressee & );

    bool operator==( const Addressee & ) const;
    bool operator!=( const Addressee & ) const;

    /**
      Return, if the address book entry is empty.
     */
    bool isEmpty() const;

    /**
      Set unique identifier.
     */
    void setUid( const TQString &uid );
    /**
      Return unique identifier.
     */
    TQString uid() const;
    /**
      Return translated label for uid field.
     */
    static TQString uidLabel();

    /**
      Set unique resource identifier.
     */
    void setUri( const TQString &uid );
    /**
      Return unique resource identifier.
     */
    TQString uri() const;
    /**
      Return translated label for uri field.
     */
    static TQString uriLabel();

    --DECLARATIONS--
    /**
      Set name fields by parsing the given string and trying to associate the
      parts of the string with according fields. This function should probably
      be a bit more clever.
     */
    void setNameFromString( const TQString & );

    /**
      Return the name of the addressee. This is calculated from all the name
      fields.
     */
    TQString realName() const;

    /**
      Return the name that consists of all name parts.
     */
    TQString assembledName() const;

    /**
      Return email address including real name.

      @param email Email address to be used to construct the full email string.
                   If this is TQString::null the preferred email address is used.
     */
    TQString fullEmail( const TQString &email=TQString::null ) const;

    /**
      Insert an email address. If the email address already exists in this
      addressee it is not duplicated.

      @param email Email address
      @param preferred Set to true, if this is the preferred email address of
                       the addressee.
     */
    void insertEmail( const TQString &email, bool preferred=false );

    /**
      Remove email address. If the email address doesn't exist, nothing happens.
     */
    void removeEmail( const TQString &email );

    /**
      Return preferred email address. This is the first email address or the
      last one added with insertEmail() with a set preferred parameter.
     */
    TQString preferredEmail() const;

    /**
      Return list of all email addresses.
     */
    TQStringList emails() const;

    /**
       Set the emails to @p list.
       The first email address gets the preferred one!
       @param list The list of email addresses.
     */
    void setEmails( const TQStringList& list);

    /**
      Insert a phone number. If a phone number with the same id already exists
      in this addressee it is not duplicated.
     */
    void insertPhoneNumber( const PhoneNumber &phoneNumber );

    /**
      Remove phone number. If no phone number with the given id exists for this
      addresse nothing happens.
     */
    void removePhoneNumber( const PhoneNumber &phoneNumber );

    /**
      Return phone number, which matches the given type.
     */
    PhoneNumber phoneNumber( int type ) const;

    /**
      Return list of all phone numbers.
     */
    PhoneNumber::List phoneNumbers() const;

    /**
      Return list of phone numbers with a special type.
     */
    PhoneNumber::List phoneNumbers( int type ) const;

    /**
      Return phone number with the given id.
     */
    PhoneNumber findPhoneNumber( const TQString &id ) const;

    /**
      Insert a key. If a key with the same id already exists
      in this addressee it is not duplicated.
     */
    void insertKey( const Key &key );

    /**
      Remove a key. If no key with the given id exists for this
      addresse nothing happens.
     */
    void removeKey( const Key &key );

    /**
      Return key, which matches the given type.
      If @p type == Key::Custom you can specify a string
      that should match. If you leave the string empty, the first
      key with a custom value is returned.
     */
    Key key( int type, TQString customTypeString = TQString::null ) const;

    /**
      Return list of all keys.
     */
    Key::List keys() const;

    /**
       Set the list of keys
       @param keys The keys to be set.
     */
    void setKeys( const Key::List& keys);

    /**
      Return list of keys with a special type.
      If @p type == Key::Custom you can specify a string
      that should match. If you leave the string empty, all custom
      keys will be returned.
     */
    Key::List keys( int type, TQString customTypeString = TQString::null  ) const;

    /**
      Return key with the given id.
     */
    Key findKey( const TQString &id ) const;

    /**
      Insert an address. If an address with the same id already exists
      in this addressee it is not duplicated.
     */
    void insertAddress( const Address &address );

    /**
      Remove address. If no address with the given id exists for this
      addresse nothing happens.
     */
    void removeAddress( const Address &address );

    /**
      Return address, which matches the given type.
     */
    Address address( int type ) const;

    /**
      Return list of all addresses.
     */
    Address::List addresses() const;

    /**
      Return list of addresses with a special type.
     */
    Address::List addresses( int type ) const;

    /**
      Return address with the given id.
     */
    Address findAddress( const TQString &id ) const;

    /**
      Insert category. If the category already exists it is not duplicated.
     */
    void insertCategory( const TQString & );

    /**
      Remove category.
     */
    void removeCategory( const TQString & );

    /**
      Return, if addressee has the given category.
     */
    bool hasCategory( const TQString & ) const;

    /**
      Set categories to given value.
     */
    void setCategories( const TQStringList & );

    /**
      Return list of all set categories.
     */
    TQStringList categories() const;

    /**
      Insert custom entry. The entry is identified by the name of the inserting
      application and a unique name. If an entry with the given app and name
      already exists its value is replaced with the new given value.

      An empty value isn't allowed (nothing happens if this is called with
      any of the three arguments being empty)
     */
    void insertCustom( const TQString &app, const TQString &name,
                       const TQString &value );

    /**
      Remove custom entry.
     */
    void removeCustom( const TQString &app, const TQString &name );

    /**
      Return value of custom entry, identified by app and entry name.
     */
    TQString custom( const TQString &app, const TQString &name ) const;

    /**
      Set all custom entries.
     */
    void setCustoms( const TQStringList & );

    /**
      Return list of all custom entries.
     */
    TQStringList customs() const;

    /**
      Parse full email address. The result is given back in fullName and email.
     */
    static void parseEmailAddress( const TQString &rawEmail, TQString &fullName,
                                   TQString &email );

    /**
      Debug output.
     */
    void dump() const;

    /**
      Returns string representation of the addressee.
     */
    TQString asString() const;

    /**
      Set resource where the addressee is from.
     */
    void setResource( Resource *resource );

    /**
      Return pointer to resource.
     */
    Resource *resource() const;

    /**
      Mark addressee as changed.
     */
    void setChanged( bool value );

    /**
      Return whether the addressee is changed.
     */
    bool changed() const;

    static void setSortMode( TDEABC::SortMode *mode );

    bool operator< ( const Addressee &addr );

  private:
    void detach();

    struct AddresseeData;
    mutable TDESharedPtr<AddresseeData> mData;

  private:
    static AddresseeData* shared_null;
    static AddresseeData* makeSharedNull();
    static TDEABC::SortMode *mSortMode;
};

KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Addressee & );
KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Addressee & );

}

#endif
