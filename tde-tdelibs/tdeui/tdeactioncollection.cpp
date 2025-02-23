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

#include "tdeactioncollection.h"
#include "tdeactionshortcutlist.h"
#include "tdetoolbar.h"
#include "kxmlguifactory.h"
#include "kxmlguiclient.h"

#include <tdeaccel.h>
#include <tdeaccelbase.h>
#include <tdeapplication.h>
#include <kdebug.h>

#include <tqpopupmenu.h>
#include <tqptrdict.h>
#include <tqvariant.h>

class TDEActionCollection::TDEActionCollectionPrivate
{
public:
  TDEActionCollectionPrivate()
  {
    m_instance = 0;
    //m_bOneTDEAccelOnly = false;
    //m_iWidgetCurrent = 0;
    m_bAutoConnectShortcuts = true;
    m_widget = 0;
    m_tdeaccel = m_builderTDEAccel = 0;
    m_dctHighlightContainers.setAutoDelete( true );
    m_highlight = false;
    m_currentHighlightAction = 0;
    m_statusCleared = true;
    m_parentGUIClient = 0L;
  }

  TDEInstance *m_instance;
  TQString m_sXMLFile;
  bool m_bAutoConnectShortcuts;
  //bool m_bOneTDEAccelOnly;
  //int m_iWidgetCurrent;
  //TQValueList<TQWidget*> m_widgetList;
  //TQValueList<TDEAccel*> m_tdeaccelList;
  TQValueList<TDEActionCollection*> m_docList;
  TQWidget *m_widget;
  TDEAccel *m_tdeaccel;
  TDEAccel *m_builderTDEAccel;

  TQAsciiDict<TDEAction> m_actionDict;
  TQPtrDict< TQPtrList<TDEAction> > m_dctHighlightContainers;
  bool m_highlight;
  TDEAction *m_currentHighlightAction;
  bool m_statusCleared;
  const KXMLGUIClient *m_parentGUIClient;
};

TDEActionCollection::TDEActionCollection( TQWidget *parent, const char *name,
                                      TDEInstance *instance )
  : TQObject( parent, name )
{
  kdDebug(129) << "TDEActionCollection::TDEActionCollection( " << parent << ", " << name << " ): this = " << this << endl; // ellis
  d = new TDEActionCollectionPrivate;
  if( parent )
    setWidget( parent );
  //d->m_bOneTDEAccelOnly = (d->m_tdeaccelList.count() > 0);
  setInstance( instance );
}


TDEActionCollection::TDEActionCollection( TQWidget *watch, TQObject* parent, const char *name,
                                      TDEInstance *instance )
  : TQObject( parent, name )
{
  kdDebug(129) << "TDEActionCollection::TDEActionCollection( " << watch << ", " << parent << ", " << name << " ): this = " << this << endl; //ellis
  d = new TDEActionCollectionPrivate;
  if( watch )
    setWidget( watch );
  //d->m_bOneTDEAccelOnly = (d->m_tdeaccelList.count() > 0);
  setInstance( instance );
}

#ifndef KDE_NO_COMPAT
// KDE 4: remove
TDEActionCollection::TDEActionCollection( TQObject *parent, const char *name,
                                      TDEInstance *instance )
  : TQObject( parent, name )
{
  kdWarning(129) << "TDEActionCollection::TDEActionCollection( TQObject *parent, const char *name, TDEInstance *instance )" << endl; //ellis
  kdDebug(129) << kdBacktrace() << endl;
  d = new TDEActionCollectionPrivate;
  TQWidget* w = dynamic_cast<TQWidget*>( parent );
  if( w )
    setWidget( w );
  //d->m_bOneTDEAccelOnly = (d->m_tdeaccelList.count() > 0);
  setInstance( instance );
}

TDEActionCollection::TDEActionCollection( const TDEActionCollection &copy )
    : TQObject()
{
  kdWarning(129) << "TDEActionCollection::TDEActionCollection( const TDEActionCollection & ): function is severely deprecated." << endl;
  d = new TDEActionCollectionPrivate;
  *this = copy;
}
#endif // KDE 4: remove end

