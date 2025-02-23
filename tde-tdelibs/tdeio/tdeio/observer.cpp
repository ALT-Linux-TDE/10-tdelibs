/* This file is part of the KDE libraries
   Copyright (C) 2000 Matej Koss <koss@miesto.sk>
                      David Faure <faure@kde.org>

   $Id$

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

#include <assert.h>

#include <kdebug.h>
#include <tdeapplication.h>
#include <dcopclient.h>
#include <kurl.h>

#include "jobclasses.h"
#include "observer.h"

#include "uiserver_stub.h"

#include "passdlg.h"
#include "slavebase.h"
#include "observer_stub.h"
#include <tdemessagebox.h>
#include <ksslinfodlg.h>
#include <ksslcertdlg.h>
#include <ksslcertificate.h>
#include <ksslcertchain.h>
#include <tdelocale.h>

using namespace TDEIO;

template class TQIntDict<TDEIO::Job>;

Observer * Observer::s_pObserver = 0L;

const int KDEBUG_OBSERVER = 7007; // Should be 7028

Observer::Observer() : DCOPObject("TDEIO::Observer")
{
    // Register app as able to receive DCOP messages
    if (kapp && !kapp->dcopClient()->isAttached())
    {
        kapp->dcopClient()->attach();
    }

    if ( !kapp->dcopClient()->isApplicationRegistered( "tdeio_uiserver" ) )
    {
        kdDebug(KDEBUG_OBSERVER) << "Starting tdeio_uiserver" << endl;
        TQString error;
        int ret = TDEApplication::startServiceByDesktopPath( "tdeio_uiserver.desktop",
                                                             TQStringList(), &error );
        if ( ret > 0 )
        {
            kdError() << "Couldn't start tdeio_uiserver from tdeio_uiserver.desktop: " << error << endl;
        } else
            kdDebug(KDEBUG_OBSERVER) << "startServiceByDesktopPath returned " << ret << endl;

    }
    if ( !kapp->dcopClient()->isApplicationRegistered( "tdeio_uiserver" ) )
        kdDebug(KDEBUG_OBSERVER) << "The application tdeio_uiserver is STILL NOT REGISTERED" << endl;
    else
        kdDebug(KDEBUG_OBSERVER) << "tdeio_uiserver registered" << endl;

    m_uiserver = new UIServer_stub( "tdeio_uiserver", "UIServer" );
}

int Observer::newJob( TDEIO::Job * job, bool showProgress )
{
    // Tell the UI Server about this new job, and give it the application id
    // at the same time
    int progressId = m_uiserver->newJob( kapp->dcopClient()->appId(), showProgress );

    // Keep the result in a dict
    m_dctJobs.insert( progressId, job );

    return progressId;
}

void Observer::jobFinished( int progressId )
{
    m_uiserver->jobFinished( progressId );
    m_dctJobs.remove( progressId );
}

void Observer::killJob( int progressId )
{
    TDEIO::Job * job = m_dctJobs[ progressId ];
    if (!job)
    {
        kdWarning() << "Can't find job to kill ! There is no job with progressId=" << progressId << " in this process" << endl;
        return;
    }
    job->kill( false /* not quietly */ );
}

MetaData Observer::metadata( int progressId )
{
    TDEIO::Job * job = m_dctJobs[ progressId ];
    assert(job);
    return job->metaData();
}

void Observer::slotTotalSize( TDEIO::Job* job, TDEIO::filesize_t size )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalSize " << job << " " << TDEIO::number(size) << endl;
  m_uiserver->totalSize64( job->progressId(), size );
}

void Observer::slotTotalFiles( TDEIO::Job* job, unsigned long files )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalFiles " << job << " " << files << endl;
  m_uiserver->totalFiles( job->progressId(), files );
}

void Observer::slotTotalDirs( TDEIO::Job* job, unsigned long dirs )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotTotalDirs " << job << " " << dirs << endl;
  m_uiserver->totalDirs( job->progressId(), dirs );
}

void Observer::slotProcessedSize( TDEIO::Job* job, TDEIO::filesize_t size )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedSize " << job << " " << job->progressId() << " " << TDEIO::number(size) << endl;
  m_uiserver->processedSize64( job->progressId(), size );
}

void Observer::slotProcessedFiles( TDEIO::Job* job, unsigned long files )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedFiles " << job << " " << files << endl;
  m_uiserver->processedFiles( job->progressId(), files );
}

void Observer::slotProcessedDirs( TDEIO::Job* job, unsigned long dirs )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotProcessedDirs " << job << " " << dirs << endl;
  m_uiserver->processedDirs( job->progressId(), dirs );
}

void Observer::slotSpeed( TDEIO::Job* job, unsigned long speed )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotSpeed " << job << " " << speed << endl;
  m_uiserver->speed( job->progressId(), speed );
}

void Observer::slotPercent( TDEIO::Job* job, unsigned long percent )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotPercent " << job << " " << percent << endl;
  m_uiserver->percent( job->progressId(), percent );
}

