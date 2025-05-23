/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Leo Savernik <l.savernik@aon.at>

   Moved from ktar.cpp by Roberto Teixeira <maragato@kde.org>

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

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <grp.h>
#include <pwd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <tqptrlist.h>
#include <tqptrstack.h>
#include <tqvaluestack.h>
#include <tqmap.h>
#include <tqcstring.h>
#include <tqdir.h>
#include <tqfile.h>

#include <kdebug.h>
#include <kfilterdev.h>
#include <kfilterbase.h>
#include <kde_file.h>

#include "karchive.h"
#include "klimitediodevice.h"

template class TQDict<KArchiveEntry>;


class KArchive::KArchivePrivate
{
public:
    KArchiveDirectory* rootDir;
    bool closeSucceeded;
};

class PosSortedPtrList : public TQPtrList<KArchiveFile> {
protected:
    int compareItems( TQPtrCollection::Item i1,
                      TQPtrCollection::Item i2 )
    {
        int pos1 = static_cast<KArchiveFile*>( i1 )->position();
        int pos2 = static_cast<KArchiveFile*>( i2 )->position();
        return ( pos1 - pos2 );
    }
};


////////////////////////////////////////////////////////////////////////
/////////////////////////// KArchive ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

KArchive::KArchive( TQIODevice * dev )
{
    d = new KArchivePrivate;
    d->rootDir = 0;
    m_dev = dev;
    m_open = false;
}

KArchive::~KArchive()
{
    if ( m_open )
        close();
    delete d->rootDir;
    delete d;
}

bool KArchive::open( int mode )
{
    if ( m_dev && !m_dev->open( mode ) )
        return false;

    if ( m_open )
        close();

    m_mode = mode;
    m_open = true;

    Q_ASSERT( d->rootDir == 0L );
    d->rootDir = 0L;

    return openArchive( mode );
}

void KArchive::close()
{
    if ( !m_open )
        return;
    // moved by holger to allow kzip to write the zip central dir
    // to the file in closeArchive()
    d->closeSucceeded = closeArchive();

    if ( m_dev )
        m_dev->close();

    delete d->rootDir;
    d->rootDir = 0;
    m_open = false;
}

bool KArchive::closeSucceeded() const
{
    return d->closeSucceeded;
}

const KArchiveDirectory* KArchive::directory() const
{
    // rootDir isn't const so that parsing-on-demand is possible
    return const_cast<KArchive *>(this)->rootDir();
}


bool KArchive::addLocalFile( const TQString& fileName, const TQString& destName )
{
    TQFileInfo fileInfo( fileName );
    if ( !fileInfo.isFile() && !fileInfo.isSymLink() )
    {
        kdWarning() << "KArchive::addLocalFile " << fileName << " doesn't exist or is not a regular file." << endl;
        return false;
    }

    KDE_struct_stat fi;
    if (KDE_lstat(TQFile::encodeName(fileName),&fi) == -1) {
        kdWarning() << "KArchive::addLocalFile stating " << fileName
        	<< " failed: " << strerror(errno) << endl;
        return false;
    }

    if (fileInfo.isSymLink()) {
        return writeSymLink(destName, fileInfo.readLink(), fileInfo.owner(),
        		fileInfo.group(), fi.st_mode, fi.st_atime, fi.st_mtime,
          		fi.st_ctime);
    }/*end if*/

    uint size = fileInfo.size();

    // the file must be opened before prepareWriting is called, otherwise
    // if the opening fails, no content will follow the already written
    // header and the tar file is effectively f*cked up
    TQFile file( fileName );
    if ( !file.open( IO_ReadOnly ) )
    {
        kdWarning() << "KArchive::addLocalFile couldn't open file " << fileName << endl;
        return false;
    }

    if ( !prepareWriting( destName, fileInfo.owner(), fileInfo.group(), size,
    		fi.st_mode, fi.st_atime, fi.st_mtime, fi.st_ctime ) )
    {
        kdWarning() << "KArchive::addLocalFile prepareWriting " << destName << " failed" << endl;
        return false;
    }

    // Read and write data in chunks to minimize memory usage
    TQByteArray array(8*1024);
    int n;
    uint total = 0;
    while ( ( n = file.readBlock( array.data(), array.size() ) ) > 0 )
    {
        if ( !writeData( array.data(), n ) )
        {
            kdWarning() << "KArchive::addLocalFile writeData failed" << endl;
            return false;
        }
        total += n;
    }
    Q_ASSERT( total == size );

    if ( !doneWriting( size ) )
    {
        kdWarning() << "KArchive::addLocalFile doneWriting failed" << endl;
        return false;
    }
    return true;
}

