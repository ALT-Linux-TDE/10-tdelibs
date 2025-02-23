/* This file is part of the KDE project
 *
 * Copyright (C) 2002 David Faure <faure@kde.org>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "browserrun.h"
#include <tdemessagebox.h>
#include <tdefiledialog.h>
#include <tdeio/job.h>
#include <tdeio/scheduler.h>
#include <tdelocale.h>
#include <kprocess.h>
#include <kstringhandler.h>
#include <kuserprofile.h>
#include <tdetempfile.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <assert.h>

using namespace KParts;

class BrowserRun::BrowserRunPrivate
{
public:
  bool m_bHideErrorDialog;
  TQString contentDisposition;
};

BrowserRun::BrowserRun( const KURL& url, const KParts::URLArgs& args,
                        KParts::ReadOnlyPart *part, TQWidget* window,
                        bool removeReferrer, bool trustedSource )
    : KRun( url, window, 0 /*mode*/, false /*is_local_file known*/, false /* no GUI */ ),
      m_args( args ), m_part( part ), m_window( window ),
      m_bRemoveReferrer( removeReferrer ), m_bTrustedSource( trustedSource )
{
  d = new BrowserRunPrivate;
  d->m_bHideErrorDialog = false;
}

// BIC: merge with above ctor
BrowserRun::BrowserRun( const KURL& url, const KParts::URLArgs& args,
                        KParts::ReadOnlyPart *part, TQWidget* window,
                        bool removeReferrer, bool trustedSource, bool hideErrorDialog )
    : KRun( url, window, 0 /*mode*/, false /*is_local_file known*/, false /* no GUI */ ),
      m_args( args ), m_part( part ), m_window( window ),
      m_bRemoveReferrer( removeReferrer ), m_bTrustedSource( trustedSource )
{
  d = new BrowserRunPrivate;
  d->m_bHideErrorDialog = hideErrorDialog;
}

BrowserRun::~BrowserRun()
{
  delete d;
}

void BrowserRun::init()
{
  if ( d->m_bHideErrorDialog )
  {
    // ### KRun doesn't call a virtual method when it finds out that the URL
    // is either malformed, or points to a non-existing local file...
    // So we need to reimplement some of the checks, to handle m_bHideErrorDialog
    if ( !m_strURL.isValid() ) {
        redirectToError( TDEIO::ERR_MALFORMED_URL, m_strURL.url() );
        return;
    }
    if ( !m_bIsLocalFile && !m_bFault && m_strURL.isLocalFile() )
      m_bIsLocalFile = true;

    if ( m_bIsLocalFile )  {
      struct stat buff;
      if ( stat( TQFile::encodeName(m_strURL.path()), &buff ) == -1 )
      {
        kdDebug(1000) << "BrowserRun::init : " << m_strURL.prettyURL() << " doesn't exist." << endl;
        redirectToError( TDEIO::ERR_DOES_NOT_EXIST, m_strURL.path() );
        return;
      }
      m_mode = buff.st_mode; // while we're at it, save it for KRun::init() to use it
    }
  }
  KRun::init();
}

