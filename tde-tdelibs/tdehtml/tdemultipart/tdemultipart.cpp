/* This file is part of the KDE project
   Copyright (C) 2002 David Faure <david@mandrakesoft.com>

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

#include "tdemultipart.h"

#include <tqvbox.h>
#include <kinstance.h>
#include <kmimetype.h>
#include <tdelocale.h>
#include <tdeio/job.h>
#include <tqfile.h>
#include <tdetempfile.h>
#include <tdemessagebox.h>
#include <tdeparts/componentfactory.h>
#include <tdeparts/genericfactory.h>
#include <tdehtml_part.h>
#include <unistd.h>
#include <kxmlguifactory.h>
#include <tqtimer.h>

typedef KParts::GenericFactory<KMultiPart> KMultiPartFactory; // factory for the part
K_EXPORT_COMPONENT_FACTORY( libtdemultipart /*library name*/, KMultiPartFactory )

//#define DEBUG_PARSING

class KLineParser
{
public:
    KLineParser() {
        m_lineComplete = false;
    }
    void addChar( char c, bool storeNewline ) {
        if ( !storeNewline && c == '\r' )
            return;
        Q_ASSERT( !m_lineComplete );
        if ( storeNewline || c != '\n' ) {
            int sz = m_currentLine.size();
            m_currentLine.resize( sz+1, TQGArray::SpeedOptim );
            m_currentLine[sz] = c;
        }
        if ( c == '\n' )
            m_lineComplete = true;
    }
    bool isLineComplete() const {
        return m_lineComplete;
    }
    TQByteArray currentLine() const {
        return m_currentLine;
    }
    void clearLine() {
        Q_ASSERT( m_lineComplete );
        reset();
    }
    void reset() {
        m_currentLine.resize( 0, TQGArray::SpeedOptim );
        m_lineComplete = false;
    }
private:
    TQByteArray m_currentLine;
    bool m_lineComplete; // true when ending with '\n'
};

/* testcase:
   Content-type: multipart/mixed;boundary=ThisRandomString

--ThisRandomString
Content-type: text/plain

Data for the first object.

--ThisRandomString
Content-type: text/plain

Data for the second and last object.

--ThisRandomString--
*/


KMultiPart::KMultiPart( TQWidget *parentWidget, const char *widgetName,
                        TQObject *parent, const char *name, const TQStringList& )
    : KParts::ReadOnlyPart( parent, name )
{
    m_filter = 0L;

    setInstance( KMultiPartFactory::instance() );

    TQVBox *box = new TQVBox( parentWidget, widgetName );
    setWidget( box );

    m_extension = new KParts::BrowserExtension( this );

    // We probably need to use m_extension to get the urlArgs in openURL...

    m_part = 0L;
    m_isHTMLPart = false;
    m_job = 0L;
    m_lineParser = new KLineParser;
    m_tempFile = 0L;

    m_timer = new TQTimer( this );
    connect( m_timer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( slotProgressInfo() ) );
}

KMultiPart::~KMultiPart()
{
    // important: delete the nested part before the part or qobject destructor runs.
    // we now delete the nested part which deletes the part's widget which makes
    // _OUR_ m_widget 0 which in turn avoids our part destructor to delete the
    // widget ;-)
    // ### additional note: it _can_ be that the part has been deleted before:
    // when we're in a html frameset and the view dies first, then it will also
    // kill the htmlpart
    if ( m_part )
        delete static_cast<KParts::ReadOnlyPart *>( m_part );
    delete m_job;
    delete m_lineParser;
    if ( m_tempFile ) {
        m_tempFile->setAutoDelete( true );
        delete m_tempFile;
    }
    delete m_filter;
    m_filter = 0L;
}


void KMultiPart::startHeader()
{
    m_bParsingHeader = true; // we expect a header to come first
    m_bGotAnyHeader = false;
    m_gzip = false;
    // just to be sure for now
    delete m_filter;
    m_filter = 0L;
}


