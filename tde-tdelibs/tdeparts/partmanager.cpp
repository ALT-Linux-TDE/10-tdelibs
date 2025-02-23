/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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

#include "partmanager.h"
#include <tdeparts/event.h>
#include <tdeparts/part.h>
#include <tdeglobal.h>
#include <kdebug.h>

#include <tqapplication.h>

//#define DEBUG_PARTMANAGER

using namespace KParts;

template class TQPtrList<Part>;

namespace KParts {

class PartManagerPrivate
{
public:
  PartManagerPrivate()
  {
    m_activeWidget = 0;
    m_activePart = 0;
    m_selectedPart = 0;
    m_selectedWidget = 0;
    m_bAllowNestedParts = false;
    m_bIgnoreScrollBars = false;
    m_activationButtonMask = TQt::LeftButton | TQt::MidButton | TQt::RightButton;
    m_reason = PartManager::NoReason;
  }
  ~PartManagerPrivate()
  {
  }
  void setReason( TQEvent* ev ) {
    switch( ev->type() ) {
    case TQEvent::MouseButtonPress:
    case TQEvent::MouseButtonDblClick: {
      TQMouseEvent* mev = static_cast<TQMouseEvent*>( ev );
      m_reason = mev->button() == TQt::LeftButton
                 ? PartManager::ReasonLeftClick
                 : ( mev->button() == TQt::MidButton
                     ? PartManager::ReasonMidClick
                     : PartManager::ReasonRightClick );
      break;
    }
    case TQEvent::FocusIn:
      m_reason = static_cast<TQFocusEvent*>( ev )->reason();
      break;
    default:
      kdWarning(1000) << "PartManagerPrivate::setReason got unexpected ev type " << ev->type() << endl;
      break;
    }
  }

  Part * m_activePart;
  TQWidget *m_activeWidget;

  TQPtrList<Part> m_parts;

  PartManager::SelectionPolicy m_policy;

  Part *m_selectedPart;
  TQWidget *m_selectedWidget;

  TQPtrList<TQWidget> m_managedTopLevelWidgets;
  short int m_activationButtonMask;
  bool m_bIgnoreScrollBars;
  bool m_bAllowNestedParts;
  int m_reason;
};

}

PartManager::PartManager( TQWidget * parent, const char * name )
 : TQObject( parent, name )
{
  d = new PartManagerPrivate;

  tqApp->installEventFilter( this );

  d->m_policy = Direct;

  addManagedTopLevelWidget( parent );
}

PartManager::PartManager( TQWidget *topLevel, TQObject *parent, const char *name )
 : TQObject( parent, name )
{
  d = new PartManagerPrivate;

  tqApp->installEventFilter( this );

  d->m_policy = Direct;

  addManagedTopLevelWidget( topLevel );
}

PartManager::~PartManager()
{
  for ( TQPtrListIterator<TQWidget> it( d->m_managedTopLevelWidgets );
        it.current(); ++it )
    disconnect( it.current(), TQ_SIGNAL( destroyed() ),
                this, TQ_SLOT( slotManagedTopLevelWidgetDestroyed() ) );

  for ( TQPtrListIterator<Part> it( d->m_parts ); it.current(); ++it )
  {
      it.current()->setManager( 0 );
  }

  // core dumps ... setActivePart( 0L );
  tqApp->removeEventFilter( this );
  delete d;
}

void PartManager::setSelectionPolicy( SelectionPolicy policy )
{
  d->m_policy = policy;
}

PartManager::SelectionPolicy PartManager::selectionPolicy() const
{
  return d->m_policy;
}

void PartManager::setAllowNestedParts( bool allow )
{
  d->m_bAllowNestedParts = allow;
}

bool PartManager::allowNestedParts() const
{
  return d->m_bAllowNestedParts;
}

void PartManager::setIgnoreScrollBars( bool ignore )
{
  d->m_bIgnoreScrollBars = ignore;
}

bool PartManager::ignoreScrollBars() const
{
  return d->m_bIgnoreScrollBars;
}

void PartManager::setActivationButtonMask( short int buttonMask )
{
    d->m_activationButtonMask = buttonMask;
}

short int PartManager::activationButtonMask() const
{
    return d->m_activationButtonMask;
}

