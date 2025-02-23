/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>

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

#include "tdehtmlimage.h"
#include "tdehtmlview.h"
#include "tdehtml_ext.h"
#include "xml/dom_docimpl.h"
#include "html/html_documentimpl.h"
#include "html/html_elementimpl.h"
#include "rendering/render_image.h"
#include "misc/loader.h"

#include <tqvbox.h>
#include <tqtimer.h>

#include <tdeio/job.h>
#include <kinstance.h>
#include <kmimetype.h>
#include <tdelocale.h>

K_EXPORT_COMPONENT_FACTORY( tdehtmlimagefactory /*NOT the part name, see Makefile.am*/, TDEHTMLImageFactory )

TDEInstance *TDEHTMLImageFactory::s_instance = 0;

TDEHTMLImageFactory::TDEHTMLImageFactory()
{
    s_instance = new TDEInstance( "tdehtmlimage" );
}

TDEHTMLImageFactory::~TDEHTMLImageFactory()
{
    delete s_instance;
}

KParts::Part *TDEHTMLImageFactory::createPartObject( TQWidget *parentWidget, const char *widgetName,
                                                   TQObject *parent, const char *name,
                                                   const char *className, const TQStringList & )
{
  TDEHTMLPart::GUIProfile prof = TDEHTMLPart::DefaultGUI;
  if ( strcmp( className, "Browser/View" ) == 0 )
    prof = TDEHTMLPart::BrowserViewGUI;
  return new TDEHTMLImage( parentWidget, widgetName, parent, name, prof );
}

TDEHTMLImage::TDEHTMLImage( TQWidget *parentWidget, const char *widgetName,
                        TQObject *parent, const char *name, TDEHTMLPart::GUIProfile prof )
    : KParts::ReadOnlyPart( parent, name ), m_image( 0 )
{
    TDEHTMLPart* parentPart = ::tqt_cast<TDEHTMLPart *>( parent );
    setInstance( TDEHTMLImageFactory::instance(), prof == TDEHTMLPart::BrowserViewGUI && !parentPart );

    TQVBox *box = new TQVBox( parentWidget, widgetName );

    m_tdehtml = new TDEHTMLPart( box, widgetName, this, "htmlimagepart", prof );
    m_tdehtml->setAutoloadImages( true );
    m_tdehtml->widget()->installEventFilter(this);
    connect( m_tdehtml->view(), TQ_SIGNAL( finishedLayout() ), this, TQ_SLOT( restoreScrollPosition() ) );

    setWidget( box );

    // VBox can't take focus, so pass it on to sub-widget
    box->setFocusProxy( m_tdehtml->widget() );

    m_ext = new TDEHTMLImageBrowserExtension( this, "be" );

    // Remove unnecessary actions.
    TDEAction *encodingAction = actionCollection()->action( "setEncoding" );
    if ( encodingAction )
    {
        encodingAction->unplugAll();
        delete encodingAction;
    }
    TDEAction *viewSourceAction= actionCollection()->action( "viewDocumentSource" );
    if ( viewSourceAction )
    {
        viewSourceAction->unplugAll();
        delete viewSourceAction;
    }

    TDEAction *selectAllAction= actionCollection()->action( "selectAll" );
    if ( selectAllAction )
    {
        selectAllAction->unplugAll();
        delete selectAllAction;
    }

    // forward important signals from the tdehtml part

    // forward opening requests to parent frame (if existing)
    TDEHTMLPart *p = ::tqt_cast<TDEHTMLPart *>(parent);
    KParts::BrowserExtension *be = p ? p->browserExtension() : m_ext;
    connect(m_tdehtml->browserExtension(), TQ_SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)),
    		be, TQ_SIGNAL(openURLRequestDelayed(const KURL &, const KParts::URLArgs &)));

    connect( m_tdehtml->browserExtension(), TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &,
             const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t) ), m_ext, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &,
             const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t) ) );

    connect( m_tdehtml->browserExtension(), TQ_SIGNAL( enableAction( const char *, bool ) ),
             m_ext, TQ_SIGNAL( enableAction( const char *, bool ) ) );

    m_ext->setURLDropHandlingEnabled( true );
}