void BrowserRun::scanFile()
{
  kdDebug(1000) << "BrowserRun::scanfile " << m_strURL.prettyURL() << endl;

  // Let's check for well-known extensions
  // Not when there is a query in the URL, in any case.
  // Optimization for http/https, findByURL doesn't trust extensions over http.
  if ( m_strURL.query().isEmpty() && !m_strURL.protocol().startsWith("http") )
  {
    KMimeType::Ptr mime = KMimeType::findByURL( m_strURL );
    assert( mime != 0L );
    if ( mime->name() != "application/octet-stream" || m_bIsLocalFile )
    {
      kdDebug(1000) << "Scanfile: MIME TYPE is " << mime->name() << endl;
      foundMimeType( mime->name() );
      return;
    }
  }

  if ( m_part )
  {
      TQString proto = m_part->url().protocol().lower();

      if (proto == "https" || proto == "webdavs") {
          m_args.metaData().insert("main_frame_request", "TRUE" );
          m_args.metaData().insert("ssl_was_in_use", "TRUE" );
          m_args.metaData().insert("ssl_activate_warnings", "TRUE" );
      } else if (proto == "http" || proto == "webdav") {
          m_args.metaData().insert("ssl_activate_warnings", "TRUE" );
          m_args.metaData().insert("ssl_was_in_use", "FALSE" );
      }

      // Set the PropagateHttpHeader meta-data if it has not already been set...
      if (!m_args.metaData().contains("PropagateHttpHeader"))
          m_args.metaData().insert("PropagateHttpHeader", "TRUE");
  }

  TDEIO::TransferJob *job;
  if ( m_args.doPost() && m_strURL.protocol().startsWith("http"))
  {
      job = TDEIO::http_post( m_strURL, m_args.postData, false );
      job->addMetaData( "content-type", m_args.contentType() );
  }
  else
      job = TDEIO::get(m_strURL, m_args.reload, false);

  if ( m_bRemoveReferrer )
     m_args.metaData().remove("referrer");

  job->addMetaData( m_args.metaData() );
  job->setWindow( m_window );
  connect( job, TQ_SIGNAL( result( TDEIO::Job *)),
           this, TQ_SLOT( slotBrowserScanFinished(TDEIO::Job *)));
  connect( job, TQ_SIGNAL( mimetype( TDEIO::Job *, const TQString &)),
           this, TQ_SLOT( slotBrowserMimetype(TDEIO::Job *, const TQString &)));
  m_job = job;
}

void BrowserRun::slotBrowserScanFinished(TDEIO::Job *job)
{
  kdDebug(1000) << "BrowserRun::slotBrowserScanFinished" << endl;
  if ( job->error() == TDEIO::ERR_IS_DIRECTORY )
  {
      // It is in fact a directory. This happens when HTTP redirects to FTP.
      // Due to the "protocol doesn't support listing" code in BrowserRun, we
      // assumed it was a file.
      kdDebug(1000) << "It is in fact a directory!" << endl;
      // Update our URL in case of a redirection
      m_strURL = static_cast<TDEIO::TransferJob *>(job)->url();
      m_job = 0;
      foundMimeType( "inode/directory" );
  }
  else
  {
      if ( job->error() )
          handleError( job );
      else
          KRun::slotScanFinished(job);
  }
}

void BrowserRun::slotBrowserMimetype( TDEIO::Job *_job, const TQString &type )
{
  Q_ASSERT( _job == m_job );
  TDEIO::TransferJob *job = static_cast<TDEIO::TransferJob *>(m_job);
  // Update our URL in case of a redirection
  //kdDebug(1000) << "old URL=" << m_strURL.url() << endl;
  //kdDebug(1000) << "new URL=" << job->url().url() << endl;
  m_strURL = job->url();
  kdDebug(1000) << "slotBrowserMimetype: found " << type << " for " << m_strURL.prettyURL() << endl;

  m_suggestedFilename = job->queryMetaData("content-disposition-filename");
  d->contentDisposition = job->queryMetaData("content-disposition-type");
  //kdDebug(1000) << "m_suggestedFilename=" << m_suggestedFilename << endl;

  // Make a copy to avoid a dead reference
  TQString _type = type;
  job->putOnHold();
  m_job = 0;

  KRun::setSuggestedFileName(m_suggestedFilename);

  foundMimeType( _type );
}

