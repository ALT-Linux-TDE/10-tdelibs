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

  addressee.setNameFromString( "Mr. Tobias Koenig Jr." );
  addressee.setNickName( "tokoe" );
  addressee.setBirthday( TQDate( 1982, 7, 19 ) );
  addressee.setMailer( "mutt1.2" );
  addressee.setTimeZone( TDEABC::TimeZone( +2 ) );

  TDEABC::Geo geo;
  geo.setLatitude( 30 );
  geo.setLongitude( 51 );
  addressee.setGeo( geo );

  addressee.setTitle( "nerd" );
  addressee.setRole( "Maintainer" );
  addressee.setOrganization( "KDE" );
  addressee.setNote( "nerver\ntouch a running system" );
  addressee.setProductId( "testId" );
  addressee.setRevision( TQDateTime::currentDateTime() );
  addressee.setSortString( "koenig" );
  addressee.setUrl( KURL( "http://wgess16.dyndns.org") );
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
  addressee.insertEmail( "tokoe@kde.org", true );
  addressee.insertEmail( "tokoe82@yahoo.de", true );

  TDEABC::PhoneNumber phone1( "3541523475", TDEABC::PhoneNumber::Pref | TDEABC::PhoneNumber::Home );
  TDEABC::PhoneNumber phone2( "+46745673475", TDEABC::PhoneNumber::Work );
  addressee.insertPhoneNumber( phone1 );
  addressee.insertPhoneNumber( phone2 );

  TDEABC::Key key( "secret key", TDEABC::Key::X509 );
  addressee.insertKey( key );

  TQStringList categories;
  categories << "Friends" << "School" << "KDE";
  addressee.setCategories( categories );

  TDEABC::Address a( TDEABC::Address::Work | TDEABC::Address::Postal | TDEABC::Address::Parcel );
  a.setStreet( "6544 Battleford Drive" );
  a.setLocality( "Raleigh" );
  a.setRegion( "NC" );
  a.setPostalCode( "27613-3502" );
  a.setCountry( "U.S.A." );
  addressee.insertAddress( a );

  addressee.insertCustom( "1hsdf", "ertuer", "iurt" );
  addressee.insertCustom( "2hsdf", "ertuer", "iurt" );
  addressee.insertCustom( "3hsdf", "ertuer", "iurt" );

  TDEABC::Addressee::List list;
  for ( int i = 0; i < 1000; ++i ) {
    TDEABC::Addressee addr = addressee;
    addr.setUid( TQString::number( i ) );
    list.append( addr );
  }

  TDEABC::VCardConverter converter;
  TQString txt = converter.createVCards( list );

  TQFile file( "out.vcf" );
  file.open( IO_WriteOnly );

  TQTextStream s( &file );
  s.setEncoding( TQTextStream::UnicodeUTF8 );
  s << txt;
  file.close();

  return 0;
}
