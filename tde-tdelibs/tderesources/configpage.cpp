/*
    This file is part of libtderesources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>

#include <tdeapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kurlrequester.h>
#include <tdelistview.h>
#include <kbuttonbox.h>
#include <ktrader.h>
#include <kinputdialog.h>

#include "resource.h"
#include "configdialog.h"

#include "configpage.h"

namespace KRES {

ResourcePageInfo::ResourcePageInfo() : TDEShared() {
  mManager = 0L;
  mConfig = 0L;
}

ResourcePageInfo::~ResourcePageInfo() {
  //delete mManager;
  mManager = 0L;
  //delete mConfig;
  mConfig = 0L;
}


class ConfigViewItem : public TQCheckListItem
{
  public:
    ConfigViewItem( TQListView *parent, Resource* resource ) :
      TQCheckListItem( parent, resource->resourceName(), CheckBox ),
      mResource( resource ),
      mIsStandard( false )
    {
      setText( 1, mResource->type() );
      setOn( mResource->isActive() );
    }

    void setStandard( bool value )
    {
      setText( 2, ( value ? i18n( "Yes" ) : TQString::null ) );
      mIsStandard = value;
    }

    bool standard() const { return mIsStandard; }
    bool readOnly() const { return mResource->readOnly(); }

    Resource *resource() { return mResource; }

    void updateItem()
    {
      setOn( mResource->isActive() );
      setText( 0, mResource->resourceName() );
      setText( 1, mResource->type() );
      setText( 2, mIsStandard ? i18n( "Yes" ) : TQString::null );
    }

  private:
    Resource* mResource;

    bool mIsStandard;
};

ConfigPage::ConfigPage( TQWidget *parent, const char *name )
  : TQWidget( parent, name ),
    mCurrentManager( 0 ),
    mCurrentConfig( 0 )
{
  setCaption( i18n( "Resource Configuration" ) );

  TQVBoxLayout *mainLayout = new TQVBoxLayout( this );

  TQGroupBox *groupBox = new TQGroupBox( i18n( "Resources" ), this );
  groupBox->setColumnLayout(0, TQt::Vertical );
  groupBox->layout()->setSpacing( 6 );
  groupBox->layout()->setMargin( 11 );
  TQGridLayout *groupBoxLayout = new TQGridLayout( groupBox->layout(), 2, 2 );

  mFamilyCombo = new KComboBox( false, groupBox );
  groupBoxLayout->addMultiCellWidget( mFamilyCombo, 0, 0, 0, 1 );

  mListView = new TDEListView( groupBox );
  mListView->setAllColumnsShowFocus( true );
  mListView->setFullWidth( true );
  mListView->addColumn( i18n( "Name" ) );
  mListView->addColumn( i18n( "Type" ) );
  mListView->addColumn( i18n( "Standard" ) );

  groupBoxLayout->addWidget( mListView, 1, 0 );
  connect(  mListView, TQ_SIGNAL( doubleClicked( TQListViewItem *, const TQPoint &, int ) ), this, TQ_SLOT( slotEdit() ) );
  KButtonBox *buttonBox = new KButtonBox( groupBox, TQt::Vertical );
  mAddButton = buttonBox->addButton( i18n( "&Add..." ), this, TQ_SLOT(slotAdd()) );
  mRemoveButton = buttonBox->addButton( i18n( "&Remove" ), this, TQ_SLOT(slotRemove()) );
  mRemoveButton->setEnabled( false );
  mEditButton = buttonBox->addButton( i18n( "&Edit..." ), this, TQ_SLOT(slotEdit()) );
  mEditButton->setEnabled( false );
  mStandardButton = buttonBox->addButton( i18n( "&Use as Standard" ), this, TQ_SLOT(slotStandard()) );
  mStandardButton->setEnabled( false );
  buttonBox->layout();

  groupBoxLayout->addWidget( buttonBox, 1, 1 );

  mainLayout->addWidget( groupBox );

  connect( mFamilyCombo, TQ_SIGNAL( activated( int ) ),
           TQ_SLOT( slotFamilyChanged( int ) ) );
  connect( mListView, TQ_SIGNAL( selectionChanged() ),
           TQ_SLOT( slotSelectionChanged() ) );
  connect( mListView, TQ_SIGNAL( clicked( TQListViewItem * ) ),
           TQ_SLOT( slotItemClicked( TQListViewItem * ) ) );

  mLastItem = 0;

  mConfig = new TDEConfig( "kcmtderesourcesrc" );
  mConfig->setGroup( "General" );

  load();
}

ConfigPage::~ConfigPage()
{
  TQValueList<TDESharedPtr<ResourcePageInfo> >::Iterator it;
  for ( it = mInfoMap.begin(); it != mInfoMap.end(); ++it ) {
    (*it)->mManager->removeObserver( this );
  }

  mConfig->writeEntry( "CurrentFamily", mFamilyCombo->currentItem() );
  delete mConfig;
  mConfig = 0;
}

void ConfigPage::load()
{
  kdDebug(5650) << "ConfigPage::load()" << endl;

  mListView->clear();
  mFamilyMap.clear();
  mInfoMap.clear();
  TQStringList familyDisplayNames;

  // KDE-3.3 compatibility code: get families from the plugins
  TQStringList compatFamilyNames;
  const TDETrader::OfferList plugins = TDETrader::self()->query( "TDEResources/Plugin" );
  TDETrader::OfferList::ConstIterator it = plugins.begin();
  TDETrader::OfferList::ConstIterator end = plugins.end();
  for ( ; it != end; ++it ) {
    const TQString family = (*it)->property( "X-TDE-ResourceFamily" ).toString();
    if ( compatFamilyNames.find( family ) == compatFamilyNames.end() )
        compatFamilyNames.append( family );
  }

  const TDETrader::OfferList managers = TDETrader::self()->query( "TDEResources/Manager" );
  TDETrader::OfferList::ConstIterator m_it;
  for( m_it = managers.begin(); m_it != managers.end(); ++m_it ) {
    TQString displayName = (*m_it)->property( "Name" ).toString();
    familyDisplayNames.append( displayName );
    TQString family = (*m_it)->property( "X-TDE-ResourceFamily" ).toString();
    if ( !family.isEmpty() ) {
      compatFamilyNames.remove( family );
      mFamilyMap.append( family );
      loadManager( family );
    }
  }

  // Rest of the kde-3.3 compat code
  TQStringList::ConstIterator cfit = compatFamilyNames.begin();
  for ( ; cfit != compatFamilyNames.end(); ++cfit ) {
      mFamilyMap.append( *cfit );
      familyDisplayNames.append( *cfit );
      loadManager( *cfit );
  }

  mCurrentManager = 0;

  mFamilyCombo->clear();
  mFamilyCombo->insertStringList( familyDisplayNames );

  int currentFamily = mConfig->readNumEntry( "CurrentFamily", 0 );
  mFamilyCombo->setCurrentItem( currentFamily );
  slotFamilyChanged( currentFamily );
  emit changed( false );
}

void ConfigPage::loadManager( const TQString& family )
{
  mCurrentManager = new Manager<Resource>( family );
  if ( mCurrentManager ) {
      mCurrentManager->addObserver( this );

      ResourcePageInfo *info = new ResourcePageInfo;
      info->mManager = mCurrentManager;
      info->mConfig = new TDEConfig( KRES::ManagerImpl::defaultConfigFile( family ) );
      info->mManager->readConfig( info->mConfig );

      mInfoMap.append( TDESharedPtr<ResourcePageInfo>(info) );
  }
}

void ConfigPage::save()
{
  saveResourceSettings();

  TQValueList<TDESharedPtr<ResourcePageInfo> >::Iterator it;
  for ( it = mInfoMap.begin(); it != mInfoMap.end(); ++it )
    (*it)->mManager->writeConfig( (*it)->mConfig );

  emit changed( false );
}

void ConfigPage::defaults()
{
}

void ConfigPage::slotFamilyChanged( int pos )
{
  if ( pos < 0 || pos >= (int)mFamilyMap.count() )
    return;

  saveResourceSettings();

  mFamily = mFamilyMap[ pos ];

  mCurrentManager = mInfoMap[ pos ]->mManager;
  mCurrentConfig = mInfoMap[ pos ]->mConfig;

  if ( !mCurrentManager )
    kdDebug(5650) << "ERROR: cannot create ResourceManager<Resource>( mFamily )" << endl;

  mListView->clear();

  if ( mCurrentManager->isEmpty() )
    defaults();

  Resource *standardResource = mCurrentManager->standardResource();

  Manager<Resource>::Iterator it;
  for ( it = mCurrentManager->begin(); it != mCurrentManager->end(); ++it ) {
    ConfigViewItem *item = new ConfigViewItem( mListView, *it );
    if ( *it == standardResource )
      item->setStandard( true );
  }

  if ( mListView->childCount() == 0 ) {
    defaults();
    emit changed( true );
    mCurrentManager->writeConfig( mCurrentConfig );
  } else {
    if ( !standardResource )
      KMessageBox::sorry( this, i18n( "There is no standard resource! Please select one." ) );

    emit changed( false );
  }
}

void ConfigPage::slotAdd()
{
  if ( !mCurrentManager )
    return;

  TQStringList types = mCurrentManager->resourceTypeNames();
  TQStringList descs = mCurrentManager->resourceTypeDescriptions();
  bool ok = false;
  TQString desc = KInputDialog::getItem( i18n( "Resource Configuration" ),
                    i18n( "Please select type of the new resource:" ), descs,
                    0, false, &ok, this );
  if ( !ok )
    return;

  TQString type = types[ descs.findIndex( desc ) ];

  // Create new resource
  Resource *resource = mCurrentManager->createResource( type );
  if ( !resource ) {
    KMessageBox::error( this, i18n("Unable to create resource of type '%1'.")
                              .arg( type ) );
    return;
  }

  resource->setResourceName( type + "-resource" );

  ConfigDialog dlg( this, mFamily, resource, "KRES::ConfigDialog" );

  if ( dlg.exec() ) {
    mCurrentManager->add( resource );

    ConfigViewItem *item = new ConfigViewItem( mListView, resource );

    mLastItem = item;

    // if there are only read-only resources we'll set this resource
    // as standard resource
    if ( !resource->readOnly() ) {
      bool onlyReadOnly = true;
      TQListViewItem *it = mListView->firstChild();
      while ( it != 0 ) {
        ConfigViewItem *confIt = static_cast<ConfigViewItem*>( it );
        if ( !confIt->readOnly() && confIt != item )
          onlyReadOnly = false;

        it = it->itemBelow();
      }

      if ( onlyReadOnly )
        item->setStandard( true );
    }

    emit changed( true );
  } else {
    delete resource;
    resource = 0;
  }
}

void ConfigPage::slotRemove()
{
  if ( !mCurrentManager )
    return;

  TQListViewItem *item = mListView->currentItem();
  ConfigViewItem *confItem = static_cast<ConfigViewItem*>( item );

  if ( !confItem )
    return;

  if ( confItem->standard() ) {
    KMessageBox::sorry( this, i18n( "You cannot remove your standard resource! Please select a new standard resource first." ) );
    return;
  }

  mCurrentManager->remove( confItem->resource() );

  if ( item == mLastItem )
    mLastItem = 0;

  mListView->takeItem( item );
  delete item;

  emit changed( true );
}

void ConfigPage::slotEdit()
{
  if ( !mCurrentManager )
    return;

  TQListViewItem *item = mListView->currentItem();
  ConfigViewItem *configItem = static_cast<ConfigViewItem*>( item );
  if ( !configItem )
    return;

  Resource *resource = configItem->resource();

  ConfigDialog dlg( this, mFamily, resource, "KRES::ConfigDialog" );

  if ( dlg.exec() ) {
    configItem->setText( 0, resource->resourceName() );
    configItem->setText( 1, resource->type() );

    if ( configItem->standard() && configItem->readOnly() ) {
      KMessageBox::sorry( this, i18n( "You cannot use a read-only resource as standard!" ) );
      configItem->setStandard( false );
    }

    mCurrentManager->change( resource );
    emit changed( true );
  }
}

void ConfigPage::slotStandard()
{
  if ( !mCurrentManager )
    return;

  ConfigViewItem *item = static_cast<ConfigViewItem*>( mListView->currentItem() );
  if ( !item )
    return;

  if ( item->readOnly() ) {
    KMessageBox::sorry( this, i18n( "You cannot use a read-only resource as standard!" ) );
    return;
  }

  if ( !item->isOn() ) {
    KMessageBox::sorry( this, i18n( "You cannot use an inactive resource as standard!" ) );
    return;
  }

  TQListViewItem *it = mListView->firstChild();
  while ( it != 0 ) {
    ConfigViewItem *configItem = static_cast<ConfigViewItem*>( it );
    if ( configItem->standard() )
      configItem->setStandard( false );
    it = it->itemBelow();
  }

  item->setStandard( true );
  mCurrentManager->setStandardResource( item->resource() );

  emit changed( true );
}

void ConfigPage::slotSelectionChanged()
{
  bool state = ( mListView->currentItem() != 0 );

  mRemoveButton->setEnabled( state );
  mEditButton->setEnabled( state );
  mStandardButton->setEnabled( state );
}

void ConfigPage::resourceAdded( Resource *resource )
{
  kdDebug(5650) << "ConfigPage::resourceAdded( " << resource->resourceName()
                << " )" << endl;

  ConfigViewItem *item = new ConfigViewItem( mListView, resource );

  item->setOn( resource->isActive() );

  mLastItem = item;

  emit changed( true );
}

void ConfigPage::resourceModified( Resource *resource )
{
  kdDebug(5650) << "ConfigPage::resourceModified( " << resource->resourceName()
                << " )" << endl;
  ConfigViewItem *item = findItem( resource );
  if ( !item ) return;

  // TODO: Reread resource config. Otherwise we won't see the modification.

  item->updateItem();
}

void ConfigPage::resourceDeleted( Resource *resource )
{
  kdDebug(5650) << "ConfigPage::resourceDeleted( " << resource->resourceName()
                << " )" << endl;

  ConfigViewItem *item = findItem( resource );
  if ( !item ) return;

  delete item;
}

ConfigViewItem *ConfigPage::findItem( Resource *resource )
{
  TQListViewItem *i;
  for( i = mListView->firstChild(); i; i = i->nextSibling() ) {
    ConfigViewItem *item = static_cast<ConfigViewItem *>( i );
    if ( item->resource() == resource ) return item;
  }
  return 0;
}

void ConfigPage::slotItemClicked( TQListViewItem *item )
{
  ConfigViewItem *configItem = static_cast<ConfigViewItem *>( item );
  if ( !configItem ) return;

  if ( configItem->standard() && !configItem->isOn() ) {
    KMessageBox::sorry( this, i18n( "You cannot deactivate the standard resource. Choose another standard resource first." ) );
    configItem->setOn( true );
    return;
  }

  if ( configItem->isOn() != configItem->resource()->isActive() ) {
    emit changed( true );
  }
}

void ConfigPage::saveResourceSettings()
{
  if ( mCurrentManager ) {
    TQListViewItem *item = mListView->firstChild();
    while ( item ) {
      ConfigViewItem *configItem = static_cast<ConfigViewItem *>( item );

      // check if standard resource
      if ( configItem->standard() && !configItem->readOnly() &&
           configItem->isOn() )
        mCurrentManager->setStandardResource( configItem->resource() );

      // check if active or passive resource
      configItem->resource()->setActive( configItem->isOn() );

      item = item->nextSibling();
    }
    mCurrentManager->writeConfig( mCurrentConfig );

    if ( !mCurrentManager->standardResource() )
      KMessageBox::sorry( this, i18n( "There is no valid standard resource! Please select one which is neither read-only nor inactive." ) );
  }
}

}

#include "configpage.moc"

