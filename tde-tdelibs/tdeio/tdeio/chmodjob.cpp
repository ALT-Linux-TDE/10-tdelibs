/* This file is part of the KDE libraries
    Copyright (C) 2000 Stephan Kulow <coolo@kde.org>
                       David Faure <faure@kde.org>
                       Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

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

#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#include <tqtimer.h>
#include <tqfile.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <tdemessagebox.h>

#include "tdeio/job.h"
#include "tdeio/chmodjob.h"

#include <kdirnotify_stub.h>

using namespace TDEIO;

ChmodJob::ChmodJob( const KFileItemList& lstItems, int permissions, int mask,
                    int newOwner, int newGroup,
                    bool recursive, bool showProgressInfo )
    : TDEIO::Job( showProgressInfo ), state( STATE_LISTING ),
      m_permissions( permissions ), m_mask( mask ),
      m_newOwner( newOwner ), m_newGroup( newGroup ),
      m_recursive( recursive ), m_lstItems( lstItems )
{
    TQTimer::singleShot( 0, this, TQ_SLOT(processList()) );
}

void ChmodJob::processList()
{
    while ( !m_lstItems.isEmpty() )
    {
        KFileItem * item = m_lstItems.first();
        if ( !item->isLink() ) // don't do anything with symlinks
        {
            // File or directory -> remember to chmod
            ChmodInfo info;
            info.url = item->url();
            // This is a toplevel file, we apply changes directly (no +X emulation here)
            info.permissions = ( m_permissions & m_mask ) | ( item->permissions() & ~m_mask );
            /*kdDebug(7007) << "\n current permissions=" << TQString::number(item->permissions(),8)
                          << "\n wanted permission=" << TQString::number(m_permissions,8)
                          << "\n with mask=" << TQString::number(m_mask,8)
                          << "\n with ~mask (mask bits we keep) =" << TQString::number((uint)~m_mask,8)
                          << "\n bits we keep =" << TQString::number(item->permissions() & ~m_mask,8)
                          << "\n new permissions = " << TQString::number(info.permissions,8)
                          << endl;*/
            m_infos.prepend( info );
            //kdDebug(7007) << "processList : Adding info for " << info.url.prettyURL() << endl;
            // Directory and recursive -> list
            if ( item->isDir() && m_recursive )
            {
                //kdDebug(7007) << "ChmodJob::processList dir -> listing" << endl;
                TDEIO::ListJob * listJob = TDEIO::listRecursive( item->url(), false /* no GUI */ );
                connect( listJob, TQ_SIGNAL(entries( TDEIO::Job *,
                                                  const TDEIO::UDSEntryList& )),
                         TQ_SLOT( slotEntries( TDEIO::Job*,
                                            const TDEIO::UDSEntryList& )));
                addSubjob( listJob );
                return; // we'll come back later, when this one's finished
            }
        }
        m_lstItems.removeFirst();
    }
    kdDebug(7007) << "ChmodJob::processList -> going to STATE_CHMODING" << endl;
    // We have finished, move on
    state = STATE_CHMODING;
    chmodNextFile();
}

void ChmodJob::slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList & list )
{
    TDEIO::UDSEntryListConstIterator it = list.begin();
    TDEIO::UDSEntryListConstIterator end = list.end();
    for (; it != end; ++it) {
        TDEIO::UDSEntry::ConstIterator it2 = (*it).begin();
        mode_t permissions = 0;
        bool isDir = false;
        bool isLink = false;
        TQString relativePath;
        for( ; it2 != (*it).end(); it2++ ) {
          switch( (*it2).m_uds ) {
            case TDEIO::UDS_NAME:
              relativePath = (*it2).m_str;
              break;
            case TDEIO::UDS_FILE_TYPE:
              isDir = S_ISDIR((*it2).m_long);
              break;
            case TDEIO::UDS_LINK_DEST:
              isLink = !(*it2).m_str.isEmpty();
              break;
            case TDEIO::UDS_ACCESS:
              permissions = (mode_t)((*it2).m_long);
              break;
            default:
              break;
          }
        }
        if ( !isLink && relativePath != TQString::fromLatin1("..") )
        {
            ChmodInfo info;
            info.url = m_lstItems.first()->url(); // base directory
            info.url.addPath( relativePath );
            int mask = m_mask;
            // Emulate -X: only give +x to files that had a +x bit already
            // So the check is the opposite : if the file had no x bit, don't touch x bits
            // For dirs this doesn't apply
            if ( !isDir )
            {
                int newPerms = m_permissions & mask;
                if ( (newPerms & 0111) && !(permissions & 0111) )
                {
                    // don't interfere with mandatory file locking
                    if ( newPerms & 02000 )
                      mask = mask & ~0101;
                    else
                      mask = mask & ~0111;
                }
            }
            info.permissions = ( m_permissions & mask ) | ( permissions & ~mask );
            /*kdDebug(7007) << "\n current permissions=" << TQString::number(permissions,8)
                          << "\n wanted permission=" << TQString::number(m_permissions,8)
                          << "\n with mask=" << TQString::number(mask,8)
                          << "\n with ~mask (mask bits we keep) =" << TQString::number((uint)~mask,8)
                          << "\n bits we keep =" << TQString::number(permissions & ~mask,8)
                          << "\n new permissions = " << TQString::number(info.permissions,8)
                          << endl;*/
            // Prepend this info in our todo list.
            // This way, the toplevel dirs are done last.
            m_infos.prepend( info );
        }
    }
}

