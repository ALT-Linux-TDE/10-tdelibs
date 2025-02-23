/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999 Lars Knoll <knoll@kde.org>
 *                     1999 Antti Koivisto <koivisto@kde.org>
 *                     2000 Simon Hausmann <hausmann@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 *                     2001-2003 George Staikos <staikos@kde.org>
 *                     2001-2003 Dirk Mueller <mueller@kde.org>
 *                     2000-2005 David Faure <faure@kde.org>
 *                     2002 Apple Computer, Inc.
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

//#define SPEED_DEBUG
#include "tdehtml_part.h"

#include "tdehtml_pagecache.h"

#include "dom/dom_string.h"
#include "dom/dom_element.h"
#include "dom/dom_exception.h"
#include "html/html_documentimpl.h"
#include "html/html_baseimpl.h"
#include "html/html_objectimpl.h"
#include "html/html_miscimpl.h"
#include "html/html_imageimpl.h"
#include "html/html_objectimpl.h"
#include "rendering/render_text.h"
#include "rendering/render_frames.h"
#include "rendering/render_layer.h"
#include "misc/htmlhashes.h"
#include "misc/loader.h"
#include "xml/dom2_eventsimpl.h"
#include "xml/dom2_rangeimpl.h"
#include "xml/xml_tokenizer.h"
#include "css/cssstyleselector.h"
#include "css/csshelper.h"
using namespace DOM;

#include "tdehtmlview.h"
#include <tdeparts/partmanager.h>
#include "ecma/kjs_proxy.h"
#include "ecma/kjs_window.h"
#include "tdehtml_settings.h"
#include "kjserrordlg.h"

#include <kjs/function.h>
#include <kjs/interpreter.h>

#include "htmlpageinfo.h"

#include <sys/types.h>
#include <assert.h>
#include <unistd.h>

#include <config.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>
#include <tdeio/job.h>
#include <tdeio/global.h>
#include <tdeio/netaccess.h>
#include <tdeprotocolmanager.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kcharsets.h>
#include <tdemessagebox.h>
#include <kstdaction.h>
#include <tdefiledialog.h>
#include <ktrader.h>
#include <kdatastream.h>
#include <tdetempfile.h>
#include <tdeglobalsettings.h>
#include <kurldrag.h>
#include <tdeapplication.h>
#include <tdeparts/browserinterface.h>
#if !defined(TQT_NO_DRAGANDDROP)
#include <tdemultipledrag.h>
#endif
#include "../tdeutils/kfinddialog.h"
#include "../tdeutils/kfind.h"

#include <ksslcertchain.h>
#include <ksslinfodlg.h>

#include <tdefileitem.h>
#include <kurifilter.h>
#include <kstatusbar.h>
#include <kurllabel.h>

#include <tqclipboard.h>
#include <tqfile.h>
#include <tqtooltip.h>
#include <tqmetaobject.h>
#include <tqucomextra_p.h>

#include "tdehtmlpart_p.h"
#include "kpassivepopup.h"
#include "tdepopupmenu.h"
#include "rendering/render_form.h"
#include <twin.h>

#define HINT_UTF8	106

namespace tdehtml {
    class PartStyleSheetLoader : public CachedObjectClient
    {
    public:
        PartStyleSheetLoader(TDEHTMLPart *part, DOM::DOMString url, DocLoader* dl)
        {
            m_part = part;
            m_cachedSheet = dl->requestStyleSheet(url, TQString(), "text/css",
                                                  true /* "user sheet" */);
            if (m_cachedSheet)
		m_cachedSheet->ref( this );
        }
        virtual ~PartStyleSheetLoader()
        {
            if ( m_cachedSheet ) m_cachedSheet->deref(this);
        }
        virtual void setStyleSheet(const DOM::DOMString&, const DOM::DOMString &sheet, const DOM::DOMString &)
        {
          if ( m_part )
            m_part->setUserStyleSheet( sheet.string() );

            delete this;
        }
        virtual void error( int, const TQString& ) {
          delete this;
        }
        TQGuardedPtr<TDEHTMLPart> m_part;
        tdehtml::CachedCSSStyleSheet *m_cachedSheet;
    };
}

void tdehtml::ChildFrame::liveConnectEvent(const unsigned long, const TQString & event, const KParts::LiveConnectExtension::ArgList & args)
{
    if (!m_part || !m_frame || !m_liveconnect)
        // hmmm
        return;

    TQString script;
    script.sprintf("%s(", event.latin1());

    KParts::LiveConnectExtension::ArgList::const_iterator i = args.begin();
    const KParts::LiveConnectExtension::ArgList::const_iterator argsBegin = i;
    const KParts::LiveConnectExtension::ArgList::const_iterator argsEnd = args.end();

    for ( ; i != argsEnd; ++i) {
        if (i != argsBegin)
            script += ",";
        if ((*i).first == KParts::LiveConnectExtension::TypeString) {
            script += "\"";
            script += TQString((*i).second).replace('\\', "\\\\").replace('"', "\\\"");
            script += "\"";
        } else
            script += (*i).second;
    }
    script += ")";
    kdDebug(6050) << "tdehtml::ChildFrame::liveConnectEvent " << script << endl;

    TDEHTMLPart * part = ::tqt_cast<TDEHTMLPart *>(m_part->parent());
    if (!part)
        return;
    if (!m_jscript)
        part->framejScript(m_part);
    if (m_jscript) {
        // we have a jscript => a part in an iframe
        KJS::Completion cmp;
        m_jscript->evaluate(TQString(), 1, script, 0L, &cmp);
    } else
        part->executeScript(m_frame->element(), script);
}

TDEHTMLFrameList::Iterator TDEHTMLFrameList::find( const TQString &name )
{
    Iterator it = begin();
    const Iterator e = end();

    for (; it!=e; ++it )
        if ( (*it)->m_name==name )
            break;

    return it;
}

TDEHTMLPart::TDEHTMLPart( TQWidget *parentWidget, const char *widgetname, TQObject *parent, const char *name, GUIProfile prof )
: KParts::ReadOnlyPart( parent, name )
{
    d = 0;
    TDEHTMLFactory::registerPart( this );
    setInstance(  TDEHTMLFactory::instance(), prof == BrowserViewGUI && !parentPart() );
    // TODO KDE4 - don't load plugins yet
    //setInstance( TDEHTMLFactory::instance(), false );
    init( new TDEHTMLView( this, parentWidget, widgetname ), prof );
}

TDEHTMLPart::TDEHTMLPart( TDEHTMLView *view, TQObject *parent, const char *name, GUIProfile prof )
: KParts::ReadOnlyPart( parent, name )
{
    d = 0;
    TDEHTMLFactory::registerPart( this );
    setInstance(  TDEHTMLFactory::instance(), prof == BrowserViewGUI && !parentPart() );
    // TODO KDE4 - don't load plugins yet
    //setInstance( TDEHTMLFactory::instance(), false );
    assert( view );
    init( view, prof );
}

void TDEHTMLPart::init( TDEHTMLView *view, GUIProfile prof )
{
  if ( prof == DefaultGUI )
    setXMLFile( "tdehtml.rc" );
  else if ( prof == BrowserViewGUI )
    setXMLFile( "tdehtml_browser.rc" );

  d = new TDEHTMLPartPrivate(parent());

  d->m_view = view;
  setWidget( d->m_view );

  d->m_guiProfile = prof;
  d->m_extension = new TDEHTMLPartBrowserExtension( this, "TDEHTMLBrowserExtension" );
  d->m_hostExtension = new TDEHTMLPartBrowserHostExtension( this );
  d->m_statusBarExtension = new KParts::StatusBarExtension( this );
  d->m_statusBarIconLabel = 0L;
  d->m_statusBarPopupLabel = 0L;
  d->m_openableSuppressedPopups = 0;

  d->m_bSecurityInQuestion = false;
  d->m_paLoadImages = 0;
  d->m_paDebugScript = 0;
  d->m_bMousePressed = false;
  d->m_bRightMousePressed = false;
  d->m_bCleared = false;
  d->m_paViewDocument = new TDEAction( i18n( "View Do&cument Source" ), CTRL + Key_U, this, TQ_SLOT( slotViewDocumentSource() ), actionCollection(), "viewDocumentSource" );
  d->m_paViewFrame = new TDEAction( i18n( "View Frame Source" ), 0, this, TQ_SLOT( slotViewFrameSource() ), actionCollection(), "viewFrameSource" );
  d->m_paViewInfo = new TDEAction( i18n( "View Document Information" ), CTRL+Key_I, this, TQ_SLOT( slotViewPageInfo() ), actionCollection(), "viewPageInfo" );
  d->m_paSaveBackground = new TDEAction( i18n( "Save &Background Image As..." ), 0, this, TQ_SLOT( slotSaveBackground() ), actionCollection(), "saveBackground" );
  d->m_paSaveDocument = KStdAction::saveAs( this, TQ_SLOT( slotSaveDocument() ), actionCollection(), "saveDocument" );
  if ( parentPart() )
      d->m_paSaveDocument->setShortcut( TDEShortcut() ); // avoid clashes
  d->m_paSaveFrame = new TDEAction( i18n( "Save &Frame As..." ), 0, this, TQ_SLOT( slotSaveFrame() ), actionCollection(), "saveFrame" );
  d->m_paSecurity = new TDEAction( i18n( "Security..." ), "decrypted", 0, this, TQ_SLOT( slotSecurity() ), actionCollection(), "security" );
  d->m_paSecurity->setWhatsThis( i18n( "Security Settings<p>"
                                       "Shows the certificate of the displayed page. Only "
				       "pages that have been transmitted using a secure, encrypted connection have a "
				       "certificate.<p> "
				       "Hint: If the image shows a closed lock, the page has been transmitted over a "
				       "secure connection.") );
  d->m_paDebugRenderTree = new TDEAction( i18n( "Print Rendering Tree to STDOUT" ), ALT + CTRL + SHIFT + Key_A, this, TQ_SLOT( slotDebugRenderTree() ), actionCollection(), "debugRenderTree" );
  d->m_paDebugDOMTree = new TDEAction( i18n( "Print DOM Tree to STDOUT" ), ALT + CTRL + SHIFT + Key_D, this, TQ_SLOT( slotDebugDOMTree() ), actionCollection(), "debugDOMTree" );
  d->m_paStopAnimations = new TDEAction( i18n( "Stop Animated Images" ), 0, this, TQ_SLOT( slotStopAnimations() ), actionCollection(), "stopAnimations" );

  d->m_paSetEncoding = new TDEActionMenu( i18n( "Set &Encoding" ), "charset", actionCollection(), "setEncoding" );
  d->m_paSetEncoding->setDelayed( false );

  d->m_automaticDetection = new TDEPopupMenu( 0L );

  d->m_automaticDetection->insertItem( i18n( "Semi-Automatic" ), 0 );
  d->m_automaticDetection->insertItem( i18n( "Arabic" ), 1 );
  d->m_automaticDetection->insertItem( i18n( "Baltic" ), 2 );
  d->m_automaticDetection->insertItem( i18n( "Central European" ), 3 );
  //d->m_automaticDetection->insertItem( i18n( "Chinese" ), 4 );
  d->m_automaticDetection->insertItem( i18n( "Greek" ), 5 );
  d->m_automaticDetection->insertItem( i18n( "Hebrew" ), 6 );
  d->m_automaticDetection->insertItem( i18n( "Japanese" ), 7 );
  //d->m_automaticDetection->insertItem( i18n( "Korean" ), 8 );
  d->m_automaticDetection->insertItem( i18n( "Russian" ), 9 );
  //d->m_automaticDetection->insertItem( i18n( "Thai" ), 10 );
  d->m_automaticDetection->insertItem( i18n( "Turkish" ), 11 );
  d->m_automaticDetection->insertItem( i18n( "Ukrainian" ), 12 );
  //d->m_automaticDetection->insertItem( i18n( "Unicode" ), 13 );
  d->m_automaticDetection->insertItem( i18n( "Western European" ), 14 );

  connect( d->m_automaticDetection, TQ_SIGNAL( activated( int ) ), this, TQ_SLOT( slotAutomaticDetectionLanguage( int ) ) );

  d->m_paSetEncoding->popupMenu()->insertItem( i18n( "Automatic Detection" ), d->m_automaticDetection, 0 );

  d->m_paSetEncoding->insert( new TDEActionSeparator( actionCollection() ) );


  d->m_manualDetection = new TDESelectAction( i18n( "short for Manual Detection", "Manual" ), 0, this, TQ_SLOT( slotSetEncoding() ), actionCollection(), "manualDetection" );
  TQStringList encodings = TDEGlobal::charsets()->descriptiveEncodingNames();
  d->m_manualDetection->setItems( encodings );
  d->m_manualDetection->setCurrentItem( -1 );
  d->m_paSetEncoding->insert( d->m_manualDetection );


  TDEConfig *config = TDEGlobal::config();
  if ( config->hasGroup( "HTML Settings" ) ) {
    config->setGroup( "HTML Settings" );
    tdehtml::Decoder::AutoDetectLanguage language;
    TQCString name = TQTextCodec::codecForLocale()->name();
    name = name.lower();

    if ( name == "cp1256" || name == "iso-8859-6" ) {
      language = tdehtml::Decoder::Arabic;
    }
    else if ( name == "cp1257" || name == "iso-8859-13" || name == "iso-8859-4" ) {
      language = tdehtml::Decoder::Baltic;
    }
    else if ( name == "cp1250" || name == "ibm852" || name == "iso-8859-2" || name == "iso-8859-3" ) {
      language = tdehtml::Decoder::CentralEuropean;
    }
    else if ( name == "cp1251" || name == "koi8-r" || name == "iso-8859-5" ) {
      language = tdehtml::Decoder::Russian;
    }
    else if ( name == "koi8-u" ) {
      language = tdehtml::Decoder::Ukrainian;
    }
    else if ( name == "cp1253" || name == "iso-8859-7" ) {
      language = tdehtml::Decoder::Greek;
    }
    else if ( name == "cp1255" || name == "iso-8859-8" || name == "iso-8859-8-i" ) {
      language = tdehtml::Decoder::Hebrew;
    }
    else if ( name == "jis7" || name == "eucjp" || name == "sjis"  ) {
      language = tdehtml::Decoder::Japanese;
    }
    else if ( name == "cp1254" || name == "iso-8859-9" ) {
      language = tdehtml::Decoder::Turkish;
    }
    else if ( name == "cp1252" || name == "iso-8859-1" || name == "iso-8859-15" ) {
      language = tdehtml::Decoder::WesternEuropean;
    }
    else
      language = tdehtml::Decoder::SemiautomaticDetection;

    int _id = config->readNumEntry( "AutomaticDetectionLanguage", language );
    d->m_automaticDetection->setItemChecked( _id, true );
    d->m_paSetEncoding->popupMenu()->setItemChecked( 0, true );

    d->m_autoDetectLanguage = static_cast< tdehtml::Decoder::AutoDetectLanguage >( _id );
  }


  d->m_paUseStylesheet = new TDESelectAction( i18n( "Use S&tylesheet"), 0, this, TQ_SLOT( slotUseStylesheet() ), actionCollection(), "useStylesheet" );

  if ( prof == BrowserViewGUI ) {
      d->m_paIncZoomFactor = new TDEHTMLZoomFactorAction( this, true, i18n(
                  "Enlarge Font" ), "zoom-in", "CTRL++;CTRL+=", this,
              TQ_SLOT( slotIncZoomFast() ), actionCollection(), "incFontSizes" );
      d->m_paIncZoomFactor->setWhatsThis( i18n( "Enlarge Font<p>"
                                                "Make the font in this window bigger. "
                            "Click and hold down the mouse button for a menu with all available font sizes." ) );
      d->m_paDecZoomFactor = new TDEHTMLZoomFactorAction( this, false, i18n(
                  "Shrink Font" ), "zoom-out", CTRL + Key_Minus, this,
              TQ_SLOT( slotDecZoomFast() ), actionCollection(), "decFontSizes" );
      d->m_paDecZoomFactor->setWhatsThis( i18n( "Shrink Font<p>"
                                                "Make the font in this window smaller. "
                            "Click and hold down the mouse button for a menu with all available font sizes." ) );
  }

  d->m_paFind = KStdAction::find( this, TQ_SLOT( slotFind() ), actionCollection(), "find" );
  d->m_paFind->setWhatsThis( i18n( "Find text<p>"
				   "Shows a dialog that allows you to find text on the displayed page." ) );

  d->m_paFindNext = KStdAction::findNext( this, TQ_SLOT( slotFindNext() ), actionCollection(), "findNext" );
  d->m_paFindNext->setWhatsThis( i18n( "Find next<p>"
				       "Find the next occurrence of the text that you "
				       "have found using the <b>Find Text</b> function" ) );

  d->m_paFindPrev = KStdAction::findPrev( this, TQ_SLOT( slotFindPrev() ), actionCollection(), "findPrevious" );
  d->m_paFindPrev->setWhatsThis( i18n( "Find previous<p>"
				       "Find the previous occurrence of the text that you "
				       "have found using the <b>Find Text</b> function" ) );

  d->m_paFindAheadText = new TDEAction( i18n("Find Text as You Type"), TDEShortcut( '/' ), this, TQ_SLOT( slotFindAheadText()),
      actionCollection(), "findAheadText");
  d->m_paFindAheadLinks = new TDEAction( i18n("Find Links as You Type"), TDEShortcut( '\'' ), this, TQ_SLOT( slotFindAheadLink()),
      actionCollection(), "findAheadLink");
  d->m_paFindAheadText->setEnabled( false );
  d->m_paFindAheadLinks->setEnabled( false );

  if ( parentPart() )
  {
      d->m_paFind->setShortcut( TDEShortcut() ); // avoid clashes
      d->m_paFindNext->setShortcut( TDEShortcut() ); // avoid clashes
      d->m_paFindPrev->setShortcut( TDEShortcut() ); // avoid clashes
      d->m_paFindAheadText->setShortcut( TDEShortcut());
      d->m_paFindAheadLinks->setShortcut( TDEShortcut());
  }

  d->m_paPrintFrame = new TDEAction( i18n( "Print Frame..." ), "frameprint", 0, this, TQ_SLOT( slotPrintFrame() ), actionCollection(), "printFrame" );
  d->m_paPrintFrame->setWhatsThis( i18n( "Print Frame<p>"
					 "Some pages have several frames. To print only a single frame, click "
					 "on it and then use this function." ) );

  d->m_paSelectAll = KStdAction::selectAll( this, TQ_SLOT( slotSelectAll() ), actionCollection(), "selectAll" );
  if ( parentPart() )
      d->m_paSelectAll->setShortcut( TDEShortcut() ); // avoid clashes

  d->m_paToggleCaretMode = new TDEToggleAction(i18n("Toggle Caret Mode"),
  				Key_F7, this, TQ_SLOT(slotToggleCaretMode()),
                                actionCollection(), "caretMode");
  d->m_paToggleCaretMode->setChecked(isCaretMode());
  if (parentPart())
      d->m_paToggleCaretMode->setShortcut(TDEShortcut()); // avoid clashes

  // set the default java(script) flags according to the current host.
  d->m_bOpenMiddleClick = d->m_settings->isOpenMiddleClickEnabled();
  d->m_bBackRightClick = d->m_settings->isBackRightClickEnabled();
  d->m_bJScriptEnabled = d->m_settings->isJavaScriptEnabled();
  setDebugScript( d->m_settings->isJavaScriptDebugEnabled() );
  d->m_bJavaEnabled = d->m_settings->isJavaEnabled();
  d->m_bPluginsEnabled = d->m_settings->isPluginsEnabled();

  // Set the meta-refresh flag...
  d->m_metaRefreshEnabled = d->m_settings->isAutoDelayedActionsEnabled ();

  connect( view, TQ_SIGNAL( zoomView( int ) ), TQ_SLOT( slotZoomView( int ) ) );

  connect( this, TQ_SIGNAL( completed() ),
           this, TQ_SLOT( updateActions() ) );
  connect( this, TQ_SIGNAL( completed( bool ) ),
           this, TQ_SLOT( updateActions() ) );
  connect( this, TQ_SIGNAL( started( TDEIO::Job * ) ),
           this, TQ_SLOT( updateActions() ) );

  d->m_popupMenuXML = KXMLGUIFactory::readConfigFile( locate( "data", "tdehtml/tdehtml_popupmenu.rc", TDEHTMLFactory::instance() ) );

  connect( tdehtml::Cache::loader(), TQ_SIGNAL( requestStarted( tdehtml::DocLoader*, tdehtml::CachedObject* ) ),
           this, TQ_SLOT( slotLoaderRequestStarted( tdehtml::DocLoader*, tdehtml::CachedObject* ) ) );
  connect( tdehtml::Cache::loader(), TQ_SIGNAL( requestDone( tdehtml::DocLoader*, tdehtml::CachedObject *) ),
           this, TQ_SLOT( slotLoaderRequestDone( tdehtml::DocLoader*, tdehtml::CachedObject *) ) );
  connect( tdehtml::Cache::loader(), TQ_SIGNAL( requestFailed( tdehtml::DocLoader*, tdehtml::CachedObject *) ),
           this, TQ_SLOT( slotLoaderRequestDone( tdehtml::DocLoader*, tdehtml::CachedObject *) ) );

  connect ( &d->m_progressUpdateTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( slotProgressUpdate() ) );

  findTextBegin(); //reset find variables

  connect( &d->m_redirectionTimer, TQ_SIGNAL( timeout() ),
           this, TQ_SLOT( slotRedirect() ) );

  d->m_dcopobject = new TDEHTMLPartIface(this);

  // TODO KDE4 - load plugins now (see also the constructors)
  //if ( prof == BrowserViewGUI && !parentPart() )
  //        loadPlugins( partObject(), this, instance() );

  // "tdehtml" catalog does not exist, our translations are in tdelibs.
  // removing this catalog from TDEGlobal::locale() prevents problems
  // with changing the language in applications at runtime -Thomas Reitelbach
  TDEGlobal::locale()->removeCatalogue("tdehtml");
}

TDEHTMLPart::~TDEHTMLPart()
{
  //kdDebug(6050) << "TDEHTMLPart::~TDEHTMLPart " << this << endl;

  TDEConfig *config = TDEGlobal::config();
  config->setGroup( "HTML Settings" );
  config->writeEntry( "AutomaticDetectionLanguage", d->m_autoDetectLanguage );

  delete d->m_automaticDetection;
  delete d->m_manualDetection;

  slotWalletClosed();
  if (!parentPart()) { // only delete it if the top tdehtml_part closes
    removeJSErrorExtension();
    delete d->m_statusBarPopupLabel;
  }

  d->m_find = 0; // deleted by its parent, the view.

  if ( d->m_manager )
  {
    d->m_manager->setActivePart( 0 );
    // We specify "this" as parent qobject for d->manager, so no need to delete it.
  }

  stopAutoScroll();
  d->m_redirectionTimer.stop();

  if (!d->m_bComplete)
    closeURL();

  disconnect( tdehtml::Cache::loader(), TQ_SIGNAL( requestStarted( tdehtml::DocLoader*, tdehtml::CachedObject* ) ),
           this, TQ_SLOT( slotLoaderRequestStarted( tdehtml::DocLoader*, tdehtml::CachedObject* ) ) );
  disconnect( tdehtml::Cache::loader(), TQ_SIGNAL( requestDone( tdehtml::DocLoader*, tdehtml::CachedObject *) ),
           this, TQ_SLOT( slotLoaderRequestDone( tdehtml::DocLoader*, tdehtml::CachedObject *) ) );
  disconnect( tdehtml::Cache::loader(), TQ_SIGNAL( requestFailed( tdehtml::DocLoader*, tdehtml::CachedObject *) ),
           this, TQ_SLOT( slotLoaderRequestDone( tdehtml::DocLoader*, tdehtml::CachedObject *) ) );

  clear();

  if ( d->m_view )
  {
    d->m_view->hide();
    d->m_view->viewport()->hide();
    d->m_view->m_part = 0;
  }

  // Have to delete this here since we forward declare it in tdehtmlpart_p and
  // at least some compilers won't call the destructor in this case.
  delete d->m_jsedlg;
  d->m_jsedlg = 0;

  if (!parentPart()) // only delete d->m_frame if the top tdehtml_part closes
      delete d->m_frame;
  delete d; d = 0;
  TDEHTMLFactory::deregisterPart( this );
}

bool TDEHTMLPart::restoreURL( const KURL &url )
{
  kdDebug( 6050 ) << "TDEHTMLPart::restoreURL " << url.url() << endl;

  d->m_redirectionTimer.stop();

  /*
   * That's not a good idea as it will call closeURL() on all
   * child frames, preventing them from further loading. This
   * method gets called from restoreState() in case of a full frameset
   * restoral, and restoreState() calls closeURL() before restoring
   * anyway.
  kdDebug( 6050 ) << "closing old URL" << endl;
  closeURL();
  */

  d->m_bComplete = false;
  d->m_bLoadEventEmitted = false;
  d->m_workingURL = url;

  // set the java(script) flags according to the current host.
  d->m_bJScriptEnabled = TDEHTMLFactory::defaultHTMLSettings()->isJavaScriptEnabled(url.host());
  setDebugScript( TDEHTMLFactory::defaultHTMLSettings()->isJavaScriptDebugEnabled() );
  d->m_bJavaEnabled = TDEHTMLFactory::defaultHTMLSettings()->isJavaEnabled(url.host());
  d->m_bPluginsEnabled = TDEHTMLFactory::defaultHTMLSettings()->isPluginsEnabled(url.host());

  m_url = url;

  d->m_restoreScrollPosition = true;
  disconnect(d->m_view, TQ_SIGNAL(finishedLayout()), this, TQ_SLOT(restoreScrollPosition()));
  connect(d->m_view, TQ_SIGNAL(finishedLayout()), this, TQ_SLOT(restoreScrollPosition()));

  TDEHTMLPageCache::self()->fetchData( d->m_cacheId, this, TQ_SLOT(slotRestoreData(const TQByteArray &)));

  emit started( 0L );

  return true;
}


bool TDEHTMLPart::openURL( const KURL &url )
{
  kdDebug( 6050 ) << "TDEHTMLPart(" << this << ")::openURL " << url.url() << endl;

  d->m_redirectionTimer.stop();

  // check to see if this is an "error://" URL. This is caused when an error
  // occurs before this part was loaded (e.g. KonqRun), and is passed to
  // tdehtmlpart so that it can display the error.
  if ( url.protocol() == "error" && url.hasSubURL() ) {
    closeURL();

    if(  d->m_bJScriptEnabled )
      d->m_statusBarText[BarOverrideText] = d->m_statusBarText[BarDefaultText] = TQString();

    /**
     * The format of the error url is that two variables are passed in the query:
     * error = int tdeio error code, errText = TQString error text from tdeio
     * and the URL where the error happened is passed as a sub URL.
     */
    KURL::List urls = KURL::split( url );
    //kdDebug(6050) << "Handling error URL. URL count:" << urls.count() << endl;

    if ( urls.count() > 1 ) {
      KURL mainURL = urls.first();
      int error = mainURL.queryItem( "error" ).toInt();
      // error=0 isn't a valid error code, so 0 means it's missing from the URL
      if ( error == 0 ) error = TDEIO::ERR_UNKNOWN;
      TQString errorText = mainURL.queryItem( "errText", HINT_UTF8 );
      urls.pop_front();
      d->m_workingURL = KURL::join( urls );
      //kdDebug(6050) << "Emitting fixed URL " << d->m_workingURL.prettyURL() << endl;
      emit d->m_extension->setLocationBarURL( d->m_workingURL.prettyURL() );
      htmlError( error, errorText, d->m_workingURL );
      return true;
    }
  }

  if (!parentPart()) { // only do it for toplevel part
    TQString host = url.isLocalFile() ? "localhost" : url.host();
    TQString userAgent = KProtocolManager::userAgentForHost(host);
    if (userAgent != KProtocolManager::userAgentForHost(TQString())) {
      if (!d->m_statusBarUALabel) {
        d->m_statusBarUALabel = new KURLLabel(d->m_statusBarExtension->statusBar());
        d->m_statusBarUALabel->setFixedHeight(instance()->iconLoader()->currentSize(TDEIcon::Small));
        d->m_statusBarUALabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed, TQSizePolicy::Fixed));
        d->m_statusBarUALabel->setUseCursor(false);
        d->m_statusBarExtension->addStatusBarItem(d->m_statusBarUALabel, 0, false);
        d->m_statusBarUALabel->setPixmap(SmallIcon("agent", instance()));
      } else {
        TQToolTip::remove(d->m_statusBarUALabel);
      }
      TQToolTip::add(d->m_statusBarUALabel, i18n("The fake user-agent '%1' is in use.").arg(userAgent));
    } else if (d->m_statusBarUALabel) {
      d->m_statusBarExtension->removeStatusBarItem(d->m_statusBarUALabel);
      delete d->m_statusBarUALabel;
      d->m_statusBarUALabel = 0L;
    }
  }

  KParts::URLArgs args( d->m_extension->urlArgs() );

  // in case
  // a) we have no frameset (don't test m_frames.count(), iframes get in there)
  // b) the url is identical with the currently displayed one (except for the htmlref!)
  // c) the url request is not a POST operation and
  // d) the caller did not request to reload the page
  // e) there was no HTTP redirection meanwhile (testcase: webmin's software/tree.cgi)
  // => we don't reload the whole document and
  // we just jump to the requested html anchor
  bool isFrameSet = false;
  if ( d->m_doc && d->m_doc->isHTMLDocument() ) {
      HTMLDocumentImpl* htmlDoc = static_cast<HTMLDocumentImpl*>(d->m_doc);
      isFrameSet = htmlDoc->body() && (htmlDoc->body()->id() == ID_FRAMESET);
  }

  if ( url.hasRef() && !isFrameSet )
  {
    bool noReloadForced = !args.reload && !args.redirectedRequest() && !args.doPost();
    if (noReloadForced && urlcmp( url.url(), m_url.url(), true, true ))
    {
        kdDebug( 6050 ) << "TDEHTMLPart::openURL, jumping to anchor. m_url = " << url.url() << endl;
        m_url = url;
        emit started( 0L );

        if ( !gotoAnchor( url.encodedHtmlRef()) )
          gotoAnchor( url.htmlRef() );

        d->m_bComplete = true;
        if (d->m_doc)
        d->m_doc->setParsing(false);

        kdDebug( 6050 ) << "completed..." << endl;
        emit completed();
        return true;
    }
  }

  // Save offset of viewport when page is reloaded to be compliant
  // to every other capable browser out there.
  if (args.reload) {
    args.xOffset = d->m_view->contentsX();
    args.yOffset = d->m_view->contentsY();
    d->m_extension->setURLArgs(args);
  }

  if (!d->m_restored)
    closeURL();

  d->m_restoreScrollPosition = d->m_restored;
  disconnect(d->m_view, TQ_SIGNAL(finishedLayout()), this, TQ_SLOT(restoreScrollPosition()));
  connect(d->m_view, TQ_SIGNAL(finishedLayout()), this, TQ_SLOT(restoreScrollPosition()));

  // initializing m_url to the new url breaks relative links when opening such a link after this call and _before_ begin() is called (when the first
  // data arrives) (Simon)
  m_url = url;
  if(m_url.protocol().startsWith( "http" ) && !m_url.host().isEmpty() &&
     m_url.path().isEmpty()) {
    m_url.setPath("/");
    emit d->m_extension->setLocationBarURL( m_url.prettyURL() );
  }
  // copy to m_workingURL after fixing m_url above
  d->m_workingURL = m_url;

  args.metaData().insert("main_frame_request", parentPart() == 0 ? "TRUE" : "FALSE" );
  args.metaData().insert("ssl_parent_ip", d->m_ssl_parent_ip);
  args.metaData().insert("ssl_parent_cert", d->m_ssl_parent_cert);
  args.metaData().insert("PropagateHttpHeader", "true");
  args.metaData().insert("ssl_was_in_use", d->m_ssl_in_use ? "TRUE" : "FALSE" );
  args.metaData().insert("ssl_activate_warnings", "TRUE" );
  args.metaData().insert("cross-domain", toplevelURL().url());

  if (d->m_restored)
  {
     args.metaData().insert("referrer", d->m_pageReferrer);
     d->m_cachePolicy = TDEIO::CC_Cache;
  }
  else if (args.reload)
     d->m_cachePolicy = TDEIO::CC_Reload;
  else
     d->m_cachePolicy = KProtocolManager::cacheControl();

  if ( args.doPost() && (m_url.protocol().startsWith("http")) )
  {
      d->m_job = TDEIO::http_post( m_url, args.postData, false );
      d->m_job->addMetaData("content-type", args.contentType() );
  }
  else
  {
      d->m_job = TDEIO::get( m_url, false, false );
      d->m_job->addMetaData("cache", TDEIO::getCacheControlString(d->m_cachePolicy));
  }

  if (widget())
     d->m_job->setWindow(widget()->topLevelWidget());
  d->m_job->addMetaData(args.metaData());

  connect( d->m_job, TQ_SIGNAL( result( TDEIO::Job* ) ),
           TQ_SLOT( slotFinished( TDEIO::Job* ) ) );
  connect( d->m_job, TQ_SIGNAL( data( TDEIO::Job*, const TQByteArray& ) ),
           TQ_SLOT( slotData( TDEIO::Job*, const TQByteArray& ) ) );
  connect ( d->m_job, TQ_SIGNAL( infoMessage( TDEIO::Job*, const TQString& ) ),
           TQ_SLOT( slotInfoMessage(TDEIO::Job*, const TQString& ) ) );
  connect( d->m_job, TQ_SIGNAL(redirection(TDEIO::Job*, const KURL& ) ),
           TQ_SLOT( slotRedirection(TDEIO::Job*, const KURL&) ) );

  d->m_bComplete = false;
  d->m_bLoadEventEmitted = false;

  // delete old status bar msg's from kjs (if it _was_ activated on last URL)
  if( d->m_bJScriptEnabled )
    d->m_statusBarText[BarOverrideText] = d->m_statusBarText[BarDefaultText] = TQString();

  // set the javascript flags according to the current url
  d->m_bJScriptEnabled = TDEHTMLFactory::defaultHTMLSettings()->isJavaScriptEnabled(url.host());
  setDebugScript( TDEHTMLFactory::defaultHTMLSettings()->isJavaScriptDebugEnabled() );
  d->m_bJavaEnabled = TDEHTMLFactory::defaultHTMLSettings()->isJavaEnabled(url.host());
  d->m_bPluginsEnabled = TDEHTMLFactory::defaultHTMLSettings()->isPluginsEnabled(url.host());


  connect( d->m_job, TQ_SIGNAL( speed( TDEIO::Job*, unsigned long ) ),
           this, TQ_SLOT( slotJobSpeed( TDEIO::Job*, unsigned long ) ) );

  connect( d->m_job, TQ_SIGNAL( percent( TDEIO::Job*, unsigned long ) ),
           this, TQ_SLOT( slotJobPercent( TDEIO::Job*, unsigned long ) ) );

  connect( d->m_job, TQ_SIGNAL( result( TDEIO::Job* ) ),
           this, TQ_SLOT( slotJobDone( TDEIO::Job* ) ) );

  d->m_jobspeed = 0;

  // If this was an explicit reload and the user style sheet should be used,
  // do a stat to see whether the stylesheet was changed in the meanwhile.
  if ( args.reload && !settings()->userStyleSheet().isEmpty() ) {
    KURL url( settings()->userStyleSheet() );
    TDEIO::StatJob *job = TDEIO::stat( url, false /* don't show progress */ );
    connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
             this, TQ_SLOT( slotUserSheetStatDone( TDEIO::Job * ) ) );
  }
  emit started( 0L );

  return true;
}

