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

#include <tqfile.h>
#include <tqtextstream.h>

#include <kabapi.h>
#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>

#include "addressbook.h"
#include "stdaddressbook.h"

using namespace TDEABC;

static const TDECmdLineOptions options[] =
{
  { "disable-autostart", I18N_NOOP( "Disable automatic startup on login" ), 0 },
  { "quiet", "", 0 },
  { "o", 0, 0 },
  { "override", I18N_NOOP( "Override existing entries" ), "1" },
  TDECmdLineLastOption
};

void readKMailEntry( const TQString &kmailEntry, TDEABC::AddressBook *ab )
{
  kdDebug() << "KMAILENTRY: " << kmailEntry << endl;

  TQString entry = kmailEntry.simplifyWhiteSpace();
  if ( entry.isEmpty() ) return;

  TQString email;
  TQString name;
  TQString comment;

  if ( entry.at( entry.length() -1 ) == ')' ) {
    int br = entry.findRev( '(' );
    if ( br >= 0 ) {
      comment = entry.mid( br + 1, entry.length() - br - 2 );
      entry.truncate( br );
      if ( entry.at( entry.length() - 1 ).isSpace() ) {
        entry.truncate( br - 1 );
      }
    }
  }

  int posSpace = entry.findRev( ' ' );
  if ( posSpace < 0 ) {
    email = entry;
    if ( !comment.isEmpty() ) {
      name = comment;
      comment = "";
    }
  } else {
    email = entry.mid( posSpace + 1 );
    name = entry.left( posSpace );
  }

  if ( email.at( 0 ) == '<' && email.at( email.length() - 1) == '>' ) {
    email = email.mid( 1, email.length() - 2 );
  }
  if ( name.at( 0 ) == '"' && name.at( name.length() - 1) == '"' ) {
    name = name.mid( 1, name.length() - 2 );
  }
  if ( name.at( 0 ) == '\'' && name.at( name.length() - 1) == '\'' ) {
    name = name.mid( 1, name.length() - 2 );
  }

  if ( name.at( name.length() -1 ) == ')' ) {
    int br = name.findRev( '(' );
    if ( br >= 0 ) {
      comment = name.mid( br + 1, name.length() - br - 2 ) + " " + comment;
      name.truncate( br );
      if ( name.at( name.length() - 1 ).isSpace() ) {
        name.truncate( br - 1 );
      }
    }
  }

  kdDebug() << "  EMAIL   : " << email   << endl;
  kdDebug() << "  NAME    : " << name    << endl;
  kdDebug() << "  COMMENT : " << comment << endl;

  TDEABC::Addressee::List al = ab->findByEmail( email );
  if ( al.isEmpty() ) {
    TDEABC::Addressee a;
    a.setNameFromString( name );
    a.insertEmail( email );
    a.setNote( comment );

    ab->insertAddressee( a );

    kdDebug() << "--INSERTED: " << a.realName() << endl;
  }
}

void importKMailAddressBook( TDEABC::AddressBook *ab )
{
  TQString fileName = locateLocal( "data", "kmail/addressbook" );
  TQString kmailConfigName = locate( "config", "kmailrc" );
  if ( !kmailConfigName.isEmpty() ) {
    TDEConfig cfg( kmailConfigName );
    cfg.setGroup( "Addressbook" );
    fileName = cfg.readPathEntry( "default", fileName );
  }
  if ( !TDEStandardDirs::exists( fileName ) ) {
    kdDebug(5700) << "Couldn't find KMail addressbook." << endl;
    return;
  }

  TQFile f( fileName );
  if ( !f.open(IO_ReadOnly) ) {
    kdDebug(5700) << "Couldn't open file '" << fileName << "'" << endl;
    return;
  }

  TQStringList kmailEntries;

  TQTextStream t( &f );
  while ( !t.eof() ) {
    kmailEntries.append( t.readLine() );
  }
  f.close();

  TQStringList::ConstIterator it;
  for ( it = kmailEntries.begin(); it != kmailEntries.end(); ++it ) {
    if ( (*it).at( 0 ) == '#' ) continue;
    bool insideQuote = false;
    int end = (*it).length() - 1;
    for ( int i = end; i; i-- ) {
      if ( (*it).at( i ) == '"' ) {
        if ( insideQuote )
          insideQuote = false;
        else
          insideQuote = true;
      } else if ( (*it).at( i ) == ',' && !insideQuote ) {
        readKMailEntry( (*it).mid( i + 1, end - i ), ab );
        end = i - 1;
      }
    }

    readKMailEntry( (*it).mid( 0, end + 1 ), ab );
  }
}

