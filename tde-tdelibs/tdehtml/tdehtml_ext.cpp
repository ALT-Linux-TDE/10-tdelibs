/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 Simon Hausmann <hausmann@kde.org>
 *               2001-2003 George Staikos <staikos@kde.org>
 *               2001-2003 Laurent Montel <montel@kde.org>
 *               2001-2003 Dirk Mueller <mueller@kde.org>
 *               2001-2003 Waldo Bastian <bastian@kde.org>
 *               2001-2003 David Faure <faure@kde.org>
 *               2001-2003 Daniel Naber <dnaber@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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

#include <assert.h>
#include "tdehtml_ext.h"
#include "tdehtmlview.h"
#include "tdehtml_pagecache.h"
#include "rendering/render_form.h"
#include "rendering/render_image.h"
#include "html/html_imageimpl.h"
#include "misc/loader.h"
#include "dom/html_form.h"
#include "dom/html_image.h"
#include <tqclipboard.h>
#include <tqfileinfo.h>
#include <tqpopupmenu.h>
#include <tqurl.h>
#include <tqmetaobject.h>
#include <tqucomextra_p.h>
#include <tqdragobject.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <tdefiledialog.h>
#include <tdeio/job.h>
#include <kprocess.h>
#include <tdetoolbarbutton.h>
#include <tdetoolbar.h>
#include <ksavefile.h>
#include <kurldrag.h>
#include <kstringhandler.h>
#include <tdeapplication.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>
#include <krun.h>
#include <kurifilter.h>
#include <kiconloader.h>
#include <kdesktopfile.h>
#include <tdemultipledrag.h>
#include <kinputdialog.h>

#include "tdehtml_factory.h"

#include "dom/dom_element.h"
#include "misc/htmltags.h"

#include "tdehtmlpart_p.h"

TDEHTMLPartBrowserExtension::TDEHTMLPartBrowserExtension( TDEHTMLPart *parent, const char *name )
: KParts::BrowserExtension( parent, name )
{
    m_part = parent;
    setURLDropHandlingEnabled( true );

    enableAction( "cut", false );
    enableAction( "copy", false );
    enableAction( "paste", false );

    m_connectedToClipboard = false;
}

int TDEHTMLPartBrowserExtension::xOffset()
{
    return m_part->view()->contentsX();
}

int TDEHTMLPartBrowserExtension::yOffset()
{
  return m_part->view()->contentsY();
}

void TDEHTMLPartBrowserExtension::saveState( TQDataStream &stream )
{
  //kdDebug( 6050 ) << "saveState!" << endl;
  m_part->saveState( stream );
}

void TDEHTMLPartBrowserExtension::restoreState( TQDataStream &stream )
{
  //kdDebug( 6050 ) << "restoreState!" << endl;
  m_part->restoreState( stream );
}

void TDEHTMLPartBrowserExtension::editableWidgetFocused( TQWidget *widget )
{
    m_editableFormWidget = widget;
    updateEditActions();

    if ( !m_connectedToClipboard && m_editableFormWidget )
    {
        connect( TQApplication::clipboard(), TQ_SIGNAL( dataChanged() ),
                 this, TQ_SLOT( updateEditActions() ) );

        if ( m_editableFormWidget->inherits( "TQLineEdit" ) || m_editableFormWidget->inherits( "TQTextEdit" ) )
            connect( m_editableFormWidget, TQ_SIGNAL( selectionChanged() ),
                     this, TQ_SLOT( updateEditActions() ) );

        m_connectedToClipboard = true;
    }
    editableWidgetFocused();
}

void TDEHTMLPartBrowserExtension::editableWidgetBlurred( TQWidget * /*widget*/ )
{
    TQWidget *oldWidget = m_editableFormWidget;

    m_editableFormWidget = 0;
    enableAction( "cut", false );
    enableAction( "paste", false );
    m_part->emitSelectionChanged();

    if ( m_connectedToClipboard )
    {
        disconnect( TQApplication::clipboard(), TQ_SIGNAL( dataChanged() ),
                    this, TQ_SLOT( updateEditActions() ) );

        if ( oldWidget )
        {
            if ( oldWidget->inherits( "TQLineEdit" ) || oldWidget->inherits( "TQTextEdit" ) )
                disconnect( oldWidget, TQ_SIGNAL( selectionChanged() ),
                            this, TQ_SLOT( updateEditActions() ) );
        }

        m_connectedToClipboard = false;
    }
    editableWidgetBlurred();
}