TDEActionCollection::TDEActionCollection( const char *name, const KXMLGUIClient *parent )
    : TQObject( 0L, name )
{
  d = new TDEActionCollectionPrivate;
  d->m_parentGUIClient=parent;
  d->m_instance=parent->instance();
}


TDEActionCollection::~TDEActionCollection()
{
  kdDebug(129) << "TDEActionCollection::~TDEActionCollection(): this = " << this << endl;
  for ( TQAsciiDictIterator<TDEAction> it( d->m_actionDict ); it.current(); ++it ) {
    TDEAction* pAction = it.current();
    if ( pAction->m_parentCollection == this )
      pAction->m_parentCollection = 0L;
  }

  delete d->m_tdeaccel;
  delete d->m_builderTDEAccel;
  delete d; d = 0;
}

void TDEActionCollection::setWidget( TQWidget* w )
{
  //if ( d->m_actionDict.count() > 0 ) {
  //  kdError(129) << "TDEActionCollection::setWidget(): must be called before any actions are added to collection!" << endl;
  //  kdDebug(129) << kdBacktrace() << endl;
  //}
  //else
  if ( !d->m_widget ) {
    d->m_widget = w;
    d->m_tdeaccel = new TDEAccel( w, this, "TDEActionCollection-TDEAccel" );
  }
  else if ( d->m_widget != w )
    kdWarning(129) << "TDEActionCollection::setWidget(): tried to change widget from " << d->m_widget << " to " << w << endl;
}

void TDEActionCollection::setAutoConnectShortcuts( bool b )
{
  d->m_bAutoConnectShortcuts = b;
}

bool TDEActionCollection::isAutoConnectShortcuts()
{
  return d->m_bAutoConnectShortcuts;
}

bool TDEActionCollection::addDocCollection( TDEActionCollection* pDoc )
{
	d->m_docList.append( pDoc );
	return true;
}

void TDEActionCollection::beginXMLPlug( TQWidget *widget )
{
	kdDebug(129) << "TDEActionCollection::beginXMLPlug( buildWidget = " << widget << " ): this = " <<  this << " d->m_builderTDEAccel = " << d->m_builderTDEAccel << endl;

	if( widget && !d->m_builderTDEAccel ) {
            d->m_builderTDEAccel = new TDEAccel( widget, this, "TDEActionCollection-BuilderTDEAccel" );
	}
}

void TDEActionCollection::endXMLPlug()
{
	kdDebug(129) << "TDEActionCollection::endXMLPlug(): this = " <<  this << endl;
	//s_tdeaccelXML = 0;
}

void TDEActionCollection::prepareXMLUnplug()
{
	kdDebug(129) << "TDEActionCollection::prepareXMLUnplug(): this = " <<  this << endl;
	unplugShortcuts( d->m_tdeaccel );

	if( d->m_builderTDEAccel ) {
		unplugShortcuts( d->m_builderTDEAccel );
		delete d->m_builderTDEAccel;
		d->m_builderTDEAccel = 0;
	}
}

void TDEActionCollection::unplugShortcuts( TDEAccel* tdeaccel )
{
  for ( TQAsciiDictIterator<TDEAction> it( d->m_actionDict ); it.current(); ++it ) {
    TDEAction* pAction = it.current();
    pAction->removeTDEAccel( tdeaccel );
  }

  for( uint i = 0; i < d->m_docList.count(); i++ )
    d->m_docList[i]->unplugShortcuts( tdeaccel );
}