void readKAddressBookEntries( const TQString &dataString, Addressee &a )
{
  // Strip "KMail:1.0" prefix and "[EOS]" suffix.
  TQString str = dataString.mid( 11, dataString.length() - 24 );

  TQStringList entries = TQStringList::split( "\n[EOR]\n ", str );

  Address homeAddress( Address::Home );
  Address businessAddress( Address::Work );
  Address otherAddress;

  TQStringList::ConstIterator it;
  for ( it = entries.begin(); it != entries.end(); ++it ) {
    int pos = (*it).find( "\n" );
    TQString fieldName = (*it).left( pos );
    TQString fieldValue = (*it).mid( pos + 2 );

    if ( fieldName == "X-HomeFax" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Home |
                                                    PhoneNumber::Fax ) );
    } else if ( fieldName == "X-OtherPhone" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, 0 ) );
    } else if ( fieldName == "X-PrimaryPhone" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Pref ) );
    } else if ( fieldName == "X-BusinessFax" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Work |
                                                    PhoneNumber::Fax ) );
    } else if ( fieldName == "X-CarPhone" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Car ) );
    } else if ( fieldName == "X-MobilePhone" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Cell ) );
    } else if ( fieldName == "X-ISDN" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Isdn ) );
    } else if ( fieldName == "X-OtherFax" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Fax ) );
    } else if ( fieldName == "X-Pager" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Pager ) );
    } else if ( fieldName == "X-BusinessPhone" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Work ) );
    } else if ( fieldName == "X-HomePhone" ) {
      a.insertPhoneNumber( PhoneNumber( fieldValue, PhoneNumber::Home ) );
    } else if ( fieldName == "X-HomeAddress" ) {
      homeAddress.setLabel( fieldValue );
    } else if ( fieldName == "X-HomeAddressStreet" ) {
      homeAddress.setStreet( fieldValue );
    } else if ( fieldName == "X-HomeAddressCity" ) {
      homeAddress.setLocality( fieldValue );
    } else if ( fieldName == "X-HomeAddressPostalCode" ) {
      homeAddress.setPostalCode( fieldValue );
    } else if ( fieldName == "X-HomeAddressState" ) {
      homeAddress.setRegion( fieldValue );
    } else if ( fieldName == "X-HomeAddressCountry" ) {
      homeAddress.setCountry( fieldValue );
    } else if ( fieldName == "X-BusinessAddress" ) {
      businessAddress.setLabel( fieldValue );
    } else if ( fieldName == "X-BusinessAddressStreet" ) {
      businessAddress.setStreet( fieldValue );
    } else if ( fieldName == "X-BusinessAddressCity" ) {
      businessAddress.setLocality( fieldValue );
    } else if ( fieldName == "X-BusinessAddressPostalCode" ) {
      businessAddress.setPostalCode( fieldValue );
    } else if ( fieldName == "X-BusinessAddressState" ) {
      businessAddress.setRegion( fieldValue );
    } else if ( fieldName == "X-BusinessAddressCountry" ) {
      businessAddress.setCountry( fieldValue );
    } else if ( fieldName == "X-OtherAddress" ) {
      otherAddress.setLabel( fieldValue );
    } else if ( fieldName == "X-OtherAddressStreet" ) {
      otherAddress.setStreet( fieldValue );
    } else if ( fieldName == "X-OtherAddressCity" ) {
      otherAddress.setLocality( fieldValue );
    } else if ( fieldName == "X-OtherAddressPostalCode" ) {
      otherAddress.setPostalCode( fieldValue );
    } else if ( fieldName == "X-OtherAddressState" ) {
      otherAddress.setRegion( fieldValue );
    } else if ( fieldName == "X-OtherAddressCountry" ) {
      otherAddress.setCountry( fieldValue );
    } else if ( fieldName == "NICKNAME" ) {
      a.setNickName( fieldValue );
    } else if ( fieldName == "ORG" ) {
      a.setOrganization( fieldValue );
    } else if ( fieldName == "ROLE" ) {
      a.setRole( fieldValue );
    } else if ( fieldName == "BDAY" ) {
      a.setBirthday( TDEGlobal::locale()->readDate( fieldValue ) );
    } else if ( fieldName == "WEBPAGE" ) {
      a.setUrl( KURL( fieldValue ) );
    } else if ( fieldName == "N" ) {
    } else if ( fieldName == "X-FirstName" ) {
    } else if ( fieldName == "X-MiddleName" ) {
    } else if ( fieldName == "X-LastName" ) {
    } else if ( fieldName == "X-Title" ) {
    } else if ( fieldName == "X-Suffix" ) {
    } else if ( fieldName == "X-FileAs" ) {
    } else if ( fieldName == "EMAIL" ) {
      a.insertEmail( fieldValue, true );
    } else if ( fieldName == "X-E-mail2" ) {
      a.insertEmail( fieldValue );
    } else if ( fieldName == "X-E-mail3" ) {
      a.insertEmail( fieldValue );
    } else if ( fieldName == "X-Notes" ) {
    } else {
      a.insertCustom( "KADDRESSBOOK", fieldName, fieldValue );
    }
  }

  if ( !homeAddress.isEmpty() ) a.insertAddress( homeAddress );
  if ( !businessAddress.isEmpty() ) a.insertAddress( businessAddress );
  if ( !otherAddress.isEmpty() ) a.insertAddress( otherAddress );
}

