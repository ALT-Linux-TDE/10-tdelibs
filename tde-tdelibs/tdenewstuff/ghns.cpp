/*
    This file is part of KDE.

    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#include <tqlabel.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <kprocess.h>
#include <kdialog.h>

#include "knewstuffgeneric.h"

#include "ghns.h"

using namespace std;

GhnsWidget::GhnsWidget()
{
  mWallpapers = new TDENewStuffGeneric( "kdesktop/wallpaper", this );

  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setMargin( KDialog::marginHint() );
  topLayout->setSpacing( KDialog::spacingHint() );

  topLayout->addWidget( new TQLabel( i18n("Get hot new stuff:"), this ) );

  TQPushButton *button = new TQPushButton( "Wallpapers", this );
  topLayout->addWidget( button );
  connect( button, TQ_SIGNAL( clicked() ), TQ_SLOT( downloadWallpapers() ) );

  topLayout->addSpacing( 5 );

  TQBoxLayout *buttonLayout = new TQHBoxLayout( topLayout );

  buttonLayout->addStretch();

  TQPushButton *closeButton = new TQPushButton( "Close", this );
  buttonLayout->addWidget( closeButton );
  connect( closeButton, TQ_SIGNAL( clicked() ), kapp, TQ_SLOT( quit() ) );
}

GhnsWidget::~GhnsWidget()
{
  delete mWallpapers;
}

void GhnsWidget::downloadWallpapers()
{
  kdDebug() << "downloadWallpapers()" << endl;

  mWallpapers->download();
}

int main(int argc,char **argv)
{
  TDEAboutData aboutData("ghns","Get Hot New Stuff","0.1");
  TDECmdLineArgs::init(argc,argv,&aboutData);

  TDEApplication app;

  GhnsWidget wid;
  app.setMainWidget( &wid );
  wid.show();

  app.exec();
}

#include "ghns.moc"