void Observer::slotInfoMessage( TDEIO::Job* job, const TQString & msg )
{
  m_uiserver->infoMessage( job->progressId(), msg );
}

void Observer::slotCopying( TDEIO::Job* job, const KURL& from, const KURL& to )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotCopying " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->copying( job->progressId(), from, to );
}

void Observer::slotMoving( TDEIO::Job* job, const KURL& from, const KURL& to )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotMoving " << job << " " << from.url() << " " << to.url() << endl;
  m_uiserver->moving( job->progressId(), from, to );
}

void Observer::slotDeleting( TDEIO::Job* job, const KURL& url )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotDeleting " << job << " " << url.url() << endl;
  m_uiserver->deleting( job->progressId(), url );
}

void Observer::slotTransferring( TDEIO::Job* job, const KURL& url )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotTransferring " << job << " " << url.url() << endl;
  m_uiserver->transferring( job->progressId(), url );
}

void Observer::slotCreatingDir( TDEIO::Job* job, const KURL& dir )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotCreatingDir " << job << " " << dir.url() << endl;
  m_uiserver->creatingDir( job->progressId(), dir );
}

void Observer::slotCanResume( TDEIO::Job* job, TDEIO::filesize_t offset )
{
  //kdDebug(KDEBUG_OBSERVER) << "** Observer::slotCanResume " << job << " " << TDEIO::number(offset) << endl;
  m_uiserver->canResume64( job->progressId(), offset );
}

void Observer::stating( TDEIO::Job* job, const KURL& url )
{
  m_uiserver->stating( job->progressId(), url );
}

void Observer::mounting( TDEIO::Job* job, const TQString & dev, const TQString & point )
{
  m_uiserver->mounting( job->progressId(), dev, point );
}

void Observer::unmounting( TDEIO::Job* job, const TQString & point )
{
  m_uiserver->unmounting( job->progressId(), point );
}

bool Observer::openPassDlg( const TQString& prompt, TQString& user,
			    TQString& pass, bool readOnly )
{
   AuthInfo info;
   info.prompt = prompt;
   info.username = user;
   info.password = pass;
   info.readOnly = readOnly;
   bool result = openPassDlg ( info );
   if ( result )
   {
     user = info.username;
     pass = info.password;
   }
   return result;
}

bool Observer::openPassDlg( TDEIO::AuthInfo& info )
{
    kdDebug(KDEBUG_OBSERVER) << "Observer::openPassDlg: User= " << info.username
                             << ", Message= " << info.prompt << endl;
    int result = TDEIO::PasswordDialog::getNameAndPassword( info.username, info.password,
                                                          &info.keepPassword, info.prompt,
                                                          info.readOnly, info.caption,
                                                          info.comment, info.commentLabel );
    if ( result == TQDialog::Accepted )
    {
        info.setModified( true );
        return true;
    }
    return false;
}

int Observer::messageBox( int progressId, int type, const TQString &text,
                          const TQString &caption, const TQString &buttonYes,
                          const TQString &buttonNo )
{
    return messageBox( progressId, type, text, caption, buttonYes, buttonNo, TQString::null );
}

