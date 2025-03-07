/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <david@mandrakesoft.com>
   Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#include "tdefileshare.h"
#include <tqdir.h>
#include <tqfile.h>
#include <tqregexp.h>
#include <kprocess.h>
#include <kprocio.h>
#include <tdelocale.h>
#include <kstaticdeleter.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <kdirwatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <kdirnotify_stub.h>
#include <ksimpleconfig.h>
#include <kuser.h>

KFileShare::Authorization KFileShare::s_authorization = NotInitialized;
//TQStringList* KFileShare::s_shareList = 0L;
//static KStaticDeleter<TQStringList> sdShareList;
TQMap<TQString,TQString>* KFileShare::s_shareMap = 0L;
static KStaticDeleter<TQMap<TQString,TQString> > sdShareMap;

KFileShare::ShareMode KFileShare::s_shareMode;
bool KFileShare::s_sambaEnabled;
bool KFileShare::s_nfsEnabled;
bool KFileShare::s_restricted;
TQString KFileShare::s_fileShareGroup;
bool KFileShare::s_sharingEnabled;


#define FILESHARECONF "/etc/security/fileshare.conf"

KFileSharePrivate::KFileSharePrivate()
{
  KDirWatch::self()->addFile(FILESHARECONF);
  connect(KDirWatch::self(), TQ_SIGNAL(dirty (const TQString&)),this,
          TQ_SLOT(slotFileChange(const TQString &)));
  connect(KDirWatch::self(), TQ_SIGNAL(created(const TQString&)),this,
          TQ_SLOT(slotFileChange(const TQString &)));
  connect(KDirWatch::self(), TQ_SIGNAL(deleted(const TQString&)),this,
          TQ_SLOT(slotFileChange(const TQString &)));
}

KFileSharePrivate::~KFileSharePrivate()
{
  KDirWatch::self()->removeFile(FILESHARECONF);
}

KFileSharePrivate *KFileSharePrivate::_self=0L;

static KStaticDeleter<KFileSharePrivate> kstFileShare;

KFileSharePrivate* KFileSharePrivate::self()
{
   if (!_self)
      _self = kstFileShare.setObject(_self, new KFileSharePrivate());
   return _self;
}

void KFileSharePrivate::slotFileChange(const TQString &file)
{
  if(file==FILESHARECONF) {
     KFileShare::readConfig();
     KFileShare::readShareList();
  }
}

void KFileShare::readConfig() // static
{    
    // Create KFileSharePrivate instance
    KFileSharePrivate::self();
    KSimpleConfig config(TQString::fromLatin1(FILESHARECONF),true);
    
    s_sharingEnabled = config.readEntry("FILESHARING", "yes") == "yes";
    s_restricted = config.readEntry("RESTRICT", "yes") == "yes";
    s_fileShareGroup = config.readEntry("FILESHAREGROUP", "fileshare");
    
    
    if (!s_sharingEnabled) 
        s_authorization = UserNotAllowed;
    else 
    if (!s_restricted )
        s_authorization = Authorized;
    else {
        // check if current user is in fileshare group
        KUserGroup shareGroup(s_fileShareGroup);
        if (shareGroup.users().findIndex(KUser()) > -1 ) 
            s_authorization = Authorized;
        else
            s_authorization = UserNotAllowed;
    }
                
    if (config.readEntry("SHARINGMODE", "simple") == "simple") 
        s_shareMode = Simple;
    else        
        s_shareMode = Advanced;
          
        
    s_sambaEnabled = config.readEntry("SAMBA", "yes") == "yes";
    s_nfsEnabled = config.readEntry("NFS", "yes") == "yes";
}

KFileShare::ShareMode KFileShare::shareMode() {
  if ( s_authorization == NotInitialized )
      readConfig();
  
  return s_shareMode;
}

bool KFileShare::sharingEnabled() {
  if ( s_authorization == NotInitialized )
      readConfig();
  
  return s_sharingEnabled;
}
   
bool KFileShare::isRestricted() {
  if ( s_authorization == NotInitialized )
      readConfig();
  
  return s_restricted;
}
    
TQString KFileShare::fileShareGroup() {
  if ( s_authorization == NotInitialized )
      readConfig();
  
  return s_fileShareGroup;
}

    
bool KFileShare::sambaEnabled() {
  if ( s_authorization == NotInitialized )
      readConfig();
  
  return s_sambaEnabled;
}
    
bool KFileShare::nfsEnabled() {
  if ( s_authorization == NotInitialized )
      readConfig();
  
  return s_nfsEnabled;
}