bool TDEHTMLPart::closeURL()
{
  if ( d->m_job )
  {
    TDEHTMLPageCache::self()->cancelEntry(d->m_cacheId);
    d->m_job->kill();
    d->m_job = 0;
  }

  if ( d->m_doc && d->m_doc->isHTMLDocument() ) {
    HTMLDocumentImpl* hdoc = static_cast<HTMLDocumentImpl*>( d->m_doc );

    if ( hdoc->body() && d->m_bLoadEventEmitted ) {
      hdoc->body()->dispatchWindowEvent( EventImpl::UNLOAD_EVENT, false, false );
      if ( d->m_doc )
        d->m_doc->updateRendering();
      d->m_bLoadEventEmitted = false;
    }
  }

  d->m_bComplete = true; // to avoid emitting completed() in slotFinishedParsing() (David)
  d->m_bLoadEventEmitted = true; // don't want that one either
  d->m_cachePolicy = KProtocolManager::cacheControl(); // reset cache policy

  disconnect(d->m_view, TQ_SIGNAL(finishedLayout()), this, TQ_SLOT(restoreScrollPosition()));

  TDEHTMLPageCache::self()->cancelFetch(this);
  if ( d->m_doc && d->m_doc->parsing() )
  {
    kdDebug( 6050 ) << " was still parsing... calling end " << endl;
    slotFinishedParsing();
    d->m_doc->setParsing(false);
  }

  if ( !d->m_workingURL.isEmpty() )
  {
    // Aborted before starting to render
    kdDebug( 6050 ) << "Aborted before starting to render, reverting location bar to " << m_url.prettyURL() << endl;
    emit d->m_extension->setLocationBarURL( m_url.prettyURL() );
  }

  d->m_workingURL = KURL();

  if ( d->m_doc && d->m_doc->docLoader() )
    tdehtml::Cache::loader()->cancelRequests( d->m_doc->docLoader() );

  // tell all subframes to stop as well
  {
    ConstFrameIt it = d->m_frames.begin();
    const ConstFrameIt end = d->m_frames.end();
    for (; it != end; ++it )
    {
      if ( (*it)->m_run )
        (*it)->m_run->abort();
      if ( !( *it )->m_part.isNull() )
        ( *it )->m_part->closeURL();
    }
  }
  // tell all objects to stop as well
  {
    ConstFrameIt it = d->m_objects.begin();
    const ConstFrameIt end = d->m_objects.end();
    for (; it != end; ++it)
    {
      if ( !( *it )->m_part.isNull() )
        ( *it )->m_part->closeURL();
    }
  }
  // Stop any started redirections as well!! (DA)
  if ( d && d->m_redirectionTimer.isActive() )
    d->m_redirectionTimer.stop();

  // null node activated.
  emit nodeActivated(Node());

  // make sure before clear() runs, we pop out of a dialog's message loop
  if ( d->m_view )
    d->m_view->closeChildDialogs();

  return true;
}

DOM::HTMLDocument TDEHTMLPart::htmlDocument() const
{
  if (d->m_doc && d->m_doc->isHTMLDocument())
    return static_cast<HTMLDocumentImpl*>(d->m_doc);
  else
    return static_cast<HTMLDocumentImpl*>(0);
}

DOM::Document TDEHTMLPart::document() const
{
    return d->m_doc;
}

TQString TDEHTMLPart::documentSource() const
{
  TQString sourceStr;
  if ( !( m_url.isLocalFile() ) && TDEHTMLPageCache::self()->isComplete( d->m_cacheId ) )
  {
     TQByteArray sourceArray;
     TQDataStream dataStream( sourceArray, IO_WriteOnly );
     TDEHTMLPageCache::self()->saveData( d->m_cacheId, &dataStream );
     TQTextStream stream( sourceArray, IO_ReadOnly );
     stream.setCodec( TQTextCodec::codecForName( encoding().latin1() ) );
     sourceStr = stream.read();
  } else
  {
    TQString tmpFile;
    if( TDEIO::NetAccess::download( m_url, tmpFile, NULL ) )
    {
      TQFile f( tmpFile );
      if ( f.open( IO_ReadOnly ) )
      {
        TQTextStream stream( &f );
        stream.setCodec( TQTextCodec::codecForName( encoding().latin1() ) );
	sourceStr = stream.read();
        f.close();
      }
      TDEIO::NetAccess::removeTempFile( tmpFile );
    }
  }

  return sourceStr;
}


KParts::BrowserExtension *TDEHTMLPart::browserExtension() const
{
  return d->m_extension;
}

KParts::BrowserHostExtension *TDEHTMLPart::browserHostExtension() const
{
  return d->m_hostExtension;
}

TDEHTMLView *TDEHTMLPart::view() const
{
  return d->m_view;
}

void TDEHTMLPart::setStatusMessagesEnabled( bool enable )
{
  d->m_statusMessagesEnabled = enable;
}

KJS::Interpreter *TDEHTMLPart::jScriptInterpreter()
{
  KJSProxy *proxy = jScript();
  if (!proxy || proxy->paused())
    return 0;

  return proxy->interpreter();
}

bool TDEHTMLPart::statusMessagesEnabled() const
{
  return d->m_statusMessagesEnabled;
}

void TDEHTMLPart::setJScriptEnabled( bool enable )
{
  if ( !enable && jScriptEnabled() && d->m_frame && d->m_frame->m_jscript ) {
    d->m_frame->m_jscript->clear();
  }
  d->m_bJScriptForce = enable;
  d->m_bJScriptOverride = true;
}

bool TDEHTMLPart::jScriptEnabled() const
{
  if(onlyLocalReferences()) return false;

  if ( d->m_bJScriptOverride )
      return d->m_bJScriptForce;
  return d->m_bJScriptEnabled;
}

void TDEHTMLPart::setMetaRefreshEnabled( bool enable )
{
  d->m_metaRefreshEnabled = enable;
}

bool TDEHTMLPart::metaRefreshEnabled() const
{
  return d->m_metaRefreshEnabled;
}

// Define this to disable dlopening kjs_html, when directly linking to it.
// You need to edit tdehtml/Makefile.am to add ./ecma/libkjs_html.la to LIBADD
// and to edit tdehtml/ecma/Makefile.am to s/kjs_html/libkjs_html/, remove libtdehtml from LIBADD,
//        remove LDFLAGS line, and replace kde_module with either lib (shared) or noinst (static)
//        Also, change the order of "ecma" and "." in tdehtml's SUBDIRS line.
// OK - that's the default now, use the opposite of the above instructions to go back
// to "dlopening it" - but it breaks exception catching in kjs_binding.cpp
#define DIRECT_LINKAGE_TO_ECMA

#ifdef DIRECT_LINKAGE_TO_ECMA
extern "C" { KJSProxy *kjs_html_init(tdehtml::ChildFrame * childframe); }
#endif

static bool createJScript(tdehtml::ChildFrame *frame)
{
#ifndef DIRECT_LINKAGE_TO_ECMA
  KLibrary *lib = KLibLoader::self()->library("kjs_html");
  if ( !lib ) {
    setJScriptEnabled( false );
    return false;
  }
  // look for plain C init function
  void *sym = lib->symbol("kjs_html_init");
  if ( !sym ) {
    lib->unload();
    setJScriptEnabled( false );
    return false;
  }
  typedef KJSProxy* (*initFunction)(tdehtml::ChildFrame *);
  initFunction initSym = (initFunction) sym;
  frame->m_jscript = (*initSym)(d->m_frame);
  frame->m_kjs_lib = lib;
#else
  frame->m_jscript = kjs_html_init(frame);
  // frame->m_kjs_lib remains 0L.
#endif
  return true;
}

KJSProxy *TDEHTMLPart::jScript()
{
  if (!jScriptEnabled()) return 0;

  if ( !d->m_frame ) {
      TDEHTMLPart * p = parentPart();
      if (!p) {
          d->m_frame = new tdehtml::ChildFrame;
          d->m_frame->m_part = this;
      } else {
          ConstFrameIt it = p->d->m_frames.begin();
          const ConstFrameIt end = p->d->m_frames.end();
          for (; it != end; ++it)
              if ((*it)->m_part.operator->() == this) {
                  d->m_frame = *it;
                  break;
              }
      }
      if ( !d->m_frame )
        return 0;
  }
  if ( !d->m_frame->m_jscript )
    if (!createJScript(d->m_frame))
      return 0;
  if (d->m_bJScriptDebugEnabled)
    d->m_frame->m_jscript->setDebugEnabled(true);

  return d->m_frame->m_jscript;
}

TQVariant TDEHTMLPart::crossFrameExecuteScript(const TQString& target,  const TQString& script)
{
  TDEHTMLPart* destpart = this;

  TQString trg = target.lower();

  if (target == "_top") {
    while (destpart->parentPart())
      destpart = destpart->parentPart();
  }
  else if (target == "_parent") {
    if (parentPart())
      destpart = parentPart();
  }
  else if (target == "_self" || target == "_blank")  {
    // we always allow these
  }
  else {
    destpart = findFrame(target);
    if (!destpart)
       destpart = this;
  }

  // easy way out?
  if (destpart == this)
    return executeScript(DOM::Node(), script);

  // now compare the domains
  if (destpart->checkFrameAccess(this))
    return destpart->executeScript(DOM::Node(), script);

  // eww, something went wrong. better execute it in our frame
  return executeScript(DOM::Node(), script);
}

//Enable this to see all JS scripts being executed
//#define KJS_VERBOSE

KJSErrorDlg *TDEHTMLPart::jsErrorExtension() {
  if (!d->m_settings->jsErrorsEnabled()) {
    return 0L;
  }

  if (parentPart()) {
    return parentPart()->jsErrorExtension();
  }

  if (!d->m_statusBarJSErrorLabel) {
    d->m_statusBarJSErrorLabel = new KURLLabel(d->m_statusBarExtension->statusBar());
    d->m_statusBarJSErrorLabel->setFixedHeight(instance()->iconLoader()->currentSize(TDEIcon::Small));
    d->m_statusBarJSErrorLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed, TQSizePolicy::Fixed));
    d->m_statusBarJSErrorLabel->setUseCursor(false);
    d->m_statusBarExtension->addStatusBarItem(d->m_statusBarJSErrorLabel, 0, false);
    TQToolTip::add(d->m_statusBarJSErrorLabel, i18n("This web page contains coding errors."));
    d->m_statusBarJSErrorLabel->setPixmap(SmallIcon("bug", instance()));
    connect(d->m_statusBarJSErrorLabel, TQ_SIGNAL(leftClickedURL()), TQ_SLOT(launchJSErrorDialog()));
    connect(d->m_statusBarJSErrorLabel, TQ_SIGNAL(rightClickedURL()), TQ_SLOT(jsErrorDialogContextMenu()));
  }
  if (!d->m_jsedlg) {
    d->m_jsedlg = new KJSErrorDlg;
    d->m_jsedlg->setURL(m_url.prettyURL());
    if (TDEGlobalSettings::showIconsOnPushButtons()) {
      d->m_jsedlg->_clear->setIconSet(SmallIconSet("locationbar_erase"));
      d->m_jsedlg->_close->setIconSet(SmallIconSet("window-close"));
    }
  }
  return d->m_jsedlg;
}

void TDEHTMLPart::removeJSErrorExtension() {
  if (parentPart()) {
    parentPart()->removeJSErrorExtension();
    return;
  }
  if (d->m_statusBarJSErrorLabel != 0) {
    d->m_statusBarExtension->removeStatusBarItem( d->m_statusBarJSErrorLabel );
    delete d->m_statusBarJSErrorLabel;
    d->m_statusBarJSErrorLabel = 0;
  }
  delete d->m_jsedlg;
  d->m_jsedlg = 0;
}

void TDEHTMLPart::disableJSErrorExtension() {
  removeJSErrorExtension();
  // These two lines are really kind of hacky, and it sucks to do this inside
  // TDEHTML but I don't know of anything that's reasonably easy as an alternative
  // right now.  It makes me wonder if there should be a more clean way to
  // contact all running "TDEHTML" instance as opposed to Konqueror instances too.
  d->m_settings->setJSErrorsEnabled(false);
  DCOPClient::mainClient()->send("konqueror*", "KonquerorIface", "reparseConfiguration()", TQByteArray());
}

void TDEHTMLPart::jsErrorDialogContextMenu() {
  TDEPopupMenu *m = new TDEPopupMenu(0L);
  m->insertItem(i18n("&Hide Errors"), this, TQ_SLOT(removeJSErrorExtension()));
  m->insertItem(i18n("&Disable Error Reporting"), this, TQ_SLOT(disableJSErrorExtension()));
  m->popup(TQCursor::pos());
}

void TDEHTMLPart::launchJSErrorDialog() {
  KJSErrorDlg *dlg = jsErrorExtension();
  if (dlg) {
    dlg->show();
    dlg->raise();
  }
}

void TDEHTMLPart::launchJSConfigDialog() {
  TQStringList args;
  args << "tdehtml_java_js";
  TDEApplication::tdeinitExec( "tdecmshell", args );
}

TQVariant TDEHTMLPart::executeScript(const TQString& filename, int baseLine, const DOM::Node& n, const TQString& script)
{
#ifdef KJS_VERBOSE
  // The script is now printed by KJS's Parser::parse
  kdDebug(6070) << "executeScript: caller='" << name() << "' filename=" << filename << " baseLine=" << baseLine /*<< " script=" << script*/ << endl;
#endif
  KJSProxy *proxy = jScript();

  if (!proxy || proxy->paused())
    return TQVariant();

  KJS::Completion comp;

  TQVariant ret = proxy->evaluate(filename, baseLine, script, n, &comp);

  /*
   *  Error handling
   */
  if (comp.complType() == KJS::Throw && !comp.value().isNull()) {
    KJSErrorDlg *dlg = jsErrorExtension();
    if (dlg) {
      KJS::UString msg = comp.value().toString(proxy->interpreter()->globalExec());
      dlg->addError(i18n("<b>Error</b>: %1: %2").arg(filename, msg.qstring()));
    }
  }

  // Handle immediate redirects now (e.g. location='foo')
  if ( !d->m_redirectURL.isEmpty() && d->m_delayRedirect == -1 )
  {
    kdDebug(6070) << "executeScript done, handling immediate redirection NOW" << endl;
    // Must abort tokenizer, no further script must execute.
    tdehtml::Tokenizer* t = d->m_doc->tokenizer();
    if(t)
      t->abort();
    d->m_redirectionTimer.start( 0, true );
  }

  return ret;
}

TQVariant TDEHTMLPart::executeScript( const TQString &script )
{
    return executeScript( DOM::Node(), script );
}

TQVariant TDEHTMLPart::executeScript( const DOM::Node &n, const TQString &script )
{
#ifdef KJS_VERBOSE
  kdDebug(6070) << "TDEHTMLPart::executeScript caller='" << name() << "' node=" << n.nodeName().string().latin1() << "(" << (n.isNull() ? 0 : n.nodeType()) << ") " /* << script */ << endl;
#endif
  KJSProxy *proxy = jScript();

  if (!proxy || proxy->paused())
    return TQVariant();
  ++(d->m_runningScripts);
  KJS::Completion comp;
  const TQVariant ret = proxy->evaluate( TQString(), 1, script, n, &comp );
  --(d->m_runningScripts);

  /*
   *  Error handling
   */
  if (comp.complType() == KJS::Throw && !comp.value().isNull()) {
    KJSErrorDlg *dlg = jsErrorExtension();
    if (dlg) {
      KJS::UString msg = comp.value().toString(proxy->interpreter()->globalExec());
      dlg->addError(i18n("<b>Error</b>: node %1: %2").arg(n.nodeName().string()).arg(msg.qstring()));
    }
  }

  if (!d->m_runningScripts && d->m_doc && !d->m_doc->parsing() && d->m_submitForm )
      submitFormAgain();

#ifdef KJS_VERBOSE
  kdDebug(6070) << "TDEHTMLPart::executeScript - done" << endl;
#endif
  return ret;
}

bool TDEHTMLPart::scheduleScript(const DOM::Node &n, const TQString& script)
{
    //kdDebug(6050) << "TDEHTMLPart::scheduleScript "<< script << endl;

    d->scheduledScript = script;
    d->scheduledScriptNode = n;

    return true;
}

TQVariant TDEHTMLPart::executeScheduledScript()
{
  if( d->scheduledScript.isEmpty() )
    return TQVariant();

  //kdDebug(6050) << "executing delayed " << d->scheduledScript << endl;

  TQVariant ret = executeScript( d->scheduledScriptNode, d->scheduledScript );
  d->scheduledScript = TQString();
  d->scheduledScriptNode = DOM::Node();

  return ret;
}

void TDEHTMLPart::setJavaEnabled( bool enable )
{
  d->m_bJavaForce = enable;
  d->m_bJavaOverride = true;
}

bool TDEHTMLPart::javaEnabled() const
{
  if (onlyLocalReferences()) return false;

#ifndef TQ_WS_QWS
  if( d->m_bJavaOverride )
      return d->m_bJavaForce;
  return d->m_bJavaEnabled;
#else
  return false;
#endif
}

KJavaAppletContext *TDEHTMLPart::javaContext()
{
  return 0;
}

KJavaAppletContext *TDEHTMLPart::createJavaContext()
{
  return 0;
}

void TDEHTMLPart::setPluginsEnabled( bool enable )
{
  d->m_bPluginsForce = enable;
  d->m_bPluginsOverride = true;
}

bool TDEHTMLPart::pluginsEnabled() const
{
  if (onlyLocalReferences()) return false;

  if ( d->m_bPluginsOverride )
      return d->m_bPluginsForce;
  return d->m_bPluginsEnabled;
}

static int s_DOMTreeIndentLevel = 0;

void TDEHTMLPart::slotDebugDOMTree()
{
  if ( d->m_doc && d->m_doc->firstChild() )
    tqDebug("%s", d->m_doc->firstChild()->toString().string().latin1());

  // Now print the contents of the frames that contain HTML

  const int indentLevel = s_DOMTreeIndentLevel++;

  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !( *it )->m_part.isNull() && (*it)->m_part->inherits( "TDEHTMLPart" ) ) {
      KParts::ReadOnlyPart* const p = ( *it )->m_part;
      kdDebug(6050) << TQString().leftJustify(s_DOMTreeIndentLevel*4,' ') << "FRAME " << p->name() << " " << endl;
      static_cast<TDEHTMLPart*>( p )->slotDebugDOMTree();
    }
  s_DOMTreeIndentLevel = indentLevel;
}

void TDEHTMLPart::slotDebugScript()
{
  if (jScript())
    jScript()->showDebugWindow();
}

void TDEHTMLPart::slotDebugRenderTree()
{
#ifndef NDEBUG
  if ( d->m_doc ) {
    d->m_doc->renderer()->printTree();
    // dump out the contents of the rendering & DOM trees
//    TQString dumps;
//    TQTextStream outputStream(dumps,IO_WriteOnly);
//    d->m_doc->renderer()->layer()->dump( outputStream );
//    kdDebug() << "dump output:" << "\n" + dumps;
  }
#endif
}

void TDEHTMLPart::slotStopAnimations()
{
  stopAnimations();
}

void TDEHTMLPart::setAutoloadImages( bool enable )
{
  if ( d->m_doc && d->m_doc->docLoader()->autoloadImages() == enable )
    return;

  if ( d->m_doc )
    d->m_doc->docLoader()->setAutoloadImages( enable );

  unplugActionList( "loadImages" );

  if ( enable ) {
    delete d->m_paLoadImages;
    d->m_paLoadImages = 0;
  }
  else if ( !d->m_paLoadImages )
    d->m_paLoadImages = new TDEAction( i18n( "Display Images on Page" ), "images_display", 0, this, TQ_SLOT( slotLoadImages() ), actionCollection(), "loadImages" );

  if ( d->m_paLoadImages ) {
    TQPtrList<TDEAction> lst;
    lst.append( d->m_paLoadImages );
    plugActionList( "loadImages", lst );
  }
}

bool TDEHTMLPart::autoloadImages() const
{
  if ( d->m_doc )
    return d->m_doc->docLoader()->autoloadImages();

  return true;
}

void TDEHTMLPart::clear()
{
  if ( d->m_bCleared )
    return;

  d->m_bCleared = true;

  d->m_bClearing = true;

  {
    ConstFrameIt it = d->m_frames.begin();
    const ConstFrameIt end = d->m_frames.end();
    for(; it != end; ++it )
    {
      // Stop HTMLRun jobs for frames
      if ( (*it)->m_run )
        (*it)->m_run->abort();
    }
  }

  {
    ConstFrameIt it = d->m_objects.begin();
    const ConstFrameIt end = d->m_objects.end();
    for(; it != end; ++it )
    {
      // Stop HTMLRun jobs for objects
      if ( (*it)->m_run )
        (*it)->m_run->abort();
    }
  }


  findTextBegin(); // resets d->m_findNode and d->m_findPos
  d->m_mousePressNode = DOM::Node();


  if ( d->m_doc )
  {
    if (d->m_doc->attached()) //the view may have detached it already
	d->m_doc->detach();
  }

  // Moving past doc so that onUnload works.
  if ( d->m_frame && d->m_frame->m_jscript )
    d->m_frame->m_jscript->clear();

  // stopping marquees
  if (d->m_doc && d->m_doc->renderer() && d->m_doc->renderer()->layer())
      d->m_doc->renderer()->layer()->suspendMarquees();

  if ( d->m_view )
    d->m_view->clear();

  // do not dereference the document before the jscript and view are cleared, as some destructors
  // might still try to access the document.
  if ( d->m_doc ) {
    d->m_doc->deref();
  }
  d->m_doc = 0;

  delete d->m_decoder;
  d->m_decoder = 0;

  // We don't want to change between parts if we are going to delete all of them anyway
  if (partManager()) {
    disconnect( partManager(), TQ_SIGNAL( activePartChanged( KParts::Part * ) ),
               this, TQ_SLOT( slotActiveFrameChanged( KParts::Part * ) ) );
  }

  if (d->m_frames.count())
  {
    TDEHTMLFrameList frames = d->m_frames;
    d->m_frames.clear();
    ConstFrameIt it = frames.begin();
    const ConstFrameIt end = frames.end();
    for(; it != end; ++it )
    {
      if ( (*it)->m_part )
      {
        partManager()->removePart( (*it)->m_part );
        delete (KParts::ReadOnlyPart *)(*it)->m_part;
      }
      delete *it;
    }
  }
  d->m_suppressedPopupOriginParts.clear();

  if (d->m_objects.count())
  {
    TDEHTMLFrameList objects = d->m_objects;
    d->m_objects.clear();
    ConstFrameIt oi = objects.begin();
    const ConstFrameIt oiEnd = objects.end();

    for (; oi != oiEnd; ++oi )
      delete *oi;
  }

  // Listen to part changes again
  if (partManager()) {
    connect( partManager(), TQ_SIGNAL( activePartChanged( KParts::Part * ) ),
             this, TQ_SLOT( slotActiveFrameChanged( KParts::Part * ) ) );
  }

  d->m_delayRedirect = 0;
  d->m_redirectURL = TQString();
  d->m_redirectionTimer.stop();
  d->m_redirectLockHistory = true;
  d->m_bClearing = false;
  d->m_frameNameId = 1;
  d->m_bFirstData = true;

  d->m_bMousePressed = false;

  d->m_selectionStart = DOM::Node();
  d->m_selectionEnd = DOM::Node();
  d->m_startOffset = 0;
  d->m_endOffset = 0;
#ifndef TQT_NO_CLIPBOARD
  connect( kapp->clipboard(), TQ_SIGNAL( selectionChanged()), TQ_SLOT( slotClearSelection()));
#endif

  d->m_jobPercent = 0;

  if ( !d->m_haveEncoding )
    d->m_encoding = TQString();
#ifdef SPEED_DEBUG
  d->m_parsetime.restart();
#endif
}

bool TDEHTMLPart::openFile()
{
  return true;
}

DOM::HTMLDocumentImpl *TDEHTMLPart::docImpl() const
{
    if ( d && d->m_doc && d->m_doc->isHTMLDocument() )
        return static_cast<HTMLDocumentImpl*>(d->m_doc);
    return 0;
}

DOM::DocumentImpl *TDEHTMLPart::xmlDocImpl() const
{
    if ( d )
        return d->m_doc;
    return 0;
}

void TDEHTMLPart::slotInfoMessage(TDEIO::Job* tdeio_job, const TQString& msg)
{
  assert(d->m_job == tdeio_job);

  if (!parentPart())
    setStatusBarText(msg, BarDefaultText);
}

void TDEHTMLPart::setPageSecurity( PageSecurity sec )
{
  emit d->m_extension->setPageSecurity( sec );
  if ( sec != NotCrypted && !d->m_statusBarIconLabel && !parentPart() ) {
    d->m_statusBarIconLabel = new KURLLabel( d->m_statusBarExtension->statusBar() );
    d->m_statusBarIconLabel->setFixedHeight( instance()->iconLoader()->currentSize(TDEIcon::Small) );
    d->m_statusBarIconLabel->setSizePolicy(TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ));
    d->m_statusBarIconLabel->setUseCursor( false );
    d->m_statusBarExtension->addStatusBarItem( d->m_statusBarIconLabel, 0, false );
    connect( d->m_statusBarIconLabel, TQ_SIGNAL( leftClickedURL() ), TQ_SLOT( slotSecurity() ) );
  } else if (d->m_statusBarIconLabel) {
    TQToolTip::remove(d->m_statusBarIconLabel);
  }

  if (d->m_statusBarIconLabel) {
    if (d->m_ssl_in_use)
      TQToolTip::add(d->m_statusBarIconLabel,
		    i18n("Session is secured with %1 bit %2.").arg(d->m_ssl_cipher_used_bits).arg(d->m_ssl_cipher));
    else TQToolTip::add(d->m_statusBarIconLabel, i18n("Session is not secured."));
  }

  TQString iconName;
  switch (sec)  {
  case NotCrypted:
    iconName = "decrypted";
    if ( d->m_statusBarIconLabel )  {
      d->m_statusBarExtension->removeStatusBarItem( d->m_statusBarIconLabel );
      delete d->m_statusBarIconLabel;
      d->m_statusBarIconLabel = 0L;
    }
    break;
  case Encrypted:
    iconName = "encrypted";
    break;
  case Mixed:
    iconName = "halfencrypted";
    break;
  }
  d->m_paSecurity->setIcon( iconName );
  if ( d->m_statusBarIconLabel )
    d->m_statusBarIconLabel->setPixmap( SmallIcon( iconName, instance() ) );
}

void TDEHTMLPart::slotData( TDEIO::Job* tdeio_job, const TQByteArray &data )
{
  assert ( d->m_job == tdeio_job );

  //kdDebug( 6050 ) << "slotData: " << data.size() << endl;
  // The first data ?
  if ( !d->m_workingURL.isEmpty() )
  {
      //kdDebug( 6050 ) << "begin!" << endl;

    // We must suspend TDEIO while we're inside begin() because it can cause
    // crashes if a window (such as kjsdebugger) goes back into the event loop,
    // more data arrives, and begin() gets called again (re-entered).
    d->m_job->suspend();
    begin( d->m_workingURL, d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );
    d->m_job->resume();

    if (d->m_cachePolicy == TDEIO::CC_Refresh)
      d->m_doc->docLoader()->setCachePolicy(TDEIO::CC_Verify);
    else
      d->m_doc->docLoader()->setCachePolicy(d->m_cachePolicy);

    d->m_workingURL = KURL();

    d->m_cacheId = TDEHTMLPageCache::self()->createCacheEntry();

    // When the first data arrives, the metadata has just been made available
    d->m_httpHeaders = d->m_job->queryMetaData("HTTP-Headers");
    time_t cacheCreationDate =  d->m_job->queryMetaData("cache-creation-date").toLong();
    d->m_doc->docLoader()->setCacheCreationDate(cacheCreationDate);

    d->m_pageServices = d->m_job->queryMetaData("PageServices");
    d->m_pageReferrer = d->m_job->queryMetaData("referrer");
    d->m_bSecurityInQuestion = false;
    d->m_ssl_in_use = (d->m_job->queryMetaData("ssl_in_use") == "TRUE");

    {
    TDEHTMLPart *p = parentPart();
    if (p && p->d->m_ssl_in_use != d->m_ssl_in_use) {
	while (p->parentPart()) p = p->parentPart();

        p->setPageSecurity( Mixed );
        p->d->m_bSecurityInQuestion = true;
    }
    }

    setPageSecurity( d->m_ssl_in_use ? Encrypted : NotCrypted );

    // Shouldn't all of this be done only if ssl_in_use == true ? (DF)
    d->m_ssl_parent_ip = d->m_job->queryMetaData("ssl_parent_ip");
    d->m_ssl_parent_cert = d->m_job->queryMetaData("ssl_parent_cert");
    d->m_ssl_peer_certificate = d->m_job->queryMetaData("ssl_peer_certificate");
    d->m_ssl_peer_chain = d->m_job->queryMetaData("ssl_peer_chain");
    d->m_ssl_peer_ip = d->m_job->queryMetaData("ssl_peer_ip");
    d->m_ssl_cipher = d->m_job->queryMetaData("ssl_cipher");
    d->m_ssl_cipher_desc = d->m_job->queryMetaData("ssl_cipher_desc");
    d->m_ssl_cipher_version = d->m_job->queryMetaData("ssl_cipher_version");
    d->m_ssl_cipher_used_bits = d->m_job->queryMetaData("ssl_cipher_used_bits");
    d->m_ssl_cipher_bits = d->m_job->queryMetaData("ssl_cipher_bits");
    d->m_ssl_cert_state = d->m_job->queryMetaData("ssl_cert_state");

    if (d->m_statusBarIconLabel) {
      TQToolTip::remove(d->m_statusBarIconLabel);
      if (d->m_ssl_in_use) {
        TQToolTip::add(d->m_statusBarIconLabel, i18n("Session is secured with %1 bit %2.").arg(d->m_ssl_cipher_used_bits).arg(d->m_ssl_cipher));
      } else {
        TQToolTip::add(d->m_statusBarIconLabel, i18n("Session is not secured."));
      }
    }

    // Check for charset meta-data
    TQString qData = d->m_job->queryMetaData("charset");
    if ( !qData.isEmpty() && !d->m_haveEncoding ) // only use information if the user didn't override the settings
       d->m_encoding = qData;


    // Support for http-refresh
    qData = d->m_job->queryMetaData("http-refresh");
    if( !qData.isEmpty())
      d->m_doc->processHttpEquiv("refresh", qData);

    // DISABLED: Support Content-Location per section 14.14 of RFC 2616.
    // See BR# 51185,BR# 82747
    /*
    TQString baseURL = d->m_job->queryMetaData ("content-location");
    if (!baseURL.isEmpty())
      d->m_doc->setBaseURL(KURL( d->m_doc->completeURL(baseURL) ));
    */

    // Support for Content-Language
    TQString language = d->m_job->queryMetaData("content-language");
    if (!language.isEmpty())
        d->m_doc->setContentLanguage(language);

    if ( !m_url.isLocalFile() ) {
        // Support for http last-modified
        d->m_lastModified = d->m_job->queryMetaData("modified");
    } else
        d->m_lastModified = TQString(); // done on-demand by lastModified()
  }

  TDEHTMLPageCache::self()->addData(d->m_cacheId, data);
  write( data.data(), data.size() );
  if (d->m_frame && d->m_frame->m_jscript)
    d->m_frame->m_jscript->dataReceived();
}

void TDEHTMLPart::slotRestoreData(const TQByteArray &data )
{
  // The first data ?
  if ( !d->m_workingURL.isEmpty() )
  {
     long saveCacheId = d->m_cacheId;
     TQString savePageReferrer = d->m_pageReferrer;
     TQString saveEncoding     = d->m_encoding;
     begin( d->m_workingURL, d->m_extension->urlArgs().xOffset, d->m_extension->urlArgs().yOffset );
     d->m_encoding     = saveEncoding;
     d->m_pageReferrer = savePageReferrer;
     d->m_cacheId = saveCacheId;
     d->m_workingURL = KURL();
  }

  //kdDebug( 6050 ) << "slotRestoreData: " << data.size() << endl;
  write( data.data(), data.size() );

  if (data.size() == 0)
  {
      //kdDebug( 6050 ) << "slotRestoreData: <<end of data>>" << endl;
     // End of data.
    if (d->m_doc && d->m_doc->parsing())
        end(); //will emit completed()
  }
}

void TDEHTMLPart::showError( TDEIO::Job* job )
{
  kdDebug(6050) << "TDEHTMLPart::showError d->m_bParsing=" << (d->m_doc && d->m_doc->parsing()) << " d->m_bComplete=" << d->m_bComplete
                << " d->m_bCleared=" << d->m_bCleared << endl;

  if (job->error() == TDEIO::ERR_NO_CONTENT)
	return;

  if ( (d->m_doc && d->m_doc->parsing()) || d->m_workingURL.isEmpty() ) // if we got any data already
    job->showErrorDialog( /*d->m_view*/ );
  else
  {
    htmlError( job->error(), job->errorText(), d->m_workingURL );
  }
}

