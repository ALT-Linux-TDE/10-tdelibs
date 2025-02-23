/* This file is part of the KDE libraries
    Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>
              (C) 1999 Simon Hausmann <hausmann@kde.org>
              (C) 2000 Nicolas Hadacek <haadcek@kde.org>
              (C) 2000 Kurt Granroth <granroth@kde.org>
              (C) 2000 Michael Koch <koch@kde.org>
              (C) 2001 Holger Freyther <freyther@kde.org>
              (C) 2002 Ellis Whitehead <ellis@kde.org>
              (C) 2002 Joseph Wenninger <jowenn@kde.org>

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

#include "tdeaction.h"

#include <assert.h>

#include <tqtooltip.h>
#include <tqwhatsthis.h>

#include <tdeaccel.h>
#include <tdeaccelbase.h>
#include <tdeaccelprivate.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <kguiitem.h>
#include <tdemainwindow.h>
#include <tdemenubar.h>
#include <tdepopupmenu.h>
#include <tdetoolbar.h>
#include <tdetoolbarbutton.h>

#include <ft2build.h>
#include <X11/Xdefs.h>
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/Shell.h>

#include <X11/Xft/Xft.h>

/**
* How it works.
* TDEActionCollection is an organizing container for TDEActions.
* TDEActionCollection keeps track of the information necessary to handle
* configuration and shortcuts.
*
* Focus Widget pointer:
* This is the widget which is the focus for action shortcuts.
* It is set either by passing a TQWidget* to the TDEActionCollection constructor
* or by calling setWidget() if the widget wasn't known when the object was
* initially constructed (as in KXMLGUIClient and KParts::PartBase)
*
* Shortcuts:
* An action's shortcut will not not be connected unless a focus widget has
* been specified in TDEActionCollection.
*
* XML Filename:
* This is used to save user-modified settings back to the *ui.rc file.
* It is set by KXMLGUIFactory.
*/

int TDEAction::getToolButtonID()
{
    static int toolbutton_no = -2;
    return toolbutton_no--;
}

//---------------------------------------------------------------------
// TDEAction::TDEActionPrivate
//---------------------------------------------------------------------

class TDEAction::TDEActionPrivate : public KGuiItem
{
public:
  TDEActionPrivate() : KGuiItem()
  {
    m_tdeaccel = 0;
    m_configurable = true;
  }

  TDEAccel *m_tdeaccel;
  TQValueList<TDEAccel*> m_tdeaccelList;

  TQString m_groupText;
  TQString m_group;

  TDEShortcut m_cut;
  TDEShortcut m_cutDefault;

  bool m_configurable;

  struct Container
  {
    Container() { m_container = 0; m_representative = 0; m_id = 0; }
    Container( const Container& s ) { m_container = s.m_container;
                                      m_id = s.m_id; m_representative = s.m_representative; }
    TQWidget* m_container;
    int m_id;
    TQWidget* m_representative;
  };

  TQValueList<Container> m_containers;
};

//---------------------------------------------------------------------
// TDEAction
//---------------------------------------------------------------------

TDEAction::TDEAction( const TQString& text, const TDEShortcut& cut,
             const TQObject* receiver, const char* slot,
             TDEActionCollection* parent, const char* name )
: TQObject( parent, name ), d(new TDEActionPrivate)
{
	initPrivate( text, cut, receiver, slot );
}

TDEAction::TDEAction( const TQString& text, const TQString& sIconName, const TDEShortcut& cut,
	const TQObject* receiver, const char* slot,
	TDEActionCollection* parent, const char* name )
: TQObject( parent, name ), d(new TDEActionPrivate)
{
	initPrivate( text, cut, receiver, slot );
	d->setIconName( sIconName );
}

TDEAction::TDEAction( const TQString& text, const TQIconSet& pix, const TDEShortcut& cut,
	const TQObject* receiver, const char* slot,
	TDEActionCollection* parent, const char* name )
: TQObject( parent, name ), d(new TDEActionPrivate)
{
	initPrivate( text, cut, receiver, slot );
	d->setIconSet( pix );
}

TDEAction::TDEAction( const KGuiItem& item, const TDEShortcut& cut,
	const TQObject* receiver, const char* slot,
	TDEActionCollection* parent, const char* name )
: TQObject( parent, name ), d(new TDEActionPrivate)
{
	initPrivate( item.text(), cut, receiver, slot );
	if( item.hasIcon() )
		setIcon( item.iconName() );
	setToolTip( item.toolTip() );
	setWhatsThis( item.whatsThis() );
}

#ifndef KDE_NO_COMPAT // KDE 4: remove
TDEAction::TDEAction( const TQString& text, const TDEShortcut& cut,
                  TQObject* parent, const char* name )
 : TQObject( parent, name ), d(new TDEActionPrivate)
{
    initPrivate( text, cut, 0, 0 );
}

TDEAction::TDEAction( const TQString& text, const TDEShortcut& cut,
                  const TQObject* receiver,
                  const char* slot, TQObject* parent, const char* name )
 : TQObject( parent, name ), d(new TDEActionPrivate)
{
    initPrivate( text, cut, receiver, slot );
}

TDEAction::TDEAction( const TQString& text, const TQIconSet& pix,
                  const TDEShortcut& cut,
                  TQObject* parent, const char* name )
 : TQObject( parent, name ), d(new TDEActionPrivate)
{
    initPrivate( text, cut, 0, 0 );
    setIconSet( pix );
}

