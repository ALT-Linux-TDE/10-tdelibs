/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "tdeapplication.h"
#include <stdio.h>

#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <tdeglobalsettings.h>
#include <kdebug.h>

int
main(int argc, char *argv[])
{
   TDEAboutData about("kapptest", "kapptest", "version");
   TDECmdLineArgs::init(argc, argv, &about);

   TDEApplication a;

   TQString error;
   TQCString dcopService;
   int pid;
   a.startServiceByDesktopName( "kaddressbook", TQString::null, &error, &dcopService, &pid );
   kdDebug() << "Started. error=" << error << " dcopService=" << dcopService << " pid=" << pid << endl;
   a.exec();
}
