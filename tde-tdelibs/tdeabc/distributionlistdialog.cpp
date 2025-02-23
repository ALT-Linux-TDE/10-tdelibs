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

#include <tqlistview.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqcombobox.h>
#include <kinputdialog.h>
#include <tqbuttongroup.h>
#include <tqradiobutton.h>

#include <tdelocale.h>
#include <kdebug.h>
#include <tdemessagebox.h>

#include "addressbook.h"
#include "addresseedialog.h"
#include "distributionlist.h"

#include "distributionlistdialog.h"
#include "distributionlistdialog.moc"

using namespace TDEABC;

DistributionListDialog::DistributionListDialog( AddressBook *addressBook, TQWidget *parent)
    : KDialogBase( parent, "", true, i18n("Configure Distribution Lists"), Ok, Ok, true)
{
  mEditor = new DistributionListEditorWidget( addressBook, this );
  setMainWidget( mEditor );

  connect( this, TQ_SIGNAL( okClicked() ), mEditor, TQ_SLOT( save() ) );
}

DistributionListDialog::~DistributionListDialog()
{
}

// TODO KDE4: Add d-pointer to EmailSelector, make sEmailMap a member variable
static TQMap<TQWidget*,TQString> *sEmailMap = 0;

EmailSelector::EmailSelector( const TQStringList &emails, const TQString &current,
                                      TQWidget *parent ) :
  KDialogBase( KDialogBase::Plain, i18n("Select Email Address"), Ok, Ok,
               parent )
{
  if (!sEmailMap)
     sEmailMap = new TQMap<TQWidget*,TQString>();
  TQFrame *topFrame = plainPage();
  TQBoxLayout *topLayout = new TQVBoxLayout( topFrame );

  mButtonGroup = new TQButtonGroup( 1, TQt::Horizontal, i18n("Email Addresses"),
                                   topFrame );
  topLayout->addWidget( mButtonGroup );

  TQStringList::ConstIterator it;
  for( it = emails.begin(); it != emails.end(); ++it ) {
    TQRadioButton *button = new TQRadioButton( *it, mButtonGroup );
    sEmailMap->insert( button, *it );
    if ( (*it) == current ) {
      mButtonGroup->setButton(mButtonGroup->id(button));
    }
  }
}

TQString EmailSelector::selected()
{
  TQButton *button = mButtonGroup->selected();
  if ( button ) return (*sEmailMap)[button];
  return TQString::null;
}

TQString EmailSelector::getEmail( const TQStringList &emails, const TQString &current,
                                     TQWidget *parent )
{
  EmailSelector *dlg = new EmailSelector( emails, current, parent );
  dlg->exec();

  TQString result = dlg->selected();

  delete dlg;

  return result;
}

class EntryItem : public TQListViewItem
{
  public:
    EntryItem( TQListView *parent, const Addressee &addressee,
               const TQString &email=TQString::null ) :
      TQListViewItem( parent ),
      mAddressee( addressee ),
      mEmail( email )
    {
      setText( 0, addressee.realName() );
      if( email.isEmpty() ) {
        setText( 1, addressee.preferredEmail() );
        setText( 2, i18n("Yes") );
      } else {
        setText( 1, email );
        setText( 2, i18n("No") );
      }
    }

    Addressee addressee() const
    {
      return mAddressee;
    }

    TQString email() const
    {
      return mEmail;
    }

  private:
    Addressee mAddressee;
    TQString mEmail;
};

