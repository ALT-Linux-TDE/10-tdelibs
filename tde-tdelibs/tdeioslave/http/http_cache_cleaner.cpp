/*
This file is part of KDE

 Copyright (C) 1999-2000 Waldo Bastian (bastian@kde.org)

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
//----------------------------------------------------------------------------
//
// KDE Http Cache cleanup tool
// $Id$

#include <time.h>
#include <stdlib.h>

#include <tqdir.h>
#include <tqstring.h>
#include <tqptrlist.h>

#include <kinstance.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <tdeprotocolmanager.h>

#include <unistd.h>

#include <kdebug.h>

time_t currentDate;
int m_maxCacheAge;
int m_maxCacheSize;

static const char appName[] = "tdeio_http_cache_cleaner";

static const char description[] = I18N_NOOP("TDE HTTP cache maintenance tool");

static const char version[] = "1.0.0";

static const TDECmdLineOptions options[] =
{
   {"clear-all", I18N_NOOP("Empty the cache"), 0},
   TDECmdLineLastOption
};

struct FileInfo {
   TQString name;
   int size; // Size in Kb.
   int age;
};

template class TQPtrList<FileInfo>;

class FileInfoList : public TQPtrList<FileInfo>
{
public:
   FileInfoList() : TQPtrList<FileInfo>() { }
   int compareItems(TQPtrCollection::Item item1, TQPtrCollection::Item item2)
      { return ((FileInfo *)item1)->age - ((FileInfo *)item2)->age; }
};

// !START OF SYNC!
// Keep the following in sync with the cache code in http.cpp
#define CACHE_REVISION "7\n"

FileInfo *readEntry( const TQString &filename)
{
   TQCString CEF = TQFile::encodeName(filename);
   FILE *fs = fopen( CEF.data(), "r");
   if (!fs)
      return 0;

   char buffer[401];
   bool ok = true;

  // CacheRevision
  if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok && (strcmp(buffer, CACHE_REVISION) != 0))
      ok = false;

   // Full URL
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;

   time_t creationDate;
   int age =0;

   // Creation Date
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      creationDate = (time_t) strtoul(buffer, 0, 10);
      age = (int) difftime(currentDate, creationDate);
      if ( m_maxCacheAge && ( age > m_maxCacheAge))
      {
         ok = false; // Expired
      }
   }

   // Expiration Date
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
//WABA: It seems I slightly misunderstood the meaning of "Expire:" header.
#if 0
      time_t expireDate;
      expireDate = (time_t) strtoul(buffer, 0, 10);
      if (expireDate && (expireDate < currentDate))
         ok = false; // Expired
#endif
   }

   // ETag
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      // Ignore ETag
   }

   // Last-Modified
   if (ok && (!fgets(buffer, 400, fs)))
      ok = false;
   if (ok)
   {
      // Ignore Last-Modified
   }


   fclose(fs);
   if (ok)
   {
      FileInfo *info = new FileInfo;
      info->age = age;
      return info;
   }

   unlink( CEF.data());
   return 0;
}
// Keep the above in sync with the cache code in http.cpp
// !END OF SYNC!

void scanDirectory(FileInfoList &fileEntries, const TQString &name, const TQString &strDir)
{
   TQDir dir(strDir);
   if (!dir.exists()) return;

   TQFileInfoList *newEntries = (TQFileInfoList *) dir.entryInfoList();

   if (!newEntries) return; // Directory not accessible ??

   for(TQFileInfo *qFileInfo = newEntries->first();
       qFileInfo;
       qFileInfo = newEntries->next())
   {
       if (qFileInfo->isFile())
       {
          FileInfo *fileInfo = readEntry( strDir + "/" + qFileInfo->fileName());
          if (fileInfo)
          {
             fileInfo->name = name + "/" + qFileInfo->fileName();
             fileInfo->size = (qFileInfo->size() + 1023) / 1024;
             fileEntries.append(fileInfo);
          }
       }
   }
}

extern "C" TDE_EXPORT int kdemain(int argc, char **argv)
{
   TDELocale::setMainCatalogue("tdelibs");
   TDECmdLineArgs::init( argc, argv, appName,
		       I18N_NOOP("TDE HTTP cache maintenance tool"),
		       description, version, true);

   TDECmdLineArgs::addCmdLineOptions( options );

   TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

   bool deleteAll = args->isSet("clear-all");

   TDEInstance ins( appName );

   if (!deleteAll)
   {
      DCOPClient *dcop = new DCOPClient();
      TQCString name = dcop->registerAs(appName, false);
      if (!name.isEmpty() && (name != appName))
      {
         fprintf(stderr, "%s: Already running! (%s)\n", appName, name.data());
         return 0;
      }
   }

   currentDate = time(0);
   m_maxCacheAge = KProtocolManager::maxCacheAge();
   m_maxCacheSize = KProtocolManager::maxCacheSize();

   if (deleteAll)
      m_maxCacheSize = -1;

   TQString strCacheDir = TDEGlobal::dirs()->saveLocation("cache", "http");

   TQDir cacheDir( strCacheDir );
   if (!cacheDir.exists())
   {
      fprintf(stderr, "%s: '%s' does not exist.\n", appName, strCacheDir.ascii());
      return 0;
   }

   TQStringList dirs = cacheDir.entryList( );

   FileInfoList cachedEntries;

   for(TQStringList::Iterator it = dirs.begin();
       it != dirs.end();
       it++)
   {
      if ((*it)[0] != '.')
      {
         scanDirectory( cachedEntries, *it, strCacheDir + "/" + *it);
      }
   }

   cachedEntries.sort();

   int maxCachedSize = m_maxCacheSize / 2;

   for(FileInfo *fileInfo = cachedEntries.first();
       fileInfo;
       fileInfo = cachedEntries.next())
   {
      if (fileInfo->size > maxCachedSize)
      {
         TQCString filename = TQFile::encodeName( strCacheDir + "/" + fileInfo->name);
         unlink(filename.data());
//         kdDebug () << appName << ": Object too big, deleting '" << filename.data() << "' (" << result<< ")" << endl;
      }
   }

   int totalSize = 0;

   for(FileInfo *fileInfo = cachedEntries.first();
       fileInfo;
       fileInfo = cachedEntries.next())
   {
      if ((totalSize + fileInfo->size) > m_maxCacheSize)
      {
         TQCString filename = TQFile::encodeName( strCacheDir + "/" + fileInfo->name);
         unlink(filename.data());
//         kdDebug () << appName << ": Cache too big, deleting '" << filename.data() << "' (" << fileInfo->size << ")" << endl;
      }
      else
      {
         totalSize += fileInfo->size;
// fprintf(stderr, "Keep in cache: %s %d %d total = %d\n", fileInfo->name.ascii(), fileInfo->size, fileInfo->age, totalSize);
      }
   }
   kdDebug () << appName << ": Current size of cache = " << totalSize << " kB." << endl;
   return 0;
}