// This is a protected method, placed here because of it's relevance to showError
void TDEHTMLPart::htmlError( int errorCode, const TQString& text, const KURL& reqUrl )
{
  kdDebug(6050) << "TDEHTMLPart::htmlError errorCode=" << errorCode << " text=" << text << endl;
  // make sure we're not executing any embedded JS
  bool bJSFO = d->m_bJScriptForce;
  bool bJSOO = d->m_bJScriptOverride;
  d->m_bJScriptForce = false;
  d->m_bJScriptOverride = true;
  begin();
  TQString errText = TQString::fromLatin1( "<HTML dir=%1><HEAD><TITLE>" )
                           .arg(TQApplication::reverseLayout() ? "rtl" : "ltr");
  errText += i18n( "Error while loading %1" ).arg( reqUrl.htmlURL() );
  errText += TQString::fromLatin1( "</TITLE></HEAD><BODY><P>" );
  errText += i18n( "An error occurred while loading <B>%1</B>:" ).arg( reqUrl.htmlURL() );
  errText += TQString::fromLatin1( "</P>" );
  errText += TQStyleSheet::convertFromPlainText( TDEIO::buildErrorString( errorCode, text ) );
  errText += TQString::fromLatin1( "</BODY></HTML>" );
  write(errText);
  end();

  d->m_bJScriptForce = bJSFO;
  d->m_bJScriptOverride = bJSOO;

  // make the working url the current url, so that reload works and
  // emit the progress signals to advance one step in the history
  // (so that 'back' works)
  m_url = reqUrl; // same as d->m_workingURL
  d->m_workingURL = KURL();
  emit started( 0 );
  emit completed();
  return;
  // following disabled until 3.1

  TQString errorName, techName, description;
  TQStringList causes, solutions;

  TQByteArray raw = TDEIO::rawErrorDetail( errorCode, text, &reqUrl );
  TQDataStream stream(raw, IO_ReadOnly);

  stream >> errorName >> techName >> description >> causes >> solutions;

  TQString url, protocol, datetime;
  url = reqUrl.prettyURL();
  protocol = reqUrl.protocol();
  datetime = TDEGlobal::locale()->formatDateTime( TQDateTime::currentDateTime(),
                                                false );

  TQString doc = TQString::fromLatin1( "<html><head><title>" );
  doc += i18n( "Error: " );
  doc += errorName;
  doc += TQString::fromLatin1( " - %1</title></head><body><h1>" ).arg( url );
  doc += i18n( "The requested operation could not be completed" );
  doc += TQString::fromLatin1( "</h1><h2>" );
  doc += errorName;
  doc += TQString::fromLatin1( "</h2>" );
  if ( !techName.isNull() ) {
    doc += TQString::fromLatin1( "<h2>" );
    doc += i18n( "Technical Reason: " );
    doc += techName;
    doc += TQString::fromLatin1( "</h2>" );
  }
  doc += TQString::fromLatin1( "<h3>" );
  doc += i18n( "Details of the Request:" );
  doc += TQString::fromLatin1( "</h3><ul><li>" );
  doc += i18n( "URL: %1" ).arg( url );
  doc += TQString::fromLatin1( "</li><li>" );
  if ( !protocol.isNull() ) {
    // uncomment for 3.1... i18n change
    // doc += i18n( "Protocol: %1" ).arg( protocol ).arg( protocol );
    doc += TQString::fromLatin1( "</li><li>" );
  }
  doc += i18n( "Date and Time: %1" ).arg( datetime );
  doc += TQString::fromLatin1( "</li><li>" );
  doc += i18n( "Additional Information: %1" ).arg( text );
  doc += TQString::fromLatin1( "</li></ul><h3>" );
  doc += i18n( "Description:" );
  doc += TQString::fromLatin1( "</h3><p>" );
  doc += description;
  doc += TQString::fromLatin1( "</p>" );
  if ( causes.count() ) {
    doc += TQString::fromLatin1( "<h3>" );
    doc += i18n( "Possible Causes:" );
    doc += TQString::fromLatin1( "</h3><ul><li>" );
    doc += causes.join( "</li><li>" );
    doc += TQString::fromLatin1( "</li></ul>" );
  }
  if ( solutions.count() ) {
    doc += TQString::fromLatin1( "<h3>" );
    doc += i18n( "Possible Solutions:" );
    doc += TQString::fromLatin1( "</h3><ul><li>" );
    doc += solutions.join( "</li><li>" );
    doc += TQString::fromLatin1( "</li></ul>" );
  }
  doc += TQString::fromLatin1( "</body></html>" );

  write( doc );
  end();
}

void TDEHTMLPart::slotFinished( TDEIO::Job * job )
{
  d->m_job = 0L;
  d->m_jobspeed = 0L;

  if (job->error())
  {
    TDEHTMLPageCache::self()->cancelEntry(d->m_cacheId);

    // The following catches errors that occur as a result of HTTP
    // to FTP redirections where the FTP URL is a directory. Since
    // TDEIO cannot change a redirection request from GET to LISTDIR,
    // we have to take care of it here once we know for sure it is
    // a directory...
    if (job->error() == TDEIO::ERR_IS_DIRECTORY)
    {
      KParts::URLArgs args;
      emit d->m_extension->openURLRequest( d->m_workingURL, args );
    }
    else
    {
      emit canceled( job->errorString() );
      // TODO: what else ?
      checkCompleted();
      showError( job );
    }

    return;
  }
  TDEIO::TransferJob *tjob = ::tqt_cast<TDEIO::TransferJob*>(job);
  if (tjob && tjob->isErrorPage()) {
    tdehtml::RenderPart *renderPart = d->m_frame ? static_cast<tdehtml::RenderPart *>(d->m_frame->m_frame) : 0;
    if (renderPart) {
      HTMLObjectElementImpl* elt = static_cast<HTMLObjectElementImpl *>(renderPart->element());
      if (!elt)
        return;
      elt->renderAlternative();
      checkCompleted();
     }
     if (d->m_bComplete) return;
  }

  //kdDebug( 6050 ) << "slotFinished" << endl;

  TDEHTMLPageCache::self()->endData(d->m_cacheId);
  if (d->m_frame && d->m_frame->m_jscript)
    d->m_frame->m_jscript->dataReceived();

  if ( d->m_doc && d->m_doc->docLoader()->expireDate() && m_url.protocol().lower().startsWith("http"))
      TDEIO::http_update_cache(m_url, false, d->m_doc->docLoader()->expireDate());

  d->m_workingURL = KURL();

  if ( d->m_doc && d->m_doc->parsing())
    end(); //will emit completed()
}

void TDEHTMLPart::begin( const KURL &url, int xOffset, int yOffset )
{
  // No need to show this for a new page until an error is triggered
  if (!parentPart()) {
    removeJSErrorExtension();
    setSuppressedPopupIndicator( false );
    d->m_openableSuppressedPopups = 0;
    for ( TQValueListIterator<TQGuardedPtr<TDEHTMLPart> > i = d->m_suppressedPopupOriginParts.begin();
          i != d->m_suppressedPopupOriginParts.end(); ++i ) {

      if (TDEHTMLPart* part = *i) {
        KJS::Window *w = KJS::Window::retrieveWindow( part );
        if (w)
          w->forgetSuppressedWindows();
      }
    }
  }

  clear();
  d->m_bCleared = false;
  d->m_cacheId = 0;
  d->m_bComplete = false;
  d->m_bLoadEventEmitted = false;

  if(url.isValid()) {
      TQString urlString = url.url();
      TDEHTMLFactory::vLinks()->insert( urlString );
      TQString urlString2 = url.prettyURL();
      if ( urlString != urlString2 ) {
          TDEHTMLFactory::vLinks()->insert( urlString2 );
      }
  }


  // ###
  //stopParser();

  KParts::URLArgs args( d->m_extension->urlArgs() );
  args.xOffset = xOffset;
  args.yOffset = yOffset;
  d->m_extension->setURLArgs( args );

  d->m_pageReferrer = TQString();

  KURL ref(url);
  d->m_referrer = ref.protocol().startsWith("http") ? ref.url() : "";

  m_url = url;

  bool servedAsXHTML = args.serviceType == "application/xhtml+xml";
  bool servedAsXML = KMimeType::mimeType(args.serviceType)->is( "text/xml" );
  // ### not sure if XHTML documents served as text/xml should use DocumentImpl or HTMLDocumentImpl
  if ( servedAsXML && !servedAsXHTML ) { // any XML derivative, except XHTML
    d->m_doc = DOMImplementationImpl::instance()->createDocument( d->m_view );
  } else {
    d->m_doc = DOMImplementationImpl::instance()->createHTMLDocument( d->m_view );
    // HTML or XHTML? (#86446)
    static_cast<HTMLDocumentImpl *>(d->m_doc)->setHTMLRequested( !servedAsXHTML );
  }
#ifndef TDEHTML_NO_CARET
//  d->m_view->initCaret();
#endif

  d->m_doc->ref();
  d->m_doc->setURL( m_url.url() );
  if (!d->m_doc->attached())
    d->m_doc->attach( );
  d->m_doc->setBaseURL( KURL() );
  d->m_doc->docLoader()->setShowAnimations( TDEHTMLFactory::defaultHTMLSettings()->showAnimations() );
  emit docCreated();

  d->m_paUseStylesheet->setItems(TQStringList());
  d->m_paUseStylesheet->setEnabled( false );

  setAutoloadImages( TDEHTMLFactory::defaultHTMLSettings()->autoLoadImages() );
  TQString userStyleSheet = TDEHTMLFactory::defaultHTMLSettings()->userStyleSheet();
  if ( !userStyleSheet.isEmpty() )
    setUserStyleSheet( KURL( userStyleSheet ) );

  d->m_doc->setRestoreState(args.docState);
  d->m_doc->open();
  connect(d->m_doc,TQ_SIGNAL(finishedParsing()),this,TQ_SLOT(slotFinishedParsing()));

  emit d->m_extension->enableAction( "print", true );

  d->m_doc->setParsing(true);
}

void TDEHTMLPart::write( const char *str, int len )
{
  if ( !d->m_decoder )
    d->m_decoder = createDecoder();

  if ( len == -1 )
    len = strlen( str );

  if ( len == 0 )
    return;

  TQString decoded = d->m_decoder->decode( str, len );

  if(decoded.isEmpty()) return;

  if(d->m_bFirstData) {
      // determine the parse mode
      d->m_doc->determineParseMode( decoded );
      d->m_bFirstData = false;

  //kdDebug(6050) << "TDEHTMLPart::write haveEnc = " << d->m_haveEncoding << endl;
      // ### this is still quite hacky, but should work a lot better than the old solution
      if(d->m_decoder->visuallyOrdered()) d->m_doc->setVisuallyOrdered();
      d->m_doc->setDecoderCodec(d->m_decoder->codec());
      d->m_doc->recalcStyle( NodeImpl::Force );
  }

  tdehtml::Tokenizer* t = d->m_doc->tokenizer();
  if(t)
    t->write( decoded, true );
}

void TDEHTMLPart::write( const TQString &str )
{
    if ( str.isNull() )
        return;

    if(d->m_bFirstData) {
        // determine the parse mode
        d->m_doc->setParseMode( DocumentImpl::Strict );
        d->m_bFirstData = false;
    }
    tdehtml::Tokenizer* t = d->m_doc->tokenizer();
    if(t)
        t->write( str, true );
}

void TDEHTMLPart::end()
{
    if (d->m_doc) {
        if (d->m_decoder) {
            TQString decoded = d->m_decoder->flush();
            if (d->m_bFirstData) {
                d->m_bFirstData = false;
                d->m_doc->determineParseMode(decoded);
            }
            write(decoded);
        }
        d->m_doc->finishParsing();
    }
}

bool TDEHTMLPart::doOpenStream( const TQString& mimeType )
{
    KMimeType::Ptr mime = KMimeType::mimeType(mimeType);
    if ( mime->is( "text/html" ) || mime->is( "text/xml" ) )
    {
        begin( url() );
        return true;
    }
    return false;
}

bool TDEHTMLPart::doWriteStream( const TQByteArray& data )
{
    write( data.data(), data.size() );
    return true;
}

bool TDEHTMLPart::doCloseStream()
{
    end();
    return true;
}


void TDEHTMLPart::paint(TQPainter *p, const TQRect &rc, int yOff, bool *more)
{
    if (!d->m_view) return;
    d->m_view->paint(p, rc, yOff, more);
}

void TDEHTMLPart::stopAnimations()
{
  if ( d->m_doc )
    d->m_doc->docLoader()->setShowAnimations( TDEHTMLSettings::KAnimationDisabled );

  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !(*it)->m_part.isNull() && (*it)->m_part->inherits( "TDEHTMLPart" ) ) {
      KParts::ReadOnlyPart* const p = ( *it )->m_part;
      static_cast<TDEHTMLPart*>( p )->stopAnimations();
    }
}

void TDEHTMLPart::resetFromScript()
{
    closeURL();
    d->m_bComplete = false;
    d->m_bLoadEventEmitted = false;
    disconnect(d->m_doc,TQ_SIGNAL(finishedParsing()),this,TQ_SLOT(slotFinishedParsing()));
    connect(d->m_doc,TQ_SIGNAL(finishedParsing()),this,TQ_SLOT(slotFinishedParsing()));
    d->m_doc->setParsing(true);

    emit started( 0L );
}

void TDEHTMLPart::slotFinishedParsing()
{
  d->m_doc->setParsing(false);
  checkEmitLoadEvent();
  disconnect(d->m_doc,TQ_SIGNAL(finishedParsing()),this,TQ_SLOT(slotFinishedParsing()));

  if (!d->m_view)
    return; // We are probably being destructed.

  checkCompleted();
}

void TDEHTMLPart::slotLoaderRequestStarted( tdehtml::DocLoader* dl, tdehtml::CachedObject *obj )
{
  if ( obj && obj->type() == tdehtml::CachedObject::Image && d->m_doc && d->m_doc->docLoader() == dl ) {
    TDEHTMLPart* p = this;
    while ( p ) {
      TDEHTMLPart* const op = p;
      ++(p->d->m_totalObjectCount);
      p = p->parentPart();
      if ( !p && op->d->m_loadedObjects <= op->d->m_totalObjectCount
        && !op->d->m_progressUpdateTimer.isActive())
	op->d->m_progressUpdateTimer.start( 200, true );
    }
  }
}

void TDEHTMLPart::slotLoaderRequestDone( tdehtml::DocLoader* dl, tdehtml::CachedObject *obj )
{
  if ( obj && obj->type() == tdehtml::CachedObject::Image && d->m_doc && d->m_doc->docLoader() == dl ) {
    TDEHTMLPart* p = this;
    while ( p ) {
      TDEHTMLPart* const op = p;
      ++(p->d->m_loadedObjects);
      p = p->parentPart();
      if ( !p && op->d->m_loadedObjects <= op->d->m_totalObjectCount && op->d->m_jobPercent <= 100
        && !op->d->m_progressUpdateTimer.isActive())
	op->d->m_progressUpdateTimer.start( 200, true );
    }
  }

  checkCompleted();
}

void TDEHTMLPart::slotProgressUpdate()
{
  int percent;
  if ( d->m_loadedObjects < d->m_totalObjectCount )
    percent = d->m_jobPercent / 4 + ( d->m_loadedObjects*300 ) / ( 4*d->m_totalObjectCount );
  else
    percent = d->m_jobPercent;

  if( d->m_bComplete )
    percent = 100;

  if (d->m_statusMessagesEnabled) {
    if( d->m_bComplete )
      emit d->m_extension->infoMessage( i18n( "Page loaded." ));
    else if ( d->m_loadedObjects < d->m_totalObjectCount && percent >= 75 )
      emit d->m_extension->infoMessage( i18n( "%n Image of %1 loaded.", "%n Images of %1 loaded.", d->m_loadedObjects).arg(d->m_totalObjectCount) );
  }

  emit d->m_extension->loadingProgress( percent );
}

void TDEHTMLPart::slotJobSpeed( TDEIO::Job* /*job*/, unsigned long speed )
{
  d->m_jobspeed = speed;
  if (!parentPart())
    setStatusBarText(jsStatusBarText(), BarOverrideText);
}

void TDEHTMLPart::slotJobPercent( TDEIO::Job* /*job*/, unsigned long percent )
{
  d->m_jobPercent = percent;

  if ( !parentPart() )
    d->m_progressUpdateTimer.start( 0, true );
}

void TDEHTMLPart::slotJobDone( TDEIO::Job* /*job*/ )
{
  d->m_jobPercent = 100;

  if ( !parentPart() )
    d->m_progressUpdateTimer.start( 0, true );
}

void TDEHTMLPart::slotUserSheetStatDone( TDEIO::Job *_job )
{
  using namespace TDEIO;

  if ( _job->error() ) {
    showError( _job );
    return;
  }

  const UDSEntry entry = dynamic_cast<TDEIO::StatJob *>( _job )->statResult();
  UDSEntry::ConstIterator it = entry.begin();
  const UDSEntry::ConstIterator end = entry.end();
  for ( ; it != end; ++it ) {
    if ( ( *it ).m_uds == UDS_MODIFICATION_TIME ) {
     break;
    }
  }

  // If the filesystem supports modification times, only reload the
  // user-defined stylesheet if necessary - otherwise always reload.
  if ( it != end ) {
    const time_t lastModified = static_cast<time_t>( ( *it ).m_long );
    if ( d->m_userStyleSheetLastModified >= lastModified ) {
      return;
    }
    d->m_userStyleSheetLastModified = lastModified;
  }

  setUserStyleSheet( KURL( settings()->userStyleSheet() ) );
}

void TDEHTMLPart::checkCompleted()
{
//   kdDebug( 6050 ) << "TDEHTMLPart::checkCompleted() " << this << " " << name() << endl;
//   kdDebug( 6050 ) << "                           parsing: " << (d->m_doc && d->m_doc->parsing()) << endl;
//   kdDebug( 6050 ) << "                           complete: " << d->m_bComplete << endl;

  // restore the cursor position
  if (d->m_doc && !d->m_doc->parsing() && !d->m_focusNodeRestored)
  {
      if (d->m_focusNodeNumber >= 0)
          d->m_doc->setFocusNode(d->m_doc->nodeWithAbsIndex(d->m_focusNodeNumber));

      d->m_focusNodeRestored = true;
  }

  bool bPendingChildRedirection = false;
  // Any frame that hasn't completed yet ?
  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it ) {
    if ( !(*it)->m_bCompleted )
    {
      //kdDebug( 6050 ) << this << " is waiting for " << (*it)->m_part << endl;
      return;
    }
    // Check for frames with pending redirections
    if ( (*it)->m_bPendingRedirection )
      bPendingChildRedirection = true;
  }

  // Any object that hasn't completed yet ?
  {
    ConstFrameIt oi = d->m_objects.begin();
    const ConstFrameIt oiEnd = d->m_objects.end();

    for (; oi != oiEnd; ++oi )
      if ( !(*oi)->m_bCompleted )
        return;
  }
  // Are we still parsing - or have we done the completed stuff already ?
  if ( d->m_bComplete || (d->m_doc && d->m_doc->parsing()) )
    return;

  // Still waiting for images/scripts from the loader ?
  int requests = 0;
  if ( d->m_doc && d->m_doc->docLoader() )
    requests = tdehtml::Cache::loader()->numRequests( d->m_doc->docLoader() );

  if ( requests > 0 )
  {
    //kdDebug(6050) << "still waiting for images/scripts from the loader - requests:" << requests << endl;
    return;
  }

  // OK, completed.
  // Now do what should be done when we are really completed.
  d->m_bComplete = true;
  d->m_cachePolicy = KProtocolManager::cacheControl(); // reset cache policy
  d->m_totalObjectCount = 0;
  d->m_loadedObjects = 0;

  TDEHTMLPart* p = this;
  while ( p ) {
    TDEHTMLPart* op = p;
    p = p->parentPart();
    if ( !p && !op->d->m_progressUpdateTimer.isActive())
      op->d->m_progressUpdateTimer.start( 0, true );
  }

  checkEmitLoadEvent(); // if we didn't do it before

  bool pendingAction = false;

  if ( !d->m_redirectURL.isEmpty() )
  {
    // DA: Do not start redirection for frames here! That action is
    // deferred until the parent emits a completed signal.
    if ( parentPart() == 0 ) {
      //kdDebug(6050) << this << " starting redirection timer" << endl;
      d->m_redirectionTimer.start( 1000 * d->m_delayRedirect, true );
    } else {
      //kdDebug(6050) << this << " not toplevel -> not starting redirection timer. Waiting for slotParentCompleted." << endl;
    }

    pendingAction = true;
  }
  else if ( bPendingChildRedirection )
  {
    pendingAction = true;
  }

  // the view will emit completed on our behalf,
  // either now or at next repaint if one is pending

  //kdDebug(6050) << this << " asks the view to emit completed. pendingAction=" << pendingAction << endl;
  d->m_view->complete( pendingAction );

  // find the alternate stylesheets
  TQStringList sheets;
  if (d->m_doc)
     sheets = d->m_doc->availableStyleSheets();
  sheets.prepend( i18n( "Automatic Detection" ) );
  d->m_paUseStylesheet->setItems( sheets );

  d->m_paUseStylesheet->setEnabled( sheets.count() > 2);
  if (sheets.count() > 2)
  {
    d->m_paUseStylesheet->setCurrentItem(kMax(sheets.findIndex(d->m_sheetUsed), 0));
    slotUseStylesheet();
  }

  setJSDefaultStatusBarText(TQString());

#ifdef SPEED_DEBUG
  kdDebug(6050) << "DONE: " <<d->m_parsetime.elapsed() << endl;
#endif
}

void TDEHTMLPart::checkEmitLoadEvent()
{
  if ( d->m_bLoadEventEmitted || !d->m_doc || d->m_doc->parsing() ) return;

  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !(*it)->m_bCompleted ) // still got a frame running -> too early
      return;

  ConstFrameIt oi = d->m_objects.begin();
  const ConstFrameIt oiEnd = d->m_objects.end();

  for (; oi != oiEnd; ++oi )
    if ( !(*oi)->m_bCompleted ) // still got a object running -> too early
      return;

  // Still waiting for images/scripts from the loader ?
  // (onload must happen afterwards, #45607)
  // ## This makes this method very similar to checkCompleted. A brave soul should try merging them.
  int requests = 0;
  if ( d->m_doc && d->m_doc->docLoader() )
    requests = tdehtml::Cache::loader()->numRequests( d->m_doc->docLoader() );

  if ( requests > 0 )
    return;

  d->m_bLoadEventEmitted = true;
  if (d->m_doc)
    d->m_doc->close();
}

const TDEHTMLSettings *TDEHTMLPart::settings() const
{
  return d->m_settings;
}

#ifndef KDE_NO_COMPAT
KURL TDEHTMLPart::baseURL() const
{
  if ( !d->m_doc ) return KURL();

  return d->m_doc->baseURL();
}

TQString TDEHTMLPart::baseTarget() const
{
  if ( !d->m_doc ) return TQString();

  return d->m_doc->baseTarget();
}
#endif

KURL TDEHTMLPart::completeURL( const TQString &url )
{
  if ( !d->m_doc ) return KURL( url );

  if (d->m_decoder)
    return KURL(d->m_doc->completeURL(url), d->m_decoder->codec()->mibEnum());

  return KURL( d->m_doc->completeURL( url ) );
}

// Called by ecma/kjs_window in case of redirections from Javascript,
// and by xml/dom_docimpl.cpp in case of http-equiv meta refresh.
void TDEHTMLPart::scheduleRedirection( int delay, const TQString &url, bool doLockHistory )
{
  kdDebug(6050) << "TDEHTMLPart::scheduleRedirection delay=" << delay << " url=" << url << endl;
  kdDebug(6050) << "current redirectURL=" << d->m_redirectURL << " with delay " << d->m_delayRedirect <<  endl;
  if( delay < 24*60*60 &&
      ( d->m_redirectURL.isEmpty() || delay <= d->m_delayRedirect) ) {
    d->m_delayRedirect = delay;
    d->m_redirectURL = url;
    d->m_redirectLockHistory = doLockHistory;
    kdDebug(6050) << " d->m_bComplete=" << d->m_bComplete << endl;
    if ( d->m_bComplete ) {
      d->m_redirectionTimer.stop();
      d->m_redirectionTimer.start( kMax(0, 1000 * d->m_delayRedirect), true );
    }
  }
}

void TDEHTMLPart::slotRedirect()
{
  kdDebug(6050) << this << " slotRedirect()" << endl;
  TQString u = d->m_redirectURL;
  d->m_delayRedirect = 0;
  d->m_redirectURL = TQString();

  // SYNC check with ecma/kjs_window.cpp::goURL !
  if ( u.find( TQString::fromLatin1( "javascript:" ), 0, false ) == 0 )
  {
    TQString script = KURL::decode_string( u.right( u.length() - 11 ) );
    kdDebug( 6050 ) << "TDEHTMLPart::slotRedirect script=" << script << endl;
    TQVariant res = executeScript( DOM::Node(), script );
    if ( res.type() == TQVariant::String ) {
      begin( url() );
      write( res.asString() );
      end();
    }
    emit completed();
    return;
  }
  KParts::URLArgs args;
  KURL cUrl( m_url );
  KURL url( u );

  // handle windows opened by JS
  if ( openedByJS() && d->m_opener )
      cUrl = d->m_opener->url();

  if (!kapp || !kapp->authorizeURLAction("redirect", cUrl, url))
  {
    kdWarning(6050) << "TDEHTMLPart::scheduleRedirection: Redirection from " << cUrl << " to " << url << " REJECTED!" << endl;
    emit completed();
    return;
  }

  if ( urlcmp( u, m_url.url(), true, true ) )
  {
    args.metaData().insert("referrer", d->m_pageReferrer);
  }

  // For javascript and META-tag based redirections:
  //   - We don't take cross-domain-ness in consideration if we are the
  //   toplevel frame because the new URL may be in a different domain as the current URL
  //   but that's ok.
  //   - If we are not the toplevel frame then we check against the toplevelURL()
  if (parentPart())
      args.metaData().insert("cross-domain", toplevelURL().url());

  args.setLockHistory( d->m_redirectLockHistory );
  // _self: make sure we don't use any <base target=>'s

  d->m_urlSelectedOpenedURL = true; // In case overriden, default to success
  urlSelected( u, 0, 0, "_self", args );

  if ( !d->m_urlSelectedOpenedURL ) // urlSelected didn't open a url, so emit completed ourselves
    emit completed();
}

void TDEHTMLPart::slotRedirection(TDEIO::Job*, const KURL& url)
{
  // the slave told us that we got redirected
  //kdDebug( 6050 ) << "redirection by TDEIO to " << url.url() << endl;
  emit d->m_extension->setLocationBarURL( url.prettyURL() );
  d->m_workingURL = url;
}

bool TDEHTMLPart::setEncoding( const TQString &name, bool override )
{
    d->m_encoding = name;
    d->m_haveEncoding = override;

    if( !m_url.isEmpty() ) {
        // reload document
        closeURL();
        KURL url = m_url;
        m_url = 0;
        d->m_restored = true;
        openURL(url);
        d->m_restored = false;
    }

    return true;
}

TQString TDEHTMLPart::encoding() const
{
    if(d->m_haveEncoding && !d->m_encoding.isEmpty())
        return d->m_encoding;

    if(d->m_decoder && d->m_decoder->encoding())
        return TQString(d->m_decoder->encoding());

    return defaultEncoding();
}

TQString TDEHTMLPart::defaultEncoding() const
{
  TQString encoding = settings()->encoding();
  if ( !encoding.isEmpty() )
    return encoding;
  // HTTP requires the default encoding to be latin1, when neither
  // the user nor the page requested a particular encoding.
  if ( url().protocol().startsWith( "http" ) )
    return "iso-8859-1";
  else
    return TDEGlobal::locale()->encoding();
}

void TDEHTMLPart::setUserStyleSheet(const KURL &url)
{
  if ( d->m_doc && d->m_doc->docLoader() )
    (void) new tdehtml::PartStyleSheetLoader(this, url.url(), d->m_doc->docLoader());
}

void TDEHTMLPart::setUserStyleSheet(const TQString &styleSheet)
{
  if ( d->m_doc )
    d->m_doc->setUserStyleSheet( styleSheet );
}

bool TDEHTMLPart::gotoAnchor( const TQString &name )
{
  if (!d->m_doc)
    return false;

  HTMLCollectionImpl *anchors =
      new HTMLCollectionImpl( d->m_doc, HTMLCollectionImpl::DOC_ANCHORS);
  anchors->ref();
  NodeImpl *n = anchors->namedItem(name);
  anchors->deref();

  if(!n) {
      n = d->m_doc->getElementById( name );
  }

  d->m_doc->setCSSTarget(n); // Setting to null will clear the current target.

  // Implement the rule that "" and "top" both mean top of page as in other browsers.
  bool quirkyName = !n && !d->m_doc->inStrictMode() && (name.isEmpty() || name.lower() == "top");

  if (quirkyName) {
      d->m_view->setContentsPos(0, 0);
      return true;
  } else if (!n) {
      kdDebug(6050) << "TDEHTMLPart::gotoAnchor node '" << name << "' not found" << endl;
      return false;
  }

  int x = 0, y = 0;
  int gox, dummy;
  HTMLElementImpl *a = static_cast<HTMLElementImpl *>(n);

  a->getUpperLeftCorner(x, y);
  if (x <= d->m_view->contentsX())
    gox = x - 10;
  else {
    gox = d->m_view->contentsX();
    if ( x + 10 > d->m_view->contentsX()+d->m_view->visibleWidth()) {
      a->getLowerRightCorner(x, dummy);
      gox = x - d->m_view->visibleWidth() + 10;
    }
  }

  d->m_view->setContentsPos(gox, y);

  return true;
}

bool TDEHTMLPart::nextAnchor()
{
  if (!d->m_doc)
    return false;
  d->m_view->focusNextPrevNode ( true );

  return true;
}

bool TDEHTMLPart::prevAnchor()
{
  if (!d->m_doc)
    return false;
  d->m_view->focusNextPrevNode ( false );

  return true;
}

void TDEHTMLPart::setStandardFont( const TQString &name )
{
    d->m_settings->setStdFontName(name);
}

void TDEHTMLPart::setFixedFont( const TQString &name )
{
    d->m_settings->setFixedFontName(name);
}

void TDEHTMLPart::setURLCursor( const TQCursor &c )
{
  d->m_linkCursor = c;
}

TQCursor TDEHTMLPart::urlCursor() const
{
  return d->m_linkCursor;
}

bool TDEHTMLPart::onlyLocalReferences() const
{
  return d->m_onlyLocalReferences;
}

void TDEHTMLPart::setOnlyLocalReferences(bool enable)
{
  d->m_onlyLocalReferences = enable;
}

void TDEHTMLPartPrivate::setFlagRecursively(
	bool TDEHTMLPartPrivate::*flag, bool value)
{
  // first set it on the current one
  this->*flag = value;

  // descend into child frames recursively
  {
    TQValueList<tdehtml::ChildFrame*>::Iterator it = m_frames.begin();
    const TQValueList<tdehtml::ChildFrame*>::Iterator itEnd = m_frames.end();
    for (; it != itEnd; ++it) {
      TDEHTMLPart* const part = static_cast<TDEHTMLPart *>((KParts::ReadOnlyPart *)(*it)->m_part);
      if (part->inherits("TDEHTMLPart"))
        part->d->setFlagRecursively(flag, value);
    }/*next it*/
  }
  // do the same again for objects
  {
    TQValueList<tdehtml::ChildFrame*>::Iterator it = m_objects.begin();
    const TQValueList<tdehtml::ChildFrame*>::Iterator itEnd = m_objects.end();
    for (; it != itEnd; ++it) {
      TDEHTMLPart* const part = static_cast<TDEHTMLPart *>((KParts::ReadOnlyPart *)(*it)->m_part);
      if (part->inherits("TDEHTMLPart"))
        part->d->setFlagRecursively(flag, value);
    }/*next it*/
  }
}

void TDEHTMLPart::setCaretMode(bool enable)
{
#ifndef TDEHTML_NO_CARET
  kdDebug(6200) << "setCaretMode(" << enable << ")" << endl;
  if (isCaretMode() == enable) return;
  d->setFlagRecursively(&TDEHTMLPartPrivate::m_caretMode, enable);
  // FIXME: this won't work on frames as expected
  if (!isEditable()) {
    if (enable) {
      view()->initCaret(true);
      view()->ensureCaretVisible();
    } else
      view()->caretOff();
  }/*end if*/
#endif // TDEHTML_NO_CARET
}

bool TDEHTMLPart::isCaretMode() const
{
  return d->m_caretMode;
}

void TDEHTMLPart::setEditable(bool enable)
{
#ifndef TDEHTML_NO_CARET
  if (isEditable() == enable) return;
  d->setFlagRecursively(&TDEHTMLPartPrivate::m_designMode, enable);
  // FIXME: this won't work on frames as expected
  if (!isCaretMode()) {
    if (enable) {
      view()->initCaret(true);
      view()->ensureCaretVisible();
    } else
      view()->caretOff();
  }/*end if*/
#endif // TDEHTML_NO_CARET
}

bool TDEHTMLPart::isEditable() const
{
  return d->m_designMode;
}

void TDEHTMLPart::setCaretPosition(DOM::Node node, long offset, bool extendSelection)
{
#ifndef TDEHTML_NO_CARET
#if 0
  kdDebug(6200) << k_funcinfo << "node: " << node.handle() << " nodeName: "
  	<< node.nodeName().string() << " offset: " << offset
	<< " extendSelection " << extendSelection << endl;
#endif
  if (view()->moveCaretTo(node.handle(), offset, !extendSelection))
    emitSelectionChanged();
  view()->ensureCaretVisible();
#endif // TDEHTML_NO_CARET
}

TDEHTMLPart::CaretDisplayPolicy TDEHTMLPart::caretDisplayPolicyNonFocused() const
{
#ifndef TDEHTML_NO_CARET
  return (CaretDisplayPolicy)view()->caretDisplayPolicyNonFocused();
#else // TDEHTML_NO_CARET
  return CaretInvisible;
#endif // TDEHTML_NO_CARET
}

void TDEHTMLPart::setCaretDisplayPolicyNonFocused(CaretDisplayPolicy policy)
{
#ifndef TDEHTML_NO_CARET
  view()->setCaretDisplayPolicyNonFocused(policy);
#endif // TDEHTML_NO_CARET
}

void TDEHTMLPart::setCaretVisible(bool show)
{
#ifndef TDEHTML_NO_CARET
  if (show) {

    NodeImpl *caretNode = xmlDocImpl()->focusNode();
    if (isCaretMode() || isEditable()
	|| (caretNode && caretNode->contentEditable())) {
      view()->caretOn();
    }/*end if*/

  } else {

    view()->caretOff();

  }/*end if*/
#endif // TDEHTML_NO_CARET
}

void TDEHTMLPart::findTextBegin()
{
  d->m_findPos = -1;
  d->m_findNode = 0;
  d->m_findPosEnd = -1;
  d->m_findNodeEnd= 0;
  d->m_findPosStart = -1;
  d->m_findNodeStart = 0;
  d->m_findNodePrevious = 0;
  delete d->m_find;
  d->m_find = 0L;
}

