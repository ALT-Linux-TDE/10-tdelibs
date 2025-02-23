/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>
              (C) 2003 Andras Mantia <amantia@kde.org>

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

#include "tdeactionclasses.h"

#include <assert.h>
#include <ft2build.h>
#include <fontconfig/fontconfig.h>

#include <tqcursor.h>
#include <tqclipboard.h>
#include <tqfontdatabase.h>
#include <tqobjectlist.h>
#include <tqwhatsthis.h>
#include <tqtimer.h>
#include <tqfile.h>
#include <tqregexp.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <tdeaccel.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdefontcombo.h>
#include <tdefontdialog.h>
#include <tdelocale.h>
#include <tdemainwindow.h>
#include <tdemenubar.h>
#include <tdepopupmenu.h>
#include <tdetoolbar.h>
#include <tdetoolbarbutton.h>
#include <kurl.h>
#include <kstandarddirs.h>
#include <kstringhandler.h>

class TDEToggleAction::TDEToggleActionPrivate
{
public:
  TDEToggleActionPrivate()
  {
    m_checked = false;
    m_checkedGuiItem = 0;
  }

  bool m_checked;
  TQString m_exclusiveGroup;
  KGuiItem* m_checkedGuiItem;
};

TDEToggleAction::TDEToggleAction( const TQString& text, const TDEShortcut& cut,
                              TQObject* parent,
                              const char* name )
    : TDEAction( text, cut, parent, name )
{
  d = new TDEToggleActionPrivate;
}

TDEToggleAction::TDEToggleAction( const TQString& text, const TDEShortcut& cut,
                              const TQObject* receiver, const char* slot,
                              TQObject* parent, const char* name )
  : TDEAction( text, cut, receiver, slot, parent, name )
{
  d = new TDEToggleActionPrivate;
}

TDEToggleAction::TDEToggleAction( const TQString& text, const TQIconSet& pix,
                              const TDEShortcut& cut,
                              TQObject* parent, const char* name )
  : TDEAction( text, pix, cut, parent, name )
{
  d = new TDEToggleActionPrivate;
}

TDEToggleAction::TDEToggleAction( const TQString& text, const TQString& pix,
                              const TDEShortcut& cut,
                              TQObject* parent, const char* name )
 : TDEAction( text, pix, cut, parent, name )
{
  d = new TDEToggleActionPrivate;
}

TDEToggleAction::TDEToggleAction( const TQString& text, const TQIconSet& pix,
                              const TDEShortcut& cut,
                              const TQObject* receiver,
                              const char* slot, TQObject* parent,
                              const char* name )
  : TDEAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new TDEToggleActionPrivate;
}

TDEToggleAction::TDEToggleAction( const TQString& text, const TQString& pix,
                              const TDEShortcut& cut,
                              const TQObject* receiver,
                              const char* slot, TQObject* parent,
                              const char* name )
  : TDEAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new TDEToggleActionPrivate;
}

TDEToggleAction::TDEToggleAction( TQObject* parent, const char* name )
    : TDEAction( parent, name )
{
  d = new TDEToggleActionPrivate;
}

TDEToggleAction::~TDEToggleAction()
{
  delete d->m_checkedGuiItem;
  delete d;
}

int TDEToggleAction::plug( TQWidget* widget, int index )
{
  if ( !::tqt_cast<TQPopupMenu *>( widget ) && !::tqt_cast<TDEToolBar *>( widget ) )
  {
    kdWarning() << "Can not plug TDEToggleAction in " << widget->className() << endl;
    return -1;
  }
  if (kapp && !kapp->authorizeTDEAction(name()))
    return -1;

  int _index = TDEAction::plug( widget, index );
  if ( _index == -1 )
    return _index;

  if ( ::tqt_cast<TDEToolBar *>( widget ) ) {
    TDEToolBar *bar = static_cast<TDEToolBar *>( widget );

    bar->setToggle( itemId( _index ), true );
    bar->setButton( itemId( _index ), isChecked() );
  }

  if ( d->m_checked )
    updateChecked( _index );

  return _index;
}

void TDEToggleAction::setChecked( bool c )
{
  if ( c == d->m_checked )
    return;
  //kdDebug(129) << "TDEToggleAction::setChecked(" << c << ") " << this << " " << name() << endl;

  d->m_checked = c;

  int len = containerCount();

  for( int i = 0; i < len; ++i )
    updateChecked( i );

  if ( c && parent() && !exclusiveGroup().isEmpty() ) {
    const TQObjectList list = parent()->childrenListObject();
    if ( !list.isEmpty() ) {
      TQObjectListIt it( list );
      for( ; it.current(); ++it ) {
          if ( ::tqt_cast<TDEToggleAction *>( it.current() ) && it.current() != this &&
            static_cast<TDEToggleAction*>(it.current())->exclusiveGroup() == exclusiveGroup() ) {
	  TDEToggleAction *a = static_cast<TDEToggleAction*>(it.current());
	  if( a->isChecked() ) {
	    a->setChecked( false );
	    emit a->toggled( false );
	  }
        }
      }
    }
  }
}

void TDEToggleAction::updateChecked( int id )
{
  TQWidget *w = container( id );

  if ( ::tqt_cast<TQPopupMenu *>( w ) ) {
    TQPopupMenu* pm = static_cast<TQPopupMenu*>(w);
    int itemId_ = itemId( id );
    if ( !d->m_checkedGuiItem )
      pm->setItemChecked( itemId_, d->m_checked );
    else {
      const KGuiItem* gui = d->m_checked ? d->m_checkedGuiItem : &guiItem();
      if ( d->m_checkedGuiItem->hasIcon() )
          pm->changeItem( itemId_, gui->iconSet( TDEIcon::Small ), gui->text() );
      else
          pm->changeItem( itemId_, gui->text() );

      // If the text doesn't change, then set the icon to be "pressed", otherwise
      // there is too little difference between checked and unchecked.
      if ( d->m_checkedGuiItem->text() == guiItem().text() )
           pm->setItemChecked( itemId_, d->m_checked );

      if ( !d->m_checkedGuiItem->whatsThis().isEmpty() ) // if empty, we keep the initial one
          pm->TQMenuData::setWhatsThis( itemId_, gui->whatsThis() );
      updateShortcut( pm, itemId_ );
    }
  }
  else if ( ::tqt_cast<TQMenuBar *>( w ) ) // not handled in plug...
    static_cast<TQMenuBar*>(w)->setItemChecked( itemId( id ), d->m_checked );
  else if ( ::tqt_cast<TDEToolBar *>( w ) )
  {
    TQWidget* r = static_cast<TDEToolBar*>( w )->getButton( itemId( id ) );
    if ( r && ::tqt_cast<TDEToolBarButton *>( r ) ) {
      static_cast<TDEToolBar*>( w )->setButton( itemId( id ), d->m_checked );
      if ( d->m_checkedGuiItem && d->m_checkedGuiItem->hasIcon() ) {
        const KGuiItem* gui = d->m_checked ? d->m_checkedGuiItem : &guiItem();
        static_cast<TDEToolBar*>( w )->setButtonIconSet( itemId( id ), gui->iconSet( TDEIcon::Toolbar ) );
      }
    }
  }
}

void TDEToggleAction::slotActivated()
{
  setChecked( !isChecked() );
  TDEAction::slotActivated();
  emit toggled( isChecked() );
}

bool TDEToggleAction::isChecked() const
{
  return d->m_checked;
}

void TDEToggleAction::setExclusiveGroup( const TQString& name )
{
  d->m_exclusiveGroup = name;
}

TQString TDEToggleAction::exclusiveGroup() const
{
  return d->m_exclusiveGroup;
}

void TDEToggleAction::setCheckedState( const KGuiItem& checkedItem )
{
  delete d->m_checkedGuiItem;
  d->m_checkedGuiItem = new KGuiItem( checkedItem );
}

TQString TDEToggleAction::toolTip() const
{
  if ( d->m_checkedGuiItem && d->m_checked )
      return d->m_checkedGuiItem->toolTip();
  else
      return TDEAction::toolTip();
}

TDERadioAction::TDERadioAction( const TQString& text, const TDEShortcut& cut,
                            TQObject* parent, const char* name )
: TDEToggleAction( text, cut, parent, name )
{
}

TDERadioAction::TDERadioAction( const TQString& text, const TDEShortcut& cut,
                            const TQObject* receiver, const char* slot,
                            TQObject* parent, const char* name )
: TDEToggleAction( text, cut, receiver, slot, parent, name )
{
}

TDERadioAction::TDERadioAction( const TQString& text, const TQIconSet& pix,
                            const TDEShortcut& cut,
                            TQObject* parent, const char* name )
