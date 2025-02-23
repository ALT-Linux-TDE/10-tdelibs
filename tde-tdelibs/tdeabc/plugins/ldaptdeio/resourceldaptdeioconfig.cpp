/*
    This file is part of libtdeabc.
    Copyright (c) 2002 - 2003 Tobias Koenig <tokoe@kde.org>

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
#include <tqapplication.h>

#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqspinbox.h>
#include <tqvbox.h>
#include <tqvgroupbox.h>
#include <tqhbuttongroup.h>
#include <tqradiobutton.h>

#include <tdeaccelmanager.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <tdelocale.h>
#include <klineedit.h>
#include <tdemessagebox.h>
#include <tdeio/netaccess.h>

#include "resourceldaptdeio.h"

#include "resourceldaptdeioconfig.h"
#include "resourceldaptdeioconfig.moc"

using namespace TDEABC;

ResourceLDAPTDEIOConfig::ResourceLDAPTDEIOConfig( TQWidget* parent,  const char* name )
  : KRES::ConfigWidget( parent, name )
{
  TQBoxLayout *mainLayout = new TQVBoxLayout( this );
  mainLayout->setAutoAdd( true );
  cfg = new LdapConfigWidget( LdapConfigWidget::W_ALL, this );

  mSubTree = new TQCheckBox( i18n( "Sub-tree query" ), this );
  TQHBox *box = new TQHBox( this );
  box->setSpacing( KDialog::spacingHint() );
  mEditButton = new TQPushButton( i18n( "Edit Attributes..." ), box );
  mCacheButton = new TQPushButton( i18n( "Offline Use..." ), box );

  connect( mEditButton, TQ_SIGNAL( clicked() ), TQ_SLOT( editAttributes() ) );
  connect( mCacheButton, TQ_SIGNAL( clicked() ), TQ_SLOT( editCache() ) );
}

void ResourceLDAPTDEIOConfig::loadSettings( KRES::Resource *res )
{
  ResourceLDAPTDEIO *resource = dynamic_cast<ResourceLDAPTDEIO*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceLDAPTDEIOConfig::loadSettings(): cast failed" << endl;
    return;
  }

  cfg->setUser( resource->user() );
  cfg->setPassword( resource->password() );
  cfg->setRealm( resource->realm() );
  cfg->setBindDN( resource->bindDN() );
  cfg->setHost( resource->host() );
  cfg->setPort(  resource->port() );
  cfg->setVer(  resource->ver() );
  cfg->setTimeLimit( resource->timeLimit() );
  cfg->setSizeLimit( resource->sizeLimit() );
  cfg->setDn( resource->dn() );
  cfg->setFilter( resource->filter() );
  cfg->setMech( resource->mech() );
  if ( resource->isTLS() ) cfg->setSecTLS();
  else if ( resource->isSSL() ) cfg->setSecSSL();
  else cfg->setSecNO();
  if ( resource->isAnonymous() ) cfg->setAuthAnon();
  else if ( resource->isSASL() ) cfg->setAuthSASL();
  else cfg->setAuthSimple();
  
  mSubTree->setChecked( resource->isSubTree() );
  mAttributes = resource->attributes();
  mRDNPrefix = resource->RDNPrefix();
  mCachePolicy = resource->cachePolicy();
  mCacheDst = resource->cacheDst();
  mAutoCache = resource->autoCache();
}

void ResourceLDAPTDEIOConfig::saveSettings( KRES::Resource *res )
{
  ResourceLDAPTDEIO *resource = dynamic_cast<ResourceLDAPTDEIO*>( res );
  
  if ( !resource ) {
    kdDebug(5700) << "ResourceLDAPTDEIOConfig::saveSettings(): cast failed" << endl;
    return;
  }

  resource->setUser( cfg->user() );
  resource->setPassword( cfg->password() );
  resource->setRealm( cfg->realm() );
  resource->setBindDN( cfg->bindDN() );
  resource->setHost( cfg->host() );
  resource->setPort( cfg->port() );
  resource->setVer( cfg->ver() );
  resource->setTimeLimit( cfg->timeLimit() );
  resource->setSizeLimit( cfg->sizeLimit() );
  resource->setDn( cfg->dn() );
  resource->setFilter( cfg->filter() );
  resource->setIsAnonymous( cfg->isAuthAnon() );
  resource->setIsSASL( cfg->isAuthSASL() );
  resource->setMech( cfg->mech() );
  resource->setIsTLS( cfg->isSecTLS() );
  resource->setIsSSL( cfg->isSecSSL() );
  resource->setIsSubTree( mSubTree->isChecked() );
  resource->setAttributes( mAttributes );
  resource->setRDNPrefix( mRDNPrefix );
  resource->setCachePolicy( mCachePolicy );
  resource->init();

}

void ResourceLDAPTDEIOConfig::editAttributes()
{
  AttributesDialog dlg( mAttributes, mRDNPrefix, this );
  if ( dlg.exec() ) {
    mAttributes = dlg.attributes();
    mRDNPrefix = dlg.rdnprefix();
  }
}

void ResourceLDAPTDEIOConfig::editCache()
{
  LDAPUrl src;
  TQStringList attr;
  
  src = cfg->url();
  src.setScope( mSubTree->isChecked() ? LDAPUrl::Sub : LDAPUrl::One );
  if (!mAttributes.empty()) {
    TQMap<TQString,TQString>::Iterator it;
    TQStringList attr;
    for ( it = mAttributes.begin(); it != mAttributes.end(); ++it ) {
      if ( !it.data().isEmpty() && it.key() != "objectClass" ) 
        attr.append( it.data() );
    }
    src.setAttributes( attr );
  }
  src.setExtension( "x-dir", "base" );
  OfflineDialog dlg( mAutoCache, mCachePolicy, src, mCacheDst, this );
  if ( dlg.exec() ) {
    mCachePolicy = dlg.cachePolicy();
    mAutoCache = dlg.autoCache();
  }
  
}

AttributesDialog::AttributesDialog( const TQMap<TQString, TQString> &attributes,
                                    int rdnprefix,                                    
                                    TQWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Attributes Configuration" ), Ok | Cancel,
                 Ok, parent, name, true, true )
{
  mNameDict.setAutoDelete( true );
  mNameDict.insert( "objectClass", new TQString( i18n( "Object classes" ) ) );
  mNameDict.insert( "commonName", new TQString( i18n( "Common name" ) ) );
  mNameDict.insert( "formattedName", new TQString( i18n( "Formatted name" ) ) );
  mNameDict.insert( "familyName", new TQString( i18n( "Family name" ) ) );
  mNameDict.insert( "givenName", new TQString( i18n( "Given name" ) ) );
  mNameDict.insert( "organization", new TQString( i18n( "Organization" ) ) );
  mNameDict.insert( "title", new TQString( i18n( "Title" ) ) );
  mNameDict.insert( "street", new TQString( i18n( "Street" ) ) );
  mNameDict.insert( "state", new TQString( i18n( "State" ) ) );
  mNameDict.insert( "city", new TQString( i18n( "City" ) ) );
  mNameDict.insert( "postalcode", new TQString( i18n( "Postal code" ) ) );
  mNameDict.insert( "mail", new TQString( i18n( "Email" ) ) );
  mNameDict.insert( "mailAlias", new TQString( i18n( "Email alias" ) ) );
  mNameDict.insert( "phoneNumber", new TQString( i18n( "Telephone number" ) ) );
  mNameDict.insert( "telephoneNumber", new TQString( i18n( "Work telephone number" ) ) );
  mNameDict.insert( "facsimileTelephoneNumber", new TQString( i18n( "Fax number" ) ) );
  mNameDict.insert( "mobile", new TQString( i18n( "Cell phone number" ) ) );
  mNameDict.insert( "pager", new TQString( i18n( "Pager" ) ) );
  mNameDict.insert( "description", new TQString( i18n( "Note" ) ) );
  mNameDict.insert( "uid", new TQString( i18n( "UID" ) ) );
  mNameDict.insert( "jpegPhoto", new TQString( i18n( "Photo" ) ) );

  // default map
  mDefaultMap.insert( "objectClass", "inetOrgPerson" );
  mDefaultMap.insert( "commonName", "cn" );
  mDefaultMap.insert( "formattedName", "displayName" );
  mDefaultMap.insert( "familyName", "sn" );
  mDefaultMap.insert( "givenName", "givenName" );
  mDefaultMap.insert( "title", "title" );
  mDefaultMap.insert( "street", "street" );
  mDefaultMap.insert( "state", "st" );
  mDefaultMap.insert( "city", "l" );
  mDefaultMap.insert( "organization", "o" );
  mDefaultMap.insert( "postalcode", "postalCode" );
  mDefaultMap.insert( "mail", "mail" );
  mDefaultMap.insert( "mailAlias", "" );
  mDefaultMap.insert( "phoneNumber", "homePhone" );
  mDefaultMap.insert( "telephoneNumber", "telephoneNumber" );
  mDefaultMap.insert( "facsimileTelephoneNumber", "facsimileTelephoneNumber" );
  mDefaultMap.insert( "mobile", "mobile" );
  mDefaultMap.insert( "pager", "pager" );
  mDefaultMap.insert( "description", "description" );
  mDefaultMap.insert( "uid", "uid" );
  mDefaultMap.insert( "jpegPhoto", "jpegPhoto" );
  
  // overwrite the default values here
  TQMap<TQString, TQString> kolabMap, netscapeMap, evolutionMap, outlookMap;

  // kolab
  kolabMap.insert( "formattedName", "display-name" );
  kolabMap.insert( "mailAlias", "mailalias" );

  // evolution
  evolutionMap.insert( "formattedName", "fileAs" );

  mMapList.append( attributes );
  mMapList.append( kolabMap );
  mMapList.append( netscapeMap );
  mMapList.append( evolutionMap );
  mMapList.append( outlookMap );

  TQFrame *page = plainPage();
  TQGridLayout *layout = new TQGridLayout( page, 4, ( attributes.count() + 4 ) >> 1,
                                         0, spacingHint() );

  TQLabel *label = new TQLabel( i18n( "Template:" ), page );
  layout->addWidget( label, 0, 0 );
  mMapCombo = new KComboBox( page );
  layout->addWidget( mMapCombo, 0, 1 );

  mMapCombo->insertItem( i18n( "User Defined" ) );
  mMapCombo->insertItem( i18n( "Kolab" ) );
  mMapCombo->insertItem( i18n( "Netscape" ) );
  mMapCombo->insertItem( i18n( "Evolution" ) );
  mMapCombo->insertItem( i18n( "Outlook" ) );
  connect( mMapCombo, TQ_SIGNAL( activated( int ) ), TQ_SLOT( mapChanged( int ) ) );

  label = new TQLabel( i18n( "RDN prefix attribute:" ), page );
  layout->addWidget( label, 1, 0 );
  mRDNCombo = new KComboBox( page );
  layout->addWidget( mRDNCombo, 1, 1 );
  mRDNCombo->insertItem( i18n( "commonName" ) );
  mRDNCombo->insertItem( i18n( "UID" ) );
  mRDNCombo->setCurrentItem( rdnprefix );

  TQMap<TQString, TQString>::ConstIterator it;
  int i, j = 0;
  for ( i = 2, it = attributes.begin(); it != attributes.end(); ++it, ++i ) {
    if ( mNameDict[ it.key() ] == 0 ) {
      i--;  
      continue;
    }
    if ( (uint)(i - 2) == ( mNameDict.count()  >> 1 ) ) {
      i = 0;
      j = 2;
    }
    kdDebug(7125) << "itkey: " << it.key() << " i: " << i << endl;
    label = new TQLabel( *mNameDict[ it.key() ] + ":", page );
    KLineEdit *lineedit = new KLineEdit( page );
    mLineEditDict.insert( it.key(), lineedit );
    lineedit->setText( it.data() );
    label->setBuddy( lineedit );
    layout->addWidget( label, i, j );
    layout->addWidget( lineedit, i, j+1 );
  }
  
  for ( i = 1; i < mMapCombo->count(); i++ ) {
    TQDictIterator<KLineEdit> it2( mLineEditDict );
    for ( ; it2.current(); ++it2 ) {
      if ( mMapList[ i ].contains( it2.currentKey() ) ) {
        if ( mMapList[ i ][ it2.currentKey() ] != it2.current()->text() ) break;
      } else {
        if ( mDefaultMap[ it2.currentKey() ] != it2.current()->text() ) break;
      }
    }
    if ( !it2.current() ) {
      mMapCombo->setCurrentItem( i );
      break;
    }
  }

  TDEAcceleratorManager::manage( this );
}

AttributesDialog::~AttributesDialog()
{
}

TQMap<TQString, TQString> AttributesDialog::attributes() const
{
  TQMap<TQString, TQString> map;

  TQDictIterator<KLineEdit> it( mLineEditDict );
  for ( ; it.current(); ++it )
    map.insert( it.currentKey(), it.current()->text() );

  return map;
}

int AttributesDialog::rdnprefix() const
{
  return mRDNCombo->currentItem();
}

void AttributesDialog::mapChanged( int pos )
{

  // apply first the default and than the spezific changes
  TQMap<TQString, TQString>::Iterator it;
  for ( it = mDefaultMap.begin(); it != mDefaultMap.end(); ++it )
    mLineEditDict[ it.key() ]->setText( it.data() );

  for ( it = mMapList[ pos ].begin(); it != mMapList[ pos ].end(); ++it ) {
    if ( !it.data().isEmpty() ) {
      KLineEdit *le = mLineEditDict[ it.key() ];
      if ( le ) le->setText( it.data() );
    }
  }
}

OfflineDialog::OfflineDialog( bool autoCache, int cachePolicy, const KURL &src, 
  const TQString &dst, TQWidget *parent, const char *name )
  : KDialogBase( Plain, i18n( "Offline Configuration" ), Ok | Cancel,
                 Ok, parent, name, true, true )
{
  TQFrame *page = plainPage();
  TQVBoxLayout *layout = new TQVBoxLayout( page );
  layout->setAutoAdd( true );

  mSrc = src; mDst = dst;
  mCacheGroup = new TQButtonGroup( 1, TQt::Horizontal, 
    i18n("Offline Cache Policy"), page );
    
  TQRadioButton *bt;
  new TQRadioButton( i18n("Do not use offline cache"), mCacheGroup );
  bt = new TQRadioButton( i18n("Use local copy if no connection"), mCacheGroup );
  new TQRadioButton( i18n("Always use local copy"), mCacheGroup );
  mCacheGroup->setButton( cachePolicy );  

  mAutoCache = new TQCheckBox( i18n("Refresh offline cache automatically"),
    page );
  mAutoCache->setChecked( autoCache );
  mAutoCache->setEnabled( bt->isChecked() );

  connect( bt, TQ_SIGNAL(toggled(bool)), mAutoCache, TQ_SLOT(setEnabled(bool)) );
  
  TQPushButton *lcache = new TQPushButton( i18n("Load into Cache"), page );
  connect( lcache, TQ_SIGNAL( clicked() ), TQ_SLOT( loadCache() ) );
}

OfflineDialog::~OfflineDialog()
{
}

bool OfflineDialog::autoCache() const
{
  return mAutoCache->isChecked();
}

int OfflineDialog::cachePolicy() const
{
  return mCacheGroup->selectedId();
}

void OfflineDialog::loadCache() 
{
  if ( TDEIO::NetAccess::download( mSrc, mDst, this ) ) {
    KMessageBox::information( this, 
      i18n("Successfully downloaded directory server contents!") );
  } else {
    KMessageBox::error( this, 
      i18n("An error occurred downloading directory server contents into file %1.").arg(mDst) );
  }
}
