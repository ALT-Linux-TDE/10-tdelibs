/*
This file is part of KDE

  Copyright (C) 1998-2000 Waldo Bastian (bastian@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <dcopclient.h>
#include <tdecmdlineargs.h>
#include <tdelocale.h>
#include <tdeapplication.h>

static const char description[] =
	I18N_NOOP("HTTP Cookie Daemon");

static const char version[] = "1.0";

static const TDECmdLineOptions options[] =
{
 { "shutdown", I18N_NOOP("Shut down cookie jar"), 0 },
 { "remove <domain>", I18N_NOOP("Remove cookies for domain"), 0 },
 { "remove-all", I18N_NOOP("Remove all cookies"), 0 },
 { "reload-config", I18N_NOOP("Reload configuration file"), 0 },
 TDECmdLineLastOption
};

extern "C" TDE_EXPORT int kdemain(int argc, char *argv[])
{
   TDELocale::setMainCatalogue("tdelibs");
   TDECmdLineArgs::init(argc, argv, "kcookiejar", I18N_NOOP("HTTP cookie daemon"),
		      description, version);

   TDECmdLineArgs::addCmdLineOptions( options );

   TDEInstance a("kcookiejar");
   
   kapp->dcopClient()->attach();

   TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
   TQCString replyType;
   TQByteArray replyData;
   if (args->isSet("remove-all"))
   {
      kapp->dcopClient()->call( "kded", "kcookiejar", "deleteAllCookies()", TQByteArray(), replyType, replyData);
   }
   if (args->isSet("remove"))
   {
      TQString domain = args->getOption("remove");
      TQByteArray params;
      TQDataStream stream(params, IO_WriteOnly);
      stream << domain;
      kapp->dcopClient()->call( "kded", "kcookiejar", "deleteCookiesFromDomain(TQString)", params, replyType, replyData);
   }
   if (args->isSet("shutdown"))
   {
      TQCString module = "kcookiejar";
      TQByteArray params;
      TQDataStream stream(params, IO_WriteOnly);
      stream << module;
      kapp->dcopClient()->call( "kded", "kded", "unloadModule(TQCString)", params, replyType, replyData);
   }
   else if(args->isSet("reload-config"))
   {
      kapp->dcopClient()->call( "kded", "kcookiejar", "reloadPolicy()", TQByteArray(), replyType, replyData);
   }
   else
   {
      TQCString module = "kcookiejar";
      TQByteArray params;
      TQDataStream stream(params, IO_WriteOnly);
      stream << module;
      kapp->dcopClient()->call( "kded", "kded", "loadModule(TQCString)", params, replyType, replyData);
   }

   return 0;
}
