/*  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Waldo Bastian <bastian@kde.org>
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

#include "config.h"

#include "tdesycoca.h"
#include "tdesycocatype.h"
#include "tdesycocafactory.h"

#include <tqdatastream.h>
#include <tqfile.h>
#include <tqbuffer.h>

#include <tdeapplication.h>
#include <dcopclient.h>
#include <tdeglobal.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#include <assert.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
              
#ifdef HAVE_SYS_MMAN_H
#include <sys/mman.h>
#endif

#if defined(Q_OS_SOLARIS) && !defined(__dilos__)
extern "C" 
{
	extern int madvise(caddr_t, size_t, int); 
}
#endif

#ifndef MAP_FAILED
#define MAP_FAILED ((void *) -1)
#endif

template class TQPtrList<KSycocaFactory>;

// The following limitations are in place:
// Maximum length of a single string: 8192 bytes
// Maximum length of a string list: 1024 strings
// Maximum number of entries: 8192
//
// The purpose of these limitations is to limit the impact
// of database corruption.

class KSycocaPrivate {
public:
    KSycocaPrivate() {
        database = 0;
        readError = false;
        updateSig = 0;
        autoRebuild = true;
    }
    TQFile *database;
    TQStringList changeList;
    TQString language;
    bool readError;
    bool autoRebuild;
    TQ_UINT32 updateSig;
    TQStringList allResourceDirs;
};

int KSycoca::version()
{
   return TDESYCOCA_VERSION;
}

// Read-only constructor
KSycoca::KSycoca()
  : DCOPObject("tdesycoca"), m_lstFactories(0), m_str(0), m_barray(0), bNoDatabase(false),
    m_sycoca_size(0), m_sycoca_mmap(0), m_timeStamp(0)
{
   d = new KSycocaPrivate;
   // Register app as able to receive DCOP messages
   if (kapp && !kapp->dcopClient()->isAttached())
   {
      kapp->dcopClient()->attach();
   }
   // We register with DCOP _before_ we try to open the database.
   // This way we can be relative sure that the KDE framework is
   // up and running (tdeinit, dcopserver, klaucnher, kded) and
   // that the database is up to date.
   openDatabase();
   _self = this;
}

bool KSycoca::openDatabase( bool openDummyIfNotFound )
{
   bool result = true;
  
   m_sycoca_mmap = 0;
   m_str = 0;
   m_barray = 0;
   TQString path;
   TQCString tdesycoca_env = getenv("TDESYCOCA");
   if (tdesycoca_env.isEmpty())
      path = TDEGlobal::dirs()->saveLocation("cache") + "tdesycoca";
   else
      path = TQFile::decodeName(tdesycoca_env);

   kdDebug(7011) << "Trying to open tdesycoca from " << path << endl;
   TQFile *database = new TQFile(path);
   bool bOpen = database->open( IO_ReadOnly );
   if (!bOpen)
   {
     path = locate("services", "tdesycoca");
     if (!path.isEmpty())
     {
       kdDebug(7011) << "Trying to open global tdesycoca from " << path << endl;
       delete database;
       database = new TQFile(path);
       bOpen = database->open( IO_ReadOnly );
     }
   }
   
   if (bOpen)
   {
     fcntl(database->handle(), F_SETFD, FD_CLOEXEC);
     m_sycoca_size = database->size();
#ifdef HAVE_MMAP
     m_sycoca_mmap = (const char *) mmap(0, m_sycoca_size,
                                PROT_READ, MAP_SHARED,
                                database->handle(), 0);
     /* POSIX mandates only MAP_FAILED, but we are paranoid so check for
        null pointer too.  */
     if (m_sycoca_mmap == (const char*) MAP_FAILED || m_sycoca_mmap == 0)
     {
        kdDebug(7011) << "mmap failed. (length = " << m_sycoca_size << ")" << endl;
#endif
        m_str = new TQDataStream(database);
#ifdef HAVE_MMAP
     }
     else
     {
#ifdef HAVE_MADVISE
	(void) madvise((char*)m_sycoca_mmap, m_sycoca_size, MADV_WILLNEED);
#endif
        m_barray = new TQByteArray();
        m_barray->setRawData(m_sycoca_mmap, m_sycoca_size);
        TQBuffer *buffer = new TQBuffer( *m_barray );
        buffer->open(IO_ReadWrite);
        m_str = new TQDataStream( buffer);
     }
#endif
     bNoDatabase = false;
   }
   else
   {
     kdDebug(7011) << "Could not open tdesycoca" << endl;

     // No database file
     delete database;
     database = 0;

     bNoDatabase = true;
     if (openDummyIfNotFound)
     {
        // We open a dummy database instead.
        //kdDebug(7011) << "No database, opening a dummy one." << endl;
        TQBuffer *buffer = new TQBuffer();
        buffer->setBuffer(TQByteArray());
        buffer->open(IO_ReadWrite);
        m_str = new TQDataStream( buffer);
        (*m_str) << (TQ_INT32) TDESYCOCA_VERSION;
        (*m_str) << (TQ_INT32) 0;
     }
     else
     {
        result = false;
     }
   }
   m_lstFactories = new KSycocaFactoryList();
   m_lstFactories->setAutoDelete( true );
   d->database = database;
   return result;
}

