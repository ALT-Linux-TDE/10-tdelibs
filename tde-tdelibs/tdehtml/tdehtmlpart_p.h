#ifndef tdehtmlpart_p_h
#define tdehtmlpart_p_h

/* This file is part of the KDE project
 *
 * Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
 *                     1999-2001 Lars Knoll <knoll@kde.org>
 *                     1999-2001 Antti Koivisto <koivisto@kde.org>
 *                     2000-2001 Simon Hausmann <hausmann@kde.org>
 *                     2000-2001 Dirk Mueller <mueller@kde.org>
 *                     2000 Stefan Schimanski <1Stein@gmx.de>
 *                     2001-2003 George Stiakos <staikos@kde.org>
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
#include <kcursor.h>
#include <klibloader.h>
#include <kxmlguifactory.h>
#include <tdeaction.h>
#include <tdeparts/partmanager.h>
#include <tdeparts/statusbarextension.h>
#include <tdeparts/browserextension.h>
#ifndef TDEHTML_NO_WALLET
#include <tdewallet.h>
#endif

#include <tqguardedptr.h>
#include <tqmap.h>
#include <tqtimer.h>
#include <tqvaluelist.h>

#include "html/html_formimpl.h"
#include "tdehtml_run.h"
#include "tdehtml_factory.h"
#include "tdehtml_events.h"
#include "tdehtml_ext.h"
#include "tdehtml_iface.h"
#include "tdehtml_settings.h"
#include "misc/decoder.h"
#include "ecma/kjs_proxy.h"

class KFind;
class KFindDialog;
class TDEPopupMenu;
class TDESelectAction;
class KURLLabel;
class KJavaAppletContext;
class KJSErrorDlg;

namespace TDEIO
{
  class Job;
  class TransferJob;
}
namespace KParts
{
  class StatusBarExtension;
}

namespace tdehtml
{
  class TDE_EXPORT ChildFrame : public TQObject
  {
      TQ_OBJECT
  public:
      enum Type { Frame, IFrame, Object };

      ChildFrame() : TQObject (0L, "tdehtml_child_frame") {
          m_jscript = 0L;
          m_kjs_lib = 0;
          m_bCompleted = false; m_bPreloaded = false; m_type = Frame; m_bNotify = false;
          m_bPendingRedirection = false;
      }

      ~ChildFrame() {
          if (m_run) m_run->abort();
          delete m_jscript;
          if ( m_kjs_lib)
              m_kjs_lib->unload();
      }

    TQGuardedPtr<tdehtml::RenderPart> m_frame;
    TQGuardedPtr<KParts::ReadOnlyPart> m_part;
    TQGuardedPtr<KParts::BrowserExtension> m_extension;
    TQGuardedPtr<KParts::LiveConnectExtension> m_liveconnect;
    TQString m_serviceName;
    TQString m_serviceType;
    KJSProxy *m_jscript;
    KLibrary *m_kjs_lib;
    bool m_bCompleted;
    TQString m_name;
    KParts::URLArgs m_args;
    TQGuardedPtr<TDEHTMLRun> m_run;
    bool m_bPreloaded;
    KURL m_workingURL;
    Type m_type;
    TQStringList m_params;
    bool m_bNotify;
    bool m_bPendingRedirection;
  protected slots:
    void liveConnectEvent(const unsigned long, const TQString&, const KParts::LiveConnectExtension::ArgList&);
  };

}

struct TDEHTMLFrameList : public TQValueList<tdehtml::ChildFrame*>
{
    Iterator find( const TQString &name ) TDE_NO_EXPORT;
};

typedef TDEHTMLFrameList::ConstIterator ConstFrameIt;
typedef TDEHTMLFrameList::Iterator FrameIt;

static int tdehtml_part_dcop_counter = 0;


class TDEHTMLWalletQueue : public TQObject
{
  TQ_OBJECT
  public:
    TDEHTMLWalletQueue(TQObject *parent) : TQObject(parent) {
#ifndef TDEHTML_NO_WALLET
      wallet = 0L;
#endif // TDEHTML_NO_WALLET
    }

    virtual ~TDEHTMLWalletQueue() {
#ifndef TDEHTML_NO_WALLET
      delete wallet;
      wallet = 0L;
#endif // TDEHTML_NO_WALLET
    }
#ifndef TDEHTML_NO_WALLET
    TDEWallet::Wallet *wallet;
#endif // TDEHTML_NO_WALLET
    typedef TQPair<DOM::HTMLFormElementImpl*, TQGuardedPtr<DOM::DocumentImpl> > Caller;
    typedef TQValueList<Caller> CallerList;
    CallerList callers;
    TQValueList<TQPair<TQString, TQMap<TQString, TQString> > > savers;

  signals:
    void walletOpened(TDEWallet::Wallet*);

  public slots:
    void walletOpened(bool success) {
#ifndef TDEHTML_NO_WALLET
      if (!success) {
        delete wallet;
        wallet = 0L;
      }
      emit walletOpened(wallet);
      if (wallet) {
        if (!wallet->hasFolder(TDEWallet::Wallet::FormDataFolder())) {
          wallet->createFolder(TDEWallet::Wallet::FormDataFolder());
        }
        for (CallerList::Iterator i = callers.begin(); i != callers.end(); ++i) {
          if ((*i).first && (*i).second) {
            (*i).first->walletOpened(wallet);
          }
        }
        wallet->setFolder(TDEWallet::Wallet::FormDataFolder());
        for (TQValueList<TQPair<TQString, TQMap<TQString, TQString> > >::Iterator i = savers.begin(); i != savers.end(); ++i) {
          wallet->writeMap((*i).first, (*i).second);
        }
      }
      callers.clear();
      savers.clear();
      wallet = 0L; // gave it away
#endif // TDEHTML_NO_WALLET
    }
};

class TDEHTMLPartPrivate
{
  TDEHTMLPartPrivate(const TDEHTMLPartPrivate & other);
  TDEHTMLPartPrivate& operator=(const TDEHTMLPartPrivate&);
public:
  TDEHTMLPartPrivate(TQObject* parent)
  {
    m_doc = 0L;
    m_decoder = 0L;
    m_wallet = 0L;
    m_bWalletOpened = false;
    m_runningScripts = 0;
    m_job = 0L;
    m_bComplete = true;
    m_bLoadEventEmitted = true;
    m_cachePolicy = TDEIO::CC_Verify;
    m_manager = 0L;
    m_settings = new TDEHTMLSettings(*TDEHTMLFactory::defaultHTMLSettings());
    m_bClearing = false;
    m_bCleared = false;
    m_zoomFactor = 100;
    m_bDnd = true;
    m_startOffset = m_endOffset = 0;
    m_startBeforeEnd = true;
    m_extendAtEnd = true;
    m_linkCursor = KCursor::handCursor();
    m_loadedObjects = 0;
    m_totalObjectCount = 0;
    m_jobPercent = 0;
    m_haveEncoding = false;
    m_activeFrame = 0L;
    m_find = 0;
    m_findDialog = 0;
    m_ssl_in_use = false;
    m_jsedlg = 0;
    m_formNotification = TDEHTMLPart::NoNotification;

#ifndef TQ_WS_QWS
    m_javaContext = 0;
#endif
    m_cacheId = 0;
    m_frameNameId = 1;

    m_restored = false;
    m_restoreScrollPosition = false;

    m_focusNodeNumber = -1;
    m_focusNodeRestored = false;

    m_bJScriptForce = false;
    m_bJScriptOverride = false;
    m_bJavaForce = false;
    m_bJavaOverride = false;
    m_bPluginsForce = false;
    m_bPluginsOverride = false;
    m_onlyLocalReferences = false;

    m_caretMode = false;
    m_designMode = false;

    m_metaRefreshEnabled = true;
    m_statusMessagesEnabled = true;

    m_bFirstData = true;
    m_submitForm = 0;
    m_delayRedirect = 0;
    m_autoDetectLanguage = tdehtml::Decoder::SemiautomaticDetection;

    // inherit settings from parent
    if(parent && parent->inherits("TDEHTMLPart"))
    {
        TDEHTMLPart* part = static_cast<TDEHTMLPart*>(parent);
        if(part->d)
        {
            m_bJScriptForce = part->d->m_bJScriptForce;
            m_bJScriptOverride = part->d->m_bJScriptOverride;
            m_bJavaForce = part->d->m_bJavaForce;
            m_bJavaOverride = part->d->m_bJavaOverride;
            m_bPluginsForce = part->d->m_bPluginsForce;
            m_bPluginsOverride = part->d->m_bPluginsOverride;
            // Same for SSL settings
            m_ssl_in_use = part->d->m_ssl_in_use;
            m_onlyLocalReferences = part->d->m_onlyLocalReferences;
            m_caretMode = part->d->m_caretMode;
            m_designMode = part->d->m_designMode;
            m_zoomFactor = part->d->m_zoomFactor;
            m_autoDetectLanguage = part->d->m_autoDetectLanguage;
            m_encoding = part->d->m_encoding;
            m_haveEncoding = part->d->m_haveEncoding;
        }
    }

    m_focusNodeNumber = -1;
    m_focusNodeRestored = false;
    m_opener = 0;
    m_openedByJS = false;
    m_newJSInterpreterExists = false;
    m_dcopobject = 0;
    m_jobspeed = 0;
    m_dcop_counter = ++tdehtml_part_dcop_counter;
    m_statusBarWalletLabel = 0L;
    m_statusBarUALabel = 0L;
    m_statusBarJSErrorLabel = 0L;
    m_userStyleSheetLastModified = 0;
    m_wq = 0;
  }
  ~TDEHTMLPartPrivate()
  {
    delete m_dcopobject;
    delete m_statusBarExtension;
    delete m_extension;
    delete m_settings;
#ifndef TDEHTML_NO_WALLET
    delete m_wallet;
#endif
#ifndef TQ_WS_QWS
    //delete m_javaContext;
#endif
  }

  TQGuardedPtr<tdehtml::ChildFrame> m_frame;
  TDEHTMLFrameList m_frames;
  TDEHTMLFrameList m_objects;

  TQGuardedPtr<TDEHTMLView> m_view;
  TDEHTMLPartBrowserExtension *m_extension;
  KParts::StatusBarExtension *m_statusBarExtension;
  TDEHTMLPartBrowserHostExtension *m_hostExtension;
  KURLLabel* m_statusBarIconLabel;
  KURLLabel* m_statusBarWalletLabel;
  KURLLabel* m_statusBarUALabel;
  KURLLabel* m_statusBarJSErrorLabel;
  KURLLabel* m_statusBarPopupLabel;
  TQValueList<TQGuardedPtr<TDEHTMLPart> > m_suppressedPopupOriginParts;
  int m_openableSuppressedPopups;
  DOM::DocumentImpl *m_doc;
  tdehtml::Decoder *m_decoder;
  TQString m_encoding;
  TQString m_sheetUsed;
  long m_cacheId;
  TQString scheduledScript;
  DOM::Node scheduledScriptNode;

  TDEWallet::Wallet* m_wallet;
  int m_runningScripts;
  bool m_bOpenMiddleClick :1;
  bool m_bBackRightClick :1;
  bool m_bJScriptEnabled :1;
  bool m_bJScriptDebugEnabled :1;
  bool m_bJavaEnabled :1;
  bool m_bPluginsEnabled :1;
  bool m_bJScriptForce :1;
  bool m_bJScriptOverride :1;
  bool m_bJavaForce :1;
  bool m_bJavaOverride :1;
  bool m_bPluginsForce :1;
  bool m_metaRefreshEnabled :1;
  bool m_bPluginsOverride :1;
  bool m_restored :1;
  bool m_restoreScrollPosition :1;
  bool m_statusMessagesEnabled :1;
  bool m_bWalletOpened :1;
  bool m_urlSelectedOpenedURL:1; // KDE4: remove
  int m_frameNameId;
  int m_dcop_counter;
  DCOPObject *m_dcopobject;

#ifndef TQ_WS_QWS
  KJavaAppletContext *m_javaContext;
#endif

  TDEHTMLSettings *m_settings;

  TDEIO::TransferJob * m_job;

  TQString m_statusBarText[3];
  unsigned long m_jobspeed;
  TQString m_lastModified;
  TQString m_httpHeaders;
  TQString m_pageServices;

  // QStrings for SSL metadata
  // Note: When adding new variables don't forget to update ::saveState()/::restoreState()!
  bool m_ssl_in_use;
  TQString m_ssl_peer_certificate,
          m_ssl_peer_chain,
          m_ssl_peer_ip,
          m_ssl_cipher,
          m_ssl_cipher_desc,
          m_ssl_cipher_version,
          m_ssl_cipher_used_bits,
          m_ssl_cipher_bits,
          m_ssl_cert_state,
          m_ssl_parent_ip,
          m_ssl_parent_cert;

  bool m_bComplete:1;
  bool m_bLoadEventEmitted:1;
  bool m_haveEncoding:1;
  bool m_onlyLocalReferences :1;
  bool m_redirectLockHistory:1;

  KURL m_workingURL;

  TDEIO::CacheControl m_cachePolicy;
  TQTimer m_redirectionTimer;
  TQTime m_parsetime;
  int m_delayRedirect;
  TQString m_redirectURL;

  TDEAction *m_paViewDocument;
  TDEAction *m_paViewFrame;
  TDEAction *m_paViewInfo;
  TDEAction *m_paSaveBackground;
  TDEAction *m_paSaveDocument;
  TDEAction *m_paSaveFrame;
  TDEAction *m_paSecurity;
  TDEActionMenu *m_paSetEncoding;
  TDESelectAction *m_paUseStylesheet;
  TDEHTMLZoomFactorAction *m_paIncZoomFactor;
  TDEHTMLZoomFactorAction *m_paDecZoomFactor;
  TDEAction *m_paLoadImages;
  TDEAction *m_paFind;
  TDEAction *m_paFindNext;
  TDEAction *m_paFindPrev;
  TDEAction *m_paFindAheadText;
  TDEAction *m_paFindAheadLinks;
  TDEAction *m_paPrintFrame;
  TDEAction *m_paSelectAll;
  TDEAction *m_paDebugScript;
  TDEAction *m_paDebugDOMTree;
  TDEAction *m_paDebugRenderTree;
  TDEAction *m_paStopAnimations;
  TDEToggleAction *m_paToggleCaretMode;

  KParts::PartManager *m_manager;

  TQString m_popupMenuXML;
  TDEHTMLPart::GUIProfile m_guiProfile;

  int m_zoomFactor;

  TQString m_strSelectedURL;
  TQString m_strSelectedURLTarget;
  TQString m_referrer;
  TQString m_pageReferrer;

  struct SubmitForm
  {
    const char *submitAction;
    TQString submitUrl;
    TQByteArray submitFormData;
    TQString target;
    TQString submitContentType;
    TQString submitBoundary;
  };

  SubmitForm *m_submitForm;

  bool m_bMousePressed;
  bool m_bRightMousePressed;
  DOM::Node m_mousePressNode; //node under the mouse when the mouse was pressed (set in the mouse handler)

  // simply using the selection limits for the caret position does not suffice
  // as we need to know on which side to extend the selection
//  DOM::Node m_caretNode;	// node containing the caret
//  long m_caretOffset;		// offset within this node (0-based)

  // the caret uses the selection variables for its position. If m_extendAtEnd
  // is true, m_selectionEnd and m_endOffset contain the mandatory caret
  // position, otherwise it's m_selectionStart and m_startOffset.
  DOM::Node m_selectionStart;
  long m_startOffset;
  DOM::Node m_selectionEnd;
  long m_endOffset;
  DOM::Node m_initialNode;	// (Node, Offset) pair on which the
  long m_initialOffset;		// selection has been initiated
  TQString m_overURL;
  TQString m_overURLTarget;

  bool m_startBeforeEnd;
  bool m_extendAtEnd;		// true if selection is to be extended at its end
  enum { ExtendByChar, ExtendByWord, ExtendByLine } m_extendMode;
  bool m_bDnd;
  bool m_bFirstData;
  bool m_bClearing;
  bool m_bCleared;
  bool m_bSecurityInQuestion;
  bool m_focusNodeRestored;

  int m_focusNodeNumber;

  TQPoint m_dragStartPos;
#ifdef TDEHTML_NO_SELECTION
  TQPoint m_dragLastPos;
#endif

  bool m_designMode;
  bool m_caretMode;

  TQCursor m_linkCursor;
  TQTimer m_scrollTimer;

  unsigned long m_loadedObjects;
  unsigned long m_totalObjectCount;
  unsigned int m_jobPercent;

  TDEHTMLPart::FormNotification m_formNotification;
  TQTimer m_progressUpdateTimer;

  TQStringList m_pluginPageQuestionAsked;

  /////////// 'Find' feature
  struct StringPortion
  {
      // Just basic ref/deref on our node to make sure it doesn't get deleted
      StringPortion( int i, DOM::NodeImpl* n ) : index(i), node(n) { if (node) node->ref(); }
      StringPortion() : index(0), node(0) {} // for QValueList
      StringPortion( const StringPortion& other ) : node(0) { operator=(other); }
      StringPortion& operator=( const StringPortion& other ) {
          index=other.index;
          if (other.node) other.node->ref();
          if (node) node->deref();
          node=other.node;
          return *this;
      }
      ~StringPortion() { if (node) node->deref(); }

      int index;
      DOM::NodeImpl *node;
  };
  TQValueList<StringPortion> m_stringPortions;

  KFind *m_find;
  KFindDialog *m_findDialog;

  struct findState
  {
    findState() : options( 0 ), last_dir( -1 ) {}
    TQStringList history;
    TQString text;
    int options;
    int last_dir; // -1=unknown,0=forward,1=backward
  };

  findState m_lastFindState;

  KJSErrorDlg *m_jsedlg;

  DOM::NodeImpl *m_findNode; // current node
  DOM::NodeImpl *m_findNodeEnd; // end node
  DOM::NodeImpl *m_findNodeStart; // start node
  DOM::NodeImpl *m_findNodePrevious; // previous node used for find
  int m_findPos; // current pos in current node
  int m_findPosEnd; // pos in end node
  int m_findPosStart; // pos in start node
  /////////

  //TQGuardedPtr<KParts::Part> m_activeFrame;
  KParts::Part * m_activeFrame;
  TQGuardedPtr<TDEHTMLPart> m_opener;
  bool m_openedByJS;
  bool m_newJSInterpreterExists; // set to 1 by setOpenedByJS, for window.open

  tdehtml::Decoder::AutoDetectLanguage m_autoDetectLanguage;
  TDEPopupMenu *m_automaticDetection;
  TDESelectAction *m_manualDetection;

  void setFlagRecursively(bool TDEHTMLPartPrivate::*flag, bool value);
  /** returns the caret node */
  DOM::Node &caretNode() {
    return m_extendAtEnd ? m_selectionEnd : m_selectionStart;
  }
  /** returns the caret offset */
  long &caretOffset() {
    return m_extendAtEnd ? m_endOffset : m_startOffset;
  }

  time_t m_userStyleSheetLastModified;

  TDEHTMLWalletQueue *m_wq;
};

#endif