TDEHTMLImage::~TDEHTMLImage()
{
    disposeImage();

    // important: delete the html part before the part or qobject destructor runs.
    // we now delete the htmlpart which deletes the part's widget which makes
    // _OUR_ m_widget 0 which in turn avoids our part destructor to delete the
    // widget ;-)
    // ### additional note: it _can_ be that the part has been deleted before:
    // when we're in a html frameset and the view dies first, then it will also
    // kill the htmlpart
    if ( m_tdehtml )
        delete static_cast<TDEHTMLPart *>( m_tdehtml );
}

bool TDEHTMLImage::openURL( const KURL &url )
{
    static const TQString &html = TDEGlobal::staticQString( "<html><body><img src=\"%1\"></body></html>" );

    disposeImage();

    m_url = url;

    emit started( 0 );

    KParts::URLArgs args = m_ext->urlArgs();
    m_mimeType = args.serviceType;

    emit setWindowCaption( url.prettyURL() );

    // Need to keep a copy of the offsets since they are cleared when emitting completed
    m_xOffset = args.xOffset;
    m_yOffset = args.yOffset;

    m_tdehtml->begin( m_url );
    m_tdehtml->setAutoloadImages( true );

    DOM::DocumentImpl *impl = dynamic_cast<DOM::DocumentImpl *>( m_tdehtml->document().handle() ); // ### hack ;-)
    if (!impl) return false;
    if ( m_ext->urlArgs().reload )
        impl->docLoader()->setCachePolicy( TDEIO::CC_Reload );

    tdehtml::DocLoader *dl = impl->docLoader();
    m_image = dl->requestImage( m_url.url() );
    if ( m_image )
        m_image->ref( this );

    m_tdehtml->write( html.arg( m_url.url() ) );
    m_tdehtml->end();

    /*
    connect( tdehtml::Cache::loader(), TQ_SIGNAL( requestDone( tdehtml::DocLoader*, tdehtml::CachedObject *) ),
            this, TQ_SLOT( updateWindowCaption() ) );
            */
    return true;
}

bool TDEHTMLImage::closeURL()
{
    disposeImage();
    return m_tdehtml->closeURL();
}

// This can happen after openURL returns, or directly from m_image->ref()
void TDEHTMLImage::notifyFinished( tdehtml::CachedObject *o )
{
    if ( !m_image || o != m_image )
        return;

    const TQPixmap &pix = m_image->pixmap();
    TQString caption;

    KMimeType::Ptr mimeType;
    if ( !m_mimeType.isEmpty() )
        mimeType = KMimeType::mimeType( m_mimeType );

    if ( mimeType ) {
        if (m_image && !m_image->suggestedTitle().isEmpty()) {
            caption = i18n( "%1 (%2 - %3x%4 Pixels)" ).arg( m_image->suggestedTitle(), mimeType->comment() ).arg( pix.width() ).arg( pix.height() );
        } else {
            caption = i18n( "%1 - %2x%3 Pixels" ).arg( mimeType->comment() )
                .arg( pix.width() ).arg( pix.height() );
        }
    } else {
        if (m_image && !m_image->suggestedTitle().isEmpty()) {
            caption = i18n( "%1 (%2x%3 Pixels)" ).arg(m_image->suggestedTitle()).arg( pix.width() ).arg( pix.height() );
        } else {
            caption = i18n( "Image - %1x%2 Pixels" ).arg( pix.width() ).arg( pix.height() );
        }
    }

    emit setWindowCaption( caption );
    emit completed();
    emit setStatusBarText(i18n("Done."));
}

void TDEHTMLImage::restoreScrollPosition()
{
    if ( m_tdehtml->view()->contentsY() == 0 ) {
        m_tdehtml->view()->setContentsPos( m_xOffset, m_yOffset );
    }
}

