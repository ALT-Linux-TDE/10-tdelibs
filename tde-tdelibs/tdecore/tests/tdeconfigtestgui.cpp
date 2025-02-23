/* This file is part of the KDE libraries
    Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)

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

#include "tdeconfigtestgui.h"
#include "tdeconfigtestgui.moc"

//
// configtest.cpp: libKDEcore example
//
// demonstrates use of TDEConfig class
//
// adapted from Qt widgets demo

#include <unistd.h>
#include <stdlib.h>
#include <tdeapplication.h>
#include <tqdialog.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqdatetime.h>
#include <kdebug.h>
#include <ksimpleconfig.h>
#include <config.h>

// Standard Qt widgets

#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>

// KDE includes
#include <tdeconfig.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

//
// Construct the TDEConfigTestView with buttons
//

TDEConfigTestView::TDEConfigTestView( TQWidget *parent, const char *name )
    : TQDialog( parent, name ),
      pConfig( 0L ),
      pFile( 0L ),
      pStream( 0L )
{
  // Set the window caption/title

  setCaption( "TDEConfig test" );

  // Label and edit for the app config file
  pAppFileLabel = new TQLabel( this, "appconfiglabel" );
  pAppFileLabel->setText( "Application config file:" );
  pAppFileLabel->setGeometry( 20, 20, 200, 20 );

  pAppFileEdit = new TQLineEdit( this, "appconfigedit" );
  pAppFileEdit->setGeometry( 240, 20, 160, 20 );
  connect( pAppFileEdit, TQ_SIGNAL(returnPressed()),
	   TQ_SLOT(appConfigEditReturnPressed()));

  // Label and edit for the group
  pGroupLabel = new TQLabel( this, "grouplabel" );
  pGroupLabel->setText( "Group:" );
  pGroupLabel->setGeometry( 20, 60, 80, 20 );

  pGroupEdit = new TQLineEdit( this, "groupedit" );
  pGroupEdit->setGeometry( 120, 60, 100, 20 );
  connect( pGroupEdit, TQ_SIGNAL(returnPressed()),
	   TQ_SLOT(groupEditReturnPressed()));

  // Edit and label for the key/value pair
  pKeyEdit = new TQLineEdit( this, "keyedit" );
  pKeyEdit->setGeometry( 20, 100, 80, 20 );
  connect( pKeyEdit, TQ_SIGNAL( returnPressed()),
	   TQ_SLOT(keyEditReturnPressed()));

  pEqualsLabel = new TQLabel( this, "equalslabel" );
  pEqualsLabel->setGeometry( 105, 100, 20, 20 );
  pEqualsLabel->setText( "=" );

  pValueEdit = new TQLineEdit( this, "valueedit" );
  pValueEdit->setGeometry( 120, 100, 100, 20 );
  pValueEdit->setText( "---" );

  pWriteButton = new TQPushButton( this, "writebutton" );
  pWriteButton->setGeometry( 20,140, 80, 20 );
  pWriteButton->setText( "Write entry" );
  connect( pWriteButton, TQ_SIGNAL(clicked()), TQ_SLOT( writeButtonClicked() ) );

  // Labels for the info line
  pInfoLabel1 = new TQLabel( this, "infolabel1" );
  pInfoLabel1->setGeometry( 20, 200, 60, 20 );
  pInfoLabel1->setText( "Info:" );

  pInfoLabel2 = new TQLabel( this, "infolabel2" );
  pInfoLabel2->setGeometry( 100, 200, 300, 20 );
  pInfoLabel2->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );

  // Quit button
  pQuitButton = new TQPushButton( this, "quitbutton" );
  pQuitButton->setText( "Quit" );
  pQuitButton->setGeometry( 340, 60, 60, 60 );
  connect( pQuitButton, TQ_SIGNAL(clicked()), tqApp, TQ_SLOT(quit()) );

  // create a default TDEConfig object in order to be able to start right away
  pConfig = new TDEConfig( TQString::null );
}

TDEConfigTestView::~TDEConfigTestView()
{
    delete pConfig;
    delete pFile;
    delete pStream;
}

void TDEConfigTestView::appConfigEditReturnPressed()
{
    // if there already was a config object, delete it and its associated data
    delete pConfig;
    pConfig = 0L;
    delete pFile;
    pFile = 0L;
    delete pStream;
    pStream = 0L;

  // create a new config object
  if( !pAppFileEdit->text().isEmpty() )
	  pConfig = new TDEConfig( pAppFileEdit->text() );

  pInfoLabel2->setText( "New config object created." );
}

void TDEConfigTestView::groupEditReturnPressed()
{
  pConfig->setGroup( pGroupEdit->text() );
  // according to the Qt doc, this is begging for trouble, but for a
  // test program this will do
  TQString aText;
  aText.sprintf( "Group set to %s", TQString( pConfig->group() ).isEmpty() ?
		 TQString("<default>").ascii() : pConfig->group().ascii() );
  pInfoLabel2->setText( aText );
}

void TDEConfigTestView::keyEditReturnPressed()
{
  TQString aValue = pConfig->readEntry( pKeyEdit->text() );
  // just checking aValue.isNull() would be easier here, but this is
  // to demonstrate the HasKey()-method. Besides, it is better data
  // encapsulation because we do not make any assumption about coding
  // non-values here.
  if( !pConfig->hasKey( pKeyEdit->text() ) )
    {
      pInfoLabel2->setText( "Key not found!" );
      pValueEdit->setText( "---" );
    }
  else
    {
      pInfoLabel2->setText( "Key found!" );
      pValueEdit->setText( aValue );
    }
}

void TDEConfigTestView::writeButtonClicked()
{
  pConfig->writeEntry( pKeyEdit->text(), TQString( pValueEdit->text() ) );
  pInfoLabel2->setText( "Entry written" );

  kdDebug() << "Entry written: " << 27 << endl;
}


int main( int argc, char **argv )
{
  TDEApplication a( argc, argv, TQCString ("bla") );

  TDEConfigTestView *w = new TDEConfigTestView();
  a.setMainWidget( w );
  return w->exec();
}