bool TDEHTMLPart::initFindNode( bool selection, bool reverse, bool fromCursor )
{
    if ( !d->m_doc )
        return false;

    DOM::NodeImpl* firstNode = 0L;
    if (d->m_doc->isHTMLDocument())
      firstNode = static_cast<HTMLDocumentImpl*>(d->m_doc)->body();
    else
      firstNode = d->m_doc;

    if ( !firstNode )
    {
      //kdDebug(6050) << k_funcinfo << "no first node (body or doc) -> return false" << endl;
      return false;
    }
    if ( firstNode->id() == ID_FRAMESET )
    {
      //kdDebug(6050) << k_funcinfo << "FRAMESET -> return false" << endl;
      return false;
    }

    if ( selection && hasSelection() )
    {
      //kdDebug(6050) << k_funcinfo << "using selection" << endl;
      if ( !fromCursor )
      {
        d->m_findNode = reverse ? d->m_selectionEnd.handle() : d->m_selectionStart.handle();
        d->m_findPos = reverse ? d->m_endOffset : d->m_startOffset;
      }
      d->m_findNodeEnd = reverse ? d->m_selectionStart.handle() : d->m_selectionEnd.handle();
      d->m_findPosEnd = reverse ? d->m_startOffset : d->m_endOffset;
      d->m_findNodeStart = !reverse ? d->m_selectionStart.handle() : d->m_selectionEnd.handle();
      d->m_findPosStart = !reverse ? d->m_startOffset : d->m_endOffset;
      d->m_findNodePrevious = d->m_findNodeStart;
    }
    else // whole document
    {
      //kdDebug(6050) << k_funcinfo << "whole doc" << endl;
      if ( !fromCursor )
      {
        d->m_findNode = firstNode;
        d->m_findPos = reverse ? -1 : 0;
      }
      d->m_findNodeEnd = reverse ? firstNode : 0;
      d->m_findPosEnd = reverse ? 0 : -1;
      d->m_findNodeStart = !reverse ? firstNode : 0;
      d->m_findPosStart = !reverse ? 0 : -1;
      d->m_findNodePrevious = d->m_findNodeStart;
      if ( reverse )
      {
        // Need to find out the really last object, to start from it
        tdehtml::RenderObject* obj = d->m_findNode ? d->m_findNode->renderer() : 0;
        if ( obj )
        {
          // find the last object in the render tree
          while ( obj->lastChild() )
          {
              obj = obj->lastChild();
          }
          // now get the last object with a NodeImpl associated
          while ( !obj->element() && obj->objectAbove() )
          {
             obj = obj->objectAbove();
          }
          d->m_findNode = obj->element();
        }
      }
    }
    return true;
}

// Old method (its API limits the available features - remove in KDE-4)
bool TDEHTMLPart::findTextNext( const TQString &str, bool forward, bool caseSensitive, bool isRegExp )
{
    if ( !initFindNode( false, !forward, d->m_findNode ) )
      return false;
    while(1)
    {
        if( (d->m_findNode->nodeType() == Node::TEXT_NODE || d->m_findNode->nodeType() == Node::CDATA_SECTION_NODE) && d->m_findNode->renderer() )
        {
            DOMString nodeText = d->m_findNode->nodeValue();
            DOMStringImpl *t = nodeText.implementation();
            TQConstString s(t->s, t->l);

            int matchLen = 0;
            if ( isRegExp ) {
              TQRegExp matcher( str );
              matcher.setCaseSensitive( caseSensitive );
              d->m_findPos = matcher.search(s.string(), d->m_findPos+1);
              if ( d->m_findPos != -1 )
                matchLen = matcher.matchedLength();
            }
            else {
              d->m_findPos = s.string().find(str, d->m_findPos+1, caseSensitive);
              matchLen = str.length();
            }

            if(d->m_findPos != -1)
            {
                int x = 0, y = 0;
                if(static_cast<tdehtml::RenderText *>(d->m_findNode->renderer())
                  ->posOfChar(d->m_findPos, x, y))
                    d->m_view->setContentsPos(x-50, y-50);

                d->m_selectionStart = d->m_findNode;
                d->m_startOffset = d->m_findPos;
                d->m_selectionEnd = d->m_findNode;
                d->m_endOffset = d->m_findPos + matchLen;
                d->m_startBeforeEnd = true;

                d->m_doc->setSelection( d->m_selectionStart.handle(), d->m_startOffset,
                                        d->m_selectionEnd.handle(), d->m_endOffset );
                emitSelectionChanged();
                return true;
            }
        }
        d->m_findPos = -1;

        NodeImpl *next;

        if ( forward )
        {
          next = d->m_findNode->firstChild();

          if(!next) next = d->m_findNode->nextSibling();
          while(d->m_findNode && !next) {
              d->m_findNode = d->m_findNode->parentNode();
              if( d->m_findNode ) {
                  next = d->m_findNode->nextSibling();
              }
          }
        }
        else
        {
          next = d->m_findNode->lastChild();

          if (!next ) next = d->m_findNode->previousSibling();
          while ( d->m_findNode && !next )
          {
            d->m_findNode = d->m_findNode->parentNode();
            if( d->m_findNode )
            {
              next = d->m_findNode->previousSibling();
            }
          }
        }

        d->m_findNode = next;
        if(!d->m_findNode) return false;
    }
}


void TDEHTMLPart::slotFind()
{
  KParts::ReadOnlyPart *part = currentFrame();
  if (!part)
    return;
  if (!part->inherits("TDEHTMLPart") )
  {
      kdError(6000) << "slotFind: part is a " << part->className() << ", can't do a search into it" << endl;
      return;
  }
  static_cast<TDEHTMLPart *>( part )->findText();
}

void TDEHTMLPart::slotFindNext()
{
  KParts::ReadOnlyPart *part = currentFrame();
  if (!part)
    return;
  if (!part->inherits("TDEHTMLPart") )
  {
      kdError(6000) << "slotFindNext: part is a " << part->className() << ", can't do a search into it" << endl;
      return;
  }
  static_cast<TDEHTMLPart *>( part )->findTextNext();
}

void TDEHTMLPart::slotFindPrev()
{
  KParts::ReadOnlyPart *part = currentFrame();
  if (!part)
    return;
  if (!part->inherits("TDEHTMLPart") )
  {
      kdError(6000) << "slotFindNext: part is a " << part->className() << ", can't do a search into it" << endl;
      return;
  }
  static_cast<TDEHTMLPart *>( part )->findTextNext( true ); // reverse
}

void TDEHTMLPart::slotFindDone()
{
  // ### remove me
}

void TDEHTMLPart::slotFindAheadText()
{
#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
  KParts::ReadOnlyPart *part = currentFrame();
  if (!part)
    return;
  if (!part->inherits("TDEHTMLPart") )
  {
      kdError(6000) << "slotFindNext: part is a " << part->className() << ", can't do a search into it" << endl;
      return;
  }
  static_cast<TDEHTMLPart *>( part )->view()->startFindAhead( false );
#endif // TDEHTML_NO_TYPE_AHEAD_FIND
}

void TDEHTMLPart::slotFindAheadLink()
{
#ifndef TDEHTML_NO_TYPE_AHEAD_FIND
  KParts::ReadOnlyPart *part = currentFrame();
  if (!part)
    return;
  if (!part->inherits("TDEHTMLPart") )
  {
      kdError(6000) << "slotFindNext: part is a " << part->className() << ", can't do a search into it" << endl;
      return;
  }
  static_cast<TDEHTMLPart *>( part )->view()->startFindAhead( true );
#endif // TDEHTML_NO_TYPE_AHEAD_FIND
}

void TDEHTMLPart::enableFindAheadActions( bool enable )
{
  // only the topmost one has shortcuts
  TDEHTMLPart* p = this;
  while( p->parentPart())
    p = p->parentPart();
  p->d->m_paFindAheadText->setEnabled( enable );
  p->d->m_paFindAheadLinks->setEnabled( enable );
}

void TDEHTMLPart::slotFindDialogDestroyed()
{
  d->m_lastFindState.options = d->m_findDialog->options();
  d->m_lastFindState.history = d->m_findDialog->findHistory();
  d->m_findDialog->deleteLater();
  d->m_findDialog = 0L;
}

void TDEHTMLPart::findText()
{
  // First do some init to make sure we can search in this frame
  if ( !d->m_doc )
    return;

  // Raise if already opened
  if ( d->m_findDialog )
  {
    KWin::activateWindow( d->m_findDialog->winId() );
    return;
  }

  // The lineedit of the dialog would make tdehtml lose its selection, otherwise
#ifndef TQT_NO_CLIPBOARD
  disconnect( kapp->clipboard(), TQ_SIGNAL(selectionChanged()), this, TQ_SLOT(slotClearSelection()) );
#endif

  // Now show the dialog in which the user can choose options.
  d->m_findDialog = new KFindDialog( false /*non-modal*/, widget(), "tdehtmlfind" );
  d->m_findDialog->setHasSelection( hasSelection() );
  d->m_findDialog->setHasCursor( d->m_findNode != 0 );
  if ( d->m_findNode ) // has a cursor -> default to 'FromCursor'
    d->m_lastFindState.options |= KFindDialog::FromCursor;

  // TODO? optionsDialog.setPattern( d->m_lastFindState.text );
  d->m_findDialog->setFindHistory( d->m_lastFindState.history );
  d->m_findDialog->setOptions( d->m_lastFindState.options );

  d->m_lastFindState.options = -1; // force update in findTextNext
  d->m_lastFindState.last_dir = -1;

  d->m_findDialog->show();
  connect( d->m_findDialog, TQ_SIGNAL(okClicked()), this, TQ_SLOT(slotFindNext()) );
  connect( d->m_findDialog, TQ_SIGNAL(finished()), this, TQ_SLOT(slotFindDialogDestroyed()) );

  findText( d->m_findDialog->pattern(), 0 /*options*/, widget(), d->m_findDialog );
}

void TDEHTMLPart::findText( const TQString &str, long options, TQWidget *parent, KFindDialog *findDialog )
{
  // First do some init to make sure we can search in this frame
  if ( !d->m_doc )
    return;

#ifndef TQT_NO_CLIPBOARD
  connect( kapp->clipboard(), TQ_SIGNAL(selectionChanged()), TQ_SLOT(slotClearSelection()) );
#endif

  // Create the KFind object
  delete d->m_find;
  d->m_find = new KFind( str, options, parent, findDialog );
  d->m_find->closeFindNextDialog(); // we use KFindDialog non-modal, so we don't want other dlg popping up
  connect( d->m_find, TQ_SIGNAL( highlight( const TQString &, int, int ) ),
           this, TQ_SLOT( slotHighlight( const TQString &, int, int ) ) );
  //connect(d->m_find, TQ_SIGNAL( findNext() ),
  //        this, TQ_SLOT( slotFindNext() ) );

  if ( !findDialog )
  {
    d->m_lastFindState.options = options;
    initFindNode( options & KFindDialog::SelectedText,
                  options & KFindDialog::FindBackwards,
                  options & KFindDialog::FromCursor );
  }
}

bool TDEHTMLPart::findTextNext()
{
  return findTextNext( false );
}

// New method
bool TDEHTMLPart::findTextNext( bool reverse )
{
  if (!d->m_find)
  {
    // We didn't show the find dialog yet, let's do it then (#49442)
    findText();
    return false;
  }

  view()->updateFindAheadTimeout();
  long options = 0;
  if ( d->m_findDialog ) // 0 when we close the dialog
  {
    if ( d->m_find->pattern() != d->m_findDialog->pattern() ) {
      d->m_find->setPattern( d->m_findDialog->pattern() );
      d->m_find->resetCounts();
    }
    options = d->m_findDialog->options();
    if ( d->m_lastFindState.options != options )
    {
      d->m_find->setOptions( options );

      if ( options & KFindDialog::SelectedText )
        Q_ASSERT( hasSelection() );

      long difference = d->m_lastFindState.options ^ options;
      if ( difference & (KFindDialog::SelectedText | KFindDialog::FromCursor ) )
      {
          // Important options changed -> reset search range
        (void) initFindNode( options & KFindDialog::SelectedText,
                             options & KFindDialog::FindBackwards,
                             options & KFindDialog::FromCursor );
      }
      d->m_lastFindState.options = options;
    }
  } else
    options = d->m_lastFindState.options;
  if( reverse )
    options = options ^ KFindDialog::FindBackwards;
  if( d->m_find->options() != options )
    d->m_find->setOptions( options );

  // Changing find direction. Start and end nodes must be switched.
  // Additionally since d->m_findNode points after the last node
  // that was searched, it needs to be "after" it in the opposite direction.
  if( d->m_lastFindState.last_dir != -1
      && bool( d->m_lastFindState.last_dir ) != bool( options & KFindDialog::FindBackwards ))
  {
    tqSwap( d->m_findNodeEnd, d->m_findNodeStart );
    tqSwap( d->m_findPosEnd, d->m_findPosStart );
    tqSwap( d->m_findNode, d->m_findNodePrevious );
    // d->m_findNode now point at the end of the last searched line - advance one node
    tdehtml::RenderObject* obj = d->m_findNode ? d->m_findNode->renderer() : 0;
    tdehtml::RenderObject* end = d->m_findNodeEnd ? d->m_findNodeEnd->renderer() : 0;
    if ( obj == end )
      obj = 0L;
    else if ( obj )
    {
      do {
        obj = (options & KFindDialog::FindBackwards) ? obj->objectAbove() : obj->objectBelow();
      } while ( obj && ( !obj->element() || obj->isInlineContinuation() ) );
    }
    if ( obj )
      d->m_findNode = obj->element();
    else
      d->m_findNode = 0;
  }
  d->m_lastFindState.last_dir = ( options & KFindDialog::FindBackwards ) ? 1 : 0;

  KFind::Result res = KFind::NoMatch;
  tdehtml::RenderObject* obj = d->m_findNode ? d->m_findNode->renderer() : 0;
  tdehtml::RenderObject* end = d->m_findNodeEnd ? d->m_findNodeEnd->renderer() : 0;
  tdehtml::RenderTextArea *tmpTextArea=0L;
  //kdDebug(6050) << k_funcinfo << "obj=" << obj << " end=" << end << endl;
  while( res == KFind::NoMatch )
  {
    if ( d->m_find->needData() )
    {
      if ( !obj ) {
        //kdDebug(6050) << k_funcinfo << "obj=0 -> done" << endl;
        break; // we're done
      }
      //kdDebug(6050) << k_funcinfo << " gathering data" << endl;
      // First make up the TQString for the current 'line' (i.e. up to \n)
      // We also want to remember the DOMNode for every portion of the string.
      // We store this in an index->node list.

      d->m_stringPortions.clear();
      bool newLine = false;
      TQString str;
      DOM::NodeImpl* lastNode = d->m_findNode;
      while ( obj && !newLine )
      {
        // Grab text from render object
        TQString s;
        bool renderAreaText = obj->parent() && (TQCString(obj->parent()->renderName())== "RenderTextArea");
        bool renderLineText = (TQCString(obj->renderName())== "RenderLineEdit");
        if ( renderAreaText )
        {
          tdehtml::RenderTextArea *parent= static_cast<tdehtml::RenderTextArea *>(obj->parent());
          s = parent->text();
          s = s.replace(0xa0, ' ');
          tmpTextArea = parent;
        }
        else if ( renderLineText )
        {
          tdehtml::RenderLineEdit *parentLine= static_cast<tdehtml::RenderLineEdit *>(obj);
          if (parentLine->widget()->echoMode() == TQLineEdit::Normal)
            s = parentLine->widget()->text();
          s = s.replace(0xa0, ' ');
        }
        else if ( obj->isText() )
        {
          bool isLink = false;

          // checks whether the node has a <A> parent
          if ( options & FindLinksOnly )
          {
            DOM::NodeImpl *parent = obj->element();
            while ( parent )
            {
              if ( parent->nodeType() == Node::ELEMENT_NODE && parent->id() == ID_A )
              {
                isLink = true;
                break;
              }
              parent = parent->parentNode();
            }
          }
          else
          {
            isLink = true;
          }

          if ( isLink && obj->parent()!=tmpTextArea )
          {
            s = static_cast<tdehtml::RenderText *>(obj)->data().string();
            s = s.replace(0xa0, ' ');
          }
        }
        else if ( obj->isBR() )
          s = '\n';
        else if ( !obj->isInline() && !str.isEmpty() )
          s = '\n';

        if ( lastNode == d->m_findNodeEnd )
          s.truncate( d->m_findPosEnd );
        if ( !s.isEmpty() )
        {
          newLine = s.find( '\n' ) != -1; // did we just get a newline?
          if( !( options & KFindDialog::FindBackwards ))
          {
            //kdDebug(6050) << "StringPortion: " << index << "-" << index+s.length()-1 << " -> " << lastNode << endl;
            d->m_stringPortions.append( TDEHTMLPartPrivate::StringPortion( str.length(), lastNode ) );
            str += s;
          }
          else // KFind itself can search backwards, so str must not be built backwards
          {
            for( TQValueList<TDEHTMLPartPrivate::StringPortion>::Iterator it = d->m_stringPortions.begin();
                 it != d->m_stringPortions.end();
                 ++it )
                (*it).index += s.length();
            d->m_stringPortions.prepend( TDEHTMLPartPrivate::StringPortion( 0, lastNode ) );
            str.prepend( s );
          }
        }
        // Compare obj and end _after_ we processed the 'end' node itself
        if ( obj == end )
          obj = 0L;
        else
        {
          // Move on to next object (note: if we found a \n already, then obj (and lastNode)
          // will point to the _next_ object, i.e. they are in advance.
          do {
            // We advance until the next RenderObject that has a NodeImpl as its element().
            // Otherwise (if we keep the 'last node', and it has a '\n') we might be stuck
            // on that object forever...
            obj = (options & KFindDialog::FindBackwards) ? obj->objectAbove() : obj->objectBelow();
          } while ( obj && ( !obj->element() || obj->isInlineContinuation() ) );
        }
        if ( obj )
          lastNode = obj->element();
        else
          lastNode = 0;
      } // end while
      //kdDebug()<<" str : "<<str<<endl;
      if ( !str.isEmpty() )
      {
        d->m_find->setData( str, d->m_findPos );
      }

      d->m_findPos = -1; // not used during the findnext loops. Only during init.
      d->m_findNodePrevious = d->m_findNode;
      d->m_findNode = lastNode;
    }
    if ( !d->m_find->needData() ) // happens if str was empty
    {
      // Let KFind inspect the text fragment, and emit highlighted if a match is found
      res = d->m_find->find();
    }
  } // end while

  if ( res == KFind::NoMatch ) // i.e. we're done
  {
    kdDebug() << "No more matches." << endl;
    if ( !(options & FindNoPopups) && d->m_find->shouldRestart() )
    {
      //kdDebug(6050) << "Restarting" << endl;
      initFindNode( false, options & KFindDialog::FindBackwards, false );
      d->m_find->resetCounts();
      findTextNext( reverse );
    }
    else // really done
    {
      //kdDebug(6050) << "Finishing" << endl;
      //delete d->m_find;
      //d->m_find = 0L;
      initFindNode( false, options & KFindDialog::FindBackwards, false );
      d->m_find->resetCounts();
      slotClearSelection();
    }
    kdDebug() << "Dialog closed." << endl;
  }

  return res == KFind::Match;
}

void TDEHTMLPart::slotHighlight( const TQString& /*text*/, int index, int length )
{
  //kdDebug(6050) << "slotHighlight index=" << index << " length=" << length << endl;
  TQValueList<TDEHTMLPartPrivate::StringPortion>::Iterator it = d->m_stringPortions.begin();
  const TQValueList<TDEHTMLPartPrivate::StringPortion>::Iterator itEnd = d->m_stringPortions.end();
  TQValueList<TDEHTMLPartPrivate::StringPortion>::Iterator prev = it;
  // We stop at the first portion whose index is 'greater than', and then use the previous one
  while ( it != itEnd && (*it).index <= index )
  {
    prev = it;
    ++it;
  }
  Q_ASSERT ( prev != itEnd );
  DOM::NodeImpl* node = (*prev).node;
  Q_ASSERT( node );

  d->m_selectionStart = node;
  d->m_startOffset = index - (*prev).index;

  tdehtml::RenderObject* obj = node->renderer();
  tdehtml::RenderTextArea *parent = 0L;
  tdehtml::RenderLineEdit *parentLine = 0L;
  bool renderLineText =false;

  TQRect highlightedRect;
  bool renderAreaText =false;
  Q_ASSERT( obj );
  if ( obj )
  {
    int x = 0, y = 0;
    renderAreaText = (TQCString(obj->parent()->renderName())== "RenderTextArea");
    renderLineText = (TQCString(obj->renderName())== "RenderLineEdit");


    if( renderAreaText )
      parent= static_cast<tdehtml::RenderTextArea *>(obj->parent());
    if ( renderLineText )
      parentLine= static_cast<tdehtml::RenderLineEdit *>(obj);
    if ( !renderLineText )
      //if (static_cast<tdehtml::RenderText *>(node->renderer())
      //    ->posOfChar(d->m_startOffset, x, y))
      {
        int dummy;
        static_cast<tdehtml::RenderText *>(node->renderer())
          ->caretPos( d->m_startOffset, false, x, y, dummy, dummy ); // more precise than posOfChar
        //kdDebug(6050) << "topleft: " << x << "," << y << endl;
        if ( x != -1 || y != -1 )
        {
          int gox = d->m_view->contentsX();
          if (x+50 > d->m_view->contentsX() + d->m_view->visibleWidth())
              gox = x - d->m_view->visibleWidth() + 50;
          if (x-10 < d->m_view->contentsX())
              gox = x - d->m_view->visibleWidth() - 10;
          if (gox < 0) gox = 0;
          d->m_view->setContentsPos(gox, y-50);
          highlightedRect.setTopLeft( d->m_view->mapToGlobal(TQPoint(x, y)) );
        }
      }
  }
  // Now look for end node
  it = prev; // no need to start from beginning again
  while ( it != itEnd && (*it).index < index + length )
  {
    prev = it;
    ++it;
  }
  Q_ASSERT ( prev != itEnd );

  d->m_selectionEnd = (*prev).node;
  d->m_endOffset = index + length - (*prev).index;
  d->m_startBeforeEnd = true;

  // if the selection is limited to a single link, that link gets focus
  if(d->m_selectionStart == d->m_selectionEnd)
  {
    bool isLink = false;

    // checks whether the node has a <A> parent
    DOM::NodeImpl *parent = d->m_selectionStart.handle();
    while ( parent )
    {
      if ( parent->nodeType() == Node::ELEMENT_NODE && parent->id() == ID_A )
      {
        isLink = true;
        break;
      }
      parent = parent->parentNode();
    }

    if(isLink == true)
    {
      d->m_doc->setFocusNode( parent );
    }
  }

#if 0
  kdDebug(6050) << "slotHighlight: " << d->m_selectionStart.handle() << "," << d->m_startOffset << " - " <<
    d->m_selectionEnd.handle() << "," << d->m_endOffset << endl;
  it = d->m_stringPortions.begin();
  for ( ; it != d->m_stringPortions.end() ; ++it )
    kdDebug(6050) << "  StringPortion: from index=" << (*it).index << " -> node=" << (*it).node << endl;
#endif
  if( renderAreaText )
  {
    if( parent )
      parent->highLightWord( length, d->m_endOffset-length );
  }
  else if ( renderLineText )
  {
    if( parentLine )
      parentLine->highLightWord( length, d->m_endOffset-length );
  }
  else
  {
    d->m_doc->setSelection( d->m_selectionStart.handle(), d->m_startOffset,
                            d->m_selectionEnd.handle(), d->m_endOffset );
    if (d->m_selectionEnd.handle()->renderer() )
    {
      int x, y, height, dummy;
      static_cast<tdehtml::RenderText *>(d->m_selectionEnd.handle()->renderer())
          ->caretPos( d->m_endOffset, false, x, y, dummy, height ); // more precise than posOfChar
      //kdDebug(6050) << "bottomright: " << x << "," << y+height << endl;
      if ( x != -1 || y != -1 )
      {
        // if ( static_cast<tdehtml::RenderText *>(d->m_selectionEnd.handle()->renderer())
        //  ->posOfChar(d->m_endOffset-1, x, y))
        highlightedRect.setBottomRight( d->m_view->mapToGlobal( TQPoint(x, y+height) ) );
      }
    }
  }
  emitSelectionChanged();

  // make the finddialog move away from the selected area
  if ( d->m_findDialog && !highlightedRect.isNull() )
  {
    highlightedRect.moveBy( -d->m_view->contentsX(), -d->m_view->contentsY() );
    //kdDebug(6050) << "avoiding " << highlightedRect << endl;
    KDialog::avoidArea( d->m_findDialog, highlightedRect );
  }
}

TQString TDEHTMLPart::selectedTextAsHTML() const
{
  if(!hasSelection()) {
    kdDebug() << "selectedTextAsHTML(): selection is not valid.  Returning empty selection" << endl;
    return TQString();
  }
  if(d->m_startOffset < 0 || d->m_endOffset <0) {
    kdDebug() << "invalid values for end/startOffset " << d->m_startOffset << " " << d->m_endOffset << endl;
    return TQString();
  }
  DOM::Range r = selection();
  if(r.isNull() || r.isDetached())
    return TQString();
  int exceptioncode = 0; //ignore the result
  return r.handle()->toHTML(exceptioncode).string();
}

TQString TDEHTMLPart::selectedText() const
{
  bool hasNewLine = true;
  bool seenTDTag = false;
  TQString text;
  DOM::Node n = d->m_selectionStart;
  while(!n.isNull()) {
      if(n.nodeType() == DOM::Node::TEXT_NODE && n.handle()->renderer()) {
        DOM::DOMStringImpl *dstr = static_cast<DOM::TextImpl*>(n.handle())->renderString();
        TQString str(dstr->s, dstr->l);
	if(!str.isEmpty()) {
          if(seenTDTag) {
	    text += "  ";
	    seenTDTag = false;
	  }
          hasNewLine = false;
          if(n == d->m_selectionStart && n == d->m_selectionEnd)
            text = str.mid(d->m_startOffset, d->m_endOffset - d->m_startOffset);
          else if(n == d->m_selectionStart)
            text = str.mid(d->m_startOffset);
          else if(n == d->m_selectionEnd)
            text += str.left(d->m_endOffset);
          else
            text += str;
	}
      }
      else {
        // This is our simple HTML -> ASCII transformation:
        unsigned short id = n.elementId();
        switch(id) {
	  case ID_TEXTAREA:
	    text += static_cast<HTMLTextAreaElementImpl*>(n.handle())->value().string();
	    break;
	  case ID_INPUT:
            if (static_cast<HTMLInputElementImpl*>(n.handle())->inputType() != HTMLInputElementImpl::PASSWORD)
  	      text += static_cast<HTMLInputElementImpl*>(n.handle())->value().string();
	    break;
	  case ID_SELECT:
	    text += static_cast<HTMLSelectElementImpl*>(n.handle())->value().string();
	    break;
          case ID_BR:
            text += "\n";
            hasNewLine = true;
            break;
          case ID_IMG:
	    text += static_cast<HTMLImageElementImpl*>(n.handle())->altText().string();
	    break;
          case ID_TD:
	    break;
          case ID_TH:
          case ID_HR:
          case ID_OL:
          case ID_UL:
          case ID_LI:
          case ID_DD:
          case ID_DL:
          case ID_DT:
          case ID_PRE:
          case ID_BLOCKQUOTE:
          case ID_DIV:
            if (!hasNewLine)
               text += "\n";
            hasNewLine = true;
            break;
          case ID_P:
          case ID_TR:
          case ID_H1:
          case ID_H2:
          case ID_H3:
          case ID_H4:
          case ID_H5:
          case ID_H6:
            if (!hasNewLine)
               text += "\n";
//            text += "\n";
            hasNewLine = true;
            break;
        }
      }
      if(n == d->m_selectionEnd) break;
      DOM::Node next = n.firstChild();
      if(next.isNull()) next = n.nextSibling();
      while( next.isNull() && !n.parentNode().isNull() ) {
        n = n.parentNode();
        next = n.nextSibling();
        unsigned short id = n.elementId();
        switch(id) {
          case ID_TD:
	    seenTDTag = true; //Add two spaces after a td if then followed by text.
	    break;
          case ID_TH:
          case ID_HR:
          case ID_OL:
          case ID_UL:
          case ID_LI:
          case ID_DD:
          case ID_DL:
          case ID_DT:
          case ID_PRE:
          case ID_BLOCKQUOTE:
          case ID_DIV:
	    seenTDTag = false;
            if (!hasNewLine)
               text += "\n";
            hasNewLine = true;
            break;
          case ID_P:
          case ID_TR:
          case ID_H1:
          case ID_H2:
          case ID_H3:
          case ID_H4:
          case ID_H5:
          case ID_H6:
            if (!hasNewLine)
               text += "\n";
//            text += "\n";
            hasNewLine = true;
            break;
        }
      }

      n = next;
    }

    if(text.isEmpty())
        return TQString();

    int start = 0;
    int end = text.length();

    // Strip leading LFs
    while ((start < end) && (text[start] == '\n'))
       ++start;

    // Strip excessive trailing LFs
    while ((start < (end-1)) && (text[end-1] == '\n') && (text[end-2] == '\n'))
       --end;

    return text.mid(start, end-start);
}

bool TDEHTMLPart::hasSelection() const
{
  if ( d->m_selectionStart.isNull() || d->m_selectionEnd.isNull() )
      return false;
  if ( d->m_selectionStart == d->m_selectionEnd &&
       d->m_startOffset == d->m_endOffset )
      return false; // empty
  return true;
}

DOM::Range TDEHTMLPart::selection() const
{
    if( d->m_selectionStart.isNull() || d->m_selectionEnd.isNull() )
        return DOM::Range();
    DOM::Range r = document().createRange();
    RangeImpl *rng = r.handle();
    int exception = 0;
    NodeImpl *n = d->m_selectionStart.handle();
    if(!n->parentNode() ||
       !n->renderer() ||
       (!n->renderer()->isReplaced() && !n->renderer()->isBR())) {
        rng->setStart( n, d->m_startOffset, exception );
	if(exception) {
	    kdDebug(6000) << "1 -selection() threw the exception " << exception << ".  Returning empty range." << endl;
	    return DOM::Range();
	}
    } else {
        int o_start = 0;
        while ((n = n->previousSibling()))
            o_start++;
	rng->setStart( d->m_selectionStart.parentNode().handle(), o_start + d->m_startOffset, exception );
	if(exception) {
	    kdDebug(6000) << "2 - selection() threw the exception " << exception << ".  Returning empty range." << endl;
	    return DOM::Range();
	}

    }

    n = d->m_selectionEnd.handle();
    if(!n->parentNode() ||
       !n->renderer() ||
       (!n->renderer()->isReplaced() && !n->renderer()->isBR())) {

	rng->setEnd( n, d->m_endOffset, exception );
	if(exception) {
	    kdDebug(6000) << "3 - selection() threw the exception " << exception << ".  Returning empty range." << endl;
	    return DOM::Range();
	}

    } else {
        int o_end = 0;
        while ((n = n->previousSibling()))
            o_end++;
	rng->setEnd( d->m_selectionEnd.parentNode().handle(), o_end + d->m_endOffset, exception);
	if(exception) {
	    kdDebug(6000) << "4 - selection() threw the exception " << exception << ".  Returning empty range." << endl;
	    return DOM::Range();
	}

    }

    return r;
}

void TDEHTMLPart::selection(DOM::Node &s, long &so, DOM::Node &e, long &eo) const
{
    s = d->m_selectionStart;
    so = d->m_startOffset;
    e = d->m_selectionEnd;
    eo = d->m_endOffset;
}

void TDEHTMLPart::setSelection( const DOM::Range &r )
{
    // Quick-fix: a collapsed range shouldn't select the whole node.
    // The real problem is in RenderCanvas::setSelection though (when index==0 the whole node is selected).
    if ( r.collapsed() )
        slotClearSelection();
    else {
        d->m_selectionStart = r.startContainer();
        d->m_startOffset = r.startOffset();
        d->m_selectionEnd = r.endContainer();
        d->m_endOffset = r.endOffset();
        d->m_doc->setSelection(d->m_selectionStart.handle(),d->m_startOffset,
                               d->m_selectionEnd.handle(),d->m_endOffset);
#ifndef TDEHTML_NO_CARET
        bool v = d->m_view->placeCaret();
        emitCaretPositionChanged(v ? d->caretNode() : 0, d->caretOffset());
#endif
    }
}

void TDEHTMLPart::slotClearSelection()
{
    bool hadSelection = hasSelection();
#ifndef TDEHTML_NO_CARET
    //kdDebug(6000) << "d->m_selectionStart " << d->m_selectionStart.handle()
    //		<< " d->m_selectionEnd " << d->m_selectionEnd.handle() << endl;
    // nothing, leave selection parameters as is
#else
    d->m_selectionStart = 0;
    d->m_startOffset = 0;
    d->m_selectionEnd = 0;
    d->m_endOffset = 0;
#endif
    if ( d->m_doc ) d->m_doc->clearSelection();
    if ( hadSelection )
      emitSelectionChanged();
#ifndef TDEHTML_NO_CARET
    bool v = d->m_view->placeCaret();
    emitCaretPositionChanged(v ? d->caretNode() : 0, d->caretOffset());
#endif
}

void TDEHTMLPart::resetHoverText()
{
   if( !d->m_overURL.isEmpty() ) // Only if we were showing a link
   {
     d->m_overURL = d->m_overURLTarget = TQString();
     emit onURL( TQString() );
     // revert to default statusbar text
     setStatusBarText(TQString(), BarHoverText);
     emit d->m_extension->mouseOverInfo(0);
  }
}