TDEAction::TDEAction( const TQString& text, const TQString& pix,
                  const TDEShortcut& cut,
                  TQObject* parent, const char* name )
: TQObject( parent, name ), d(new TDEActionPrivate)
{
    initPrivate( text, cut, 0, 0 );
    d->setIconName( pix );
}

TDEAction::TDEAction( const TQString& text, const TQIconSet& pix,
                  const TDEShortcut& cut,
                  const TQObject* receiver, const char* slot, TQObject* parent,
                  const char* name )
 : TQObject( parent, name ), d(new TDEActionPrivate)
{
    initPrivate( text, cut, receiver, slot );
    setIconSet( pix );
}

TDEAction::TDEAction( const TQString& text, const TQString& pix,
                  const TDEShortcut& cut,
                  const TQObject* receiver, const char* slot, TQObject* parent,
                  const char* name )
  : TQObject( parent, name ), d(new TDEActionPrivate)
{
    initPrivate( text, cut, receiver, slot );
    d->setIconName(pix);
}

TDEAction::TDEAction( TQObject* parent, const char* name )
 : TQObject( parent, name ), d(new TDEActionPrivate)
{
    initPrivate( TQString::null, TDEShortcut(), 0, 0 );
}
#endif // KDE 4: remove end

TDEAction::~TDEAction()
{
    kdDebug(129) << "TDEAction::~TDEAction( this = \"" << name() << "\" )" << endl; // -- ellis
#ifndef KDE_NO_COMPAT
     if (d->m_tdeaccel)
       unplugAccel();
#endif

    // If actionCollection hasn't already been destructed,
    if ( m_parentCollection ) {
        m_parentCollection->take( this );

        const TQValueList<TDEAccel*> & accelList = d->m_tdeaccelList;
        TQValueList<TDEAccel*>::const_iterator itr = accelList.constBegin();
        const TQValueList<TDEAccel*>::const_iterator itrEnd = accelList.constEnd();

        const char * const namePtr = name();
        for (; itr != itrEnd; ++itr )
            (*itr)->remove(namePtr);

    }

    // Do not call unplugAll from here, as tempting as it sounds.
    // TDEAction is designed around the idea that you need to plug
    // _and_ to unplug it "manually". Unplugging leads to an important
    // slowdown when e.g. closing the window, in which case we simply
    // want to destroy everything asap, not to remove actions one by one
    // from the GUI.

    delete d;
}

void TDEAction::initPrivate( const TQString& text, const TDEShortcut& cut,
                  const TQObject* receiver, const char* slot )
{
    d->m_cutDefault = cut;

    m_parentCollection = dynamic_cast<TDEActionCollection *>( parent() );
    kdDebug(129) << "TDEAction::initPrivate(): this = " << this << " name = \"" << name() << "\" cut = " << cut.toStringInternal() << " m_parentCollection = " << m_parentCollection << endl;
    if ( m_parentCollection )
        m_parentCollection->insert( this );

    if ( receiver && slot )
        connect( this, TQ_SIGNAL( activated() ), receiver, slot );

    if( !cut.isNull() && !qstrcmp( name(), "unnamed" ) )
        kdWarning(129) << "TDEAction::initPrivate(): trying to assign a shortcut (" << cut.toStringInternal() << ") to an unnamed action." << endl;
    d->setText( text );
    initShortcut( cut );
}

bool TDEAction::isPlugged() const
{
  return (!d->m_containers.empty()) || d->m_tdeaccel;
}

bool TDEAction::isPlugged( const TQWidget *container ) const
{
  return findContainer( container ) > -1;
}

bool TDEAction::isPlugged( const TQWidget *container, int id ) const
{
  int i = findContainer( container );
  return ( i > -1 && itemId( i ) == id );
}

bool TDEAction::isPlugged( const TQWidget *container, const TQWidget *_representative ) const
{
  int i = findContainer( container );
  return ( i > -1 && representative( i ) == _representative );
}


/*
Three actionCollection conditions:
	1) Scope is known on creation and TDEAccel object is created (e.g. TDEMainWindow)
	2) Scope is unknown and no TDEAccel object is available (e.g. KXMLGUIClient)
		a) addClient() will be called on object
		b) we just want to add the actions to another KXMLGUIClient object

The question is how to do we incorporate #2b into the XMLGUI framework?


We have a KCommandHistory object with undo and redo actions in a passed actionCollection
We have a KoDoc object which holds a KCommandHistory object and the actionCollection
We have two KoView objects which both point to the same KoDoc object
Undo and Redo should be available in both KoView objects, and
	calling the undo->setEnabled() should affect both KoViews

When addClient is called, it needs to be able to find the undo and redo actions
When it calls plug() on them, they need to be inserted into the TDEAccel object of the appropriate KoView

In this case, the actionCollection belongs to KoDoc and we need to let it know that its shortcuts
have the same scope as the KoView actionCollection

KXMLGUIClient::addSubActionCollection

Document:
	create document actions

View
	create view actions
	add document actionCollection as sub-collection

A parentCollection is created
Scenario 1: parentCollection has a focus widget set (e.g. via TDEMainWindow)
	A TDEAccel object is created in the parentCollection
	A TDEAction is created with parent=parentCollection
	The shortcut is inserted into this actionCollection
	Scenario 1a: xml isn't used
		done
	Scenario 1b: KXMLGUIBuilder::addClient() called
		setWidget is called -- ignore
		shortcuts are set
Scenario 2: parentCollection has no focus widget (e.g., KParts)
	A TDEAction is created with parent=parentCollection
	Scenario 2a: xml isn't used
		no shortcuts
	Scenario 2b: KXMLGUIBuilder::addClient() called
		setWidget is called
		shortcuts are inserted into current TDEAccel
		shortcuts are set in all other TDEAccels, if the action is present in the other TDEAccels
*/

