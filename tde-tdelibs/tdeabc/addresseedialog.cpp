/*
    This file is part of libtdeabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqgroupbox.h>
#include <tqregexp.h>

#include <tdelocale.h>
#include <kdebug.h>

#include "stdaddressbook.h"

#include "addresseedialog.h"
#include "addresseedialog.moc"

using namespace TDEABC;

AddresseeItem::AddresseeItem( TQListView *parent, const Addressee &addressee ) :
  TQListViewItem( parent ),
  mAddressee( addressee )
{
  setText( Name, addressee.realName() );
  setText( Email, addressee.preferredEmail() );
}

TQString AddresseeItem::key( int column, bool ) const
{
  if (column == Email) {
    TQString value = text(Email);
    TQRegExp emailRe("<\\S*>");
    int match = emailRe.search(value);
    if (match > -1)
      value = value.mid(match + 1, emailRe.matchedLength() - 2);

    return value.lower();
  }

  return text(column).lower();
}

AddresseeDialog::AddresseeDialog( TQWidget *parent, bool multiple ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Addressee"),
               Ok|Cancel, Ok, parent ), mMultiple( multiple )
{
  TQWidget *topWidget = plainPage();

  TQBoxLayout *topLayout = new TQHBoxLayout( topWidget );
  TQBoxLayout *listLayout = new TQVBoxLayout;
  topLayout->addLayout( listLayout );

  mAddresseeList = new TDEListView( topWidget );
  mAddresseeList->addColumn( i18n("Name") );
  mAddresseeList->addColumn( i18n("Email") );
  mAddresseeList->setAllColumnsShowFocus( true );
  mAddresseeList->setFullWidth( true );
  listLayout->addWidget( mAddresseeList );
  connect( mAddresseeList, TQ_SIGNAL( doubleClicked( TQListViewItem * ) ),
           TQ_SLOT( slotOk() ) );
  connect( mAddresseeList, TQ_SIGNAL( selectionChanged( TQListViewItem * ) ),
           TQ_SLOT( updateEdit( TQListViewItem * ) ) );

  mAddresseeEdit = new KLineEdit( topWidget );
  mAddresseeEdit->setCompletionMode( TDEGlobalSettings::CompletionAuto );
  connect( mAddresseeEdit->completionObject(), TQ_SIGNAL( match( const TQString & ) ),
           TQ_SLOT( selectItem( const TQString & ) ) );
  mAddresseeEdit->setFocus();
  mAddresseeEdit->completionObject()->setIgnoreCase( true );
  listLayout->addWidget( mAddresseeEdit );

  setInitialSize( TQSize( 450, 300 ) );

  if ( mMultiple ) {
    TQBoxLayout *selectedLayout = new TQVBoxLayout;
    topLayout->addLayout( selectedLayout );
    topLayout->setSpacing( spacingHint() );

    TQGroupBox *selectedGroup = new TQGroupBox( 1, TQt::Horizontal, i18n("Selected"),
                                              topWidget );
    selectedLayout->addWidget( selectedGroup );

    mSelectedList = new TDEListView( selectedGroup );
    mSelectedList->addColumn( i18n("Name") );
    mSelectedList->addColumn( i18n("Email") );
    mSelectedList->setAllColumnsShowFocus( true );
    mSelectedList->setFullWidth( true );
    connect( mSelectedList, TQ_SIGNAL( doubleClicked( TQListViewItem * ) ),
             TQ_SLOT( removeSelected() ) );

    TQPushButton *unselectButton = new TQPushButton( i18n("Unselect"), selectedGroup );
    connect ( unselectButton, TQ_SIGNAL( clicked() ), TQ_SLOT( removeSelected() ) );

    connect( mAddresseeList, TQ_SIGNAL( clicked( TQListViewItem * ) ),
             TQ_SLOT( addSelected( TQListViewItem * ) ) );

    setInitialSize( TQSize( 650, 350 ) );
  }

  mAddressBook = StdAddressBook::self( true );
  connect( mAddressBook, TQ_SIGNAL( addressBookChanged( AddressBook* ) ),
           TQ_SLOT( addressBookChanged() ) );
  connect( mAddressBook, TQ_SIGNAL( loadingFinished( Resource* ) ),
           TQ_SLOT( addressBookChanged() ) );

  loadAddressBook();
}

AddresseeDialog::~AddresseeDialog()
{
}

void AddresseeDialog::loadAddressBook()
{
  mAddresseeList->clear();
  mItemDict.clear();
  mAddresseeEdit->completionObject()->clear();

  AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    AddresseeItem *item = new AddresseeItem( mAddresseeList, (*it) );
    addCompletionItem( (*it).realName(), item );
    addCompletionItem( (*it).preferredEmail(), item );
  }
}

void AddresseeDialog::addCompletionItem( const TQString &str, TQListViewItem *item )
{
  if ( str.isEmpty() ) return;

  mItemDict.insert( str, item );
  mAddresseeEdit->completionObject()->addItem( str );
}

void AddresseeDialog::selectItem( const TQString &str )
{
  if ( str.isEmpty() ) return;

  TQListViewItem *item = mItemDict.find( str );
  if ( item ) {
    mAddresseeList->blockSignals( true );
    mAddresseeList->setSelected( item, true );
    mAddresseeList->ensureItemVisible( item );
    mAddresseeList->blockSignals( false );
  }
}

void AddresseeDialog::updateEdit( TQListViewItem *item )
{
  mAddresseeEdit->setText( item->text( 0 ) );
  mAddresseeEdit->setSelection( 0, item->text( 0 ).length() );
}

void AddresseeDialog::addSelected( TQListViewItem *item )
{
  AddresseeItem *addrItem = dynamic_cast<AddresseeItem *>( item );
  if ( !addrItem ) return;

  Addressee a = addrItem->addressee();

  TQListViewItem *selectedItem = mSelectedDict.find( a.uid() );
  if ( !selectedItem ) {
    selectedItem = new AddresseeItem( mSelectedList, a );
    mSelectedDict.insert( a.uid(), selectedItem );
  }
}

void AddresseeDialog::removeSelected()
{
  TQListViewItem *item = mSelectedList->selectedItem();
  AddresseeItem *addrItem = dynamic_cast<AddresseeItem *>( item );
  if ( !addrItem ) return;

  mSelectedDict.remove( addrItem->addressee().uid() );
  delete addrItem;
}

Addressee AddresseeDialog::addressee()
{
  AddresseeItem *aItem = 0;

  if ( mMultiple )
    aItem = dynamic_cast<AddresseeItem *>( mSelectedList->firstChild() );
  else
    aItem = dynamic_cast<AddresseeItem *>( mAddresseeList->selectedItem() );

  if (aItem) return aItem->addressee();
  return Addressee();
}

Addressee::List AddresseeDialog::addressees()
{
  Addressee::List al;
  AddresseeItem *aItem = 0;

  if ( mMultiple ) {
    TQListViewItem *item = mSelectedList->firstChild();
    while( item ) {
      aItem = dynamic_cast<AddresseeItem *>( item );
      if ( aItem ) al.append( aItem->addressee() );
      item = item->nextSibling();
    }
  }
  else
  {
    aItem = dynamic_cast<AddresseeItem *>( mAddresseeList->selectedItem() );
    if (aItem) al.append( aItem->addressee() );
  }

  return al;
}

Addressee AddresseeDialog::getAddressee( TQWidget *parent )
{
  AddresseeDialog *dlg = new AddresseeDialog( parent );
  Addressee addressee;
  int result = dlg->exec();

  if ( result == TQDialog::Accepted ) {
    addressee =  dlg->addressee();
  }

  delete dlg;
  return addressee;
}

Addressee::List AddresseeDialog::getAddressees( TQWidget *parent )
{
  AddresseeDialog *dlg = new AddresseeDialog( parent, true );
  Addressee::List addressees;
  int result = dlg->exec();
  if ( result == TQDialog::Accepted ) {
    addressees =  dlg->addressees();
  }

  delete dlg;
  return addressees;
}

void AddresseeDialog::addressBookChanged()
{
  loadAddressBook();
}
