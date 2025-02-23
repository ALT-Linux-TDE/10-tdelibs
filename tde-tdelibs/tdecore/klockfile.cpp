/*
   This file is part of the KDE libraries
   Copyright (c) 2004 Waldo Bastian <bastian@kde.org>
   
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

#include <klockfile.h>

#include <config.h>

#include <sys/types.h>
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif
#include <signal.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

#include <tqfile.h>
#include <tqtextstream.h>

#include <kde_file.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <tdetempfile.h>

// TODO: http://www.spinnaker.de/linux/nfs-locking.html
// TODO: Make regression test

class TDELockFile::TDELockFilePrivate {
public:
   TQString file;
   int staleTime;
   bool isLocked;
   bool recoverLock;
   bool linkCountSupport;
   TQTime staleTimer;
   KDE_struct_stat statBuf;
   int pid;
   TQString hostname;
   TQString instance;
   TQString lockRecoverFile;
};


// 30 seconds
TDELockFile::TDELockFile(const TQString &file)
{
  d = new TDELockFilePrivate();
  d->file = file;
  d->staleTime = 30;
  d->isLocked = false;
  d->recoverLock = false;
  d->linkCountSupport = true;
}

TDELockFile::~TDELockFile()
{
  unlock();
  delete d;
}

int 
TDELockFile::staleTime() const
{
  return d->staleTime;
}


void
TDELockFile::setStaleTime(int _staleTime)
{
  d->staleTime = _staleTime;
}

static bool statResultIsEqual(KDE_struct_stat &st_buf1, KDE_struct_stat &st_buf2)
{
#define FIELD_EQ(what)       (st_buf1.what == st_buf2.what)
  return FIELD_EQ(st_dev) && FIELD_EQ(st_ino) && 
         FIELD_EQ(st_uid) && FIELD_EQ(st_gid) && FIELD_EQ(st_nlink);
#undef FIELD_EQ
}

static bool testLinkCountSupport(const TQCString &fileName)
{
   KDE_struct_stat st_buf;
   // Check if hardlinks raise the link count at all?
   ::link( fileName, fileName+".test" );
   int result = KDE_lstat( fileName, &st_buf );
   ::unlink( fileName+".test" );
   return ((result == 0) && (st_buf.st_nlink == 2));
}

static TDELockFile::LockResult lockFile(const TQString &lockFile, KDE_struct_stat &st_buf, bool &linkCountSupport)
{
  TQCString lockFileName = TQFile::encodeName( lockFile );
  int result = KDE_lstat( lockFileName, &st_buf );
  if (result == 0)
     return TDELockFile::LockFail;
  
  KTempFile uniqueFile(lockFile, TQString::null, 0644);
  uniqueFile.setAutoDelete(true);
  if (uniqueFile.status() != 0)
     return TDELockFile::LockError;

  char hostname[256];
  hostname[0] = 0;
  gethostname(hostname, 255);
  hostname[255] = 0;
  TQCString instanceName = TDECmdLineArgs::appName();

  (*(uniqueFile.textStream())) << TQString::number(getpid()) << endl
      << instanceName << endl
      << hostname << endl;
  uniqueFile.close();
  
  TQCString uniqueName = TQFile::encodeName( uniqueFile.name() );
      
#ifdef Q_OS_UNIX
  // Create lock file
  result = ::link( uniqueName, lockFileName );
  if (result != 0)
     return TDELockFile::LockError;
     
  if (!linkCountSupport)
     return TDELockFile::LockOK;
#else
  //TODO for win32
  return TDELockFile::LockOK;
#endif

  KDE_struct_stat st_buf2;
  result = KDE_lstat( uniqueName, &st_buf2 );
  if (result != 0)
     return TDELockFile::LockError;

  result = KDE_lstat( lockFileName, &st_buf );
  if (result != 0)
     return TDELockFile::LockError;

  if (!statResultIsEqual(st_buf, st_buf2) || S_ISLNK(st_buf.st_mode) || S_ISLNK(st_buf2.st_mode))
  {
     // SMBFS supports hardlinks by copying the file, as a result the above test will always fail
     if ((st_buf.st_nlink == 1) && (st_buf2.st_nlink == 1) && (st_buf.st_ino != st_buf2.st_ino))
     {
        linkCountSupport = testLinkCountSupport(uniqueName);
        if (!linkCountSupport)
           return TDELockFile::LockOK; // Link count support is missing... assume everything is OK.
     }
     return TDELockFile::LockFail;
  }

  return TDELockFile::LockOK;
}

static TDELockFile::LockResult deleteStaleLock(const TQString &lockFile, KDE_struct_stat &st_buf, bool &linkCountSupport)
{
   // This is dangerous, we could be deleting a new lock instead of
   // the old stale one, let's be very careful
              
   // Create temp file
   KTempFile ktmpFile(lockFile);
   if (ktmpFile.status() != 0)
      return TDELockFile::LockError;
              
   TQCString lckFile = TQFile::encodeName(lockFile);
   TQCString tmpFile = TQFile::encodeName(ktmpFile.name());
   ktmpFile.close();
   ktmpFile.unlink();
              
#ifdef Q_OS_UNIX
   // link to lock file
   if (::link(lckFile, tmpFile) != 0)
      return TDELockFile::LockFail; // Try again later
#else
   //TODO for win32
   return TDELockFile::LockOK;
#endif

   // check if link count increased with exactly one
   // and if the lock file still matches
   KDE_struct_stat st_buf1;
   KDE_struct_stat st_buf2;
   memcpy(&st_buf1, &st_buf, sizeof(KDE_struct_stat));
   st_buf1.st_nlink++;
   if ((KDE_lstat(tmpFile, &st_buf2) == 0) && statResultIsEqual(st_buf1, st_buf2))
   {
      if ((KDE_lstat(lckFile, &st_buf2) == 0) && statResultIsEqual(st_buf1, st_buf2))
      {
         // - - if yes, delete lock file, delete temp file, retry lock
         tqWarning("[tdecore] Deleting stale lockfile %s", lckFile.data());
         ::unlink(lckFile);
         ::unlink(tmpFile);
         return TDELockFile::LockOK;
      }
   }
   
   // SMBFS supports hardlinks by copying the file, as a result the above test will always fail
   if (linkCountSupport)
   {
      linkCountSupport = testLinkCountSupport(tmpFile);
   }

   if (!linkCountSupport && 
       (KDE_lstat(lckFile, &st_buf2) == 0) && 
       statResultIsEqual(st_buf, st_buf2))
   {
      // Without support for link counts we will have a little race condition
      tqWarning("[tdecore] Deleting stale lockfile %s", lckFile.data());
      ::unlink(lckFile);
      ::unlink(tmpFile);
      return TDELockFile::LockOK;
   }
   
   // Failed to delete stale lock file
   tqWarning("[tdecore] WARNING: Problem deleting stale lockfile %s", lckFile.data());
   ::unlink(tmpFile);
   return TDELockFile::LockFail;
}


TDELockFile::LockResult TDELockFile::lock(int options)
{
  if (d->isLocked)
     return TDELockFile::LockOK;

  TDELockFile::LockResult result;     
  int hardErrors = 5;
  int n = 5;
  while(true)
  {
     KDE_struct_stat st_buf;
     result = lockFile(d->file, st_buf, d->linkCountSupport);
     if (result == TDELockFile::LockOK)
     {
        d->staleTimer = TQTime();
        break;
     }
     else if (result == TDELockFile::LockError)
     {
        d->staleTimer = TQTime();
        if (--hardErrors == 0)
        {
           break;
        }
     }
     else // TDELockFile::Fail
     {
        if (!d->staleTimer.isNull() && !statResultIsEqual(d->statBuf, st_buf))
           d->staleTimer = TQTime();
           
        if (!d->staleTimer.isNull())
        {
           bool isStale = false;
           if ((d->pid > 0) && !d->hostname.isEmpty())
           {
              // Check if hostname is us
              char hostname[256];
              hostname[0] = 0;
              gethostname(hostname, 255);
              hostname[255] = 0;
              
              if (d->hostname == hostname)
              {
                 // Check if pid still exists
                 int res = ::kill(d->pid, 0);
                 if ((res == -1) && (errno == ESRCH))
                    isStale = true;
              }
           }
           if (d->staleTimer.elapsed() > (d->staleTime*1000))
              isStale = true;
           
           if (isStale)
           {
              if ((options & LockForce) == 0)
                 return TDELockFile::LockStale;
                 
              result = deleteStaleLock(d->file, d->statBuf, d->linkCountSupport);

              if (result == TDELockFile::LockOK)
              {
                 // Lock deletion successful
                 d->staleTimer = TQTime();
                 continue; // Now try to get the new lock
              }
              else if (result != TDELockFile::LockFail)
              {
                 return result;
              }
           }
        }
        else
        {
           memcpy(&(d->statBuf), &st_buf, sizeof(KDE_struct_stat));
           d->staleTimer.start();
           
           d->pid = -1;
           d->hostname = TQString::null;
           d->instance = TQString::null;
        
           TQFile file(d->file);
           if (file.open(IO_ReadOnly))
           {
              TQTextStream ts(&file);
              if (!ts.atEnd())
                 d->pid = ts.readLine().toInt();
              if (!ts.atEnd())
                 d->instance = ts.readLine();
              if (!ts.atEnd())
                 d->hostname = ts.readLine();
           }
        }
     }
        
     if ((options & LockNoBlock) != 0)
        break;
     
     struct timeval tv;
     tv.tv_sec = 0;
     tv.tv_usec = n*((TDEApplication::random() % 200)+100);
     if (n < 2000)
        n = n * 2;
     
#ifdef Q_OS_UNIX
     select(0, 0, 0, 0, &tv);
#else
     //TODO for win32
#endif
  }
  if (result == LockOK)
     d->isLocked = true;
  return result;
}
   
bool TDELockFile::isLocked() const
{
  return d->isLocked;
}
   
void TDELockFile::unlock()
{
  if (d->isLocked)
  {
     ::unlink(TQFile::encodeName(d->file));
     d->isLocked = false;
  }
}

bool TDELockFile::getLockInfo(int &pid, TQString &hostname, TQString &appname)
{
  if (d->pid == -1)
     return false;
  pid = d->pid;
  hostname = d->hostname;
  appname = d->instance;
  return true;
}