BrowserRun::NonEmbeddableResult BrowserRun::handleNonEmbeddable( const TQString& _mimeType )
{
    TQString mimeType( _mimeType );
    Q_ASSERT( !m_bFinished ); // only come here if the mimetype couldn't be embedded
    // Support for saving remote files.
    if ( mimeType != "inode/directory" && // dirs can't be saved
         !m_strURL.isLocalFile() )
    {
        if ( isTextExecutable(mimeType) )
            mimeType = TQString::fromLatin1("text/plain"); // view, don't execute
        kdDebug(1000) << "BrowserRun: ask for saving" << endl;
        KService::Ptr offer = KServiceTypeProfile::preferredService(mimeType, "Application");
        // ... -> ask whether to save
        KParts::BrowserRun::AskSaveResult res = askSave( m_strURL, offer, mimeType, m_suggestedFilename );
        if ( res == KParts::BrowserRun::Save ) {
            save( m_strURL, m_suggestedFilename );
            kdDebug(1000) << "BrowserRun::handleNonEmbeddable: Save: returning Handled" << endl;
            m_bFinished = true;
            return Handled;
        }
        else if ( res == KParts::BrowserRun::Cancel ) {
            // saving done or canceled
            kdDebug(1000) << "BrowserRun::handleNonEmbeddable: Cancel: returning Handled" << endl;
            m_bFinished = true;
            return Handled;
        }
        else // "Open" chosen (done by KRun::foundMimeType, called when returning NotHandled)
        {
            // If we were in a POST, we can't just pass a URL to an external application.
            // We must save the data to a tempfile first.
            if ( m_args.doPost() )
            {
                kdDebug(1000) << "BrowserRun: request comes from a POST, can't pass a URL to another app, need to save" << endl;
                m_sMimeType = mimeType;
                TQString extension;
                TQString fileName = m_suggestedFilename.isEmpty() ? m_strURL.fileName() : m_suggestedFilename;
                int extensionPos = fileName.findRev( '.' );
                if ( extensionPos != -1 )
                    extension = fileName.mid( extensionPos ); // keep the '.'
                KTempFile tempFile( TQString::null, extension );
                KURL destURL;
                destURL.setPath( tempFile.name() );
                TDEIO::Job *job = TDEIO::file_copy( m_strURL, destURL, 0600, true /*overwrite*/, false /*no resume*/, true /*progress info*/ );
                job->setWindow (m_window);
                connect( job, TQ_SIGNAL( result( TDEIO::Job *)),
                         this, TQ_SLOT( slotCopyToTempFileResult(TDEIO::Job *)) );
                return Delayed; // We'll continue after the job has finished
            }
        }
    }

    // Check if running is allowed
    if ( !m_bTrustedSource && // ... and untrusted source...
         !allowExecution( mimeType, m_strURL ) ) // ...and the user said no (for executables etc.)
    {
        m_bFinished = true;
        return Handled;
    }

    TDEIO::SimpleJob::removeOnHold(); // Kill any slave that was put on hold.
    return NotHandled;
}

//static
bool BrowserRun::allowExecution( const TQString &serviceType, const KURL &url )
{
    if ( !isExecutable( serviceType ) )
      return true;

    if ( !url.isLocalFile() ) // Don't permit to execute remote files
        return false;

    return ( KMessageBox::warningContinueCancel( 0, i18n( "Do you really want to execute '%1'? " ).arg( url.prettyURL() ),
    i18n("Execute File?"), i18n("Execute") ) == KMessageBox::Continue );
}

static TQString makeQuestion( const KURL& url, const TQString& mimeType, const TQString& suggestedFilename )
{
    TQString surl = KStringHandler::csqueeze( url.prettyURL() );
    KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
    TQString comment = mimeType;

    // Test if the mimeType is not recognize as octet-stream.
    // If so then keep mime-type as comment
    if (mime->name() != KMimeType::defaultMimeType()) {
        // The mime-type is known so display the comment instead of mime-type
        comment = mime->comment();
    }
    // The strange order in the i18n() calls below is due to the possibility
    // of surl containing a '%'
    if ( suggestedFilename.isEmpty() )
        return i18n("Open '%2'?\nType: %1").arg(comment, surl);
    else
        return i18n("Open '%3'?\nName: %2\nType: %1").arg(comment, suggestedFilename, surl);
}

//static
BrowserRun::AskSaveResult BrowserRun::askSave( const KURL & url, KService::Ptr offer, const TQString& mimeType, const TQString & suggestedFilename )
{
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
    // NOTE: Keep this function in sync with tdebase/kcontrol/filetypes/filetypedetails.cpp
    //       FileTypeDetails::updateAskSave()

    TQString question = makeQuestion( url, mimeType, suggestedFilename );

    // Text used for the open button
    TQString openText = (offer && !offer->name().isEmpty())
                       ? i18n("&Open with '%1'").arg(offer->name())
                       : i18n("&Open With...");

    int choice = KMessageBox::questionYesNoCancel(
        0L, question, url.host(),
        KStdGuiItem::saveAs(), openText,
        TQString::fromLatin1("askSave")+ mimeType ); // dontAskAgainName, KEEP IN SYNC!!!

    return choice == KMessageBox::Yes ? Save : ( choice == KMessageBox::No ? Open : Cancel );
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
}