void TDEHTMLPartBrowserExtension::setExtensionProxy( KParts::BrowserExtension *proxy )
{
    if ( m_extensionProxy )
    {
        disconnect( m_extensionProxy, TQ_SIGNAL( enableAction( const char *, bool ) ),
                    this, TQ_SLOT( extensionProxyActionEnabled( const char *, bool ) ) );
        if ( m_extensionProxy->inherits( "TDEHTMLPartBrowserExtension" ) )
        {
            disconnect( m_extensionProxy, TQ_SIGNAL( editableWidgetFocused() ),
                        this, TQ_SLOT( extensionProxyEditableWidgetFocused() ) );
            disconnect( m_extensionProxy, TQ_SIGNAL( editableWidgetBlurred() ),
                        this, TQ_SLOT( extensionProxyEditableWidgetBlurred() ) );
        }
    }

    m_extensionProxy = proxy;

    if ( m_extensionProxy )
    {
        connect( m_extensionProxy, TQ_SIGNAL( enableAction( const char *, bool ) ),
                 this, TQ_SLOT( extensionProxyActionEnabled( const char *, bool ) ) );
        if ( m_extensionProxy->inherits( "TDEHTMLPartBrowserExtension" ) )
        {
            connect( m_extensionProxy, TQ_SIGNAL( editableWidgetFocused() ),
                     this, TQ_SLOT( extensionProxyEditableWidgetFocused() ) );
            connect( m_extensionProxy, TQ_SIGNAL( editableWidgetBlurred() ),
                     this, TQ_SLOT( extensionProxyEditableWidgetBlurred() ) );
        }

        enableAction( "cut", m_extensionProxy->isActionEnabled( "cut" ) );
        enableAction( "copy", m_extensionProxy->isActionEnabled( "copy" ) );
        enableAction( "paste", m_extensionProxy->isActionEnabled( "paste" ) );
    }
    else
    {
        updateEditActions();
        enableAction( "copy", false ); // ### re-check this
    }
}

void TDEHTMLPartBrowserExtension::cut()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "cut()" );
        return;
    }

    if ( !m_editableFormWidget )
        return;

    if ( m_editableFormWidget->inherits( "TQLineEdit" ) )
        static_cast<TQLineEdit *>( &(*m_editableFormWidget) )->cut();
    else if ( m_editableFormWidget->inherits( "TQTextEdit" ) )
        static_cast<TQTextEdit *>( &(*m_editableFormWidget) )->cut();
}

void TDEHTMLPartBrowserExtension::copy()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "copy()" );
        return;
    }

    kdDebug( 6050 ) << "************! TDEHTMLPartBrowserExtension::copy()" << endl;
    if ( !m_editableFormWidget )
    {
        // get selected text and paste to the clipboard
        TQString text= m_part->selectedText();
	text.replace( TQChar( 0xa0 ), ' ' );


        TQClipboard *cb = TQApplication::clipboard();
        disconnect( cb, TQ_SIGNAL( selectionChanged() ), m_part, TQ_SLOT( slotClearSelection() ) );
#ifndef TQT_NO_MIMECLIPBOARD
	TQString htmltext;
	/*
	 * When selectionModeEnabled, that means the user has just selected
	 * the text, not ctrl+c to copy it.  The selection clipboard
	 * doesn't seem to support mime type, so to save time, don't calculate
	 * the selected text as html.
	 * optomisation disabled for now until everything else works.
	*/
	//if(!cb->selectionModeEnabled())
	    htmltext = m_part->selectedTextAsHTML();
	TQTextDrag *textdrag = new TQTextDrag(text, 0L);
	KMultipleDrag *drag = new KMultipleDrag( m_editableFormWidget );
	drag->addDragObject( textdrag );
	if(!htmltext.isEmpty()) {
	    htmltext.replace( TQChar( 0xa0 ), ' ' );
	    TQTextDrag *htmltextdrag = new TQTextDrag(htmltext, 0L);
	    htmltextdrag->setSubtype("html");
	    drag->addDragObject( htmltextdrag );
	}
        cb->setData(drag);
#else
	cb->setText(text);
#endif

        connect( cb, TQ_SIGNAL( selectionChanged() ), m_part, TQ_SLOT( slotClearSelection() ) );
    }
    else
    {
        if ( m_editableFormWidget->inherits( "TQLineEdit" ) )
            static_cast<TQLineEdit *>( &(*m_editableFormWidget) )->copy();
        else if ( m_editableFormWidget->inherits( "TQTextEdit" ) )
            static_cast<TQTextEdit *>( &(*m_editableFormWidget) )->copy();
    }
}

void TDEHTMLPartBrowserExtension::searchProvider()
{
    // action name is of form "previewProvider[<searchproviderprefix>:]"
    const TQString searchProviderPrefix = TQString( sender()->name() ).mid( 14 );

    KURIFilterData data;
    TQStringList list;
    data.setData( searchProviderPrefix + m_part->selectedText() );
    list << "kurisearchfilter" << "kuriikwsfilter";

    if( !KURIFilter::self()->filterURI(data, list) )
    {
        KDesktopFile file("searchproviders/google.desktop", true, "services");
	TQString encodedSearchTerm = m_part->selectedText();
	TQUrl::encode(encodedSearchTerm);
	data.setData(file.readEntry("Query").replace("\\{@}", encodedSearchTerm));
    }

    KParts::URLArgs args;
    args.frameName = "_blank";

    emit m_part->browserExtension()->openURLRequest( data.uri(), args );
}

void TDEHTMLPartBrowserExtension::openSelection()
{
    KParts::URLArgs args;
    args.frameName = "_blank";

    emit m_part->browserExtension()->openURLRequest( m_part->selectedText(), args );
}