bool KArchive::addLocalDirectory( const TQString& path, const TQString& destName )
{
    TQString dot = ".";
    TQString dotdot = "..";
    TQDir dir( path );
    if ( !dir.exists() )
        return false;
    dir.setFilter(dir.filter() | TQDir::Hidden);
    TQStringList files = dir.entryList();
    for ( TQStringList::Iterator it = files.begin(); it != files.end(); ++it )
    {
        if ( *it != dot && *it != dotdot )
        {
            TQString fileName = path + "/" + *it;
//            kdDebug() << "storing " << fileName << endl;
            TQString dest = destName.isEmpty() ? *it : (destName + "/" + *it);
            TQFileInfo fileInfo( fileName );

            if ( fileInfo.isFile() || fileInfo.isSymLink() )
                addLocalFile( fileName, dest );
            else if ( fileInfo.isDir() )
                addLocalDirectory( fileName, dest );
            // We omit sockets
        }
    }
    return true;
}

bool KArchive::writeFile( const TQString& name, const TQString& user, const TQString& group, uint size, const char* data )
{
    mode_t perm = 0100644;
    time_t the_time = time(0);
    return writeFile(name,user,group,size,perm,the_time,the_time,the_time,data);
}

bool KArchive::prepareWriting( const TQString& name, const TQString& user,
    			const TQString& group, uint size, mode_t perm,
       			time_t atime, time_t mtime, time_t ctime ) {
  PrepareWritingParams params;
  params.name = &name;
  params.user = &user;
  params.group = &group;
  params.size = size;
  params.perm = perm;
  params.atime = atime;
  params.mtime = mtime;
  params.ctime = ctime;
  virtual_hook(VIRTUAL_PREPARE_WRITING,&params);
  return params.retval;
}

bool KArchive::prepareWriting_impl(const TQString &name, const TQString &user,
    			const TQString &group, uint size, mode_t /*perm*/,
       			time_t /*atime*/, time_t /*mtime*/, time_t /*ctime*/ ) {
  kdWarning(7040) << "New prepareWriting API not implemented in this class." << endl
  		<< "Falling back to old API (metadata information will be lost)" << endl;
  return prepareWriting(name,user,group,size);
}

bool KArchive::writeFile( const TQString& name, const TQString& user,
    			const TQString& group, uint size, mode_t perm,
       			time_t atime, time_t mtime, time_t ctime,
       			const char* data ) {
  WriteFileParams params;
  params.name = &name;
  params.user = &user;
  params.group = &group;
  params.size = size;
  params.perm = perm;
  params.atime = atime;
  params.mtime = mtime;
  params.ctime = ctime;
  params.data = data;
  virtual_hook(VIRTUAL_WRITE_FILE,&params);
  return params.retval;
}

bool KArchive::writeFile_impl( const TQString& name, const TQString& user,
    			const TQString& group, uint size, mode_t perm,
       			time_t atime, time_t mtime, time_t ctime,
       			const char* data ) {

    if ( !prepareWriting( name, user, group, size, perm, atime, mtime, ctime ) )
    {
        kdWarning() << "KArchive::writeFile prepareWriting failed" << endl;
        return false;
    }

    // Write data
    // Note: if data is 0L, don't call writeBlock, it would terminate the KFilterDev
    if ( data && size && !writeData( data, size ) )
    {
        kdWarning() << "KArchive::writeFile writeData failed" << endl;
        return false;
    }

    if ( !doneWriting( size ) )
    {
        kdWarning() << "KArchive::writeFile doneWriting failed" << endl;
        return false;
    }
    return true;
}

bool KArchive::writeDir(const TQString& name, const TQString& user,
    			const TQString& group, mode_t perm,
       			time_t atime, time_t mtime, time_t ctime) {
  WriteDirParams params;
  params.name = &name;
  params.user = &user;
  params.group = &group;
  params.perm = perm;
  params.atime = atime;
  params.mtime = mtime;
  params.ctime = ctime;
  virtual_hook(VIRTUAL_WRITE_DIR,&params);
  return params.retval;
}

bool KArchive::writeDir_impl(const TQString &name, const TQString &user,
    			const TQString &group, mode_t /*perm*/,
       			time_t /*atime*/, time_t /*mtime*/, time_t /*ctime*/ ) {
  kdWarning(7040) << "New writeDir API not implemented in this class." << endl
  		<< "Falling back to old API (metadata information will be lost)" << endl;
  return writeDir(name,user,group);
}

