/* This file is part of the KDE desktop environment

   Copyright (C) 2001, 2002 Michael Brade <brade@kde.org>

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

#include <tqlayout.h>
#include <tqpushbutton.h>

#include <tdeapplication.h>
#include <kdirlister.h>
#include <kdebug.h>

#include "kdirlistertest.h"

#include <cstdlib>


KDirListerTest::KDirListerTest( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  lister = new KDirLister( false /* true */ );
  debug = new PrintSignals;

  TQVBoxLayout* layout = new TQVBoxLayout( this );

  TQPushButton* startH = new TQPushButton( "Start listing $HOME", this );
  TQPushButton* startR= new TQPushButton( "Start listing /", this );
  TQPushButton* test = new TQPushButton( "Many", this );
  TQPushButton* startT = new TQPushButton( "tarfile", this );

  layout->addWidget( startH );
  layout->addWidget( startR );
  layout->addWidget( startT );
  layout->addWidget( test );
  resize( layout->sizeHint() );

  connect( startR, TQ_SIGNAL( clicked() ), TQ_SLOT( startRoot() ) );
  connect( startH, TQ_SIGNAL( clicked() ), TQ_SLOT( startHome() ) );
  connect( startT, TQ_SIGNAL( clicked() ), TQ_SLOT( startTar() ) );
  connect( test, TQ_SIGNAL( clicked() ), TQ_SLOT( test() ) );

  connect( lister, TQ_SIGNAL( started( const KURL & ) ),
           debug,  TQ_SLOT( started( const KURL & ) ) );
  connect( lister, TQ_SIGNAL( completed() ),
           debug,  TQ_SLOT( completed() ) );
  connect( lister, TQ_SIGNAL( completed( const KURL & ) ),
           debug,  TQ_SLOT( completed( const KURL & ) ) );
  connect( lister, TQ_SIGNAL( canceled() ),
           debug,  TQ_SLOT( canceled() ) );
  connect( lister, TQ_SIGNAL( canceled( const KURL & ) ),
           debug,  TQ_SLOT( canceled( const KURL & ) ) );
  connect( lister, TQ_SIGNAL( redirection( const KURL & ) ),
           debug,  TQ_SLOT( redirection( const KURL & ) ) );
  connect( lister, TQ_SIGNAL( redirection( const KURL &, const KURL & ) ),
           debug,  TQ_SLOT( redirection( const KURL &, const KURL & ) ) );
  connect( lister, TQ_SIGNAL( clear() ),
           debug,  TQ_SLOT( clear() ) );
  connect( lister, TQ_SIGNAL( newItems( const KFileItemList & ) ),
           debug,  TQ_SLOT( newItems( const KFileItemList & ) ) );
  connect( lister, TQ_SIGNAL( itemsFilteredByMime( const KFileItemList & ) ),
           debug,  TQ_SLOT( itemsFilteredByMime( const KFileItemList & ) ) );
  connect( lister, TQ_SIGNAL( deleteItem( KFileItem * ) ),
           debug,  TQ_SLOT( deleteItem( KFileItem * ) ) );
  connect( lister, TQ_SIGNAL( refreshItems( const KFileItemList & ) ),
           debug,  TQ_SLOT( refreshItems( const KFileItemList & ) ) );
  connect( lister, TQ_SIGNAL( infoMessage( const TQString& ) ),
           debug,  TQ_SLOT( infoMessage( const TQString& ) ) );
  connect( lister, TQ_SIGNAL( percent( int ) ),
           debug,  TQ_SLOT( percent( int ) ) );
  connect( lister, TQ_SIGNAL( totalSize( TDEIO::filesize_t ) ),
           debug,  TQ_SLOT( totalSize( TDEIO::filesize_t ) ) );
  connect( lister, TQ_SIGNAL( processedSize( TDEIO::filesize_t ) ),
           debug,  TQ_SLOT( processedSize( TDEIO::filesize_t ) ) );
  connect( lister, TQ_SIGNAL( speed( int ) ),
           debug,  TQ_SLOT( speed( int ) ) );

  connect( lister, TQ_SIGNAL( completed() ),
           this,  TQ_SLOT( completed() ) );
}

KDirListerTest::~KDirListerTest()
{
  delete lister;
}

void KDirListerTest::startHome()
{
  KURL home( getenv( "HOME" ) );
  lister->openURL( home, false, false );
//  lister->stop();
}

void KDirListerTest::startRoot()
{
  KURL root( "file:/" );
  lister->openURL( root, true, true );
// lister->stop( root );
}

void KDirListerTest::startTar()
{
  KURL root( "file:/home/jowenn/aclocal_1.tgz" );
  lister->openURL( root, true, true );
// lister->stop( root );
}

void KDirListerTest::test()
{
  KURL home( getenv( "HOME" ) );
  KURL root( "file:/" );
/*  lister->openURL( home, true, false );
  lister->openURL( root, true, true );
  lister->openURL( KURL("file:/etc"), true, true );
  lister->openURL( root, true, true );
  lister->openURL( KURL("file:/dev"), true, true );
  lister->openURL( KURL("file:/tmp"), true, true );
  lister->openURL( KURL("file:/usr/include"), true, true );
  lister->updateDirectory( KURL("file:/usr/include") );
  lister->updateDirectory( KURL("file:/usr/include") );
  lister->openURL( KURL("file:/usr/"), true, true );
*/
  lister->openURL( KURL("file:/dev"), true, true );
}

void KDirListerTest::completed()
{
    if ( lister->url().path() == "/")
    {
        KFileItem* item = lister->findByURL( "/tmp" );
        if ( item )
            kdDebug() << "Found /tmp: " << item << endl;
        else
            kdWarning() << "/tmp not found! Bug in findByURL?" << endl;
    }
}

int main ( int argc, char *argv[] )
{
  TDEApplication app( argc, argv, "kdirlistertest", true /*styles*/ );

  KDirListerTest *test = new KDirListerTest( 0 );
  test->show();
  app.setMainWidget( test );
  return app.exec();
}

#include "kdirlistertest.moc"