void TDEHTMLPart::overURL( const TQString &url, const TQString &target, bool /*shiftPressed*/ )
{
  KURL u = completeURL(url);

  // special case for <a href="">
  if ( url.isEmpty() )
    u.setFileName( url );

  emit onURL( url );

  if ( url.isEmpty() ) {
    setStatusBarText(u.htmlURL(), BarHoverText);
    return;
  }

  if (url.find( TQString::fromLatin1( "javascript:" ),0, false ) == 0 ) {
    TQString jscode = KURL::decode_string( url.mid( url.find( "javascript:", 0, false ) ) );
    jscode = KStringHandler::rsqueeze( jscode, 80 ); // truncate if too long
    if (url.startsWith("javascript:window.open"))
      jscode += i18n(" (In new window)");
    setStatusBarText( TQStyleSheet::escape( jscode ), BarHoverText );
    return;
  }

  KFileItem item(u, TQString(), KFileItem::Unknown);
  emit d->m_extension->mouseOverInfo(&item);

  TQString com;

  KMimeType::Ptr typ = KMimeType::findByURL( u );

  if ( typ )
    com = typ->comment( u, false );

  if ( !u.isValid() ) {
    setStatusBarText(u.htmlURL(), BarHoverText);
    return;
  }

  if ( u.isLocalFile() )
  {
    // TODO : use TDEIO::stat() and create a KFileItem out of its result,
    // to use KFileItem::statusBarText()
    TQCString path = TQFile::encodeName( u.path() );

    struct stat buff;
    bool ok = !stat( path.data(), &buff );

    struct stat lbuff;
    if (ok) ok = !lstat( path.data(), &lbuff );

    TQString text = u.htmlURL();
    TQString text2 = text;

    if (ok && S_ISLNK( lbuff.st_mode ) )
    {
      TQString tmp;
      if ( com.isNull() )
        tmp = i18n( "Symbolic Link");
      else
        tmp = i18n("%1 (Link)").arg(com);
      char buff_two[1024];
      text += " -> ";
      int n = readlink ( path.data(), buff_two, 1022);
      if (n == -1)
      {
        text2 += "  ";
        text2 += tmp;
        setStatusBarText(text2, BarHoverText);
        return;
      }
      buff_two[n] = 0;

      text += buff_two;
      text += "  ";
      text += tmp;
    }
    else if ( ok && S_ISREG( buff.st_mode ) )
    {
      if (buff.st_size < 1024)
        text = i18n("%2 (%1 bytes)").arg((long) buff.st_size).arg(text2); // always put the URL last, in case it contains '%'
      else
      {
        float d = (float) buff.st_size/1024.0;
        text = i18n("%2 (%1 K)").arg(TDEGlobal::locale()->formatNumber(d, 2)).arg(text2); // was %.2f
      }
      text += "  ";
      text += com;
    }
    else if ( ok && S_ISDIR( buff.st_mode ) )
    {
      text += "  ";
      text += com;
    }
    else
    {
      text += "  ";
      text += com;
    }
    setStatusBarText(text, BarHoverText);
  }
  else
  {
    TQString extra;
    if (target.lower() == "_blank")
    {
      extra = i18n(" (In new window)");
    }
    else if (!target.isEmpty() &&
             (target.lower() != "_top") &&
             (target.lower() != "_self") &&
             (target.lower() != "_parent"))
    {
      TDEHTMLPart *p = this;
      while (p->parentPart())
          p = p->parentPart();
      if (!p->frameExists(target))
        extra = i18n(" (In new window)");
      else
        extra = i18n(" (In other frame)");
    }

    if (u.protocol() == TQString::fromLatin1("mailto")) {
      TQString mailtoMsg /* = TQString::fromLatin1("<img src=%1>").arg(locate("icon", TQString::fromLatin1("locolor/16x16/actions/mail_send.png")))*/;
      mailtoMsg += i18n("Email to: ") + KURL::decode_string(u.path());
      TQStringList queries = TQStringList::split('&', u.query().mid(1));
      TQStringList::Iterator it = queries.begin();
      const TQStringList::Iterator itEnd = queries.end();
      for (; it != itEnd; ++it)
        if ((*it).startsWith(TQString::fromLatin1("subject=")))
          mailtoMsg += i18n(" - Subject: ") + KURL::decode_string((*it).mid(8));
        else if ((*it).startsWith(TQString::fromLatin1("cc=")))
          mailtoMsg += i18n(" - CC: ") + KURL::decode_string((*it).mid(3));
        else if ((*it).startsWith(TQString::fromLatin1("bcc=")))
          mailtoMsg += i18n(" - BCC: ") + KURL::decode_string((*it).mid(4));
      mailtoMsg = TQStyleSheet::escape(mailtoMsg);
      mailtoMsg.replace(TQRegExp("([\n\r\t]|[ ]{10})"), TQString());
      setStatusBarText("<qt>"+mailtoMsg, BarHoverText);
      return;
    }
   // Is this check necessary at all? (Frerich)
#if 0
    else if (u.protocol() == TQString::fromLatin1("http")) {
        DOM::Node hrefNode = nodeUnderMouse().parentNode();
        while (hrefNode.nodeName().string() != TQString::fromLatin1("A") && !hrefNode.isNull())
          hrefNode = hrefNode.parentNode();

        if (!hrefNode.isNull()) {
          DOM::Node hreflangNode = hrefNode.attributes().getNamedItem("HREFLANG");
          if (!hreflangNode.isNull()) {
            TQString countryCode = hreflangNode.nodeValue().string().lower();
            // Map the language code to an appropriate country code.
            if (countryCode == TQString::fromLatin1("en"))
              countryCode = TQString::fromLatin1("gb");
            TQString flagImg = TQString::fromLatin1("<img src=%1>").arg(
                locate("locale", TQString::fromLatin1("l10n/")
                + countryCode
                + TQString::fromLatin1("/flag.png")));
            emit setStatusBarText(flagImg + u.prettyURL() + extra);
          }
        }
      }
#endif
    setStatusBarText(u.htmlURL() + extra, BarHoverText);
  }
}

//
// This executes in the active part on a click or other url selection action in
// that active part.
//
void TDEHTMLPart::urlSelected( const TQString &url, int button, int state, const TQString &_target, KParts::URLArgs args )
{
  // The member var is so that slotRedirection still calls the virtual urlSelected
  // but is able to know if is opened a url. KDE4: just make urlSelected return a bool
  // and move the urlSelectedIntern code back here.
  d->m_urlSelectedOpenedURL = urlSelectedIntern( url, button, state, _target, args );
}

// Return value: true if an url was opened, false if not (e.g. error, or jumping to anchor)
bool TDEHTMLPart::urlSelectedIntern( const TQString &url, int button, int state, const TQString &_target, KParts::URLArgs args )
{
  bool hasTarget = false;

  TQString target = _target;
  if ( target.isEmpty() && d->m_doc )
    target = d->m_doc->baseTarget();
  if ( !target.isEmpty() )
      hasTarget = true;

  if ( url.find( TQString::fromLatin1( "javascript:" ), 0, false ) == 0 )
  {
    crossFrameExecuteScript( target, KURL::decode_string( url.mid( 11 ) ) );
    return false;
  }

  KURL cURL = completeURL(url);
  // special case for <a href="">  (IE removes filename, mozilla doesn't)
  if ( url.isEmpty() )
    cURL.setFileName( url ); // removes filename

  if ( !cURL.isValid() )
    // ### ERROR HANDLING
    return false;

  kdDebug(6050) << this << " urlSelected: complete URL:" << cURL.url() << " target=" << target << endl;

  if ( state & ControlButton )
  {
    args.setNewTab(true);
    emit d->m_extension->createNewWindow( cURL, args );
    return true;
  }

  if ( button == TQt::LeftButton && ( state & ShiftButton ) )
  {
    TDEIO::MetaData metaData;
    metaData["referrer"] = d->m_referrer;
    TDEHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save As" ), cURL, metaData );
    return false;
  }

  if (!checkLinkSecurity(cURL,
			 i18n( "<qt>This untrusted page links to<BR><B>%1</B>.<BR>Do you want to follow the link?" ),
			 i18n( "Follow" )))
    return false;

  args.frameName = target;

  args.metaData().insert("main_frame_request",
                         parentPart() == 0 ? "TRUE":"FALSE");
  args.metaData().insert("ssl_parent_ip", d->m_ssl_parent_ip);
  args.metaData().insert("ssl_parent_cert", d->m_ssl_parent_cert);
  args.metaData().insert("PropagateHttpHeader", "true");
  args.metaData().insert("ssl_was_in_use", d->m_ssl_in_use ? "TRUE":"FALSE");
  args.metaData().insert("ssl_activate_warnings", "TRUE");

  if ( hasTarget && target != "_self" && target != "_top" && target != "_blank" && target != "_parent" )
  {
    // unknown frame names should open in a new window.
    tdehtml::ChildFrame *frame = recursiveFrameRequest( this, cURL, args, false );
    if ( frame )
    {
      args.metaData()["referrer"] = d->m_referrer;
      requestObject( frame, cURL, args );
      return true;
    }
  }

  if (!d->m_referrer.isEmpty() && !args.metaData().contains("referrer"))
    args.metaData()["referrer"] = d->m_referrer;


  if ( button == TQt::NoButton && (state & ShiftButton) && (state & ControlButton) )
  {
    emit d->m_extension->createNewWindow( cURL, args );
    return true;
  }

  if ( state & ShiftButton)
  {
    KParts::WindowArgs winArgs;
    winArgs.lowerWindow = true;
    KParts::ReadOnlyPart *newPart = 0;
    emit d->m_extension->createNewWindow( cURL, args, winArgs, newPart );
    return true;
  }

  //If we're asked to open up an anchor in the current URL, in current window,
  //merely gotoanchor, and do not reload the new page. Note that this does
  //not apply if the URL is the same page, but without a ref
  if (cURL.hasRef() && (!hasTarget || target == "_self"))
  {
    KURL curUrl = this->url();
    if (urlcmp(cURL.url(), curUrl.url(),
              false,  // ignore trailing / diff, IE does, even if FFox doesn't
              true))  // don't care if the ref changes!
    {
      m_url = cURL;
      emit d->m_extension->openURLNotify();
      if ( !gotoAnchor( m_url.encodedHtmlRef()) )
        gotoAnchor( m_url.htmlRef() );
      emit d->m_extension->setLocationBarURL( m_url.prettyURL() );
      return false; // we jumped, but we didn't open a URL
    }
  }

  if ( !d->m_bComplete && !hasTarget )
    closeURL();

  view()->viewport()->unsetCursor();
  emit d->m_extension->openURLRequest( cURL, args );
  return true;
}

void TDEHTMLPart::slotViewDocumentSource()
{
  KURL url(m_url);
  bool isTempFile = false;
  if (!(url.isLocalFile()) && TDEHTMLPageCache::self()->isComplete(d->m_cacheId))
  {
     KTempFile sourceFile(TQString(), defaultExtension());
     if (sourceFile.status() == 0)
     {
        TDEHTMLPageCache::self()->saveData(d->m_cacheId, sourceFile.dataStream());
        url = KURL();
        url.setPath(sourceFile.name());
        isTempFile = true;
     }
  }

  (void) KRun::runURL( url, TQString::fromLatin1("text/plain"), isTempFile );
}

void TDEHTMLPart::slotViewPageInfo()
{
  TDEHTMLInfoDlg *dlg = new TDEHTMLInfoDlg(NULL, "TDEHTML Page Info Dialog", false, (WFlags)WDestructiveClose);
  dlg->_close->setGuiItem(KStdGuiItem::close());

  if (d->m_doc)
     dlg->_title->setText(d->m_doc->title().string());

  // If it's a frame, set the caption to "Frame Information"
  if ( parentPart() && d->m_doc && d->m_doc->isHTMLDocument() ) {
     dlg->setCaption(i18n("Frame Information"));
  }

  TQString editStr = TQString();

  if (!d->m_pageServices.isEmpty())
    editStr = i18n("   <a href=\"%1\">[Properties]</a>").arg(d->m_pageServices);

  TQString squeezedURL = KStringHandler::csqueeze( url().prettyURL(), 80 );
  dlg->_url->setText("<a href=\"" + url().url() + "\">" + squeezedURL + "</a>" + editStr);
  if (lastModified().isEmpty())
  {
    dlg->_lastModified->hide();
    dlg->_lmLabel->hide();
  }
  else
    dlg->_lastModified->setText(lastModified());

  const TQString& enc = encoding();
  if (enc.isEmpty()) {
    dlg->_eLabel->hide();
    dlg->_encoding->hide();
  } else {
    dlg->_encoding->setText(enc);
  }
  /* populate the list view now */
  const TQStringList headers = TQStringList::split("\n", d->m_httpHeaders);

  TQStringList::ConstIterator it = headers.begin();
  const TQStringList::ConstIterator itEnd = headers.end();

  for (; it != itEnd; ++it) {
    const TQStringList header = TQStringList::split(TQRegExp(":[ ]+"), *it);
    if (header.count() != 2)
       continue;
    new TQListViewItem(dlg->_headers, header[0], header[1]);
  }

  dlg->show();
  /* put no code here */
}


void TDEHTMLPart::slotViewFrameSource()
{
  KParts::ReadOnlyPart *frame = currentFrame();
  if ( !frame )
    return;

  KURL url = frame->url();
  bool isTempFile = false;
  if (!(url.isLocalFile()) && frame->inherits("TDEHTMLPart"))
  {
       long cacheId = static_cast<TDEHTMLPart *>(frame)->d->m_cacheId;

       if (TDEHTMLPageCache::self()->isComplete(cacheId))
       {
           KTempFile sourceFile(TQString(), defaultExtension());
           if (sourceFile.status() == 0)
           {
               TDEHTMLPageCache::self()->saveData(cacheId, sourceFile.dataStream());
               url = KURL();
               url.setPath(sourceFile.name());
               isTempFile = true;
           }
     }
  }

  (void) KRun::runURL( url, TQString::fromLatin1("text/plain"), isTempFile );
}

KURL TDEHTMLPart::backgroundURL() const
{
  // ### what about XML documents? get from CSS?
  if (!d->m_doc || !d->m_doc->isHTMLDocument())
    return KURL();

  TQString relURL = static_cast<HTMLDocumentImpl*>(d->m_doc)->body()->getAttribute( ATTR_BACKGROUND ).string();

  return KURL( m_url, relURL );
}

void TDEHTMLPart::slotSaveBackground()
{
  TDEIO::MetaData metaData;
  metaData["referrer"] = d->m_referrer;
  TDEHTMLPopupGUIClient::saveURL( d->m_view, i18n("Save Background Image As"), backgroundURL(), metaData );
}

void TDEHTMLPart::slotSaveDocument()
{
  KURL srcURL( m_url );

  if ( srcURL.fileName(false).isEmpty() )
    srcURL.setFileName( "index" + defaultExtension() );

  TDEIO::MetaData metaData;
  // Referre unknown?
  TDEHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save As" ), srcURL, metaData, "text/html", d->m_cacheId );
}

void TDEHTMLPart::slotSecurity()
{
//   kdDebug( 6050 ) << "Meta Data:" << endl
//                   << d->m_ssl_peer_cert_subject
//                   << endl
//                   << d->m_ssl_peer_cert_issuer
//                   << endl
//                   << d->m_ssl_cipher
//                   << endl
//                   << d->m_ssl_cipher_desc
//                   << endl
//                   << d->m_ssl_cipher_version
//                   << endl
//                   << d->m_ssl_good_from
//                   << endl
//                   << d->m_ssl_good_until
//                   << endl
//                   << d->m_ssl_cert_state
//                   << endl;

  KSSLInfoDlg *kid = new KSSLInfoDlg(d->m_ssl_in_use, widget(), "kssl_info_dlg", true );

  if (d->m_bSecurityInQuestion)
	  kid->setSecurityInQuestion(true);

  if (d->m_ssl_in_use) {
    KSSLCertificate *x = KSSLCertificate::fromString(d->m_ssl_peer_certificate.local8Bit());
    if (x) {
       // Set the chain back onto the certificate
       const TQStringList cl = TQStringList::split(TQString("\n"), d->m_ssl_peer_chain);
       TQPtrList<KSSLCertificate> ncl;

       ncl.setAutoDelete(true);
       TQStringList::ConstIterator it = cl.begin();
       const TQStringList::ConstIterator itEnd = cl.end();
       for (; it != itEnd; ++it) {
          KSSLCertificate* const y = KSSLCertificate::fromString((*it).local8Bit());
          if (y) ncl.append(y);
       }

       if (ncl.count() > 0)
          x->chain().setChain(ncl);

       kid->setup(x,
                  d->m_ssl_peer_ip,
                  m_url.url(),
                  d->m_ssl_cipher,
                  d->m_ssl_cipher_desc,
                  d->m_ssl_cipher_version,
                  d->m_ssl_cipher_used_bits.toInt(),
                  d->m_ssl_cipher_bits.toInt(),
                  (KSSLCertificate::KSSLValidation) d->m_ssl_cert_state.toInt()
                  );
        kid->exec();
        delete x;
     } else kid->exec();
  } else kid->exec();
}

void TDEHTMLPart::slotSaveFrame()
{
    KParts::ReadOnlyPart *frame = currentFrame();
    if ( !frame )
        return;

    KURL srcURL( frame->url() );

    if ( srcURL.fileName(false).isEmpty() )
        srcURL.setFileName( "index" + defaultExtension() );

    TDEIO::MetaData metaData;
    // Referrer unknown?
    TDEHTMLPopupGUIClient::saveURL( d->m_view, i18n( "Save Frame As" ), srcURL, metaData, "text/html" );
}

void TDEHTMLPart::slotSetEncoding()
{
  d->m_automaticDetection->setItemChecked( int( d->m_autoDetectLanguage ), false );
  d->m_paSetEncoding->popupMenu()->setItemChecked( 0, false );
  d->m_paSetEncoding->popupMenu()->setItemChecked( d->m_paSetEncoding->popupMenu()->idAt( 2 ), true );

  TQString enc = TDEGlobal::charsets()->encodingForName( d->m_manualDetection->currentText() );
  setEncoding( enc, true );
}

void TDEHTMLPart::slotUseStylesheet()
{
  if (d->m_doc)
  {
    bool autoselect = (d->m_paUseStylesheet->currentItem() == 0);
    d->m_sheetUsed = autoselect ? TQString() : d->m_paUseStylesheet->currentText();
    d->m_doc->updateStyleSelector();
  }
}

void TDEHTMLPart::updateActions()
{
  bool frames = false;

  TQValueList<tdehtml::ChildFrame*>::ConstIterator it = d->m_frames.begin();
  const TQValueList<tdehtml::ChildFrame*>::ConstIterator end = d->m_frames.end();
  for (; it != end; ++it )
      if ( (*it)->m_type == tdehtml::ChildFrame::Frame )
      {
          frames = true;
          break;
      }

  d->m_paViewFrame->setEnabled( frames );
  d->m_paSaveFrame->setEnabled( frames );

  if ( frames )
    d->m_paFind->setText( i18n( "&Find in Frame..." ) );
  else
    d->m_paFind->setText( i18n( "&Find..." ) );

  KParts::Part *frame = 0;

  if ( frames )
    frame = currentFrame();

  bool enableFindAndSelectAll = true;

  if ( frame )
    enableFindAndSelectAll = frame->inherits( "TDEHTMLPart" );

  d->m_paFind->setEnabled( enableFindAndSelectAll );
  d->m_paSelectAll->setEnabled( enableFindAndSelectAll );

  bool enablePrintFrame = false;

  if ( frame )
  {
    TQObject *ext = KParts::BrowserExtension::childObject( frame );
    if ( ext )
      enablePrintFrame = ext->metaObject()->slotNames().contains( "print()" );
  }

  d->m_paPrintFrame->setEnabled( enablePrintFrame );

  TQString bgURL;

  // ### frames
  if ( d->m_doc && d->m_doc->isHTMLDocument() && static_cast<HTMLDocumentImpl*>(d->m_doc)->body() && !d->m_bClearing )
    bgURL = static_cast<HTMLDocumentImpl*>(d->m_doc)->body()->getAttribute( ATTR_BACKGROUND ).string();

  d->m_paSaveBackground->setEnabled( !bgURL.isEmpty() );

  if ( d->m_paDebugScript )
    d->m_paDebugScript->setEnabled( d->m_frame ? d->m_frame->m_jscript : 0L );
}

KParts::LiveConnectExtension *TDEHTMLPart::liveConnectExtension( const tdehtml::RenderPart *frame) const {
    const ConstFrameIt end = d->m_objects.end();
    for(ConstFrameIt it = d->m_objects.begin(); it != end; ++it )
        if ((*it)->m_frame == frame)
            return (*it)->m_liveconnect;
    return 0L;
}

bool TDEHTMLPart::requestFrame( tdehtml::RenderPart *frame, const TQString &url, const TQString &frameName,
                              const TQStringList &params, bool isIFrame )
{
  //kdDebug( 6050 ) << this << " requestFrame( ..., " << url << ", " << frameName << " )" << endl;
  FrameIt it = d->m_frames.find( frameName );
  if ( it == d->m_frames.end() )
  {
    tdehtml::ChildFrame * child = new tdehtml::ChildFrame;
    //kdDebug( 6050 ) << "inserting new frame into frame map " << frameName << endl;
    child->m_name = frameName;
    it = d->m_frames.append( child );
  }

  (*it)->m_type = isIFrame ? tdehtml::ChildFrame::IFrame : tdehtml::ChildFrame::Frame;
  (*it)->m_frame = frame;
  (*it)->m_params = params;

  // Support for <frame src="javascript:string">
  if ( url.find( TQString::fromLatin1( "javascript:" ), 0, false ) == 0 )
  {
    if ( processObjectRequest(*it, KURL("about:blank"), TQString("text/html") ) ) {
      TDEHTMLPart* p = static_cast<TDEHTMLPart*>(static_cast<KParts::ReadOnlyPart *>((*it)->m_part));

      // See if we want to replace content with javascript: output..
      TQVariant res = p->executeScript( DOM::Node(), KURL::decode_string( url.right( url.length() - 11) ) );
      if ( res.type() == TQVariant::String ) {
        p->begin();
        p->write( res.asString() );
        p->end();
      }
      return true;
    }
    return false;
  }
  KURL u = url.isEmpty() ? KURL() : completeURL( url );
  return requestObject( *it, u );
}

TQString TDEHTMLPart::requestFrameName()
{
   return TQString::fromLatin1("<!--frame %1-->").arg(d->m_frameNameId++);
}

bool TDEHTMLPart::requestObject( tdehtml::RenderPart *frame, const TQString &url, const TQString &serviceType,
                               const TQStringList &params )
{
  //kdDebug( 6005 ) << "TDEHTMLPart::requestObject " << this << " frame=" << frame << endl;
  tdehtml::ChildFrame *child = new tdehtml::ChildFrame;
  FrameIt it = d->m_objects.append( child );
  (*it)->m_frame = frame;
  (*it)->m_type = tdehtml::ChildFrame::Object;
  (*it)->m_params = params;

  KParts::URLArgs args;
  args.serviceType = serviceType;
  if (!requestObject( *it, completeURL( url ), args ) && !(*it)->m_run) {
      (*it)->m_bCompleted = true;
      return false;
  }
  return true;
}

bool TDEHTMLPart::requestObject( tdehtml::ChildFrame *child, const KURL &url, const KParts::URLArgs &_args )
{
  if (!checkLinkSecurity(url))
  {
    kdDebug(6005) << this << " TDEHTMLPart::requestObject checkLinkSecurity refused" << endl;
    return false;
  }
  if ( child->m_bPreloaded )
  {
    kdDebug(6005) << "TDEHTMLPart::requestObject preload" << endl;
    if ( child->m_frame && child->m_part )
      child->m_frame->setWidget( child->m_part->widget() );

    child->m_bPreloaded = false;
    return true;
  }

  //kdDebug(6005) << "TDEHTMLPart::requestObject child=" << child << " child->m_part=" << child->m_part << endl;

  KParts::URLArgs args( _args );

  if ( child->m_run )
    child->m_run->abort();

  if ( child->m_part && !args.reload && urlcmp( child->m_part->url().url(), url.url(), true, true ) )
    args.serviceType = child->m_serviceType;

  child->m_args = args;
  child->m_args.reload = (d->m_cachePolicy == TDEIO::CC_Reload);
  child->m_serviceName = TQString();
  if (!d->m_referrer.isEmpty() && !child->m_args.metaData().contains( "referrer" ))
    child->m_args.metaData()["referrer"] = d->m_referrer;

  child->m_args.metaData().insert("PropagateHttpHeader", "true");
  child->m_args.metaData().insert("ssl_parent_ip", d->m_ssl_parent_ip);
  child->m_args.metaData().insert("ssl_parent_cert", d->m_ssl_parent_cert);
  child->m_args.metaData().insert("main_frame_request",
                                  parentPart() == 0 ? "TRUE":"FALSE");
  child->m_args.metaData().insert("ssl_was_in_use",
                                  d->m_ssl_in_use ? "TRUE":"FALSE");
  child->m_args.metaData().insert("ssl_activate_warnings", "TRUE");
  child->m_args.metaData().insert("cross-domain", toplevelURL().url());

  // We want a TDEHTMLPart if the HTML says <frame src=""> or <frame src="about:blank">
  if ((url.isEmpty() || url.url() == "about:blank") && args.serviceType.isEmpty())
    args.serviceType = TQString::fromLatin1( "text/html" );

  if ( args.serviceType.isEmpty() ) {
    kdDebug(6050) << "Running new TDEHTMLRun for " << this << " and child=" << child << endl;
    child->m_run = new TDEHTMLRun( this, child, url, child->m_args, true );
    d->m_bComplete = false; // ensures we stop it in checkCompleted...
    return false;
  } else {
    return processObjectRequest( child, url, args.serviceType );
  }
}

bool TDEHTMLPart::processObjectRequest( tdehtml::ChildFrame *child, const KURL &_url, const TQString &mimetype )
{
  //kdDebug( 6050 ) << "TDEHTMLPart::processObjectRequest trying to create part for " << mimetype << endl;

  // IMPORTANT: create a copy of the url here, because it is just a reference, which was likely to be given
  // by an emitting frame part (emit openURLRequest( blahurl, ... ) . A few lines below we delete the part
  // though -> the reference becomes invalid -> crash is likely
  KURL url( _url );

  // tdehtmlrun called us this way to indicate a loading error
  if ( d->m_onlyLocalReferences || ( url.isEmpty() && mimetype.isEmpty() ) )
  {
      child->m_bCompleted = true;
      checkCompleted();
      return true;
  }

  if (child->m_bNotify)
  {
      child->m_bNotify = false;
      if ( !child->m_args.lockHistory() )
          emit d->m_extension->openURLNotify();
  }

  if ( child->m_serviceType != mimetype || !child->m_part )
  {
    // Before attempting to load a part, check if the user wants that.
    // Many don't like getting ZIP files embedded.
    // However we don't want to ask for flash and other plugin things..
    if ( child->m_type != tdehtml::ChildFrame::Object )
    {
      TQString suggestedFilename;
      if ( child->m_run )
        suggestedFilename = child->m_run->suggestedFilename();

      KParts::BrowserRun::AskSaveResult res = KParts::BrowserRun::askEmbedOrSave(
        url, mimetype, suggestedFilename  );
      switch( res ) {
      case KParts::BrowserRun::Save:
        TDEHTMLPopupGUIClient::saveURL( widget(), i18n( "Save As" ), url, child->m_args.metaData(), TQString(), 0, suggestedFilename);
        // fall-through
      case KParts::BrowserRun::Cancel:
        child->m_bCompleted = true;
        checkCompleted();
        return true; // done
      default: // Open
        break;
      }
    }

    TQStringList dummy; // the list of servicetypes handled by the part is now unused.
    KParts::ReadOnlyPart *part = createPart( d->m_view->viewport(), child->m_name.ascii(), this, child->m_name.ascii(), mimetype, child->m_serviceName, dummy, child->m_params );

    if ( !part )
    {
        if ( child->m_frame )
          if (child->m_frame->partLoadingErrorNotify( child, url, mimetype ))
            return true; // we succeeded after all (a fallback was used)

        checkEmitLoadEvent();
        return false;
    }

    //CRITICAL STUFF
    if ( child->m_part )
    {
      if (!::tqt_cast<TDEHTMLPart*>(child->m_part) && child->m_jscript)
          child->m_jscript->clear();
      partManager()->removePart( (KParts::ReadOnlyPart *)child->m_part );
      delete (KParts::ReadOnlyPart *)child->m_part;
      if (child->m_liveconnect) {
        disconnect(child->m_liveconnect, TQ_SIGNAL(partEvent(const unsigned long, const TQString &, const KParts::LiveConnectExtension::ArgList &)), child, TQ_SLOT(liveConnectEvent(const unsigned long, const TQString&, const KParts::LiveConnectExtension::ArgList &)));
        child->m_liveconnect = 0L;
      }
    }

    child->m_serviceType = mimetype;
    if ( child->m_frame  && part->widget() )
      child->m_frame->setWidget( part->widget() );

    if ( child->m_type != tdehtml::ChildFrame::Object )
      partManager()->addPart( part, false );
//  else
//      kdDebug(6005) << "AH! NO FRAME!!!!!" << endl;

    child->m_part = part;

    if (::tqt_cast<TDEHTMLPart*>(part)) {
      static_cast<TDEHTMLPart*>(part)->d->m_frame = child;
    } else if (child->m_frame) {
      child->m_liveconnect = KParts::LiveConnectExtension::childObject(part);
      if (child->m_liveconnect)
        connect(child->m_liveconnect, TQ_SIGNAL(partEvent(const unsigned long, const TQString &, const KParts::LiveConnectExtension::ArgList &)), child, TQ_SLOT(liveConnectEvent(const unsigned long, const TQString&, const KParts::LiveConnectExtension::ArgList &)));
    }
    KParts::StatusBarExtension *sb = KParts::StatusBarExtension::childObject(part);
    if (sb)
      sb->setStatusBar( d->m_statusBarExtension->statusBar() );

    connect( part, TQ_SIGNAL( started( TDEIO::Job *) ),
             this, TQ_SLOT( slotChildStarted( TDEIO::Job *) ) );
    connect( part, TQ_SIGNAL( completed() ),
             this, TQ_SLOT( slotChildCompleted() ) );
    connect( part, TQ_SIGNAL( completed(bool) ),
             this, TQ_SLOT( slotChildCompleted(bool) ) );
    connect( part, TQ_SIGNAL( setStatusBarText( const TQString & ) ),
                this, TQ_SIGNAL( setStatusBarText( const TQString & ) ) );
    if ( part->inherits( "TDEHTMLPart" ) )
    {
      connect( this, TQ_SIGNAL( completed() ),
               part, TQ_SLOT( slotParentCompleted() ) );
      connect( this, TQ_SIGNAL( completed(bool) ),
               part, TQ_SLOT( slotParentCompleted() ) );
      // As soon as the child's document is created, we need to set its domain
      // (but we do so only once, so it can't be simply done in the child)
      connect( part, TQ_SIGNAL( docCreated() ),
               this, TQ_SLOT( slotChildDocCreated() ) );
    }

    child->m_extension = KParts::BrowserExtension::childObject( part );

    if ( child->m_extension )
    {
      connect( child->m_extension, TQ_SIGNAL( openURLNotify() ),
               d->m_extension, TQ_SIGNAL( openURLNotify() ) );

      connect( child->m_extension, TQ_SIGNAL( openURLRequestDelayed( const KURL &, const KParts::URLArgs & ) ),
               this, TQ_SLOT( slotChildURLRequest( const KURL &, const KParts::URLArgs & ) ) );

      connect( child->m_extension, TQ_SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ),
               d->m_extension, TQ_SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & ) ) );
      connect( child->m_extension, TQ_SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs &, const KParts::WindowArgs &, KParts::ReadOnlyPart *& ) ),
               d->m_extension, TQ_SIGNAL( createNewWindow( const KURL &, const KParts::URLArgs & , const KParts::WindowArgs &, KParts::ReadOnlyPart *&) ) );

      connect( child->m_extension, TQ_SIGNAL( popupMenu( const TQPoint &, const KFileItemList & ) ),
               d->m_extension, TQ_SIGNAL( popupMenu( const TQPoint &, const KFileItemList & ) ) );
      connect( child->m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KFileItemList & ) ),
               d->m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KFileItemList & ) ) );
      connect( child->m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KFileItemList &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags ) ),
               d->m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KFileItemList &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags ) ) );
      connect( child->m_extension, TQ_SIGNAL( popupMenu( const TQPoint &, const KURL &, const TQString &, mode_t ) ),
               d->m_extension, TQ_SIGNAL( popupMenu( const TQPoint &, const KURL &, const TQString &, mode_t ) ) );
      connect( child->m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &, const TQString &, mode_t ) ),
               d->m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &, const TQString &, mode_t ) ) );
      connect( child->m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t ) ),
               d->m_extension, TQ_SIGNAL( popupMenu( KXMLGUIClient *, const TQPoint &, const KURL &, const KParts::URLArgs &, KParts::BrowserExtension::PopupFlags, mode_t ) ) );

      connect( child->m_extension, TQ_SIGNAL( infoMessage( const TQString & ) ),
               d->m_extension, TQ_SIGNAL( infoMessage( const TQString & ) ) );

      connect( child->m_extension, TQ_SIGNAL( requestFocus( KParts::ReadOnlyPart * ) ),
               this, TQ_SLOT( slotRequestFocus( KParts::ReadOnlyPart * ) ) );

      child->m_extension->setBrowserInterface( d->m_extension->browserInterface() );
    }
  }
  else if ( child->m_frame && child->m_part &&
            child->m_frame->widget() != child->m_part->widget() )
    child->m_frame->setWidget( child->m_part->widget() );

  checkEmitLoadEvent();
  // Some JS code in the load event may have destroyed the part
  // In that case, abort
  if ( !child->m_part )
    return false;

  if ( child->m_bPreloaded )
  {
    if ( child->m_frame && child->m_part )
      child->m_frame->setWidget( child->m_part->widget() );

    child->m_bPreloaded = false;
    return true;
  }

  child->m_args.reload = (d->m_cachePolicy == TDEIO::CC_Reload);

  // make sure the part has a way to find out about the mimetype.
  // we actually set it in child->m_args in requestObject already,
  // but it's useless if we had to use a TDEHTMLRun instance, as the
  // point the run object is to find out exactly the mimetype.
  child->m_args.serviceType = mimetype;

  // if not a frame set child as completed
  child->m_bCompleted = child->m_type == tdehtml::ChildFrame::Object;

  if ( child->m_extension )
    child->m_extension->setURLArgs( child->m_args );

  if(url.protocol() == "javascript" || url.url() == "about:blank") {
      if (!child->m_part->inherits("TDEHTMLPart"))
          return false;

      TDEHTMLPart* p = static_cast<TDEHTMLPart*>(static_cast<KParts::ReadOnlyPart *>(child->m_part));

      p->begin();
      if (d->m_doc && p->d->m_doc)
        p->d->m_doc->setBaseURL(d->m_doc->baseURL());
      if (!url.url().startsWith("about:")) {
        p->write(url.path());
      } else {
	p->m_url = url;
        // we need a body element. testcase: <iframe id="a"></iframe><script>alert(a.document.body);</script>
        p->write("<HTML><TITLE></TITLE><BODY></BODY></HTML>");
      }
      p->end();
      return true;
  }
  else if ( !url.isEmpty() )
  {
      //kdDebug( 6050 ) << "opening " << url.url() << " in frame " << child->m_part << endl;
      bool b = child->m_part->openURL( url );
      if (child->m_bCompleted)
          checkCompleted();
      return b;
  }
  else
  {
      child->m_bCompleted = true;
      checkCompleted();
      return true;
  }
}