bool KMultiPart::openURL( const KURL &url )
{
    m_url = url;
    m_lineParser->reset();
    startHeader();

    KParts::URLArgs args = m_extension->urlArgs();
    //m_mimeType = args.serviceType;

    // Hmm, args.reload is set to true when reloading, but this doesn't seem to be enough...
    // I get "HOLD: Reusing held slave for <url>", and the old data

    m_job = TDEIO::get( url, args.reload, false );

    emit started( 0 /*m_job*/ ); // don't pass the job, it would interfer with our own infoMessage

    connect( m_job, TQ_SIGNAL( result( TDEIO::Job * ) ),
             this, TQ_SLOT( slotJobFinished( TDEIO::Job * ) ) );
    connect( m_job, TQ_SIGNAL( data( TDEIO::Job *, const TQByteArray & ) ),
             this, TQ_SLOT( slotData( TDEIO::Job *, const TQByteArray & ) ) );

    m_numberOfFrames = 0;
    m_numberOfFramesSkipped = 0;
    m_totalNumberOfFrames = 0;
    m_qtime.start();
    m_timer->start( 1000 ); //1s

    return true;
}

// Yes, libtdenetwork's has such a parser already (MultiPart),
// but it works on the complete string, expecting the whole data to be available....
// The version here is asynchronous.
void KMultiPart::slotData( TDEIO::Job *job, const TQByteArray &data )
{
    if (m_boundary.isNull())
    {
       TQString tmp = job->queryMetaData("media-boundary");
       kdDebug() << "Got Boundary from tdeio-http '" << tmp << "'" << endl;
       if ( !tmp.isEmpty() ) {
           if (tmp.startsWith("--"))
               m_boundary = tmp.latin1();
           else
               m_boundary = TQCString("--")+tmp.latin1();
           m_boundaryLength = m_boundary.length();
       }
    }
    // Append to m_currentLine until eol
    for ( uint i = 0; i < data.size() ; ++i )
    {
        // Store char. Skip if '\n' and currently parsing a header.
        m_lineParser->addChar( data[i], !m_bParsingHeader );
        if ( m_lineParser->isLineComplete() )
        {
            TQByteArray lineData = m_lineParser->currentLine();
#ifdef DEBUG_PARSING
            kdDebug() << "lineData.size()=" << lineData.size() << endl;
#endif
            TQCString line( lineData.data(), lineData.size()+1 ); // deep copy
            // 0-terminate the data, but only for the line-based tests below
            // We want to keep the raw data in case it ends up in sendData()
            int sz = line.size();
            if ( sz > 0 )
                line[sz-1] = '\0';
#ifdef DEBUG_PARSING
            kdDebug() << "[" << m_bParsingHeader << "] line='" << line << "'" << endl;
#endif
            if ( m_bParsingHeader )
            {
                if ( !line.isEmpty() )
                    m_bGotAnyHeader = true;
                if ( m_boundary.isNull() )
                {
                    if ( !line.isEmpty() ) {
#ifdef DEBUG_PARSING
                        kdDebug() << "Boundary is " << line << endl;
#endif
                        m_boundary = line;
                        m_boundaryLength = m_boundary.length();
                    }
                }
                else if ( !tqstrnicmp( line.data(), "Content-Encoding:", 17 ) )
                {
                    TQString encoding = TQString::fromLatin1(line.data()+17).stripWhiteSpace().lower();
                    if (encoding == "gzip" || encoding == "x-gzip") {
                        m_gzip = true;
                    } else {
                        kdDebug() << "FIXME: unhandled encoding type in KMultiPart: " << encoding << endl;
                    }
                }
                // parse Content-Type
                else if ( !tqstrnicmp( line.data(), "Content-Type:", 13 ) )
                {
                    Q_ASSERT( m_nextMimeType.isNull() );
                    m_nextMimeType = TQString::fromLatin1( line.data() + 14 ).stripWhiteSpace();
                    int semicolon = m_nextMimeType.find( ';' );
                    if ( semicolon != -1 )
                        m_nextMimeType = m_nextMimeType.left( semicolon );
                    kdDebug() << "m_nextMimeType=" << m_nextMimeType << endl;
                }
                // Empty line, end of headers (if we had any header line before)
                else if ( line.isEmpty() && m_bGotAnyHeader )
                {
                    m_bParsingHeader = false;
#ifdef DEBUG_PARSING
                    kdDebug() << "end of headers" << endl;
#endif
                    startOfData();
                }
                // First header (when we know it from tdeio_http)
                else if ( line == m_boundary )
                    ; // nothing to do
                else if ( !line.isEmpty() ) // this happens with e.g. Set-Cookie:
                    kdDebug() << "Ignoring header " << line << endl;
            } else {
                if ( !tqstrncmp( line, m_boundary, m_boundaryLength ) )
                {
#ifdef DEBUG_PARSING
                    kdDebug() << "boundary found!" << endl;
                    kdDebug() << "after it is " << line.data() + m_boundaryLength << endl;
#endif
                    // Was it the very last boundary ?
                    if ( !tqstrncmp( line.data() + m_boundaryLength, "--", 2 ) )
                    {
#ifdef DEBUG_PARSING
                        kdDebug() << "Completed!" << endl;
#endif
                        endOfData();
                        emit completed();
                    } else
                    {
                        char nextChar = *(line.data() + m_boundaryLength);
#ifdef DEBUG_PARSING
                        kdDebug() << "KMultiPart::slotData nextChar='" << nextChar << "'" << endl;
#endif
                        if ( nextChar == '\n' || nextChar == '\r' ) {
                            endOfData();
                            startHeader();
                        }
                        else {
                            // otherwise, false hit, it has trailing stuff
                            sendData( lineData );
                        }
                    }
                } else {
                    // send to part
                    sendData( lineData );
                }
            }
            m_lineParser->clearLine();
        }
    }
}