void TDEHTMLImage::guiActivateEvent( KParts::GUIActivateEvent *e )
{
    // prevent the base implementation from emitting setWindowCaption with
    // our url. It destroys our pretty, previously caption. Konq saves/restores
    // the caption for us anyway.
    if ( e->activated() )
        return;
    KParts::ReadOnlyPart::guiActivateEvent(e);
}

/*
void TDEHTMLImage::slotImageJobFinished( TDEIO::Job *job )
{
    if ( job->error() )
    {
        job->showErrorDialog();
        emit canceled( job->errorString() );
    }
    else
    {
        emit completed();
        TQTimer::singleShot( 0, this, TQ_SLOT( updateWindowCaption() ) );
    }
}

void TDEHTMLImage::updateWindowCaption()
{
    if ( !m_tdehtml )
        return;

    DOM::HTMLDocumentImpl *impl = dynamic_cast<DOM::HTMLDocumentImpl *>( m_tdehtml->document().handle() );
    if ( !impl )
        return;

    DOM::HTMLElementImpl *body = impl->body();
    if ( !body )
        return;

    DOM::NodeImpl *image = body->firstChild();
    if ( !image )
        return;

    tdehtml::RenderImage *renderImage = dynamic_cast<tdehtml::RenderImage *>( image->renderer() );
    if ( !renderImage )
        return;

    TQPixmap pix = renderImage->pixmap();

    TQString caption;

    KMimeType::Ptr mimeType;
    if ( !m_mimeType.isEmpty() )
        mimeType = KMimeType::mimeType( m_mimeType );

    if ( mimeType )
        caption = i18n( "%1 - %2x%3 Pixels" ).arg( mimeType->comment() )
                  .arg( pix.width() ).arg( pix.height() );
    else
        caption = i18n( "Image - %1x%2 Pixels" ).arg( pix.width() ).arg( pix.height() );

    emit setWindowCaption( caption );
    emit completed();
    emit setStatusBarText(i18n("Done."));
}
*/

void TDEHTMLImage::disposeImage()
{
    if ( !m_image )
        return;

    m_image->deref( this );
    m_image = 0;
}

bool TDEHTMLImage::eventFilter(TQObject *, TQEvent *e) {
    switch (e->type()) {
      case TQEvent::DragEnter:
      case TQEvent::DragMove:
      case TQEvent::DragLeave:
      case TQEvent::Drop: {
        // find out if this part is embedded in a frame, and send the
	// event to its outside widget
	TDEHTMLPart *p = ::tqt_cast<TDEHTMLPart *>(parent());
	if (p)
	    return TQApplication::sendEvent(p->widget(), e);
        // otherwise simply forward all dnd events to the part widget,
	// konqueror will handle them properly there
        return TQApplication::sendEvent(widget(), e);
      }
      default: ;
    }
    return false;
}

TDEHTMLImageBrowserExtension::TDEHTMLImageBrowserExtension( TDEHTMLImage *parent, const char *name )
    : KParts::BrowserExtension( parent, name )
{
    m_imgPart = parent;
}

int TDEHTMLImageBrowserExtension::xOffset()
{
    return m_imgPart->doc()->view()->contentsX();
}

int TDEHTMLImageBrowserExtension::yOffset()
{
    return m_imgPart->doc()->view()->contentsY();
}

void TDEHTMLImageBrowserExtension::print()
{
    static_cast<TDEHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->print();
}

void TDEHTMLImageBrowserExtension::reparseConfiguration()
{
    static_cast<TDEHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->reparseConfiguration();
    m_imgPart->doc()->setAutoloadImages( true );
}


void TDEHTMLImageBrowserExtension::disableScrolling()
{
    static_cast<TDEHTMLPartBrowserExtension *>( m_imgPart->doc()->browserExtension() )->disableScrolling();
}

using namespace KParts;

#include "tdehtmlimage.moc"
