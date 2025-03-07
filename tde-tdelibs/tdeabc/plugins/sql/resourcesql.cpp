/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <tqsqldatabase.h>
#include <tqsqlcursor.h>

#include <kdebug.h>
#include <tdeglobal.h>
#include <klineedit.h>
#include <tdelocale.h>

#include "resourcesql.h"
#include "resourcesqlconfig.h"

using namespace TDEABC;

extern "C"
{
  TDE_EXPORT void *init_tdeabc_sql()
  {
    return new KRES::PluginFactory<ResourceSql,ResourceSqlConfig>();
  }
}

ResourceSql::ResourceSql( AddressBook *ab, const TDEConfig *config )
  : Resource( ab ), mDb( 0 )
{
  TQString user, password, db, host;

  user = config->readEntry( "SqlUser" );
  password = cryptStr( config->readEntry( "SqlPassword " ) );
  db = config->readEntry( "SqlName" );
  host = config->readEntry( "SqlHost" );

  init( user, password, db, host );
}

ResourceSql::ResourceSql( AddressBook *ab, const TQString &user,
    const TQString &password, const TQString &db, const TQString &host )
  : Resource( ab ), mDb( 0 )
{
  init( user, password, db, host );
}

void ResourceSql::init( const TQString &user, const TQString &password,
    const TQString &db, const TQString &host )
{
  mUser = user;
  mPassword = password;
  mDbName = db;
  mHost = host;
}

Ticket *ResourceSql::requestSaveTicket()
{
  if ( !addressBook() ) {
    kdDebug(5700) << "no addressbook" << endl;
    return 0;
  }

  return createTicket( this );
}

void ResourceSql::releaseSaveTicket( Ticket *ticket )
{
  delete ticket;
}

bool ResourceSql::open()
{
  TQStringList drivers = TQSqlDatabase::drivers();
  for ( TQStringList::Iterator it = drivers.begin(); it != drivers.end(); ++it ) {
    kdDebug(5700) << "Driver: " << (*it) << endl;
  }

  mDb = TQSqlDatabase::addDatabase( "QMYSQL3" );

  if ( !mDb ) {
    kdDebug(5700) << "Error. Unable to connect to database." << endl;
    return false;
  }

  mDb->setDatabaseName( mDbName );
  mDb->setUserName( mUser );
  mDb->setPassword( mPassword );
  mDb->setHostName( mHost );

  if ( !mDb->open() ) {
    kdDebug(5700) << "Error. Unable to open database '" << mDbName << "'." << endl;
    return false;
  }

  return true;
}

void ResourceSql::close()
{
  mDb->close();
}

bool ResourceSql::load()
{
  TQSqlQuery query( "select addressId, name, familyName, givenName, "
      "additionalName, prefix, suffix, nickname, birthday, "
      "mailer, timezone, geo_latitude, geo_longitude, title, "
      "role, organization, note, productId, revision, "
      "sortString, url from kaddressbook_main_" + mUser );

  while ( query.next() ) {
    TQString addrId = query.value(0).toString();

    Addressee addr;
    addr.setResource( this );
    addr.setUid( addrId );
    addr.setName( query.value(1).toString() );
    addr.setFamilyName( query.value(2).toString() );
    addr.setGivenName( query.value(3).toString() );
    addr.setAdditionalName( query.value(4).toString() );
    addr.setPrefix( query.value(5).toString() );
    addr.setSuffix( query.value(6).toString() );
    addr.setNickName( query.value(7).toString() );
    addr.setBirthday( query.value(8).toDateTime() );
    addr.setMailer( query.value(9).toString() );
    addr.setTimeZone( TimeZone( query.value(10).toInt() ) );
    addr.setGeo( Geo( query.value(11).toDouble(), query.value(12).toDouble() ) );
    addr.setTitle( query.value(13).toString() );
    addr.setRole( query.value(14).toString() );
    addr.setOrganization( query.value(15).toString() );
    addr.setNote( query.value(16).toString() );
    addr.setProductId( query.value(17).toString() );
    addr.setRevision( query.value(18).toDateTime() );
    addr.setSortString( query.value(19).toString() );
    addr.setUrl( query.value(20).toString() );

    // emails
    {
      TQSqlQuery emailsQuery( "select email, preferred from kaddressbook_emails "
          "where addressId = '" + addrId + "'" );
      while ( emailsQuery.next() )
        addr.insertEmail( emailsQuery.value( 0 ).toString(),
      emailsQuery.value( 1 ).toInt() );
    }

    // phones
    {
      TQSqlQuery phonesQuery( "select number, type from kaddressbook_phones "
          "where addressId = '" + addrId + "'" );
      while ( phonesQuery.next() )
        addr.insertPhoneNumber( PhoneNumber( phonesQuery.value( 0 ).toString(),
      phonesQuery.value( 1 ).toInt() ) );
    }    

    // addresses
    {
      TQSqlQuery addressesQuery( "select postOfficeBox, extended, street, "
          "locality, region, postalCode, country, label, type "
          "from kaddressbook_addresses where addressId = '" + addrId + "'" );
      while ( addressesQuery.next() ) {
        Address a;
        a.setPostOfficeBox( addressesQuery.value(0).toString() );
        a.setExtended( addressesQuery.value(1).toString() );
        a.setStreet( addressesQuery.value(2).toString() );
        a.setLocality( addressesQuery.value(3).toString() );
        a.setRegion( addressesQuery.value(4).toString() );
        a.setPostalCode( addressesQuery.value(5).toString() );
        a.setCountry( addressesQuery.value(6).toString() );
        a.setLabel( addressesQuery.value(7).toString() );
        a.setType( addressesQuery.value(8).toInt() );

        addr.insertAddress( a );
      }
    }

    // categories
    {
      TQSqlQuery categoriesQuery( "select category from kaddressbook_categories "
          "where addressId = '" + addrId + "'" );
      while ( categoriesQuery.next() )
        addr.insertCategory( categoriesQuery.value( 0 ).toString() );
    }

    // customs
    {
      TQSqlQuery customsQuery( "select app, name, value from kaddressbook_customs "
          "where addressId = '" + addrId + "'" );
      while ( customsQuery.next() )
        addr.insertCustom( customsQuery.value( 0 ).toString(),
      customsQuery.value( 1 ).toString(),
      customsQuery.value( 2 ).toString());
    }

    addressBook()->insertAddressee( addr );
  }
 
  return true;
}