void importKab( TDEABC::AddressBook *ab, bool override, bool quiet )
{
  TQString fileName = TDEGlobal::dirs()->saveLocation( "data", "kab/" );
  fileName += "addressbook.kab";
  if ( !TQFile::exists( fileName ) ) {
    if ( !quiet ) {
      KMessageBox::error( 0, "<qt>" + i18n( "Address book file <b>%1</b> not found! Make sure the old address book is located there and you have read permission for this file." )
                          .arg( fileName ) + "</qt>" );
    }
    kdDebug(5700) << "No KDE 2 addressbook found." << endl;
    return;
  }

  kdDebug(5700) << "Converting old-style kab addressbook to "
               "new-style tdeabc addressbook." << endl;

  KabAPI kab( 0 );
  if ( kab.init() != ::AddressBook::NoError ) {
    kdDebug(5700) << "Error initing kab" << endl;
    exit( 1 );
  }

  KabKey key;
  ::AddressBook::Entry entry;

  int num = kab.addressbook()->noOfEntries();

  kdDebug(5700) << "kab Addressbook has " << num << " entries." << endl;

  for ( int i = 0; i < num; ++i ) {
    if ( ::AddressBook::NoError != kab.addressbook()->getKey( i, key ) ) {
      kdDebug(5700) << "Error getting key for index " << i << " from kab." << endl;
      continue;
    }
    if ( ::AddressBook::NoError != kab.addressbook()->getEntry( key, entry ) ) {
      kdDebug(5700) << "Error getting entry for index " << i << " from kab." << endl;
      continue;
    }

    Addressee a;

    // Convert custom entries
    int count = 0;
    bool idFound = false;
    TQStringList::ConstIterator customIt;
    for ( customIt = entry.custom.begin(); customIt != entry.custom.end(); ++customIt ) {
      if ( (*customIt).startsWith( "X-KABC-UID:" ) ) {
        a.setUid( (*customIt).mid( (*customIt).find( ":" ) + 1 ) );
        idFound = true;
      } else if ( (*customIt).startsWith( "KMail:1.0\n" ) ) {
        readKAddressBookEntries( *customIt, a );
      } else {
        a.insertCustom( "tdeab2tdeabc", TQString::number( count++ ), *customIt );
      }
    }
    if ( idFound ) {
      if ( !override ) continue;
    } else {
      entry.custom << "X-KABC-UID:" + a.uid();
      ::AddressBook::ErrorCode error = kab.addressbook()->change( key, entry );
      if ( error != ::AddressBook::NoError ) {
        kdDebug(5700) << "kab.change returned with error " << error << endl;
      } else {
        kdDebug(5700) << "Wrote back to kab uid " << a.uid() << endl;
      }
    }

    a.setTitle( entry.title );
    a.setFormattedName( entry.fn );
    a.setPrefix( entry.nameprefix );
    a.setGivenName( entry.firstname );
    a.setAdditionalName( entry.middlename );
    a.setFamilyName( entry.lastname );
    a.setBirthday( entry.birthday );

    TQStringList::ConstIterator emailIt;
    for ( emailIt = entry.emails.begin(); emailIt != entry.emails.end(); ++emailIt )
      a.insertEmail( *emailIt );

    TQStringList::ConstIterator phoneIt;
    for ( phoneIt = entry.telephone.begin(); phoneIt != entry.telephone.end(); ++phoneIt ) {
      int kabType = (*phoneIt++).toInt();
      if ( phoneIt == entry.telephone.end() ) break;
      TQString number = *phoneIt;
      int type = 0;
      if ( kabType == ::AddressBook::Fixed ) type = PhoneNumber::Voice;
      else if ( kabType == ::AddressBook::Mobile ) type = PhoneNumber::Cell | PhoneNumber::Voice;
      else if ( kabType == ::AddressBook::Fax ) type = PhoneNumber::Fax;
      else if ( kabType == ::AddressBook::Modem ) type = PhoneNumber::Modem;
      a.insertPhoneNumber( PhoneNumber( number, type ) );
    }

    if ( entry.URLs.count() > 0 ) {
      a.setUrl( KURL( entry.URLs.first() ) );
      if ( entry.URLs.count() > 1 ) {
        kdWarning() << "More than one URL. Ignoring all but the first." << endl;
      }
    }

    int noAdr = entry.noOfAddresses();
    for ( int j = 0; j < noAdr; ++j ) {
      ::AddressBook::Entry::Address kabAddress;
      entry.getAddress( j, kabAddress );

      Address adr;

      adr.setStreet( kabAddress.address );
      adr.setPostalCode( kabAddress.zip );
      adr.setLocality( kabAddress.town );
      adr.setCountry( kabAddress.country );
      adr.setRegion( kabAddress.state );

      TQString label;
      if ( !kabAddress.headline.isEmpty() ) label += kabAddress.headline + "\n";
      if ( !kabAddress.position.isEmpty() ) label += kabAddress.position + "\n";
      if ( !kabAddress.org.isEmpty() ) label += kabAddress.org + "\n";
      if ( !kabAddress.orgUnit.isEmpty() ) label += kabAddress.orgUnit + "\n";
      if ( !kabAddress.orgSubUnit.isEmpty() ) label += kabAddress.orgSubUnit + "\n";
      if ( !kabAddress.deliveryLabel.isEmpty() ) label += kabAddress.deliveryLabel + "\n";
      adr.setLabel( label );

      a.insertAddress( adr );
    }

    TQString note = entry.comment;

    if ( !entry.user1.isEmpty() ) note += "\nUser1: " + entry.user1;
    if ( !entry.user2.isEmpty() ) note += "\nUser2: " + entry.user2;
    if ( !entry.user3.isEmpty() ) note += "\nUser3: " + entry.user3;
    if ( !entry.user4.isEmpty() ) note += "\nUser4: " + entry.user4;

    if ( !entry.keywords.count() == 0 ) note += "\nKeywords: " + entry.keywords.join( ", " );

    TQStringList::ConstIterator talkIt;
    for ( talkIt = entry.talk.begin(); talkIt != entry.talk.end(); ++talkIt ) {
      note += "\nTalk: " + (*talkIt);
    }

    a.setNote( note );

    a.setPrefix( entry.rank + a.prefix() );  // Add rank to prefix

    a.setCategories( entry.categories );

    kdDebug(5700) << "Addressee: " << a.familyName() << endl;

    ab->insertAddressee( a );
  }

  kab.save( true );
}

int main( int argc, char **argv )
{
  TDEAboutData aboutData( "tdeab2tdeabc", I18N_NOOP( "Kab to Kabc Converter" ), "0.1" );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  TDECmdLineArgs::init( argc, argv, &aboutData );
  TDECmdLineArgs::addCmdLineOptions( options );

  TDEApplication app;

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

  bool override = false;

  if ( args->isSet( "override" ) ) {
    kdDebug() << "Override existing entries." << endl;

    override = true;
  }

  bool quiet = false;

  if ( args->isSet( "quiet" ) )
    quiet = true;

  if ( args->isSet( "disable-autostart" ) ) {
    kdDebug() << "Disable autostart." << endl;

    TDEConfig *config = app.config();
    config->setGroup( "Startup" );
    config->writeEntry( "EnableAutostart", false );
  }

  TDEABC::AddressBook *tdeabcBook = StdAddressBook::self();

  importKMailAddressBook( tdeabcBook );

  importKab( tdeabcBook, override, quiet );

  StdAddressBook::save();

  kdDebug(5700) << "Saved tdeabc addressbook to '" << tdeabcBook->identifier() << "'" << endl;
}