DistributionListEditorWidget::DistributionListEditorWidget( AddressBook *addressBook, TQWidget *parent) :
  TQWidget( parent ),
  mAddressBook( addressBook )
{
  kdDebug(5700) << "DistributionListEditor()" << endl;

  TQBoxLayout *topLayout = new TQVBoxLayout( this );
  topLayout->setSpacing( KDialog::spacingHint() );

  TQBoxLayout *nameLayout = new TQHBoxLayout( topLayout) ;

  mNameCombo = new TQComboBox( this );
  nameLayout->addWidget( mNameCombo );
  connect( mNameCombo, TQ_SIGNAL( activated( int ) ), TQ_SLOT( updateEntryView() ) );

  mNewButton = new TQPushButton( i18n("New List..."), this );
  nameLayout->addWidget( mNewButton );
  connect( mNewButton, TQ_SIGNAL( clicked() ), TQ_SLOT( newList() ) );

  mEditButton = new TQPushButton( i18n("Rename List..."), this );
  nameLayout->addWidget( mEditButton );
  connect( mEditButton, TQ_SIGNAL( clicked() ), TQ_SLOT( editList() ) );

  mRemoveButton = new TQPushButton( i18n("Remove List"), this );
  nameLayout->addWidget( mRemoveButton );
  connect( mRemoveButton, TQ_SIGNAL( clicked() ), TQ_SLOT( removeList() ) );

  TQGridLayout *gridLayout = new TQGridLayout( topLayout, 3, 3 );
  gridLayout->setColStretch(1, 1);

  TQLabel *listLabel = new TQLabel( i18n("Available addresses:"), this );
  gridLayout->addWidget( listLabel, 0, 0 );

  mListLabel = new TQLabel( this );
  gridLayout->addMultiCellWidget( mListLabel, 0, 0, 1, 2 );

  mAddresseeView = new TQListView( this );
  mAddresseeView->addColumn( i18n("Name") );
  mAddresseeView->addColumn( i18n("Preferred Email") );
  mAddresseeView->setAllColumnsShowFocus( true );
  gridLayout->addWidget( mAddresseeView, 1, 0 );
  connect( mAddresseeView, TQ_SIGNAL( selectionChanged() ),
           TQ_SLOT( slotSelectionAddresseeViewChanged() ) );
  connect( mAddresseeView, TQ_SIGNAL( doubleClicked( TQListViewItem * ) ),
           TQ_SLOT( addEntry() ) );

  mAddEntryButton = new TQPushButton( i18n("Add Entry"), this );
  mAddEntryButton->setEnabled(false);
  gridLayout->addWidget( mAddEntryButton, 2, 0 );
  connect( mAddEntryButton, TQ_SIGNAL( clicked() ), TQ_SLOT( addEntry() ) );

  mEntryView = new TQListView( this );
  mEntryView->addColumn( i18n("Name") );
  mEntryView->addColumn( i18n("Email") );
  mEntryView->addColumn( i18n("Use Preferred") );
  mEntryView->setEnabled(false);
  mEntryView->setAllColumnsShowFocus( true );
  gridLayout->addMultiCellWidget( mEntryView, 1, 1, 1, 2 );
  connect( mEntryView, TQ_SIGNAL( selectionChanged() ),
           TQ_SLOT( slotSelectionEntryViewChanged() ) );

  mChangeEmailButton = new TQPushButton( i18n("Change Email..."), this );
  gridLayout->addWidget( mChangeEmailButton, 2, 1 );
  connect( mChangeEmailButton, TQ_SIGNAL( clicked() ), TQ_SLOT( changeEmail() ) );

  mRemoveEntryButton = new TQPushButton( i18n("Remove Entry"), this );
  gridLayout->addWidget( mRemoveEntryButton, 2, 2 );
  connect( mRemoveEntryButton, TQ_SIGNAL( clicked() ), TQ_SLOT( removeEntry() ) );

  mManager = new DistributionListManager( mAddressBook );
  mManager->load();

  updateAddresseeView();
  updateNameCombo();
}

DistributionListEditorWidget::~DistributionListEditorWidget()
{
  kdDebug(5700) << "~DistributionListEditor()" << endl;

  delete mManager;
}

void DistributionListEditorWidget::save()
{
  mManager->save();
}

void DistributionListEditorWidget::slotSelectionEntryViewChanged()
{
  EntryItem *entryItem = static_cast<EntryItem *>( mEntryView->selectedItem() );
  bool state=entryItem;

  mChangeEmailButton->setEnabled(state);
  mRemoveEntryButton->setEnabled(state);
}

