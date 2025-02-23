/*
    This file is part of KDE 

    Copyright (C) 1998 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License 
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqstring.h>
#include <tdeapplication.h>
#include <dcopclient.h>
#include <stdio.h>
#include <tqvaluelist.h>
#include <kservice.h>


int main(int argc, char *argv[])
{
   TDEApplication::tdeinitExec("konsole");

   TDEApplication k(argc, argv, "tdelaunchertest");
 
   kapp->dcopClient()->registerAs( kapp->name()) ;

#if 0
   TQString error;
   TQCString dcopService;
   int pid;
   int result = TDEApplication::startServiceByDesktopName(
		TQString::fromLatin1("konsole"), TQString::null, &error, &dcopService, &pid );

   printf("Result = %d, error = \"%s\", dcopService = \"%s\", pid = %d\n",
      result, error.ascii(), dcopService.data(), pid);

   result = TDEApplication::startServiceByDesktopName(
		TQString::fromLatin1("konqueror"), TQString::null,  &error, &dcopService, &pid );

   printf("Result = %d, error = \"%s\", dcopService = \"%s\", pid = %d\n",
      result, error.ascii(), dcopService.data(), pid);
#endif
}

