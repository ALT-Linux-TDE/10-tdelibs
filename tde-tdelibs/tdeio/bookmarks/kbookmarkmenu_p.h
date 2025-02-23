/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#ifndef __kbookmarkmenu_p_h__
#define __kbookmarkmenu_p_h__

#include <sys/types.h>

#include <tqptrlist.h>
#include <tqptrstack.h>
#include <tqobject.h>
#include <tqlistview.h>

#include <kdialogbase.h>
#include <tdelocale.h>
#include <tdeaction.h>

#include "kbookmark.h"
#include "kbookmarkimporter.h"
#include "kbookmarkmanager.h"

class TQString;
class TQPopupMenu;
class TQPushButton;
class TQListView;
class KLineEdit;
class KBookmark;
class KBookmarkGroup;
class TDEAction;
class TDEActionMenu;
class TDEActionCollection;
class KBookmarkOwner;
class KBookmarkMenu;
class KBookmarkBar;
class TDEPopupMenu;

class KImportedBookmarksActionMenu : public TDEActionMenu {
  TQ_OBJECT
  TQ_PROPERTY( TQString type READ type WRITE setType )
  TQ_PROPERTY( TQString location READ location WRITE setLocation )
public:
  const TQString type() const { return m_type; }
  void setType(const TQString &type) { m_type = type; }
  const TQString location() const { return m_location; }
  void setLocation(const TQString &location) { m_location = location; }
private:
  TQString m_type;
  TQString m_location;
public:
  KImportedBookmarksActionMenu( 
    const TQString &text, const TQString& sIconName,
    TDEActionCollection* parent, const char* name)
  : TDEActionMenu(text, sIconName, parent, name) {
     ;
  }
};

class KBookmarkActionMenu : public TDEActionMenu {
  TQ_OBJECT
  TQ_PROPERTY( TQString url READ url WRITE setUrl )
  TQ_PROPERTY( TQString address READ address WRITE setAddress )
  TQ_PROPERTY( bool readOnly READ readOnly WRITE setReadOnly )
public:
  const TQString url() const { return m_url; }
  void setUrl(const TQString &url) { m_url = url; }
  const TQString address() const { return m_address; }
  void setAddress(const TQString &address) { m_address = address; }
  bool readOnly() const { return m_readOnly; }
  void setReadOnly(bool readOnly) { m_readOnly = readOnly; }
private:
  TQString m_url;
  TQString m_address;
  bool m_readOnly;
public:
  KBookmarkActionMenu( 
    const TQString &text, const TQString& sIconName,
    TDEActionCollection* parent, const char* name)
  : TDEActionMenu(text, sIconName, parent, name) {
     ;
  }
};

class KBookmarkAction : public TDEAction {
  TQ_OBJECT
  TQ_PROPERTY( TQString url READ url WRITE setUrl )
  TQ_PROPERTY( TQString address READ address WRITE setAddress )
public:
  const TQString url() const { return m_url; }
  void setUrl(const TQString &url) { m_url = url; }
  const TQString address() const { return m_address; }
  void setAddress(const TQString &address) { m_address = address; }
private:
  TQString m_url;
  TQString m_address;
public:
  // KDE4: remove
  KBookmarkAction(
    const TQString& text, const TQString& sIconName, const TDEShortcut& cut,
    const TQObject* receiver, const char* slot,
    TDEActionCollection* parent, const char* name)
  : TDEAction(text, sIconName, cut, receiver, slot, parent, name) {
  }
  KBookmarkAction(
    const TQString& text, const TQString& sIconName, const TDEShortcut& cut,
    TDEActionCollection* parent, const char* name)
  : TDEAction(text, sIconName, cut, parent, name) {
  }
};

class KBookmarkEditFields {
public:
  typedef enum { FolderFieldsSet, BookmarkFieldsSet } FieldsSet;
  KLineEdit * m_url;
  KLineEdit * m_title;
  KBookmarkEditFields(TQWidget *main, TQBoxLayout *vbox, FieldsSet isFolder);
  void setName(const TQString &str);
  void setLocation(const TQString &str);
};

class KBookmarkEditDialog : public KDialogBase
{
  TQ_OBJECT

public:
  typedef enum { ModifyMode, InsertionMode } BookmarkEditType;

  KBookmarkEditDialog( const TQString& title, const TQString& url, KBookmarkManager *, BookmarkEditType editType, const TQString& address = TQString::null,
                       TQWidget * = 0, const char * = 0, const TQString& caption = i18n( "Add Bookmark" ) );

  TQString finalUrl() const;
  TQString finalTitle() const;
  TQString finalAddress() const;

protected slots:
  void slotOk();
  void slotCancel();
  void slotUser1();
  void slotDoubleClicked(TQListViewItem* item);

private:
  TQWidget * m_main;
  KBookmarkEditFields * m_fields;
  TQListView * m_folderTree;
  TQPushButton * m_button;
  KBookmarkManager * m_mgr;
  BookmarkEditType m_editType;
  TQString m_address;
};

class KBookmarkFolderTreeItem : public TQListViewItem
{
  // make this an accessor
  friend class KBookmarkFolderTree;
public:
  KBookmarkFolderTreeItem( TQListView *, const KBookmark & );
  KBookmarkFolderTreeItem( KBookmarkFolderTreeItem *, TQListViewItem *, const KBookmarkGroup & );
private:
  KBookmark m_bookmark;
};

class KBookmarkFolderTree
{
public:
  static TQListView* createTree( KBookmarkManager *, TQWidget * = 0, const char * = 0, const TQString& = TQString::null );
  static void fillTree( TQListView*, KBookmarkManager *, const TQString& = TQString::null );
  static TQString selectedAddress( TQListView* );
  static void setAddress( TQListView *, const TQString & );
};

class KBookmarkSettings 
{
public:
  bool m_advancedaddbookmark;
  bool m_contextmenu;
  bool m_quickactions;
  bool m_filteredtoolbar;
  static KBookmarkSettings *s_self;
  static void readSettings();
  static KBookmarkSettings *self();
};

class RMB
{
public:
  static void begin_rmb_action(KBookmarkMenu *);
  static void begin_rmb_action(KBookmarkBar *);
  bool invalid( int val );
  KBookmark atAddress(const TQString & address);
  void fillContextMenu( TQPopupMenu* contextMenu, const TQString & address, int val );
  void fillContextMenu2( TQPopupMenu* contextMenu, const TQString & address, int val );
  void slotRMBActionEditAt( int val );
  void slotRMBActionProperties( int val );
  void slotRMBActionInsert( int val );
  void slotRMBActionRemove( int val );
  void slotRMBActionCopyLocation( int val );
  void hidePopup();
public:
  TQObject *recv;
  KBookmarkManager *m_pManager;
  TQString s_highlightedAddress;
  TQString m_parentAddress;
  KBookmarkOwner *m_pOwner;
  TQWidget *m_parentMenu;
};

#endif
