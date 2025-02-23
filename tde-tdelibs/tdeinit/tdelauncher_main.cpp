/*
  This file is part of the KDE libraries
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

#include "config.h"

#include <unistd.h>
#include <fcntl.h>

#include "tdeapplication.h"
#include "tdelauncher.h"
#include "tdecmdlineargs.h"
#include "kcrash.h"
#include "kdebug.h"
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <tqcstring.h>
#include <tdelocale.h>

#include "tdelauncher_cmds.h"

static void sig_handler(int sig_num)
{
   // No recursion
   signal( SIGHUP, SIG_IGN);
   signal( SIGTERM, SIG_IGN);
   fprintf(stderr, "[tdelauncher] Exiting on signal %d\n", sig_num);
   TDELauncher::destruct(255);
}

static TDECmdLineOptions options[] =
{
  { "new-startup", "Internal", 0 },
  TDECmdLineLastOption
};

extern "C" TDE_EXPORT int kdemain( int argc, char**argv )
{
   // Started via tdeinit.
   if (fcntl(LAUNCHER_FD, F_GETFD) == -1)
   {
      fprintf(stderr, "%s", i18n("[tdelauncher] This program is not supposed to be started manually.\n"
                                 "[tdelauncher] It is started automatically by tdeinit.\n").local8Bit().data());
      return 1;
   }

   TQCString cname = TDEApplication::launcher();
   char *name = cname.data();
   TDECmdLineArgs::init(argc, argv, name, "TDELauncher", "A service launcher.",
                       "v1.0");

   TDELauncher::addCmdLineOptions();
   TDECmdLineArgs::addCmdLineOptions( options );

   // WABA: Make sure not to enable session management.
   putenv(strdup("SESSION_MANAGER="));

   // Allow the locale to initialize properly
   TDELocale::setMainCatalogue("tdelibs");

   TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

   int maxTry = 3;
   while(true)
   {
      TQCString dcopName = TDEApplication::dcopClient()->registerAs(name, false);
      if (dcopName.isEmpty())
      {
         kdWarning() << "[tdelauncher] DCOP communication problem!" << endl;
         return 1;
      }
      if (dcopName == cname)
         break; // Good!

      if (--maxTry == 0)
      {
         kdWarning() << "[tdelauncher] Another instance of tdelauncher is already running!" << endl;
         return 1;
      }
      
      // Wait a bit...
      kdWarning() << "[tdelauncher] Waiting for already running tdelauncher to exit." << endl;
      sleep(1);

      // Try again...
   }
   
   TDELauncher *launcher = new TDELauncher(LAUNCHER_FD, args->isSet("new-startup"));
   launcher->dcopClient()->setDefaultObject( name );
   launcher->dcopClient()->setDaemonMode( true );

   TDECrash::setEmergencySaveFunction(sig_handler);
   signal( SIGHUP, sig_handler);
   signal( SIGPIPE, SIG_IGN);
   signal( SIGTERM, sig_handler);

   launcher->exec();
   return 0;
}