/*
shortcut may be set:
	- on construction
	- on plug
	- on reading XML
	- on plugAccel (deprecated)

On Construction: [via initShortcut()]
	insert into TDEAccel of m_parentCollection,
		if tdeaccel() && isAutoConnectShortcuts() exists

On Plug: [via plug() -> plugShortcut()]
	insert into TDEAccel of m_parentCollection, if exists and not already inserted into

On Read XML: [via setShortcut()]
	set in all current TDEAccels
	insert into TDEAccel of m_parentCollection, if exists and not already inserted into
*/

TDEAccel* TDEAction::tdeaccelCurrent()
{
  if( m_parentCollection && m_parentCollection->builderTDEAccel() )
    return m_parentCollection->builderTDEAccel();
  else if( m_parentCollection && m_parentCollection->tdeaccel() )
    return m_parentCollection->tdeaccel();
  else
    return 0L;
}

// Only to be called from initPrivate()
bool TDEAction::initShortcut( const TDEShortcut& cut )
{
    d->m_cut = cut;

    // Only insert action into TDEAccel if it has a valid name,
    if( qstrcmp( name(), "unnamed" ) &&
        m_parentCollection &&
        m_parentCollection->isAutoConnectShortcuts() &&
        m_parentCollection->tdeaccel() )
    {
        insertTDEAccel( m_parentCollection->tdeaccel() );
        return true;
    }
    return false;
 }

// Only to be called from plug()
void TDEAction::plugShortcut()
{
  TDEAccel* const tdeaccel = tdeaccelCurrent();

  //kdDebug(129) << "TDEAction::plugShortcut(): this = " << this << " tdeaccel() = " << (m_parentCollection ? m_parentCollection->tdeaccel() : 0) << endl;
  if( tdeaccel && qstrcmp( name(), "unnamed" ) ) {
    // Check if already plugged into current TDEAccel object
    const TQValueList<TDEAccel*> & accelList = d->m_tdeaccelList;
    TQValueList<TDEAccel*>::const_iterator itr = accelList.constBegin();
    const TQValueList<TDEAccel*>::const_iterator itrEnd = accelList.constEnd();

    for( ; itr != itrEnd; ++itr) {
      if( (*itr) == tdeaccel )
        return;
    }

    insertTDEAccel( tdeaccel );
  }
}

bool TDEAction::setShortcut( const TDEShortcut& cut )
{
  bool bChanged = (d->m_cut != cut);
  d->m_cut = cut;

  TDEAccel* const tdeaccel = tdeaccelCurrent();
  bool bInsertRequired = true;
  // Apply new shortcut to all existing TDEAccel objects

  const TQValueList<TDEAccel*> & accelList = d->m_tdeaccelList;
  TQValueList<TDEAccel*>::const_iterator itr = accelList.constBegin();
  const TQValueList<TDEAccel*>::const_iterator itrEnd = accelList.constEnd();

  for( ; itr != itrEnd; ++itr) {
    // Check whether shortcut has already been plugged into
    //  the current tdeaccel object.
    if( (*itr) == tdeaccel )
      bInsertRequired = false;
    if( bChanged )
      updateTDEAccelShortcut( *itr );
  }

  // Only insert action into TDEAccel if it has a valid name,
  if( tdeaccel && bInsertRequired && qstrcmp( name(), "unnamed" ) )
    insertTDEAccel( tdeaccel );

  if( bChanged ) {
#ifndef KDE_NO_COMPAT    // KDE 4: remove
    if ( d->m_tdeaccel )
      d->m_tdeaccel->setShortcut( name(), cut );
#endif    // KDE 4: remove end
      int len = containerCount();
      for( int i = 0; i < len; ++i )
          updateShortcut( i );
  }
  return true;
}

bool TDEAction::updateTDEAccelShortcut( TDEAccel* tdeaccel )
{
  // Check if action is permitted
  if (kapp && !kapp->authorizeTDEAction(name()))
    return false;

  bool b = true;

  if ( !tdeaccel->actions().actionPtr( name() ) ) {
    if(!d->m_cut.isNull() ) {
      kdDebug(129) << "Inserting " << name() << ", " << d->text() << ", " << d->plainText() << endl;
      b = tdeaccel->insert( name(), d->plainText(), TQString::null,
          d->m_cut,
          this, TQ_SLOT(slotActivated()),
          isShortcutConfigurable(), isEnabled() );
    }
  }
  else
    b = tdeaccel->setShortcut( name(), d->m_cut );

  return b;
}