bool KArchive::writeSymLink(const TQString &name, const TQString &target,
    			const TQString &user, const TQString &group,
    			mode_t perm, time_t atime, time_t mtime, time_t ctime) {
  WriteSymlinkParams params;
  params.name = &name;
  params.target = &target;
  params.user = &user;
  params.group = &group;
  params.perm = perm;
  params.atime = atime;
  params.mtime = mtime;
  params.ctime = ctime;
  virtual_hook(VIRTUAL_WRITE_SYMLINK,&params);
  return params.retval;
}

bool KArchive::writeSymLink_impl(const TQString &/*name*/,const TQString &/*target*/,
    			const TQString &/*user*/, const TQString &/*group*/,
    			mode_t /*perm*/, time_t /*atime*/, time_t /*mtime*/,
    			time_t /*ctime*/) {
  kdWarning(7040) << "writeSymLink not implemented in this class." << endl
  		<< "No fallback available." << endl;
  // FIXME: better return true here for compatibility with KDE < 3.2
  return false;
}

bool KArchive::writeData( const char* data, uint size )
{
    WriteDataParams params;
    params.data = data;
    params.size = size;
    virtual_hook( VIRTUAL_WRITE_DATA, &params );
    return params.retval;
}

bool KArchive::writeData_impl( const char* data, uint size )
{
    Q_ASSERT( device() );
    return device()->writeBlock( data, size ) == (TQ_LONG)size;
}

KArchiveDirectory * KArchive::rootDir()
{
    if ( !d->rootDir )
    {
        //kdDebug() << "Making root dir " << endl;
        struct passwd* pw =  getpwuid( getuid() );
        struct group* grp = getgrgid( getgid() );
        TQString username = pw ? TQFile::decodeName(pw->pw_name) : TQString::number( getuid() );
        TQString groupname = grp ? TQFile::decodeName(grp->gr_name) : TQString::number( getgid() );

        d->rootDir = new KArchiveDirectory( this, TQString::fromLatin1("/"), (int)(0777 + S_IFDIR), 0, username, groupname, TQString::null );
    }
    return d->rootDir;
}

KArchiveDirectory * KArchive::findOrCreate( const TQString & path )
{
    //kdDebug() << "KArchive::findOrCreate " << path << endl;
    if ( path.isEmpty() || path == "/" || path == "." ) // root dir => found
    {
        //kdDebug() << "KArchive::findOrCreate returning rootdir" << endl;
        return rootDir();
    }
    // Important note : for tar files containing absolute paths
    // (i.e. beginning with "/"), this means the leading "/" will
    // be removed (no KDirectory for it), which is exactly the way
    // the "tar" program works (though it displays a warning about it)
    // See also KArchiveDirectory::entry().

    // Already created ? => found
    KArchiveEntry* ent = rootDir()->entry( path );
    if ( ent )
    {
        if ( ent->isDirectory() )
            //kdDebug() << "KArchive::findOrCreate found it" << endl;
            return (KArchiveDirectory *) ent;
        else
            kdWarning() << "Found " << path << " but it's not a directory" << endl;
    }

    // Otherwise go up and try again
    int pos = path.findRev( '/' );
    KArchiveDirectory * parent;
    TQString dirname;
    if ( pos == -1 ) // no more slash => create in root dir
    {
        parent =  rootDir();
        dirname = path;
    }
    else
    {
        TQString left = path.left( pos );
        dirname = path.mid( pos + 1 );
        parent = findOrCreate( left ); // recursive call... until we find an existing dir.
    }

    //kdDebug() << "KTar : found parent " << parent->name() << " adding " << dirname << " to ensure " << path << endl;
    // Found -> add the missing piece
    KArchiveDirectory * e = new KArchiveDirectory( this, dirname, d->rootDir->permissions(),
                                                   d->rootDir->date(), d->rootDir->user(),
                                                   d->rootDir->group(), TQString::null );
    parent->addEntry( e );
    return e; // now a directory to <path> exists
}

void KArchive::setDevice( TQIODevice * dev )
{
    m_dev = dev;
}

void KArchive::setRootDir( KArchiveDirectory *rootDir )
{
    Q_ASSERT( !d->rootDir ); // Call setRootDir only once during parsing please ;)
    d->rootDir = rootDir;
}

