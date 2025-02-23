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

#include <tqdatastream.h>
#include <tqimage.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <kstandarddirs.h>

#include "addressbook.h"
#include "addressee.h"
#include "picture.h"
#include "sound.h"

#include "binaryformat.h"

#define BINARY_FORMAT_VERSION 1

using namespace TDEABC;

extern "C"
{
  TDE_EXPORT FormatPlugin *format()
  {
    return new BinaryFormat;
  }
}

bool BinaryFormat::load( Addressee &addressee, TQFile *file )
{
  kdDebug(5700) << "BinaryFormat::load()" << endl;
  TQDataStream stream( file );

  if ( !checkHeader( stream ) )
    return false;

  loadAddressee( addressee, stream );

  return true;
}

bool BinaryFormat::loadAll( AddressBook*, Resource *resource, TQFile *file )
{
  kdDebug(5700) << "BinaryFormat::loadAll()" << endl;

  TQDataStream stream( file );

  if ( !checkHeader( stream ) )
    return false;

  TQ_UINT32 entries;

  stream >> entries;

  for ( uint i = 0; i < entries; ++i ) {
    Addressee addressee;
    loadAddressee( addressee, stream );
    addressee.setResource( resource );
    addressee.setChanged( false );
    resource->insertAddressee( addressee );
  }

  return true;
}

void BinaryFormat::save( const Addressee &addressee, TQFile *file )
{
  kdDebug(5700) << "BinaryFormat::save()" << endl;

  TQDataStream stream( file );

  writeHeader( stream );

  TQ_UINT32 entries = 1;
  stream << entries;
  saveAddressee( addressee, stream );
}

void BinaryFormat::saveAll( AddressBook*, Resource *resource, TQFile *file )
{
  kdDebug(5700) << "BinaryFormat::saveAll()" << endl;

  TQ_UINT32 counter = 0;
  TQDataStream stream( file );

  writeHeader( stream );
  // set dummy number of entries
  stream << counter;

  Resource::Iterator it;
  for ( it = resource->begin(); it != resource->end(); ++it ) {
    saveAddressee( (*it), stream );
    counter++;
    (*it).setChanged( false );
  }

  // set real number of entries
  stream.device()->at( 2 * sizeof( TQ_UINT32 ) );
  stream << counter;
}

bool BinaryFormat::checkFormat( TQFile *file ) const
{
  kdDebug(5700) << "BinaryFormat::checkFormat()" << endl;

  TQDataStream stream( file );

  return checkHeader( stream );
}

bool BinaryFormat::checkHeader( TQDataStream &stream ) const
{
  TQ_UINT32 magic, version;
    
  stream >> magic >> version;

  TQFile *file = dynamic_cast<TQFile*>( stream.device() );

  if ( !file ) {
    kdError() << i18n("Not a file?") << endl;
    return false;
  }

  if ( magic != 0x2e93e ) {
    kdError() << TQString(i18n("File '%1' is not binary format.").arg( file->name() )) << endl;
    return false;
  }

  if ( version != BINARY_FORMAT_VERSION ) {
    kdError() << TQString(i18n("File '%1' is the wrong version.").arg( file->name() )) << endl;
    return false;
  }

  return true;
}

void BinaryFormat::writeHeader( TQDataStream &stream )
{
  TQ_UINT32 magic, version;
    
  magic = 0x2e93e;
  version = BINARY_FORMAT_VERSION;

  stream << magic << version;
}

void BinaryFormat::loadAddressee( Addressee &addressee, TQDataStream &stream )
{
  stream >> addressee;
/*
  // load pictures
  Picture photo = addressee.photo();
  Picture logo = addressee.logo();

  if ( photo.isIntern() ) {
    TQImage img;
    if ( !img.load( locateLocal( "data", "tdeabc/photos/" ) + addressee.uid() ) )
      kdDebug(5700) << "No photo available for '" << addressee.uid() << "'." << endl;

    addressee.setPhoto( img );
  }

  if ( logo.isIntern() ) {
    TQImage img;
    if ( !img.load( locateLocal( "data", "tdeabc/logos/" ) + addressee.uid() ) )
      kdDebug(5700) << "No logo available for '" << addressee.uid() << "'." << endl;

    addressee.setLogo( img );
  }

  // load sound
  // TODO: load sound data from file
*/
}

void BinaryFormat::saveAddressee( const Addressee &addressee, TQDataStream &stream )
{
  stream << addressee;
/*
  // load pictures
  Picture photo = addressee.photo();
  Picture logo = addressee.logo();

  if ( photo.isIntern() ) {
    TQImage img = photo.data();
    TQString fileName = locateLocal( "data", "tdeabc/photos/" ) + addressee.uid();

    if ( !img.save( fileName, "PNG" ) )
      kdDebug(5700) << "Unable to save photo for '" << addressee.uid() << "'." << endl;
  }

  if ( logo.isIntern() ) {
    TQImage img = logo.data();
    TQString fileName = locateLocal( "data", "tdeabc/logos/" ) + addressee.uid();

    if ( !img.save( fileName, "PNG" ) )
      kdDebug(5700) << "Unable to save logo for '" << addressee.uid() << "'." << endl;
  }

  // save sound
  // TODO: save the sound data to file
*/
}