void ChmodJob::chmodNextFile()
{
    if ( !m_infos.isEmpty() )
    {
        ChmodInfo info = m_infos.first();
        m_infos.remove( m_infos.begin() );
        // First update group / owner (if local file)
        // (permissions have to set after, in case of suid and sgid)
        if ( info.url.isLocalFile() && ( m_newOwner != -1 || m_newGroup != -1 ) )
        {
            TQString path = info.url.path();
            if ( chown( TQFile::encodeName(path), m_newOwner, m_newGroup ) != 0 )
            {
                int answer = KMessageBox::warningContinueCancel( 0, i18n( "<qt>Could not modify the ownership of file <b>%1</b>. You have insufficient access to the file to perform the change.</qt>" ).arg(path), TQString::null, i18n("&Skip File") );
                if (answer == KMessageBox::Cancel)
                {
                    m_error = ERR_USER_CANCELED;
                    emitResult();
                    return;
                }
            }
        }

        kdDebug(7007) << "ChmodJob::chmodNextFile chmod'ing " << info.url.prettyURL()
                      << " to " << TQString::number(info.permissions,8) << endl;
        TDEIO::SimpleJob * job = TDEIO::chmod( info.url, info.permissions );
        // copy the metadata for acl and default acl
        const TQString aclString = queryMetaData( "ACL_STRING" );
        const TQString defaultAclString = queryMetaData( "DEFAULT_ACL_STRING" );
        if ( !aclString.isEmpty() )
            job->addMetaData( "ACL_STRING", aclString );
        if ( !defaultAclString.isEmpty() )
            job->addMetaData( "DEFAULT_ACL_STRING", defaultAclString );
        addSubjob(job);
    }
    else
        // We have finished
        emitResult();
}

void ChmodJob::slotResult( TDEIO::Job * job )
{
    if ( job->error() )
    {
        m_error = job->error();
        m_errorText = job->errorText();
        emitResult();
        return;
    }
    //kdDebug(7007) << " ChmodJob::slotResult( TDEIO::Job * job ) m_lstItems:" << m_lstItems.count() << endl;
    switch ( state )
    {
        case STATE_LISTING:
            subjobs.remove(job);
            m_lstItems.removeFirst();
            kdDebug(7007) << "ChmodJob::slotResult -> processList" << endl;
            processList();
            return;
        case STATE_CHMODING:
            subjobs.remove(job);
            kdDebug(7007) << "ChmodJob::slotResult -> chmodNextFile" << endl;
            chmodNextFile();
            return;
        default:
            assert(0);
            return;
    }
}

// antlarr: KDE 4: Make owner and group be const TQString &
TDEIO_EXPORT ChmodJob *TDEIO::chmod( const KFileItemList& lstItems, int permissions, int mask,
                      TQString owner, TQString group,
                      bool recursive, bool showProgressInfo )
{
    uid_t newOwnerID = (uid_t)-1; // chown(2) : -1 means no change
    if ( !owner.isEmpty() )
    {
        struct passwd* pw = getpwnam(TQFile::encodeName(owner));
        if ( pw == 0L )
            kdError(250) << " ERROR: No user " << owner << endl;
        else
            newOwnerID = pw->pw_uid;
    }
    gid_t newGroupID = (gid_t)-1; // chown(2) : -1 means no change
    if ( !group.isEmpty() )
    {
        struct group* g = getgrnam(TQFile::encodeName(group));
        if ( g == 0L )
            kdError(250) << " ERROR: No group " << group << endl;
        else
            newGroupID = g->gr_gid;
    }
    return new ChmodJob( lstItems, permissions, mask, newOwnerID, newGroupID, recursive, showProgressInfo );
}

void ChmodJob::virtual_hook( int id, void* data )
{ TDEIO::Job::virtual_hook( id, data ); }

#include "chmodjob.moc"