// Read-write constructor - only for KBuildSycoca
KSycoca::KSycoca( bool /* dummy */ )
  : DCOPObject("tdesycoca_building"), m_lstFactories(0), m_str(0), m_barray(0), bNoDatabase(false),
    m_sycoca_size(0), m_sycoca_mmap(0)
{
   d = new KSycocaPrivate;
   m_lstFactories = new KSycocaFactoryList();
   m_lstFactories->setAutoDelete( true );
   _self = this;
}

static void delete_tdesycoca_self() {
  delete KSycoca::_self;
}

KSycoca * KSycoca::self()
{
    if (!_self) {
        tqAddPostRoutine(delete_tdesycoca_self);
        _self = new KSycoca();
    }
  return _self;
}

KSycoca::~KSycoca()
{
   closeDatabase();
   delete d;
   _self = 0L;
}

void KSycoca::closeDatabase()
{
   TQIODevice *device = 0;
   if (m_str)
      device = m_str->device();
#ifdef HAVE_MMAP
   if (device && m_sycoca_mmap)
   {
      TQBuffer *buf = static_cast<TQBuffer*>(device);
      buf->buffer().resetRawData(m_sycoca_mmap, m_sycoca_size);
      // Solaris has munmap(char*, size_t) and everything else should
      // be happy with a char* for munmap(void*, size_t)
      munmap((char*) m_sycoca_mmap, m_sycoca_size);
      m_sycoca_mmap = 0;
   }
#endif

   delete m_str;
   m_str = 0;
   delete device;
   if (d->database != device)
      delete d->database;
   if (m_barray) delete m_barray;
   m_barray = 0;
   device = 0;
   d->database = 0;
   // It is very important to delete all factories here
   // since they cache information about the database file
   delete m_lstFactories;
   m_lstFactories = 0L;
}

void KSycoca::addFactory( KSycocaFactory *factory )
{
   assert(m_lstFactories);
   m_lstFactories->append(factory);
}

bool KSycoca::isChanged(const char *type)
{
    return self()->d->changeList.contains(type);
}

void KSycoca::notifyDatabaseChanged(const TQStringList &changeList)
{
    d->changeList = changeList;
    //kdDebug(7011) << "got a notifyDatabaseChanged signal !" << endl;
    // kded tells us the database file changed
    // Close the database and forget all about what we knew
    // The next call to any public method will recreate
    // everything that's needed.
    closeDatabase();

    // Now notify applications
    emit databaseChanged();
}

TQDataStream * KSycoca::findEntry(int offset, KSycocaType &type)
{
   if ( !m_str )
      openDatabase();
   //kdDebug(7011) << TQString("KSycoca::_findEntry(offset=%1)").arg(offset,8,16) << endl;
   m_str->device()->at(offset);
   TQ_INT32 aType;
   (*m_str) >> aType;
   type = (KSycocaType) aType;
   //kdDebug(7011) << TQString("KSycoca::found type %1").arg(aType) << endl;
   return m_str;
}

bool KSycoca::checkVersion(bool abortOnError)
{
   if ( !m_str )
   {
      if( !openDatabase(false /* don't open dummy db if not found */) )
        return false; // No database found

      // We should never get here... if a database was found then m_str shouldn't be 0L.
      assert(m_str);
   }
   m_str->device()->at(0);
   TQ_INT32 aVersion;
   (*m_str) >> aVersion;
   if ( aVersion < TDESYCOCA_VERSION )
   {
      kdWarning(7011) << "Found version " << aVersion << ", expecting version " << TDESYCOCA_VERSION << " or higher." << endl;
      if (!abortOnError) return false;
      kdError(7011) << "Outdated database ! Stop kded and restart it !" << endl;
      abort();
   }
   return true;
}

TQDataStream * KSycoca::findFactory(KSycocaFactoryId id)
{
   // The constructor found no database, but we want one
   if (bNoDatabase)
   {
      closeDatabase(); // close the dummy one
      // Check if new database already available
      if ( !openDatabase(false /* no dummy one*/) )
      {
         static bool triedLaunchingKdeinit = false;
         if (!triedLaunchingKdeinit) // try only once
         {
           triedLaunchingKdeinit = true;
           kdDebug(7011) << "findFactory: we have no database.... launching tdeinit" << endl;
           TDEApplication::startKdeinit();
           // Ok, the new database should be here now, open it.
         }
         if (!openDatabase(false))
            return 0L; // Still no database - uh oh
      }
   }
   // rewind and check
   if (!checkVersion(false))
   {
     kdWarning(7011) << "Outdated database found" << endl;
     return 0L;
   }
   TQ_INT32 aId;
   TQ_INT32 aOffset;
   while(true)
   {
      (*m_str) >> aId;
      //kdDebug(7011) << TQString("KSycoca::findFactory : found factory %1").arg(aId) << endl;
      if (aId == 0)
      {
         kdError(7011) << "Error, KSycocaFactory (id = " << int(id) << ") not found!" << endl;
         break;
      }
      (*m_str) >> aOffset;
      if (aId == id)
      {
         //kdDebug(7011) << TQString("KSycoca::findFactory(%1) offset %2").arg((int)id).arg(aOffset) << endl;
         m_str->device()->at(aOffset);
         return m_str;
      }
   }
   return 0;
}

