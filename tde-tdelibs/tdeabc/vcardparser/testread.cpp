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

#include <iostream>
#include <stdlib.h>

#include <tqfile.h>
#include <tqtextstream.h>

#include <kprocess.h>
#include <kdebug.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdelocale.h>
#include <tdeaboutdata.h>

#include "tdeabc/vcardconverter.h"
#include "vcard.h"

static const TDECmdLineOptions options[] =
{
  {"vcard21", I18N_NOOP("vCard 2.1"), 0},
  {"+inputfile", I18N_NOOP("Input file"), 0},
  TDECmdLineLastOption
};

int main( int argc, char **argv )
{
  TDEApplication::disableAutoDcopRegistration();

  TDEAboutData aboutData( "testread", "vCard test reader", "0.1" );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  TDECmdLineArgs::init( argc, argv, &aboutData );
  TDECmdLineArgs::addCmdLineOptions( options );

  TDEApplication app( false, false );

  TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

  if ( args->count() != 1 ) {
    std::cerr << "Missing argument" << std::endl;
    return 1;
  }

  TQString inputFile( args->arg( 0 ) );

  TQFile file( inputFile );
  if ( !file.open( IO_ReadOnly ) ) {
    tqDebug( "Unable to open file '%s' for reading!", file.name().latin1() );
    return 1;
  }

  TQString text;

  TQTextStream s( &file );
  s.setEncoding( TQTextStream::UnicodeUTF8 );
  text = s.read();
  file.close();

  TDEABC::VCardConverter converter;
  TDEABC::Addressee::List list = converter.parseVCards( text );

  if ( args->isSet( "vcard21" ) ) {
    text = converter.createVCards( list, TDEABC::VCardConverter::v2_1 ); // uses version 2.1
  } else {
    text = converter.createVCards( list ); // uses version 3.0
  }

  std::cout << text.utf8();

  return 0;
}
