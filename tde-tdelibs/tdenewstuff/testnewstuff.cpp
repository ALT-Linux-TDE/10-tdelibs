/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqlayout.h>
#include <tqfile.h>
#include <tqtextstream.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <kprocess.h>
#include <kdialog.h>

#include "testnewstuff.h"
#include "testnewstuff.moc"

using namespace std;

bool TestNewStuff::install( const TQString &fileName )
{
  kdDebug() << "TestNewStuff::install(): " << fileName << endl;
  TQFile f( fileName );
  if ( !f.open( IO_ReadOnly ) ) {
    kdDebug() << "Error opening file." << endl;
    return false;
  }
  TQTextStream ts( &f );
  kdDebug() << "--BEGIN-NEW_STUFF--" << endl;
  cout << ts.read().utf8();
  kdDebug() << "---END-NEW_STUFF---" << endl;
  return true;
}

bool TestNewStuff::createUploadFile( const TQString &fileName )
{
  TDEProcess p;
  p << "touch" << fileName;
  p.start(TDEProcess::Block);
  kdDebug() << "TestNewStuff::createUploadFile(): " << fileName << endl;
  return true;
}


MyWidget::MyWidget()
{
  mNewStuff = new TestNewStuff;

  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );
  
  TQPushButton *downloadButton = new TQPushButton( "Download", this );
  topLayout->addWidget( downloadButton );
  connect( downloadButton, TQ_SIGNAL( clicked() ), TQ_SLOT( download() ) );

  TQPushButton *uploadButton = new TQPushButton( "Upload", this );
  topLayout->addWidget( uploadButton );
  connect( uploadButton, TQ_SIGNAL( clicked() ), TQ_SLOT( upload() ) );

  topLayout->addSpacing( 5 );

  TQPushButton *closeButton = new TQPushButton( "Close", this );
  topLayout->addWidget( closeButton );
  connect( closeButton, TQ_SIGNAL( clicked() ), kapp, TQ_SLOT( quit() ) );
}

MyWidget::~MyWidget()
{
  delete mNewStuff;
}

void MyWidget::download()
{
  kdDebug() << "MyWidget::download()" << endl;

  mNewStuff->download();
}

void MyWidget::upload()
{
  kdDebug() << "MyWidget::download()" << endl;

  mNewStuff->upload();
}


int main(int argc,char **argv)
{
  TDEAboutData aboutData("knewstufftest","TDENewStuff Test","0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);

  TDEApplication app;

  MyWidget wid;
  app.setMainWidget( &wid );
  wid.show();

  app.exec();
}
