/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <tdelocale.h>
#include <tdeconfig.h>
#include <tdeglobal.h>

#include "field.h"
#include "address.h"

using namespace TDEABC;

class Field::FieldImpl
{
  public:
    FieldImpl( int fieldId, int category = 0,
               const TQString &label = TQString::null,
               const TQString &key = TQString::null,
               const TQString &app = TQString::null )
      : mFieldId( fieldId ), mCategory( category ), mLabel( label ),
        mKey( key ), mApp( app ) {}
  
    enum FieldId
    {
      CustomField,
      --ENUMS--
    };

    int fieldId() { return mFieldId; }
    int category() { return mCategory; }
    
    TQString label() { return mLabel; }
    TQString key() { return mKey; }
    TQString app() { return mApp; }
    
  private:
    int mFieldId;
    int mCategory;

    TQString mLabel;
    TQString mKey;
    TQString mApp;
};


Field::List Field::mAllFields;
Field::List Field::mDefaultFields;
Field::List Field::mCustomFields;


Field::Field( FieldImpl *impl )
{
  mImpl = impl;
}

Field::~Field()
{
  delete mImpl;
}

TQString Field::label()
{
  switch ( mImpl->fieldId() ) {
    --CASELABEL--
    case FieldImpl::CustomField:
      return mImpl->label();
    default:
      return i18n("Unknown Field");
  }
}

int Field::category()
{
  return mImpl->category();
}

TQString Field::categoryLabel( int category )
{
  switch ( category ) {
    case All:
      return i18n("All");
    case Frequent:
      return i18n("Frequent");
    case Address:
      return i18n("street/postal","Address");
    case Email:
      return i18n("Email");
    case Personal:
      return i18n("Personal");
    case Organization:
      return i18n("Organization");
    case CustomCategory:
      return i18n("Custom");
    default:
      return i18n("Undefined");
  }
}

TQString Field::value( const TDEABC::Addressee &a )
{
  switch ( mImpl->fieldId() ) {
    --CASEVALUE--
    case FieldImpl::Email:
      return a.preferredEmail();
    case FieldImpl::Birthday:
      if ( a.birthday().isValid() )
        return a.birthday().date().toString( TQt::ISODate );
      else
        return TQString::null;
    case FieldImpl::Url:
      return a.url().prettyURL();
    case FieldImpl::HomePhone:
    {
      PhoneNumber::List::ConstIterator it;

      {
        // check for preferred number
        const PhoneNumber::List list = a.phoneNumbers( PhoneNumber::Home | PhoneNumber::Pref );
        for ( it = list.begin(); it != list.end(); ++it )
          if ( ((*it).type() & ~(PhoneNumber::Pref)) == PhoneNumber::Home )
            return (*it).number();
      }

      {
        // check for normal home number
        const PhoneNumber::List list = a.phoneNumbers( PhoneNumber::Home );
        for ( it = list.begin(); it != list.end(); ++it )
          if ( ((*it).type() & ~(PhoneNumber::Pref)) == PhoneNumber::Home )
            return (*it).number();
      }

      return TQString::null;
    }
    case FieldImpl::BusinessPhone:
    {
      PhoneNumber::List::ConstIterator it;

      {
        // check for preferred number
        const PhoneNumber::List list = a.phoneNumbers( PhoneNumber::Work | PhoneNumber::Pref );
        for ( it = list.begin(); it != list.end(); ++it )
          if ( ((*it).type() & ~(PhoneNumber::Pref)) == PhoneNumber::Work )
            return (*it).number();
      }

      {
        // check for normal work number
        const PhoneNumber::List list = a.phoneNumbers( PhoneNumber::Work );
        for ( it = list.begin(); it != list.end(); ++it )
          if ( ((*it).type() & ~(PhoneNumber::Pref)) == PhoneNumber::Work )
            return (*it).number();
      }

      return TQString::null;
    }
    case FieldImpl::MobilePhone:
      return a.phoneNumber( PhoneNumber::Cell ).number();
    case FieldImpl::HomeFax:
      return a.phoneNumber( PhoneNumber::Home | PhoneNumber::Fax ).number();
    case FieldImpl::BusinessFax:
      return a.phoneNumber( PhoneNumber::Work | PhoneNumber::Fax ).number();
    case FieldImpl::CarPhone:
      return a.phoneNumber( PhoneNumber::Car ).number();
    case FieldImpl::Isdn:
      return a.phoneNumber( PhoneNumber::Isdn ).number();
    case FieldImpl::Pager:
      return a.phoneNumber( PhoneNumber::Pager ).number();
    case FieldImpl::HomeAddressStreet:
      return a.address( Address::Home ).street();
    case FieldImpl::HomeAddressPostOfficeBox:
      return a.address( Address::Home ).postOfficeBox();
    case FieldImpl::HomeAddressLocality:
      return a.address( Address::Home ).locality();
    case FieldImpl::HomeAddressRegion:
      return a.address( Address::Home ).region();
    case FieldImpl::HomeAddressPostalCode:
      return a.address( Address::Home ).postalCode();
    case FieldImpl::HomeAddressCountry:
      return a.address( Address::Home ).country();
    case FieldImpl::HomeAddressLabel:
      return a.address( Address::Home ).label();
    case FieldImpl::BusinessAddressStreet:
      return a.address( Address::Work ).street();
    case FieldImpl::BusinessAddressPostOfficeBox:
      return a.address( Address::Work ).postOfficeBox();
    case FieldImpl::BusinessAddressLocality:
      return a.address( Address::Work ).locality();
    case FieldImpl::BusinessAddressRegion:
      return a.address( Address::Work ).region();
    case FieldImpl::BusinessAddressPostalCode:
      return a.address( Address::Work ).postalCode();
    case FieldImpl::BusinessAddressCountry:
      return a.address( Address::Work ).country();
    case FieldImpl::BusinessAddressLabel:
      return a.address( Address::Work ).label();
    case FieldImpl::CustomField:
      return a.custom( mImpl->app(), mImpl->key() );
    default:
      return TQString::null;
  }
}

