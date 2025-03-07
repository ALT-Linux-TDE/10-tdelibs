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

#include <config.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_TEST
#include <test.h>
#endif

#include <tqdatetime.h>
#include <tqdir.h>

#include <kde_file.h>
#include "tdeapplication.h"
#include "ksavefile.h"
#include "kstandarddirs.h"

KSaveFile::KSaveFile(const TQString &filename, int mode)
 : mTempFile(true)
{
   // follow symbolic link, if any
   TQString real_filename = TDEStandardDirs::realFilePath(filename);

   // we only check here if the directory can be written to
   // the actual filename isn't written to, but replaced later
   // with the contents of our tempfile
   if (!checkAccess(real_filename, W_OK))
   {
      mTempFile.setError(EACCES);
      return;
   }

   if (mTempFile.create(real_filename, TQString::fromLatin1(".new"), mode))
   {
      mFileName = real_filename; // Set filename upon success

      // if we're overwriting an existing file, ensure temp file's
      // permissions are the same as existing file so the existing
      // file's permissions are preserved
      KDE_struct_stat stat_buf;
      if (KDE_stat(TQFile::encodeName(real_filename), &stat_buf)==0)
      {
         // But only if we own the existing file
         if (stat_buf.st_uid == getuid())
         {
            bool changePermission = true;
            if (stat_buf.st_gid != getgid())
      {
               if (fchown(mTempFile.handle(), (uid_t) -1, stat_buf.st_gid) != 0)
               {
                  // Use standard permission if we can't set the group
                  changePermission = false;
               }
            }
            if (changePermission)
               fchmod(mTempFile.handle(), stat_buf.st_mode);
         }
      }
   }
}

KSaveFile::~KSaveFile()
{
   if (mTempFile.bOpen)
      close(); // Close if we were still open
}

TQString
KSaveFile::name() const
{
   return mFileName;
}

void
KSaveFile::abort()
{
   mTempFile.close();
   mTempFile.unlink();
}

bool
KSaveFile::close()
{
   if (mTempFile.name().isEmpty() || mTempFile.handle()==-1)
      return false; // Save was aborted already
   if (!mTempFile.sync())
   {
      abort();
      return false;
   }
   if (mTempFile.close())
   {
      if (0==KDE_rename(TQFile::encodeName(mTempFile.name()), TQFile::encodeName(mFileName)))
         return true; // Success!
      mTempFile.setError(errno);
   }
   // Something went wrong, make sure to delete the interim file.
   mTempFile.unlink();
   return false;
}

static int
write_all(int fd, const char *buf, size_t len)
{
   while (len > 0)
   {
      int written = write(fd, buf, len);
      if (written < 0)
      {
          if (errno == EINTR)
             continue;
          return -1;
      }
      buf += written;
      len -= written;
   }
   return 0;
}

bool KSaveFile::backupFile( const TQString& qFilename, const TQString& backupDir,
                            const TQString& backupExtension)
{
   TQCString cFilename = TQFile::encodeName(qFilename);
   const char *filename = cFilename.data();

   int fd = KDE_open( filename, O_RDONLY );
   if (fd < 0)
      return false;

   KDE_struct_stat buff;
   if ( KDE_fstat( fd, &buff) < 0 )
   {
      ::close( fd );
      return false;
   }

   TQCString cBackup;
   if ( backupDir.isEmpty() )
       cBackup = cFilename;
   else
   {
       TQCString nameOnly;
       int slash = cFilename.findRev('/');
       if (slash < 0)
	   nameOnly = cFilename;
       else
	   nameOnly = cFilename.mid(slash + 1);
       cBackup = TQFile::encodeName(backupDir);
       if ( backupDir[backupDir.length()-1] != (TQChar)'/' )
           cBackup += '/';
       cBackup += nameOnly;
   }
   cBackup += TQFile::encodeName(backupExtension);
   const char *backup = cBackup.data();
   int permissions = buff.st_mode & 07777;

   if ( KDE_stat( backup, &buff) == 0)
   {
      if ( unlink( backup ) != 0 )
      {
         ::close(fd);
         return false;
      }
   }

   mode_t old_umask = umask(0);
   int fd2 = KDE_open( backup, O_WRONLY | O_CREAT | O_EXCL, permissions | S_IWUSR);
   umask(old_umask);

   if ( fd2 < 0 )
   {
      ::close(fd);
      return false;
   }

    char buffer[ 32*1024 ];

    while( 1 )
    {
       int n = ::read( fd, buffer, 32*1024 );
       if (n == -1)
       {
          if (errno == EINTR)
              continue;
          ::close(fd);
          ::close(fd2);
          return false;
       }
       if (n == 0)
          break; // Finished

       if (write_all( fd2, buffer, n))
       {
          ::close(fd);
          ::close(fd2);
          return false;
       }
    }

    ::close( fd );

    if (::close(fd2))
        return false;
    return true;
}