/*void TDEActionCollection::addWidget( TQWidget* w )
{
  if( !d->m_bOneTDEAccelOnly ) {
    kdDebug(129) << "TDEActionCollection::addWidget( " << w << " ): this = " << this << endl;
    for( uint i = 0; i < d->m_widgetList.count(); i++ ) {
      if( d->m_widgetList[i] == w ) {
        d->m_iWidgetCurrent = i;
        return;
      }
  }
    d->m_iWidgetCurrent = d->m_widgetList.count();
    d->m_widgetList.append( w );
    d->m_tdeaccelList.append( new TDEAccel( w, this, "TDEActionCollection-TDEAccel" ) );
  }
}

void TDEActionCollection::removeWidget( TQWidget* w )
{
  if( !d->m_bOneTDEAccelOnly ) {
    kdDebug(129) << "TDEActionCollection::removeWidget( " << w << " ): this = " << this << endl;
    for( uint i = 0; i < d->m_widgetList.count(); i++ ) {
      if( d->m_widgetList[i] == w ) {
        // Remove TDEAccel object from children.
        TDEAccel* pTDEAccel = d->m_tdeaccelList[i];
        for ( TQAsciiDictIterator<TDEAction> it( d->m_actionDict ); it.current(); ++it ) {
          TDEAction* pAction = it.current();
          if ( pAction->m_parentCollection == this ) {
            pAction->removeTDEAccel( pTDEAccel );
          }
        }
        delete pTDEAccel;

        d->m_widgetList.remove( d->m_widgetList.at( i ) );
        d->m_tdeaccelList.remove( d->m_tdeaccelList.at( i ) );

        if( d->m_iWidgetCurrent == (int)i )
          d->m_iWidgetCurrent = -1;
        else if( d->m_iWidgetCurrent > (int)i )
          d->m_iWidgetCurrent--;
        return;
      }
    }
    kdWarning(129) << "TDEActionCollection::removeWidget( " << w << " ): widget not in list." << endl;
  }
}

bool TDEActionCollection::ownsTDEAccel() const
{
  return d->m_bOneTDEAccelOnly;
}

uint TDEActionCollection::widgetCount() const
{
  return d->m_widgetList.count();
}

const TDEAccel* TDEActionCollection::widgetTDEAccel( uint i ) const
{
  return d->m_tdeaccelList[i];
}*/

TDEAccel* TDEActionCollection::tdeaccel()
{
  //if( d->m_tdeaccelList.count() > 0 )
  //  return d->m_tdeaccelList[d->m_iWidgetCurrent];
  //else
  //  return 0;
  return d->m_tdeaccel;
}

const TDEAccel* TDEActionCollection::tdeaccel() const
{
  //if( d->m_tdeaccelList.count() > 0 )
  //  return d->m_tdeaccelList[d->m_iWidgetCurrent];
  //else
  //  return 0;
  return d->m_tdeaccel;
}

// Return the key to use in d->m_actionDict for the given action.
// Usually name(), except when unnamed.
static const char* actionDictKey( TDEAction* action, char* buffer )
{
  const char* name = action->name();
  if( !qstrcmp( name, "unnamed" ) )
  {
     sprintf(buffer, "unnamed-%p", (void *)action);
     return buffer;
  }
  return name;
}

void TDEActionCollection::_insert( TDEAction* action )
{
  char unnamed_name[100];
  const char *name = actionDictKey( action, unnamed_name );
  TDEAction *a = d->m_actionDict[ name ];
  if ( a == action )
      return;

  d->m_actionDict.insert( name, action );

  emit inserted( action );
}

void TDEActionCollection::_remove( TDEAction* action )
{
  char unnamed_name[100];
  const char *name = actionDictKey( action, unnamed_name );

  TDEAction *a = d->m_actionDict.take( name );
  if ( !a || a != action )
      return;

  emit removed( action );
  // note that we delete the action without its parent collection set to 0.
  // This triggers tdeaccel::remove, to remove any shortcut.
  delete a;
}

TDEAction* TDEActionCollection::_take( TDEAction* action )
{
  char unnamed_name[100];
  const char *name = actionDictKey( action, unnamed_name );

  TDEAction *a = d->m_actionDict.take( name );
  if ( !a || a != action )
      return 0;

  if ( a->m_parentCollection == this )
      a->m_parentCollection = 0;

  emit removed( action );

  return a;
}