void KMultiPart::setPart( const TQString& mimeType )
{
    KXMLGUIFactory *guiFactory = factory();
    if ( guiFactory ) // seems to be 0 when restoring from SM
        guiFactory->removeClient( this );
    kdDebug() << "KMultiPart::setPart " << mimeType << endl;
    delete m_part;
    // Try to find an appropriate viewer component
    m_part = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>
             ( m_mimeType, TQString::null, widget(), 0L, this, 0L );
    if ( !m_part ) {
        // TODO launch external app
        KMessageBox::error( widget(), i18n("No handler found for %1!").arg(m_mimeType) );
        return;
    }
    // By making the part a child XMLGUIClient of ours, we get its GUI merged in.
    insertChildClient( m_part );
    m_part->widget()->show();

    connect( m_part, TQ_SIGNAL( completed() ),
             this, TQ_SLOT( slotPartCompleted() ) );

    m_isHTMLPart = ( mimeType == "text/html" );
    KParts::BrowserExtension* childExtension = KParts::BrowserExtension::childObject( m_part );

    if ( childExtension )
    {

        // Forward signals from the part's browser extension
        // this is very related (but not exactly like) TDEHTMLPart::processObjectRequest

        connect( childExtension, TQ_SIGNAL( openURLNotify() ),
                 m_extension, TQ_SIGNAL( openURLNotify() ) );

        connect( childExtension, TQ_SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
                 m_extension, TQ_SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ) );

        connect( childExtension, TQ_SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ),
                 m_extension, TQ_SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ) );
        connect( childExtension, TQ_SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs &, const KParts::WindowArgs &, KParts::ReadOnlyPart *& ) ),
                 m_extension, TQ_SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & , const KParts::WindowArgs &, KParts::ReadOnlyPart *&) ) );

        // Keep in sync with tdehtml_part.cpp
        connect( childExtension, TQ_SIGNAL( popupMenu( const TQPoint &, const KFileItemList & ) ),
                 m_extension, TQ_SIGNAL( popupMenu( const TQPoint &, const KFileItemList & ) ) );
        connect( childExtension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KFileItemList & ) ),
                 m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KFileItemList & ) ) );
        connect( childExtension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KFileItemList &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags ) ),
                 m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KFileItemList &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags ) ) );
        connect( childExtension, TQ_SIGNAL( popupMenu( const TQPoint &, const KURL &, const TQString &, mode_t ) ),
                 m_extension, TQ_SIGNAL( popupMenu( const TQPoint &, const KURL &, const TQString &, mode_t ) ) );
        connect( childExtension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &, const TQString &, mode_t ) ),
                 m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &, const TQString &, mode_t ) ) );
        connect( childExtension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t ) ),
                 m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t ) ) );


        if ( m_isHTMLPart )
            connect( childExtension, TQ_SIGNAL( infoMessage( const TQString & ) ),
                     m_extension, TQ_SIGNAL( infoMessage( const TQString & ) ) );
        // For non-HTML we prefer to show our infoMessage ourselves.

        childExtension->setBrowserInterface( m_extension->browserInterface() );

        connect( childExtension, TQ_SIGNAL( enableAction( const char *, bool ) ),
                 m_extension, TQ_SIGNAL( enableAction( const char *, bool ) ) );
        connect( childExtension, TQ_SIGNAL( setLocationBarURL( const TQString& ) ),
                 m_extension, TQ_SIGNAL( setLocationBarURL( const TQString& ) ) );
        connect( childExtension, TQ_SIGNAL( setIconURL( const KURL& ) ),
                 m_extension, TQ_SIGNAL( setIconURL( const KURL& ) ) );
        connect( childExtension, TQ_SIGNAL( loadingProgress( int ) ),
                 m_extension, TQ_SIGNAL( loadingProgress( int ) ) );
        if ( m_isHTMLPart ) // for non-HTML we have our own
            connect( childExtension, TQ_SIGNAL( speedProgress( int ) ),
                     m_extension, TQ_SIGNAL( speedProgress( int ) ) );
        connect( childExtension, TQ_SIGNAL( selectionInfo( const KFileItemList& ) ),
                 m_extension, TQ_SIGNAL( selectionInfo( const KFileItemList& ) ) );
        connect( childExtension, TQ_SIGNAL( selectionInfo( const TQString& ) ),
                 m_extension, TQ_SIGNAL( selectionInfo( const TQString& ) ) );
        connect( childExtension, TQ_SIGNAL( selectionInfo( const KURL::List& ) ),
                 m_extension, TQ_SIGNAL( selectionInfo( const KURL::List& ) ) );
        connect( childExtension, TQ_SIGNAL( mouseOverInfo( const KFileItem* ) ),
                 m_extension, TQ_SIGNAL( mouseOverInfo( const KFileItem* ) ) );
        connect( childExtension, TQ_SIGNAL( moveTopLevelWidget( int, int ) ),
                 m_extension, TQ_SIGNAL( moveTopLevelWidget( int, int ) ) );
        connect( childExtension, TQ_SIGNAL( resizeTopLevelWidget( int, int ) ),
                 m_extension, TQ_SIGNAL( resizeTopLevelWidget( int, int ) ) );
    }

    m_partIsLoading = false;
    // Load the part's plugins too.
    // ###### This is a hack. The bug is that TDEHTMLPart doesn't load its plugins
    // if className != "Browser/View".
    loadPlugins( this, m_part, m_part->instance() );
    // Get the part's GUI to appear
    if ( guiFactory )
        guiFactory->addClient( this );
}