bool ResourceSql::save( Ticket * )
{
  // we have to delete all entries for this user and reinsert them
  TQSqlQuery query( "select addressId from kaddressbook_main_" + mUser );

  while ( query.next() ) {
    TQString addrId = query.value( 0 ).toString();
    TQSqlQuery q;
	
    q.exec( "DELETE FROM kaddressbook_emails WHERE addressId = '" + addrId + "'" );
    q.exec( "DELETE FROM kaddressbook_phones WHERE addressId = '" + addrId + "'" );
    q.exec( "DELETE FROM kaddressbook_addresses WHERE addressId = '" + addrId + "'" );
    q.exec( "DELETE FROM kaddressbook_categories WHERE addressId = '" + addrId + "'" );
    q.exec( "DELETE FROM kaddressbook_customs WHERE addressId = '" + addrId + "'" );

    q.exec( "DELETE FROM kaddressbook_main_" + mUser + " WHERE addressId = '" + addrId + "'" );
  }

  // let's start...
  AddressBook::Iterator it;
  for ( it = addressBook()->begin(); it != addressBook()->end(); ++it ) {
    if ( (*it).resource() != this && (*it).resource() != 0 ) // save only my and new entries
      continue;

    TQString uid = (*it).uid();

    query.exec( "INSERT INTO kaddressbook_main_" + mUser + " VALUES ('" +
        (*it).uid() + "','" +
        (*it).name() + "','" +
        (*it).familyName() + "','" +
        (*it).givenName() + "','" +
        (*it).additionalName() + "','" +
        (*it).prefix() + "','" +
        (*it).suffix() + "','" +
        (*it).nickName() + "','" +
        (*it).birthday().toString( TQt::ISODate ) + "','" +
        (*it).mailer() + "','" +
        TQString::number( (*it).timeZone().offset() ) + "','" +
        TQString::number( (*it).geo().latitude() ) + "','" +
        TQString::number( (*it).geo().longitude() ) + "','" +
        (*it).title() + "','" +
        (*it).role() + "','" +
        (*it).organization() + "','" +
        (*it).note() + "','" +
        (*it).productId() + "','" +
        (*it).revision().toString( TQt::ISODate ) + "','" +
        (*it).sortString() + "','" +
        (*it).url().url() + "')"
    );

    // emails
    {
      TQStringList emails = (*it).emails();
      TQStringList::ConstIterator it;
      bool preferred = true;
      for( it = emails.begin(); it != emails.end(); ++it ) {
        query.exec("INSERT INTO kaddressbook_emails VALUES ('" +
            uid + "','" +
            (*it) + "','" +
            TQString::number(preferred) + "')");
        preferred = false;
      }
    }

    // phonenumbers
    {
      PhoneNumber::List phoneNumberList = (*it).phoneNumbers();
      PhoneNumber::List::ConstIterator it;
      for( it = phoneNumberList.begin(); it != phoneNumberList.end(); ++it ) {
        query.exec("INSERT INTO kaddressbook_phones VALUES ('" +
            uid + "','" +
            (*it).number() + "','" +
            TQString::number( (*it).type() ) + "')");
      }
    }

    // postal addresses
    {
      Address::List addressList = (*it).addresses();
      Address::List::ConstIterator it;
      for( it = addressList.begin(); it != addressList.end(); ++it ) {
        query.exec("INSERT INTO kaddressbook_addresses VALUES ('" +
            uid + "','" +
            (*it).postOfficeBox() + "','" +
            (*it).extended() + "','" +
            (*it).street() + "','" +
            (*it).locality() + "','" +
            (*it).region() + "','" +
            (*it).postalCode() + "','" +
            (*it).country() + "','" +
            (*it).label() + "','" +
            TQString::number( (*it).type() ) + "')");
      }
    }

    // categories
    {
      TQStringList categories = (*it).categories();
      TQStringList::ConstIterator it;
      for( it = categories.begin(); it != categories.end(); ++it )
        query.exec("INSERT INTO kaddressbook_categories VALUES ('" +
            uid + "','" +
            (*it) + "')");
    }

    // customs
    {
      TQStringList list = (*it).customs();
      TQStringList::ConstIterator it;
      for( it = list.begin(); it != list.end(); ++it ) {
        int dashPos = (*it).find( '-' );
        int colonPos = (*it).find( ':' );
        TQString app = (*it).left( dashPos );
        TQString name = (*it).mid( dashPos + 1, colonPos - dashPos - 1 );
        TQString value = (*it).right( (*it).length() - colonPos - 1 );

        query.exec("INSERT INTO kaddressbook_categories VALUES ('" +
            uid + "','" + app + "','" + name + "','" + value + "')");
      }
    }
  }

  return true;
}

TQString ResourceSql::identifier() const
{
  return mHost + "_" + mDbName;
}