KParts::ReadOnlyPart *TDEHTMLPart::createPart( TQWidget *parentWidget, const char *widgetName,
                                             TQObject *parent, const char *name, const TQString &mimetype,
                                             TQString &serviceName, TQStringList &serviceTypes,
                                             const TQStringList &params )
{
  TQString constr;
  if ( !serviceName.isEmpty() )
    constr.append( TQString::fromLatin1( "Name == '%1'" ).arg( serviceName ) );

  TDETrader::OfferList offers = TDETrader::self()->query( mimetype, "KParts/ReadOnlyPart", constr, TQString() );

  if ( offers.isEmpty() ) {
    int pos = mimetype.find( "-plugin" );
    if (pos < 0)
        return 0L;
    TQString stripped_mime = mimetype.left( pos );
    offers = TDETrader::self()->query( stripped_mime, "KParts/ReadOnlyPart", constr, TQString() );
    if ( offers.isEmpty() )
        return 0L;
  }

  TDETrader::OfferList::ConstIterator it = offers.begin();
  const TDETrader::OfferList::ConstIterator itEnd = offers.end();
  for ( ; it != itEnd; ++it )
  {
    KService::Ptr service = (*it);

    KLibFactory* const factory = KLibLoader::self()->factory( TQFile::encodeName(service->library()) );
    if ( factory ) {
      KParts::ReadOnlyPart *res = 0L;

      const char *className = "KParts::ReadOnlyPart";
      if ( service->serviceTypes().contains( "Browser/View" ) )
        className = "Browser/View";

      if ( factory->inherits( "KParts::Factory" ) )
        res = static_cast<KParts::ReadOnlyPart *>(static_cast<KParts::Factory *>( factory )->createPart( parentWidget, widgetName, parent, name, className, params ));
      else
        res = static_cast<KParts::ReadOnlyPart *>(factory->create( parentWidget, widgetName, className ));

      if ( res ) {
        serviceTypes = service->serviceTypes();
        serviceName = service->name();
        return res;
      }
    } else {
      // TODO KMessageBox::error and i18n, like in KonqFactory::createView?
      kdWarning() << TQString(TQString("There was an error loading the module %1.\nThe diagnostics is:\n%2")
                      .arg(service->name()).arg(KLibLoader::self()->lastErrorMessage())) << endl;
    }
  }
  return 0;
}

KParts::PartManager *TDEHTMLPart::partManager()
{
  if ( !d->m_manager && d->m_view )
  {
    d->m_manager = new KParts::PartManager( d->m_view->topLevelWidget(), this, "tdehtml part manager" );
    d->m_manager->setAllowNestedParts( true );
    connect( d->m_manager, TQ_SIGNAL( activePartChanged( KParts::Part * ) ),
             this, TQ_SLOT( slotActiveFrameChanged( KParts::Part * ) ) );
    connect( d->m_manager, TQ_SIGNAL( partRemoved( KParts::Part * ) ),
             this, TQ_SLOT( slotPartRemoved( KParts::Part * ) ) );
  }

  return d->m_manager;
}

void TDEHTMLPart::submitFormAgain()
{
  disconnect(this, TQ_SIGNAL(completed()), this, TQ_SLOT(submitFormAgain()));
  if( d->m_doc && !d->m_doc->parsing() && d->m_submitForm)
    TDEHTMLPart::submitForm( d->m_submitForm->submitAction, d->m_submitForm->submitUrl, d->m_submitForm->submitFormData, d->m_submitForm->target, d->m_submitForm->submitContentType, d->m_submitForm->submitBoundary );

  delete d->m_submitForm;
  d->m_submitForm = 0;
}

void TDEHTMLPart::submitFormProxy( const char *action, const TQString &url, const TQByteArray &formData, const TQString &_target, const TQString& contentType, const TQString& boundary )
{
  submitForm(action, url, formData, _target, contentType, boundary);
}

void TDEHTMLPart::submitForm( const char *action, const TQString &url, const TQByteArray &formData, const TQString &_target, const TQString& contentType, const TQString& boundary )
{
  kdDebug(6000) << this << ": TDEHTMLPart::submitForm target=" << _target << " url=" << url << endl;
  if (d->m_formNotification == TDEHTMLPart::Only) {
    emit formSubmitNotification(action, url, formData, _target, contentType, boundary);
    return;
  } else if (d->m_formNotification == TDEHTMLPart::Before) {
    emit formSubmitNotification(action, url, formData, _target, contentType, boundary);
  }

  KURL u = completeURL( url );

  if ( !u.isValid() )
  {
    // ### ERROR HANDLING!
    return;
  }

  // Form security checks
  //
  /*
   * If these form security checks are still in this place in a month or two
   * I'm going to simply delete them.
   */

  /* This is separate for a reason.  It has to be _before_ all script, etc,
   * AND I don't want to break anything that uses checkLinkSecurity() in
   * other places.
   */

  if (!d->m_submitForm) {
    if (u.protocol() != "https" && u.protocol() != "mailto") {
      if (d->m_ssl_in_use) {    // Going from SSL -> nonSSL
        int rc = KMessageBox::warningContinueCancel(NULL, i18n("Warning:  This is a secure form but it is attempting to send your data back unencrypted."
                                                               "\nA third party may be able to intercept and view this information."
                                                               "\nAre you sure you wish to continue?"),
                                                    i18n("Network Transmission"),KGuiItem(i18n("&Send Unencrypted")));
        if (rc == KMessageBox::Cancel)
          return;
      } else {                  // Going from nonSSL -> nonSSL
        KSSLSettings kss(true);
        if (kss.warnOnUnencrypted()) {
          int rc = KMessageBox::warningContinueCancel(NULL,
                                                      i18n("Warning: Your data is about to be transmitted across the network unencrypted."
                                                           "\nAre you sure you wish to continue?"),
                                                      i18n("Network Transmission"),
                                                      KGuiItem(i18n("&Send Unencrypted")),
                                                      "WarnOnUnencryptedForm");
          // Move this setting into KSSL instead
          TDEConfig *config = kapp->config();
          TQString grpNotifMsgs = TQString::fromLatin1("Notification Messages");
          TDEConfigGroupSaver saver( config, grpNotifMsgs );

          if (!config->readBoolEntry("WarnOnUnencryptedForm", true)) {
            config->deleteEntry("WarnOnUnencryptedForm");
            config->sync();
            kss.setWarnOnUnencrypted(false);
            kss.save();
          }
          if (rc == KMessageBox::Cancel)
            return;
      	}
      }
    }

    if (u.protocol() == "mailto") {
      int rc = KMessageBox::warningContinueCancel(NULL,
                                                  i18n("This site is attempting to submit form data via email.\n"
                                                       "Do you want to continue?"),
                                                  i18n("Network Transmission"),
                                                  KGuiItem(i18n("&Send Email")),
                                                  "WarnTriedEmailSubmit");

      if (rc == KMessageBox::Cancel) {
        return;
      }
    }
  }

  // End form security checks
  //

  TQString urlstring = u.url();

  if ( urlstring.find( TQString::fromLatin1( "javascript:" ), 0, false ) == 0 ) {
    urlstring = KURL::decode_string(urlstring);
    crossFrameExecuteScript( _target, urlstring.right( urlstring.length() - 11) );
    return;
  }

  if (!checkLinkSecurity(u,
			 i18n( "<qt>The form will be submitted to <BR><B>%1</B><BR>on your local filesystem.<BR>Do you want to submit the form?" ),
			 i18n( "Submit" )))
    return;

  KParts::URLArgs args;

  if (!d->m_referrer.isEmpty())
     args.metaData()["referrer"] = d->m_referrer;

  args.metaData().insert("PropagateHttpHeader", "true");
  args.metaData().insert("ssl_parent_ip", d->m_ssl_parent_ip);
  args.metaData().insert("ssl_parent_cert", d->m_ssl_parent_cert);
  args.metaData().insert("main_frame_request",
                         parentPart() == 0 ? "TRUE":"FALSE");
  args.metaData().insert("ssl_was_in_use", d->m_ssl_in_use ? "TRUE":"FALSE");
  args.metaData().insert("ssl_activate_warnings", "TRUE");
//WABA: When we post a form we should treat it as the main url
//the request should never be considered cross-domain
//args.metaData().insert("cross-domain", toplevelURL().url());
  args.frameName = _target.isEmpty() ? d->m_doc->baseTarget() : _target ;

  // Handle mailto: forms
  if (u.protocol() == "mailto") {
      // 1)  Check for attach= and strip it
      TQString q = u.query().mid(1);
      TQStringList nvps = TQStringList::split("&", q);
      bool triedToAttach = false;

      TQStringList::Iterator nvp = nvps.begin();
      const TQStringList::Iterator nvpEnd = nvps.end();

// cannot be a for loop as if something is removed we don't want to do ++nvp, as
// remove returns an iterator pointing to the next item

      while (nvp != nvpEnd) {
         const TQStringList pair = TQStringList::split("=", *nvp);
         if (pair.count() >= 2) {
            if (pair.first().lower() == "attach") {
               nvp = nvps.remove(nvp);
               triedToAttach = true;
            } else {
               ++nvp;
            }
         } else {
            ++nvp;
         }
      }

      if (triedToAttach)
         KMessageBox::information(NULL, i18n("This site attempted to attach a file from your computer in the form submission. The attachment was removed for your protection."), i18n("TDE"), "WarnTriedAttach");

      // 2)  Append body=
      TQString bodyEnc;
      if (contentType.lower() == "multipart/form-data") {
         // FIXME: is this correct?  I suspect not
         bodyEnc = KURL::encode_string(TQString::fromLatin1(formData.data(),
                                                           formData.size()));
      } else if (contentType.lower() == "text/plain") {
         // Convention seems to be to decode, and s/&/\n/
         TQString tmpbody = TQString::fromLatin1(formData.data(),
                                               formData.size());
         tmpbody.replace(TQRegExp("[&]"), "\n");
         tmpbody.replace(TQRegExp("[+]"), " ");
         tmpbody = KURL::decode_string(tmpbody);  // Decode the rest of it
         bodyEnc = KURL::encode_string(tmpbody);  // Recode for the URL
      } else {
         bodyEnc = KURL::encode_string(TQString::fromLatin1(formData.data(),
                                                           formData.size()));
      }

      nvps.append(TQString("body=%1").arg(bodyEnc));
      q = nvps.join("&");
      u.setQuery(q);
  }

  if ( strcmp( action, "get" ) == 0 ) {
    if (u.protocol() != "mailto")
       u.setQuery( TQString::fromLatin1( formData.data(), formData.size() ) );
    args.setDoPost( false );
  }
  else {
    args.postData = formData;
    args.setDoPost( true );

    // construct some user headers if necessary
    if (contentType.isNull() || contentType == "application/x-www-form-urlencoded")
      args.setContentType( "Content-Type: application/x-www-form-urlencoded" );
    else // contentType must be "multipart/form-data"
      args.setContentType( "Content-Type: " + contentType + "; boundary=" + boundary );
  }

  if ( d->m_doc->parsing() || d->m_runningScripts > 0 ) {
    if( d->m_submitForm ) {
      kdDebug(6000) << "TDEHTMLPart::submitForm ABORTING!" << endl;
      return;
    }
    d->m_submitForm = new TDEHTMLPartPrivate::SubmitForm;
    d->m_submitForm->submitAction = action;
    d->m_submitForm->submitUrl = url;
    d->m_submitForm->submitFormData = formData;
    d->m_submitForm->target = _target;
    d->m_submitForm->submitContentType = contentType;
    d->m_submitForm->submitBoundary = boundary;
    connect(this, TQ_SIGNAL(completed()), this, TQ_SLOT(submitFormAgain()));
  }
  else
  {
    emit d->m_extension->openURLRequest( u, args );
  }
}

void TDEHTMLPart::popupMenu( const TQString &linkUrl )
{
  KURL popupURL;
  KURL linkKURL;
  KParts::URLArgs args;
  TQString referrer;
  KParts::BrowserExtension::PopupFlags itemflags=KParts::BrowserExtension::ShowBookmark | KParts::BrowserExtension::ShowReload;

  if ( linkUrl.isEmpty() ) { // click on background
    TDEHTMLPart* tdehtmlPart = this;
    while ( tdehtmlPart->parentPart() )
    {
      tdehtmlPart=tdehtmlPart->parentPart();
    }
    popupURL = tdehtmlPart->url();
    referrer = tdehtmlPart->pageReferrer();
    if (hasSelection())
      itemflags = KParts::BrowserExtension::ShowTextSelectionItems;
    else
      itemflags |= KParts::BrowserExtension::ShowNavigationItems;
  } else {               // click on link
    popupURL = completeURL( linkUrl );
    linkKURL = popupURL;
    referrer = this->referrer();

    if (!(d->m_strSelectedURLTarget).isEmpty() &&
           (d->m_strSelectedURLTarget.lower() != "_top") &&
           (d->m_strSelectedURLTarget.lower() != "_self") &&
	   (d->m_strSelectedURLTarget.lower() != "_parent")) {
      if (d->m_strSelectedURLTarget.lower() == "_blank")
        args.setForcesNewWindow(true);
      else {
	TDEHTMLPart *p = this;
	while (p->parentPart())
	  p = p->parentPart();
	if (!p->frameExists(d->m_strSelectedURLTarget))
          args.setForcesNewWindow(true);
      }
    }
  }

  // Danger, Will Robinson. The Popup might stay around for a much
  // longer time than TDEHTMLPart. Deal with it.
  TDEHTMLPopupGUIClient* client = new TDEHTMLPopupGUIClient( this, d->m_popupMenuXML, linkKURL );
  TQGuardedPtr<TQObject> guard( client );

  TQString mimetype = TQString::fromLatin1( "text/html" );
  args.metaData()["referrer"] = referrer;

  if (!linkUrl.isEmpty())				// over a link
  {
    if (popupURL.isLocalFile())				// safe to do this
    {
      mimetype = KMimeType::findByURL(popupURL,0,true,false)->name();
    }
    else						// look at "extension" of link
    {
      const TQString fname(popupURL.fileName(false));
      if (!fname.isEmpty() && !popupURL.hasRef() && popupURL.query().isEmpty())
      {
        KMimeType::Ptr pmt = KMimeType::findByPath(fname,0,true);

        // Further check for mime types guessed from the extension which,
        // on a web page, are more likely to be a script delivering content
        // of undecidable type. If the mime type from the extension is one
        // of these, don't use it.  Retain the original type 'text/html'.
        if (pmt->name() != KMimeType::defaultMimeType() &&
            !pmt->is("application/x-perl") &&
            !pmt->is("application/x-perl-module") &&
            !pmt->is("application/x-php") &&
            !pmt->is("application/x-python-bytecode") &&
            !pmt->is("application/x-python") &&
            !pmt->is("application/x-shellscript"))
          mimetype = pmt->name();
      }
    }
  }

  args.serviceType = mimetype;

  emit d->m_extension->popupMenu( client, TQCursor::pos(), popupURL, args, itemflags, S_IFREG /*always a file*/);

  if ( !guard.isNull() ) {
     delete client;
     emit popupMenu(linkUrl, TQCursor::pos());
     d->m_strSelectedURL = d->m_strSelectedURLTarget = TQString();
  }
}

void TDEHTMLPart::slotParentCompleted()
{
  //kdDebug(6050) << this << " slotParentCompleted()" << endl;
  if ( !d->m_redirectURL.isEmpty() && !d->m_redirectionTimer.isActive() )
  {
    //kdDebug(6050) << this << ": starting timer for child redirection -> " << d->m_redirectURL << endl;
    d->m_redirectionTimer.start( 1000 * d->m_delayRedirect, true );
  }
}

void TDEHTMLPart::slotChildStarted( TDEIO::Job *job )
{
  tdehtml::ChildFrame *child = frame( sender() );

  assert( child );

  child->m_bCompleted = false;

  if ( d->m_bComplete )
  {
#if 0
    // WABA: Looks like this belongs somewhere else
    if ( !parentPart() ) // "toplevel" html document? if yes, then notify the hosting browser about the document (url) changes
    {
      emit d->m_extension->openURLNotify();
    }
#endif
    d->m_bComplete = false;
    emit started( job );
  }
}

void TDEHTMLPart::slotChildCompleted()
{
  slotChildCompleted( false );
}

void TDEHTMLPart::slotChildCompleted( bool pendingAction )
{
  tdehtml::ChildFrame *child = frame( sender() );

  if ( child ) {
    kdDebug(6050) << this << " slotChildCompleted child=" << child << " m_frame=" << child->m_frame << endl;
    child->m_bCompleted = true;
    child->m_bPendingRedirection = pendingAction;
    child->m_args = KParts::URLArgs();
  }
  checkCompleted();
}

void TDEHTMLPart::slotChildDocCreated()
{
  const TDEHTMLPart* htmlFrame = static_cast<const TDEHTMLPart *>(sender());
  // Set domain to the frameset's domain
  // This must only be done when loading the frameset initially (#22039),
  // not when following a link in a frame (#44162).
  if ( d->m_doc && d->m_doc->isHTMLDocument() )
  {
    if ( sender()->inherits("TDEHTMLPart") )
    {
      DOMString domain = static_cast<HTMLDocumentImpl*>(d->m_doc)->domain();
      if (htmlFrame->d->m_doc && htmlFrame->d->m_doc->isHTMLDocument() )
        //kdDebug(6050) << "TDEHTMLPart::slotChildDocCreated: url: " << htmlFrame->m_url.url() << endl;
        static_cast<HTMLDocumentImpl*>(htmlFrame->d->m_doc)->setDomain( domain );
    }
  }
  // So it only happens once
  disconnect( htmlFrame, TQ_SIGNAL( docCreated() ), this, TQ_SLOT( slotChildDocCreated() ) );
}

void TDEHTMLPart::slotChildURLRequest( const KURL &url, const KParts::URLArgs &args )
{
  tdehtml::ChildFrame *child = frame( sender()->parent() );
  TDEHTMLPart *callingHtmlPart = const_cast<TDEHTMLPart *>(dynamic_cast<const TDEHTMLPart *>(sender()->parent()));

  // TODO: handle child target correctly! currently the script are always executed fur the parent
  TQString urlStr = url.url();
  if ( urlStr.find( TQString::fromLatin1( "javascript:" ), 0, false ) == 0 ) {
      TQString script = KURL::decode_string( urlStr.right( urlStr.length() - 11 ) );
      executeScript( DOM::Node(), script );
      return;
  }

  TQString frameName = args.frameName.lower();
  if ( !frameName.isEmpty() ) {
    if ( frameName == TQString::fromLatin1( "_top" ) )
    {
      emit d->m_extension->openURLRequest( url, args );
      return;
    }
    else if ( frameName == TQString::fromLatin1( "_blank" ) )
    {
      emit d->m_extension->createNewWindow( url, args );
      return;
    }
    else if ( frameName == TQString::fromLatin1( "_parent" ) )
    {
      KParts::URLArgs newArgs( args );
      newArgs.frameName = TQString();

      emit d->m_extension->openURLRequest( url, newArgs );
      return;
    }
    else if ( frameName != TQString::fromLatin1( "_self" ) )
    {
      tdehtml::ChildFrame *_frame = recursiveFrameRequest( callingHtmlPart, url, args );

      if ( !_frame )
      {
        emit d->m_extension->openURLRequest( url, args );
        return;
      }

      child = _frame;
    }
  }

  if ( child && child->m_type != tdehtml::ChildFrame::Object ) {
      // Inform someone that we are about to show something else.
      child->m_bNotify = true;
      requestObject( child, url, args );
  }  else if ( frameName== "_self" ) // this is for embedded objects (via <object>) which want to replace the current document
  {
      KParts::URLArgs newArgs( args );
      newArgs.frameName = TQString();
      emit d->m_extension->openURLRequest( url, newArgs );
  }
}

void TDEHTMLPart::slotRequestFocus( KParts::ReadOnlyPart * )
{
  emit d->m_extension->requestFocus(this);
}

tdehtml::ChildFrame *TDEHTMLPart::frame( const TQObject *obj )
{
    assert( obj->inherits( "KParts::ReadOnlyPart" ) );
    const KParts::ReadOnlyPart* const part = static_cast<const KParts::ReadOnlyPart *>( obj );

    FrameIt it = d->m_frames.begin();
    const FrameIt end = d->m_frames.end();
    for (; it != end; ++it )
      if ( (KParts::ReadOnlyPart *)(*it)->m_part == part )
        return *it;

    FrameIt oi = d->m_objects.begin();
    const FrameIt oiEnd = d->m_objects.end();
    for (; oi != oiEnd; ++oi )
      if ( (KParts::ReadOnlyPart *)(*oi)->m_part == part )
        return *oi;

    return 0L;
}

//#define DEBUG_FINDFRAME

bool TDEHTMLPart::checkFrameAccess(TDEHTMLPart *callingHtmlPart)
{
  if (callingHtmlPart == this)
    return true; // trivial

  if (htmlDocument().isNull()) {
#ifdef DEBUG_FINDFRAME
    kdDebug(6050) << "TDEHTMLPart::checkFrameAccess: Empty part " << this << " URL = " << m_url << endl;
#endif
    return false; // we are empty?
  }

  // now compare the domains
  if (callingHtmlPart && !callingHtmlPart->htmlDocument().isNull() &&
      !htmlDocument().isNull())  {
    DOM::DOMString actDomain = callingHtmlPart->htmlDocument().domain();
    DOM::DOMString destDomain = htmlDocument().domain();

#ifdef DEBUG_FINDFRAME
    kdDebug(6050) << "TDEHTMLPart::checkFrameAccess: actDomain = '" << actDomain.string() << "' destDomain = '" << destDomain.string() << "'" << endl;
#endif

    if (actDomain == destDomain)
      return true;
  }
#ifdef DEBUG_FINDFRAME
  else
  {
    kdDebug(6050) << "TDEHTMLPart::checkFrameAccess: Unknown part/domain " << callingHtmlPart << " tries to access part " << this << endl;
  }
#endif
  return false;
}

TDEHTMLPart *
TDEHTMLPart::findFrameParent( KParts::ReadOnlyPart *callingPart, const TQString &f, tdehtml::ChildFrame **childFrame )
{
#ifdef DEBUG_FINDFRAME
  kdDebug(6050) << "TDEHTMLPart::findFrameParent: this = " << this << " URL = " << m_url << " name = " << name() << " findFrameParent( " << f << " )" << endl;
#endif
  // Check access
  TDEHTMLPart* const callingHtmlPart = dynamic_cast<TDEHTMLPart *>(callingPart);

  if (!checkFrameAccess(callingHtmlPart))
     return 0;

  // match encoding used in KonqView::setViewName()
  if (!childFrame && !parentPart() && (TQString::fromLocal8Bit(name()) == f))
     return this;

  FrameIt it = d->m_frames.find( f );
  const FrameIt end = d->m_frames.end();
  if ( it != end )
  {
#ifdef DEBUG_FINDFRAME
     kdDebug(6050) << "TDEHTMLPart::findFrameParent: FOUND!" << endl;
#endif
     if (childFrame)
        *childFrame = *it;
     return this;
  }

  it = d->m_frames.begin();
  for (; it != end; ++it )
  {
    KParts::ReadOnlyPart* const p = (*it)->m_part;
    if ( p && p->inherits( "TDEHTMLPart" ))
    {
      TDEHTMLPart* const frameParent = static_cast<TDEHTMLPart*>(p)->findFrameParent(callingPart, f, childFrame);
      if (frameParent)
         return frameParent;
    }
  }
  return 0;
}


TDEHTMLPart *TDEHTMLPart::findFrame( const TQString &f )
{
  tdehtml::ChildFrame *childFrame;
  TDEHTMLPart *parentFrame = findFrameParent(this, f, &childFrame);
  if (parentFrame)
  {
     KParts::ReadOnlyPart *p = childFrame->m_part;
     if ( p && p->inherits( "TDEHTMLPart" ))
        return static_cast<TDEHTMLPart *>(p);
  }
  return 0;
}

KParts::ReadOnlyPart *TDEHTMLPart::findFramePart(const TQString &f)
{
  tdehtml::ChildFrame *childFrame;
  return findFrameParent(this, f, &childFrame) ? static_cast<KParts::ReadOnlyPart *>(childFrame->m_part) : 0L;
}

KParts::ReadOnlyPart *TDEHTMLPart::currentFrame() const
{
  KParts::ReadOnlyPart* part = (KParts::ReadOnlyPart*)(this);
  // Find active part in our frame manager, in case we are a frameset
  // and keep doing that (in case of nested framesets).
  // Just realized we could also do this recursively, calling part->currentFrame()...
  while ( part && part->inherits("TDEHTMLPart") &&
          static_cast<TDEHTMLPart *>(part)->d->m_frames.count() > 0 ) {
    TDEHTMLPart* frameset = static_cast<TDEHTMLPart *>(part);
    part = static_cast<KParts::ReadOnlyPart *>(frameset->partManager()->activePart());
    if ( !part ) return frameset;
  }
  return part;
}

bool TDEHTMLPart::frameExists( const TQString &frameName )
{
  ConstFrameIt it = d->m_frames.find( frameName );
  if ( it == d->m_frames.end() )
    return false;

  // WABA: We only return true if the child actually has a frame
  // set. Otherwise we might find our preloaded-selve.
  // This happens when we restore the frameset.
  return (!(*it)->m_frame.isNull());
}

KJSProxy *TDEHTMLPart::framejScript(KParts::ReadOnlyPart *framePart)
{
  TDEHTMLPart* const kp = ::tqt_cast<TDEHTMLPart*>(framePart);
  if (kp)
    return kp->jScript();

  FrameIt it = d->m_frames.begin();
  const FrameIt itEnd = d->m_frames.end();

  for (; it != itEnd; ++it)
    if (framePart == (*it)->m_part) {
      if (!(*it)->m_jscript)
        createJScript(*it);
      return (*it)->m_jscript;
    }
  return 0L;
}

TDEHTMLPart *TDEHTMLPart::parentPart()
{
  return ::tqt_cast<TDEHTMLPart *>( parent() );
}

tdehtml::ChildFrame *TDEHTMLPart::recursiveFrameRequest( TDEHTMLPart *callingHtmlPart, const KURL &url,
                                                     const KParts::URLArgs &args, bool callParent )
{
#ifdef DEBUG_FINDFRAME
  kdDebug( 6050 ) << "TDEHTMLPart::recursiveFrameRequest this = " << this << ", frame = " << args.frameName << ", url = " << url << endl;
#endif
  tdehtml::ChildFrame *childFrame;
  TDEHTMLPart *childPart = findFrameParent(callingHtmlPart, args.frameName, &childFrame);
  if (childPart)
  {
     if (childPart == this)
        return childFrame;

     childPart->requestObject( childFrame, url, args );
     return 0;
  }

  if ( parentPart() && callParent )
  {
     tdehtml::ChildFrame *res = parentPart()->recursiveFrameRequest( callingHtmlPart, url, args, callParent );

     if ( res )
       parentPart()->requestObject( res, url, args );
  }

  return 0L;
}

#ifndef NDEBUG
static int s_saveStateIndentLevel = 0;
#endif

void TDEHTMLPart::saveState( TQDataStream &stream )
{
#ifndef NDEBUG
  TQString indent = TQString().leftJustify( s_saveStateIndentLevel * 4, ' ' );
  const int indentLevel = s_saveStateIndentLevel++;
  kdDebug( 6050 ) << indent << "saveState this=" << this << " '" << name() << "' saving URL " << m_url.url() << endl;
#endif

  stream << m_url << (TQ_INT32)d->m_view->contentsX() << (TQ_INT32)d->m_view->contentsY()
         << (TQ_INT32) d->m_view->contentsWidth() << (TQ_INT32) d->m_view->contentsHeight() << (TQ_INT32) d->m_view->marginWidth() << (TQ_INT32) d->m_view->marginHeight();

  // save link cursor position
  int focusNodeNumber;
  if (!d->m_focusNodeRestored)
      focusNodeNumber = d->m_focusNodeNumber;
  else if (d->m_doc && d->m_doc->focusNode())
      focusNodeNumber = d->m_doc->nodeAbsIndex(d->m_doc->focusNode());
  else
      focusNodeNumber = -1;
  stream << focusNodeNumber;

  // Save the doc's cache id.
  stream << d->m_cacheId;

  // Save the state of the document (Most notably the state of any forms)
  TQStringList docState;
  if (d->m_doc)
  {
     docState = d->m_doc->docState();
  }
  stream << d->m_encoding << d->m_sheetUsed << docState;

  stream << d->m_zoomFactor;

  stream << d->m_httpHeaders;
  stream << d->m_pageServices;
  stream << d->m_pageReferrer;

  // Save ssl data
  stream << d->m_ssl_in_use
         << d->m_ssl_peer_certificate
         << d->m_ssl_peer_chain
         << d->m_ssl_peer_ip
         << d->m_ssl_cipher
         << d->m_ssl_cipher_desc
         << d->m_ssl_cipher_version
         << d->m_ssl_cipher_used_bits
         << d->m_ssl_cipher_bits
         << d->m_ssl_cert_state
         << d->m_ssl_parent_ip
         << d->m_ssl_parent_cert;


  TQStringList frameNameLst, frameServiceTypeLst, frameServiceNameLst;
  KURL::List frameURLLst;
  TQValueList<TQByteArray> frameStateBufferLst;

  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
  {
    if ( !(*it)->m_part )
       continue;

    frameNameLst << (*it)->m_name;
    frameServiceTypeLst << (*it)->m_serviceType;
    frameServiceNameLst << (*it)->m_serviceName;
    frameURLLst << (*it)->m_part->url();

    TQByteArray state;
    TQDataStream frameStream( state, IO_WriteOnly );

    if ( (*it)->m_extension )
      (*it)->m_extension->saveState( frameStream );

    frameStateBufferLst << state;
  }

  // Save frame data
  stream << (TQ_UINT32) frameNameLst.count();
  stream << frameNameLst << frameServiceTypeLst << frameServiceNameLst << frameURLLst << frameStateBufferLst;
#ifndef NDEBUG
  s_saveStateIndentLevel = indentLevel;
#endif
}

void TDEHTMLPart::restoreState( TQDataStream &stream )
{
  KURL u;
  TQ_INT32 xOffset, yOffset, wContents, hContents, mWidth, mHeight;
  TQ_UINT32 frameCount;
  TQStringList frameNames, frameServiceTypes, docState, frameServiceNames;
  KURL::List frameURLs;
  TQValueList<TQByteArray> frameStateBuffers;
  TQValueList<int> fSizes;
  TQString encoding, sheetUsed;
  long old_cacheId = d->m_cacheId;

  stream >> u >> xOffset >> yOffset >> wContents >> hContents >> mWidth >> mHeight;

  d->m_view->setMarginWidth( mWidth );
  d->m_view->setMarginHeight( mHeight );

  // restore link cursor position
  // nth node is active. value is set in checkCompleted()
  stream >> d->m_focusNodeNumber;
  d->m_focusNodeRestored = false;

  stream >> d->m_cacheId;

  stream >> encoding >> sheetUsed >> docState;

  d->m_encoding = encoding;
  d->m_sheetUsed = sheetUsed;

  int zoomFactor;
  stream >> zoomFactor;
  setZoomFactor(zoomFactor);

  stream >> d->m_httpHeaders;
  stream >> d->m_pageServices;
  stream >> d->m_pageReferrer;

  // Restore ssl data
  stream >> d->m_ssl_in_use
         >> d->m_ssl_peer_certificate
         >> d->m_ssl_peer_chain
         >> d->m_ssl_peer_ip
         >> d->m_ssl_cipher
         >> d->m_ssl_cipher_desc
         >> d->m_ssl_cipher_version
         >> d->m_ssl_cipher_used_bits
         >> d->m_ssl_cipher_bits
         >> d->m_ssl_cert_state
         >> d->m_ssl_parent_ip
         >> d->m_ssl_parent_cert;

  setPageSecurity( d->m_ssl_in_use ? Encrypted : NotCrypted );

  stream >> frameCount >> frameNames >> frameServiceTypes >> frameServiceNames
         >> frameURLs >> frameStateBuffers;

  d->m_bComplete = false;
  d->m_bLoadEventEmitted = false;

//   kdDebug( 6050 ) << "restoreState() docState.count() = " << docState.count() << endl;
//   kdDebug( 6050 ) << "m_url " << m_url.url() << " <-> " << u.url() << endl;
//   kdDebug( 6050 ) << "m_frames.count() " << d->m_frames.count() << " <-> " << frameCount << endl;

  if (d->m_cacheId == old_cacheId)
  {
    // Partial restore
    d->m_redirectionTimer.stop();

    FrameIt fIt = d->m_frames.begin();
    const FrameIt fEnd = d->m_frames.end();

    for (; fIt != fEnd; ++fIt )
        (*fIt)->m_bCompleted = false;

    fIt = d->m_frames.begin();

    TQStringList::ConstIterator fNameIt = frameNames.begin();
    TQStringList::ConstIterator fServiceTypeIt = frameServiceTypes.begin();
    TQStringList::ConstIterator fServiceNameIt = frameServiceNames.begin();
    KURL::List::ConstIterator fURLIt = frameURLs.begin();
    TQValueList<TQByteArray>::ConstIterator fBufferIt = frameStateBuffers.begin();

    for (; fIt != fEnd; ++fIt, ++fNameIt, ++fServiceTypeIt, ++fServiceNameIt, ++fURLIt, ++fBufferIt )
    {
      tdehtml::ChildFrame* const child = *fIt;

//      kdDebug( 6050 ) <<  *fNameIt  << " ---- " <<  *fServiceTypeIt << endl;

      if ( child->m_name != *fNameIt || child->m_serviceType != *fServiceTypeIt )
      {
        child->m_bPreloaded = true;
        child->m_name = *fNameIt;
        child->m_serviceName = *fServiceNameIt;
        processObjectRequest( child, *fURLIt, *fServiceTypeIt );
      }
      if ( child->m_part )
      {
        child->m_bCompleted = false;
        if ( child->m_extension && !(*fBufferIt).isEmpty() )
        {
          TQDataStream frameStream( *fBufferIt, IO_ReadOnly );
          child->m_extension->restoreState( frameStream );
        }
        else
          child->m_part->openURL( *fURLIt );
      }
    }

    KParts::URLArgs args( d->m_extension->urlArgs() );
    args.xOffset = xOffset;
    args.yOffset = yOffset;
    args.docState = docState;
    d->m_extension->setURLArgs( args );

    d->m_view->resizeContents( wContents,  hContents);
    d->m_view->setContentsPos( xOffset, yOffset );

    m_url = u;
  }
  else
  {
    // Full restore.
    closeURL();
    // We must force a clear because we want to be sure to delete all
    // frames.
    d->m_bCleared = false;
    clear();
    d->m_encoding = encoding;
    d->m_sheetUsed = sheetUsed;

    TQStringList::ConstIterator fNameIt = frameNames.begin();
    const TQStringList::ConstIterator fNameEnd = frameNames.end();

    TQStringList::ConstIterator fServiceTypeIt = frameServiceTypes.begin();
    TQStringList::ConstIterator fServiceNameIt = frameServiceNames.begin();
    KURL::List::ConstIterator fURLIt = frameURLs.begin();
    TQValueList<TQByteArray>::ConstIterator fBufferIt = frameStateBuffers.begin();

    for (; fNameIt != fNameEnd; ++fNameIt, ++fServiceTypeIt, ++fServiceNameIt, ++fURLIt, ++fBufferIt )
    {
      tdehtml::ChildFrame* const newChild = new tdehtml::ChildFrame;
      newChild->m_bPreloaded = true;
      newChild->m_name = *fNameIt;
      newChild->m_serviceName = *fServiceNameIt;

//      kdDebug( 6050 ) << *fNameIt << " ---- " << *fServiceTypeIt << endl;

      const FrameIt childFrame = d->m_frames.append( newChild );

      processObjectRequest( *childFrame, *fURLIt, *fServiceTypeIt );

      (*childFrame)->m_bPreloaded = true;

      if ( (*childFrame)->m_part )
      {
        if ( (*childFrame)->m_extension ) {
          if ( (*childFrame)->m_extension && !(*fBufferIt).isEmpty() )
          {
            TQDataStream frameStream( *fBufferIt, IO_ReadOnly );
            (*childFrame)->m_extension->restoreState( frameStream );
          }
          else {
            (*childFrame)->m_part->openURL( *fURLIt );
          }
        }
      }
    }

    KParts::URLArgs args( d->m_extension->urlArgs() );
    args.xOffset = xOffset;
    args.yOffset = yOffset;
    args.docState = docState;

    d->m_extension->setURLArgs( args );
    if (!TDEHTMLPageCache::self()->isComplete(d->m_cacheId))
    {
       d->m_restored = true;
       openURL( u );
       d->m_restored = false;
    }
    else
    {
       restoreURL( u );
    }
  }

}