void KMultiPart::startOfData()
{
    kdDebug() << "KMultiPart::startOfData" << endl;
    Q_ASSERT( !m_nextMimeType.isNull() );
    if( m_nextMimeType.isNull() )
        return;

    if ( m_gzip )
    {
        m_filter = new HTTPFilterGZip;
        connect( m_filter, TQ_SIGNAL( output( const TQByteArray& ) ), this, TQ_SLOT( reallySendData( const TQByteArray& ) ) );
    }

    if ( m_mimeType != m_nextMimeType )
    {
        // Need to switch parts (or create the initial one)
        m_mimeType = m_nextMimeType;
        setPart( m_mimeType );
    }
    Q_ASSERT( m_part );
    // Pass URLArgs (e.g. reload)
    KParts::BrowserExtension* childExtension = KParts::BrowserExtension::childObject( m_part );
    if ( childExtension )
        childExtension->setURLArgs( m_extension->urlArgs() );

    m_nextMimeType = TQString::null;
    if ( m_tempFile ) {
        m_tempFile->setAutoDelete( true );
        delete m_tempFile;
        m_tempFile = 0;
    }
    if ( m_isHTMLPart )
    {
        TDEHTMLPart* htmlPart = static_cast<TDEHTMLPart *>( static_cast<KParts::ReadOnlyPart *>( m_part ) );
        htmlPart->begin( url() );
    }
    else
    {
        // ###### TODO use a TQByteArray and a data: URL instead
        m_tempFile = new KTempFile;
    }
}

void KMultiPart::sendData( const TQByteArray& line )
{
    if ( m_filter )
    {
        m_filter->slotInput( line );
    }
    else
    {
        reallySendData( line );
    }
}

void KMultiPart::reallySendData( const TQByteArray& line )
{
    if ( m_isHTMLPart )
    {
        TDEHTMLPart* htmlPart = static_cast<TDEHTMLPart *>( static_cast<KParts::ReadOnlyPart *>( m_part ) );
        htmlPart->write( line.data(), line.size() );
    }
    else if ( m_tempFile )
    {
        m_tempFile->file()->writeBlock( line.data(), line.size() );
    }
}

