/*
    This file is part of KDE 

    Copyright (C) 2004 Waldo Bastian (bastian@kde.org)

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

#include <stdio.h>
#include <stdlib.h>

#include <tqdatetime.h>
#include <tqstring.h>

#include <tdeapplication.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <kstandarddirs.h>

#include "../kcookiejar.cpp"

static const char *description = "KCookiejar regression test";

static KCookieJar *jar;
static TQCString *lastYear;
static TQCString *nextYear;
static TDEConfig *config = 0;


static TDECmdLineOptions options[] =
{
    { "+testfile", "Regression test to run", 0},
    TDECmdLineLastOption
};

static void FAIL(const TQString &msg)
{
   tqWarning("%s", msg.local8Bit().data());
   exit(1);
}

static void popArg(TQCString &command, TQCString & line)
{
   int i = line.find(' ');
   if (i != -1)
   {
      command = line.left(i);
      line = line.mid(i+1);
   }
   else
   {
      command = line;
      line = 0;
   }   
}


static void popArg(TQString &command, TQCString & line)
{
   int i = line.find(' ');
   if (i != -1)
   {
      command = TQString::fromLatin1(line.left(i));
      line = line.mid(i+1);
   }
   else
   {
      command = TQString::fromLatin1(line);
      line = 0;
   }   
}

static void clearConfig()
{
   delete config;
   TQString file = locateLocal("config", "kcookiejar-testconfig");
   TQFile::remove(file);
   config = new TDEConfig(file);
   config->setGroup("Cookie Policy");
   config->writeEntry("RejectCrossDomainCookies", false);
   config->writeEntry("AcceptSessionCookies", false);
   config->writeEntry("IgnoreExpirationDate", false);
   config->writeEntry("CookieGlobalAdvice", "Ask");
   jar->loadConfig(config, false);
}

static void clearCookies()
{
   jar->eatAllCookies();
}

static void saveCookies()
{
   TQString file = locateLocal("config", "kcookiejar-testcookies");
   TQFile::remove(file);
   jar->saveCookies(file);
   delete jar;
   jar = new KCookieJar();
   clearConfig();
   jar->loadCookies(file);
}

static void processCookie(TQCString &line)
{
   TQString policy;
   popArg(policy, line);
   KCookieAdvice expectedAdvice = KCookieJar::strToAdvice(policy);
   if (expectedAdvice == KCookieDunno)
      FAIL(TQString("Unknown accept policy '%1'").arg(policy));

   TQString urlStr;
   popArg(urlStr, line);
   KURL url(urlStr);
   if (!url.isValid())
      FAIL(TQString("Invalid URL '%1'").arg(urlStr));
   if (url.isEmpty())
      FAIL(TQString("Missing URL"));

   line.replace("%LASTYEAR%", *lastYear);
   line.replace("%NEXTYEAR%", *nextYear);

   KHttpCookieList list = jar->makeCookies(urlStr, line, 0);
   
   if (list.isEmpty())
      FAIL(TQString("Failed to make cookies from: '%1'").arg(line));

   for(KHttpCookie *cookie = list.first();
       cookie; cookie = list.next())
   {
      KCookieAdvice cookieAdvice = jar->cookieAdvice(cookie);
      if (cookieAdvice != expectedAdvice)
         FAIL(urlStr+TQString("\n'%2'\nGot advice '%3' expected '%4'").arg(line)
              .arg(KCookieJar::adviceToStr(cookieAdvice))
              .arg(KCookieJar::adviceToStr(expectedAdvice)));
      jar->addCookie(cookie);
   }
}

static void processCheck(TQCString &line)
{
   TQString urlStr;
   popArg(urlStr, line);
   KURL url(urlStr);
   if (!url.isValid())
      FAIL(TQString("Invalid URL '%1'").arg(urlStr));
   if (url.isEmpty())
      FAIL(TQString("Missing URL"));

   TQString expectedCookies = TQString::fromLatin1(line);

   TQString cookies = jar->findCookies(urlStr, false, 0, 0).stripWhiteSpace();
   if (cookies != expectedCookies)
      FAIL(urlStr+TQString("\nGot '%1' expected '%2'")
              .arg(cookies, expectedCookies));
}

static void processClear(TQCString &line)
{
   if (line == "CONFIG")
      clearConfig();
   else if (line == "COOKIES")
      clearCookies();
   else 
      FAIL(TQString("Unknown command 'CLEAR %1'").arg(line));
}

static void processConfig(TQCString &line)
{
   TQCString key;
   popArg(key, line);

   if (key.isEmpty())
      FAIL(TQString("Missing Key"));

   config->setGroup("Cookie Policy");
   config->writeEntry(key.data(), line.data());
   jar->loadConfig(config, false);
}

static void processLine(TQCString line)
{
   if (line.isEmpty())
      return;
   
   if (line[0] == '#')
   {
      if (line[1] == '#')
         tqWarning("%s", line.data());   
      return;
   }

   TQCString command;
   popArg(command, line);
   if (command.isEmpty())
      return;
      
   if (command == "COOKIE")
      processCookie(line);
   else if (command == "CHECK")
      processCheck(line);
   else if (command == "CLEAR")
      processClear(line);
   else if (command == "CONFIG")
      processConfig(line);
   else if (command == "SAVE")
      saveCookies();
   else
      FAIL(TQString("Unknown command '%1'").arg(command));
}

static void runRegression(const TQString &filename)
{
   FILE *file = fopen(filename.local8Bit(), "r");
   if (!file)
      FAIL(TQString("Can't open '%1'").arg(filename));
      
   char buf[4096];
   while (fgets(buf, sizeof(buf), file))
   {
      int l = strlen(buf);
      if (l)
      {
         l--;
         buf[l] = 0;
      }
      processLine(buf);
   }
   tqWarning("%s OK", filename.local8Bit().data());
}

int main(int argc, char *argv[])
{
   TQString arg1;
   TQCString arg2;
   TQString result;

   lastYear = new TQCString(TQString("Fri, 04-May-%1 01:00:00 GMT").arg(TQDate::currentDate().year()-1).utf8());
   nextYear = new TQCString(TQString(" expires=Fri, 04-May-%1 01:00:00 GMT").arg(TQDate::currentDate().year()+1).utf8());

   TDEAboutData about("kcookietest", "kcookietest", "1.0", description, TDEAboutData::License_GPL, "(C) 2004 Waldo Bastian");
   TDECmdLineArgs::init( argc, argv, &about);

   TDECmdLineArgs::addCmdLineOptions( options );

   TDEInstance a("kcookietest");
   
   TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
   if (args->count() != 1)
      TDECmdLineArgs::usage();

   jar = new KCookieJar;

   clearConfig();   
      
   TQString file = args->url(0).path();
   runRegression(file);
   return 0;
}
