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

#include <kdebug.h>
#include <tdelocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>

#include <tqfile.h>

#include "vcardformatplugin.h"

#include "formatfactory.h"

using namespace TDEABC;

FormatFactory *FormatFactory::mSelf = 0;
static KStaticDeleter<FormatFactory> factoryDeleter;

FormatFactory *FormatFactory::self()
{
  kdDebug(5700) << "FormatFactory::self()" << endl;

  if ( !mSelf )
    factoryDeleter.setObject( mSelf, new FormatFactory );

  return mSelf;
}

FormatFactory::FormatFactory()
{
  mFormatList.setAutoDelete( true );

  // dummy entry for default format
  FormatInfo *info = new FormatInfo;
  info->library = "<NoLibrary>";
  info->nameLabel = i18n( "vCard" );
  info->descriptionLabel = i18n( "vCard Format" );
  mFormatList.insert( "vcard", info );

  const TQStringList list = TDEGlobal::dirs()->findAllResources( "data" ,"tdeabc/formats/*.desktop", true, true );
  for ( TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
  {
    KSimpleConfig config( *it, true );

    if ( !config.hasGroup( "Misc" ) || !config.hasGroup( "Plugin" ) )
	    continue;

    info = new FormatInfo;

    config.setGroup( "Plugin" );
    TQString type = config.readEntry( "Type" );
    info->library = config.readEntry( "X-TDE-Library" );

    config.setGroup( "Misc" );
    info->nameLabel = config.readEntry( "Name" );
    info->descriptionLabel = config.readEntry( "Comment", i18n( "No description available." ) );

    mFormatList.insert( type, info );
  }
}

FormatFactory::~FormatFactory()
{
  mFormatList.clear();
}

TQStringList FormatFactory::formats()
{
  TQStringList retval;

  // make sure 'vcard' is the first entry
  retval << "vcard";

  TQDictIterator<FormatInfo> it( mFormatList );
  for ( ; it.current(); ++it )
    if ( it.currentKey() != "vcard" )
      retval << it.currentKey();

  return retval;
}

FormatInfo *FormatFactory::info( const TQString &type )
{
  if ( type.isEmpty() )
    return 0;
  else
    return mFormatList[ type ];
}

FormatPlugin *FormatFactory::format( const TQString& type )
{
  FormatPlugin *format = 0;

  if ( type.isEmpty() )
    return 0;

  if ( type == "vcard" ) {
    format = new VCardFormatPlugin;
    format->setType( type );
    format->setNameLabel( i18n( "vCard" ) );
    format->setDescriptionLabel( i18n( "vCard Format" ) );
    return format;
  }

  FormatInfo *fi = mFormatList[ type ];
  if (!fi)
	  return 0;
  TQString libName = fi->library;

  KLibrary *library = openLibrary( libName );
  if ( !library )
    return 0;

  void *format_func = library->symbol( "format" );

  if ( format_func ) {
    format = ((FormatPlugin* (*)())format_func)();
    format->setType( type );
    format->setNameLabel( fi->nameLabel );
    format->setDescriptionLabel( fi->descriptionLabel );
  } else {
    kdDebug( 5700 ) << "'" << libName << "' is not a format plugin." << endl;
    return 0;
  }

  return format;
}


KLibrary *FormatFactory::openLibrary( const TQString& libName )
{
  KLibrary *library = 0;

  TQString path = KLibLoader::findLibrary( TQFile::encodeName( libName ) );

  if ( path.isEmpty() ) {
    kdDebug( 5700 ) << "No format plugin library was found!" << endl;
    return 0;
  }

  library = KLibLoader::self()->library( TQFile::encodeName( path ) );

  if ( !library ) {
    kdDebug( 5700 ) << "Could not load library '" << libName << "'" << endl;
    return 0;
  }

  return library;
}