//static
BrowserRun::AskSaveResult BrowserRun::askEmbedOrSave( const KURL & url, const TQString& mimeType, const TQString & suggestedFilename, int flags )
{
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
    // NOTE: Keep this funcion in sync with tdebase/kcontrol/filetypes/filetypedetails.cpp
    //       FileTypeDetails::updateAskSave()

    KMimeType::Ptr mime = KMimeType::mimeType( mimeType );
    // Don't ask for:
    // - html (even new tabs would ask, due to about:blank!)
    // - dirs obviously (though not common over HTTP :),
    // - images (reasoning: no need to save, most of the time, because fast to see)
    // e.g. postscript is different, because takes longer to read, so
    // it's more likely that the user might want to save it.
    // - multipart/* ("server push", see tdemultipart)
    // - other strange 'internal' mimetypes like print/manager...
    // KEEP IN SYNC!!!
    if (flags != (int)AttachmentDisposition && (
         mime->is( "text/html" ) ||
         mime->is( "text/xml" ) ||
         mime->is( "inode/directory" ) ||
         mimeType.startsWith( "image" ) ||
         mime->is( "multipart/x-mixed-replace" ) ||
         mime->is( "multipart/replace" ) ||
         mimeType.startsWith( "print" ) ) )
        return Open;

    TQString question = makeQuestion( url, mimeType, suggestedFilename );

    int choice = KMessageBox::questionYesNoCancel(
        0L, question, url.host(),
        KStdGuiItem::saveAs(), KGuiItem( i18n( "&Open" ), "document-open"),
        TQString::fromLatin1("askEmbedOrSave")+ mimeType ); // dontAskAgainName, KEEP IN SYNC!!!
    return choice == KMessageBox::Yes ? Save : ( choice == KMessageBox::No ? Open : Cancel );
    // SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC SYNC
}

// Default implementation, overridden in TDEHTMLRun
void BrowserRun::save( const KURL & url, const TQString & suggestedFilename )
{
    simpleSave( url, suggestedFilename, m_window );
}

// static
void BrowserRun::simpleSave( const KURL & url, const TQString & suggestedFilename )
{
    simpleSave (url, suggestedFilename, 0);
}

void BrowserRun::simpleSave( const KURL & url, const TQString & suggestedFilename,
                             TQWidget* window )
{
    // DownloadManager <-> konqueror integration
    // find if the integration is enabled
    // the empty key  means no integration
    // only use the downloadmanager for non-local urls
    if ( !url.isLocalFile() )
    {
        TDEConfig cfg("konquerorrc", false, false);
        cfg.setGroup("HTML Settings");
        TQString downloadManger = cfg.readPathEntry("DownloadManager");
        if (!downloadManger.isEmpty())
        {
            // then find the download manager location
            kdDebug(1000) << "Using: "<<downloadManger <<" as Download Manager" <<endl;
            TQString cmd=TDEStandardDirs::findExe(downloadManger);
            if (cmd.isEmpty())
            {
                TQString errMsg=i18n("The Download Manager (%1) could not be found in your $PATH ").arg(downloadManger);
                TQString errMsgEx= i18n("Try to reinstall it  \n\nThe integration with Konqueror will be disabled!");
                KMessageBox::detailedSorry(0,errMsg,errMsgEx);
                cfg.writePathEntry("DownloadManager",TQString::null);
                cfg.sync ();
            }
            else
            {
                // ### suggestedFilename not taken into account. Fix this (and
                // the duplicated code) with shiny new KDownload class for 3.2 (pfeiffer)
                // Until the shiny new class comes about, send the suggestedFilename
                // along with the actual URL to download. (DA)
                cmd += " " + TDEProcess::quote(url.url());
                if ( !suggestedFilename.isEmpty() )
                    cmd +=" " + TDEProcess::quote(suggestedFilename);

                kdDebug(1000) << "Calling command  " << cmd << endl;
                // slave is already on hold (slotBrowserMimetype())
                TDEIO::Scheduler::publishSlaveOnHold();
                KRun::runCommand(cmd);
                return;
            }
        }
    }

    // no download manager available, let's do it ourself
    KFileDialog *dlg = new KFileDialog( TQString::null, TQString::null /*all files*/,
                                        window , "filedialog", true );
    dlg->setOperationMode( KFileDialog::Saving );
    dlg->setCaption(i18n("Save As"));

    dlg->setSelection( suggestedFilename.isEmpty() ? url.fileName() : suggestedFilename );
    if ( dlg->exec() )
    {
        KURL destURL( dlg->selectedURL() );
        if ( destURL.isValid() )
        {
            TDEIO::Job *job = TDEIO::copy( url, destURL );
            job->setWindow (window);
            job->setAutoErrorHandlingEnabled( true );
        }
    }
    delete dlg;
}