void TDEAction::insertTDEAccel( TDEAccel* tdeaccel )
{
  //kdDebug(129) << "TDEAction::insertTDEAccel( " << tdeaccel << " ): this = " << this << endl;
  if ( !tdeaccel->actions().actionPtr( name() ) ) {
    if( updateTDEAccelShortcut( tdeaccel ) ) {
      d->m_tdeaccelList.append( tdeaccel );
      connect( tdeaccel, TQ_SIGNAL(destroyed()), this, TQ_SLOT(slotDestroyed()) );
    }
  }
  else
    kdWarning(129) << "TDEAction::insertTDEAccel( tdeaccel = " << tdeaccel << " ): TDEAccel object already contains an action name \"" << name() << "\"" << endl; // -- ellis
}

void TDEAction::removeTDEAccel( TDEAccel* tdeaccel )
{
  //kdDebug(129) << "TDEAction::removeTDEAccel( " << i << " ): this = " << this << endl;
  TQValueList<TDEAccel*> & accelList = d->m_tdeaccelList;
  TQValueList<TDEAccel*>::iterator itr = accelList.begin();
  const TQValueList<TDEAccel*>::iterator itrEnd = accelList.end();

  for( ; itr != itrEnd; ++itr) {
    if( (*itr) == tdeaccel ) {
      tdeaccel->remove( name() );
      accelList.remove( itr );
      disconnect( tdeaccel, TQ_SIGNAL(destroyed()), this, TQ_SLOT(slotDestroyed()) );
      break;
    }
  }
}

#ifndef KDE_NO_COMPAT
// KDE 4: remove
void TDEAction::setAccel( int keyQt )
{
  setShortcut( TDEShortcut(keyQt) );
}
#endif // KDE 4: remove end

void TDEAction::updateShortcut( int i )
{
  int id = itemId( i );

  TQWidget* w = container( i );
  if ( ::tqt_cast<TQPopupMenu *>( w ) ) {
    TQPopupMenu* menu = static_cast<TQPopupMenu*>(w);
    updateShortcut( menu, id );
  }
  else if ( ::tqt_cast<TQMenuBar *>( w ) )
    static_cast<TQMenuBar*>(w)->setAccel( d->m_cut.keyCodeQt(), id );
}

void TDEAction::updateShortcut( TQPopupMenu* menu, int id )
{
  //kdDebug(129) << "TDEAction::updateShortcut(): this = " << this << " d->m_tdeaccelList.count() = " << d->m_tdeaccelList.count() << endl;
  // If the action has a TDEAccel object,
  //  show the string representation of its shortcut.
  if ( d->m_tdeaccel || d->m_tdeaccelList.count() ) {
    TQString s = menu->text( id );
    int i = s.find( '\t' );
    if ( i >= 0 )
      s.replace( i+1, s.length()-i, d->m_cut.seq(0).toString() );
    else
      s += "\t" + d->m_cut.seq(0).toString();

    menu->changeItem( id, s );
  }
  // Otherwise insert the shortcut itself into the popup menu.
  else {
    // This is a fall-hack in case the TDEAction is missing a proper parent collection.
    //  It should be removed eventually. --ellis
    menu->setAccel( d->m_cut.keyCodeQt(), id );
    kdDebug(129) << "TDEAction::updateShortcut(): name = \"" << name() << "\", cut = " << d->m_cut.toStringInternal() << "; No TDEAccel, probably missing a parent collection." << endl;
  }
}

const TDEShortcut& TDEAction::shortcut() const
{
  return d->m_cut;
}

const TDEShortcut& TDEAction::shortcutDefault() const
{
  return d->m_cutDefault;
}

TQString TDEAction::shortcutText() const
{
  return d->m_cut.toStringInternal();
}

void TDEAction::setShortcutText( const TQString& s )
{
  setShortcut( TDEShortcut(s) );
}

#ifndef KDE_NO_COMPAT // Remove in KDE 4
int TDEAction::accel() const
{
  return d->m_cut.keyCodeQt();
}
#endif

void TDEAction::setGroup( const TQString& grp )
{
  d->m_group = grp;

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateGroup( i );
}

void TDEAction::updateGroup( int )
{
  // DO SOMETHING
}

TQString TDEAction::group() const
{
  return d->m_group;
}

bool TDEAction::isEnabled() const
{
  return d->isEnabled();
}

bool TDEAction::isShortcutConfigurable() const
{
  return d->m_configurable;
}

void TDEAction::setToolTip( const TQString& tt )
{
  d->setToolTip( tt );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateToolTip( i );
}

void TDEAction::updateToolTip( int i )
{
  TQWidget *w = container( i );

  if ( ::tqt_cast<TDEToolBar *>( w ) )
    TQToolTip::add( static_cast<TDEToolBar*>(w)->getWidget( itemId( i ) ), d->toolTip() );
}

TQString TDEAction::toolTip() const
{
  return d->toolTip();
}