bool PartManager::eventFilter( TQObject *obj, TQEvent *ev )
{

  if ( ev->type() != TQEvent::MouseButtonPress &&
       ev->type() != TQEvent::MouseButtonDblClick &&
       ev->type() != TQEvent::FocusIn )
    return false;

  if ( !obj->isWidgetType() )
    return false;

  TQWidget *w = static_cast<TQWidget*>( obj );

  if ( ( w->testWFlags( WType_Dialog ) && w->isModal() ) ||
       w->testWFlags( WType_Popup ) || w->testWFlags( WStyle_Tool ) )
    return false;

  TQMouseEvent* mev = 0L;
  if ( ev->type() == TQEvent::MouseButtonPress || ev->type() == TQEvent::MouseButtonDblClick )
  {
      mev = static_cast<TQMouseEvent*>( ev );
#ifdef DEBUG_PARTMANAGER
      kdDebug(1000) << "PartManager::eventFilter button: " << mev->button() << " " << "d->m_activationButtonMask=" << d->m_activationButtonMask << endl;
#endif
      if ( ( mev->button() & d->m_activationButtonMask ) == 0 )
        return false; // ignore this button
  }

  Part * part;
  while ( w )
  {
    TQPoint pos;

    if ( !d->m_managedTopLevelWidgets.containsRef( w->topLevelWidget() ) )
      return false;

    if ( d->m_bIgnoreScrollBars && w->inherits( "TQScrollBar" ) )
      return false;

    if ( mev ) // mouse press or mouse double-click event
    {
      pos = mev->globalPos();
      part = findPartFromWidget( w, pos );
    } else
      part = findPartFromWidget( w );

#ifdef DEBUG_PARTMANAGER
    TQCString evType = ( ev->type() == TQEvent::MouseButtonPress ) ? "MouseButtonPress"
                      : ( ev->type() == TQEvent::MouseButtonDblClick ) ? "MouseButtonDblClick"
                      : ( ev->type() == TQEvent::FocusIn ) ? "FocusIn" : "OTHER! ERROR!";
#endif
    if ( part ) // We found a part whose widget is w
    {
      if ( d->m_policy == PartManager::TriState )
      {
        if ( ev->type() == TQEvent::MouseButtonDblClick )
        {
          if ( part == d->m_activePart && w == d->m_activeWidget )
            return false;

#ifdef DEBUG_PARTMANAGER
          kdDebug(1000) << "PartManager::eventFilter dblclick -> setActivePart" << part << endl;
#endif
          d->setReason( ev );
          setActivePart( part, w );
          d->m_reason = NoReason;
          return true;
        }

        if ( ( d->m_selectedWidget != w || d->m_selectedPart != part ) &&
             ( d->m_activeWidget != w || d->m_activePart != part ) )
        {
          if ( part->isSelectable() )
            setSelectedPart( part, w );
          else {
#ifdef DEBUG_PARTMANAGER
              kdDebug(1000) << "Part " << part << " (non-selectable) made active because " << w->className() << " got event" << " " << evType << endl;
#endif
              d->setReason( ev );
              setActivePart( part, w );
              d->m_reason = NoReason;
          }
          return true;
        }
        else if ( d->m_selectedWidget == w && d->m_selectedPart == part )
        {
#ifdef DEBUG_PARTMANAGER
          kdDebug(1000) << "Part " << part << " made active (from selected) because " << w->className() << " got event" << " " << evType << endl;
#endif
          d->setReason( ev );
          setActivePart( part, w );
          d->m_reason = NoReason;
          return true;
        }
        else if ( d->m_activeWidget == w && d->m_activePart == part )
        {
          setSelectedPart( 0L );
          return false;
        }

        return false;
      }
      else if ( part != d->m_activePart )
      {
#ifdef DEBUG_PARTMANAGER
        kdDebug(1000) << "Part " << part << " made active because " << w->className() << " got event" << " " << evType << endl;
#endif
        d->setReason( ev );
        setActivePart( part, w );
        d->m_reason = NoReason;
      }

      return false;
    }

    w = w->parentWidget();

    if ( w && ( ( w->testWFlags( WType_Dialog ) && w->isModal() ) ||
                w->testWFlags( WType_Popup ) || w->testWFlags( WStyle_Tool ) ) )
    {
#ifdef DEBUG_PARTMANAGER
      kdDebug(1000) << TQString("No part made active although %1/%2 got event - loop aborted").arg(obj->name()).arg(obj->className()) << endl;
#endif
      return false;
    }

  }

#ifdef DEBUG_PARTMANAGER
  kdDebug(1000) << TQString("No part made active although %1/%2 got event").arg(obj->name()).arg(obj->className()) << endl;
#endif
  return false;
}

Part * PartManager::findPartFromWidget( TQWidget * widget, const TQPoint &pos )
{
  TQPtrListIterator<Part> it ( d->m_parts );
  for ( ; it.current() ; ++it )
  {
    Part *part = it.current()->hitTest( widget, pos );
    if ( part && d->m_parts.findRef( part ) != -1 )
      return part;
  }
  return 0L;
}