void TDEActionCollection::_clear()
{
  TQAsciiDictIterator<TDEAction> it( d->m_actionDict );
  while ( it.current() )
    _remove( it.current() );
}

void TDEActionCollection::insert( TDEAction* action )   { _insert( action ); }
void TDEActionCollection::remove( TDEAction* action )   { _remove( action ); }
TDEAction* TDEActionCollection::take( TDEAction* action ) { return _take( action ); }
void TDEActionCollection::clear()                     { _clear(); }
TDEAccel* TDEActionCollection::accel()                  { return tdeaccel(); }
const TDEAccel* TDEActionCollection::accel() const      { return tdeaccel(); }
TDEAccel* TDEActionCollection::builderTDEAccel() const    { return d->m_builderTDEAccel; }

TDEAction* TDEActionCollection::action( const char* name, const char* classname ) const
{
  TDEAction* pAction = 0;

  if ( !classname && name )
    pAction = d->m_actionDict[ name ];

  else {
    TQAsciiDictIterator<TDEAction> it( d->m_actionDict );
    for( ; it.current(); ++it )
    {
      if ( ( !name || !strcmp( it.current()->name(), name ) ) &&
          ( !classname || !strcmp( it.current()->className(), classname ) ) ) {
        pAction = it.current();
        break;
      }
    }
  }

  if( !pAction ) {
    for( uint i = 0; i < d->m_docList.count() && !pAction; i++ )
      pAction = d->m_docList[i]->action( name, classname );
  }

  return pAction;
}

TDEAction* TDEActionCollection::action( int index ) const
{
  TQAsciiDictIterator<TDEAction> it( d->m_actionDict );
  it += index;
  return it.current();
//  return d->m_actions.at( index );
}

bool TDEActionCollection::readShortcutSettings( const TQString& sConfigGroup, TDEConfigBase* pConfig )
{
  return TDEActionShortcutList(this).readSettings( sConfigGroup, pConfig );
}

bool TDEActionCollection::writeShortcutSettings( const TQString& sConfigGroup, TDEConfigBase* pConfig ) const
{
  return TDEActionShortcutList((TDEActionCollection*)this).writeSettings( sConfigGroup, pConfig );
}

uint TDEActionCollection::count() const
{
  return d->m_actionDict.count();
}

TQStringList TDEActionCollection::groups() const
{
  TQStringList lst;

  TQAsciiDictIterator<TDEAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    if ( !it.current()->group().isEmpty() && !lst.contains( it.current()->group() ) )
      lst.append( it.current()->group() );

  return lst;
}

TDEActionPtrList TDEActionCollection::actions( const TQString& group ) const
{
  TDEActionPtrList lst;

  TQAsciiDictIterator<TDEAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    if ( it.current()->group() == group )
      lst.append( it.current() );
    else if ( it.current()->group().isEmpty() && group.isEmpty() )
      lst.append( it.current() );

  return lst;
}

TDEActionPtrList TDEActionCollection::actions() const
{
  TDEActionPtrList lst;

  TQAsciiDictIterator<TDEAction> it( d->m_actionDict );
  for( ; it.current(); ++it )
    lst.append( it.current() );

  return lst;
}

void TDEActionCollection::setInstance( TDEInstance *instance )
{
  if ( instance )
    d->m_instance = instance;
  else
    d->m_instance = TDEGlobal::instance();
}

TDEInstance *TDEActionCollection::instance() const
{
  return d->m_instance;
}

void TDEActionCollection::setXMLFile( const TQString& sXMLFile )
{
  d->m_sXMLFile = sXMLFile;
}

const TQString& TDEActionCollection::xmlFile() const
{
  return d->m_sXMLFile;
}

void TDEActionCollection::setHighlightingEnabled( bool enable )
{
  d->m_highlight = enable;
}