void TDEHTMLPartBrowserExtension::paste()
{
    if ( m_extensionProxy )
    {
        callExtensionProxyMethod( "paste()" );
        return;
    }

    if ( !m_editableFormWidget )
        return;

    if ( m_editableFormWidget->inherits( "TQLineEdit" ) )
        static_cast<TQLineEdit *>( &(*m_editableFormWidget) )->paste();
    else if ( m_editableFormWidget->inherits( "TQTextEdit" ) )
        static_cast<TQTextEdit *>( &(*m_editableFormWidget) )->paste();
}

void TDEHTMLPartBrowserExtension::callExtensionProxyMethod( const char *method )
{
    if ( !m_extensionProxy )
        return;

    int slot = m_extensionProxy->metaObject()->findSlot( method );
    if ( slot == -1 )
        return;

    TQUObject o[ 1 ];
    m_extensionProxy->tqt_invoke( slot, o );
}

void TDEHTMLPartBrowserExtension::updateEditActions()
{
    if ( !m_editableFormWidget )
    {
        enableAction( "cut", false );
        enableAction( "copy", false );
        enableAction( "paste", false );
        return;
    }

    // ### duplicated from KonqMainWindow::slotClipboardDataChanged
#ifndef TQT_NO_MIMECLIPBOARD // Handle minimalized versions of Qt Embedded
    TQMimeSource *data = TQApplication::clipboard()->data();
    enableAction( "paste", data->provides( "text/plain" ) );
#else
    TQString data=TQApplication::clipboard()->text();
    enableAction( "paste", data.contains("://"));
#endif
    bool hasSelection = false;

    if( m_editableFormWidget) {
        if ( ::tqt_cast<TQLineEdit*>(m_editableFormWidget))
            hasSelection = static_cast<TQLineEdit *>( &(*m_editableFormWidget) )->hasSelectedText();
        else if(::tqt_cast<TQTextEdit*>(m_editableFormWidget))
            hasSelection = static_cast<TQTextEdit *>( &(*m_editableFormWidget) )->hasSelectedText();
    }

    enableAction( "copy", hasSelection );
    enableAction( "cut", hasSelection );
}

void TDEHTMLPartBrowserExtension::extensionProxyEditableWidgetFocused() {
	editableWidgetFocused();
}

void TDEHTMLPartBrowserExtension::extensionProxyEditableWidgetBlurred() {
	editableWidgetBlurred();
}

void TDEHTMLPartBrowserExtension::extensionProxyActionEnabled( const char *action, bool enable )
{
    // only forward enableAction calls for actions we actually do forward
    if ( strcmp( action, "cut" ) == 0 ||
         strcmp( action, "copy" ) == 0 ||
         strcmp( action, "paste" ) == 0 ) {
        enableAction( action, enable );
    }
}

void TDEHTMLPartBrowserExtension::reparseConfiguration()
{
  m_part->reparseConfiguration();
}

void TDEHTMLPartBrowserExtension::print()
{
  m_part->view()->print();
}

void TDEHTMLPartBrowserExtension::disableScrolling()
{
  TQScrollView *scrollView = m_part->view();
  if (scrollView) {
    scrollView->setVScrollBarMode(TQScrollView::AlwaysOff);
    scrollView->setHScrollBarMode(TQScrollView::AlwaysOff);
  }
}

class TDEHTMLPopupGUIClient::TDEHTMLPopupGUIClientPrivate
{
public:
  TDEHTMLPart *m_tdehtml;
  KURL m_url;
  KURL m_imageURL;
  TQPixmap m_pixmap;
  TQString m_suggestedFilename;
};


