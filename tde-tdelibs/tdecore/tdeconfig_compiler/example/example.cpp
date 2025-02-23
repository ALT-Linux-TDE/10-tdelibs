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

#include "exampleprefs_base.h"

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>

int main( int argc, char **argv )
{
  TDEAboutData aboutData( "example", I18N_NOOP("cfgc example"), "0.1" );
  aboutData.addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  TDECmdLineArgs::init( argc, argv, &aboutData );

  TDEApplication app;

  ExamplePrefsBase *prefs = new ExamplePrefsBase("Trans1", "Folder2");
  
  prefs->readConfig();

  prefs->setAnotherOption(17);

  kdWarning() << "Another Option  = " << prefs->anotherOption() << endl;
  kdWarning() << "Another Option2 = " << prefs->anotherOption2() << endl;
  kdWarning() << "MyPaths         = " << prefs->myPaths() << endl;
  kdWarning() << "MyPaths2        = " << prefs->myPaths2() << endl;
}