void DistributionListEditorWidget::newList()
{
  bool ok;
  TQString name = KInputDialog::getText( i18n( "New Distribution List" ),
    i18n( "Please enter &name:" ), TQString::null, &ok );
  if (!ok) return;

  new DistributionList( mManager, name );

  mNameCombo->clear();
  mNameCombo->insertStringList( mManager->listNames() );
  mNameCombo->setCurrentItem( mNameCombo->count() - 1 );

  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditorWidget::editList()
{
  TQString oldName = mNameCombo->currentText();
  bool ok;
  TQString name = KInputDialog::getText( i18n( "Distribution List" ),
    i18n( "Please change &name:" ), oldName, &ok );
  if (!ok) return;

  DistributionList *list = mManager->list( oldName );
  list->setName( name );

  mNameCombo->clear();
  mNameCombo->insertStringList( mManager->listNames() );
  mNameCombo->setCurrentItem( mNameCombo->count() - 1 );

  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditorWidget::removeList()
{
  int result = KMessageBox::warningContinueCancel( this,
      i18n("Delete distribution list '%1'?") .arg( mNameCombo->currentText() ),
      TQString::null, KStdGuiItem::del() );

  if ( result != KMessageBox::Continue ) return;

  mManager->remove( mManager->list( mNameCombo->currentText() ) );
  mNameCombo->removeItem( mNameCombo->currentItem() );

  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditorWidget::addEntry()
{
  AddresseeItem *addresseeItem =
      static_cast<AddresseeItem *>( mAddresseeView->selectedItem() );

  if( !addresseeItem ) {
    kdDebug(5700) << "DLE::addEntry(): No addressee selected." << endl;
    return;
  }

  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) {
    kdDebug(5700) << "DLE::addEntry(): No dist list '" << mNameCombo->currentText() << "'" << endl;
    return;
  }

  list->insertEntry( addresseeItem->addressee() );
  updateEntryView();
  slotSelectionAddresseeViewChanged();
}

void DistributionListEditorWidget::removeEntry()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  EntryItem *entryItem =
      static_cast<EntryItem *>( mEntryView->selectedItem() );
  if ( !entryItem ) return;

  list->removeEntry( entryItem->addressee(), entryItem->email() );
  delete entryItem;
}

void DistributionListEditorWidget::changeEmail()
{
  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) return;

  EntryItem *entryItem =
      static_cast<EntryItem *>( mEntryView->selectedItem() );
  if ( !entryItem ) return;

  TQString email = EmailSelector::getEmail( entryItem->addressee().emails(),
                                           entryItem->email(), this );
  list->removeEntry( entryItem->addressee(), entryItem->email() );
  list->insertEntry( entryItem->addressee(), email );

  updateEntryView();
}

void DistributionListEditorWidget::updateEntryView()
{
  if ( mNameCombo->currentText().isEmpty() ) {
    mListLabel->setText( i18n("Selected addressees:") );
  } else {
    mListLabel->setText( i18n("Selected addresses in '%1':")
                         .arg( mNameCombo->currentText() ) );
  }

  mEntryView->clear();

  DistributionList *list = mManager->list( mNameCombo->currentText() );
  if ( !list ) {
    mEditButton->setEnabled(false);
    mRemoveButton->setEnabled(false);
    mChangeEmailButton->setEnabled(false);
    mRemoveEntryButton->setEnabled(false);
    mAddresseeView->setEnabled(false);
    mEntryView->setEnabled(false);
    return;
  } else {
    mEditButton->setEnabled(true);
    mRemoveButton->setEnabled(true);
    mAddresseeView->setEnabled(true);
    mEntryView->setEnabled(true);
  }

  DistributionList::Entry::List entries = list->entries();
  DistributionList::Entry::List::ConstIterator it;
  for( it = entries.begin(); it != entries.end(); ++it ) {
    new EntryItem( mEntryView, (*it).addressee, (*it).email );
  }

  EntryItem *entryItem = static_cast<EntryItem *>( mEntryView->selectedItem() );
  bool state=entryItem;

  mChangeEmailButton->setEnabled(state);
  mRemoveEntryButton->setEnabled(state);
}

void DistributionListEditorWidget::updateAddresseeView()
{
  mAddresseeView->clear();

  AddressBook::Iterator it;
  for( it = mAddressBook->begin(); it != mAddressBook->end(); ++it ) {
    new AddresseeItem( mAddresseeView, *it );
  }
}

void DistributionListEditorWidget::updateNameCombo()
{
  mNameCombo->insertStringList( mManager->listNames() );

  updateEntryView();
}

void DistributionListEditorWidget::slotSelectionAddresseeViewChanged()
{
  AddresseeItem *addresseeItem =
      static_cast<AddresseeItem *>( mAddresseeView->selectedItem() );
  bool state=addresseeItem;
  mAddEntryButton->setEnabled( state && !mNameCombo->currentText().isEmpty());
}