TDEHTMLPopupGUIClient::TDEHTMLPopupGUIClient( TDEHTMLPart *tdehtml, const TQString &doc, const KURL &url )
  : TQObject( tdehtml )
{
  d = new TDEHTMLPopupGUIClientPrivate;
  d->m_tdehtml = tdehtml;
  d->m_url = url;
  bool isImage = false;
  bool hasSelection = tdehtml->hasSelection();
  setInstance( tdehtml->instance() );

  DOM::Element e;
  e = tdehtml->nodeUnderMouse();

  if ( !e.isNull() && (e.elementId() == ID_IMG ||
                       (e.elementId() == ID_INPUT && !static_cast<DOM::HTMLInputElement>(e).src().isEmpty())))
  {
    if (e.elementId() == ID_IMG) {
      DOM::HTMLImageElementImpl *ie = static_cast<DOM::HTMLImageElementImpl*>(e.handle());
      tdehtml::RenderImage *ri = dynamic_cast<tdehtml::RenderImage*>(ie->renderer());
      if (ri && ri->contentObject()) {
        d->m_suggestedFilename = static_cast<tdehtml::CachedImage*>(ri->contentObject())->suggestedFilename();
      }
    }
    isImage=true;
  }

  if (hasSelection)
  {
      TDEAction* copyAction = KStdAction::copy( d->m_tdehtml->browserExtension(), TQ_SLOT( copy() ), actionCollection(), "copy" );
      copyAction->setText(i18n("&Copy Text"));
      copyAction->setEnabled(d->m_tdehtml->browserExtension()->isActionEnabled( "copy" ));
      actionCollection()->insert( tdehtml->actionCollection()->action( "selectAll" ) );


      // Fill search provider entries
      TDEConfig config("kuriikwsfilterrc");
      config.setGroup("General");
      const TQString defaultEngine = config.readEntry("DefaultSearchEngine", "google");
      const char keywordDelimiter = config.readNumEntry("KeywordDelimiter", ':');

      // search text
      TQString selectedText = tdehtml->selectedText();
      selectedText.replace("&", "&&");
      if ( selectedText.length()>18 ) {
        selectedText.truncate(15);
        selectedText+="...";
      }

      // default search provider
      KService::Ptr service = KService::serviceByDesktopPath(TQString("searchproviders/%1.desktop").arg(defaultEngine));

      // search provider icon
      TQPixmap icon;
      KURIFilterData data;
      TQStringList list;
      data.setData( TQString("some keyword") );
      list << "kurisearchfilter" << "kuriikwsfilter";

      TQString name;
      if ( KURIFilter::self()->filterURI(data, list) )
      {
        TQString iconPath = locate("cache", KMimeType::favIconForURL(data.uri()) + ".png");
        if ( iconPath.isEmpty() )
          icon = SmallIcon("edit-find");
        else
          icon = TQPixmap( iconPath );
        name = service->name();
      }
      else
      {
        icon = SmallIcon("google");
        name = "Google";
      }

      // using .arg(foo, bar) instead of .arg(foo).arg(bar), as foo can contain %x
      new TDEAction( i18n( "Search for '%1' with %2" ).arg( selectedText, name ), icon, 0, d->m_tdehtml->browserExtension(),
                     TQ_SLOT( searchProvider() ), actionCollection(), "searchProvider" );

      // favorite search providers
      TQStringList favoriteEngines;
      favoriteEngines << "google" << "google_groups" << "google_news" << "webster" << "dmoz" << "wikipedia";
      favoriteEngines = config.readListEntry("FavoriteSearchEngines", favoriteEngines);

      if ( !favoriteEngines.isEmpty()) {
        TDEActionMenu* providerList = new TDEActionMenu( i18n( "Search for '%1' with" ).arg( selectedText ), actionCollection(), "searchProviderList" );

        TQStringList::ConstIterator it = favoriteEngines.begin();
        for ( ; it != favoriteEngines.end(); ++it ) {
          if (*it==defaultEngine)
            continue;
          service = KService::serviceByDesktopPath(TQString("searchproviders/%1.desktop").arg(*it));
          if (!service)
            continue;
          const TQString searchProviderPrefix = *(service->property("Keys").toStringList().begin()) + keywordDelimiter;
          data.setData( searchProviderPrefix + "some keyword" );

          if ( KURIFilter::self()->filterURI(data, list) )
          {
            TQString iconPath = locate("cache", KMimeType::favIconForURL(data.uri()) + ".png");
            if ( iconPath.isEmpty() )
              icon = SmallIcon("edit-find");
            else
              icon = TQPixmap( iconPath );
            name = service->name();

            providerList->insert( new TDEAction( name, icon, 0, d->m_tdehtml->browserExtension(),
              TQ_SLOT( searchProvider() ), actionCollection(), TQString( "searchProvider" + searchProviderPrefix ).latin1() ) );
          }
        }
      }


      if ( selectedText.contains("://") && KURL(selectedText).isValid() )
         new TDEAction( i18n( "Open '%1'" ).arg( selectedText ), "window-new", 0,
         d->m_tdehtml->browserExtension(), TQ_SLOT( openSelection() ), actionCollection(), "openSelection" );
  }
  else if ( url.isEmpty() && !isImage )
  {
      actionCollection()->insert( tdehtml->actionCollection()->action( "security" ) );
      actionCollection()->insert( tdehtml->actionCollection()->action( "setEncoding" ) );
      new TDEAction( i18n( "Stop Animations" ), 0, this, TQ_SLOT( slotStopAnimations() ),
                   actionCollection(), "stopanimations" );
  }

  if ( !url.isEmpty() )
  {
    if (url.protocol() == "mailto")
    {
      new TDEAction( i18n( "Copy Email Address" ), 0, this, TQ_SLOT( slotCopyLinkLocation() ),
                 actionCollection(), "copylinklocation" );
    }
    else
    {
      new TDEAction( i18n( "&Save Link As..." ), 0, this, TQ_SLOT( slotSaveLinkAs() ),
                 actionCollection(), "savelinkas" );
      new TDEAction( i18n( "Copy &Link Address" ), 0, this, TQ_SLOT( slotCopyLinkLocation() ),
                 actionCollection(), "copylinklocation" );
    }
  }

  // frameset? -> add "Reload Frame" etc.
  if (!hasSelection)
  {
    if ( tdehtml->parentPart() )
    {
      new TDEAction( i18n( "Open in New &Window" ), "window-new", 0, this, TQ_SLOT( slotFrameInWindow() ),
                                          actionCollection(), "frameinwindow" );
      new TDEAction( i18n( "Open in &This Window" ), 0, this, TQ_SLOT( slotFrameInTop() ),
                                          actionCollection(), "frameintop" );
      new TDEAction( i18n( "Open in &New Tab" ), "tab_new", 0, this, TQ_SLOT( slotFrameInTab() ),
                                       actionCollection(), "frameintab" );
      new TDEAction( i18n( "Reload Frame" ), 0, this, TQ_SLOT( slotReloadFrame() ),
                                        actionCollection(), "reloadframe" );

      if ( TDEHTMLFactory::defaultHTMLSettings()->isAdFilterEnabled() ) {
          if ( tdehtml->d->m_frame->m_type == tdehtml::ChildFrame::IFrame )
              new TDEAction( i18n( "Block IFrame..." ), 0, this, TQ_SLOT( slotBlockIFrame() ), actionCollection(), "blockiframe" );
      }

      new TDEAction( i18n( "View Frame Source" ), 0, d->m_tdehtml, TQ_SLOT( slotViewDocumentSource() ),
                                          actionCollection(), "viewFrameSource" );
      new TDEAction( i18n( "View Frame Information" ), 0, d->m_tdehtml, TQ_SLOT( slotViewPageInfo() ), actionCollection(), "viewFrameInfo" );
      // This one isn't in tdehtml_popupmenu.rc anymore, because Print isn't either,
      // and because print frame is already in the toolbar and the menu.
      // But leave this here, so that it's easy to read it.
      new TDEAction( i18n( "Print Frame..." ), "frameprint", 0, d->m_tdehtml->browserExtension(), TQ_SLOT( print() ), actionCollection(), "printFrame" );
      new TDEAction( i18n( "Save &Frame As..." ), 0, d->m_tdehtml, TQ_SLOT( slotSaveFrame() ), actionCollection(), "saveFrame" );

      actionCollection()->insert( tdehtml->parentPart()->actionCollection()->action( "viewDocumentSource" ) );
      actionCollection()->insert( tdehtml->parentPart()->actionCollection()->action( "viewPageInfo" ) );
    } else {
      actionCollection()->insert( tdehtml->actionCollection()->action( "viewDocumentSource" ) );
      actionCollection()->insert( tdehtml->actionCollection()->action( "viewPageInfo" ) );
    }
  } else if (isImage || !url.isEmpty()) {
    actionCollection()->insert( tdehtml->actionCollection()->action( "viewDocumentSource" ) );
    actionCollection()->insert( tdehtml->actionCollection()->action( "viewPageInfo" ) );
    new TDEAction( i18n( "Stop Animations" ), 0, this, TQ_SLOT( slotStopAnimations() ),
                 actionCollection(), "stopanimations" );
  }

  if (isImage)
  {
    if ( e.elementId() == ID_IMG ) {
      d->m_imageURL = KURL( static_cast<DOM::HTMLImageElement>( e ).src().string() );
      DOM::HTMLImageElementImpl *imageimpl = static_cast<DOM::HTMLImageElementImpl *>( e.handle() );
      Q_ASSERT(imageimpl);
      if(imageimpl) // should be true always.  right?
      {
        if(imageimpl->complete()) {
	  d->m_pixmap = imageimpl->currentPixmap();
	}
      }
    }
    else
      d->m_imageURL = KURL( static_cast<DOM::HTMLInputElement>( e ).src().string() );
    new TDEAction( i18n( "Save Image As..." ), 0, this, TQ_SLOT( slotSaveImageAs() ),
                 actionCollection(), "saveimageas" );
    new TDEAction( i18n( "Send Image..." ), 0, this, TQ_SLOT( slotSendImage() ),
                 actionCollection(), "sendimage" );


#ifndef TQT_NO_MIMECLIPBOARD
    (new TDEAction( i18n( "Copy Image" ), 0, this, TQ_SLOT( slotCopyImage() ),
                 actionCollection(), "copyimage" ))->setEnabled(!d->m_pixmap.isNull());
#endif

    if(d->m_pixmap.isNull()) {    //fallback to image location if still loading the image.  this will always be true if ifdef TQT_NO_MIMECLIPBOARD
      new TDEAction( i18n( "Copy Image Location" ), 0, this, TQ_SLOT( slotCopyImageLocation() ),
                   actionCollection(), "copyimagelocation" );
    }

    TQString name = KStringHandler::csqueeze(d->m_imageURL.fileName()+d->m_imageURL.query(), 25);
    new TDEAction( i18n( "View Image (%1)" ).arg(d->m_suggestedFilename.isEmpty() ? name.replace("&", "&&") : d->m_suggestedFilename.replace("&", "&&")), 0, this, TQ_SLOT( slotViewImage() ),
                 actionCollection(), "viewimage" );

    if (TDEHTMLFactory::defaultHTMLSettings()->isAdFilterEnabled())
    {
      new TDEAction( i18n( "Block Image..." ), 0, this, TQ_SLOT( slotBlockImage() ),
                   actionCollection(), "blockimage" );

      if (!d->m_imageURL.host().isEmpty() &&
          !d->m_imageURL.protocol().isEmpty())
      {
        new TDEAction( i18n( "Block Images From %1" ).arg(d->m_imageURL.host()), 0, this, TQ_SLOT( slotBlockHost() ),
                     actionCollection(), "blockhost" );
      }
    }
  }

  setXML( doc );
  setDOMDocument( TQDomDocument(), true ); // ### HACK

  TQDomElement menu = domDocument().documentElement().namedItem( "Menu" ).toElement();

  if ( actionCollection()->count() > 0 )
    menu.insertBefore( domDocument().createElement( "separator" ), menu.firstChild() );
}