void TDEHTMLPart::show()
{
  if ( d->m_view )
    d->m_view->show();
}

void TDEHTMLPart::hide()
{
  if ( d->m_view )
    d->m_view->hide();
}

DOM::Node TDEHTMLPart::nodeUnderMouse() const
{
    return d->m_view->nodeUnderMouse();
}

DOM::Node TDEHTMLPart::nonSharedNodeUnderMouse() const
{
    return d->m_view->nonSharedNodeUnderMouse();
}

void TDEHTMLPart::emitSelectionChanged()
{
  emit d->m_extension->enableAction( "copy", hasSelection() );
  if ( d->m_findDialog )
       d->m_findDialog->setHasSelection( hasSelection() );

  emit d->m_extension->selectionInfo( selectedText() );
  emit selectionChanged();
}

int TDEHTMLPart::zoomFactor() const
{
  return d->m_zoomFactor;
}

// ### make the list configurable ?
static const int zoomSizes[] = { 20, 40, 60, 80, 90, 95, 100, 105, 110, 120, 140, 160, 180, 200, 250, 300 };
static const int zoomSizeCount = (sizeof(zoomSizes) / sizeof(int));
static const int minZoom = 20;
static const int maxZoom = 300;

// My idea of useful stepping ;-) (LS)
extern const int TDE_NO_EXPORT fastZoomSizes[] = { 20, 50, 75, 90, 100, 120, 150, 200, 300 };
extern const int TDE_NO_EXPORT fastZoomSizeCount = sizeof fastZoomSizes / sizeof fastZoomSizes[0];

void TDEHTMLPart::slotIncZoom()
{
  zoomIn(zoomSizes, zoomSizeCount);
}

void TDEHTMLPart::slotDecZoom()
{
  zoomOut(zoomSizes, zoomSizeCount);
}

void TDEHTMLPart::slotIncZoomFast()
{
  zoomIn(fastZoomSizes, fastZoomSizeCount);
}

void TDEHTMLPart::slotDecZoomFast()
{
  zoomOut(fastZoomSizes, fastZoomSizeCount);
}

void TDEHTMLPart::zoomIn(const int stepping[], int count)
{
  int zoomFactor = d->m_zoomFactor;

  if (zoomFactor < maxZoom) {
    // find the entry nearest to the given zoomsizes
    for (int i = 0; i < count; ++i)
      if (stepping[i] > zoomFactor) {
        zoomFactor = stepping[i];
        break;
      }
    setZoomFactor(zoomFactor);
  }
}

void TDEHTMLPart::zoomOut(const int stepping[], int count)
{
    int zoomFactor = d->m_zoomFactor;
    if (zoomFactor > minZoom) {
      // find the entry nearest to the given zoomsizes
      for (int i = count-1; i >= 0; --i)
        if (stepping[i] < zoomFactor) {
          zoomFactor = stepping[i];
          break;
        }
      setZoomFactor(zoomFactor);
    }
}

void TDEHTMLPart::setZoomFactor (int percent)
{
  if (percent < minZoom) percent = minZoom;
  if (percent > maxZoom) percent = maxZoom;
  if (d->m_zoomFactor == percent) return;
  d->m_zoomFactor = percent;

  if(d->m_doc) {
      TQApplication::setOverrideCursor( TQt::waitCursor );
    if (d->m_doc->styleSelector())
      d->m_doc->styleSelector()->computeFontSizes(d->m_doc->paintDeviceMetrics(), d->m_zoomFactor);
    d->m_doc->recalcStyle( NodeImpl::Force );
    TQApplication::restoreOverrideCursor();
  }

  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !( *it )->m_part.isNull() && (*it)->m_part->inherits( "TDEHTMLPart" ) ) {
      KParts::ReadOnlyPart* const p = ( *it )->m_part;
      static_cast<TDEHTMLPart*>( p )->setZoomFactor(d->m_zoomFactor);
    }

  if ( d->m_guiProfile == BrowserViewGUI ) {
      d->m_paDecZoomFactor->setEnabled( d->m_zoomFactor > minZoom );
      d->m_paIncZoomFactor->setEnabled( d->m_zoomFactor < maxZoom );
  }
}

void TDEHTMLPart::slotZoomView( int delta )
{
  if ( delta < 0 )
    slotIncZoom();
  else
    slotDecZoom();
}

void TDEHTMLPart::setStatusBarText( const TQString& text, StatusBarPriority p)
{
  if (!d->m_statusMessagesEnabled)
    return;

  d->m_statusBarText[p] = text;

  // shift handling ?
  TQString tobe = d->m_statusBarText[BarHoverText];
  if (tobe.isEmpty())
    tobe = d->m_statusBarText[BarOverrideText];
  if (tobe.isEmpty()) {
    tobe = d->m_statusBarText[BarDefaultText];
    if (!tobe.isEmpty() && d->m_jobspeed)
      tobe += " ";
    if (d->m_jobspeed)
      tobe += i18n( "(%1/s)" ).arg( TDEIO::convertSize( d->m_jobspeed ) );
  }
  tobe = "<qt>"+tobe;

  emit ReadOnlyPart::setStatusBarText(tobe);
}


void TDEHTMLPart::setJSStatusBarText( const TQString &text )
{
  setStatusBarText(text, BarOverrideText);
}

void TDEHTMLPart::setJSDefaultStatusBarText( const TQString &text )
{
  setStatusBarText(text, BarDefaultText);
}

TQString TDEHTMLPart::jsStatusBarText() const
{
    return d->m_statusBarText[BarOverrideText];
}

TQString TDEHTMLPart::jsDefaultStatusBarText() const
{
   return d->m_statusBarText[BarDefaultText];
}

TQString TDEHTMLPart::referrer() const
{
   return d->m_referrer;
}

TQString TDEHTMLPart::pageReferrer() const
{
   KURL referrerURL = KURL( d->m_pageReferrer );
   if (referrerURL.isValid())
   {
      TQString protocol = referrerURL.protocol();

      if ((protocol == "http") ||
         ((protocol == "https") && (m_url.protocol() == "https")))
      {
          referrerURL.setRef(TQString());
          referrerURL.setUser(TQString());
          referrerURL.setPass(TQString());
          return referrerURL.url();
      }
   }

   return TQString();
}


TQString TDEHTMLPart::lastModified() const
{
  if ( d->m_lastModified.isEmpty() && m_url.isLocalFile() ) {
    // Local file: set last-modified from the file's mtime.
    // Done on demand to save time when this isn't needed - but can lead
    // to slightly wrong results if updating the file on disk w/o reloading.
    TQDateTime lastModif = TQFileInfo( m_url.path() ).lastModified();
    d->m_lastModified = lastModif.toString( TQt::LocalDate );
  }
  //kdDebug(6050) << "TDEHTMLPart::lastModified: " << d->m_lastModified << endl;
  return d->m_lastModified;
}

void TDEHTMLPart::slotLoadImages()
{
  if (d->m_doc )
    d->m_doc->docLoader()->setAutoloadImages( !d->m_doc->docLoader()->autoloadImages() );

  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if ( !( *it )->m_part.isNull() && (*it)->m_part->inherits( "TDEHTMLPart" ) ) {
      KParts::ReadOnlyPart* const p = ( *it )->m_part;
      static_cast<TDEHTMLPart*>( p )->slotLoadImages();
    }
}

void TDEHTMLPart::reparseConfiguration()
{
  TDEHTMLSettings *settings = TDEHTMLFactory::defaultHTMLSettings();
  settings->init();

  setAutoloadImages( settings->autoLoadImages() );
  if (d->m_doc)
     d->m_doc->docLoader()->setShowAnimations( settings->showAnimations() );

  d->m_bOpenMiddleClick = settings->isOpenMiddleClickEnabled();
  d->m_bBackRightClick = settings->isBackRightClickEnabled();
  d->m_bJScriptEnabled = settings->isJavaScriptEnabled(m_url.host());
  setDebugScript( settings->isJavaScriptDebugEnabled() );
  d->m_bJavaEnabled = settings->isJavaEnabled(m_url.host());
  d->m_bPluginsEnabled = settings->isPluginsEnabled(m_url.host());
  d->m_metaRefreshEnabled = settings->isAutoDelayedActionsEnabled ();

  delete d->m_settings;
  d->m_settings = new TDEHTMLSettings(*TDEHTMLFactory::defaultHTMLSettings());

  TQApplication::setOverrideCursor( TQt::waitCursor );
  tdehtml::CSSStyleSelector::reparseConfiguration();
  if(d->m_doc) d->m_doc->updateStyleSelector();
  TQApplication::restoreOverrideCursor();

  if (TDEHTMLFactory::defaultHTMLSettings()->isAdFilterEnabled())
     runAdFilter();
}

TQStringList TDEHTMLPart::frameNames() const
{
  TQStringList res;

  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if (!(*it)->m_bPreloaded)
      res += (*it)->m_name;

  return res;
}

TQPtrList<KParts::ReadOnlyPart> TDEHTMLPart::frames() const
{
  TQPtrList<KParts::ReadOnlyPart> res;

  ConstFrameIt it = d->m_frames.begin();
  const ConstFrameIt end = d->m_frames.end();
  for (; it != end; ++it )
    if (!(*it)->m_bPreloaded)
      res.append( (*it)->m_part );

  return res;
}

bool TDEHTMLPart::openURLInFrame( const KURL &url, const KParts::URLArgs &urlArgs )
{
    kdDebug( 6050 ) << this << "TDEHTMLPart::openURLInFrame " << url << endl;
  FrameIt it = d->m_frames.find( urlArgs.frameName );

  if ( it == d->m_frames.end() )
    return false;

  // Inform someone that we are about to show something else.
  if ( !urlArgs.lockHistory() )
      emit d->m_extension->openURLNotify();

  requestObject( *it, url, urlArgs );

  return true;
}

void TDEHTMLPart::setDNDEnabled( bool b )
{
  d->m_bDnd = b;
}

bool TDEHTMLPart::dndEnabled() const
{
  return d->m_bDnd;
}

void TDEHTMLPart::customEvent( TQCustomEvent *event )
{
  if ( tdehtml::MousePressEvent::test( event ) )
  {
    tdehtmlMousePressEvent( static_cast<tdehtml::MousePressEvent *>( event ) );
    return;
  }

  if ( tdehtml::MouseDoubleClickEvent::test( event ) )
  {
    tdehtmlMouseDoubleClickEvent( static_cast<tdehtml::MouseDoubleClickEvent *>( event ) );
    return;
  }

  if ( tdehtml::MouseMoveEvent::test( event ) )
  {
    tdehtmlMouseMoveEvent( static_cast<tdehtml::MouseMoveEvent *>( event ) );
    return;
  }

  if ( tdehtml::MouseReleaseEvent::test( event ) )
  {
    tdehtmlMouseReleaseEvent( static_cast<tdehtml::MouseReleaseEvent *>( event ) );
    return;
  }

  if ( tdehtml::DrawContentsEvent::test( event ) )
  {
    tdehtmlDrawContentsEvent( static_cast<tdehtml::DrawContentsEvent *>( event ) );
    return;
  }

  KParts::ReadOnlyPart::customEvent( event );
}

/** returns the position of the first inline text box of the line at
 * coordinate y in renderNode
 *
 * This is a helper function for line-by-line text selection.
 */
static bool firstRunAt(tdehtml::RenderObject *renderNode, int y, NodeImpl *&startNode, long &startOffset)
{
    for (tdehtml::RenderObject *n = renderNode; n; n = n->nextSibling()) {
        if (n->isText()) {
            tdehtml::RenderText* const textRenderer = static_cast<tdehtml::RenderText *>(n);
            const tdehtml::InlineTextBoxArray &runs = textRenderer->inlineTextBoxes();
            const unsigned lim = runs.count();
            for (unsigned i = 0; i != lim; ++i) {
                if (runs[i]->m_y == y && textRenderer->element()) {
                    startNode = textRenderer->element();
                    startOffset = runs[i]->m_start;
                    return true;
                }
            }
        }

        if (firstRunAt(n->firstChild(), y, startNode, startOffset)) {
            return true;
        }
    }

    return false;
}

/** returns the position of the last inline text box of the line at
 * coordinate y in renderNode
 *
 * This is a helper function for line-by-line text selection.
 */
static bool lastRunAt(tdehtml::RenderObject *renderNode, int y, NodeImpl *&endNode, long &endOffset)
{
    tdehtml::RenderObject *n = renderNode;
    if (!n) {
        return false;
    }
    tdehtml::RenderObject *next;
    while ((next = n->nextSibling())) {
        n = next;
    }

    while (1) {
        if (lastRunAt(n->firstChild(), y, endNode, endOffset)) {
            return true;
        }

        if (n->isText()) {
            tdehtml::RenderText* const textRenderer =  static_cast<tdehtml::RenderText *>(n);
            const tdehtml::InlineTextBoxArray &runs = textRenderer->inlineTextBoxes();
            for (int i = (int)runs.count()-1; i >= 0; --i) {
                if (runs[i]->m_y == y && textRenderer->element()) {
                    endNode = textRenderer->element();
                    endOffset = runs[i]->m_start + runs[i]->m_len;
                    return true;
                }
            }
        }

        if (n == renderNode) {
            return false;
        }

        n = n->previousSibling();
    }
}

void TDEHTMLPart::tdehtmlMousePressEvent( tdehtml::MousePressEvent *event )
{
  DOM::DOMString url = event->url();
  TQMouseEvent *_mouse = event->qmouseEvent();
  DOM::Node innerNode = event->innerNode();
  d->m_mousePressNode = innerNode;

   d->m_dragStartPos = _mouse->pos();

   if ( !event->url().isNull() ) {
     d->m_strSelectedURL = event->url().string();
     d->m_strSelectedURLTarget = event->target().string();
   }
   else
     d->m_strSelectedURL = d->m_strSelectedURLTarget = TQString();

  if ( _mouse->button() == TQt::LeftButton ||
       _mouse->button() == TQt::MidButton )
  {
    d->m_bMousePressed = true;

#ifndef TDEHTML_NO_SELECTION
    if ( _mouse->button() == TQt::LeftButton )
    {
      if ( (!d->m_strSelectedURL.isNull() && !isEditable())
	        || (!d->m_mousePressNode.isNull() && d->m_mousePressNode.elementId() == ID_IMG) )
	  return;
      if ( !innerNode.isNull()  && innerNode.handle()->renderer()) {
          int offset = 0;
          DOM::NodeImpl* node = 0;
          tdehtml::RenderObject::SelPointState state;
          innerNode.handle()->renderer()->checkSelectionPoint( event->x(), event->y(),
                                                               event->absX()-innerNode.handle()->renderer()->xPos(),
                                                               event->absY()-innerNode.handle()->renderer()->yPos(), node, offset, state );
          d->m_extendMode = d->ExtendByChar;
#ifdef TDEHTML_NO_CARET
          d->m_selectionStart = node;
          d->m_startOffset = offset;
          //if ( node )
          //  kdDebug(6005) << "TDEHTMLPart::tdehtmlMousePressEvent selectionStart=" << d->m_selectionStart.handle()->renderer()
          //                << " offset=" << d->m_startOffset << endl;
          //else
          //  kdDebug(6005) << "TDEHTML::tdehtmlMousePressEvent selectionStart=(nil)" << endl;
          d->m_selectionEnd = d->m_selectionStart;
          d->m_endOffset = d->m_startOffset;
          d->m_doc->clearSelection();
#else // TDEHTML_NO_CARET
	  d->m_view->moveCaretTo(node, offset, (_mouse->state() & ShiftButton) == 0);
#endif // TDEHTML_NO_CARET
	  d->m_initialNode = d->m_selectionStart;
	  d->m_initialOffset = d->m_startOffset;
//           kdDebug(6000) << "press: initOfs " << d->m_initialOffset << endl;
      }
      else
      {
#ifndef TDEHTML_NO_CARET
        // simply leave it. Is this a good idea?
#else
        d->m_selectionStart = DOM::Node();
        d->m_selectionEnd = DOM::Node();
#endif
      }
      emitSelectionChanged();
      startAutoScroll();
    }
#else
    d->m_dragLastPos = _mouse->globalPos();
#endif
  }

  if ( _mouse->button() == TQt::RightButton && parentPart() != 0 && d->m_bBackRightClick )
  {
    d->m_bRightMousePressed = true;
  } else if ( _mouse->button() == TQt::RightButton )
  {
    popupMenu( d->m_strSelectedURL );
    // might be deleted, don't touch "this"
  }
}

void TDEHTMLPart::tdehtmlMouseDoubleClickEvent( tdehtml::MouseDoubleClickEvent *event )
{
  TQMouseEvent *_mouse = event->qmouseEvent();
  if ( _mouse->button() == TQt::LeftButton )
  {
    d->m_bMousePressed = true;
    DOM::Node innerNode = event->innerNode();
    // Find selectionStart again, tdehtmlMouseReleaseEvent lost it
    if ( !innerNode.isNull() && innerNode.handle()->renderer()) {
      int offset = 0;
      DOM::NodeImpl* node = 0;
      tdehtml::RenderObject::SelPointState state;
      innerNode.handle()->renderer()->checkSelectionPoint( event->x(), event->y(),
                                                           event->absX()-innerNode.handle()->renderer()->xPos(),
                                                           event->absY()-innerNode.handle()->renderer()->yPos(), node, offset, state);

      //kdDebug() << k_funcinfo << "checkSelectionPoint returned node=" << node << " offset=" << offset << endl;

      if ( node && node->renderer() )
      {
        // Extend selection to a complete word (double-click) or line (triple-click)
        bool selectLine = (event->clickCount() == 3);
        d->m_extendMode = selectLine ? d->ExtendByLine : d->ExtendByWord;

	// Extend existing selection if Shift was pressed
	if (_mouse->state() & ShiftButton) {
          d->caretNode() = node;
	  d->caretOffset() = offset;
          d->m_startBeforeEnd = RangeImpl::compareBoundaryPoints(
      			d->m_selectionStart.handle(), d->m_startOffset,
			d->m_selectionEnd.handle(), d->m_endOffset) <= 0;
          d->m_initialNode = d->m_extendAtEnd ? d->m_selectionStart : d->m_selectionEnd;
          d->m_initialOffset = d->m_extendAtEnd ? d->m_startOffset : d->m_endOffset;
	} else {
	  d->m_selectionStart = d->m_selectionEnd = node;
	  d->m_startOffset = d->m_endOffset = offset;
          d->m_startBeforeEnd = true;
          d->m_initialNode = node;
          d->m_initialOffset = offset;
	}
//         kdDebug(6000) << "dblclk: initOfs " << d->m_initialOffset << endl;

        // Extend the start
        extendSelection( d->m_selectionStart.handle(), d->m_startOffset, d->m_selectionStart, d->m_startOffset, !d->m_startBeforeEnd, selectLine );
        // Extend the end
        extendSelection( d->m_selectionEnd.handle(), d->m_endOffset, d->m_selectionEnd, d->m_endOffset, d->m_startBeforeEnd, selectLine );

        //kdDebug() << d->m_selectionStart.handle() << " " << d->m_startOffset << "  -  " <<
        //  d->m_selectionEnd.handle() << " " << d->m_endOffset << endl;

        emitSelectionChanged();
        d->m_doc
          ->setSelection(d->m_selectionStart.handle(),d->m_startOffset,
                         d->m_selectionEnd.handle(),d->m_endOffset);
#ifndef TDEHTML_NO_CARET
        bool v = d->m_view->placeCaret();
        emitCaretPositionChanged(v ? d->caretNode() : 0, d->caretOffset());
#endif
        startAutoScroll();
      }
    }
  }
}

void TDEHTMLPart::extendSelection( DOM::NodeImpl* node, long offset, DOM::Node& selectionNode, long& selectionOffset, bool right, bool selectLines )
{
  tdehtml::RenderObject* obj = node->renderer();

  if (obj->isText() && selectLines) {
    int pos;
    tdehtml::RenderText *renderer = static_cast<tdehtml::RenderText *>(obj);
    tdehtml::InlineTextBox *run = renderer->findInlineTextBox( offset, pos );
    DOMString t = node->nodeValue();
    DOM::NodeImpl* selNode = 0;
    long selOfs = 0;

    if (!run)
      return;

    int selectionPointY = run->m_y;

    // Go up to first non-inline element.
    tdehtml::RenderObject *renderNode = renderer;
    while (renderNode && renderNode->isInline())
      renderNode = renderNode->parent();

    renderNode = renderNode->firstChild();

    if (right) {
      // Look for all the last child in the block that is on the same line
      // as the selection point.
      if (!lastRunAt (renderNode, selectionPointY, selNode, selOfs))
        return;
    } else {
      // Look for all the first child in the block that is on the same line
      // as the selection point.
      if (!firstRunAt (renderNode, selectionPointY, selNode, selOfs))
        return;
    }

    selectionNode = selNode;
    selectionOffset = selOfs;
    return;
  }

  TQString str;
  int len = 0;
  if ( obj->isText() ) { // can be false e.g. when double-clicking on a disabled submit button
    str = static_cast<tdehtml::RenderText *>(obj)->data().string();
    len = str.length();
  }
  //kdDebug() << "extendSelection right=" << right << " offset=" << offset << " len=" << len << " Starting at obj=" << obj << endl;
  TQChar ch;
  do {
    // Last char was ok, point to it
    if ( node ) {
      selectionNode = node;
      selectionOffset = offset;
    }

    // Get another char
    while ( obj && ( (right && offset >= len-1) || (!right && offset <= 0) ) )
    {
      obj = right ? obj->objectBelow() : obj->objectAbove();
      //kdDebug() << "obj=" << obj << endl;
      if ( obj ) {
        //kdDebug() << "isText=" << obj->isText() << endl;
        str = TQString();
        if ( obj->isText() )
          str = static_cast<tdehtml::RenderText *>(obj)->data().string();
        else if ( obj->isBR() )
          str = '\n';
        else if ( !obj->isInline() ) {
          obj = 0L; // parag limit -> done
          break;
        }
        len = str.length();
        //kdDebug() << "str=" << str << " length=" << len << endl;
        // set offset - note that the first thing will be a ++ or -- on it.
        if ( right )
          offset = -1;
        else
          offset = len;
      }
    }
    if ( !obj ) // end of parag or document
      break;
    node = obj->element();
    if ( right )
    {
      Q_ASSERT( offset < len-1 );
      ++offset;
    }
    else
    {
      Q_ASSERT( offset > 0 );
      --offset;
    }

    // Test that char
    ch = str[ (int)offset ];
    //kdDebug() << " offset=" << offset << " ch=" << TQString(ch) << endl;
  } while ( !ch.isSpace() && !ch.isPunct() );

  // make offset point after last char
  if (right) ++selectionOffset;
}

#ifndef TDEHTML_NO_SELECTION
void TDEHTMLPart::extendSelectionTo(int x, int y, int absX, int absY, const DOM::Node &innerNode)
{
      int offset;
      //kdDebug(6000) << "TDEHTMLPart::tdehtmlMouseMoveEvent x=" << event->x() << " y=" << event->y() << endl;
      DOM::NodeImpl* node=0;
      tdehtml::RenderObject::SelPointState state;
      innerNode.handle()->renderer()->checkSelectionPoint( x, y,
                                                           absX-innerNode.handle()->renderer()->xPos(),
                                                           absY-innerNode.handle()->renderer()->yPos(), node, offset, state);
      if (!node || !node->renderer()) return;

      // Words at the beginning/end of line cannot be deselected in
      // ExtendByWord mode. Therefore, do not enforce it if the selection
      // point does not match the node under the mouse cursor.
      bool withinNode = innerNode == node;

      // we have to get to know if end is before start or not...
      // shouldn't be null but it can happen with dynamic updating of nodes
      if (d->m_selectionStart.isNull() || d->m_selectionEnd.isNull() ||
          d->m_initialNode.isNull() ||
          !d->m_selectionStart.handle()->renderer() ||
          !d->m_selectionEnd.handle()->renderer()) return;

      if (d->m_extendMode != d->ExtendByChar) {
        // check whether we should extend at the front, or at the back
        bool caretBeforeInit = RangeImpl::compareBoundaryPoints(
      			d->caretNode().handle(), d->caretOffset(),
			d->m_initialNode.handle(), d->m_initialOffset) <= 0;
        bool nodeBeforeInit = RangeImpl::compareBoundaryPoints(node, offset,
			d->m_initialNode.handle(), d->m_initialOffset) <= 0;
        // have to fix up start to point to the original end
        if (caretBeforeInit != nodeBeforeInit) {
//         kdDebug(6000) << "extto cbi: " << caretBeforeInit << " startBefEnd " << d->m_startBeforeEnd << " extAtEnd " << d->m_extendAtEnd << " (" << d->m_startOffset << ") - (" << d->m_endOffset << ")" << " initOfs " << d->m_initialOffset << endl;
          extendSelection(d->m_initialNode.handle(), d->m_initialOffset,
	  	d->m_extendAtEnd ? d->m_selectionStart : d->m_selectionEnd,
		d->m_extendAtEnd ? d->m_startOffset : d->m_endOffset,
		nodeBeforeInit, d->m_extendMode == d->ExtendByLine);
	}
      }

      d->caretNode() = node;
      d->caretOffset() = offset;
      //kdDebug( 6000 ) << "setting end of selection to " << d->m_selectionEnd.handle() << "/" << d->m_endOffset << endl;

      d->m_startBeforeEnd = RangeImpl::compareBoundaryPoints(
      			d->m_selectionStart.handle(), d->m_startOffset,
			d->m_selectionEnd.handle(), d->m_endOffset) <= 0;

      if ( !d->m_selectionStart.isNull() && !d->m_selectionEnd.isNull() )
      {
//         kdDebug(6000) << "extto: startBefEnd " << d->m_startBeforeEnd << " extAtEnd " << d->m_extendAtEnd << " (" << d->m_startOffset << ") - (" << d->m_endOffset << ")" << " initOfs " << d->m_initialOffset << endl;
        if (d->m_extendMode != d->ExtendByChar && withinNode)
          extendSelection( node, offset, d->caretNode(), d->caretOffset(), d->m_startBeforeEnd ^ !d->m_extendAtEnd, d->m_extendMode == d->ExtendByLine );

        if (d->m_selectionEnd == d->m_selectionStart && d->m_endOffset < d->m_startOffset)
          d->m_doc
            ->setSelection(d->m_selectionStart.handle(),d->m_endOffset,
                           d->m_selectionEnd.handle(),d->m_startOffset);
        else if (d->m_startBeforeEnd)
          d->m_doc
            ->setSelection(d->m_selectionStart.handle(),d->m_startOffset,
                           d->m_selectionEnd.handle(),d->m_endOffset);
        else
          d->m_doc
            ->setSelection(d->m_selectionEnd.handle(),d->m_endOffset,
                           d->m_selectionStart.handle(),d->m_startOffset);
      }
#ifndef TDEHTML_NO_CARET
      d->m_view->placeCaret();
#endif
}

bool TDEHTMLPart::isExtendingSelection() const
{
  // This is it, the whole detection. tdehtmlMousePressEvent only sets this
  // on LMB or MMB, but never on RMB. As text selection doesn't work for MMB,
  // it's sufficient to only rely on this flag to detect selection extension.
  return d->m_bMousePressed;
}
#endif // TDEHTML_NO_SELECTION

