/*
    This file is part of libtdeabc.

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

#include <tdeabc/addressee.h>
#include <tdeabc/phonenumber.h>
#include <tdeabc/address.h>
#include <tdeabc/key.h>
#include <tdeabc/picture.h>
#include <tdeabc/sound.h>
#include <tdeabc/secrecy.h>
#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>

#include <tqfile.h>
#include <tqtextstream.h>

#include "vcardconverter.h"

int main( int argc, char **argv )
{
  TDEAboutData aboutData( "testwrite", "vCard test writer", "0.1" );

  TDECmdLineArgs::init( argc, argv, &aboutData );

  TDEApplication app( false, false );


  TDEABC::Addressee addressee;

  addressee.setNameFromString( TQString::fromUtf8("Иван Иванов") );
  addressee.setNickName( TQString::fromUtf8("иванчо") );
  addressee.setBirthday( TQDate( 1981, 7, 19 ) );
  addressee.setMailer( "mutt1.2" );
  addressee.setTimeZone( TDEABC::TimeZone( +2 ) );

  TDEABC::Geo geo;
  geo.setLatitude( 30 );
  geo.setLongitude( 51 );
  addressee.setGeo( geo );

  addressee.setTitle( TQString::fromUtf8("Др") );
  addressee.setRole( TQString::fromUtf8("Самарянин") );
  addressee.setOrganization( TQString::fromUtf8("България ООД") );
  addressee.setNote( TQString::fromUtf8("не\nпипай работеща система") );
  addressee.setProductId( "testId" );
  addressee.setRevision( TQDateTime::currentDateTime() );
  addressee.setSortString( TQString::fromUtf8("сортиране") );
  addressee.setUrl( KURL( "http://wgess17.dyndns.org") );
  addressee.setSecrecy(  TDEABC::Secrecy( TDEABC::Secrecy::Confidential ) );
/*
  TQImage img;
  img.load( "testimg.png", "PNG" );
  TDEABC::Picture photo;
  photo.setData( img );
  addressee.setPhoto( photo );

  TQImage img2;
  img2.load( "testimg.png", "PNG" );
  TDEABC::Picture logo;
  logo.setData( img2 );
  addressee.setLogo( logo );

  TQFile soundFile( "testsound.wav" );
  soundFile.open( IO_ReadOnly );
  TQByteArray data = soundFile.readAll();
  soundFile.close();
  TDEABC::Sound sound;
  sound.setData( data );
  addressee.setSound( sound );
*/
  addressee.insertEmail( TQString::fromUtf8("иван.иванов@българия.оод"), true );
  addressee.insertEmail( TQString::fromUtf8("иванчо@yahoo.de"), true );

  TDEABC::PhoneNumber phone1( "029876543", TDEABC::PhoneNumber::Pref | TDEABC::PhoneNumber::Home );
  TDEABC::PhoneNumber phone2( "+359888111222", TDEABC::PhoneNumber::Work );
  addressee.insertPhoneNumber( phone1 );
  addressee.insertPhoneNumber( phone2 );

  TDEABC::Key key( "secret key", TDEABC::Key::X509 );
  addressee.insertKey( key );

  TQStringList categories;
  categories << "Friends" << "School" << "KDE";
  addressee.setCategories( categories );

  TDEABC::Address a( TDEABC::Address::Work | TDEABC::Address::Postal | TDEABC::Address::Parcel );
  a.setStreet( TQString::fromUtf8("Цар Борис III") );
  a.setLocality( TQString::fromUtf8("София" ));
  a.setRegion( TQString::fromUtf8("София град" ));
  a.setPostalCode( TQString::fromUtf8("1000" ));
  a.setCountry( TQString::fromUtf8("България" ));
  addressee.insertAddress( a );

  addressee.insertCustom( "1hsdf", "test1",TQString::fromUtf8( "ежзик" ));
  addressee.insertCustom( "2hsdf", "test2",TQString::fromUtf8( "ежзик" ));
  addressee.insertCustom( "3hsdf", "test3",TQString::fromUtf8( "ежзик" ));

  addressee.dump();

  TDEABC::Addressee::List list;
  for ( int i = 0; i < 20; ++i ) {
    TDEABC::Addressee addr = addressee;
    addr.setUid( TQString::number( i ) );
    list.append( addr );
  }

  TDEABC::VCardConverter converter;
  TQString txt = converter.createVCards( list );

  TQFile file( "out2.vcf" );
  file.open( IO_WriteOnly );

  TQTextStream s( &file );
  s.setEncoding( TQTextStream::UnicodeUTF8 );
  s << txt;
  file.close();

  return 0;
}