TDEHTMLPopupGUIClient::~TDEHTMLPopupGUIClient()
{
  delete d;
}

void TDEHTMLPopupGUIClient::slotSaveLinkAs()
{
  TDEIO::MetaData metaData;
  metaData["referrer"] = d->m_tdehtml->referrer();
  saveURL( d->m_tdehtml->widget(), i18n( "Save Link As" ), d->m_url, metaData );
}

void TDEHTMLPopupGUIClient::slotSendImage()
{
    TQStringList urls;
    urls.append( d->m_imageURL.url());
    TQString subject = d->m_imageURL.url();
    kapp->invokeMailer(TQString::null, TQString::null, TQString::null, subject,
                       TQString::null, //body
                       TQString::null,
                       urls); // attachments


}

void TDEHTMLPopupGUIClient::slotSaveImageAs()
{
  TDEIO::MetaData metaData;
  metaData["referrer"] = d->m_tdehtml->referrer();
  saveURL( d->m_tdehtml->widget(), i18n( "Save Image As" ), d->m_imageURL, metaData, TQString::null, 0, d->m_suggestedFilename );
}

void TDEHTMLPopupGUIClient::slotBlockHost()
{
    TQString name=d->m_imageURL.protocol()+"://"+d->m_imageURL.host()+"/*";
    TDEHTMLFactory::defaultHTMLSettings()->addAdFilter( name );
    d->m_tdehtml->reparseConfiguration();
}