int TDEAction::plug( TQWidget *w, int index )
{
  //kdDebug(129) << "TDEAction::plug( " << w << ", " << index << " )" << endl;
  if (!w ) {
	kdWarning(129) << "TDEAction::plug called with 0 argument\n";
 	return -1;
  }

  // Ellis: print warning if there is a shortcut, but no TDEAccel available (often due to no widget available in the actioncollection)
  // David: Well, it doesn't matter much, things still work (e.g. Undo in koffice) via TQAccel.
  // We should probably re-enable the warning for things that only TDEAccel can do, though - e.g. WIN key (mapped to Meta).
#if 0 //ndef NDEBUG
  TDEAccel* tdeaccel = tdeaccelCurrent();
  if( !d->m_cut.isNull() && !tdeaccel ) {
    kdDebug(129) << "TDEAction::plug(): has no TDEAccel object; this = " << this << " name = " << name() << " parentCollection = " << m_parentCollection << endl; // ellis
  }
#endif

  // Check if action is permitted
  if (kapp && !kapp->authorizeTDEAction(name()))
    return -1;

  plugShortcut();

  if ( ::tqt_cast<TQPopupMenu *>( w ) )
  {
    TQPopupMenu* menu = static_cast<TQPopupMenu*>( w );
    int id;
    // Don't insert shortcut into menu if it's already in a TDEAccel object.
    int keyQt = (d->m_tdeaccelList.count() || d->m_tdeaccel) ? 0 : d->m_cut.keyCodeQt();

    if ( d->hasIcon() )
    {
        TDEInstance *instance;
        if ( m_parentCollection )
          instance = m_parentCollection->instance();
        else
          instance = TDEGlobal::instance();
        id = menu->insertItem( d->iconSet( TDEIcon::Small, 0, instance ), d->text(), this,//dsweet
                                 TQ_SLOT( slotPopupActivated() ), keyQt,
                                 -1, index );
    }
    else
        id = menu->insertItem( d->text(), this,
                               TQ_SLOT( slotPopupActivated() ),
                               keyQt, -1, index );

    // If the shortcut is already in a TDEAccel object, then
    //  we need to set the menu item's shortcut text.
    if ( d->m_tdeaccelList.count() || d->m_tdeaccel )
        updateShortcut( menu, id );

    // call setItemEnabled only if the item really should be disabled,
    // because that method is slow and the item is per default enabled
    if ( !d->isEnabled() )
        menu->setItemEnabled( id, false );

    if ( !d->whatsThis().isEmpty() )
        menu->TQMenuData::setWhatsThis( id, whatsThisWithIcon() );

    addContainer( menu, id );
    connect( menu, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( menu, this );

    return d->m_containers.count() - 1;
  }
  else if ( ::tqt_cast<TDEToolBar *>( w ) )
  {
    TDEToolBar *bar = static_cast<TDEToolBar *>( w );

    int id_ = getToolButtonID();
    TDEInstance *instance;
    if ( m_parentCollection )
      instance = m_parentCollection->instance();
    else
      instance = TDEGlobal::instance();

    if ( icon().isEmpty() && !iconSet().pixmap().isNull() ) // old code using TQIconSet directly
    {
        bar->insertButton( iconSet().pixmap(), id_, TQ_SIGNAL( buttonClicked(int, TQt::ButtonState) ), this,
                           TQ_SLOT( slotButtonClicked(int, TQt::ButtonState) ),
                           d->isEnabled(), d->plainText(), index );
    }
    else
    {
        TQString icon = d->iconName();
        if ( icon.isEmpty() )
            icon = "unknown";
        bar->insertButton( icon, id_, TQ_SIGNAL( buttonClicked(int, TQt::ButtonState) ), this,
                           TQ_SLOT( slotButtonClicked(int, TQt::ButtonState) ),
                           d->isEnabled(), d->plainText(), index, instance );
    }

    TDEToolBarButton* ktb = bar->getButton(id_);
    ktb->setName( TQCString("toolbutton_")+name() );

    if ( !d->whatsThis().isEmpty() )
        TQWhatsThis::add( bar->getButton(id_), whatsThisWithIcon() );

    if ( !d->toolTip().isEmpty() )
      TQToolTip::add( bar->getButton(id_), d->toolTip() );

    addContainer( bar, id_ );

    connect( bar, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotDestroyed() ) );

    if ( m_parentCollection )
      m_parentCollection->connectHighlight( bar, this );

    return containerCount() - 1;
  }

  return -1;
}

void TDEAction::unplug( TQWidget *w )
{
  int i = findContainer( w );
  if ( i == -1 )
    return;
  int id = itemId( i );

  if ( ::tqt_cast<TQPopupMenu *>( w ) )
  {
    TQPopupMenu *menu = static_cast<TQPopupMenu *>( w );
    menu->removeItem( id );
  }
  else if ( ::tqt_cast<TDEToolBar *>( w ) )
  {
    TDEToolBar *bar = static_cast<TDEToolBar *>( w );
    bar->removeItemDelayed( id );
  }
  else if ( ::tqt_cast<TQMenuBar *>( w ) )
  {
    TQMenuBar *bar = static_cast<TQMenuBar *>( w );
    bar->removeItem( id );
  }

  removeContainer( i );
  if ( m_parentCollection )
    m_parentCollection->disconnectHighlight( w, this );
}