bool TDEActionCollection::highlightingEnabled() const
{
  return d->m_highlight;
}

void TDEActionCollection::connectHighlight( TQWidget *container, TDEAction *action )
{
  if ( !d->m_highlight )
    return;

  TQPtrList<TDEAction> *actionList = d->m_dctHighlightContainers[ container ];

  if ( !actionList )
  {
    actionList = new TQPtrList<TDEAction>;

    if ( ::tqt_cast<TQPopupMenu *>( container ) )
    {
      connect( container, TQ_SIGNAL( highlighted( int ) ),
               this, TQ_SLOT( slotMenuItemHighlighted( int ) ) );
      connect( container, TQ_SIGNAL( aboutToHide() ),
               this, TQ_SLOT( slotMenuAboutToHide() ) );
    }
    else if ( ::tqt_cast<TDEToolBar *>( container ) )
    {
      connect( container, TQ_SIGNAL( highlighted( int, bool ) ),
               this, TQ_SLOT( slotToolBarButtonHighlighted( int, bool ) ) );
    }

    connect( container, TQ_SIGNAL( destroyed() ),
             this, TQ_SLOT( slotDestroyed() ) );

    d->m_dctHighlightContainers.insert( container, actionList );
  }

  actionList->append( action );
}

void TDEActionCollection::disconnectHighlight( TQWidget *container, TDEAction *action )
{
  if ( !d->m_highlight )
    return;

  TQPtrList<TDEAction> *actionList = d->m_dctHighlightContainers[ container ];

  if ( !actionList )
    return;

  actionList->removeRef( action );

  if ( actionList->isEmpty() )
    d->m_dctHighlightContainers.remove( container );
}

void TDEActionCollection::slotMenuItemHighlighted( int id )
{
  if ( !d->m_highlight )
    return;

  if ( d->m_currentHighlightAction )
    emit actionHighlighted( d->m_currentHighlightAction, false );

  TQWidget *container = static_cast<TQWidget *>( const_cast<TQObject *>( sender() ));

  d->m_currentHighlightAction = findAction( container, id );

  if ( !d->m_currentHighlightAction )
  {
      if ( !d->m_statusCleared )
          emit clearStatusText();
      d->m_statusCleared = true;
      return;
  }

  d->m_statusCleared = false;
  emit actionHighlighted( d->m_currentHighlightAction );
  emit actionHighlighted( d->m_currentHighlightAction, true );
  emit actionStatusText( d->m_currentHighlightAction->toolTip() );
}

void TDEActionCollection::slotMenuAboutToHide()
{
    if ( d->m_currentHighlightAction )
        emit actionHighlighted( d->m_currentHighlightAction, false );
    d->m_currentHighlightAction = 0;

    if ( !d->m_statusCleared )
        emit clearStatusText();
    d->m_statusCleared = true;
}

void TDEActionCollection::slotToolBarButtonHighlighted( int id, bool highlight )
{
  if ( !d->m_highlight )
    return;

  TQWidget *container = static_cast<TQWidget *>( const_cast<TQObject *>( sender() ));

  TDEAction *action = findAction( container, id );

  if ( !action )
  {
      d->m_currentHighlightAction = 0;
      // use tooltip groups for toolbar status text stuff instead (Simon)
//      emit clearStatusText();
      return;
  }

  emit actionHighlighted( action, highlight );

  if ( highlight )
    d->m_currentHighlightAction = action;
  else
  {
    d->m_currentHighlightAction = 0;
//    emit clearStatusText();
  }
}

void TDEActionCollection::slotDestroyed()
{
    d->m_dctHighlightContainers.remove( reinterpret_cast<void *>( const_cast<TQObject*>(sender()) ) );
}

TDEAction *TDEActionCollection::findAction( TQWidget *container, int id )
{
  TQPtrList<TDEAction> *actionList = d->m_dctHighlightContainers[ reinterpret_cast<void *>( container ) ];

  if ( !actionList )
    return 0;

  TQPtrListIterator<TDEAction> it( *actionList );
  for (; it.current(); ++it )
    if ( it.current()->isPlugged( container, id ) )
      return it.current();

  return 0;
}