void TDEHTMLPopupGUIClient::slotBlockImage()
{
    bool ok = false;

    TQString url = KInputDialog::getText( i18n("Add URL to Filter"),
                                         i18n("Enter the URL:"),
                                         d->m_imageURL.url(),
                                         &ok);
    if ( ok ) {
        TDEHTMLFactory::defaultHTMLSettings()->addAdFilter( url );
        d->m_tdehtml->reparseConfiguration();
    }
}

void TDEHTMLPopupGUIClient::slotBlockIFrame()
{
    bool ok = false;
    TQString url = KInputDialog::getText( i18n( "Add URL to Filter"),
                                               i18n("Enter the URL:"),
                                               d->m_tdehtml->url().url(),
                                               &ok );
    if ( ok ) {
        TDEHTMLFactory::defaultHTMLSettings()->addAdFilter( url );
        d->m_tdehtml->reparseConfiguration();
    }
}

void TDEHTMLPopupGUIClient::slotCopyLinkLocation()
{
  KURL safeURL(d->m_url);
  safeURL.setPass(TQString::null);
#ifndef TQT_NO_MIMECLIPBOARD
  // Set it in both the mouse selection and in the clipboard
  KURL::List lst;
  lst.append( safeURL );
  TQApplication::clipboard()->setData( new KURLDrag( lst ), TQClipboard::Clipboard );
  TQApplication::clipboard()->setData( new KURLDrag( lst ), TQClipboard::Selection );
#else
  TQApplication::clipboard()->setText( safeURL.url() ); //FIXME(E): Handle multiple entries
#endif
}

void TDEHTMLPopupGUIClient::slotStopAnimations()
{
  d->m_tdehtml->stopAnimations();
}

void TDEHTMLPopupGUIClient::slotCopyImage()
{
#ifndef TQT_NO_MIMECLIPBOARD
  KURL safeURL(d->m_imageURL);
  safeURL.setPass(TQString::null);

  KURL::List lst;
  lst.append( safeURL );
  KMultipleDrag *drag = new KMultipleDrag(d->m_tdehtml->view(), "Image");

  drag->addDragObject( new TQImageDrag(d->m_pixmap.convertToImage()) );
  drag->addDragObject( new KURLDrag(lst, d->m_tdehtml->view(), "Image URL") );

  // Set it in both the mouse selection and in the clipboard
  TQApplication::clipboard()->setData( drag, TQClipboard::Clipboard );
  TQApplication::clipboard()->setData( new KURLDrag(lst), TQClipboard::Selection );
#else
  kdDebug() << "slotCopyImage called when the clipboard does not support this.  This should not be possible." << endl;
#endif
}