bool Field::setValue( TDEABC::Addressee &a, const TQString &value )
{
  switch ( mImpl->fieldId() ) {
    --CASESETVALUE--
    case FieldImpl::MobilePhone:
      {
        PhoneNumber number = a.phoneNumber( PhoneNumber::Cell );
        number.setNumber( value );
        a.insertPhoneNumber( number );
        return true;
      }
    case FieldImpl::HomeFax:
      {
        PhoneNumber number = a.phoneNumber( PhoneNumber::Home | PhoneNumber::Fax );
        number.setNumber( value );
        a.insertPhoneNumber( number );
        return true;
      }
    case FieldImpl::BusinessFax:
      {
        PhoneNumber number = a.phoneNumber( PhoneNumber::Work | PhoneNumber::Fax );
        number.setNumber( value );
        a.insertPhoneNumber( number );
        return true;
      }
    case FieldImpl::CarPhone:
      {
        PhoneNumber number = a.phoneNumber( PhoneNumber::Car );
        number.setNumber( value );
        a.insertPhoneNumber( number );
        return true;
      }
    case FieldImpl::Isdn:
      {
        PhoneNumber number = a.phoneNumber( PhoneNumber::Isdn );
        number.setNumber( value );
        a.insertPhoneNumber( number );
        return true;
      }
    case FieldImpl::Pager:
      {
        PhoneNumber number = a.phoneNumber( PhoneNumber::Pager );
        number.setNumber( value );
        a.insertPhoneNumber( number );
        return true;
      }
    case FieldImpl::HomeAddressStreet:
      {
        TDEABC::Address address = a.address( Address::Home );
        address.setStreet( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::HomeAddressPostOfficeBox:
      {
        TDEABC::Address address = a.address( Address::Home );
        address.setPostOfficeBox( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::HomeAddressLocality:
      {
        TDEABC::Address address = a.address( Address::Home );
        address.setLocality( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::HomeAddressRegion:
      {
        TDEABC::Address address = a.address( Address::Home );
        address.setRegion( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::HomeAddressPostalCode:
      {
        TDEABC::Address address = a.address( Address::Home );
        address.setPostalCode( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::HomeAddressCountry:
      {
        TDEABC::Address address = a.address( Address::Home );
        address.setCountry( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::HomeAddressLabel:
      {
        TDEABC::Address address = a.address( Address::Home );
        address.setLabel( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::BusinessAddressStreet:
      {
        TDEABC::Address address = a.address( Address::Work );
        address.setStreet( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::BusinessAddressPostOfficeBox:
      {
        TDEABC::Address address = a.address( Address::Work );
        address.setPostOfficeBox( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::BusinessAddressLocality:
      {
        TDEABC::Address address = a.address( Address::Work );
        address.setLocality( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::BusinessAddressRegion:
      {
        TDEABC::Address address = a.address( Address::Work );
        address.setRegion( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::BusinessAddressPostalCode:
      {
        TDEABC::Address address = a.address( Address::Work );
        address.setPostalCode( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::BusinessAddressCountry:
      {
        TDEABC::Address address = a.address( Address::Work );
        address.setCountry( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::BusinessAddressLabel:
      {
        TDEABC::Address address = a.address( Address::Work );
        address.setLabel( value );
        a.insertAddress( address );
        return true;
      }
    case FieldImpl::Birthday:
      a.setBirthday( TQDate::fromString( value, TQt::ISODate ) );
      return true;
    case FieldImpl::CustomField:
      a.insertCustom( mImpl->app(), mImpl->key(), value );
      return true;
    default:
      return false;
  }
}

TQString Field::sortKey( const TDEABC::Addressee &a )
{
  switch ( mImpl->fieldId() ) {
    --CASEVALUE--
    case FieldImpl::Birthday:
      if ( a.birthday().isValid() ) {
        TQDate date = a.birthday().date();
        TQString key;
        key.sprintf( "%02d-%02d", date.month(), date.day() );
        return key;
      } else
        return TQString( "00-00" );
    default:
      return value( a ).lower();
  }
}

bool Field::isCustom()
{
  return mImpl->fieldId() == FieldImpl::CustomField;
}

Field::List Field::allFields()
{
  if ( mAllFields.isEmpty() ) {
    --CREATEFIELDS--
  }

  return mAllFields;
}

Field::List Field::defaultFields()
{
  if ( mDefaultFields.isEmpty() ) {
    createDefaultField( FieldImpl::FormattedName );
    createDefaultField( FieldImpl::Email );
  }

  return mDefaultFields;
}

void Field::createField( int id, int category )
{
  mAllFields.append( new Field( new FieldImpl( id, category ) ) );
}

void Field::createDefaultField( int id, int category )
{
  mDefaultFields.append( new Field( new FieldImpl( id, category ) ) );
}

void Field::deleteFields()
{
  Field::List::ConstIterator it;

  for ( it = mAllFields.constBegin(); it != mAllFields.constEnd(); ++it ) {
    delete (*it);
  }
  mAllFields.clear();

  for ( it = mDefaultFields.constBegin(); it != mDefaultFields.constEnd(); ++it ) {
    delete (*it);
  }
  mDefaultFields.clear();

  for ( it = mCustomFields.constBegin(); it != mCustomFields.constEnd(); ++it ) {
    delete (*it);
  }
  mCustomFields.clear();
}

void Field::saveFields( const TQString &identifier,
                        const Field::List &fields )
{
  TDEConfig *cfg = TDEGlobal::config();
  TDEConfigGroupSaver( cfg, "KABCFields" );

  saveFields( cfg, identifier, fields );
}

void Field::saveFields( TDEConfig *cfg, const TQString &identifier,
                        const Field::List &fields )
{
  TQValueList<int> fieldIds;
  
  int custom = 0;
  Field::List::ConstIterator it;
  for( it = fields.begin(); it != fields.end(); ++it ) {
    fieldIds.append( (*it)->mImpl->fieldId() );
    if( (*it)->isCustom() ) {
      TQStringList customEntry;
      customEntry << (*it)->mImpl->label();
      customEntry << (*it)->mImpl->key();
      customEntry << (*it)->mImpl->app();
      cfg->writeEntry( "KABC_CustomEntry_" + identifier + "_" +
                       TQString::number( custom++ ), customEntry );
    }
  }
  
  cfg->writeEntry( identifier, fieldIds );
}

Field::List Field::restoreFields( const TQString &identifier )
{
  TDEConfig *cfg = TDEGlobal::config();
  TDEConfigGroupSaver( cfg, "KABCFields" );
 
  return restoreFields( cfg, identifier );
}

Field::List Field::restoreFields( TDEConfig *cfg, const TQString &identifier )
{
  const TQValueList<int> fieldIds = cfg->readIntListEntry( identifier );

  Field::List fields;

  int custom = 0;
  TQValueList<int>::ConstIterator it;
  for( it = fieldIds.begin(); it != fieldIds.end(); ++it ) {
    FieldImpl *f = 0;
    if ( (*it) == FieldImpl::CustomField ) {
      TQStringList customEntry = cfg->readListEntry( "KABC_CustomEntry_" +
                                                 identifier + "_" +
                                                 TQString::number( custom++ ) );
      f = new FieldImpl( *it, CustomCategory, customEntry[ 0 ],
                         customEntry[ 1 ], customEntry[ 2 ] );
    } else {
      f = new FieldImpl( *it );
    }
    fields.append( new Field( f ) );
  }
  
  return fields;
}

bool Field::equals( Field *field )
{
  bool sameId = ( mImpl->fieldId() == field->mImpl->fieldId() );

  if ( !sameId ) return false;

  if ( mImpl->fieldId() != FieldImpl::CustomField ) return true;
  
  return mImpl->key() == field->mImpl->key();
}

Field *Field::createCustomField( const TQString &label, int category,
                                 const TQString &key, const TQString &app )
{
  Field *field = new Field( new FieldImpl( FieldImpl::CustomField,
                                           category | CustomCategory,
                                           label, key, app ) );
  mCustomFields.append( field );

  return field;
}