const KXMLGUIClient *TDEActionCollection::parentGUIClient() const
{
	return d->m_parentGUIClient;
}

#ifndef KDE_NO_COMPAT
// KDE 4: remove
TDEActionCollection TDEActionCollection::operator+(const TDEActionCollection &c ) const
{
  kdWarning(129) << "TDEActionCollection::operator+(): function is severely deprecated." << endl;
  TDEActionCollection ret( *this );

  TQValueList<TDEAction *> actions = c.actions();
  TQValueList<TDEAction *>::ConstIterator it = actions.begin();
  TQValueList<TDEAction *>::ConstIterator end = actions.end();
  for (; it != end; ++it )
    ret.insert( *it );

  return ret;
}

TDEActionCollection &TDEActionCollection::operator=( const TDEActionCollection &copy )
{
  kdWarning(129) << "TDEActionCollection::operator=(): function is severely deprecated." << endl;
  //d->m_bOneTDEAccelOnly = copy.d->m_bOneTDEAccelOnly;
  //d->m_iWidgetCurrent = copy.d->m_iWidgetCurrent;
  //d->m_widgetList = copy.d->m_widgetList;
  //d->m_tdeaccelList = copy.d->m_tdeaccelList;
  d->m_widget = copy.d->m_widget;
  d->m_tdeaccel = copy.d->m_tdeaccel;
  d->m_actionDict = copy.d->m_actionDict;
  setInstance( copy.instance() );
  return *this;
}

TDEActionCollection &TDEActionCollection::operator+=( const TDEActionCollection &c )
{
  kdWarning(129) << "TDEActionCollection::operator+=(): function is severely deprecated." << endl;
  TQAsciiDictIterator<TDEAction> it(c.d->m_actionDict);
  for ( ; it.current(); ++it )
    insert( it.current() );

  return *this;
}
#endif // KDE 4: remove end

//---------------------------------------------------------------------
// TDEActionShortcutList
//---------------------------------------------------------------------

TDEActionShortcutList::TDEActionShortcutList( TDEActionCollection* pColl )
: m_actions( *pColl )
	{ }
TDEActionShortcutList::~TDEActionShortcutList()
	{ }
uint TDEActionShortcutList::count() const
	{ return m_actions.count(); }
TQString TDEActionShortcutList::name( uint i ) const
	{ return m_actions.action(i)->name(); }
TQString TDEActionShortcutList::label( uint i ) const
	{ return m_actions.action(i)->text(); }
TQString TDEActionShortcutList::whatsThis( uint i ) const
	{ return m_actions.action(i)->whatsThis(); }
const TDEShortcut& TDEActionShortcutList::shortcut( uint i ) const
	{ return m_actions.action(i)->shortcut(); }
const TDEShortcut& TDEActionShortcutList::shortcutDefault( uint i ) const
	{ return m_actions.action(i)->shortcutDefault(); }
bool TDEActionShortcutList::isConfigurable( uint i ) const
	{ return m_actions.action(i)->isShortcutConfigurable(); }
bool TDEActionShortcutList::setShortcut( uint i, const TDEShortcut& cut )
	{ return m_actions.action(i)->setShortcut( cut ); }
const TDEInstance* TDEActionShortcutList::instance() const
	{ return m_actions.instance(); }
TQVariant TDEActionShortcutList::getOther( Other, uint ) const
	{ return TQVariant(); }
bool TDEActionShortcutList::setOther( Other, uint, TQVariant )
	{ return false; }
const TDEAction *TDEActionShortcutList::action( uint i) const
	{ return m_actions.action(i); }