void TDEAction::plugAccel(TDEAccel *kacc, bool configurable)
{
#if 0 //ndef NDEBUG
  kdWarning(129) << "TDEAction::plugAccel(): call to deprecated action." << endl;
  kdDebug(129) << kdBacktrace() << endl;
  //kdDebug(129) << "TDEAction::plugAccel( kacc = " << kacc << " ): name \"" << name() << "\"" << endl;
#endif
  if ( d->m_tdeaccel )
    unplugAccel();

  // If the parent collection's accel ptr isn't set yet
  //if ( m_parentCollection && !m_parentCollection->accel() )
  //  m_parentCollection->setAccel( kacc );

  // We can only plug this action into the given TDEAccel object
  //  if it does not already contain an action with the same name.
  if ( !kacc->actions().actionPtr(name()) )
  {
    d->m_tdeaccel = kacc;
    d->m_tdeaccel->insert(name(), d->plainText(), TQString::null,
        TDEShortcut(d->m_cut),
        this, TQ_SLOT(slotActivated()),
        configurable, isEnabled());
    connect(d->m_tdeaccel, TQ_SIGNAL(destroyed()), this, TQ_SLOT(slotDestroyed()));
    //connect(d->m_tdeaccel, TQ_SIGNAL(keycodeChanged()), this, TQ_SLOT(slotKeycodeChanged()));
  }
  else
  {
#if 0 //ndef NDEBUG
    kdWarning(129) << "TDEAction::plugAccel( kacc = " << kacc << " ): TDEAccel object already contains an action name \"" << name() << "\"" << endl; // -- ellis
#endif
  }
}

void TDEAction::unplugAccel()
{
  //kdDebug(129) << "TDEAction::unplugAccel() " << this << " " << name() << endl;
  if ( d->m_tdeaccel )
  {
    d->m_tdeaccel->remove(name());
    d->m_tdeaccel = 0;
  }
}

void TDEAction::plugMainWindowAccel( TQWidget *w )
{
  // Note: topLevelWidget() stops too early, we can't use it.
  TQWidget * tl = w;
  TQWidget * n;
  while ( !tl->isDialog() && ( n = tl->parentWidget() ) ) // lookup parent and store
    tl = n;

  TDEMainWindow * mw = dynamic_cast<TDEMainWindow *>(tl); // try to see if it's a tdemainwindow
  if (mw)
    plugAccel( mw->accel() );
  else
    kdDebug(129) << "TDEAction::plugMainWindowAccel: Toplevel widget isn't a TDEMainWindow, can't plug accel. " << tl << endl;
}

void TDEAction::setEnabled(bool enable)
{
  //kdDebug(129) << "TDEAction::setEnabled( " << enable << " ): this = " << this << " d->m_tdeaccelList.count() = " << d->m_tdeaccelList.count() << endl;
  if ( enable == d->isEnabled() )
    return;

#ifndef KDE_NO_COMPAT
  // KDE 4: remove
  if (d->m_tdeaccel)
    d->m_tdeaccel->setEnabled(name(), enable);
#endif  // KDE 4: remove end

  const TQValueList<TDEAccel*> & accelList = d->m_tdeaccelList;
  TQValueList<TDEAccel*>::const_iterator itr = accelList.constBegin();
  const TQValueList<TDEAccel*>::const_iterator itrEnd = accelList.constEnd();

  const char * const namePtr = name();

  for ( ; itr != itrEnd; ++itr )
    (*itr)->setEnabled( namePtr, enable );

  d->setEnabled( enable );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateEnabled( i );

  emit enabled( d->isEnabled() );
}

void TDEAction::updateEnabled( int i )
{
    TQWidget *w = container( i );

    if ( ::tqt_cast<TQPopupMenu *>( w ) )
      static_cast<TQPopupMenu*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
    else if ( ::tqt_cast<TQMenuBar *>( w ) )
      static_cast<TQMenuBar*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
    else if ( ::tqt_cast<TDEToolBar *>( w ) )
      static_cast<TDEToolBar*>(w)->setItemEnabled( itemId( i ), d->isEnabled() );
}

void TDEAction::setShortcutConfigurable( bool b )
{
    d->m_configurable = b;
}

void TDEAction::setText( const TQString& text )
{
#ifndef KDE_NO_COMPAT
  // KDE 4: remove
  if (d->m_tdeaccel) {
    TDEAccelAction* pAction = d->m_tdeaccel->actions().actionPtr(name());
    if (pAction)
      pAction->setLabel( text );
  }
#endif  // KDE 4: remove end
  const TQValueList<TDEAccel*> & accelList = d->m_tdeaccelList;
  TQValueList<TDEAccel*>::const_iterator itr = accelList.constBegin();
  const TQValueList<TDEAccel*>::const_iterator itrEnd = accelList.constEnd();

  const char * const namePtr = name();

  for( ; itr != itrEnd; ++itr ) {
    TDEAccelAction* const pAction = (*itr)->actions().actionPtr(namePtr);
    if (pAction)
      pAction->setLabel( text );
  }

  d->setText( text );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateText( i );
}

void TDEAction::updateText( int i )
{
  TQWidget *w = container( i );

  if ( ::tqt_cast<TQPopupMenu *>( w ) ) {
    int id = itemId( i );
    static_cast<TQPopupMenu*>(w)->changeItem( id, d->text() );
    if (!d->m_cut.isNull())
      updateShortcut( static_cast<TQPopupMenu*>(w), id );
  }
  else if ( ::tqt_cast<TQMenuBar *>( w ) )
    static_cast<TQMenuBar*>(w)->changeItem( itemId( i ), d->text() );
  else if ( ::tqt_cast<TDEToolBar *>( w ) )
  {
    TQWidget *button = static_cast<TDEToolBar *>(w)->getWidget( itemId( i ) );
    if ( ::tqt_cast<TDEToolBarButton *>( button ) )
      static_cast<TDEToolBarButton *>(button)->setText( d->plainText() );
  }
}

TQString TDEAction::text() const
{
  return d->text();
}