void TDEHTMLPopupGUIClient::slotCopyImageLocation()
{
  KURL safeURL(d->m_imageURL);
  safeURL.setPass(TQString::null);
#ifndef TQT_NO_MIMECLIPBOARD
  // Set it in both the mouse selection and in the clipboard
  KURL::List lst;
  lst.append( safeURL );
  TQApplication::clipboard()->setData( new KURLDrag( lst ), TQClipboard::Clipboard );
  TQApplication::clipboard()->setData( new KURLDrag( lst ), TQClipboard::Selection );
#else
  TQApplication::clipboard()->setText( safeURL.url() ); //FIXME(E): Handle multiple entries
#endif
}

void TDEHTMLPopupGUIClient::slotViewImage()
{
  d->m_tdehtml->browserExtension()->createNewWindow(d->m_imageURL);
}

void TDEHTMLPopupGUIClient::slotReloadFrame()
{
  KParts::URLArgs args( d->m_tdehtml->browserExtension()->urlArgs() );
  args.reload = true;
  args.metaData()["referrer"] = d->m_tdehtml->pageReferrer();
  // reload document
  d->m_tdehtml->closeURL();
  d->m_tdehtml->browserExtension()->setURLArgs( args );
  d->m_tdehtml->openURL( d->m_tdehtml->url() );
}

void TDEHTMLPopupGUIClient::slotFrameInWindow()
{
  KParts::URLArgs args( d->m_tdehtml->browserExtension()->urlArgs() );
  args.metaData()["referrer"] = d->m_tdehtml->pageReferrer();
  args.metaData()["forcenewwindow"] = "true";
  emit d->m_tdehtml->browserExtension()->createNewWindow( d->m_tdehtml->url(), args );
}

void TDEHTMLPopupGUIClient::slotFrameInTop()
{
  KParts::URLArgs args( d->m_tdehtml->browserExtension()->urlArgs() );
  args.metaData()["referrer"] = d->m_tdehtml->pageReferrer();
  args.frameName = "_top";
  emit d->m_tdehtml->browserExtension()->openURLRequest( d->m_tdehtml->url(), args );
}

void TDEHTMLPopupGUIClient::slotFrameInTab()
{
  KParts::URLArgs args( d->m_tdehtml->browserExtension()->urlArgs() );
  args.metaData()["referrer"] = d->m_tdehtml->pageReferrer();
  args.setNewTab(true);
  emit d->m_tdehtml->browserExtension()->createNewWindow( d->m_tdehtml->url(), args );
}

void TDEHTMLPopupGUIClient::saveURL( TQWidget *parent, const TQString &caption,
                                   const KURL &url,
                                   const TQMap<TQString, TQString> &metadata,
                                   const TQString &filter, long cacheId,
                                   const TQString & suggestedFilename )
{
  TQString name = TQString::fromLatin1( "index.html" );
  if ( !suggestedFilename.isEmpty() )
    name = suggestedFilename;
  else if ( !url.fileName().isEmpty() )
    name = url.fileName();

  KURL destURL;
  int query;
  do {
    query = KMessageBox::Yes;
    destURL = KFileDialog::getSaveURL( name, filter, parent, caption );
      if( destURL.isLocalFile() )
      {
        TQFileInfo info( destURL.path() );
        if( info.exists() ) {
          // TODO: use TDEIO::RenameDlg (shows more information)
          query = KMessageBox::warningContinueCancel( parent, i18n( "A file named \"%1\" already exists. " "Are you sure you want to overwrite it?" ).arg( info.fileName() ), i18n( "Overwrite File?" ), i18n( "Overwrite" ) );
        }
       }
   } while ( query == KMessageBox::Cancel );

  if ( destURL.isValid() )
    saveURL(url, destURL, metadata, cacheId);
}

void TDEHTMLPopupGUIClient::saveURL( const KURL &url, const KURL &destURL,
                                   const TQMap<TQString, TQString> &metadata,
                                   long cacheId )
{
    if ( destURL.isValid() )
    {
        bool saved = false;
        if (TDEHTMLPageCache::self()->isComplete(cacheId))
        {
            if (destURL.isLocalFile())
            {
                KSaveFile destFile(destURL.path());
                if (destFile.status() == 0)
                {
                    TDEHTMLPageCache::self()->saveData(cacheId, destFile.dataStream());
                    saved = true;
                }
            }
            else
            {
                // save to temp file, then move to final destination.
                KTempFile destFile;
                if (destFile.status() == 0)
                {
                    TDEHTMLPageCache::self()->saveData(cacheId, destFile.dataStream());
                    destFile.close();
                    KURL url2 = KURL();
                    url2.setPath(destFile.name());
                    TDEIO::file_move(url2, destURL, -1, true /*overwrite*/);
                    saved = true;
                }
            }
        }
        if(!saved)
        {
          // DownloadManager <-> konqueror integration
          // find if the integration is enabled
          // the empty key  means no integration
          // only use download manager for non-local urls!
          bool downloadViaKIO = true;
          if ( !url.isLocalFile() )
          {
            TDEConfig cfg("konquerorrc", false, false);
            cfg.setGroup("HTML Settings");
            TQString downloadManger = cfg.readPathEntry("DownloadManager");
            if (!downloadManger.isEmpty())
            {
                // then find the download manager location
                kdDebug(1000) << "Using: "<<downloadManger <<" as Download Manager" <<endl;
                TQString cmd = TDEStandardDirs::findExe(downloadManger);
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
                    downloadViaKIO = false;
                    KURL cleanDest = destURL;
                    cleanDest.setPass( TQString::null ); // don't put password into commandline
                    cmd += " " + TDEProcess::quote(url.url()) + " " +
                           TDEProcess::quote(cleanDest.url());
                    kdDebug(1000) << "Calling command  "<<cmd<<endl;
                    KRun::runCommand(cmd);
                }
            }
          }

          if ( downloadViaKIO )
          {
              TDEIO::Job *job = TDEIO::file_copy( url, destURL, -1, true /*overwrite*/ );
              job->setMetaData(metadata);
              job->addMetaData("MaxCacheSize", "0"); // Don't store in http cache.
              job->addMetaData("cache", "cache"); // Use entry from cache if available.
              job->setAutoErrorHandlingEnabled( true );
          }
        } //end if(!saved)
    }
}