////////////////////////////////////////////////////////////////////////
/////////////////////// KArchiveEntry //////////////////////////////////
////////////////////////////////////////////////////////////////////////
KArchiveEntry::KArchiveEntry( KArchive* t, const TQString& name, int access, int date,
                      const TQString& user, const TQString& group, const
                      TQString& symlink)
{
  m_name = name;
  m_access = access;
  m_date = date;
  m_user = user;
  m_group = group;
  m_symlink = symlink;
  m_archive = t;

}

TQDateTime KArchiveEntry::datetime() const
{
  TQDateTime d;
  d.setTime_t( m_date );
  return d;
}

////////////////////////////////////////////////////////////////////////
/////////////////////// KArchiveFile ///////////////////////////////////
////////////////////////////////////////////////////////////////////////

KArchiveFile::KArchiveFile( KArchive* t, const TQString& name, int access, int date,
                    const TQString& user, const TQString& group,
                    const TQString & symlink,
                    int pos, int size )
  : KArchiveEntry( t, name, access, date, user, group, symlink )
{
  m_pos = pos;
  m_size = size;
}

int KArchiveFile::position() const
{
  return m_pos;
}

int KArchiveFile::size() const
{
  return m_size;
}

TQByteArray KArchiveFile::data() const
{
  archive()->device()->at( m_pos );

  // Read content
  TQByteArray arr( m_size );
  if ( m_size )
  {
    assert( arr.data() );
    int n = archive()->device()->readBlock( arr.data(), m_size );
    if ( n != m_size )
      arr.resize( n );
  }
  return arr;
}

// ** This should be a virtual method, and this code should be in ktar.cpp
TQIODevice *KArchiveFile::device() const
{
    return new KLimitedIODevice( archive()->device(), m_pos, m_size );
}

void KArchiveFile::copyTo(const TQString& dest) const
{
  TQFile f( dest + "/"  + name() );
  f.open( IO_ReadWrite | IO_Truncate );
  f.writeBlock( data() );
  f.close();
}

////////////////////////////////////////////////////////////////////////
//////////////////////// KArchiveDirectory /////////////////////////////////
////////////////////////////////////////////////////////////////////////


KArchiveDirectory::KArchiveDirectory( KArchive* t, const TQString& name, int access,
                              int date,
                              const TQString& user, const TQString& group,
                              const TQString &symlink)
  : KArchiveEntry( t, name, access, date, user, group, symlink )
{
  m_entries.setAutoDelete( true );
}

TQStringList KArchiveDirectory::entries() const
{
  TQStringList l;

  TQDictIterator<KArchiveEntry> it( m_entries );
  for( ; it.current(); ++it )
    l.append( it.currentKey() );

  return l;
}

KArchiveEntry* KArchiveDirectory::entry( TQString name )
  // not "const TQString & name" since we want a local copy
  // (to remove leading slash if any)
{
  int pos = name.find( '/' );
  if ( pos == 0 ) // ouch absolute path (see also KArchive::findOrCreate)
  {
    if (name.length()>1)
    {
      name = name.mid( 1 ); // remove leading slash
      pos = name.find( '/' ); // look again
    }
    else // "/"
      return this;
  }
  // trailing slash ? -> remove
  if ( pos != -1 && pos == (int)name.length()-1 )
  {
    name = name.left( pos );
    pos = name.find( '/' ); // look again
  }
  if ( pos != -1 )
  {
    TQString left = name.left( pos );
    TQString right = name.mid( pos + 1 );

    //kdDebug() << "KArchiveDirectory::entry left=" << left << " right=" << right << endl;

    KArchiveEntry* e = m_entries[ left ];
    if ( !e || !e->isDirectory() )
      return 0;
    return ((KArchiveDirectory*)e)->entry( right );
  }

  return m_entries[ name ];
}

const KArchiveEntry* KArchiveDirectory::entry( TQString name ) const
{
  return ((KArchiveDirectory*)this)->entry( name );
}

void KArchiveDirectory::addEntry( KArchiveEntry* entry )
{
  if( entry->name().isEmpty() ) {
    return;
  }
  if( m_entries[ entry->name() ] ) {
      kdWarning() << "KArchiveDirectory::addEntry: directory " << name()
                  << " has entry " << entry->name() << " already" << endl;
  }
  m_entries.insert( entry->name(), entry );
}