TQString TDEAction::plainText() const
{
  return d->plainText( );
}

void TDEAction::setIcon( const TQString &icon )
{
  d->setIconName( icon );

  // now handle any toolbars
  int len = containerCount();
  for ( int i = 0; i < len; ++i )
    updateIcon( i );
}

void TDEAction::updateIcon( int id )
{
  TQWidget* w = container( id );

  if ( ::tqt_cast<TQPopupMenu *>( w ) ) {
    int itemId_ = itemId( id );
    static_cast<TQPopupMenu*>(w)->changeItem( itemId_, d->iconSet( TDEIcon::Small ), d->text() );
    if (!d->m_cut.isNull())
      updateShortcut( static_cast<TQPopupMenu*>(w), itemId_ );
  }
  else if ( ::tqt_cast<TQMenuBar *>( w ) )
    static_cast<TQMenuBar*>(w)->changeItem( itemId( id ), d->iconSet( TDEIcon::Small ), d->text() );
  else if ( ::tqt_cast<TDEToolBar *>( w ) )
    static_cast<TDEToolBar *>(w)->setButtonIcon( itemId( id ), d->iconName() );
}

TQString TDEAction::icon() const
{
  return d->iconName( );
}

void TDEAction::setIconSet( const TQIconSet &iconset )
{
  d->setIconSet( iconset );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateIconSet( i );
}


void TDEAction::updateIconSet( int id )
{
  TQWidget *w = container( id );

  if ( ::tqt_cast<TQPopupMenu *>( w ) )
  {
    int itemId_ = itemId( id );
    static_cast<TQPopupMenu*>(w)->changeItem( itemId_, d->iconSet(), d->text() );
    if (!d->m_cut.isNull())
      updateShortcut( static_cast<TQPopupMenu*>(w), itemId_ );
  }
  else if ( ::tqt_cast<TQMenuBar *>( w ) )
    static_cast<TQMenuBar*>(w)->changeItem( itemId( id ), d->iconSet(), d->text() );
  else if ( ::tqt_cast<TDEToolBar *>( w ) )
  {
    if ( icon().isEmpty() && d->hasIcon() ) // only if there is no named icon ( scales better )
      static_cast<TDEToolBar *>(w)->setButtonIconSet( itemId( id ), d->iconSet() );
    else
      static_cast<TDEToolBar *>(w)->setButtonIconSet( itemId( id ), d->iconSet( TDEIcon::Small ) );
  }
}

TQIconSet TDEAction::iconSet( TDEIcon::Group group, int size ) const
{
    return d->iconSet( group, size );
}

bool TDEAction::hasIcon() const
{
  return d->hasIcon();
}

void TDEAction::setWhatsThis( const TQString& text )
{
  d->setWhatsThis(  text );

  int len = containerCount();
  for( int i = 0; i < len; ++i )
    updateWhatsThis( i );
}

void TDEAction::updateWhatsThis( int i )
{
  TQPopupMenu* pm = popupMenu( i );
  if ( pm )
  {
    pm->TQMenuData::setWhatsThis( itemId( i ), d->whatsThis() );
    return;
  }

  TDEToolBar *tb = toolBar( i );
  if ( tb )
  {
    TQWidget *w = tb->getButton( itemId( i ) );
    TQWhatsThis::remove( w );
    TQWhatsThis::add( w, d->whatsThis() );
    return;
  }
}

TQString TDEAction::whatsThis() const
{
  return d->whatsThis();
}

TQString TDEAction::whatsThisWithIcon() const
{
    TQString text = whatsThis();
    if (!d->iconName().isEmpty())
      return TQString::fromLatin1("<img source=\"small|%1\"> %2").arg(d->iconName() ).arg(text);
    return text;
}

TQWidget* TDEAction::container( int index ) const
{
  assert( index < containerCount() );
  return d->m_containers[ index ].m_container;
}

TDEToolBar* TDEAction::toolBar( int index ) const
{
    return dynamic_cast<TDEToolBar *>( d->m_containers[ index ].m_container );
}

TQPopupMenu* TDEAction::popupMenu( int index ) const
{
    return dynamic_cast<TQPopupMenu *>( d->m_containers[ index ].m_container );
}

TQWidget* TDEAction::representative( int index ) const
{
  return d->m_containers[ index ].m_representative;
}

int TDEAction::itemId( int index ) const
{
  return d->m_containers[ index ].m_id;
}

int TDEAction::containerCount() const
{
  return d->m_containers.count();
}

uint TDEAction::tdeaccelCount() const
{
  return d->m_tdeaccelList.count();
}

void TDEAction::addContainer( TQWidget* c, int id )
{
  TDEActionPrivate::Container p;
  p.m_container = c;
  p.m_id = id;
  d->m_containers.append( p );
}

void TDEAction::addContainer( TQWidget* c, TQWidget* w )
{
  TDEActionPrivate::Container p;
  p.m_container = c;
  p.m_representative = w;
  d->m_containers.append( p );
}

void TDEAction::activate()
{
  emit activated( TDEAction::EmulatedActivation, TQt::NoButton );
  slotActivated();
}

void TDEAction::slotActivated()
{
  const TQObject *senderObj = sender();
  if ( senderObj )
  {
    if ( ::tqt_cast<TDEAccelPrivate *>( senderObj ) )
        emit activated( TDEAction::AccelActivation, TQt::NoButton );
  }
  emit activated();
}