TQString KSycoca::kfsstnd_prefixes()
{
   if (bNoDatabase) return "";
   if (!checkVersion(false)) return "";
   TQ_INT32 aId;
   TQ_INT32 aOffset;
   // skip factories offsets
   while(true)
   {
      (*m_str) >> aId;
      if ( aId )
        (*m_str) >> aOffset;
      else
        break; // just read 0
   }
   // We now point to the header
   TQString prefixes;
   KSycocaEntry::read(*m_str, prefixes);
   (*m_str) >> m_timeStamp;
   KSycocaEntry::read(*m_str, d->language);
   (*m_str) >> d->updateSig;
   KSycocaEntry::read(*m_str, d->allResourceDirs);
   return prefixes;
}

TQ_UINT32 KSycoca::timeStamp()
{
   if (!m_timeStamp)
      (void) kfsstnd_prefixes();
   return m_timeStamp;
}

TQ_UINT32 KSycoca::updateSignature()
{
   if (!m_timeStamp)
      (void) kfsstnd_prefixes();
   return d->updateSig;
}

TQString KSycoca::language()
{
   if (d->language.isEmpty())
      (void) kfsstnd_prefixes();
   return d->language;
}

TQStringList KSycoca::allResourceDirs()
{
   if (!m_timeStamp)
      (void) kfsstnd_prefixes();
   return d->allResourceDirs;
}

TQString KSycoca::determineRelativePath( const TQString & _fullpath, const char *_resource )
{
  TQString sRelativeFilePath;
  TQStringList dirs = TDEGlobal::dirs()->resourceDirs( _resource );
  TQStringList::ConstIterator dirsit = dirs.begin();
  for ( ; dirsit != dirs.end() && sRelativeFilePath.isEmpty(); ++dirsit ) {
    // might need canonicalPath() ...
    if ( _fullpath.find( *dirsit ) == 0 ) // path is dirs + relativePath
      sRelativeFilePath = _fullpath.mid( (*dirsit).length() ); // skip appsdirs
  }
  if ( sRelativeFilePath.isEmpty() )
    kdFatal(7011) << TQString(TQString("Couldn't find %1 in any %2 dir !!!").arg( _fullpath ).arg( _resource)) << endl;
  //else
    // debug code
    //kdDebug(7011) << sRelativeFilePath << endl;
  return sRelativeFilePath;
}

KSycoca * KSycoca::_self = 0L;

void KSycoca::flagError()
{
   tqWarning("ERROR: KSycoca database corruption!");
   if (_self)
   {
      if (_self->d->readError)
         return;
      _self->d->readError = true;
      if (_self->d->autoRebuild)
         if(system("tdebuildsycoca") < 0) // Rebuild the damned thing.
	   tqWarning("ERROR: Running KSycoca failed.");
   }
}

void KSycoca::disableAutoRebuild()
{
   d->autoRebuild = false;
}

bool KSycoca::readError()
{
   bool b = false;
   if (_self)
   {
      b = _self->d->readError;
      _self->d->readError = false;
   }
   return b;
}

void KSycocaEntry::read( TQDataStream &s, TQString &str )
{
  TQ_UINT32 bytes;
  s >> bytes;                          // read size of string
  if ( bytes > 8192 ) {                // null string or too big
      if (bytes != 0xffffffff)
         KSycoca::flagError();
      str = TQString::null;
  } 
  else if ( bytes > 0 ) {              // not empty
      int bt = bytes/2;
      str.setLength( bt );
      TQChar* ch = (TQChar *) str.unicode();
      char t[8192];
      char *b = t;
      s.readRawBytes( b, bytes );
      while ( bt-- ) {
          *ch++ = (ushort) (((ushort)b[0])<<8) | (uchar)b[1];
	  b += 2;
      }
  } else {
      str = "";
  }
}

void KSycocaEntry::read( TQDataStream &s, TQStringList &list )
{
  list.clear();
  TQ_UINT32 count;
  s >> count;                          // read size of list
  if (count >= 1024)
  {
     KSycoca::flagError();
     return;
  }
  for(TQ_UINT32 i = 0; i < count; i++)
  {
     TQString str;
     read(s, str);
     list.append( str );
     if (s.atEnd())
     {
        KSycoca::flagError();
        return;
     }
  }
}

void KSycoca::virtual_hook( int id, void* data )
{ DCOPObject::virtual_hook( id, data ); }

void KSycocaEntry::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdesycoca.moc"