: TDEToggleAction( text, pix, cut, parent, name )
{
}

TDERadioAction::TDERadioAction( const TQString& text, const TQString& pix,
                            const TDEShortcut& cut,
                            TQObject* parent, const char* name )
: TDEToggleAction( text, pix, cut, parent, name )
{
}

TDERadioAction::TDERadioAction( const TQString& text, const TQIconSet& pix,
                            const TDEShortcut& cut,
                            const TQObject* receiver, const char* slot,
                            TQObject* parent, const char* name )
: TDEToggleAction( text, pix, cut, receiver, slot, parent, name )
{
}

TDERadioAction::TDERadioAction( const TQString& text, const TQString& pix,
                            const TDEShortcut& cut,
                            const TQObject* receiver, const char* slot,
                            TQObject* parent, const char* name )
: TDEToggleAction( text, pix, cut, receiver, slot, parent, name )
{
}

TDERadioAction::TDERadioAction( TQObject* parent, const char* name )
: TDEToggleAction( parent, name )
{
}

void TDERadioAction::slotActivated()
{
  if ( isChecked() )
  {
    const TQObject *senderObj = sender();

    if ( !senderObj || !::tqt_cast<const TDEToolBarButton *>( senderObj ) )
      return;

    const_cast<TDEToolBarButton *>( static_cast<const TDEToolBarButton *>( senderObj ) )->on( true );

    return;
  }

  TDEToggleAction::slotActivated();
}

class TDESelectAction::TDESelectActionPrivate
{
public:
  TDESelectActionPrivate()
  {
    m_edit = false;
    m_menuAccelsEnabled = true;
    m_menu = 0;
    m_current = -1;
    m_comboWidth = -1;
    m_maxComboViewCount = -1;
  }
  bool m_edit;
  bool m_menuAccelsEnabled;
  TQPopupMenu *m_menu;
  int m_current;
  int m_comboWidth;
  TQStringList m_list;
  int m_maxComboViewCount;

  TQString makeMenuText( const TQString &_text )
  {
      if ( m_menuAccelsEnabled )
        return _text;
      TQString text = _text;
      uint i = 0;
      while ( i < text.length() ) {
          if ( text[ i ] == '&' ) {
              text.insert( i, '&' );
              i += 2;
          }
          else
              ++i;
      }
      return text;
  }
};

TDESelectAction::TDESelectAction( const TQString& text, const TDEShortcut& cut,
                              TQObject* parent, const char* name )
  : TDEAction( text, cut, parent, name )
{
  d = new TDESelectActionPrivate;
}

TDESelectAction::TDESelectAction( const TQString& text, const TDEShortcut& cut,
                              const TQObject* receiver, const char* slot,
                              TQObject* parent, const char* name )
  : TDEAction( text, cut, receiver, slot, parent, name )
{
  d = new TDESelectActionPrivate;
}

TDESelectAction::TDESelectAction( const TQString& text, const TQIconSet& pix,
                              const TDEShortcut& cut,
                              TQObject* parent, const char* name )
  : TDEAction( text, pix, cut, parent, name )
{
  d = new TDESelectActionPrivate;
}

TDESelectAction::TDESelectAction( const TQString& text, const TQString& pix,
                              const TDEShortcut& cut,
                              TQObject* parent, const char* name )
  : TDEAction( text, pix, cut, parent, name )
{
  d = new TDESelectActionPrivate;
}

TDESelectAction::TDESelectAction( const TQString& text, const TQIconSet& pix,
                              const TDEShortcut& cut,
                              const TQObject* receiver,
                              const char* slot, TQObject* parent,
                              const char* name )
  : TDEAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new TDESelectActionPrivate;
}

TDESelectAction::TDESelectAction( const TQString& text, const TQString& pix,
                              const TDEShortcut& cut,
                              const TQObject* receiver,
                              const char* slot, TQObject* parent,
                              const char* name )
  : TDEAction( text, pix, cut, receiver, slot, parent, name )
{
  d = new TDESelectActionPrivate;
}

TDESelectAction::TDESelectAction( TQObject* parent, const char* name )
  : TDEAction( parent, name )
{
  d = new TDESelectActionPrivate;
}

TDESelectAction::~TDESelectAction()
{
  assert(d);
  delete d->m_menu;
  delete d; d = 0;
}

void TDESelectAction::setCurrentItem( int id )
{
    if ( id >= (int)d->m_list.count() ) {
        Q_ASSERT(id < (int)d->m_list.count());
        return;
    }

    if ( d->m_menu )
    {
        if ( d->m_current >= 0 )
            d->m_menu->setItemChecked( d->m_current, false );
        if ( id >= 0 )
            d->m_menu->setItemChecked( id, true );
    }

    d->m_current = id;

    int len = containerCount();

    for( int i = 0; i < len; ++i )
        updateCurrentItem( i );

    //    emit TDEAction::activated();
    //    emit activated( currentItem() );
    //    emit activated( currentText() );
}

void TDESelectAction::setComboWidth( int width )
{
  if ( width < 0 )
    return;

  d->m_comboWidth=width;

  int len = containerCount();

  for( int i = 0; i < len; ++i )
    updateComboWidth( i );

}

void TDESelectAction::setMaxComboViewCount( int n )
{
  d->m_maxComboViewCount = n;
}

TQPopupMenu* TDESelectAction::popupMenu() const
{
	kdDebug(129) << "TDEAction::popupMenu()" << endl; // remove -- ellis
  if ( !d->m_menu )
  {
    d->m_menu = new TDEPopupMenu(0L, "TDESelectAction::popupMenu()");
    setupMenu();
    if ( d->m_current >= 0 )
      d->m_menu->setItemChecked( d->m_current, true );
  }

  return d->m_menu;
}

void TDESelectAction::setupMenu() const
{
    if ( !d->m_menu )
        return;
    d->m_menu->clear();

    TQStringList::ConstIterator it = d->m_list.begin();
    for( uint id = 0; it != d->m_list.end(); ++it, ++id ) {
        TQString text = *it;
        if ( !text.isEmpty() )
            d->m_menu->insertItem( d->makeMenuText( text ), this, TQ_SLOT( slotActivated( int ) ), 0, id );
        else
            d->m_menu->insertSeparator();
    }
}

void TDESelectAction::changeItem( int index, const TQString& text )
{
  if ( index < 0 || index >= (int)d->m_list.count() )
  {
    kdWarning() << "TDESelectAction::changeItem Index out of scope" << endl;
    return;
  }

  d->m_list[ index ] = text;

  if ( d->m_menu )
    d->m_menu->changeItem( index, d->makeMenuText( text ) );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    changeItem( i, index, text );
}

void TDESelectAction::changeItem( int id, int index, const TQString& text)
{
  if ( index < 0 )
        return;

  TQWidget* w = container( id );
  if ( ::tqt_cast<TDEToolBar *>( w ) )
  {
     TQWidget* r = (static_cast<TDEToolBar*>( w ))->getWidget( itemId( id ) );
     if ( ::tqt_cast<TQComboBox *>( r ) )
     {
        TQComboBox *b = static_cast<TQComboBox*>( r );
        b->changeItem(text, index );
     }
  }
}

void TDESelectAction::setItems( const TQStringList &lst )
{
  d->m_list = lst;
  d->m_current = -1;

  setupMenu();

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateItems( i );

  // Disable if empty and not editable
  setEnabled ( lst.count() > 0 || d->m_edit );
}

TQStringList TDESelectAction::items() const
{
  return d->m_list;
}

TQString TDESelectAction::currentText() const
{
  if ( currentItem() < 0 )
    return TQString::null;

  return d->m_list[ currentItem() ];
}

int TDESelectAction::currentItem() const
{
  return d->m_current;
}

void TDESelectAction::updateCurrentItem( int id )
{
  if ( d->m_current < 0 )
        return;

  TQWidget* w = container( id );
  if ( ::tqt_cast<TDEToolBar *>( w ) ) {
    TQWidget* r = static_cast<TDEToolBar*>( w )->getWidget( itemId( id ) );
    if ( ::tqt_cast<TQComboBox *>( r ) ) {
      TQComboBox *b = static_cast<TQComboBox*>( r );
      b->setCurrentItem( d->m_current );
    }
  }
}

int TDESelectAction::comboWidth() const
{
  return d->m_comboWidth;
}

void TDESelectAction::updateComboWidth( int id )
{
  TQWidget* w = container( id );
  if ( ::tqt_cast<TDEToolBar *>( w ) ) {
    TQWidget* r = static_cast<TDEToolBar*>( w )->getWidget( itemId( id ) );
    if ( ::tqt_cast<TQComboBox *>( r ) ) {
      TQComboBox *cb = static_cast<TQComboBox*>( r );
      cb->setMinimumWidth( d->m_comboWidth );
      cb->setMaximumWidth( d->m_comboWidth );
    }
  }
}