int Observer::messageBox( int progressId, int type, const TQString &text,
                          const TQString &caption, const TQString &buttonYes,
                          const TQString &buttonNo, const TQString &dontAskAgainName )
{
    kdDebug() << "Observer::messageBox " << type << " " << text << " - " << caption << endl;
    int result = -1;
    TDEConfig *config = new TDEConfig("tdeioslaverc");
    KMessageBox::setDontShowAskAgainConfig(config);

    switch (type) {
        case TDEIO::SlaveBase::QuestionYesNo:
            result = KMessageBox::questionYesNo( 0L, // parent ?
                                               text, caption, buttonYes, buttonNo, dontAskAgainName );
            break;
        case TDEIO::SlaveBase::WarningYesNo:
            result = KMessageBox::warningYesNo( 0L, // parent ?
                                              text, caption, buttonYes, buttonNo, dontAskAgainName );
            break;
        case TDEIO::SlaveBase::WarningContinueCancel:
            result = KMessageBox::warningContinueCancel( 0L, // parent ?
                                              text, caption, buttonYes, dontAskAgainName );
            break;
        case TDEIO::SlaveBase::WarningYesNoCancel:
            result = KMessageBox::warningYesNoCancel( 0L, // parent ?
                                              text, caption, buttonYes, buttonNo, dontAskAgainName );
            break;
        case TDEIO::SlaveBase::Information:
            KMessageBox::information( 0L, // parent ?
                                      text, caption, dontAskAgainName );
            result = 1; // whatever
            break;
        case TDEIO::SlaveBase::SSLMessageBox:
        {
            TQCString observerAppId = caption.utf8(); // hack, see slaveinterface.cpp
            // Contact the object "TDEIO::Observer" in the application <appId>
            // Yes, this could be the same application we are, but not necessarily.
            Observer_stub observer( observerAppId, "TDEIO::Observer" );

            TDEIO::MetaData meta = observer.metadata( progressId );
            KSSLInfoDlg *kid = new KSSLInfoDlg(meta["ssl_in_use"].upper()=="TRUE", 0L /*parent?*/, 0L, true);
            KSSLCertificate *x = KSSLCertificate::fromString(meta["ssl_peer_certificate"].local8Bit());
            if (x) {
               // Set the chain back onto the certificate
               TQStringList cl =
                      TQStringList::split(TQString("\n"), meta["ssl_peer_chain"]);
               TQPtrList<KSSLCertificate> ncl;

               ncl.setAutoDelete(true);
               for (TQStringList::Iterator it = cl.begin(); it != cl.end(); ++it) {
                  KSSLCertificate *y = KSSLCertificate::fromString((*it).local8Bit());
                  if (y) ncl.append(y);
               }

               if (ncl.count() > 0)
                  x->chain().setChain(ncl);

               kid->setup( x,
                           meta["ssl_peer_ip"],
                           text, // the URL
                           meta["ssl_cipher"],
                           meta["ssl_cipher_desc"],
                           meta["ssl_cipher_version"],
                           meta["ssl_cipher_used_bits"].toInt(),
                           meta["ssl_cipher_bits"].toInt(),
                           KSSLCertificate::KSSLValidation(meta["ssl_cert_state"].toInt()));
               kdDebug(7024) << "Showing SSL Info dialog" << endl;
               kid->exec();
               delete x;
               kdDebug(7024) << "SSL Info dialog closed" << endl;
            } else {
               KMessageBox::information( 0L, // parent ?
                                         i18n("The peer SSL certificate appears to be corrupt."), i18n("SSL") );
            }
            // This doesn't have to get deleted.  It deletes on it's own.
            result = 1; // whatever
            break;
        }
        default:
            kdWarning() << "Observer::messageBox: unknown type " << type << endl;
            result = 0;
            break;
    }
    KMessageBox::setDontShowAskAgainConfig(0);
    delete config;
    return result;
#if 0
    TQByteArray data, replyData;
    TQCString replyType;
    TQDataStream arg( data, IO_WriteOnly );
    arg << progressId;
    arg << type;
    arg << text;
    arg << caption;
    arg << buttonYes;
    arg << buttonNo;
    if ( kapp->dcopClient()->call( "tdeio_uiserver", "UIServer", "messageBox(int,int,TQString,TQString,TQString,TQString)", data, replyType, replyData, true )
        && replyType == "int" )
    {
        int result;
        TQDataStream _reply_stream( replyData, IO_ReadOnly );
        _reply_stream >> result;
        kdDebug(KDEBUG_OBSERVER) << "Observer::messageBox got result " << result << endl;
        return result;
    }
    kdDebug(KDEBUG_OBSERVER) << "Observer::messageBox call failed" << endl;
    return 0;
#endif
}

RenameDlg_Result Observer::open_RenameDlg( TDEIO::Job* job,
                                           const TQString & caption,
                                           const TQString& src, const TQString & dest,
                                           RenameDlg_Mode mode, TQString& newDest,
                                           TDEIO::filesize_t sizeSrc,
                                           TDEIO::filesize_t sizeDest,
                                           time_t ctimeSrc,
                                           time_t ctimeDest,
                                           time_t mtimeSrc,
                                           time_t mtimeDest
                                           )
{
  kdDebug(KDEBUG_OBSERVER) << "Observer::open_RenameDlg job=" << job << endl;
  if (job)
    kdDebug(KDEBUG_OBSERVER) << "                        progressId=" << job->progressId() << endl;
  // Hide existing dialog box if any
  if (job && job->progressId())
    m_uiserver->setJobVisible( job->progressId(), false );
  // We now do it in process => KDE4: move this code out of Observer (back to job.cpp), so that
  // opening the rename dialog doesn't start uiserver for nothing if progressId=0 (e.g. F2 in konq)
  RenameDlg_Result res = TDEIO::open_RenameDlg( caption, src, dest, mode,
                                               newDest, sizeSrc, sizeDest,
                                               ctimeSrc, ctimeDest, mtimeSrc,
                                               mtimeDest );
  if (job && job->progressId())
    m_uiserver->setJobVisible( job->progressId(), true );
  return res;
}

SkipDlg_Result Observer::open_SkipDlg( TDEIO::Job* job,
                                       bool _multi,
                                       const TQString& _error_text )
{
  kdDebug(KDEBUG_OBSERVER) << "Observer::open_SkipDlg job=" << job << " progressId=" << job->progressId() << endl;
  // Hide existing dialog box if any
  if (job && job->progressId())
      m_uiserver->setJobVisible( job->progressId(), false );
  // We now do it in process. So this method is a useless wrapper around TDEIO::open_RenameDlg.
  SkipDlg_Result res = TDEIO::open_SkipDlg( _multi, _error_text );
  if (job && job->progressId())
      m_uiserver->setJobVisible( job->progressId(), true );
  return res;
}

void Observer::virtual_hook( int id, void* data )
{ DCOPObject::virtual_hook( id, data ); }

#include "observer.moc"
