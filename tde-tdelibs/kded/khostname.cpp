/*  This file is part of the KDE libraries
 *  Copyright (C) 2001 Waldo Bastian <bastian@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <tqfile.h>
#include <tqtextstream.h>
#include <tqregexp.h>

#include <dcopclient.h>

#include <tdecmdlineargs.h>
#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdeaboutdata.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <kde_file.h>

static TDECmdLineOptions options[] = {
   { "+old", I18N_NOOP("Old hostname"), 0 },
   { "+new", I18N_NOOP("New hostname"), 0 },
   TDECmdLineLastOption
};

static const char appName[] = "kdontchangethehostname";
static const char appVersion[] = "1.1";

class KHostName
{
public:
   KHostName();

   void changeX();
   void changeDcop();
   void changeStdDirs(const TQCString &type);
   void changeSessionManager();

protected:
   TQCString oldName;
   TQCString newName;
   TQCString display;
   TQCString home;
};

KHostName::KHostName()
{
   TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
   if (args->count() != 2)
      args->usage();
   oldName = args->arg(0);
   newName = args->arg(1);
   if (oldName == newName)
      exit(0);  

   home = ::getenv("HOME");
   if (home.isEmpty())
   {
      fprintf(stderr, "%s", i18n("Error: HOME environment variable not set.\n").local8Bit().data());
      exit(1);
   }

   display = ::getenv("DISPLAY");
   // strip the screen number from the display
   display.replace(TQRegExp("\\.[0-9]+$"), "");
   if (display.isEmpty())
   {
      fprintf(stderr, "%s", i18n("Error: DISPLAY environment variable not set.\n").local8Bit().data());
      exit(1);
   }
}

static QCStringList split(const TQCString &str)
{
   const char *s = str.data();
   QCStringList result;
   while (*s)
   {
      const char *i = strchr(s, ' ');
      if (!i)
      {
         result.append(TQCString(s));
         return result;
      }
      result.append(TQCString(s, i-s+1));
      s = i;
      while (*s == ' ') s++;
   }
   return result;
}

void KHostName::changeX()
{
   const char* xauthlocalhostname = getenv("XAUTHLOCALHOSTNAME");
   TQString cmd = "xauth -n list";
   FILE *xFile = popen(TQFile::encodeName(cmd), "r");
   if (!xFile)
   {
      fprintf(stderr, "Warning: Can't run xauth.\n");
      return;   
   }
   QCStringList lines;
   {
      char buf[1024+1];
      while (!feof(xFile))
      {
         buf[1024]='\0';
         TQCString line = fgets(buf, 1024, xFile);
         if (line.length())
            line.truncate(line.length()-1); // Strip LF.
         if (!line.isEmpty())
            lines.append(line);
      }
   }
   pclose(xFile);

   for(QCStringList::ConstIterator it = lines.begin();
      it != lines.end(); ++it)
   {
      QCStringList entries = split(*it);
      if (entries.count() != 3)
         continue;

      TQCString netId = entries[0];
      TQCString authName = entries[1];
      TQCString authKey = entries[2];

      int i = netId.findRev(':');
      if (i == -1)
         continue;
      TQCString netDisplay = netId.mid(i);
      if (netDisplay != display)
         continue;

      i = netId.find('/');
      if (i == -1)
         continue;

      TQCString newNetId = newName+netId.mid(i);
      TQCString oldNetId = netId.left(i);

      if(oldNetId != oldName
        && (!xauthlocalhostname || strcmp(xauthlocalhostname, oldNetId.data()) != 0))
        continue;

      // don't nuke the xauth when XAUTHLOCALHOSTNAME points to it
      if (!xauthlocalhostname || oldNetId != xauthlocalhostname)
      {
        cmd = "xauth -n remove "+TDEProcess::quote(netId);
        system(TQFile::encodeName(cmd));
      }
      cmd = "xauth -n add ";
      cmd += TDEProcess::quote(newNetId);
      cmd += " ";
      cmd += TDEProcess::quote(authName);
      cmd += " ";
      cmd += TDEProcess::quote(authKey);
      system(TQFile::encodeName(cmd));
   }
}

void KHostName::changeDcop()
{
   TQCString origFNameOld = DCOPClient::dcopServerFileOld(oldName);
   TQCString fname = DCOPClient::dcopServerFile(oldName);
   TQCString origFName = fname;
   FILE *dcopFile = fopen(fname.data(), "r");
   if (!dcopFile)
   {
      fprintf(stderr, "Warning: Can't open '%s' for reading.\n", fname.data());
      return;
   }

   TQCString line1, line2;
   {
     char buf[1024+1];
     line1 = fgets(buf, 1024, dcopFile);
     if (line1.length())
            line1.truncate(line1.length()-1); // Strip LF.

     line2 = fgets(buf, 1024, dcopFile);
     if (line2.length())
            line2.truncate(line2.length()-1); // Strip LF.
   }
   fclose(dcopFile);

   TQCString oldNetId = line1;

   if (!newName.isEmpty())
   {
      int i = line1.findRev(':');
      if (i == -1)
      {
         fprintf(stderr, "Warning: File '%s' has unexpected format.\n", fname.data());
         return;
      }
      line1 = "local/"+newName+line1.mid(i);
      TQCString newNetId = line1;
      fname = DCOPClient::dcopServerFile(newName);
      unlink(fname.data());
      dcopFile = fopen(fname.data(), "w");
      if (!dcopFile)
      {
         fprintf(stderr, "Warning: Can't open '%s' for writing.\n", fname.data());
         return;
      }

      fputs(line1.data(), dcopFile);
      fputc('\n', dcopFile);
      fputs(line2.data(), dcopFile);
      fputc('\n', dcopFile);

      fclose(dcopFile);

      TQCString compatLink = DCOPClient::dcopServerFileOld(newName);
      ::symlink(fname.data(), compatLink.data()); // Compatibility link

      // Update .ICEauthority
      TQString cmd = ICEAUTH_COMMAND " list "+TDEProcess::quote("netid="+oldNetId);
      FILE *iceFile = popen(TQFile::encodeName(cmd), "r");
      if (!iceFile)
      {
         fprintf(stderr, "Warning: Can't run iceauth.\n");
         return;   
      }
      QCStringList lines;
      {
         char buf[1024+1];
         while (!feof(iceFile))
         {
            TQCString line = fgets(buf, 1024, iceFile);
            if (line.length())
               line.truncate(line.length()-1); // Strip LF.
            if (!line.isEmpty())
               lines.append(line);
         }
      }
      pclose(iceFile);

      for(QCStringList::ConstIterator it = lines.begin();
         it != lines.end(); ++it)
      {
         QCStringList entries = split(*it);
         if (entries.count() != 5)
            continue;

         TQCString protName = entries[0];
         TQCString netId = entries[2];
         TQCString authName = entries[3];
         TQCString authKey = entries[4];
         if (netId != oldNetId)
            continue;

         cmd = ICEAUTH_COMMAND " add ";
         cmd += TDEProcess::quote(protName);
         cmd += " '' ";
         cmd += TDEProcess::quote(newNetId);
         cmd += " ";
         cmd += TDEProcess::quote(authName);
         cmd += " ";
         cmd += TDEProcess::quote(authKey);
         system(TQFile::encodeName(cmd));
      }
   }

   // Remove old entries, but only if XAUTHLOCALHOSTNAME doesn't point
   // to it
   char* xauthlocalhostname = getenv("XAUTHLOCALHOSTNAME");
   if (!xauthlocalhostname || !oldNetId.contains(xauthlocalhostname))
   {
      TQString cmd = ICEAUTH_COMMAND " remove "+TDEProcess::quote("netid="+oldNetId);
      system(TQFile::encodeName(cmd));
      unlink(origFName.data());
      origFName = DCOPClient::dcopServerFileOld(oldName); // Compatibility link
      unlink(origFName.data());
   }
}

void KHostName::changeStdDirs(const TQCString &type)
{
   // We make links to the old dirs cause we can't delete the old dirs.
   TQCString oldDir = TQFile::encodeName(TQString("%1%2-%3").arg(TDEGlobal::dirs()->localtdedir()).arg(type.data()).arg(oldName.data()));
   TQCString newDir = TQFile::encodeName(TQString("%1%2-%3").arg(TDEGlobal::dirs()->localtdedir()).arg(type.data()).arg(newName.data()));

   KDE_struct_stat st_buf;

   int result = KDE_lstat(oldDir.data(), &st_buf);
   if (result == 0)
   {
      if (S_ISLNK(st_buf.st_mode))
      {
         char buf[4096+1];
         result = readlink(oldDir.data(), buf, 4096);
         if (result >= 0)
         {
            buf[result] = 0;
            result = symlink(buf, newDir.data());
         }
      }
      else if (S_ISDIR(st_buf.st_mode))
      {
         result = symlink(oldDir.data(), newDir.data());
      }
      else
      {
         result = -1;
      }
   }
   if (result != 0)
   {
      system(("lnusertemp "+type).data());
   }
}

void KHostName::changeSessionManager()
{
   TQCString sm = ::getenv("SESSION_MANAGER");
   if (sm.isEmpty())
   {
      fprintf(stderr, "Warning: No session management specified.\n");
      return;
   }
   int i = sm.findRev(':');
   if ((i == -1) || (sm.left(6) != "local/"))
   {
      fprintf(stderr, "Warning: Session Management socket '%s' has unexpected format.\n", sm.data());
      return;
   }
   sm = "local/"+newName+sm.mid(i);
   TQCString name = "SESSION_MANAGER";
   TQByteArray params;
   TQDataStream stream(params, IO_WriteOnly);
   stream << name << sm;
   DCOPClient *client = new DCOPClient();
   if (!client->attach())
   {
      fprintf(stderr, "Warning: DCOP communication problem, can't fix Session Management.\n");
      delete client;
      return;
   }
   TQCString launcher = TDEApplication::launcher();
   client->send(launcher, launcher, "setLaunchEnv(TQCString,TQCString)", params);
   delete client;
}

int main(int argc, char **argv)
{
   TDELocale::setMainCatalogue("tdelibs");
   TDEAboutData d(appName, I18N_NOOP("KDontChangeTheHostName"), appVersion,
                I18N_NOOP("Informs TDE about a change in hostname"),
                TDEAboutData::License_GPL, "(c) 2001 Waldo Bastian");
   d.addAuthor("Waldo Bastian", I18N_NOOP("Author"), "bastian@kde.org");

   TDECmdLineArgs::init(argc, argv, &d);
   TDECmdLineArgs::addCmdLineOptions(options);

   TDEInstance k(&d);

   KHostName hn;

   hn.changeX();
   hn.changeDcop();
   hn.changeStdDirs("socket");
   hn.changeStdDirs("tmp");
   hn.changeSessionManager();
}

