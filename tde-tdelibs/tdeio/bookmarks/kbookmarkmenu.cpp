/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#include "kbookmarkmenu.h"
#include "kbookmarkmenu_p.h"
#include "kbookmarkimporter.h"
#include "kbookmarkimporter_opera.h"
#include "kbookmarkimporter_ie.h"
#include "kbookmarkdrag.h"

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tdepopupmenu.h>
#include <tdestdaccel.h>
#include <kstdaction.h>
#include <kstringhandler.h>

#include <tqclipboard.h>
#include <tqfile.h>
#include <tqheader.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqlistview.h>
#include <tqpushbutton.h>

#include <dptrtemplate.h>

template class TQPtrList<KBookmarkMenu>;

static TQString makeTextNodeMod(KBookmark bk, const TQString &m_nodename, const TQString &m_newText) {
  TQDomNode subnode = bk.internalElement().namedItem(m_nodename);
  if (subnode.isNull()) {
    subnode = bk.internalElement().ownerDocument().createElement(m_nodename);
    bk.internalElement().appendChild(subnode);
  }

  if (subnode.firstChild().isNull()) {
    TQDomText domtext = subnode.ownerDocument().createTextNode("");
    subnode.appendChild(domtext);
  }

  TQDomText domtext = subnode.firstChild().toText();

  TQString m_oldText = domtext.data();
  domtext.setData(m_newText);

  return m_oldText;
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkMenu::KBookmarkMenu( KBookmarkManager* mgr,
                              KBookmarkOwner * _owner, TDEPopupMenu * _parentMenu,
                              TDEActionCollection *collec, bool _isRoot, bool _add,
                              const TQString & parentAddress )
  : TQObject(),
    m_bIsRoot(_isRoot), m_bAddBookmark(_add),
    m_bAddShortcuts(true),
    m_pManager(mgr), m_pOwner(_owner),
    m_parentMenu( _parentMenu ),
    m_actionCollection( collec ),
    m_parentAddress( parentAddress )
{
  m_parentMenu->setKeyboardShortcutsEnabled( true );

  m_lstSubMenus.setAutoDelete( true );
  m_actions.setAutoDelete( true );

  if (m_actionCollection)
  {
    m_actionCollection->setHighlightingEnabled(true);
    disconnect( m_actionCollection, TQ_SIGNAL( actionHighlighted( TDEAction * ) ), 0, 0 );
    connect( m_actionCollection, TQ_SIGNAL( actionHighlighted( TDEAction * ) ),
             this, TQ_SLOT( slotActionHighlighted( TDEAction * ) ) );
  }

  m_bNSBookmark = m_parentAddress.isNull();
  if ( !m_bNSBookmark ) // not for the netscape bookmark
  {
    //kdDebug(7043) << "KBookmarkMenu::KBookmarkMenu " << this << " address : " << m_parentAddress << endl;

    connect( _parentMenu, TQ_SIGNAL( aboutToShow() ),
             TQ_SLOT( slotAboutToShow() ) );

    if ( KBookmarkSettings::self()->m_contextmenu )
    {
      (void) _parentMenu->contextMenu();
      connect( _parentMenu, TQ_SIGNAL( aboutToShowContextMenu(TDEPopupMenu*, int, TQPopupMenu*) ),
               this, TQ_SLOT( slotAboutToShowContextMenu(TDEPopupMenu*, int, TQPopupMenu*) ));
    }

    if ( m_bIsRoot )
    {
      connect( m_pManager, TQ_SIGNAL( changed(const TQString &, const TQString &) ),
               TQ_SLOT( slotBookmarksChanged(const TQString &) ) );
    }
  }

  // add entries that possibly have a shortcut, so they are available _before_ first popup
  if ( m_bIsRoot )
  {
    if ( m_bAddBookmark )
    {
      addAddBookmark();
      if ( extOwner() )
        addAddBookmarksList(); // FIXME
    }

    addEditBookmarks();
  }

  m_bDirty = true;
}

KBookmarkMenu::~KBookmarkMenu()
{
  //kdDebug(7043) << "KBookmarkMenu::~KBookmarkMenu() " << this << endl;
  TQPtrListIterator<TDEAction> it( m_actions );
  for (; it.current(); ++it )
    it.current()->unplugAll();

  m_lstSubMenus.clear();
  m_actions.clear();
}

void KBookmarkMenu::ensureUpToDate()
{
  slotAboutToShow();
}

void KBookmarkMenu::slotAboutToShow()
{
  // Did the bookmarks change since the last time we showed them ?
  if ( m_bDirty )
  {
    m_bDirty = false;
    refill();
  }
}

TQString KBookmarkMenu::s_highlightedAddress;
TQString KBookmarkMenu::s_highlightedImportType;
TQString KBookmarkMenu::s_highlightedImportLocation;

void KBookmarkMenu::slotActionHighlighted( TDEAction* action )
{
  if (action->isA("KBookmarkActionMenu") || action->isA("KBookmarkAction"))
  {
    s_highlightedAddress = action->property("address").toString();
    //kdDebug() << "KBookmarkMenu::slotActionHighlighted" << s_highlightedAddress << endl;
  }
  else if (action->isA("KImportedBookmarksActionMenu"))
  {
    s_highlightedImportType = action->property("type").toString();
    s_highlightedImportLocation = action->property("location").toString();
  }
  else
  {
    s_highlightedAddress = TQString::null;
    s_highlightedImportType = TQString::null;
    s_highlightedImportLocation = TQString::null;
  }
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

class KBookmarkMenuRMBAssoc : public dPtrTemplate<KBookmarkMenu, RMB> { };
template<> TQPtrDict<RMB>* dPtrTemplate<KBookmarkMenu, RMB>::d_ptr = 0;

static RMB* rmbSelf(KBookmarkMenu *m) { return KBookmarkMenuRMBAssoc::d(m); }

// TODO check via dcop before making any changes to the bookmarks file???

void RMB::begin_rmb_action(KBookmarkMenu *self)
{
  RMB *s = rmbSelf(self);
  s->recv = self;
  s->m_parentAddress = self->m_parentAddress;
  s->s_highlightedAddress = KBookmarkMenu::s_highlightedAddress;
  s->m_pManager = self->m_pManager;
  s->m_pOwner = self->m_pOwner;
  s->m_parentMenu = self->m_parentMenu;
}

bool RMB::invalid( int val )
{
  bool valid = true;

  if (val == 1)
    s_highlightedAddress = m_parentAddress;

  if (s_highlightedAddress.isNull())
    valid = false;

  return !valid;
}

KBookmark RMB::atAddress(const TQString & address)
{
  KBookmark bookmark = m_pManager->findByAddress( address );
  Q_ASSERT(!bookmark.isNull());
  return bookmark;
}

void KBookmarkMenu::slotAboutToShowContextMenu( TDEPopupMenu*, int, TQPopupMenu* contextMenu )
{
  //kdDebug(7043) << "KBookmarkMenu::slotAboutToShowContextMenu" << s_highlightedAddress << endl;
  if (s_highlightedAddress.isNull())
  {
    TDEPopupMenu::contextMenuFocus()->hideContextMenu();
    return;
  }
  contextMenu->clear();
  fillContextMenu( contextMenu, s_highlightedAddress, 0 );
}

void RMB::fillContextMenu( TQPopupMenu* contextMenu, const TQString & address, int val )
{
  KBookmark bookmark = atAddress(address);

  int id;

  // binner:
  // "Add Bookmark Here" when pointing at a bookmark looks strange and if you
  // call it you have to close and reopen the menu to see an entry was added?
  //
  // TODO rename these, but, message freeze... umm...

//  if (bookmark.isGroup()) {
    id = contextMenu->insertItem( SmallIcon("bookmark_add"), i18n( "Add Bookmark Here" ), recv, TQ_SLOT(slotRMBActionInsert(int)) );
    contextMenu->setItemParameter( id, val );
/*  }
  else
  {
    id = contextMenu->insertItem( SmallIcon("bookmark_add"), i18n( "Add Bookmark Here" ), recv, TQ_SLOT(slotRMBActionInsert(int)) );
    contextMenu->setItemParameter( id, val );
  }*/
}

void RMB::fillContextMenu2( TQPopupMenu* contextMenu, const TQString & address, int val )
{
  KBookmark bookmark = atAddress(address);

  int id;

  if (bookmark.isGroup()) {
    id = contextMenu->insertItem( i18n( "Open Folder in Bookmark Editor" ), recv, TQ_SLOT(slotRMBActionEditAt(int)) );
    contextMenu->setItemParameter( id, val );
    contextMenu->insertSeparator();
    id = contextMenu->insertItem( SmallIcon("edit-delete"), i18n( "Delete Folder" ), recv, TQ_SLOT(slotRMBActionRemove(int)) );
    contextMenu->setItemParameter( id, val );
    contextMenu->insertSeparator();
    id = contextMenu->insertItem( i18n( "Properties" ), recv, TQ_SLOT(slotRMBActionProperties(int)) );
    contextMenu->setItemParameter( id, val );
  }
  else
  {
    id = contextMenu->insertItem( i18n( "Copy Link Address" ), recv, TQ_SLOT(slotRMBActionCopyLocation(int)) );
    contextMenu->setItemParameter( id, val );
    contextMenu->insertSeparator();
    id = contextMenu->insertItem( SmallIcon("edit-delete"), i18n( "Delete Bookmark" ), recv, TQ_SLOT(slotRMBActionRemove(int)) );
    contextMenu->setItemParameter( id, val );
    contextMenu->insertSeparator();
    id = contextMenu->insertItem( i18n( "Properties" ), recv, TQ_SLOT(slotRMBActionProperties(int)) );
    contextMenu->setItemParameter( id, val );
  }
}

void RMB::slotRMBActionEditAt( int val )
{
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionEditAt" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  KBookmark bookmark = atAddress(s_highlightedAddress);

  m_pManager->slotEditBookmarksAtAddress( s_highlightedAddress );
}

void RMB::slotRMBActionProperties( int val )
{
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionProperties" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  KBookmark bookmark = atAddress(s_highlightedAddress);

  TQString folder = bookmark.isGroup() ? TQString::null : bookmark.url().pathOrURL();
  KBookmarkEditDialog dlg( bookmark.fullText(), folder,
                           m_pManager, KBookmarkEditDialog::ModifyMode, 0,
                           0, 0, i18n("Bookmark Properties") );
  if ( dlg.exec() != KDialogBase::Accepted )
    return;

  makeTextNodeMod(bookmark, "title", dlg.finalTitle());
  if ( !dlg.finalUrl().isNull() )
  {
    KURL u = KURL::fromPathOrURL(dlg.finalUrl());
    bookmark.internalElement().setAttribute("href", u.url(0, 106));
  }

  kdDebug(7043) << "Requested move to " << dlg.finalAddress() << "!" << endl;

  KBookmarkGroup parentBookmark = atAddress(m_parentAddress).toGroup();
  m_pManager->emitChanged( parentBookmark );
}

void RMB::slotRMBActionInsert( int val )
{
  kdDebug(7043) << "KBookmarkMenu::slotRMBActionInsert" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  TQString url = m_pOwner->currentURL();
  if (url.isEmpty())
  {
    KMessageBox::error( 0L, i18n("Cannot add bookmark with empty URL."));
    return;
  }
  TQString title = m_pOwner->currentTitle();
  if (title.isEmpty())
    title = url;

  KBookmark bookmark = atAddress( s_highlightedAddress );

  // TODO use unique title

  if (bookmark.isGroup())
  {
    KBookmarkGroup parentBookmark = bookmark.toGroup();
    Q_ASSERT(!parentBookmark.isNull());
    parentBookmark.addBookmark( m_pManager, title, KURL(  url ) );
    m_pManager->emitChanged( parentBookmark );
  }
  else
  {
    KBookmarkGroup parentBookmark = bookmark.parentGroup();
    Q_ASSERT(!parentBookmark.isNull());
    KBookmark newBookmark = parentBookmark.addBookmark( m_pManager, title, KURL( url ) );
    parentBookmark.moveItem( newBookmark, parentBookmark.previous(bookmark) );
    m_pManager->emitChanged( parentBookmark );
  }
}

void RMB::slotRMBActionRemove( int val )
{
  //kdDebug(7043) << "KBookmarkMenu::slotRMBActionRemove" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  KBookmark bookmark = atAddress( s_highlightedAddress );
  bool folder = bookmark.isGroup();

  if (KMessageBox::warningContinueCancel(
          m_parentMenu,
          folder ? i18n("Are you sure you wish to remove the bookmark folder\n\"%1\"?").arg(bookmark.text())
                 : i18n("Are you sure you wish to remove the bookmark\n\"%1\"?").arg(bookmark.text()),
          folder ? i18n("Bookmark Folder Deletion")
                 : i18n("Bookmark Deletion"),
          KStdGuiItem::del())
        != KMessageBox::Continue
     )
    return;

  KBookmarkGroup parentBookmark = atAddress( m_parentAddress ).toGroup();
  parentBookmark.deleteBookmark( bookmark );
  m_pManager->emitChanged( parentBookmark );
  if (m_parentMenu)
    m_parentMenu->hide();
}

void RMB::slotRMBActionCopyLocation( int val )
{
  //kdDebug(7043) << "KBookmarkMenu::slotRMBActionCopyLocation" << s_highlightedAddress << endl;
  if (invalid(val)) { hidePopup(); return; }

  KBookmark bookmark = atAddress( s_highlightedAddress );

  if ( !bookmark.isGroup() )
  {
    kapp->clipboard()->setData( KBookmarkDrag::newDrag(bookmark, 0),
                                TQClipboard::Selection );
    kapp->clipboard()->setData( KBookmarkDrag::newDrag(bookmark, 0),
                                TQClipboard::Clipboard );
  }
}

void RMB::hidePopup() {
  TDEPopupMenu::contextMenuFocus()->hideContextMenu();
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

void KBookmarkMenu::fillContextMenu( TQPopupMenu* contextMenu, const TQString & address, int val )
{
  RMB::begin_rmb_action(this);
  rmbSelf(this)->fillContextMenu(contextMenu, address, val);
  emit aboutToShowContextMenu( rmbSelf(this)->atAddress(address), contextMenu);
  rmbSelf(this)->fillContextMenu2(contextMenu, address, val);
}

void KBookmarkMenu::slotRMBActionEditAt( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionEditAt( val ); }

void KBookmarkMenu::slotRMBActionProperties( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionProperties( val ); }

void KBookmarkMenu::slotRMBActionInsert( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionInsert( val ); }

void KBookmarkMenu::slotRMBActionRemove( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionRemove( val ); }

void KBookmarkMenu::slotRMBActionCopyLocation( int val )
{ RMB::begin_rmb_action(this); rmbSelf(this)->slotRMBActionCopyLocation( val ); }

void KBookmarkMenu::slotBookmarksChanged( const TQString & groupAddress )
{
  if (m_bNSBookmark)
    return;

  if ( groupAddress == m_parentAddress )
  {
    //kdDebug(7043) << "KBookmarkMenu::slotBookmarksChanged -> setting m_bDirty on " << groupAddress << endl;
    m_bDirty = true;
  }
  else
  {
    // Iterate recursively into child menus
    TQPtrListIterator<KBookmarkMenu> it( m_lstSubMenus );
    for (; it.current(); ++it )
    {
      it.current()->slotBookmarksChanged( groupAddress );
    }
  }
}

void KBookmarkMenu::refill()
{
  //kdDebug(7043) << "KBookmarkMenu::refill()" << endl;
  m_lstSubMenus.clear();

  TQPtrListIterator<TDEAction> it( m_actions );
  for (; it.current(); ++it )
    it.current()->unplug( m_parentMenu );

  m_parentMenu->clear();
  m_actions.clear();

  fillBookmarkMenu();
  m_parentMenu->adjustSize();
}

void KBookmarkMenu::addAddBookmarksList()
{
  if (!kapp->authorizeTDEAction("bookmarks"))
     return;

  TQString title = i18n( "Bookmark Tabs as Folder..." );

  TDEAction * paAddBookmarksList = new TDEAction( title,
                                          "bookmarks_list_add",
                                          0,
                                          this,
                                          TQ_SLOT( slotAddBookmarksList() ),
                                          m_actionCollection, m_bIsRoot ? "add_bookmarks_list" : 0 );

  paAddBookmarksList->setToolTip( i18n( "Add a folder of bookmarks for all open tabs." ) );

  paAddBookmarksList->plug( m_parentMenu );
  m_actions.append( paAddBookmarksList );
}

void KBookmarkMenu::addAddBookmark()
{
  if (!kapp->authorizeTDEAction("bookmarks"))
     return;

  TQString title = i18n( "Add Bookmark" );

  TDEAction * paAddBookmarks = new TDEAction( title,
                                          "bookmark_add",
                                          m_bIsRoot && m_bAddShortcuts ? TDEStdAccel::addBookmark() : TDEShortcut(),
                                          this,
                                          TQ_SLOT( slotAddBookmark() ),
                                          m_actionCollection, m_bIsRoot ? "add_bookmark" : 0 );

  paAddBookmarks->setToolTip( i18n( "Add a bookmark for the current document" ) );

  paAddBookmarks->plug( m_parentMenu );
  m_actions.append( paAddBookmarks );
}

void KBookmarkMenu::addEditBookmarks()
{
  if (!kapp->authorizeTDEAction("bookmarks"))
     return;

  TDEAction * m_paEditBookmarks = KStdAction::editBookmarks( m_pManager, TQ_SLOT( slotEditBookmarks() ),
                                                             m_actionCollection, "edit_bookmarks" );
  m_paEditBookmarks->plug( m_parentMenu );
  m_paEditBookmarks->setToolTip( i18n( "Edit your bookmark collection in a separate window" ) );
  m_actions.append( m_paEditBookmarks );
}

void KBookmarkMenu::addNewFolder()
{
  if (!kapp->authorizeTDEAction("bookmarks"))
     return;

  TQString title = i18n( "&New Bookmark Folder..." );
  int p;
  while ( ( p = title.find( '&' ) ) >= 0 )
    title.remove( p, 1 );

  TDEAction * paNewFolder = new TDEAction( title,
                                       "folder-new", //"folder",
                                       0,
                                       this,
                                       TQ_SLOT( slotNewFolder() ),
                                       m_actionCollection );

  paNewFolder->setToolTip( i18n( "Create a new bookmark folder in this menu" ) );

  paNewFolder->plug( m_parentMenu );
  m_actions.append( paNewFolder );
}

void KBookmarkMenu::fillBookmarkMenu()
{
  if (!kapp->authorizeTDEAction("bookmarks"))
     return;

  if ( m_bIsRoot )
  {
    if ( m_bAddBookmark )
    {
      addAddBookmark();
      if ( extOwner() )
        addAddBookmarksList(); // FIXME
    }

    addEditBookmarks();

    if ( m_bAddBookmark && !KBookmarkSettings::self()->m_advancedaddbookmark )
      addNewFolder();
  }

  if ( m_bIsRoot
    && KBookmarkManager::userBookmarksFile() == m_pManager->path() )
  {
    bool haveSep = false;

    TQValueList<TQString> keys = KBookmarkMenu::dynamicBookmarksList();
    TQValueList<TQString>::const_iterator it;
    for ( it = keys.begin(); it != keys.end(); ++it )
    {
       DynMenuInfo info;
       info = showDynamicBookmarks((*it));

       if ( !info.show || !TQFile::exists( info.location ) )
          continue;

       if (!haveSep)
       {
          m_parentMenu->insertSeparator();
          haveSep = true;
       }

       TDEActionMenu * actionMenu;
       actionMenu = new KImportedBookmarksActionMenu(
                              info.name, info.type,
                              m_actionCollection, "kbookmarkmenu" );

       actionMenu->setProperty( "type", info.type );
       actionMenu->setProperty( "location", info.location );

       actionMenu->plug( m_parentMenu );
       m_actions.append( actionMenu );

       KBookmarkMenu *subMenu =
          new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->popupMenu(),
                             m_actionCollection, false,
                             m_bAddBookmark, TQString::null );
       connect( subMenu, TQ_SIGNAL( openBookmark( const TQString &, TQt::ButtonState ) ),
                this, TQ_SIGNAL( openBookmark( const TQString &, TQt::ButtonState ) ));
       m_lstSubMenus.append(subMenu);

       connect(actionMenu->popupMenu(), TQ_SIGNAL(aboutToShow()), subMenu, TQ_SLOT(slotNSLoad()));
    }
  }

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  bool separatorInserted = false;
  for ( KBookmark bm = parentBookmark.first(); !bm.isNull();  bm = parentBookmark.next(bm) )
  {
    TQString text = KStringHandler::csqueeze(bm.fullText(), 60);
    text.replace( '&', "&&" );
    if ( !separatorInserted && m_bIsRoot) {
      // inserted before the first konq bookmark, to avoid the separator if no konq bookmark
      m_parentMenu->insertSeparator();
      separatorInserted = true;
    }
    if ( !bm.isGroup() )
    {
      if ( bm.isSeparator() )
      {
        m_parentMenu->insertSeparator();
      }
      else
      {
        //kdDebug(7043) << "Creating URL bookmark menu item for " << bm.text() << endl;
        TDEAction * action = new KBookmarkAction( text, bm.icon(), 0, m_actionCollection, 0 );
        connect(action, TQ_SIGNAL( activated ( TDEAction::ActivationReason, TQt::ButtonState )),
                this, TQ_SLOT( slotBookmarkSelected( TDEAction::ActivationReason, TQt::ButtonState ) ));

        action->setProperty( "url", bm.url().url() );
        action->setProperty( "address", bm.address() );

        action->setToolTip( bm.url().pathOrURL() );

        action->plug( m_parentMenu );
        m_actions.append( action );
      }
    }
    else
    {
      //kdDebug(7043) << "Creating bookmark submenu named " << bm.text() << endl;
      TDEActionMenu * actionMenu = new KBookmarkActionMenu( text, bm.icon(),
                                                          m_actionCollection,
                                                          "kbookmarkmenu" );
      actionMenu->setProperty( "address", bm.address() );
      actionMenu->plug( m_parentMenu );
      m_actions.append( actionMenu );

      KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_pOwner, actionMenu->popupMenu(),
                                                  m_actionCollection, false,
                                                  m_bAddBookmark,
                                                  bm.address() );

      connect(subMenu, TQ_SIGNAL( aboutToShowContextMenu( const KBookmark &, TQPopupMenu * ) ),
                 this, TQ_SIGNAL( aboutToShowContextMenu( const KBookmark &, TQPopupMenu * ) ));
      connect(subMenu, TQ_SIGNAL( openBookmark( const TQString &, TQt::ButtonState ) ),
                this, TQ_SIGNAL( openBookmark( const TQString &, TQt::ButtonState ) ));
      m_lstSubMenus.append( subMenu );
    }
  }

  if ( !m_bIsRoot && m_bAddBookmark )
  {
    if ( m_parentMenu->count() > 0 )
      m_parentMenu->insertSeparator();

    if ( KBookmarkSettings::self()->m_quickactions )
    {
      TDEActionMenu * actionMenu = new TDEActionMenu( i18n("Quick Actions"), m_actionCollection, 0L );
      fillContextMenu( actionMenu->popupMenu(), m_parentAddress, 1 );
      actionMenu->plug( m_parentMenu );
      m_actions.append( actionMenu );
    }
    else
    {
      addAddBookmark();
      if ( extOwner() )
        addAddBookmarksList(); // FIXME
      addNewFolder();
    }
  }
}

void KBookmarkMenu::slotAddBookmarksList()
{
  KExtendedBookmarkOwner *extOwner = dynamic_cast<KExtendedBookmarkOwner*>(m_pOwner);
  if (!extOwner)
  {
    kdWarning() << "erm, sorry ;-)" << endl;
    return;
  }

  KExtendedBookmarkOwner::QStringPairList list;
  extOwner->fillBookmarksList( list );

  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  KBookmarkGroup group = parentBookmark.createNewFolder( m_pManager );
  if ( group.isNull() )
    return; // user canceled i guess

  KExtendedBookmarkOwner::QStringPairList::const_iterator it;
  for ( it = list.begin(); it != list.end(); ++it )
    group.addBookmark( m_pManager, (*it).first, KURL((*it).second) );

  m_pManager->emitChanged( parentBookmark );
}


void KBookmarkMenu::slotAddBookmark()
{
  KBookmarkGroup parentBookmark;
  parentBookmark = m_pManager->addBookmarkDialog(m_pOwner->currentURL(), m_pOwner->currentTitle(), m_parentAddress);
  if (!parentBookmark.isNull())
    m_pManager->emitChanged( parentBookmark );
}

void KBookmarkMenu::slotNewFolder()
{
  if ( !m_pOwner ) return; // this view doesn't handle bookmarks...
  KBookmarkGroup parentBookmark = m_pManager->findByAddress( m_parentAddress ).toGroup();
  Q_ASSERT(!parentBookmark.isNull());
  KBookmarkGroup group = parentBookmark.createNewFolder( m_pManager );
  if ( !group.isNull() )
  {
    KBookmarkGroup parentGroup = group.parentGroup();
    m_pManager->emitChanged( parentGroup );
  }
}

void KBookmarkMenu::slotBookmarkSelected( TDEAction::ActivationReason /*reason*/, TQt::ButtonState state )
{
  kdDebug(7043) << "KBookmarkMenu::slotBookmarkSelected()" << endl;
  if ( !m_pOwner ) return; // this view doesn't handle bookmarks...
  const TDEAction* action =  dynamic_cast<const TDEAction *>(sender());
  if(action)
  {
      const TQString& url = sender()->property("url").toString();
      m_pOwner->openBookmarkURL( url );
      emit openBookmark( url, state );
  }
}

void KBookmarkMenu::slotBookmarkSelected()
{
    slotBookmarkSelected(TDEAction::PopupMenuActivation, TQt::NoButton);
}

KExtendedBookmarkOwner* KBookmarkMenu::extOwner()
{
  return dynamic_cast<KExtendedBookmarkOwner*>(m_pOwner);
}

void KBookmarkMenu::slotNSLoad()
{
  // only fill menu once
  m_parentMenu->disconnect(TQ_SIGNAL(aboutToShow()));

  // not NSImporter, but kept old name for BC reasons
  KBookmarkMenuNSImporter importer( m_pManager, this, m_actionCollection );
  importer.openBookmarks(s_highlightedImportLocation, s_highlightedImportType);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkEditFields::KBookmarkEditFields(TQWidget *main, TQBoxLayout *vbox, FieldsSet fieldsSet)
{
  bool isF = (fieldsSet != FolderFieldsSet);

  TQGridLayout *grid = new TQGridLayout( vbox, 2, isF ? 2 : 1 );

  m_title = new KLineEdit( main );
  grid->addWidget( m_title, 0, 1 );
  grid->addWidget( new TQLabel( m_title, i18n( "Name:" ), main ), 0, 0 );
  m_title->setFocus();
  if (isF)
  {
    m_url = new KLineEdit( main );
    grid->addWidget( m_url, 1, 1 );
    grid->addWidget( new TQLabel( m_url, i18n( "Location:" ), main ), 1, 0 );
  }
  else
  {
    m_url = 0;
  }

  main->setMinimumSize( 300, 0 );
}

void KBookmarkEditFields::setName(const TQString &str)
{
  m_title->setText(str);
}

void KBookmarkEditFields::setLocation(const TQString &str)
{
  m_url->setText(str);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

// TODO - make the dialog use Properties as a title when in Modify mode... (dirk noticed the bug...)
KBookmarkEditDialog::KBookmarkEditDialog(const TQString& title, const TQString& url, KBookmarkManager * mgr, BookmarkEditType editType, const TQString& address,
                                         TQWidget * parent, const char * name, const TQString& caption )
  : KDialogBase(parent, name, true, caption,
                (editType == InsertionMode) ? (User1|Ok|Cancel) : (Ok|Cancel),
                Ok, false, KGuiItem()),
    m_folderTree(0), m_mgr(mgr), m_editType(editType), m_address(address)
{
  setButtonOK( (editType == InsertionMode) ? KGuiItem( i18n( "&Add" ), "bookmark_add") : i18n( "&Update" ) );
  if (editType == InsertionMode) {
    setButtonGuiItem( User1, KGuiItem( i18n( "&New Folder..." ), "folder-new") );
  }

  bool folder = url.isNull();

  m_main = new TQWidget( this );
  setMainWidget( m_main );

  TQBoxLayout *vbox = new TQVBoxLayout( m_main, 0, spacingHint() );
  KBookmarkEditFields::FieldsSet fs =
    folder ? KBookmarkEditFields::FolderFieldsSet
           : KBookmarkEditFields::BookmarkFieldsSet;
  m_fields = new KBookmarkEditFields(m_main, vbox, fs);
  m_fields->setName(title);
  if ( !folder )
    m_fields->setLocation(url);

  if ( editType == InsertionMode )
  {
    m_folderTree = KBookmarkFolderTree::createTree( m_mgr, m_main, name, m_address );
    connect( m_folderTree, TQ_SIGNAL( doubleClicked(TQListViewItem*) ),
             this,         TQ_SLOT( slotDoubleClicked(TQListViewItem*) ) );
    vbox->addWidget( m_folderTree );
    connect( this, TQ_SIGNAL( user1Clicked() ), TQ_SLOT( slotUser1() ) );
  }
}

void KBookmarkEditDialog::slotDoubleClicked( TQListViewItem* item )
{
  Q_ASSERT( m_folderTree );
  m_folderTree->setCurrentItem( item );
  accept();
}

void KBookmarkEditDialog::slotOk()
{
  accept();
}

void KBookmarkEditDialog::slotCancel()
{
  reject();
}

TQString KBookmarkEditDialog::finalAddress() const
{
  Q_ASSERT( m_folderTree );
  return KBookmarkFolderTree::selectedAddress( m_folderTree );
}

TQString KBookmarkEditDialog::finalUrl() const
{
  return m_fields->m_url ? m_fields->m_url->text() : TQString::null;
}

TQString KBookmarkEditDialog::finalTitle() const
{
  return m_fields->m_title ? m_fields->m_title->text() : TQString::null;
}

void KBookmarkEditDialog::slotUser1()
{
  // kdDebug(7043) << "KBookmarkEditDialog::slotUser1" << endl;
  Q_ASSERT( m_folderTree );

  TQString address = KBookmarkFolderTree::selectedAddress( m_folderTree );
  if ( address.isNull() ) return;
  KBookmarkGroup bm = m_mgr->findByAddress( address ).toGroup();
  Q_ASSERT(!bm.isNull());
  Q_ASSERT(m_editType == InsertionMode);

  KBookmarkGroup group = bm.createNewFolder( m_mgr );
  if ( !group.isNull() )
  {
    KBookmarkGroup parentGroup = group.parentGroup();
    m_mgr->emitChanged( parentGroup );
  }
  KBookmarkFolderTree::fillTree( m_folderTree, m_mgr );
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

static void fillGroup( TQListView* listview, KBookmarkFolderTreeItem * parentItem, KBookmarkGroup group, bool expandOpenGroups = true, const TQString& address = TQString::null )
{
  bool noSubGroups = true;
  KBookmarkFolderTreeItem * lastItem = 0L;
  KBookmarkFolderTreeItem * item = 0L;
  for ( KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk) )
  {
    if ( bk.isGroup() )
    {
      KBookmarkGroup grp = bk.toGroup();
      item = new KBookmarkFolderTreeItem( parentItem, lastItem, grp );
      fillGroup( listview, item, grp, expandOpenGroups, address );
      if ( expandOpenGroups && grp.isOpen() )
        item->setOpen( true );
      lastItem = item;
      noSubGroups = false;
    }
    if (bk.address() == address) {
      listview->setCurrentItem( lastItem );
      listview->ensureItemVisible( item );
    }
  }
  if ( noSubGroups ) {
     parentItem->setOpen( true );
  }
}

TQListView* KBookmarkFolderTree::createTree( KBookmarkManager* mgr, TQWidget* parent, const char* name, const TQString& address )
{
  TQListView *listview = new TQListView( parent, name );

  listview->setRootIsDecorated( false );
  listview->header()->hide();
  listview->addColumn( i18n("Bookmark"), 200 );
  listview->setSorting( -1, false );
  listview->setSelectionMode( TQListView::Single );
  listview->setAllColumnsShowFocus( true );
  listview->setResizeMode( TQListView::AllColumns );
  listview->setMinimumSize( 60, 100 );

  fillTree( listview, mgr, address );

  return listview;
}

void KBookmarkFolderTree::fillTree( TQListView *listview, KBookmarkManager* mgr, const TQString& address )
{
  listview->clear();

  KBookmarkGroup root = mgr->root();
  KBookmarkFolderTreeItem * rootItem = new KBookmarkFolderTreeItem( listview, root );
  listview->setCurrentItem( rootItem );
  rootItem->setSelected( true );
  fillGroup( listview, rootItem, root, (address == root.groupAddress() || address.isNull()) ? true : false, address );
  rootItem->setOpen( true );
}

static KBookmarkFolderTreeItem* ft_cast( TQListViewItem *i )
{
  return static_cast<KBookmarkFolderTreeItem*>( i );
}

TQString KBookmarkFolderTree::selectedAddress( TQListView *listview )
{
  if ( !listview)
    return TQString::null;
  KBookmarkFolderTreeItem *item = ft_cast( listview->currentItem() );
  return item ? item->m_bookmark.address() : TQString::null;
}

void KBookmarkFolderTree::setAddress( TQListView *listview, const TQString & address )
{
  KBookmarkFolderTreeItem* it = ft_cast( listview->firstChild() );
  while ( true ) {
    kdDebug(7043) << it->m_bookmark.address() << endl;
    it = ft_cast( it->itemBelow() );
    if ( !it )
      return;
    if ( it->m_bookmark.address() == address )
      break;
  }
  it->setSelected( true );
  listview->setCurrentItem( it );
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

// toplevel item
KBookmarkFolderTreeItem::KBookmarkFolderTreeItem( TQListView *parent, const KBookmark & gp )
   : TQListViewItem(parent, i18n("Bookmarks")), m_bookmark(gp)
{
  setPixmap(0, SmallIcon("bookmark"));
  setExpandable(true);
}

// group
KBookmarkFolderTreeItem::KBookmarkFolderTreeItem( KBookmarkFolderTreeItem *parent, TQListViewItem *after, const KBookmarkGroup & gp )
   : TQListViewItem(parent, after, gp.fullText()), m_bookmark(gp)
{
  setPixmap(0, SmallIcon( gp.icon() ) );
  setExpandable(true);
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

// NOTE - KBookmarkMenuNSImporter is really === KBookmarkMenuImporter
//        i.e, it is _not_ ns specific. and in KDE4 it should be renamed.

void KBookmarkMenuNSImporter::openNSBookmarks()
{
  openBookmarks( KNSBookmarkImporter::netscapeBookmarksFile(), "netscape" );
}

void KBookmarkMenuNSImporter::openBookmarks( const TQString &location, const TQString &type )
{
  mstack.push(m_menu);

  KBookmarkImporterBase *importer = KBookmarkImporterBase::factory(type);
  if (!importer)
     return;
  importer->setFilename(location);
  connectToImporter(*importer);
  importer->parse();

  delete importer;
}

void KBookmarkMenuNSImporter::connectToImporter(const TQObject &importer)
{
  connect( &importer, TQ_SIGNAL( newBookmark( const TQString &, const TQCString &, const TQString & ) ),
           TQ_SLOT( newBookmark( const TQString &, const TQCString &, const TQString & ) ) );
  connect( &importer, TQ_SIGNAL( newFolder( const TQString &, bool, const TQString & ) ),
           TQ_SLOT( newFolder( const TQString &, bool, const TQString & ) ) );
  connect( &importer, TQ_SIGNAL( newSeparator() ), TQ_SLOT( newSeparator() ) );
  connect( &importer, TQ_SIGNAL( endFolder() ), TQ_SLOT( endFolder() ) );
}

void KBookmarkMenuNSImporter::newBookmark( const TQString & text, const TQCString & url, const TQString & )
{
  TQString _text = KStringHandler::csqueeze(text);
  _text.replace( '&', "&&" );
  TDEAction * action = new KBookmarkAction(_text, "text-html", 0, 0, "", m_actionCollection, 0);
  connect(action, TQ_SIGNAL( activated ( TDEAction::ActivationReason, TQt::ButtonState )),
          m_menu, TQ_SLOT( slotBookmarkSelected( TDEAction::ActivationReason, TQt::ButtonState ) ));
  action->setProperty( "url", url );
  action->setToolTip( url );
  action->plug( mstack.top()->m_parentMenu );
  mstack.top()->m_actions.append( action );
}

void KBookmarkMenuNSImporter::newFolder( const TQString & text, bool, const TQString & )
{
  TQString _text = KStringHandler::csqueeze(text);
  _text.replace( '&', "&&" );
  TDEActionMenu * actionMenu = new TDEActionMenu( _text, "folder", m_actionCollection, 0L );
  actionMenu->plug( mstack.top()->m_parentMenu );
  mstack.top()->m_actions.append( actionMenu );
  KBookmarkMenu *subMenu = new KBookmarkMenu( m_pManager, m_menu->m_pOwner, actionMenu->popupMenu(),
                                              m_actionCollection, false,
                                              m_menu->m_bAddBookmark, TQString::null );
  connect( subMenu, TQ_SIGNAL( openBookmark( const TQString &, TQt::ButtonState ) ),
           m_menu, TQ_SIGNAL( openBookmark( const TQString &, TQt::ButtonState ) ));
  mstack.top()->m_lstSubMenus.append( subMenu );

  mstack.push(subMenu);
}

void KBookmarkMenuNSImporter::newSeparator()
{
  mstack.top()->m_parentMenu->insertSeparator();
}

void KBookmarkMenuNSImporter::endFolder()
{
  mstack.pop();
}

/********************************************************************/
/********************************************************************/
/********************************************************************/

KBookmarkMenu::DynMenuInfo KBookmarkMenu::showDynamicBookmarks( const TQString &id )
{
  TDEConfig config("kbookmarkrc", false, false);
  config.setGroup("Bookmarks");

  DynMenuInfo info;
  info.show = false;

  if (!config.hasKey("DynamicMenus")) {
    // upgrade path
    if (id == "netscape") {
      KBookmarkManager *manager = KBookmarkManager::userBookmarksManager();
      info.show = manager->root().internalElement().attribute("hide_nsbk") != "yes";
      info.location = KNSBookmarkImporter::netscapeBookmarksFile();
      info.type = "netscape";
      info.name = i18n("Netscape Bookmarks");
    } // else, no show

  } else {
    // have new version config
    if (config.hasGroup("DynamicMenu-" + id)) {
      config.setGroup("DynamicMenu-" + id);
      info.show = config.readBoolEntry("Show");
      info.location = config.readPathEntry("Location");
      info.type = config.readEntry("Type");
      info.name = config.readEntry("Name");
    } // else, no show
  }

  return info;
}

TQStringList KBookmarkMenu::dynamicBookmarksList()
{
  TDEConfig config("kbookmarkrc", false, false);
  config.setGroup("Bookmarks");

  TQStringList mlist;
  if (config.hasKey("DynamicMenus"))
    mlist = config.readListEntry("DynamicMenus");
  else
    mlist << "netscape";

  return mlist;
}

void KBookmarkMenu::setDynamicBookmarks(const TQString &id, const DynMenuInfo &newMenu)
{
  TDEConfig config("kbookmarkrc", false, false);

  // add group unconditionally
  config.setGroup("DynamicMenu-" + id);
  config.writeEntry("Show", newMenu.show);
  config.writePathEntry("Location", newMenu.location);
  config.writeEntry("Type", newMenu.type);
  config.writeEntry("Name", newMenu.name);

  TQStringList elist;

  config.setGroup("Bookmarks");
  if (!config.hasKey("DynamicMenus")) {
    if (newMenu.type != "netscape") {
      // update from old xbel method to new rc method
      // though only if not writing the netscape setting
      config.setGroup("DynamicMenu-" "netscape");
      DynMenuInfo xbelSetting;
      xbelSetting = showDynamicBookmarks("netscape");
      config.writeEntry("Show", xbelSetting.show);
      config.writePathEntry("Location", xbelSetting.location);
      config.writeEntry("Type", xbelSetting.type);
      config.writeEntry("Name", xbelSetting.name);
    }
  } else {
    elist = config.readListEntry("DynamicMenus");
  }

  // make sure list includes type
  config.setGroup("Bookmarks");
  if (elist.contains(id) < 1) {
    elist << id;
    config.writeEntry("DynamicMenus", elist);
  }

  config.sync();
}

#include "kbookmarkmenu.moc"
#include "kbookmarkmenu_p.moc"
