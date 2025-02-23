/*
 *  Copyright (C) 2005 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <kmimetype.h>
#include <kinstance.h>
#include <ktempdir.h>
#include <kprotocolinfo.h>
#include <tqdir.h>
#include <assert.h>
#include <stdlib.h>
#include <unistd.h>

static void checkIcon( const KURL& url, const TQString& expectedIcon )
{
  TQString icon = KMimeType::iconForURL( url );
  if ( icon == expectedIcon )
    tqDebug( "icon for %s is %s, OK", url.prettyURL().latin1(), icon.latin1() );
  else {
    tqDebug( "ERROR: icon for %s is %s, expected %s!", url.prettyURL().latin1(), icon.latin1(), expectedIcon.latin1() );
    exit(1);
  }
}

static void checkExtention( const TQString& fileName, const TQString& expectedExtention ) {
  TQString ext;
  KMimeType::diagnoseFileName(fileName, ext);
  if ( ext == expectedExtention) {
    tqDebug("extension of %s is '%s: OK'", fileName.latin1(), ext.latin1());
  } else {
    tqDebug("extension of %s is '%s, expected %s!'", fileName.latin1(), ext.latin1(), expectedExtention.latin1());
    exit(1);
  }
}

int main( int argc, char** argv )
{
  TDEInstance blah("kmimetypetest");

  // Obviously those tests will need to be fixed if we ever change the name of the icons
  // but at least they unit-test KMimeType::iconForURL.
  KURL url;

  // safely check a "regular" folder
  url.setPath( TQDir::homeDirPath() );
  checkIcon( url, "folder" );
  
  // safely check a non-readable folder
  if (0 != geteuid()) { // can't do this test if we're root
    KTempDir tmp( TQString::null, 0 );
    tmp.setAutoDelete( true );
    url.setPath( tmp.name() );
    checkIcon( url, "folder_locked" );
    chmod( TQFile::encodeName( tmp.name() ), 0500 ); // so we can 'rm -rf' it
  }

  // safely check the trash folder
  if ( KProtocolInfo::isKnownProtocol( TQString("trash") ) ) {
    checkIcon( "trash:/", "trashcan_full" ); // #100321
    checkIcon( "trash:/foo/", "folder" );
  }

  checkExtention(TQString::fromLatin1("foo.pdf"), TQString::fromLatin1("*.pdf"));
  checkExtention(TQString::fromLatin1("foo.ps"), TQString::fromLatin1("*.ps"));

  return 0;
}