Part * PartManager::findPartFromWidget( TQWidget * widget )
{
  TQPtrListIterator<Part> it ( d->m_parts );
  for ( ; it.current() ; ++it )
  {
    if ( widget == it.current()->widget() )
      return it.current();
  }
  return 0L;
}

void PartManager::addPart( Part *part, bool setActive )
{
  if ( d->m_parts.findRef( part ) != -1 ) // don't add parts more than once :)
  {
#ifdef DEBUG_PARTMANAGER
    kdWarning(1000) << k_funcinfo << part << " already added" << kdBacktrace(5) << endl;
#endif
    return;
  }

  d->m_parts.append( part );

  part->setManager( this );

  if ( setActive )
  {
    setActivePart( part );
    if ( part->widget() )
      part->widget()->setFocus();
  }

  // Prevent focus problems
  if ( part->widget() && part->widget()->focusPolicy() == TQWidget::NoFocus )
  {
    kdWarning(1000) << "Part '" << part->name() << "' has a widget " << part->widget()->name() << " with a focus policy of NoFocus. It should have at least a ClickFocus policy, for part activation to work well." << endl;
  }
  if ( part->widget() && part->widget()->focusPolicy() == TQWidget::TabFocus )
  {
    kdWarning(1000) << "Part '" << part->name() << "' has a widget " << part->widget()->name() << " with a focus policy of TabFocus. It should have at least a ClickFocus policy, for part activation to work well." << endl;
  }

  if ( setActive && part->widget() )
    part->widget()->show();
  emit partAdded( part );
}

void PartManager::removePart( Part *part )
{
  if ( d->m_parts.findRef( part ) == -1 )
  {
    kdFatal(1000) << TQString(TQString("Can't remove part %1, not in KPartManager's list.").arg(part->name())) << endl;
    return;
  }

  //Warning. The part could be already deleted
  //kdDebug(1000) << TQString("Part %1 removed").arg(part->name()) << endl;
  int nb = d->m_parts.count();
  bool ok = d->m_parts.removeRef( part );
  Q_ASSERT( ok );
  Q_ASSERT( (int)d->m_parts.count() == nb-1 );
  part->setManager(0);

  emit partRemoved( part );

  if ( part == d->m_activePart )
    setActivePart( 0 );
  if ( part == d->m_selectedPart )
    setSelectedPart( 0 );
}

void PartManager::replacePart( Part * oldPart, Part * newPart, bool setActive )
{
  //kdDebug(1000) << "replacePart " << oldPart->name() << "-> " << newPart->name() << " setActive=" << setActive << endl;
  // This methods does exactly removePart + addPart but without calling setActivePart(0) in between
  if ( d->m_parts.findRef( oldPart ) == -1 )
  {
    kdFatal(1000) << TQString(TQString("Can't remove part %1, not in KPartManager's list.").arg(oldPart->name())) << endl;
    return;
  }

  d->m_parts.removeRef( oldPart );
  oldPart->setManager(0);

  emit partRemoved( oldPart );

  addPart( newPart, setActive );
}

void PartManager::setActivePart( Part *part, TQWidget *widget )
{
  if ( part && d->m_parts.findRef( part ) == -1 )
  {
      kdWarning( 1000 ) << "PartManager::setActivePart : trying to activate a non-registered part! " << part->name() << endl;
      return; // don't allow someone call setActivePart with a part we don't know about
  }

  //check whether nested parts are disallowed and activate the top parent part then, by traversing the
  //tree recursively (Simon)
  if ( part && !d->m_bAllowNestedParts )
  {
    TQObject *parentPart = part->parent(); // ### this relies on people using KParts::Factory!
    if ( parentPart && parentPart->inherits( "KParts::Part" ) )
    {
      KParts::Part *parPart = static_cast<KParts::Part *>( parentPart );
      setActivePart( parPart, parPart->widget() );
      return;
    }
  }

#ifdef DEBUG_PARTMANAGER
  kdDebug(1000) << "PartManager::setActivePart d->m_activePart=" << d->m_activePart << "<->part=" << part
                << " d->m_activeWidget=" << d->m_activeWidget << "<->widget=" << widget << endl;
#endif

  // don't activate twice
  if ( d->m_activePart && part && d->m_activePart == part &&
       (!widget || d->m_activeWidget == widget) )
    return;

  KParts::Part *oldActivePart = d->m_activePart;
  TQWidget *oldActiveWidget = d->m_activeWidget;

  setSelectedPart( 0L );

  d->m_activePart = part;
  d->m_activeWidget = widget;

  if ( oldActivePart )
  {
    KParts::Part *savedActivePart = part;
    TQWidget *savedActiveWidget = widget;

    PartActivateEvent ev( false, oldActivePart, oldActiveWidget );
    TQApplication::sendEvent( oldActivePart, &ev );
    if ( oldActiveWidget )
    {
      disconnect( oldActiveWidget, TQ_SIGNAL( destroyed() ),
                  this, TQ_SLOT( slotWidgetDestroyed() ) );
      TQApplication::sendEvent( oldActiveWidget, &ev );
    }

    d->m_activePart = savedActivePart;
    d->m_activeWidget = savedActiveWidget;
  }

  if ( d->m_activePart )
  {
    if ( !widget )
      d->m_activeWidget = part->widget();

    PartActivateEvent ev( true, d->m_activePart, d->m_activeWidget );
    TQApplication::sendEvent( d->m_activePart, &ev );
    if ( d->m_activeWidget )
    {
      connect( d->m_activeWidget, TQ_SIGNAL( destroyed() ),
               this, TQ_SLOT( slotWidgetDestroyed() ) );
      TQApplication::sendEvent( d->m_activeWidget, &ev );
    }
  }
  // Set the new active instance in TDEGlobal
  setActiveInstance( d->m_activePart ? d->m_activePart->instance() : 0L );

  kdDebug(1000) << this << " emitting activePartChanged " << d->m_activePart << endl;
  emit activePartChanged( d->m_activePart );
}