TDEHTMLPartBrowserHostExtension::TDEHTMLPartBrowserHostExtension( TDEHTMLPart *part )
: KParts::BrowserHostExtension( part )
{
  m_part = part;
}

TDEHTMLPartBrowserHostExtension::~TDEHTMLPartBrowserHostExtension()
{
}

TQStringList TDEHTMLPartBrowserHostExtension::frameNames() const
{
  return m_part->frameNames();
}

const TQPtrList<KParts::ReadOnlyPart> TDEHTMLPartBrowserHostExtension::frames() const
{
  return m_part->frames();
}

bool TDEHTMLPartBrowserHostExtension::openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs )
{
  return m_part->openURLInFrame( url, urlArgs );
}

void TDEHTMLPartBrowserHostExtension::virtual_hook( int id, void *data )
{
  if (id == VIRTUAL_FIND_FRAME_PARENT)
  {
    FindFrameParentParams *param = static_cast<FindFrameParentParams*>(data);
    TDEHTMLPart *parentPart = m_part->findFrameParent(param->callingPart, param->frame);
    if (parentPart)
       param->parent = parentPart->browserHostExtension();
    return;
  }
  BrowserHostExtension::virtual_hook( id, data );
}


// defined in tdehtml_part.cpp
extern const int TDE_NO_EXPORT fastZoomSizes[];
extern const int TDE_NO_EXPORT fastZoomSizeCount;

// BCI: remove in KDE 4
TDEHTMLZoomFactorAction::TDEHTMLZoomFactorAction( TDEHTMLPart *part, bool direction, const TQString &text, const TQString &icon, const TQObject *receiver, const char *slot, TQObject *parent, const char *name )
    : TDEAction( text, icon, 0, receiver, slot, parent, name )
{
    init(part, direction);
}

TDEHTMLZoomFactorAction::TDEHTMLZoomFactorAction( TDEHTMLPart *part, bool direction, const TQString &text, const TQString &icon, const TDEShortcut &cut, const TQObject *receiver, const char *slot, TQObject *parent, const char *name )
    : TDEAction( text, icon, cut, receiver, slot, parent, name )
{
    init(part, direction);
}

void TDEHTMLZoomFactorAction::init(TDEHTMLPart *part, bool direction)
{
    m_direction = direction;
    m_part = part;

    m_popup = new TQPopupMenu;
    // xgettext: no-c-format
    m_popup->insertItem( i18n( "Default Font Size (100%)" ) );

    int m = m_direction ? 1 : -1;
    int ofs = fastZoomSizeCount / 2;       // take index of 100%

    // this only works if there is an odd number of elements in fastZoomSizes[]
    for ( int i = m; i != m*(ofs+1); i += m )
    {
        int num = i * m;
        TQString numStr = TQString::number( num );
        if ( num > 0 ) numStr.prepend( '+' );

        // xgettext: no-c-format
        m_popup->insertItem( i18n( "%1%" ).arg( fastZoomSizes[ofs + i] ) );
    }

    connect( m_popup, TQ_SIGNAL( activated( int ) ), this, TQ_SLOT( slotActivated( int ) ) );
}

TDEHTMLZoomFactorAction::~TDEHTMLZoomFactorAction()
{
    delete m_popup;
}

int TDEHTMLZoomFactorAction::plug( TQWidget *w, int index )
{
    int containerId = TDEAction::plug( w, index );
    if ( containerId == -1 || !w->inherits( "TDEToolBar" ) )
        return containerId;

    TDEToolBarButton *button = static_cast<TDEToolBar *>( w )->getButton( itemId( containerId ) );
    if ( !button )
        return containerId;

    button->setDelayedPopup( m_popup );
    return containerId;
}

void TDEHTMLZoomFactorAction::slotActivated( int id )
{
    int idx = m_popup->indexOf( id );

    if (idx == 0)
        m_part->setZoomFactor(100);
    else
        m_part->setZoomFactor(fastZoomSizes[fastZoomSizeCount/2 + (m_direction ? 1 : -1)*idx]);
}

#include "tdehtml_ext.moc"