void KArchiveDirectory::copyTo(const TQString& dest, bool recursiveCopy ) const
{
  TQDir root;
  const TQString destDir(TQDir(dest).absPath()); // get directory path without any "." or ".."

  PosSortedPtrList fileList;
  TQMap<int, TQString> fileToDir;

  TQStringList::Iterator it;

  // placeholders for iterated items
  KArchiveDirectory* curDir;
  TQString curDirName;

  TQStringList dirEntries;
  KArchiveEntry* curEntry;
  KArchiveFile* curFile;


  TQPtrStack<KArchiveDirectory> dirStack;
  TQValueStack<TQString> dirNameStack;

  dirStack.push( this );     // init stack at current directory
  dirNameStack.push( destDir ); // ... with given path
  do {
    curDir = dirStack.pop();

    // extract only to specified folder if it is located within archive's extraction folder
    // otherwise put file under root position in extraction folder
    TQString curDirName = dirNameStack.pop();
    if (!TQDir(curDirName).absPath().startsWith(destDir)) {
        kdWarning() << "Attempted export into folder" << curDirName
            << "which is outside of the extraction root folder" << destDir << "."
            << "Changing export of contained files to extraction root folder.";
        curDirName = destDir;
    }
    root.mkdir(curDirName);

    dirEntries = curDir->entries();
    for ( it = dirEntries.begin(); it != dirEntries.end(); ++it ) {
      curEntry = curDir->entry(*it);
      if (!curEntry->symlink().isEmpty()) {
          const TQString linkName = curDirName+'/'+curEntry->name();
          kdDebug() << "symlink(" << curEntry->symlink() << ',' << linkName << ')';
#ifdef Q_OS_UNIX
          if (!::symlink(curEntry->symlink().local8Bit(), linkName.local8Bit())) {
              kdDebug() << "symlink(" << curEntry->symlink() << ',' << linkName << ") failed:" << strerror(errno);
          }
#endif
      } else {
          if ( curEntry->isFile() ) {
              curFile = dynamic_cast<KArchiveFile*>( curEntry );
              if (curFile) {
                  fileList.append( curFile );
                  fileToDir.insert( curFile->position(), curDirName );
              }
          }

          if ( curEntry->isDirectory() )
              if ( recursiveCopy ) {
                  KArchiveDirectory *ad = dynamic_cast<KArchiveDirectory*>( curEntry );
                  if (ad) {
                      dirStack.push( ad );
                      dirNameStack.push( curDirName + "/" + curEntry->name() );
                  }
              }
      }
    }
  } while (!dirStack.isEmpty());

  fileList.sort();  // sort on m_pos, so we have a linear access

  KArchiveFile* f;
  for ( f = fileList.first(); f; f = fileList.next() ) {
    int pos = f->position();
    f->copyTo( fileToDir[pos] );
  }
}

void KArchive::virtual_hook( int id, void* data )
{
    switch (id) {
      case VIRTUAL_WRITE_DATA: {
        WriteDataParams* params = reinterpret_cast<WriteDataParams *>(data);
        params->retval = writeData_impl( params->data, params->size );
        break;
      }
      case VIRTUAL_WRITE_SYMLINK: {
        WriteSymlinkParams *params = reinterpret_cast<WriteSymlinkParams *>(data);
        params->retval = writeSymLink_impl(*params->name,*params->target,
        		*params->user,*params->group,params->perm,
          		params->atime,params->mtime,params->ctime);
        break;
      }
      case VIRTUAL_WRITE_DIR: {
        WriteDirParams *params = reinterpret_cast<WriteDirParams *>(data);
        params->retval = writeDir_impl(*params->name,*params->user,
        		*params->group,params->perm,
          		params->atime,params->mtime,params->ctime);
        break;
      }
      case VIRTUAL_WRITE_FILE: {
        WriteFileParams *params = reinterpret_cast<WriteFileParams *>(data);
        params->retval = writeFile_impl(*params->name,*params->user,
        		*params->group,params->size,params->perm,
          		params->atime,params->mtime,params->ctime,
            		params->data);
        break;
      }
      case VIRTUAL_PREPARE_WRITING: {
        PrepareWritingParams *params = reinterpret_cast<PrepareWritingParams *>(data);
        params->retval = prepareWriting_impl(*params->name,*params->user,
        		*params->group,params->size,params->perm,
          		params->atime,params->mtime,params->ctime);
        break;
      }
      default:
        /*BASE::virtual_hook( id, data )*/;
    }/*end switch*/
}

void KArchiveEntry::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KArchiveFile::virtual_hook( int id, void* data )
{ KArchiveEntry::virtual_hook( id, data ); }

void KArchiveDirectory::virtual_hook( int id, void* data )
{ KArchiveEntry::virtual_hook( id, data ); }