void PartManager::setActiveInstance( TDEInstance * instance )
{
  // It's a separate method to allow redefining this behavior
  TDEGlobal::_activeInstance = instance;
}

Part *PartManager::activePart() const
{
  return d->m_activePart;
}

TQWidget *PartManager::activeWidget() const
{
  return  d->m_activeWidget;
}

void PartManager::setSelectedPart( Part *part, TQWidget *widget )
{
  if ( part == d->m_selectedPart && widget == d->m_selectedWidget )
    return;

  Part *oldPart = d->m_selectedPart;
  TQWidget *oldWidget = d->m_selectedWidget;

  d->m_selectedPart = part;
  d->m_selectedWidget = widget;

  if ( part && !widget )
    d->m_selectedWidget = part->widget();

  if ( oldPart )
  {
    PartSelectEvent ev( false, oldPart, oldWidget );
    TQApplication::sendEvent( oldPart, &ev );
    TQApplication::sendEvent( oldWidget, &ev );
  }

  if ( d->m_selectedPart )
  {
    PartSelectEvent ev( true, d->m_selectedPart, d->m_selectedWidget );
    TQApplication::sendEvent( d->m_selectedPart, &ev );
    TQApplication::sendEvent( d->m_selectedWidget, &ev );
  }
}

Part *PartManager::selectedPart() const
{
  return d->m_selectedPart;
}

TQWidget *PartManager::selectedWidget() const
{
  return d->m_selectedWidget;
}

void PartManager::slotObjectDestroyed()
{
  kdDebug(1000) << "KPartManager::slotObjectDestroyed()" << endl;
  removePart( const_cast<Part *>( static_cast<const Part *>( sender() ) ) );
}

void PartManager::slotWidgetDestroyed()
{
  kdDebug(1000) << "KPartsManager::slotWidgetDestroyed()" << endl;
  if ( static_cast<const TQWidget *>( sender() ) == d->m_activeWidget )
    setActivePart( 0L ); //do not remove the part because if the part's widget dies, then the
                         //part will delete itself anyway, invoking removePart() in its destructor
}

const TQPtrList<Part> *PartManager::parts() const
{
  return &d->m_parts;
}

void PartManager::addManagedTopLevelWidget( const TQWidget *topLevel )
{
  if ( !topLevel->isTopLevel() )
    return;

  if ( d->m_managedTopLevelWidgets.containsRef( topLevel ) )
    return;

  d->m_managedTopLevelWidgets.append( topLevel );
  connect( topLevel, TQ_SIGNAL( destroyed() ),
           this, TQ_SLOT( slotManagedTopLevelWidgetDestroyed() ) );
}

void PartManager::removeManagedTopLevelWidget( const TQWidget *topLevel )
{
  if ( !topLevel->isTopLevel() )
    return;

  if ( d->m_managedTopLevelWidgets.findRef( topLevel ) == -1 )
    return;

  d->m_managedTopLevelWidgets.remove();
}

void PartManager::slotManagedTopLevelWidgetDestroyed()
{
  const TQWidget *widget = static_cast<const TQWidget *>( sender() );
  removeManagedTopLevelWidget( widget );
}

int PartManager::reason() const
{
  return d->m_reason;
}

void PartManager::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "partmanager.moc"
