/* This file is part of the KDE project
   Copyright (C) 2002 Anders Lund <anders.lund@lund.tdcadsl.dk>

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


#include "tdeactionselector.h"

#include <tdelocale.h>
#include <kiconloader.h>
#include <kdialog.h> // for spacingHint()
#include <kdebug.h>
#include <tqapplication.h>
#include <tqlistbox.h>
#include <tqtoolbutton.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqevent.h>
#include <tqwhatsthis.h>

class TDEActionSelectorPrivate {
  public:
  TQListBox *availableListBox, *selectedListBox;
  TQToolButton *btnAdd, *btnRemove, *btnUp, *btnDown;
  TQLabel *lAvailable, *lSelected;
  bool moveOnDoubleClick, keyboardEnabled;
  TDEActionSelector::ButtonIconSize iconSize;
  TQString addIcon, removeIcon, upIcon, downIcon;
  TDEActionSelector::InsertionPolicy availableInsertionPolicy, selectedInsertionPolicy;
  bool showUpDownButtons;
};

//BEGIN Constructor/destructor

TDEActionSelector::TDEActionSelector( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  d = new TDEActionSelectorPrivate();
  d->moveOnDoubleClick = true;
  d->keyboardEnabled = true;
  d->iconSize = SmallIcon;
  d->addIcon = TQApplication::reverseLayout()? "back" : "forward";
  d->removeIcon = TQApplication::reverseLayout()? "forward" : "back";
  d->upIcon = "go-up";
  d->downIcon = "go-down";
  d->availableInsertionPolicy = Sorted;
  d->selectedInsertionPolicy = BelowCurrent;
  d->showUpDownButtons = true;

  //int isz = IconSize( TDEIcon::Small );

  TQHBoxLayout *lo = new TQHBoxLayout( this );
  lo->setSpacing( KDialog::spacingHint() );

  TQVBoxLayout *loAv = new TQVBoxLayout( lo );
  d->lAvailable = new TQLabel( i18n("&Available:"), this );
  loAv->addWidget( d->lAvailable );
  d->availableListBox = new TQListBox( this );
  loAv->addWidget( d->availableListBox );
  d->lAvailable->setBuddy( d->availableListBox );

  TQVBoxLayout *loHBtns = new TQVBoxLayout( lo );
  loHBtns->addStretch( 1 );
  d->btnAdd = new TQToolButton( this );
  loHBtns->addWidget( d->btnAdd );
  d->btnRemove = new TQToolButton( this );
  loHBtns->addWidget( d->btnRemove );
  loHBtns->addStretch( 1 );

  TQVBoxLayout *loS = new TQVBoxLayout( lo );
  d->lSelected = new TQLabel( i18n("&Selected:"), this );
  loS->addWidget( d->lSelected );
  d->selectedListBox = new TQListBox( this );
  loS->addWidget( d->selectedListBox );
  d->lSelected->setBuddy( d->selectedListBox );

  TQVBoxLayout *loVBtns = new TQVBoxLayout( lo );
  loVBtns->addStretch( 1 );
  d->btnUp = new TQToolButton( this );
  d->btnUp->setAutoRepeat( true );
  loVBtns->addWidget( d->btnUp );
  d->btnDown = new TQToolButton( this );
  d->btnDown->setAutoRepeat( true );
  loVBtns->addWidget( d->btnDown );
  loVBtns->addStretch( 1 );

  loadIcons();

  connect( d->btnAdd, TQ_SIGNAL(clicked()), this, TQ_SLOT(buttonAddClicked()) );
  connect( d->btnRemove, TQ_SIGNAL(clicked()), this, TQ_SLOT(buttonRemoveClicked()) );
  connect( d->btnUp, TQ_SIGNAL(clicked()), this, TQ_SLOT(buttonUpClicked()) );
  connect( d->btnDown, TQ_SIGNAL(clicked()), this, TQ_SLOT(buttonDownClicked()) );
  connect( d->availableListBox, TQ_SIGNAL(doubleClicked(TQListBoxItem*)),
           this, TQ_SLOT(itemDoubleClicked(TQListBoxItem*)) );
  connect( d->selectedListBox, TQ_SIGNAL(doubleClicked(TQListBoxItem*)),
           this, TQ_SLOT(itemDoubleClicked(TQListBoxItem*)) );
  connect( d->availableListBox, TQ_SIGNAL(currentChanged(TQListBoxItem*)),
           this, TQ_SLOT(slotCurrentChanged(TQListBoxItem *)) );
  connect( d->selectedListBox, TQ_SIGNAL(currentChanged(TQListBoxItem*)),
           this, TQ_SLOT(slotCurrentChanged(TQListBoxItem *)) );

  d->availableListBox->installEventFilter( this );
  d->selectedListBox->installEventFilter( this );
}

TDEActionSelector::~TDEActionSelector()
{
  delete d;
}

//END Constructor/destroctor

//BEGIN Public Methods

TQListBox *TDEActionSelector::availableListBox() const
{
  return d->availableListBox;
}

TQListBox *TDEActionSelector::selectedListBox() const
{
  return d->selectedListBox;
}

void TDEActionSelector::setButtonIcon( const TQString &icon, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->addIcon = icon;
    d->btnAdd->setIconSet( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonRemove:
    d->removeIcon = icon;
    d->btnRemove->setIconSet( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonUp:
    d->upIcon = icon;
    d->btnUp->setIconSet( SmallIconSet( icon, d->iconSize ) );
    break;
    case ButtonDown:
    d->downIcon = icon;
    d->btnDown->setIconSet( SmallIconSet( icon, d->iconSize ) );
    break;
    default:
    kdDebug(13001)<<"TDEActionSelector::setButtonIcon: DAINBREAD!"<<endl;
  }
}

void TDEActionSelector::setButtonIconSet( const TQIconSet &iconset, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->btnAdd->setIconSet( iconset );
    break;
    case ButtonRemove:
    d->btnRemove->setIconSet( iconset );
    break;
    case ButtonUp:
    d->btnUp->setIconSet( iconset );
    break;
    case ButtonDown:
    d->btnDown->setIconSet( iconset );
    break;
    default:
    kdDebug(13001)<<"TDEActionSelector::setButtonIconSet: DAINBREAD!"<<endl;
  }
}

void TDEActionSelector::setButtonTooltip( const TQString &tip, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    d->btnAdd->setTextLabel( tip );
    break;
    case ButtonRemove:
    d->btnRemove->setTextLabel( tip );
    break;
    case ButtonUp:
    d->btnUp->setTextLabel( tip );
    break;
    case ButtonDown:
    d->btnDown->setTextLabel( tip );
    break;
    default:
    kdDebug(13001)<<"TDEActionSelector::setButtonToolTip: DAINBREAD!"<<endl;
  }
}

void TDEActionSelector::setButtonWhatsThis( const TQString &text, MoveButton button )
{
  switch ( button )
  {
    case ButtonAdd:
    TQWhatsThis::add( d->btnAdd, text );
    break;
    case ButtonRemove:
    TQWhatsThis::add( d->btnRemove, text );
    break;
    case ButtonUp:
    TQWhatsThis::add( d->btnUp, text );
    break;
    case ButtonDown:
    TQWhatsThis::add( d->btnDown, text );
    break;
    default:
    kdDebug(13001)<<"TDEActionSelector::setButtonWhatsThis: DAINBREAD!"<<endl;
  }
}

void TDEActionSelector::setButtonsEnabled()
{
  d->btnAdd->setEnabled( d->availableListBox->currentItem() > -1 );
  d->btnRemove->setEnabled( d->selectedListBox->currentItem() > -1 );
  d->btnUp->setEnabled( d->selectedListBox->currentItem() > 0 );
  d->btnDown->setEnabled( d->selectedListBox->currentItem() > -1 &&
                          d->selectedListBox->currentItem() < (int)d->selectedListBox->count() - 1 );
}

//END Public Methods

//BEGIN Properties

bool TDEActionSelector::moveOnDoubleClick() const
{
  return d->moveOnDoubleClick;
}

void TDEActionSelector::setMoveOnDoubleClick( bool b )
{
  d->moveOnDoubleClick = b;
}

bool TDEActionSelector::keyboardEnabled() const
{
  return d->keyboardEnabled;
}

void TDEActionSelector::setKeyboardEnabled( bool b )
{
  d->keyboardEnabled = b;
}

TQString TDEActionSelector::availableLabel() const
{
  return d->lAvailable->text();
}

void TDEActionSelector::setAvailableLabel( const TQString &text )
{
  d->lAvailable->setText( text );
}

TQString TDEActionSelector::selectedLabel() const
{
  return d->lSelected->text();
}

void TDEActionSelector::setSelectedLabel( const TQString &text )
{
  d->lSelected->setText( text );
}

TDEActionSelector::ButtonIconSize TDEActionSelector::buttonIconSize() const
{
  return d->iconSize;
}

void TDEActionSelector::setButtonIconSize( ButtonIconSize size )
{
  d->iconSize = size;
  // reload icons
  loadIcons();
}

TDEActionSelector::InsertionPolicy TDEActionSelector::availableInsertionPolicy() const
{
  return d->availableInsertionPolicy;
}

void TDEActionSelector::setAvailableInsertionPolicy( InsertionPolicy p )
{
  d->availableInsertionPolicy = p;
}

TDEActionSelector::InsertionPolicy TDEActionSelector::selectedInsertionPolicy() const
{
  return d->selectedInsertionPolicy;
}

void TDEActionSelector::setSelectedInsertionPolicy( InsertionPolicy p )
{
  d->selectedInsertionPolicy = p;
}

bool TDEActionSelector::showUpDownButtons() const
{
  return d->showUpDownButtons;
}

void TDEActionSelector::setShowUpDownButtons( bool show )
{
  d->showUpDownButtons = show;
  if ( show )
  {
    d->btnUp->show();
    d->btnDown->show();
  }
  else
  {
    d->btnUp->hide();
    d->btnDown->hide();
  }
}

//END Properties

//BEGIN Public Slots

void TDEActionSelector::polish()
{
  setButtonsEnabled();
}

//END Public Slots

//BEGIN Protected
void TDEActionSelector::keyPressEvent( TQKeyEvent *e )
{
  if ( ! d->keyboardEnabled ) return;
  if ( (e->state() & TQt::ControlButton) )
  {
    switch ( e->key() )
    {
      case Key_Right:
      buttonAddClicked();
      break;
      case Key_Left:
      buttonRemoveClicked();
      break;
      case Key_Up:
      buttonUpClicked();
      break;
      case Key_Down:
      buttonDownClicked();
      break;
      default:
      e->ignore();
      return;
    }
  }
}

bool TDEActionSelector::eventFilter( TQObject *o, TQEvent *e )
{
  if ( d->keyboardEnabled && e->type() == TQEvent::KeyPress )
  {
    if  ( (((TQKeyEvent*)e)->state() & TQt::ControlButton) )
    {
      switch ( ((TQKeyEvent*)e)->key() )
      {
        case Key_Right:
        buttonAddClicked();
        break;
        case Key_Left:
        buttonRemoveClicked();
        break;
        case Key_Up:
        buttonUpClicked();
        break;
        case Key_Down:
        buttonDownClicked();
        break;
        default:
        return TQWidget::eventFilter( o, e );
        break;
      }
      return true;
    }
    else if ( o->inherits( "TQListBox" ) )
    {
      switch ( ((TQKeyEvent*)e)->key() )
      {
        case Key_Return:
        case Key_Enter:
        TQListBox *lb = (TQListBox*)o;
        int index = lb->currentItem();
        if ( index < 0 ) break;
        moveItem( lb->item( index ) );
        return true;
      }
    }
  }
  return TQWidget::eventFilter( o, e );
}

//END Protected

//BEGIN Private Slots

void TDEActionSelector::buttonAddClicked()
{
  // move all selected items from available to selected listbox
  TQListBoxItem *item = d->availableListBox->firstItem();
  while ( item ) {
    if ( item->isSelected() ) {
      d->availableListBox->takeItem( item );
      d->selectedListBox->insertItem( item, insertionIndex( d->selectedListBox, d->selectedInsertionPolicy ) );
      d->selectedListBox->setCurrentItem( item );
      emit added( item );
      item = d->availableListBox->firstItem();
    } else
      item = item->next();
  }
  if ( d->selectedInsertionPolicy == Sorted )
    d->selectedListBox->sort();
  d->selectedListBox->setFocus();
}

void TDEActionSelector::buttonRemoveClicked()
{
  // move all selected items from selected to available listbox
  TQListBoxItem *item = d->selectedListBox->firstItem();
  while ( item ) {
    if ( item->isSelected() ) {
      d->selectedListBox->takeItem( item );
      d->availableListBox->insertItem( item, insertionIndex( d->availableListBox, d->availableInsertionPolicy ) );
      d->availableListBox->setCurrentItem( item );
      emit removed( item );
      item = d->selectedListBox->firstItem();
    } else
      item = item->next();
  }
  if ( d->availableInsertionPolicy == Sorted )
    d->availableListBox->sort();
  d->availableListBox->setFocus();
}

void TDEActionSelector::buttonUpClicked()
{
  int c = d->selectedListBox->currentItem();
  if ( c < 1 ) return;
  TQListBoxItem *item = d->selectedListBox->item( c );
  d->selectedListBox->takeItem( item );
  d->selectedListBox->insertItem( item, c-1 );
  d->selectedListBox->setCurrentItem( item );
  emit movedUp( item );
}

void TDEActionSelector::buttonDownClicked()
{
  int c = d->selectedListBox->currentItem();
  if ( c < 0 || c == int( d->selectedListBox->count() ) - 1 ) return;
  TQListBoxItem *item = d->selectedListBox->item( c );
  d->selectedListBox->takeItem( item );
  d->selectedListBox->insertItem( item, c+1 );
  d->selectedListBox->setCurrentItem( item );
  emit movedDown( item );
}

void TDEActionSelector::itemDoubleClicked( TQListBoxItem *item )
{
  if ( d->moveOnDoubleClick )
    moveItem( item );
}

//END Private Slots

//BEGIN Private Methods

void TDEActionSelector::loadIcons()
{
  d->btnAdd->setIconSet( SmallIconSet( d->addIcon, d->iconSize ) );
  d->btnRemove->setIconSet( SmallIconSet( d->removeIcon, d->iconSize ) );
  d->btnUp->setIconSet( SmallIconSet( d->upIcon, d->iconSize ) );
  d->btnDown->setIconSet( SmallIconSet( d->downIcon, d->iconSize ) );
}

void TDEActionSelector::moveItem( TQListBoxItem *item )
{
  TQListBox *lbFrom = item->listBox();
  TQListBox *lbTo;
  if ( lbFrom == d->availableListBox )
    lbTo = d->selectedListBox;
  else if ( lbFrom == d->selectedListBox )
    lbTo = d->availableListBox;
  else  //?! somewhat unlikely...
    return;

  InsertionPolicy p = ( lbTo == d->availableListBox ) ?
                        d->availableInsertionPolicy : d->selectedInsertionPolicy;

  lbFrom->takeItem( item );
  lbTo->insertItem( item, insertionIndex( lbTo, p ) );
  lbTo->setFocus();
  lbTo->setCurrentItem( item );

  if ( p == Sorted )
    lbTo->sort();
  if ( lbTo == d->selectedListBox )
    emit added( item );
  else
    emit removed( item );
}

int TDEActionSelector::insertionIndex( TQListBox *lb, InsertionPolicy policy )
{
  int index;
  switch ( policy )
  {
    case BelowCurrent:
    index = lb->currentItem();
    if ( index > -1 ) index += 1;
    break;
    case AtTop:
    index = 0;
    break;
    default:
    index = -1;
  }
  return index;
}

//END Private Methods
#include "tdeactionselector.moc"