void TDESelectAction::updateItems( int id )
{
  kdDebug(129) << "TDEAction::updateItems( " << id << ", lst )" << endl; // remove -- ellis
  TQWidget* w = container( id );
  if ( ::tqt_cast<TDEToolBar *>( w ) ) {
    TQWidget* r = static_cast<TDEToolBar*>( w )->getWidget( itemId( id ) );
    if ( ::tqt_cast<TQComboBox *>( r ) ) {
      TQComboBox *cb = static_cast<TQComboBox*>( r );
      cb->clear();
      TQStringList lst = comboItems();
      TQStringList::ConstIterator it = lst.begin();
      for( ; it != lst.end(); ++it )
        cb->insertItem( *it );
      // qt caches and never recalculates the sizeHint()
      // qcombobox.cpp recommends calling setFont to invalidate the sizeHint
      // setFont sets own_font = True, so we're a bit mean and calll
      // unsetFont which calls setFont and then overwrites the own_font
      cb->unsetFont();
    }
   }
}

int TDESelectAction::plug( TQWidget *widget, int index )
{
  if (kapp && !kapp->authorizeTDEAction(name()))
    return -1;
  kdDebug(129) << "TDESelectAction::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( ::tqt_cast<TQPopupMenu *>( widget) )
  {
    // Create the PopupMenu and store it in m_menu
    (void)popupMenu();

    TQPopupMenu* menu = static_cast<TQPopupMenu*>( widget );
    int id;
    if ( hasIcon() )
      id = menu->insertItem( iconSet(), text(), d->m_menu, -1, index );
    else
      id = menu->insertItem( text(), d->m_menu, -1, index );

    if ( !isEnabled() )
        menu->setItemEnabled( id, false );

    TQString wth = whatsThis();
    if ( !wth.isEmpty() )
        menu->TQMenuData::setWhatsThis( id, wth );

    addContainer( menu, id );
    connect( menu, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( ::tqt_cast<TDEToolBar *>( widget ) )
  {
    TDEToolBar* bar = static_cast<TDEToolBar*>( widget );
    int id_ = TDEAction::getToolButtonID();
    bar->insertCombo( comboItems(), id_, isEditable(),
                      TQ_SIGNAL( activated( const TQString & ) ), this,
                      TQ_SLOT( slotActivated( const TQString & ) ), isEnabled(),
                      toolTip(), -1, index );

    TQComboBox *cb = bar->getCombo( id_ );
    if ( cb )
    {
      if (!isEditable()) cb->setFocusPolicy(TQWidget::NoFocus);
      cb->setMinimumWidth( cb->sizeHint().width() );
      if ( d->m_comboWidth > 0 )
      {
        cb->setMinimumWidth( d->m_comboWidth );
        cb->setMaximumWidth( d->m_comboWidth );
      }
      cb->setInsertionPolicy( TQComboBox::NoInsertion );
      TQWhatsThis::add( cb, whatsThis() );
      if ( d->m_maxComboViewCount != -1 ) cb->setSizeLimit( d->m_maxComboViewCount );
    }

    addContainer( bar, id_ );

    connect( bar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    updateCurrentItem( containerCount() - 1 );

    return containerCount() - 1;
  }
  else if ( ::tqt_cast<TQMenuBar *>( widget ) )
  {
    // Create the PopupMenu and store it in m_menu
    (void)popupMenu();

    TQMenuBar* menu = static_cast<TQMenuBar*>( widget );
    int id = menu->insertItem( text(), d->m_menu, -1, index );

    if ( !isEnabled() )
        menu->setItemEnabled( id, false );

    TQString wth = whatsThis();
    if ( !wth.isEmpty() )
        menu->TQMenuData::setWhatsThis( id, wth );

    addContainer( menu, id );
    connect( menu, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  kdWarning() << "Can not plug TDEAction in " << widget->className() << endl;
  return -1;
}

TQStringList TDESelectAction::comboItems() const
{
  if( d->m_menuAccelsEnabled ) {
    TQStringList lst;
    TQStringList::ConstIterator it = d->m_list.begin();
    for( ; it != d->m_list.end(); ++it )
    {
      TQString item = *it;
      int i = item.find( '&' );
      if ( i > -1 )
        item = item.remove( i, 1 );
      lst.append( item );
    }
    return lst;
  }
  else
    return d->m_list;
}

void TDESelectAction::clear()
{
  if ( d->m_menu )
    d->m_menu->clear();

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateClear( i );
}

void TDESelectAction::updateClear( int id )
{
  TQWidget* w = container( id );
  if ( ::tqt_cast<TDEToolBar *>( w ) ) {
    TQWidget* r = static_cast<TDEToolBar*>( w )->getWidget( itemId( id ) );
    if ( ::tqt_cast<TQComboBox *>( r ) ) {
      TQComboBox *b = static_cast<TQComboBox*>( r );
      b->clear();
    }
  }
}

void TDESelectAction::slotActivated( int id )
{
  if ( d->m_current == id )
    return;

  setCurrentItem( id );
  // Delay this. Especially useful when the slot connected to activated() will re-create
  // the menu, e.g. in the recent files action. This prevents a crash.
  TQTimer::singleShot( 0, this, TQ_SLOT( slotActivated() ) );
}

void TDESelectAction::slotActivated( const TQString &text )
{
  if ( isEditable() )
  {
    TQStringList lst = d->m_list;
    if(!lst.contains(text))
    {
      lst.append( text );
      setItems( lst );
    }
  }

  int i = d->m_list.findIndex( text );
  if ( i > -1 )
      setCurrentItem( i );
  else
      setCurrentItem( comboItems().findIndex( text ) );
  // Delay this. Especially useful when the slot connected to activated() will re-create
  // the menu, e.g. in the recent files action. This prevents a crash.
  TQTimer::singleShot( 0, this, TQ_SLOT( slotActivated() ) );
}

void TDESelectAction::slotActivated()
{
  TDEAction::slotActivated();
  kdDebug(129) << "TDESelectAction::slotActivated currentItem=" << currentItem() << " currentText=" << currentText() << endl;
  emit activated( currentItem() );
  emit activated( currentText() );
}

void TDESelectAction::setEditable( bool edit )
{
  d->m_edit = edit;
}

bool TDESelectAction::isEditable() const
{
  return d->m_edit;
}

void TDESelectAction::setRemoveAmpersandsInCombo( bool b )
{
  setMenuAccelsEnabled( b );
}

bool TDESelectAction::removeAmpersandsInCombo() const
{
  return menuAccelsEnabled( );
}

void TDESelectAction::setMenuAccelsEnabled( bool b )
{
  d->m_menuAccelsEnabled = b;
}

bool TDESelectAction::menuAccelsEnabled() const
{
  return d->m_menuAccelsEnabled;
}

class TDEListAction::TDEListActionPrivate
{
public:
  TDEListActionPrivate()
  {
    m_current = 0;
  }
  int m_current;
};

TDEListAction::TDEListAction( const TQString& text, const TDEShortcut& cut,
                          TQObject* parent, const char* name )
  : TDESelectAction( text, cut, parent, name )
{
  d = new TDEListActionPrivate;
}

TDEListAction::TDEListAction( const TQString& text, const TDEShortcut& cut,
                          const TQObject* receiver, const char* slot,
                          TQObject* parent, const char* name )
  : TDESelectAction( text, cut, parent, name )
{
  d = new TDEListActionPrivate;
  if ( receiver )
    connect( this, TQ_SIGNAL( activated( int ) ), receiver, slot );
}

TDEListAction::TDEListAction( const TQString& text, const TQIconSet& pix,
                          const TDEShortcut& cut,
                          TQObject* parent, const char* name )
  : TDESelectAction( text, pix, cut, parent, name )
{
  d = new TDEListActionPrivate;
}

TDEListAction::TDEListAction( const TQString& text, const TQString& pix,
                          const TDEShortcut& cut,
                          TQObject* parent, const char* name )
  : TDESelectAction( text, pix, cut, parent, name )
{
  d = new TDEListActionPrivate;
}

TDEListAction::TDEListAction( const TQString& text, const TQIconSet& pix,
                          const TDEShortcut& cut, const TQObject* receiver,
                          const char* slot, TQObject* parent,
                          const char* name )
  : TDESelectAction( text, pix, cut, parent, name )
{
  d = new TDEListActionPrivate;
  if ( receiver )
    connect( this, TQ_SIGNAL( activated( int ) ), receiver, slot );
}

TDEListAction::TDEListAction( const TQString& text, const TQString& pix,
                          const TDEShortcut& cut, const TQObject* receiver,
                          const char* slot, TQObject* parent,
                          const char* name )
  : TDESelectAction( text, pix, cut, parent, name )
{
  d = new TDEListActionPrivate;
  if ( receiver )
    connect( this, TQ_SIGNAL( activated( int ) ), receiver, slot );
}

TDEListAction::TDEListAction( TQObject* parent, const char* name )
  : TDESelectAction( parent, name )
{
  d = new TDEListActionPrivate;
}

TDEListAction::~TDEListAction()
{
  delete d; d = 0;
}

void TDEListAction::setCurrentItem( int index )
{
  TDESelectAction::setCurrentItem( index );
  d->m_current = index;

  //  emit TDEAction::activated();
  //  emit activated( currentItem() );
  // emit activated( currentText() );
}

TQString TDEListAction::currentText() const
{
  return TDESelectAction::currentText();
}

int TDEListAction::currentItem() const
{
  return d->m_current;
}

class TDERecentFilesAction::TDERecentFilesActionPrivate
{
public:
  TDERecentFilesActionPrivate()
  {
    m_maxItems = 0;
    m_popup = 0;
  }
  uint m_maxItems;
  TDEPopupMenu *m_popup;
  TQMap<TQString, TQString> m_shortNames;
  TQMap<TQString, KURL> m_urls;
};

TDERecentFilesAction::TDERecentFilesAction( const TQString& text,
                                        const TDEShortcut& cut,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : TDEListAction( text, cut, parent, name)
{
  d = new TDERecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

TDERecentFilesAction::TDERecentFilesAction( const TQString& text,
                                        const TDEShortcut& cut,
                                        const TQObject* receiver,
                                        const char* slot,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : TDEListAction( text, cut, parent, name)
{
  d = new TDERecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  if ( receiver )
    connect( this,     TQ_SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

TDERecentFilesAction::TDERecentFilesAction( const TQString& text,
                                        const TQIconSet& pix,
                                        const TDEShortcut& cut,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : TDEListAction( text, pix, cut, parent, name)
{
  d = new TDERecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

TDERecentFilesAction::TDERecentFilesAction( const TQString& text,
                                        const TQString& pix,
                                        const TDEShortcut& cut,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : TDEListAction( text, pix, cut, parent, name)
{
  d = new TDERecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

TDERecentFilesAction::TDERecentFilesAction( const TQString& text,
                                        const TQIconSet& pix,
                                        const TDEShortcut& cut,
                                        const TQObject* receiver,
                                        const char* slot,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : TDEListAction( text, pix, cut, parent, name)
{
  d = new TDERecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  if ( receiver )
    connect( this,     TQ_SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

TDERecentFilesAction::TDERecentFilesAction( const TQString& text,
                                        const TQString& pix,
                                        const TDEShortcut& cut,
                                        const TQObject* receiver,
                                        const char* slot,
                                        TQObject* parent, const char* name,
                                        uint maxItems )
  : TDEListAction( text, pix, cut, parent, name)
{
  d = new TDERecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();

  if ( receiver )
    connect( this,     TQ_SIGNAL(urlSelected(const KURL&)),
             receiver, slot );
}

TDERecentFilesAction::TDERecentFilesAction( TQObject* parent, const char* name,
                                        uint maxItems )
  : TDEListAction( parent, name )
{
  d = new TDERecentFilesActionPrivate;
  d->m_maxItems = maxItems;

  init();
}

void TDERecentFilesAction::init()
{
  TDERecentFilesAction *that = const_cast<TDERecentFilesAction*>(this);
  that->d->m_popup = new TDEPopupMenu;
  connect(d->m_popup, TQ_SIGNAL(aboutToShow()), this, TQ_SLOT(menuAboutToShow()));
  connect(d->m_popup, TQ_SIGNAL(activated(int)), this, TQ_SLOT(menuItemActivated(int)));
  connect( this, TQ_SIGNAL( activated( const TQString& ) ),
           this, TQ_SLOT( itemSelected( const TQString& ) ) );

  setMenuAccelsEnabled( false );
}

TDERecentFilesAction::~TDERecentFilesAction()
{
  delete d->m_popup;
  delete d; d = 0;
}

uint TDERecentFilesAction::maxItems() const
{
    return d->m_maxItems;
}

void TDERecentFilesAction::setMaxItems( uint maxItems )
{
    TQStringList lst = TDESelectAction::items();
    uint oldCount   = lst.count();

    // set new maxItems
    d->m_maxItems = maxItems;

    // remove all items that are too much
    while( lst.count() > maxItems )
    {
        // remove last item
        TQString lastItem = lst.last();
        d->m_shortNames.erase( lastItem );
        d->m_urls.erase( lastItem );
        lst.remove( lastItem );
    }

    // set new list if changed
    if( lst.count() != oldCount )
        setItems( lst );
}

void TDERecentFilesAction::addURL( const KURL& url )
{
    addURL( url, url.fileName() );
}

void TDERecentFilesAction::addURL( const KURL& url, const TQString& name )
{
    if ( url.isLocalFile() && !TDEGlobal::dirs()->relativeLocation("tmp", url.path()).startsWith("/"))
       return;
    const TQString file = url.pathOrURL();
    TQStringList lst = TDESelectAction::items();

    // remove file if already in list
    const TQStringList::Iterator end = lst.end();
    for ( TQStringList::Iterator it = lst.begin(); it != end; ++it )
    {
      TQString title = (*it);
      if ( title.endsWith( file + "]" ) )
      {
        lst.remove( it );
        d->m_urls.erase( title );
        d->m_shortNames.erase( title );
        break;
      }
    }
    // remove last item if already maxitems in list
    if( lst.count() == d->m_maxItems )
    {
        // remove last item
        const TQString lastItem = lst.last();
        d->m_shortNames.erase( lastItem );
        d->m_urls.erase( lastItem );
        lst.remove( lastItem );
    }

    // add file to list
    const TQString title = name + " [" + file + "]";
    d->m_shortNames.insert( title, name );
    d->m_urls.insert( title, url );
    lst.prepend( title );
    setItems( lst );
}

void TDERecentFilesAction::removeURL( const KURL& url )
{
    TQStringList lst = TDESelectAction::items();
    TQString     file = url.pathOrURL();

    // remove url
    TQStringList::Iterator end = lst.end();
    for ( TQStringList::Iterator it = lst.begin(); it != end; ++it )
    {
      if ( (*it).endsWith( file + "]" ))
      {
        d->m_shortNames.erase( (*it) );
        d->m_urls.erase( (*it) );
        lst.remove( it );
        setItems( lst );
        break;
      }
    }
}

void TDERecentFilesAction::clearURLList()
{
    clear();
    d->m_shortNames.clear();
    d->m_urls.clear();
}

void TDERecentFilesAction::loadEntries( TDEConfig* config, TQString groupname)
{
    TQString     key;
    TQString     value;
    TQString     nameKey;
    TQString     nameValue;
    TQString      title;
    TQString     oldGroup;
    TQStringList lst;
    KURL        url;

    oldGroup = config->group();

    if (groupname.isEmpty())
      groupname = "RecentFiles";
    config->setGroup( groupname );

    // read file list
    for( unsigned int i = 1 ; i <= d->m_maxItems ; i++ )
    {
        key = TQString( "File%1" ).arg( i );
        value = config->readPathEntry( key );
        url = KURL::fromPathOrURL( value );

        // Don't restore if file doesn't exist anymore
        if (url.isLocalFile() && !TQFile::exists(url.path()))
          continue;

        nameKey = TQString( "Name%1" ).arg( i );
        nameValue = config->readPathEntry( nameKey, url.fileName() );
        title = nameValue + " [" + value + "]";
        if (!value.isNull())
        {
          lst.append( title );
          d->m_shortNames.insert( title, nameValue );
          d->m_urls.insert( title, url );
        }
    }

    // set file
    setItems( lst );

    config->setGroup( oldGroup );
}

void TDERecentFilesAction::saveEntries( TDEConfig* config, TQString groupname )
{
    TQString     key;
    TQString     value;
    TQString     oldGroup;
    TQStringList lst = TDESelectAction::items();

    oldGroup = config->group();

    if (groupname.isEmpty())
      groupname = "RecentFiles";
    config->deleteGroup( groupname, true );
    config->setGroup( groupname );

    // write file list
    for( unsigned int i = 1 ; i <= lst.count() ; i++ )
    {
        //kdDebug(129) << "Entry for " << lst[i-1] << d->m_urls[ lst[ i - 1 ] ] << endl;
        key = TQString( "File%1" ).arg( i );
        value = d->m_urls[ lst[ i - 1 ] ].pathOrURL();
        config->writePathEntry( key, value );
        key = TQString( "Name%1" ).arg( i );
        value = d->m_shortNames[ lst[ i - 1 ] ];
        config->writePathEntry( key, value );
    }

    config->setGroup( oldGroup );
}

void TDERecentFilesAction::itemSelected( const TQString& text )
{
    //return a copy of the URL since the slot where it is connected might call
    //addURL or removeURL where the d->m_urls.erase( title ) could destroy the
    //d->m_urls[ text ] and the emitted URL will be invalid in the rest of the slot
    emit urlSelected( KURL(d->m_urls[ text ]) );
}

void TDERecentFilesAction::menuItemActivated( int id )
{
    TQString text = d->m_popup->text(id);
    //return a copy of the URL since the slot where it is connected might call
    //addURL or removeURL where the d->m_urls.erase( title ) could destroy the
    //d->m_urls[ text ] and the emitted URL will be invalid in the rest of the slot
    emit urlSelected( KURL(d->m_urls[ text ]) );
}

void TDERecentFilesAction::menuAboutToShow()
{
    TDEPopupMenu *menu = d->m_popup;
    menu->clear();
    TQStringList list = TDESelectAction::items();
    for ( TQStringList::Iterator it = list.begin(); it != list.end(); ++it )
    {
       menu->insertItem(*it);
    }
}

int TDERecentFilesAction::plug( TQWidget *widget, int index )
{
  if (kapp && !kapp->authorizeTDEAction(name()))
    return -1;
  // This is very related to TDEActionMenu::plug.
  // In fact this class could be an interesting base class for TDEActionMenu
  if ( ::tqt_cast<TDEToolBar *>( widget ) )
  {
    TDEToolBar *bar = (TDEToolBar *)widget;

    int id_ = TDEAction::getToolButtonID();

    TDEInstance * instance;
    if ( m_parentCollection )
        instance = m_parentCollection->instance();
    else
        instance = TDEGlobal::instance();

    bar->insertButton( icon(), id_, TQ_SIGNAL( clicked() ), this,
                       TQ_SLOT( slotClicked() ), isEnabled(), plainText(),
                       index, instance );

    addContainer( bar, id_ );

    connect( bar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    bar->setDelayedPopup( id_, d->m_popup, true);

    if ( !whatsThis().isEmpty() )
        TQWhatsThis::add( bar->getButton( id_ ), whatsThisWithIcon() );

    return containerCount() - 1;
  }

  return TDEListAction::plug( widget, index );
}

void TDERecentFilesAction::slotClicked()
{
  TDEAction::slotActivated();
}

void TDERecentFilesAction::slotActivated(const TQString& text)
{
  TDEListAction::slotActivated(text);
}


void TDERecentFilesAction::slotActivated(int id)
{
  TDEListAction::slotActivated(id);
}


void TDERecentFilesAction::slotActivated()
{
  emit activated( currentItem() );
  emit activated( currentText() );
}

//KDE4: rename to urls() and return a KURL::List
TQStringList TDERecentFilesAction::items() const
{
    TQStringList lst = TDESelectAction::items();
    TQStringList result;

    for( unsigned int i = 1 ; i <= lst.count() ; i++ )
    {
        result += d->m_urls[ lst[ i - 1 ] ].prettyURL(0, KURL::StripFileProtocol);
    }

    return result;
}

//KDE4: remove
TQStringList TDERecentFilesAction::completeItems() const
{
    return TDESelectAction::items();
}


class TDEFontAction::TDEFontActionPrivate
{
public:
  TDEFontActionPrivate()
  {
  }
  TQStringList m_fonts;
};

TDEFontAction::TDEFontAction( const TQString& text,
                          const TDEShortcut& cut, TQObject* parent,
                          const char* name )
  : TDESelectAction( text, cut, parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, 0 );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::TDEFontAction( const TQString& text, const TDEShortcut& cut,
                          const TQObject* receiver, const char* slot,
                          TQObject* parent, const char* name )
    : TDESelectAction( text, cut, receiver, slot, parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, 0 );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::TDEFontAction( const TQString& text, const TQIconSet& pix,
                          const TDEShortcut& cut,
                          TQObject* parent, const char* name )
    : TDESelectAction( text, pix, cut, parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, 0 );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::TDEFontAction( const TQString& text, const TQString& pix,
                          const TDEShortcut& cut,
                          TQObject* parent, const char* name )
    : TDESelectAction( text, pix, cut, parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, 0 );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::TDEFontAction( const TQString& text, const TQIconSet& pix,
                          const TDEShortcut& cut,
                          const TQObject* receiver, const char* slot,
                          TQObject* parent, const char* name )
    : TDESelectAction( text, pix, cut, receiver, slot, parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, 0 );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::TDEFontAction( const TQString& text, const TQString& pix,
                          const TDEShortcut& cut,
                          const TQObject* receiver, const char* slot,
                          TQObject* parent, const char* name )
    : TDESelectAction( text, pix, cut, receiver, slot, parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, 0 );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::TDEFontAction( uint fontListCriteria, const TQString& text,
                          const TDEShortcut& cut, TQObject* parent,
                          const char* name )
    : TDESelectAction( text, cut, parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, fontListCriteria );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::TDEFontAction( uint fontListCriteria, const TQString& text, const TQString& pix,
                          const TDEShortcut& cut,
                          TQObject* parent, const char* name )
    : TDESelectAction( text, pix, cut, parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, fontListCriteria );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::TDEFontAction( TQObject* parent, const char* name )
  : TDESelectAction( parent, name )
{
    d = new TDEFontActionPrivate;
    TDEFontChooser::getFontList( d->m_fonts, 0 );
    TDESelectAction::setItems( d->m_fonts );
    setEditable( true );
}

TDEFontAction::~TDEFontAction()
{
    delete d;
    d = 0;
}

/*
 * Maintenance note: Keep in sync with TDEFontCombo::setCurrentFont()
 */
void TDEFontAction::setFont( const TQString &family )
{
    TQString lowerName = family.lower();
    int i = 0;
    for ( TQStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
    {
       if ((*it).lower() == lowerName)
       {
          setCurrentItem(i);
          return;
       }
    }
    i = lowerName.find(" [");
    if (i>-1)
    {
       lowerName = lowerName.left(i);
       i = 0;
       for ( TQStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
       {
          if ((*it).lower() == lowerName)
          {
             setCurrentItem(i);
             return;
          }
       }
    }

    lowerName += " [";
    i = 0;
    for ( TQStringList::Iterator it = d->m_fonts.begin(); it != d->m_fonts.end(); ++it, ++i )
    {
       if ((*it).lower().startsWith(lowerName))
       {
          setCurrentItem(i);
          return;
       }
    }

    // nothing matched yet, try a fontconfig reverse lookup and
    // check again to solve an alias
    FcPattern *pattern = NULL;
    FcConfig *config = NULL;
    FcResult result;
    TQString realFamily;
    TQRegExp regExp("[-:]");
    pattern = FcNameParse( (unsigned char*) family.ascii() );
    FcDefaultSubstitute(pattern);
    FcConfigSubstitute (config, pattern, FcMatchPattern);
    pattern = FcFontMatch(NULL, pattern, &result);
    realFamily = (char*)FcNameUnparse(pattern);
    realFamily.remove(realFamily.find(regExp), realFamily.length());

    if ( !realFamily.isEmpty() && realFamily != family )
       setFont( realFamily );
    else
       kdDebug(129) << "Font not found " << family.lower() << endl;
}

int TDEFontAction::plug( TQWidget *w, int index )
{
  if (kapp && !kapp->authorizeTDEAction(name()))
    return -1;
  if ( ::tqt_cast<TDEToolBar *>( w ) )
  {
    TDEToolBar* bar = static_cast<TDEToolBar*>( w );
    int id_ = TDEAction::getToolButtonID();
    TDEFontCombo *cb = new TDEFontCombo( items(), bar );
    connect( cb, TQ_SIGNAL( activated( const TQString & ) ),
             TQ_SLOT( slotActivated( const TQString & ) ) );
    cb->setEnabled( isEnabled() );
    bar->insertWidget( id_, comboWidth(), cb, index );
    cb->setMinimumWidth( cb->sizeHint().width() );

    addContainer( bar, id_ );

    connect( bar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    updateCurrentItem( containerCount() - 1 );

    return containerCount() - 1;
  }
  else return TDESelectAction::plug( w, index );
}

class TDEFontSizeAction::TDEFontSizeActionPrivate
{
public:
  TDEFontSizeActionPrivate()
  {
  }
};

TDEFontSizeAction::TDEFontSizeAction( const TQString& text,
                                  const TDEShortcut& cut,
                                  TQObject* parent, const char* name )
  : TDESelectAction( text, cut, parent, name )
{
  init();
}

TDEFontSizeAction::TDEFontSizeAction( const TQString& text,
                                  const TDEShortcut& cut,
                                  const TQObject* receiver, const char* slot,
                                  TQObject* parent, const char* name )
  : TDESelectAction( text, cut, receiver, slot, parent, name )
{
  init();
}

TDEFontSizeAction::TDEFontSizeAction( const TQString& text, const TQIconSet& pix,
                                  const TDEShortcut& cut,
                                  TQObject* parent, const char* name )
  : TDESelectAction( text, pix, cut, parent, name )
{
  init();
}

TDEFontSizeAction::TDEFontSizeAction( const TQString& text, const TQString& pix,
                                  const TDEShortcut& cut,
                                  TQObject* parent, const char* name )
  : TDESelectAction( text, pix, cut, parent, name )
{
  init();
}

TDEFontSizeAction::TDEFontSizeAction( const TQString& text, const TQIconSet& pix,
                                  const TDEShortcut& cut,
                                  const TQObject* receiver,
                                  const char* slot, TQObject* parent,
                                  const char* name )
    : TDESelectAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

TDEFontSizeAction::TDEFontSizeAction( const TQString& text, const TQString& pix,
                                  const TDEShortcut& cut,
                                  const TQObject* receiver,
                                  const char* slot, TQObject* parent,
                                  const char* name )
  : TDESelectAction( text, pix, cut, receiver, slot, parent, name )
{
  init();
}

TDEFontSizeAction::TDEFontSizeAction( TQObject* parent, const char* name )
  : TDESelectAction( parent, name )
{
  init();
}

TDEFontSizeAction::~TDEFontSizeAction()
{
    delete d;
    d = 0;
}

void TDEFontSizeAction::init()
{
    d = new TDEFontSizeActionPrivate;

    setEditable( true );
    TQFontDatabase fontDB;
    TQValueList<int> sizes = fontDB.standardSizes();
    TQStringList lst;
    for ( TQValueList<int>::Iterator it = sizes.begin(); it != sizes.end(); ++it )
        lst.append( TQString::number( *it ) );

    setItems( lst );
}

void TDEFontSizeAction::setFontSize( int size )
{
    if ( size == fontSize() ) {
        setCurrentItem( items().findIndex( TQString::number( size ) ) );
        return;
    }

    if ( size < 1 ) {
        kdWarning() << "TDEFontSizeAction: Size " << size << " is out of range" << endl;
        return;
    }

    int index = items().findIndex( TQString::number( size ) );
    if ( index == -1 ) {
        // Insert at the correct position in the list (to keep sorting)
        TQValueList<int> lst;
        // Convert to list of ints
        TQStringList itemsList = items();
        for (TQStringList::Iterator it = itemsList.begin() ; it != itemsList.end() ; ++it)
            lst.append( (*it).toInt() );
        // New size
        lst.append( size );
        // Sort the list
        qHeapSort( lst );
        // Convert back to string list
        TQStringList strLst;
        for (TQValueList<int>::Iterator it = lst.begin() ; it != lst.end() ; ++it)
            strLst.append( TQString::number(*it) );
        TDESelectAction::setItems( strLst );
        // Find new current item
        index = lst.findIndex( size );
        setCurrentItem( index );
    }
    else
        setCurrentItem( index );


    //emit TDEAction::activated();
    //emit activated( index );
    //emit activated( TQString::number( size ) );
    //emit fontSizeChanged( size );
}

int TDEFontSizeAction::fontSize() const
{
  return currentText().toInt();
}

void TDEFontSizeAction::slotActivated( int index )
{
  TDESelectAction::slotActivated( index );

  emit fontSizeChanged( items()[ index ].toInt() );
}

void TDEFontSizeAction::slotActivated( const TQString& size )
{
  setFontSize( size.toInt() ); // insert sorted first
  TDESelectAction::slotActivated( size );
  emit fontSizeChanged( size.toInt() );
}

class TDEActionMenu::TDEActionMenuPrivate
{
public:
  TDEActionMenuPrivate()
  {
    m_popup = new TDEPopupMenu(0L,"TDEActionMenu::TDEActionMenuPrivate");
    m_delayed = true;
    m_stickyMenu = true;
  }
  ~TDEActionMenuPrivate()
  {
    delete m_popup; m_popup = 0;
  }
  TDEPopupMenu *m_popup;
  bool m_delayed;
  bool m_stickyMenu;
};

TDEActionMenu::TDEActionMenu( TQObject* parent, const char* name )
  : TDEAction( parent, name )
{
  d = new TDEActionMenuPrivate;
  setShortcutConfigurable( false );
}

TDEActionMenu::TDEActionMenu( const TQString& text, TQObject* parent,
                          const char* name )
  : TDEAction( text, 0, parent, name )
{
  d = new TDEActionMenuPrivate;
  setShortcutConfigurable( false );
}

TDEActionMenu::TDEActionMenu( const TQString& text, const TQIconSet& icon,
                          TQObject* parent, const char* name )
  : TDEAction( text, icon, 0, parent, name )
{
  d = new TDEActionMenuPrivate;
  setShortcutConfigurable( false );
}

TDEActionMenu::TDEActionMenu( const TQString& text, const TQString& icon,
                          TQObject* parent, const char* name )
  : TDEAction( text, icon, 0, parent, name )
{
  d = new TDEActionMenuPrivate;
  setShortcutConfigurable( false );
}

TDEActionMenu::~TDEActionMenu()
{
    unplugAll();
    kdDebug(129) << "TDEActionMenu::~TDEActionMenu()" << endl; // ellis
    delete d; d = 0;
}

void TDEActionMenu::popup( const TQPoint& global )
{
  popupMenu()->popup( global );
}

TDEPopupMenu* TDEActionMenu::popupMenu() const
{
  return d->m_popup;
}

void TDEActionMenu::insert( TDEAction* cmd, int index )
{
  if ( cmd )
    cmd->plug( d->m_popup, index );
}

void TDEActionMenu::remove( TDEAction* cmd )
{
  if ( cmd )
    cmd->unplug( d->m_popup );
}

bool TDEActionMenu::delayed() const {
    return d->m_delayed;
}

void TDEActionMenu::setDelayed(bool _delayed) {
    d->m_delayed = _delayed;
}

bool TDEActionMenu::stickyMenu() const {
    return d->m_stickyMenu;
}

void TDEActionMenu::setStickyMenu(bool sticky) {
    d->m_stickyMenu = sticky;
}

int TDEActionMenu::plug( TQWidget* widget, int index )
{
  if (kapp && !kapp->authorizeTDEAction(name()))
    return -1;
  kdDebug(129) << "TDEActionMenu::plug( " << widget << ", " << index << " )" << endl; // remove -- ellis
  if ( ::tqt_cast<TQPopupMenu *>( widget ) )
  {
    TQPopupMenu* menu = static_cast<TQPopupMenu*>( widget );
    int id;
    if ( hasIcon() )
      id = menu->insertItem( iconSet(), text(), d->m_popup, -1, index );
    else
      id = menu->insertItem( text(), d->m_popup, -1, index );

    if ( !isEnabled() )
      menu->setItemEnabled( id, false );

    addContainer( menu, id );
    connect( menu, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( menu, this );

    return containerCount() - 1;
  }
  else if ( ::tqt_cast<TDEToolBar *>( widget ) )
  {
    TDEToolBar *bar = static_cast<TDEToolBar *>( widget );

    int id_ = TDEAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() )
      bar->insertButton( iconSet().pixmap(), id_, TQ_SIGNAL( clicked() ), this,
                         TQ_SLOT( slotActivated() ), isEnabled(), plainText(),
                         index );
    else
    {
      TDEInstance *instance;

      if ( m_parentCollection )
        instance = m_parentCollection->instance();
      else
        instance = TDEGlobal::instance();

      bar->insertButton( icon(), id_, TQ_SIGNAL( clicked() ), this,
                         TQ_SLOT( slotActivated() ), isEnabled(), plainText(),
                         index, instance );
    }

    addContainer( bar, id_ );

    if (!whatsThis().isEmpty())
      TQWhatsThis::add( bar->getButton(id_), whatsThis() );

    connect( bar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    if (delayed()) {
        bar->setDelayedPopup( id_, popupMenu(), stickyMenu() );
    } else {
        bar->getButton(id_)->setPopup(popupMenu(), stickyMenu() );
    }

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( bar, this );

    return containerCount() - 1;
  }
  else if ( ::tqt_cast<TQMenuBar *>( widget ) )
  {
    TQMenuBar *bar = static_cast<TQMenuBar *>( widget );

    int id;

    id = bar->insertItem( text(), popupMenu(), -1, index );

    if ( !isEnabled() )
        bar->setItemEnabled( id, false );

    addContainer( bar, id );
    connect( bar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

////////

TDEToolBarPopupAction::TDEToolBarPopupAction( const TQString& text,
                                          const TQString& icon,
                                          const TDEShortcut& cut,
                                          TQObject* parent, const char* name )
  : TDEAction( text, icon, cut, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

TDEToolBarPopupAction::TDEToolBarPopupAction( const TQString& text,
                                          const TQString& icon,
                                          const TDEShortcut& cut,
                                          const TQObject* receiver,
                                          const char* slot, TQObject* parent,
                                          const char* name )
  : TDEAction( text, icon, cut, receiver, slot, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

TDEToolBarPopupAction::TDEToolBarPopupAction( const KGuiItem& item,
                                          const TDEShortcut& cut,
                                          const TQObject* receiver,
                                          const char* slot, TDEActionCollection* parent,
                                          const char* name )
  : TDEAction( item, cut, receiver, slot, parent, name )
{
  m_popup = 0;
  m_delayed = true;
  m_stickyMenu = true;
}

TDEToolBarPopupAction::~TDEToolBarPopupAction()
{
    delete m_popup;
}

bool TDEToolBarPopupAction::delayed() const {
    return m_delayed;
}

void TDEToolBarPopupAction::setDelayed(bool delayed) {
    m_delayed = delayed;
}

bool TDEToolBarPopupAction::stickyMenu() const {
    return m_stickyMenu;
}

void TDEToolBarPopupAction::setStickyMenu(bool sticky) {
    m_stickyMenu = sticky;
}

int TDEToolBarPopupAction::plug( TQWidget *widget, int index )
{
  if (kapp && !kapp->authorizeTDEAction(name()))
    return -1;
  // This is very related to TDEActionMenu::plug.
  // In fact this class could be an interesting base class for TDEActionMenu
  if ( ::tqt_cast<TDEToolBar *>( widget ) )
  {
    TDEToolBar *bar = (TDEToolBar *)widget;

    int id_ = TDEAction::getToolButtonID();

    if ( icon().isEmpty() && !iconSet().isNull() ) {
        bar->insertButton( iconSet().pixmap(), id_, TQ_SIGNAL( buttonClicked(int, TQt::ButtonState) ), this,
                           TQ_SLOT( slotButtonClicked(int, TQt::ButtonState) ),
                           isEnabled(), plainText(),
                           index );
    } else {
        TDEInstance * instance;
        if ( m_parentCollection )
            instance = m_parentCollection->instance();
        else
            instance = TDEGlobal::instance();

        bar->insertButton( icon(), id_, TQ_SIGNAL( buttonClicked(int, TQt::ButtonState) ), this,
                           TQ_SLOT( slotButtonClicked(int, TQt::ButtonState) ),
                           isEnabled(), plainText(),
                           index, instance );
    }

    addContainer( bar, id_ );

    connect( bar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    if (delayed()) {
        bar->setDelayedPopup( id_, popupMenu(), stickyMenu() );
    } else {
        bar->getButton(id_)->setPopup(popupMenu(), stickyMenu());
    }

    if ( !whatsThis().isEmpty() )
        TQWhatsThis::add( bar->getButton( id_ ), whatsThisWithIcon() );

    return containerCount() - 1;
  }

  return TDEAction::plug( widget, index );
}

TDEPopupMenu *TDEToolBarPopupAction::popupMenu() const
{
    if ( !m_popup ) {
        TDEToolBarPopupAction *that = const_cast<TDEToolBarPopupAction*>(this);
        that->m_popup = new TDEPopupMenu;
    }
    return m_popup;
}

////////

TDEToggleToolBarAction::TDEToggleToolBarAction( const char* toolBarName,
         const TQString& text, TDEActionCollection* parent, const char* name )
  : TDEToggleAction( text, TDEShortcut(), parent, name )
  , m_toolBarName( toolBarName )
  , m_toolBar( 0L )
{
}

TDEToggleToolBarAction::TDEToggleToolBarAction( TDEToolBar *toolBar, const TQString &text,
                                            TDEActionCollection *parent, const char *name )
  : TDEToggleAction( text, TDEShortcut(), parent, name )
  , m_toolBarName( 0 ), m_toolBar( toolBar )
{
}

TDEToggleToolBarAction::~TDEToggleToolBarAction()
{
}

int TDEToggleToolBarAction::plug( TQWidget* w, int index )
{
  if (kapp && !kapp->authorizeTDEAction(name()))
      return -1;

  if ( !m_toolBar ) {
    // Note: topLevelWidget() stops too early, we can't use it.
    TQWidget * tl = w;
    TQWidget * n;
    while ( !tl->isDialog() && ( n = tl->parentWidget() ) ) // lookup parent and store
      tl = n;

    TDEMainWindow * mw = dynamic_cast<TDEMainWindow *>(tl); // try to see if it's a tdemainwindow

    if ( mw )
        m_toolBar = mw->toolBar( m_toolBarName );
  }

  if( m_toolBar ) {
    setChecked( m_toolBar->isVisible() );
    connect( m_toolBar, TQ_SIGNAL(visibilityChanged(bool)), this, TQ_SLOT(setChecked(bool)) );
    // Also emit toggled when the toolbar's visibility changes (see comment in header)
    connect( m_toolBar, TQ_SIGNAL(visibilityChanged(bool)), this, TQ_SIGNAL(toggled(bool)) );
  } else {
    setEnabled( false );
  }

  return TDEToggleAction::plug( w, index );
}

void TDEToggleToolBarAction::setChecked( bool c )
{
  if( m_toolBar && c != m_toolBar->isVisible() ) {
    if( c ) {
      m_toolBar->show();
    } else {
      m_toolBar->hide();
    }
    TQMainWindow* mw = m_toolBar->mainWindow();
    if ( mw && ::tqt_cast<TDEMainWindow *>( mw ) )
      static_cast<TDEMainWindow *>( mw )->setSettingsDirty();
  }
  TDEToggleAction::setChecked( c );
}

////////

TDEToggleFullScreenAction::TDEToggleFullScreenAction( const TDEShortcut &cut,
                             const TQObject* receiver, const char* slot,
                             TQObject* parent, TQWidget* window,
                             const char* name )
  : TDEToggleAction( TQString::null, cut, receiver, slot, parent, name ),
    window( NULL )
{
  setWindow( window );
}

TDEToggleFullScreenAction::~TDEToggleFullScreenAction()
{
}

void TDEToggleFullScreenAction::setWindow( TQWidget* w )
{
  if( window )
    window->removeEventFilter( this );
  window = w;
  if( window )
    window->installEventFilter( this );
}

void TDEToggleFullScreenAction::setChecked( bool c )
{
  if (c)
  {
     setText(i18n("Exit F&ull Screen Mode"));
     setIcon("view-restore");
  }
  else
  {
     setText(i18n("F&ull Screen Mode"));
     setIcon("view-fullscreen");
  }
  TDEToggleAction::setChecked( c );
}

bool TDEToggleFullScreenAction::eventFilter( TQObject* o, TQEvent* e )
{
    if( o == window )
        if( e->type() == TQEvent::WindowStateChange )
            {
            if( window->isFullScreen() != isChecked())
                slotActivated(); // setChecked( window->isFullScreen()) wouldn't emit signals
            }
    return false;
}

////////

KWidgetAction::KWidgetAction( TQWidget* widget,
    const TQString& text, const TDEShortcut& cut,
    const TQObject* receiver, const char* slot,
    TDEActionCollection* parent, const char* name )
  : TDEAction( text, cut, receiver, slot, parent, name )
  , m_widget( widget )
  , m_autoSized( false )
{
  connect( this, TQ_SIGNAL(enabled(bool)), widget, TQ_SLOT(setEnabled(bool)) );
}

KWidgetAction::~KWidgetAction()
{
}

void KWidgetAction::setAutoSized( bool autoSized )
{
  if( m_autoSized == autoSized )
    return;

  m_autoSized = autoSized;

  if( !m_widget || !isPlugged() )
    return;

  TDEToolBar* toolBar = (TDEToolBar*)m_widget->parent();
  int i = findContainer( toolBar );
  if ( i == -1 )
    return;
  int id = itemId( i );

  toolBar->setItemAutoSized( id, m_autoSized );
}

int KWidgetAction::plug( TQWidget* w, int index )
{
  if (kapp && !kapp->authorizeTDEAction(name()))
      return -1;

  if ( !::tqt_cast<TDEToolBar *>( w ) ) {
    kdError() << "KWidgetAction::plug: KWidgetAction must be plugged into TDEToolBar." << endl;
    return -1;
  }
  if ( !m_widget ) {
    kdError() << "KWidgetAction::plug: Widget was deleted or null!" << endl;
    return -1;
  }

  TDEToolBar* toolBar = static_cast<TDEToolBar*>( w );

  int id = TDEAction::getToolButtonID();

  m_widget->reparent( toolBar, TQPoint() );
  toolBar->insertWidget( id, 0, m_widget, index );
  toolBar->setItemAutoSized( id, m_autoSized );

  TQWhatsThis::add( m_widget, whatsThis() );
  addContainer( toolBar, id );

  connect( toolBar, TQ_SIGNAL( toolbarDestroyed() ), this, TQ_SLOT( slotToolbarDestroyed() ) );
  connect( toolBar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

  return containerCount() - 1;
}

void KWidgetAction::unplug( TQWidget *w )
{
  if( !m_widget || !isPlugged() )
    return;

  TDEToolBar* toolBar = (TDEToolBar*)m_widget->parent();
  if ( toolBar == w )
  {
      disconnect( toolBar, TQ_SIGNAL( toolbarDestroyed() ), this, TQ_SLOT( slotToolbarDestroyed() ) );
      m_widget->reparent( 0L, TQPoint(), false /*showIt*/ );
  }
  TDEAction::unplug( w );
}

void KWidgetAction::slotToolbarDestroyed()
{
  //Q_ASSERT( m_widget ); // When exiting the app the widget could be destroyed before the toolbar.
  Q_ASSERT( isPlugged() );
  if( !m_widget || !isPlugged() )
    return;

  // Don't let a toolbar being destroyed, delete my widget.
  m_widget->reparent( 0L, TQPoint(), false /*showIt*/ );
}

////////

TDEActionSeparator::TDEActionSeparator( TQObject *parent, const char *name )
  : TDEAction( parent, name )
{
}

TDEActionSeparator::~TDEActionSeparator()
{
}

int TDEActionSeparator::plug( TQWidget *widget, int index )
{
  if ( ::tqt_cast<TQPopupMenu *>( widget) )
  {
    TQPopupMenu* menu = static_cast<TQPopupMenu*>( widget );

    int id = menu->insertSeparator( index );

    addContainer( menu, id );
    connect( menu, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( ::tqt_cast<TQMenuBar *>( widget ) )
  {
    TQMenuBar *menuBar = static_cast<TQMenuBar *>( widget );

    int id = menuBar->insertSeparator( index );

    addContainer( menuBar, id );

    connect( menuBar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }
  else if ( ::tqt_cast<TDEToolBar *>( widget ) )
  {
    TDEToolBar *toolBar = static_cast<TDEToolBar *>( widget );

    int id = toolBar->insertSeparator( index );

    addContainer( toolBar, id );

    connect( toolBar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    return containerCount() - 1;
  }

  return -1;
}

TDEPasteTextAction::TDEPasteTextAction( const TQString& text,
                            const TQString& icon,
                            const TDEShortcut& cut,
                            const TQObject* receiver,
                            const char* slot, TQObject* parent,
                            const char* name)
  : TDEAction( text, icon, cut, receiver, slot, parent, name )
{
  m_popup = new TDEPopupMenu;
  connect(m_popup, TQ_SIGNAL(aboutToShow()), this, TQ_SLOT(menuAboutToShow()));
  connect(m_popup, TQ_SIGNAL(activated(int)), this, TQ_SLOT(menuItemActivated(int)));
  m_popup->setCheckable(true);
  m_mixedMode = true;
}

TDEPasteTextAction::~TDEPasteTextAction()
{
  delete m_popup;
}

void TDEPasteTextAction::setMixedMode(bool mode)
{
  m_mixedMode = mode;
}

int TDEPasteTextAction::plug( TQWidget *widget, int index )
{
  if (kapp && !kapp->authorizeTDEAction(name()))
    return -1;
  if ( ::tqt_cast<TDEToolBar *>( widget ) )
  {
    TDEToolBar *bar = (TDEToolBar *)widget;

    int id_ = TDEAction::getToolButtonID();

    TDEInstance * instance;
    if ( m_parentCollection )
        instance = m_parentCollection->instance();
    else
        instance = TDEGlobal::instance();

    bar->insertButton( icon(), id_, TQ_SIGNAL( clicked() ), this,
                       TQ_SLOT( slotActivated() ), isEnabled(), plainText(),
                       index, instance );

    addContainer( bar, id_ );

    connect( bar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    bar->setDelayedPopup( id_, m_popup, true );

    if ( !whatsThis().isEmpty() )
        TQWhatsThis::add( bar->getButton( id_ ), whatsThisWithIcon() );

    return containerCount() - 1;
  }

  return TDEAction::plug( widget, index );
}

void TDEPasteTextAction::menuAboutToShow()
{
    m_popup->clear();
    TQStringList list;
    DCOPClient *client = kapp->dcopClient();
    if (client->isAttached() && client->isApplicationRegistered("klipper")) {
      DCOPRef klipper("klipper","klipper");
      DCOPReply reply = klipper.call("getClipboardHistoryMenu");
      if (reply.isValid())
        list = reply;
    }
    TQString clipboardText = tqApp->clipboard()->text(TQClipboard::Clipboard);
    if (list.isEmpty())
        list << clipboardText;
    bool found = false;
    for ( TQStringList::ConstIterator it = list.begin(); it != list.end(); ++it )
    {
      TQString text = KStringHandler::cEmSqueeze((*it).simplifyWhiteSpace(), m_popup->fontMetrics(), 20);
      text.replace("&", "&&");
      int id = m_popup->insertItem(text);
      if (!found && *it == clipboardText)
      {
        m_popup->setItemChecked(id, true);
        found = true;
      }
    }
}

void TDEPasteTextAction::menuItemActivated( int id)
{
    DCOPClient *client = kapp->dcopClient();
    if (client->isAttached() && client->isApplicationRegistered("klipper")) {
      DCOPRef klipper("klipper","klipper");
      DCOPReply reply = klipper.call("getClipboardHistoryItem(int)", m_popup->indexOf(id));
      if (!reply.isValid())
        return;
      TQString clipboardText = reply;
      reply = klipper.call("setClipboardContents(TQString)", clipboardText);
      if (reply.isValid())
        kdDebug(129) << "Clipboard: " << TQString(tqApp->clipboard()->text(TQClipboard::Clipboard)) << endl;
    }
    TQTimer::singleShot(20, this, TQ_SLOT(slotActivated()));
}

void TDEPasteTextAction::slotActivated()
{
  if (!m_mixedMode) {
    TQWidget *w = tqApp->widgetAt(TQCursor::pos(), true);
    TQMimeSource *data = TQApplication::clipboard()->data();
    if (!data->provides("text/plain") && w) {
      m_popup->popup(w->mapToGlobal(TQPoint(0, w->height())));
    } else
      TDEAction::slotActivated();
  } else
    TDEAction::slotActivated();
}


void TDEToggleAction::virtual_hook( int id, void* data )
{ TDEAction::virtual_hook( id, data ); }

void TDERadioAction::virtual_hook( int id, void* data )
{ TDEToggleAction::virtual_hook( id, data ); }

void TDESelectAction::virtual_hook( int id, void* data )
{ TDEAction::virtual_hook( id, data ); }

void TDEListAction::virtual_hook( int id, void* data )
{ TDESelectAction::virtual_hook( id, data ); }

void TDERecentFilesAction::virtual_hook( int id, void* data )
{ TDEListAction::virtual_hook( id, data ); }

void TDEFontAction::virtual_hook( int id, void* data )
{ TDESelectAction::virtual_hook( id, data ); }

void TDEFontSizeAction::virtual_hook( int id, void* data )
{ TDESelectAction::virtual_hook( id, data ); }

void TDEActionMenu::virtual_hook( int id, void* data )
{ TDEAction::virtual_hook( id, data ); }

void TDEToolBarPopupAction::virtual_hook( int id, void* data )
{ TDEAction::virtual_hook( id, data ); }

void TDEToggleToolBarAction::virtual_hook( int id, void* data )
{ TDEToggleAction::virtual_hook( id, data ); }

void TDEToggleFullScreenAction::virtual_hook( int id, void* data )
{ TDEToggleAction::virtual_hook( id, data ); }

void KWidgetAction::virtual_hook( int id, void* data )
{ TDEAction::virtual_hook( id, data ); }

void TDEActionSeparator::virtual_hook( int id, void* data )
{ TDEAction::virtual_hook( id, data ); }

void TDEPasteTextAction::virtual_hook( int id, void* data )
{ TDEAction::virtual_hook( id, data ); }

#include "tdeactionclasses.moc"