void KMultiPart::endOfData()
{
    Q_ASSERT( m_part );
    if ( m_isHTMLPart )
    {
        TDEHTMLPart* htmlPart = static_cast<TDEHTMLPart *>( static_cast<KParts::ReadOnlyPart *>( m_part ) );
        htmlPart->end();
    } else if ( m_tempFile )
    {
        m_tempFile->close();
        if ( m_partIsLoading )
        {
            // The part is still loading the last data! Let it proceed then
            // Otherwise we'd keep cancelling it, and nothing would ever show up...
            kdDebug() << "KMultiPart::endOfData part isn't ready, skipping frame" << endl;
            ++m_numberOfFramesSkipped;
            m_tempFile->setAutoDelete( true );
        }
        else
        {
            kdDebug() << "KMultiPart::endOfData opening " << m_tempFile->name() << endl;
            KURL url;
            url.setPath( m_tempFile->name() );
            m_partIsLoading = true;
            (void) m_part->openURL( url );
        }
        delete m_tempFile;
        m_tempFile = 0L;
    }
}

void KMultiPart::slotPartCompleted()
{
    if ( !m_isHTMLPart )
    {
        Q_ASSERT( m_part );
        // Delete temp file used by the part
        Q_ASSERT( m_part->url().isLocalFile() );
	kdDebug() << "slotPartCompleted deleting " << m_part->url().path() << endl;
        (void) unlink( TQFile::encodeName( m_part->url().path() ) );
        m_partIsLoading = false;
        ++m_numberOfFrames;
        // Do not emit completed from here.
    }
}

bool KMultiPart::closeURL()
{
    m_timer->stop();
    if ( m_part )
        return m_part->closeURL();
    return true;
}

void KMultiPart::guiActivateEvent( KParts::GUIActivateEvent * )
{
    // Not public!
    //if ( m_part )
    //    m_part->guiActivateEvent( e );
}

void KMultiPart::slotJobFinished( TDEIO::Job *job )
{
    if ( job->error() )
    {
        // TODO use tdehtml's error:// scheme
        job->showErrorDialog();
        emit canceled( job->errorString() );
    }
    else
    {
        /*if ( m_tdehtml->view()->contentsY() == 0 )
        {
            KParts::URLArgs args = m_ext->urlArgs();
            m_tdehtml->view()->setContentsPos( args.xOffset, args.yOffset );
        }*/

        emit completed();

        //TQTimer::singleShot( 0, this, TQ_SLOT( updateWindowCaption() ) );
    }
    m_job = 0L;
}

void KMultiPart::slotProgressInfo()
{
    int time = m_qtime.elapsed();
    if ( !time ) return;
    if ( m_totalNumberOfFrames == m_numberOfFrames + m_numberOfFramesSkipped )
        return; // No change, don't overwrite statusbar messages if any
    //kdDebug() << m_numberOfFrames << " in " << time << " milliseconds" << endl;
    TQString str( "%1 frames per second, %2 frames skipped per second" );
    str = str.arg( 1000.0 * (double)m_numberOfFrames / (double)time );
    str = str.arg( 1000.0 * (double)m_numberOfFramesSkipped / (double)time );
    m_totalNumberOfFrames = m_numberOfFrames + m_numberOfFramesSkipped;
    //kdDebug() << str << endl;
    emit m_extension->infoMessage( str );
}

TDEAboutData* KMultiPart::createAboutData()
{
    TDEAboutData* aboutData = new TDEAboutData( "tdemultipart", I18N_NOOP("KMultiPart"),
                                             "0.1",
                                            I18N_NOOP( "Embeddable component for multipart/mixed" ),
                                             TDEAboutData::License_GPL,
                                             "(c) 2001, David Faure <david@mandrakesoft.com>");
    return aboutData;
}

#if 0
KMultiPartBrowserExtension::KMultiPartBrowserExtension( KMultiPart *parent, const char *name )
    : KParts::BrowserExtension( parent, name )
{
    m_imgPart = parent;
}

int KMultiPartBrowserExtension::xOffset()
{
    return m_imgPart->doc()->view()->contentsX();
}

int KMultiPartBrowserExtension::yOffset()
{
    return m_imgPart->doc()->view()->contentsY();
}

void KMultiPartBrowserExtension::print()
{
    static_cast<TDEHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->print();
}

void KMultiPartBrowserExtension::reparseConfiguration()
{
    static_cast<TDEHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->reparseConfiguration();
    m_imgPart->doc()->setAutoloadImages( true );
}
#endif

#include "tdemultipart.moc"