// This catches signals emitted by TDEActions inserted into QPopupMenu
// We do crude things inside it, because we need to know which
// TQPopupMenu emitted the signal. We need to be sure that it is
// only called by QPopupMenus, we plugged us in.
void TDEAction::slotPopupActivated()
{
  if( ::tqt_cast<TQSignal *>(sender()))
  {
    int id = dynamic_cast<const TQSignal *>(sender())->value().toInt();
    int pos = findContainer(id);
    if(pos != -1)
    {
      TQPopupMenu* qpm = dynamic_cast<TQPopupMenu *>( container(pos) );
      if(qpm)
      {
        TDEPopupMenu* kpm = dynamic_cast<TDEPopupMenu *>( qpm );
        TQt::ButtonState state;
        if ( kpm ) // TDEPopupMenu? Nice, it stores the state.
            state = kpm->state();
        else { // just a QPopupMenu? We'll ask for the state now then (small race condition?)
            kdDebug(129) << "TDEAction::slotPopupActivated not a TDEPopupMenu -> using keyboardMouseState()" << endl;
            state = TDEApplication::keyboardMouseState();
        }
        emit activated( TDEAction::PopupMenuActivation, state );
        slotActivated();
        return;
      }
    }
  }

  kdWarning(129)<<"Don't connect TDEAction::slotPopupActivated() to anything, expect into QPopupMenus which are in containers. Use slotActivated instead."<<endl;
  emit activated( TDEAction::PopupMenuActivation, TQt::NoButton );
  slotActivated();
}

void TDEAction::slotButtonClicked( int, TQt::ButtonState state )
{
  kdDebug(129) << "slotButtonClicked() state=" << state << endl;
  emit activated( TDEAction::ToolBarActivation, state );

  // RightButton isn't really an activation
  if ( ( state & TQt::LeftButton ) || ( state & TQt::MidButton ) )
    slotActivated();
}


void TDEAction::slotDestroyed()
{
  kdDebug(129) << "TDEAction::slotDestroyed(): this = " << this << ", name = \"" << name() << "\", sender = " << sender() << endl;
  const TQObject* const o = sender();

#ifndef KDE_NO_COMPAT  // KDE 4: remove
  if ( o == d->m_tdeaccel )
  {
    d->m_tdeaccel = 0;
    return;
  }
#endif  // KDE 4: remove end
  TQValueList<TDEAccel*> & accelList = d->m_tdeaccelList;
  TQValueList<TDEAccel*>::iterator itr = accelList.begin();
  const TQValueList<TDEAccel*>::iterator itrEnd = accelList.end();

  for( ; itr != itrEnd; ++itr)
  {
    if ( o == *itr )
    {
      disconnect( *itr, TQ_SIGNAL(destroyed()), this, TQ_SLOT(slotDestroyed()) );
      accelList.remove(itr);
      return;
    }
  }

  int i;
  do
  {
    i = findContainer(  static_cast<const TQWidget*>(o)  );
    if ( i != -1 )
      removeContainer( i );
  } while ( i != -1 );
}

int TDEAction::findContainer( const TQWidget* widget ) const
{
  int pos = 0;

  const TQValueList<TDEActionPrivate::Container> & containers = d->m_containers;

  TQValueList<TDEActionPrivate::Container>::ConstIterator it = containers.constBegin();
  const TQValueList<TDEActionPrivate::Container>::ConstIterator itEnd = containers.constEnd();

  while( it != itEnd )
  {
    if ( (*it).m_representative == widget || (*it).m_container == widget )
      return pos;
    ++it;
    ++pos;
  }

  return -1;
}

int TDEAction::findContainer( const int id ) const
{
  int pos = 0;

  const TQValueList<TDEActionPrivate::Container> & containers = d->m_containers;

  TQValueList<TDEActionPrivate::Container>::ConstIterator it = containers.constBegin();
  const TQValueList<TDEActionPrivate::Container>::ConstIterator itEnd = containers.constEnd();

  while( it != itEnd )
  {
    if ( (*it).m_id == id )
      return pos;
    ++it;
    ++pos;
  }

  return -1;
}

void TDEAction::removeContainer( int index )
{
  int i = 0;

  TQValueList<TDEActionPrivate::Container> & containers = d->m_containers;

  TQValueList<TDEActionPrivate::Container>::Iterator it = containers.begin();
  const TQValueList<TDEActionPrivate::Container>::Iterator itEnd = containers.end();

  while( it != itEnd )
  {
    if ( i == index )
    {
      containers.remove( it );
      return;
    }
    ++it;
    ++i;
  }
}

// FIXME: Remove this (ellis)
void TDEAction::slotKeycodeChanged()
{
  kdDebug(129) << "TDEAction::slotKeycodeChanged()" << endl; // -- ellis
  TDEAccelAction* pAction = d->m_tdeaccel->actions().actionPtr(name());
  if( pAction )
    setShortcut(pAction->shortcut());
}

TDEActionCollection *TDEAction::parentCollection() const
{
    return m_parentCollection;
}

void TDEAction::unplugAll()
{
  while ( containerCount() != 0 )
    unplug( container( 0 ) );
}

const KGuiItem& TDEAction::guiItem() const
{
    return *d;
}

void TDEAction::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdeaction.moc"