void KFileShare::readShareList() 
{
    KFileSharePrivate::self();
    if ( !s_shareMap )
	sdShareMap.setObject( s_shareMap, new TQMap<TQString,TQString> );
    else
	s_shareMap->clear();

    // /usr/sbin on Mandrake, $PATH allows flexibility for other distributions
    TQString exe = findExe( "filesharelist" );
    if (exe.isEmpty()) {
        s_authorization = ErrorNotFound;
        return;
    }
    KProcIO proc;
    proc << exe;
    if ( !proc.start( TDEProcess::Block ) ) {
        kdError() << "Can't run " << exe << endl;
        s_authorization = ErrorNotFound;
        return;
    }

    // Reading code shamelessly stolen from khostname.cpp ;)
    TQString line;
    TQString options;
    TQString path;
    int length;
    TQRegExp rx_line("([^\\s]+)\\s+(.*)");
    do {
        length = proc.readln(line, true);
	if ( length > 0 )
	{
            if ( line[length-1] != '/' )
                line += '/';
            if( rx_line.search( line ) != -1 ) {
                options = rx_line.cap(1);
                path    = rx_line.cap(2);
                (*s_shareMap)[path] = options;
            }
            kdDebug(7000) << "Shared dir:" << line << endl;
        }
    } while (length > -1);
}


int KFileShare::isDirectoryShared( const TQString& _path )
{
    int ret(0);

    if ( ! s_shareMap )
        readShareList();

    TQString path( _path );
    if ( path[path.length()-1] != '/' )
        path += '/';
    //return s_shareList && s_shareList->contains( path );
    if( (*s_shareMap).contains(path) && !((*s_shareMap)[path].isEmpty()) ) {
        ret+=1;
        if( (*s_shareMap)[path].find("readwrite") != -1 )
            ret+=2;
    }
    
    return ret;
}

KFileShare::Authorization KFileShare::authorization()
{
    // The app should do this on startup, but if it doesn't, let's do here.
    if ( s_authorization == NotInitialized )
        readConfig();
    return s_authorization;
}

TQString KFileShare::findExe( const char* exeName )
{
   // /usr/sbin on Mandrake, $PATH allows flexibility for other distributions
   TQString path = TQString::fromLocal8Bit(getenv("PATH")) + TQString::fromLatin1(":/usr/sbin");
   TQString exe = TDEStandardDirs::findExe( exeName, path );
   if (exe.isEmpty())
       kdError() << exeName << " not found in " << path << endl;
   return exe;
}

bool KFileShare::setShared( const TQString& path, bool shared )
{
   return SuSEsetShared( path, shared, false );
}

bool KFileShare::SuSEsetShared( const TQString& path, bool shared, bool rw )
{
    if (! KFileShare::sharingEnabled() ||
          KFileShare::shareMode() == Advanced)
       return false;

    TQString exe = KFileShare::findExe( "fileshareset" );
    if (exe.isEmpty())
        return false;

    // we want to share, so we kick it first - just to be sure
    TDEProcess proc;
    proc << exe;
    proc << "--remove";
    proc << path;
    proc.start( TDEProcess::Block );
    proc.clearArguments();
        
    proc << exe;
     if( rw )
         proc << "--rw";
    if ( shared )
        proc << "--add";
    else
        proc << "--remove";
    proc << path;
    proc.start( TDEProcess::Block ); // should be ok, the perl script terminates fast
    bool ok = proc.normalExit() && (proc.exitStatus() == 0);
    kdDebug(7000) << "KFileSharePropsPlugin::setShared normalExit=" 
                  << proc.normalExit() << endl;
    kdDebug(7000) << "KFileSharePropsPlugin::setShared exitStatus=" 
                  << proc.exitStatus() << endl;
    if ( proc.normalExit() ) {
      switch( proc.exitStatus() ) {
        case 1: 
          // User is not authorized
          break;
        case 3:
          // Called script with --add, but path was already shared before.
          // Result is nevertheless what the client wanted, so
          // this is alright.
          ok = true;
          break;
        case 4:
          // Invalid mount point
          break;
        case 5: 
          // Called script with --remove, but path was not shared before.
          // Result is nevertheless what the client wanted, so
          // this is alright.
          ok = true; 
          break;
        case 6:
          // There is no export method
          break;                    
        case 7:
          // file sharing is disabled
          break;            
        case 8:
          // advanced sharing is enabled
          break;          
        case 255:
          // Abitrary error
          break;                
      }
    } 
    
    return ok;
}

bool KFileShare::sambaActive()
{
    // rcsmb is not executable by users, try ourselves
    int status = system( "/sbin/checkproc -p /var/run/samba/smbd.pid /usr/sbin/smbd" );
    return status != -1 && WIFEXITED( status ) && WEXITSTATUS( status ) == 0;
}

bool KFileShare::nfsActive()
{
    // rcnfsserver is not executable by users, try ourselves
    int status = system( "/sbin/checkproc /usr/sbin/rpc.mountd" );
    if( status != -1 && WIFEXITED( status ) && WEXITSTATUS( status ) == 0 )
    {
        status = system( "/sbin/checkproc -n nfsd" );
        if( status != -1 && WIFEXITED( status ) && WEXITSTATUS( status ) == 0 )
            return true;
    }
    return false;
}

#include "tdefileshare.moc"