void TDEHTMLPart::tdehtmlMouseMoveEvent( tdehtml::MouseMoveEvent *event )
{
  TQMouseEvent *_mouse = event->qmouseEvent();

  if( d->m_bRightMousePressed && parentPart() != 0 && d->m_bBackRightClick )
  {
    popupMenu( d->m_strSelectedURL );
    d->m_strSelectedURL = d->m_strSelectedURLTarget = TQString();
    d->m_bRightMousePressed = false;
  }

  DOM::DOMString url = event->url();
  DOM::DOMString target = event->target();
  DOM::Node innerNode = event->innerNode();

#ifndef TQT_NO_DRAGANDDROP
  if( d->m_bDnd && d->m_bMousePressed &&
      ( (!d->m_strSelectedURL.isEmpty() && !isEditable())
        || (!d->m_mousePressNode.isNull() && d->m_mousePressNode.elementId() == ID_IMG) ) ) {
    if ( ( d->m_dragStartPos - _mouse->pos() ).manhattanLength() <= TDEGlobalSettings::dndEventDelay() )
      return;

    TQPixmap pix;
    HTMLImageElementImpl *img = 0L;
    TQDragObject *drag = 0;
    KURL u;

    // tqDebug("****************** Event URL: %s", url.string().latin1());
    // tqDebug("****************** Event Target: %s", target.string().latin1());

    // Normal image...
    if ( url.length() == 0 && innerNode.handle() && innerNode.handle()->id() == ID_IMG )
    {
      img = static_cast<HTMLImageElementImpl *>(innerNode.handle());
      u = KURL( completeURL( tdehtml::parseURL(img->getAttribute(ATTR_SRC)).string() ) );
      pix = KMimeType::mimeType("image/png")->pixmap(TDEIcon::Desktop);
    }
    else
    {
      // Text or image link...
      u = completeURL( d->m_strSelectedURL );
      pix = KMimeType::pixmapForURL(u, 0, TDEIcon::Desktop, TDEIcon::SizeMedium);
    }

    u.setPass(TQString());

    KURLDrag* urlDrag = new KURLDrag( u, img ? 0 : d->m_view->viewport() );
    if ( !d->m_referrer.isEmpty() )
      urlDrag->metaData()["referrer"] = d->m_referrer;

    if( img && img->complete()) {
      KMultipleDrag *mdrag = new KMultipleDrag( d->m_view->viewport() );
      mdrag->addDragObject( new TQImageDrag( img->currentImage(), 0L ) );
      mdrag->addDragObject( urlDrag );
      drag = mdrag;
    }
    else
      drag = urlDrag;

    if ( !pix.isNull() )
      drag->setPixmap( pix );

    stopAutoScroll();
    if(drag)
      drag->drag();

    // when we finish our drag, we need to undo our mouse press
    d->m_bMousePressed = false;
    d->m_strSelectedURL = d->m_strSelectedURLTarget = TQString();
    return;
  }
#endif

  // Not clicked -> mouse over stuff
  if ( !d->m_bMousePressed )
  {
    // The mouse is over something
    if ( url.length() )
    {
      bool shiftPressed = ( _mouse->state() & ShiftButton );

      // Image map
      if ( !innerNode.isNull() && innerNode.elementId() == ID_IMG )
      {
        HTMLImageElementImpl *i = static_cast<HTMLImageElementImpl *>(innerNode.handle());
        if ( i && i->isServerMap() )
        {
          tdehtml::RenderObject *r = i->renderer();
          if(r)
          {
            int absx, absy, vx, vy;
            r->absolutePosition(absx, absy);
            view()->contentsToViewport( absx, absy, vx, vy );

            int x(_mouse->x() - vx), y(_mouse->y() - vy);

            d->m_overURL = url.string() + TQString("?%1,%2").arg(x).arg(y);
            d->m_overURLTarget = target.string();
            overURL( d->m_overURL, target.string(), shiftPressed );
            return;
          }
        }
      }

      // normal link
      if ( d->m_overURL.isEmpty() || d->m_overURL != url || d->m_overURLTarget != target )
      {
        d->m_overURL = url.string();
        d->m_overURLTarget = target.string();
        overURL( d->m_overURL, target.string(), shiftPressed );
      }
    }
    else  // Not over a link...
    {
      // reset to "default statusbar text"
      resetHoverText();
    }
  }
  else {
#ifndef TDEHTML_NO_SELECTION
    // selection stuff
    if( d->m_bMousePressed && innerNode.handle() && innerNode.handle()->renderer() &&
        ( (_mouse->state() & TQt::LeftButton) != 0 )) {
      extendSelectionTo(event->x(), event->y(),
                        event->absX(), event->absY(), innerNode);
#else
      if ( d->m_doc && d->m_view ) {
        TQPoint diff( _mouse->globalPos() - d->m_dragLastPos );

        if ( abs( diff.x() ) > 64 || abs( diff.y() ) > 64 ) {
          d->m_view->scrollBy( -diff.x(), -diff.y() );
          d->m_dragLastPos = _mouse->globalPos();
        }
#endif
    }
  }

}

void TDEHTMLPart::tdehtmlMouseReleaseEvent( tdehtml::MouseReleaseEvent *event )
{
  DOM::Node innerNode = event->innerNode();
  d->m_mousePressNode = DOM::Node();

  if ( d->m_bMousePressed ) {
    setStatusBarText(TQString(), BarHoverText);
    stopAutoScroll();
  }

  // Used to prevent mouseMoveEvent from initiating a drag before
  // the mouse is pressed again.
  d->m_bMousePressed = false;

  TQMouseEvent *_mouse = event->qmouseEvent();
  if ( _mouse->button() == TQt::RightButton && parentPart() != 0 && d->m_bBackRightClick )
  {
    d->m_bRightMousePressed = false;
    KParts::BrowserInterface *tmp_iface = d->m_extension->browserInterface();
    if( tmp_iface ) {
      tmp_iface->callMethod( "goHistory(int)", -1 );
    }
  }
#ifndef TQT_NO_CLIPBOARD
  if ((d->m_guiProfile == BrowserViewGUI) && (_mouse->button() == TQt::MidButton) && (event->url().isNull())) {
    kdDebug( 6050 ) << "TDEHTMLPart::tdehtmlMouseReleaseEvent() MMB shouldOpen="
                    << d->m_bOpenMiddleClick << endl;

    if (d->m_bOpenMiddleClick) {
    TDEHTMLPart *p = this;
    while (p->parentPart()) p = p->parentPart();
    p->d->m_extension->pasteRequest();
  }
  }
#endif

#ifndef TDEHTML_NO_SELECTION
  // delete selection in case start and end position are at the same point
  if(d->m_selectionStart == d->m_selectionEnd && d->m_startOffset == d->m_endOffset) {
#ifndef TDEHTML_NO_CARET
    d->m_extendAtEnd = true;
#else
    d->m_selectionStart = 0;
    d->m_selectionEnd = 0;
    d->m_startOffset = 0;
    d->m_endOffset = 0;
#endif
    emitSelectionChanged();
  } else {
    // we have to get to know if end is before start or not...
//     kdDebug(6000) << "rel: startBefEnd " << d->m_startBeforeEnd << " extAtEnd " << d->m_extendAtEnd << " (" << d->m_startOffset << ") - (" << d->m_endOffset << ")" << endl;
    DOM::Node n = d->m_selectionStart;
    d->m_startBeforeEnd = false;
    if( d->m_selectionStart == d->m_selectionEnd ) {
      if( d->m_startOffset < d->m_endOffset )
        d->m_startBeforeEnd = true;
    } else {
#if 0
      while(!n.isNull()) {
        if(n == d->m_selectionEnd) {
          d->m_startBeforeEnd = true;
          break;
        }
        DOM::Node next = n.firstChild();
        if(next.isNull()) next = n.nextSibling();
        while( next.isNull() && !n.parentNode().isNull() ) {
          n = n.parentNode();
          next = n.nextSibling();
        }
        n = next;
      }
#else
      // shouldn't be null but it can happen with dynamic updating of nodes
      if (d->m_selectionStart.isNull() || d->m_selectionEnd.isNull() ||
          !d->m_selectionStart.handle()->renderer() ||
          !d->m_selectionEnd.handle()->renderer()) return;
      d->m_startBeforeEnd = RangeImpl::compareBoundaryPoints(
      			d->m_selectionStart.handle(), d->m_startOffset,
			d->m_selectionEnd.handle(), d->m_endOffset) <= 0;
#endif
    }
    if(!d->m_startBeforeEnd)
    {
      DOM::Node tmpNode = d->m_selectionStart;
      int tmpOffset = d->m_startOffset;
      d->m_selectionStart = d->m_selectionEnd;
      d->m_startOffset = d->m_endOffset;
      d->m_selectionEnd = tmpNode;
      d->m_endOffset = tmpOffset;
      d->m_startBeforeEnd = true;
      d->m_extendAtEnd = !d->m_extendAtEnd;
    }
#ifndef TDEHTML_NO_CARET
    bool v = d->m_view->placeCaret();
    emitCaretPositionChanged(v ? d->caretNode() : 0, d->caretOffset());
#endif
    // get selected text and paste to the clipboard
#ifndef TQT_NO_CLIPBOARD
    TQString text = selectedText();
    text.replace(TQChar(0xa0), ' ');
    disconnect( kapp->clipboard(), TQ_SIGNAL( selectionChanged()), this, TQ_SLOT( slotClearSelection()));
    kapp->clipboard()->setText(text,TQClipboard::Selection);
    connect( kapp->clipboard(), TQ_SIGNAL( selectionChanged()), TQ_SLOT( slotClearSelection()));
#endif
    //kdDebug( 6000 ) << "selectedText = " << text << endl;
    emitSelectionChanged();
//kdDebug(6000) << "rel2: startBefEnd " << d->m_startBeforeEnd << " extAtEnd " << d->m_extendAtEnd << " (" << d->m_startOffset << ") - (" << d->m_endOffset << "), caretOfs " << d->caretOffset() << endl;
  }
#endif
  d->m_initialNode = 0;		// don't hold nodes longer than necessary
  d->m_initialOffset = 0;

}

void TDEHTMLPart::tdehtmlDrawContentsEvent( tdehtml::DrawContentsEvent * )
{
}

void TDEHTMLPart::guiActivateEvent( KParts::GUIActivateEvent *event )
{
  if ( event->activated() )
  {
    emitSelectionChanged();
    emit d->m_extension->enableAction( "print", d->m_doc != 0 );

    if ( !d->m_settings->autoLoadImages() && d->m_paLoadImages )
    {
        TQPtrList<TDEAction> lst;
        lst.append( d->m_paLoadImages );
        plugActionList( "loadImages", lst );
    }
  }
}

void TDEHTMLPart::slotPrintFrame()
{
  if ( d->m_frames.count() == 0 )
    return;

  KParts::ReadOnlyPart *frame = currentFrame();
  if (!frame)
    return;

  KParts::BrowserExtension *ext = KParts::BrowserExtension::childObject( frame );

  if ( !ext )
    return;

  TQMetaObject *mo = ext->metaObject();

  int idx = mo->findSlot( "print()", true );
  if ( idx >= 0 ) {
    TQUObject o[ 1 ];
    ext->tqt_invoke( idx, o );
  }
}

void TDEHTMLPart::slotSelectAll()
{
  KParts::ReadOnlyPart *part = currentFrame();
  if (part && part->inherits("TDEHTMLPart"))
    static_cast<TDEHTMLPart *>(part)->selectAll();
}

void TDEHTMLPart::startAutoScroll()
{
   connect(&d->m_scrollTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( slotAutoScroll() ));
   d->m_scrollTimer.start(100, false);
}

void TDEHTMLPart::stopAutoScroll()
{
   disconnect(&d->m_scrollTimer, TQ_SIGNAL( timeout() ), this, TQ_SLOT( slotAutoScroll() ));
   if (d->m_scrollTimer.isActive())
       d->m_scrollTimer.stop();
}


void TDEHTMLPart::slotAutoScroll()
{
    if (d->m_view)
      d->m_view->doAutoScroll();
    else
      stopAutoScroll(); // Safety
}

void TDEHTMLPart::runAdFilter()
{
    if ( parentPart() )
        parentPart()->runAdFilter();

    if ( !d->m_doc )
        return;

    TQPtrDictIterator<tdehtml::CachedObject> it( d->m_doc->docLoader()->m_docObjects );
    for ( ; it.current(); ++it )
        if ( it.current()->type() == tdehtml::CachedObject::Image ) {
            tdehtml::CachedImage *image = static_cast<tdehtml::CachedImage *>(it.current());
            bool wasBlocked = image->m_wasBlocked;
            image->m_wasBlocked = TDEHTMLFactory::defaultHTMLSettings()->isAdFiltered( d->m_doc->completeURL( (*it).url().string() ) );
            if ( image->m_wasBlocked != wasBlocked )
                image->do_notify(image->pixmap(), image->valid_rect());
        }

    if ( TDEHTMLFactory::defaultHTMLSettings()->isHideAdsEnabled() ) {
        for ( NodeImpl *nextNode, *node = d->m_doc; node; node = nextNode ) {

            // We might be deleting 'node' shortly.
            nextNode = node->traverseNextNode();

            if ( node->id() == ID_IMG ||
                 node->id() == ID_IFRAME ||
                 (node->id() == ID_INPUT && static_cast<HTMLInputElementImpl *>(node)->inputType() == HTMLInputElementImpl::IMAGE ))
            {
                if ( TDEHTMLFactory::defaultHTMLSettings()->isAdFiltered( d->m_doc->completeURL( static_cast<ElementImpl *>(node)->getAttribute(ATTR_SRC).string() ) ) )
                {
                    // We found an IMG, IFRAME or INPUT (of type IMAGE) matching a filter.
                    node->ref();
                    NodeImpl *parent = node->parent();
                    if( parent )
                    {
                        int exception = 0;
                        parent->removeChild(node, exception);
                    }
                    node->deref();
                }
            }
        }
    }
}

void TDEHTMLPart::selectAll()
{
  if (!d->m_doc) return;

  NodeImpl *first;
  if (d->m_doc->isHTMLDocument())
    first = static_cast<HTMLDocumentImpl*>(d->m_doc)->body();
  else
    first = d->m_doc;
  NodeImpl *next;

  // Look for first text/cdata node that has a renderer,
  // or first childless replaced element
  while ( first && !(first->renderer()
  	&& ((first->nodeType() == Node::TEXT_NODE || first->nodeType() == Node::CDATA_SECTION_NODE)
		|| (first->renderer()->isReplaced() && !first->renderer()->firstChild()))))
  {
    next = first->firstChild();
    if ( !next ) next = first->nextSibling();
    while( first && !next )
    {
      first = first->parentNode();
      if ( first )
        next = first->nextSibling();
    }
    first = next;
  }

  NodeImpl *last;
  if (d->m_doc->isHTMLDocument())
    last = static_cast<HTMLDocumentImpl*>(d->m_doc)->body();
  else
    last = d->m_doc;
  // Look for last text/cdata node that has a renderer,
  // or last childless replaced element
  // ### Instead of changing this loop, use findLastSelectableNode
  // in render_table.cpp (LS)
  while ( last && !(last->renderer()
  	&& ((last->nodeType() == Node::TEXT_NODE || last->nodeType() == Node::CDATA_SECTION_NODE)
		|| (last->renderer()->isReplaced() && !last->renderer()->lastChild()))))
  {
    next = last->lastChild();
    if ( !next ) next = last->previousSibling();
    while ( last && !next )
    {
      last = last->parentNode();
      if ( last )
        next = last->previousSibling();
    }
    last = next;
  }

  if ( !first || !last )
    return;
  Q_ASSERT(first->renderer());
  Q_ASSERT(last->renderer());
  d->m_selectionStart = first;
  d->m_startOffset = 0;
  d->m_selectionEnd = last;
  d->m_endOffset = last->nodeValue().length();
  d->m_startBeforeEnd = true;

  d->m_doc->setSelection( d->m_selectionStart.handle(), d->m_startOffset,
                          d->m_selectionEnd.handle(), d->m_endOffset );

  emitSelectionChanged();
}

bool TDEHTMLPart::checkLinkSecurity(const KURL &linkURL,const TQString &message, const TQString &button)
{
  bool linkAllowed = true;

  if ( d->m_doc )
    linkAllowed = kapp && kapp->authorizeURLAction("redirect", url(), linkURL);

  if ( !linkAllowed ) {
    tdehtml::Tokenizer *tokenizer = d->m_doc->tokenizer();
    if (tokenizer)
      tokenizer->setOnHold(true);

    int response = KMessageBox::Cancel;
    if (!message.isEmpty())
    {
	    response = KMessageBox::warningContinueCancel( 0,
							   message.arg(linkURL.htmlURL()),
							   i18n( "Security Warning" ),
							   button);
    }
    else
    {
	    KMessageBox::error( 0,
				i18n( "<qt>Access by untrusted page to<BR><B>%1</B><BR> denied.").arg(linkURL.htmlURL()),
				i18n( "Security Alert" ));
    }

    if (tokenizer)
       tokenizer->setOnHold(false);
    return (response==KMessageBox::Continue);
  }
  return true;
}

void TDEHTMLPart::slotPartRemoved( KParts::Part *part )
{
//    kdDebug(6050) << "TDEHTMLPart::slotPartRemoved " << part << endl;
    if ( part == d->m_activeFrame )
    {
        d->m_activeFrame = 0L;
        if ( !part->inherits( "TDEHTMLPart" ) )
        {
            if (factory()) {
                factory()->removeClient( part );
            }
            if (childClients()->containsRef(part)) {
                removeChildClient( part );
            }
        }
    }
}

void TDEHTMLPart::slotActiveFrameChanged( KParts::Part *part )
{
//    kdDebug(6050) << "TDEHTMLPart::slotActiveFrameChanged this=" << this << "part=" << part << endl;
    if ( part == this )
    {
        kdError(6050) << "strange error! we activated ourselves" << endl;
        assert( false );
        return;
    }
//    kdDebug(6050) << "TDEHTMLPart::slotActiveFrameChanged d->m_activeFrame=" << d->m_activeFrame << endl;
    if ( d->m_activeFrame && d->m_activeFrame->widget() && d->m_activeFrame->widget()->inherits( "TQFrame" ) )
    {
        TQFrame *frame = static_cast<TQFrame *>( d->m_activeFrame->widget() );
        if (frame->frameStyle() != TQFrame::NoFrame)
        {
           frame->setFrameStyle( TQFrame::StyledPanel | TQFrame::Sunken);
           frame->repaint();
        }
    }

    if( d->m_activeFrame && !d->m_activeFrame->inherits( "TDEHTMLPart" ) )
    {
        if (factory()) {
            factory()->removeClient( d->m_activeFrame );
        }
        removeChildClient( d->m_activeFrame );
    }
    if( part && !part->inherits( "TDEHTMLPart" ) )
    {
        if (factory()) {
            factory()->addClient( part );
        }
        insertChildClient( part );
    }


    d->m_activeFrame = part;

    if ( d->m_activeFrame && d->m_activeFrame->widget()->inherits( "TQFrame" ) )
    {
        TQFrame *frame = static_cast<TQFrame *>( d->m_activeFrame->widget() );
        if (frame->frameStyle() != TQFrame::NoFrame)
        {
           frame->setFrameStyle( TQFrame::StyledPanel | TQFrame::Plain);
           frame->repaint();
        }
        kdDebug(6050) << "new active frame " << d->m_activeFrame << endl;
    }

    updateActions();

    // (note: childObject returns 0 if the argument is 0)
    d->m_extension->setExtensionProxy( KParts::BrowserExtension::childObject( d->m_activeFrame ) );
}

void TDEHTMLPart::setActiveNode(const DOM::Node &node)
{
    if (!d->m_doc || !d->m_view)
        return;

    // Set the document's active node
    d->m_doc->setFocusNode(node.handle());

    // Scroll the view if necessary to ensure that the new focus node is visible
    TQRect rect  = node.handle()->getRect();
    d->m_view->ensureVisible(rect.right(), rect.bottom());
    d->m_view->ensureVisible(rect.left(), rect.top());
}

DOM::Node TDEHTMLPart::activeNode() const
{
    return DOM::Node(d->m_doc?d->m_doc->focusNode():0);
}

DOM::EventListener *TDEHTMLPart::createHTMLEventListener( TQString code, TQString name, NodeImpl* node )
{
  KJSProxy *proxy = jScript();

  if (!proxy)
    return 0;

  return proxy->createHTMLEventHandler( m_url.url(), name, code, node );
}

TDEHTMLPart *TDEHTMLPart::opener()
{
    return d->m_opener;
}

void TDEHTMLPart::setOpener(TDEHTMLPart *_opener)
{
    d->m_opener = _opener;
}

bool TDEHTMLPart::openedByJS()
{
    return d->m_openedByJS;
}

void TDEHTMLPart::setOpenedByJS(bool _openedByJS)
{
    d->m_openedByJS = _openedByJS;
}

void TDEHTMLPart::preloadStyleSheet(const TQString &url, const TQString &stylesheet)
{
    tdehtml::Cache::preloadStyleSheet(url, stylesheet);
}

void TDEHTMLPart::preloadScript(const TQString &url, const TQString &script)
{
    tdehtml::Cache::preloadScript(url, script);
}

TQCString TDEHTMLPart::dcopObjectId() const
{
  TQCString id;
  id.sprintf("html-widget%d", d->m_dcop_counter);
  return id;
}

long TDEHTMLPart::cacheId() const
{
  return d->m_cacheId;
}

bool TDEHTMLPart::restored() const
{
  return d->m_restored;
}

bool TDEHTMLPart::pluginPageQuestionAsked(const TQString& mimetype) const
{
  // parentPart() should be const!
  TDEHTMLPart* parent = const_cast<TDEHTMLPart *>(this)->parentPart();
  if ( parent )
    return parent->pluginPageQuestionAsked(mimetype);

  return d->m_pluginPageQuestionAsked.contains(mimetype);
}

void TDEHTMLPart::setPluginPageQuestionAsked(const TQString& mimetype)
{
  if ( parentPart() )
    parentPart()->setPluginPageQuestionAsked(mimetype);

  d->m_pluginPageQuestionAsked.append(mimetype);
}

void TDEHTMLPart::slotAutomaticDetectionLanguage( int _id )
{
  d->m_automaticDetection->setItemChecked( _id, true );

  switch ( _id ) {
    case 0 :
      d->m_autoDetectLanguage = tdehtml::Decoder::SemiautomaticDetection;
      break;
    case 1 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Arabic;
      break;
    case 2 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Baltic;
      break;
    case 3 :
      d->m_autoDetectLanguage = tdehtml::Decoder::CentralEuropean;
      break;
    case 4 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Chinese;
      break;
    case 5 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Greek;
      break;
    case 6 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Hebrew;
      break;
    case 7 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Japanese;
      break;
    case 8 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Korean;
      break;
    case 9 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Russian;
      break;
    case 10 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Thai;
      break;
    case 11 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Turkish;
      break;
    case 12 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Ukrainian;
      break;
    case 13 :
      d->m_autoDetectLanguage = tdehtml::Decoder::Unicode;
      break;
    case 14 :
      d->m_autoDetectLanguage = tdehtml::Decoder::WesternEuropean;
      break;
    default :
      d->m_autoDetectLanguage = tdehtml::Decoder::SemiautomaticDetection;
      break;
  }

  for ( int i = 0; i <= 14; ++i ) {
    if ( i != _id )
      d->m_automaticDetection->setItemChecked( i, false );
  }

  d->m_paSetEncoding->popupMenu()->setItemChecked( 0, true );

  setEncoding( TQString(), false );

  if( d->m_manualDetection )
    d->m_manualDetection->setCurrentItem( -1 );
  d->m_paSetEncoding->popupMenu()->setItemChecked( d->m_paSetEncoding->popupMenu()->idAt( 2 ), false );
}

tdehtml::Decoder *TDEHTMLPart::createDecoder()
{
    tdehtml::Decoder *dec = new tdehtml::Decoder();
    if( !d->m_encoding.isNull() )
        dec->setEncoding( d->m_encoding.latin1(),
            d->m_haveEncoding ? tdehtml::Decoder::UserChosenEncoding : tdehtml::Decoder::EncodingFromHTTPHeader);
    else {
        // Inherit the default encoding from the parent frame if there is one.
        const char *defaultEncoding = (parentPart() && parentPart()->d->m_decoder)
            ? parentPart()->d->m_decoder->encoding() : settings()->encoding().latin1();
        dec->setEncoding(defaultEncoding, tdehtml::Decoder::DefaultEncoding);
    }
#ifdef APPLE_CHANGES
    if (d->m_doc)
        d->m_doc->setDecoder(d->m_decoder);
#endif
    dec->setAutoDetectLanguage( d->m_autoDetectLanguage );
    return dec;
}

void TDEHTMLPart::emitCaretPositionChanged(const DOM::Node &node, long offset) {
  emit caretPositionChanged(node, offset);
}

void TDEHTMLPart::restoreScrollPosition()
{
  KParts::URLArgs args = d->m_extension->urlArgs();

  if ( m_url.hasRef() && !d->m_restoreScrollPosition && !args.reload) {
    if ( !d->m_doc || !d->m_doc->parsing() )
      disconnect(d->m_view, TQ_SIGNAL(finishedLayout()), this, TQ_SLOT(restoreScrollPosition()));
    if ( !gotoAnchor(m_url.encodedHtmlRef()) )
      gotoAnchor(m_url.htmlRef());
    return;
  }

  // Check whether the viewport has become large enough to encompass the stored
  // offsets. If the document has been fully loaded, force the new coordinates,
  // even if the canvas is too short (can happen when user resizes the window
  // during loading).
  if (d->m_view->contentsHeight() - d->m_view->visibleHeight() >= args.yOffset
      || d->m_bComplete) {
    d->m_view->setContentsPos(args.xOffset, args.yOffset);
    disconnect(d->m_view, TQ_SIGNAL(finishedLayout()), this, TQ_SLOT(restoreScrollPosition()));
  }
}


void TDEHTMLPart::openWallet(DOM::HTMLFormElementImpl *form)
{
#ifndef TDEHTML_NO_WALLET
  TDEHTMLPart *p;

  for (p = parentPart(); p && p->parentPart(); p = p->parentPart()) {
  }

  if (p) {
    p->openWallet(form);
    return;
  }

  if (onlyLocalReferences()) { // avoid triggering on local apps, thumbnails
    return;
  }

  if (d->m_wallet) {
    if (d->m_bWalletOpened) {
      if (d->m_wallet->isOpen()) {
        form->walletOpened(d->m_wallet);
        return;
      }
      d->m_wallet->deleteLater();
      d->m_wallet = 0L;
      d->m_bWalletOpened = false;
    }
  }

  if (!d->m_wq) {
    TDEWallet::Wallet *wallet = TDEWallet::Wallet::openWallet(TDEWallet::Wallet::NetworkWallet(), widget() ? widget()->topLevelWidget()->winId() : 0, TDEWallet::Wallet::Asynchronous);
    d->m_wq = new TDEHTMLWalletQueue(this);
    d->m_wq->wallet = wallet;
    connect(wallet, TQ_SIGNAL(walletOpened(bool)), d->m_wq, TQ_SLOT(walletOpened(bool)));
    connect(d->m_wq, TQ_SIGNAL(walletOpened(TDEWallet::Wallet*)), this, TQ_SLOT(walletOpened(TDEWallet::Wallet*)));
  }
  assert(form);
  d->m_wq->callers.append(TDEHTMLWalletQueue::Caller(form, form->getDocument()));
#endif // TDEHTML_NO_WALLET
}


void TDEHTMLPart::saveToWallet(const TQString& key, const TQMap<TQString,TQString>& data)
{
#ifndef TDEHTML_NO_WALLET
  TDEHTMLPart *p;

  for (p = parentPart(); p && p->parentPart(); p = p->parentPart()) {
  }

  if (p) {
    p->saveToWallet(key, data);
    return;
  }

  if (d->m_wallet) {
    if (d->m_bWalletOpened) {
      if (d->m_wallet->isOpen()) {
        if (!d->m_wallet->hasFolder(TDEWallet::Wallet::FormDataFolder())) {
          d->m_wallet->createFolder(TDEWallet::Wallet::FormDataFolder());
        }
        d->m_wallet->setFolder(TDEWallet::Wallet::FormDataFolder());
        d->m_wallet->writeMap(key, data);
        return;
      }
      d->m_wallet->deleteLater();
      d->m_wallet = 0L;
      d->m_bWalletOpened = false;
    }
  }

  if (!d->m_wq) {
    TDEWallet::Wallet *wallet = TDEWallet::Wallet::openWallet(TDEWallet::Wallet::NetworkWallet(), widget() ? widget()->topLevelWidget()->winId() : 0, TDEWallet::Wallet::Asynchronous);
    d->m_wq = new TDEHTMLWalletQueue(this);
    d->m_wq->wallet = wallet;
    connect(wallet, TQ_SIGNAL(walletOpened(bool)), d->m_wq, TQ_SLOT(walletOpened(bool)));
    connect(d->m_wq, TQ_SIGNAL(walletOpened(TDEWallet::Wallet*)), this, TQ_SLOT(walletOpened(TDEWallet::Wallet*)));
  }
  d->m_wq->savers.append(qMakePair(key, data));
#endif // TDEHTML_NO_WALLET
}


void TDEHTMLPart::dequeueWallet(DOM::HTMLFormElementImpl *form) {
#ifndef TDEHTML_NO_WALLET
  TDEHTMLPart *p;

  for (p = parentPart(); p && p->parentPart(); p = p->parentPart()) {
  }

  if (p) {
    p->dequeueWallet(form);
    return;
  }

  if (d->m_wq) {
    d->m_wq->callers.remove(TDEHTMLWalletQueue::Caller(form, form->getDocument()));
  }
#endif // TDEHTML_NO_WALLET
}


void TDEHTMLPart::walletOpened(TDEWallet::Wallet *wallet) {
#ifndef TDEHTML_NO_WALLET
  assert(!d->m_wallet);
  assert(d->m_wq);

  d->m_wq->deleteLater(); // safe?
  d->m_wq = 0L;

  if (!wallet) {
    d->m_bWalletOpened = false;
    return;
  }

  d->m_wallet = wallet;
  d->m_bWalletOpened = true;
  connect(d->m_wallet, TQ_SIGNAL(walletClosed()), TQ_SLOT(slotWalletClosed()));

  if (!d->m_statusBarWalletLabel) {
    d->m_statusBarWalletLabel = new KURLLabel(d->m_statusBarExtension->statusBar());
    d->m_statusBarWalletLabel->setFixedHeight(instance()->iconLoader()->currentSize(TDEIcon::Small));
    d->m_statusBarWalletLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed, TQSizePolicy::Fixed));
    d->m_statusBarWalletLabel->setUseCursor(false);
    d->m_statusBarExtension->addStatusBarItem(d->m_statusBarWalletLabel, 0, false);
    d->m_statusBarWalletLabel->setPixmap(SmallIcon("wallet_open", instance()));
    connect(d->m_statusBarWalletLabel, TQ_SIGNAL(leftClickedURL()), TQ_SLOT(launchWalletManager()));
    connect(d->m_statusBarWalletLabel, TQ_SIGNAL(rightClickedURL()), TQ_SLOT(walletMenu()));
  } else {
    TQToolTip::remove(d->m_statusBarWalletLabel);
  }
  TQToolTip::add(d->m_statusBarWalletLabel, i18n("The wallet '%1' is open and being used for form data and passwords.").arg(TDEWallet::Wallet::NetworkWallet()));
#endif // TDEHTML_NO_WALLET
}


TDEWallet::Wallet *TDEHTMLPart::wallet()
{
#ifndef TDEHTML_NO_WALLET
  TDEHTMLPart *p;

  for (p = parentPart(); p && p->parentPart(); p = p->parentPart())
    ;

  if (p)
    return p->wallet();

#endif // TDEHTML_NO_WALLET
  return d->m_wallet;
}


void TDEHTMLPart::slotWalletClosed()
{
#ifndef TDEHTML_NO_WALLET
  if (d->m_wallet) {
    d->m_wallet->deleteLater();
    d->m_wallet = 0L;
  }
  d->m_bWalletOpened = false;
  if (d->m_statusBarWalletLabel) {
    d->m_statusBarExtension->removeStatusBarItem(d->m_statusBarWalletLabel);
    delete d->m_statusBarWalletLabel;
    d->m_statusBarWalletLabel = 0L;
  }
#endif // TDEHTML_NO_WALLET
}

void TDEHTMLPart::launchWalletManager()
{
#ifndef TDEHTML_NO_WALLET
  if (!DCOPClient::mainClient()->isApplicationRegistered("tdewalletmanager")) {
    TDEApplication::startServiceByDesktopName("tdewalletmanager_show");
  } else {
    DCOPRef r("tdewalletmanager", "tdewalletmanager-mainwindow#1");
    r.send("show");
    r.send("raise");
  }
#endif // TDEHTML_NO_WALLET
}

void TDEHTMLPart::walletMenu()
{
#ifndef TDEHTML_NO_WALLET
  TDEPopupMenu *m = new TDEPopupMenu(0L);
  m->insertItem(i18n("&Close Wallet"), this, TQ_SLOT(slotWalletClosed()));
  m->popup(TQCursor::pos());
#endif // TDEHTML_NO_WALLET
}

void TDEHTMLPart::slotToggleCaretMode()
{
  setCaretMode(d->m_paToggleCaretMode->isChecked());
}

void TDEHTMLPart::setFormNotification(TDEHTMLPart::FormNotification fn) {
  d->m_formNotification = fn;
}

TDEHTMLPart::FormNotification TDEHTMLPart::formNotification() const {
  return d->m_formNotification;
}

KURL TDEHTMLPart::toplevelURL()
{
  TDEHTMLPart* part = this;
  while (part->parentPart())
    part = part->parentPart();

  if (!part)
    return KURL();

  return part->url();
}

bool TDEHTMLPart::isModified() const
{
  if ( !d->m_doc )
    return false;

  return d->m_doc->unsubmittedFormChanges();
}

void TDEHTMLPart::setDebugScript( bool enable )
{
  unplugActionList( "debugScriptList" );
  if ( enable ) {
    if (!d->m_paDebugScript) {
      d->m_paDebugScript = new TDEAction( i18n( "JavaScript &Debugger" ), 0, this, TQ_SLOT( slotDebugScript() ), actionCollection(), "debugScript" );
    }
    d->m_paDebugScript->setEnabled( d->m_frame ? d->m_frame->m_jscript : 0L );
    TQPtrList<TDEAction> lst;
    lst.append( d->m_paDebugScript );
    plugActionList( "debugScriptList", lst );
  }
  d->m_bJScriptDebugEnabled = enable;
}

void TDEHTMLPart::setSuppressedPopupIndicator( bool enable )
{
    setSuppressedPopupIndicator( enable, 0 );
}

void TDEHTMLPart::setSuppressedPopupIndicator( bool enable, TDEHTMLPart *originPart )
{
    if ( parentPart() ) {
        parentPart()->setSuppressedPopupIndicator( enable, originPart );
        return;
    }

    if ( enable && originPart ) {
        d->m_openableSuppressedPopups++;
        if ( d->m_suppressedPopupOriginParts.findIndex( originPart ) == -1 )
            d->m_suppressedPopupOriginParts.append( originPart );
    }

    if ( enable && !d->m_statusBarPopupLabel ) {
        d->m_statusBarPopupLabel = new KURLLabel( d->m_statusBarExtension->statusBar() );
        d->m_statusBarPopupLabel->setFixedHeight( instance()->iconLoader()->currentSize( TDEIcon::Small) );
        d->m_statusBarPopupLabel->setSizePolicy( TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Fixed ));
        d->m_statusBarPopupLabel->setUseCursor( false );
        d->m_statusBarExtension->addStatusBarItem( d->m_statusBarPopupLabel, 0, false );
        d->m_statusBarPopupLabel->setPixmap( SmallIcon( "window_suppressed", instance() ) );
        TQToolTip::add( d->m_statusBarPopupLabel, i18n("This page was prevented from opening a new window via JavaScript." ) );

        connect(d->m_statusBarPopupLabel, TQ_SIGNAL(leftClickedURL()), TQ_SLOT(suppressedPopupMenu()));
        if (d->m_settings->jsPopupBlockerPassivePopup()) {
            TQPixmap px;
            px = MainBarIcon( "window_suppressed" );
            KPassivePopup::message(i18n("Popup Window Blocked"),i18n("This page has attempted to open a popup window but was blocked.\nYou can click on this icon in the status bar to control this behavior\nor to open the popup."),px,d->m_statusBarPopupLabel);
        }
    } else if ( !enable && d->m_statusBarPopupLabel ) {
        TQToolTip::remove( d->m_statusBarPopupLabel );
        d->m_statusBarExtension->removeStatusBarItem( d->m_statusBarPopupLabel );
        delete d->m_statusBarPopupLabel;
        d->m_statusBarPopupLabel = 0L;
    }
}

void TDEHTMLPart::suppressedPopupMenu() {
  TDEPopupMenu *m = new TDEPopupMenu(0L);
  m->setCheckable(true);
  if ( d->m_openableSuppressedPopups )
      m->insertItem(i18n("&Show Blocked Popup Window","Show %n Blocked Popup Windows", d->m_openableSuppressedPopups), this, TQ_SLOT(showSuppressedPopups()));
  m->insertItem(i18n("Show Blocked Window Passive Popup &Notification"), this, TQ_SLOT(togglePopupPassivePopup()),0,57);
  m->setItemChecked(57,d->m_settings->jsPopupBlockerPassivePopup());
  m->insertItem(i18n("&Configure JavaScript New Window Policies..."), this, TQ_SLOT(launchJSConfigDialog()));
  m->popup(TQCursor::pos());
}

void TDEHTMLPart::togglePopupPassivePopup() {
  // Same hack as in disableJSErrorExtension()
  d->m_settings->setJSPopupBlockerPassivePopup( !d->m_settings->jsPopupBlockerPassivePopup() );
  DCOPClient::mainClient()->send("konqueror*", "KonquerorIface", "reparseConfiguration()", TQByteArray());
}

void TDEHTMLPart::showSuppressedPopups() {
    for ( TQValueListIterator<TQGuardedPtr<TDEHTMLPart> > i = d->m_suppressedPopupOriginParts.begin();
          i != d->m_suppressedPopupOriginParts.end(); ++i ) {
      if (TDEHTMLPart* part = *i) {
        KJS::Window *w = KJS::Window::retrieveWindow( part );
        if (w) {
            w->showSuppressedWindows();
            w->forgetSuppressedWindows();
        }
      }
    }
    setSuppressedPopupIndicator( false );
    d->m_openableSuppressedPopups = 0;
    d->m_suppressedPopupOriginParts.clear();
}

// Extension to use for "view document source", "save as" etc.
// Using the right extension can help the viewer get into the right mode (#40496)
TQString TDEHTMLPart::defaultExtension() const
{
    if ( !d->m_doc )
        return ".html";
    if ( !d->m_doc->isHTMLDocument() )
        return ".xml";
    return d->m_doc->htmlMode() == DOM::DocumentImpl::XHtml ? ".xhtml" : ".html";
}

bool TDEHTMLPart::inProgress() const
{
    if (d->m_runningScripts || (d->m_doc && d->m_doc->parsing()))
        return true;

    // Any frame that hasn't completed yet ?
    ConstFrameIt it = d->m_frames.begin();
    const ConstFrameIt end = d->m_frames.end();
    for (; it != end; ++it ) {
        if ((*it)->m_run || !(*it)->m_bCompleted)
	    return true;
    }

    return d->m_submitForm || !d->m_redirectURL.isEmpty() || d->m_redirectionTimer.isActive() || d->m_job;
}

using namespace KParts;
#include "tdehtml_part.moc"
#include "tdehtmlpart_p.moc"