bool TDEActionShortcutList::save() const
{
	const KXMLGUIClient* guiClient=m_actions.parentGUIClient();
	const TQString xmlFile=guiClient ? guiClient->xmlFile() : m_actions.xmlFile();
	kdDebug(129) << "TDEActionShortcutList::save(): xmlFile = " << xmlFile << endl;

	if( m_actions.xmlFile().isEmpty() )
		return writeSettings();

	TQString attrShortcut  = TQString::fromLatin1("shortcut");
	TQString attrAccel     = TQString::fromLatin1("accel"); // Depricated attribute

	// Read XML file
	TQString sXml( KXMLGUIFactory::readConfigFile( xmlFile, false, instance() ) );
	TQDomDocument doc;
	doc.setContent( sXml );

	// Process XML data

        // Get hold of ActionProperties tag
        TQDomElement elem = KXMLGUIFactory::actionPropertiesElement( doc );

	// now, iterate through our actions
	uint nSize = count();
	for( uint i = 0; i < nSize; i++ ) {
		const TQString& sName = name(i);

		bool bSameAsDefault = (shortcut(i) == shortcutDefault(i));
		//kdDebug(129) << "name = " << sName << " shortcut = " << shortcut(i).toStringInternal() << " def = " << shortcutDefault(i).toStringInternal() << endl;

		// now see if this element already exists
                // and create it if necessary (unless bSameAsDefault)
		TQDomElement act_elem = KXMLGUIFactory::findActionByName( elem, sName, !bSameAsDefault );
                if ( act_elem.isNull() )
                    continue;

		act_elem.removeAttribute( attrAccel );
		if( bSameAsDefault ) {
			act_elem.removeAttribute( attrShortcut );
			//kdDebug(129) << "act_elem.attributes().count() = " << act_elem.attributes().count() << endl;
			if( act_elem.attributes().count() == 1 )
				elem.removeChild( act_elem );
		} else {
			act_elem.setAttribute( attrShortcut, shortcut(i).toStringInternal() );
		}
	}

	// Write back to XML file
	return KXMLGUIFactory::saveConfigFile( doc, guiClient ? guiClient->localXMLFile() : m_actions.xmlFile(), instance() );
}

//---------------------------------------------------------------------
// TDEActionPtrShortcutList
//---------------------------------------------------------------------

TDEActionPtrShortcutList::TDEActionPtrShortcutList( TDEActionPtrList& list )
: m_actions( list )
	{ }
TDEActionPtrShortcutList::~TDEActionPtrShortcutList()
	{ }
uint TDEActionPtrShortcutList::count() const
	{ return m_actions.count(); }
TQString TDEActionPtrShortcutList::name( uint i ) const
	{ return m_actions[i]->name(); }
TQString TDEActionPtrShortcutList::label( uint i ) const
	{ return m_actions[i]->text(); }
TQString TDEActionPtrShortcutList::whatsThis( uint i ) const
	{ return m_actions[i]->whatsThis(); }
const TDEShortcut& TDEActionPtrShortcutList::shortcut( uint i ) const
	{ return m_actions[i]->shortcut(); }
const TDEShortcut& TDEActionPtrShortcutList::shortcutDefault( uint i ) const
	{ return m_actions[i]->shortcutDefault(); }
bool TDEActionPtrShortcutList::isConfigurable( uint i ) const
	{ return m_actions[i]->isShortcutConfigurable(); }
bool TDEActionPtrShortcutList::setShortcut( uint i, const TDEShortcut& cut )
	{ return m_actions[i]->setShortcut( cut ); }
TQVariant TDEActionPtrShortcutList::getOther( Other, uint ) const
	{ return TQVariant(); }
bool TDEActionPtrShortcutList::setOther( Other, uint, TQVariant )
	{ return false; }
bool TDEActionPtrShortcutList::save() const
	{ return false; }

void TDEActionShortcutList::virtual_hook( int id, void* data )
{ TDEShortcutList::virtual_hook( id, data ); }

void TDEActionPtrShortcutList::virtual_hook( int id, void* data )
{ TDEShortcutList::virtual_hook( id, data ); }

void TDEActionCollection::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdeactioncollection.moc"