void BrowserRun::slotStatResult( TDEIO::Job *job )
{
    if ( job->error() ) {
        kdDebug(1000) << "BrowserRun::slotStatResult : " << job->errorString() << endl;
        handleError( job );
    } else
        KRun::slotStatResult( job );
}

void BrowserRun::handleError( TDEIO::Job * job )
{
    if ( !job ) { // Shouldn't happen, see docu.
        kdWarning(1000) << "BrowserRun::handleError called with job=0! hideErrorDialog=" << d->m_bHideErrorDialog << endl;
        return;
    }

    if (d->m_bHideErrorDialog && job->error() != TDEIO::ERR_NO_CONTENT)
    {
        redirectToError( job->error(), job->errorText() );
        return;
    }

    // Reuse code in KRun, to benefit from d->m_showingError etc.
    KRun::slotStatResult( job );
}

void BrowserRun::redirectToError( int error, const TQString& errorText )
{
    /**
     * To display this error in TDEHTMLPart instead of inside a dialog box,
     * we tell konq that the mimetype is text/html, and we redirect to
     * an error:/ URL that sends the info to tdehtml.
     *
     * The format of the error:/ URL is error:/?query#url,
     * where two variables are passed in the query:
     * error = int tdeio error code, errText = TQString error text from tdeio
     * The sub-url is the URL that we were trying to open.
     */
    KURL newURL(TQString("error:/?error=%1&errText=%2")
                .arg( error ).arg( KURL::encode_string(errorText) ), 106 );
    m_strURL.setPass( TQString::null ); // don't put the password in the error URL

    KURL::List lst;
    lst << newURL << m_strURL;
    m_strURL = KURL::join( lst );
    //kdDebug(1202) << "BrowserRun::handleError m_strURL=" << m_strURL.prettyURL() << endl;

    m_job = 0;
    foundMimeType( "text/html" );
}

void BrowserRun::slotCopyToTempFileResult(TDEIO::Job *job)
{
    if ( job->error() ) {
        job->showErrorDialog( m_window );
    } else {
        // Same as KRun::foundMimeType but with a different URL
        (void) (KRun::runURL( static_cast<TDEIO::FileCopyJob *>(job)->destURL(), m_sMimeType ));
    }
    m_bFault = true; // see above
    m_bFinished = true;
    m_timer.start( 0, true );
}

bool BrowserRun::isTextExecutable( const TQString &serviceType )
{
    return ( serviceType == "application/x-desktop" ||
             serviceType == "media/builtin-mydocuments" ||
             serviceType == "media/builtin-mycomputer" ||
             serviceType == "media/builtin-mynetworkplaces" ||
             serviceType == "media/builtin-printers" ||
             serviceType == "media/builtin-trash" ||
             serviceType == "media/builtin-webbrowser" ||
             serviceType == "application/x-shellscript" );
}

bool BrowserRun::isExecutable( const TQString &serviceType )
{
    return KRun::isExecutable( serviceType );
}

bool BrowserRun::hideErrorDialog() const
{
    return d->m_bHideErrorDialog;
}

TQString BrowserRun::contentDisposition() const {
    return d->contentDisposition;
}

#include "browserrun.moc"
