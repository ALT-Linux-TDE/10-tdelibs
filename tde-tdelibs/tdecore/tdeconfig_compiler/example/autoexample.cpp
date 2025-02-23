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

#include "general_base.h"
#include "myoptions_base.h"

#include "exampleprefs_base.h"

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <tdeconfigdialog.h>

#include <tqlabel.h>

int main( int argc, char **argv )
{
  TDEAboutData aboutData( "example", I18N_NOOP("autoconfig example"), "0.1" );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  TDECmdLineArgs::init( argc, argv, &aboutData );

  TDEApplication app;

  ExamplePrefsBase configSkeleton( "dummy1", "dummy2" );
  configSkeleton.readConfig();

  TDEConfigDialog *dialog = new TDEConfigDialog( 0, "settings", &configSkeleton );
  
  GeneralBase *general = new GeneralBase( 0 );
  dialog->addPage( general, i18n("General"), "General", "" );

  MyOptionsBase *myOptions = new MyOptionsBase( 0 );
  dialog->addPage( myOptions, i18n("MyOptions"), "MyOptions", "" );

  app.setMainWidget( dialog );

  dialog->show();
    
  return app.exec();
}
