/*
    This file is part of libtdeabc.
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

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

#include <tqobjectlist.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqspinbox.h>
#include <tqvgroupbox.h>
#include <tqhbuttongroup.h>
#include <tqradiobutton.h>

#include <tdemessagebox.h>
#include <tdeaccelmanager.h>
#include <kdialogbase.h>
#include <tdelocale.h>
#include <klineedit.h>
#include <kcombobox.h>
#include <kprogress.h>

#include <kdebug.h>

#include "ldapconfigwidget.h"
#include "ldapconfigwidget.moc"

using namespace TDEABC;

LdapConfigWidget::LdapConfigWidget( TQWidget* parent,
  const char* name, WFlags fl ) : TQWidget( parent, name, fl )
{
  mProg = 0;
  mFlags = 0;
  mainLayout = new TQGridLayout( this, 12, 4, 0,
      KDialog::spacingHint() );
}

LdapConfigWidget::LdapConfigWidget( int flags, TQWidget* parent,
  const char* name, WFlags fl ) : TQWidget( parent, name, fl )
{
  mFlags = flags;
  mProg = 0;
  mainLayout = new TQGridLayout( this, 12, 4, 0,
      KDialog::spacingHint() );
  initWidget();
}

LdapConfigWidget::~LdapConfigWidget()
{
}

void LdapConfigWidget::initWidget()
{
  TQLabel *label;

  mUser = mPassword = mHost = mDn = mBindDN = mRealm = mFilter = 0;
  mPort = mVer = mTimeLimit = mSizeLimit = 0;
  mAnonymous = mSimple = mSASL = mSecNO = mSecTLS = mSecSSL = 0;
  mEditButton =  mQueryMech = 0;
  mMech = 0;
  int row = 0;
  int col;
  
  if ( mFlags & W_USER ) {
    label = new TQLabel( i18n( "User:" ), this );
    mUser = new KLineEdit( this, "kcfg_ldapuser" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mUser, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_BINDDN ) {
    label = new TQLabel( i18n( "Bind DN:" ), this );
    mBindDN = new KLineEdit( this, "kcfg_ldapbinddn" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mBindDN, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_REALM ) {
    label = new TQLabel( i18n( "Realm:" ), this );
    mRealm = new KLineEdit( this, "kcfg_ldaprealm" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mRealm, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_PASS ) {
    label = new TQLabel( i18n( "Password:" ), this );
    mPassword = new KLineEdit( this, "kcfg_ldappassword" );
    mPassword->setEchoMode( KLineEdit::Password );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mPassword, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_HOST ) {
    label = new TQLabel( i18n( "Host:" ), this );
    mHost = new KLineEdit( this, "kcfg_ldaphost" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mHost, row, row, 1, 3 );
    row++;
  }

  col = 0;
  if ( mFlags & W_PORT ) {
    label = new TQLabel( i18n( "Port:" ), this );
    mPort = new TQSpinBox( 0, 65535, 1, this, "kcfg_ldapport" );
    mPort->setSizePolicy( TQSizePolicy( TQSizePolicy::Maximum, TQSizePolicy::Preferred ) );
    mPort->setValue( 389 );

    mainLayout->addWidget( label, row, col );
    mainLayout->addWidget( mPort, row, col+1 );
    col += 2;
  }

  if ( mFlags & W_VER ) {
    label = new TQLabel( i18n( "LDAP version:" ), this );
    mVer = new TQSpinBox( 2, 3, 1, this, "kcfg_ldapver" );
    mVer->setSizePolicy( TQSizePolicy( TQSizePolicy::Maximum, TQSizePolicy::Preferred ) );
    mVer->setValue( 3 );
    mainLayout->addWidget( label, row, col );
    mainLayout->addWidget( mVer, row, col+1 );
  }
  if ( mFlags & ( W_PORT | W_VER ) ) row++;

  col = 0;
  if ( mFlags & W_SIZELIMIT ) {
    label = new TQLabel( i18n( "Size limit:" ), this );
    mSizeLimit = new TQSpinBox( 0, 9999999, 1, this, "kcfg_ldapsizelimit" );
    mSizeLimit->setSizePolicy( TQSizePolicy( TQSizePolicy::Maximum, TQSizePolicy::Preferred ) );
    mSizeLimit->setValue( 0 );
    mSizeLimit->setSpecialValueText( i18n("Default") );
    mainLayout->addWidget( label, row, col );
    mainLayout->addWidget( mSizeLimit, row, col+1 );
    col += 2;
  }

  if ( mFlags & W_TIMELIMIT ) {
    label = new TQLabel( i18n( "Time limit:" ), this );
    mTimeLimit = new TQSpinBox( 0, 9999999, 1, this, "kcfg_ldaptimelimit" );
    mTimeLimit->setSizePolicy( TQSizePolicy( TQSizePolicy::Maximum, TQSizePolicy::Preferred ) );
    mTimeLimit->setValue( 0 );
    mTimeLimit->setSuffix( i18n(" sec") );
    mTimeLimit->setSpecialValueText( i18n("Default") );
    mainLayout->addWidget( label, row, col );
    mainLayout->addWidget( mTimeLimit, row, col+1 );
  }
  if ( mFlags & ( W_SIZELIMIT | W_TIMELIMIT ) ) row++;

  if ( mFlags & W_DN ) {
    label = new TQLabel( i18n( "Distinguished Name", "DN:" ), this );
    mDn = new KLineEdit( this, "kcfg_ldapdn" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mDn, row, row, 1, 1 );
    //without host query doesn't make sense
    if ( mHost ) {
      TQPushButton *dnquery = new TQPushButton( i18n( "Query Server" ), this );
      connect( dnquery, TQ_SIGNAL( clicked() ), TQ_SLOT( mQueryDNClicked() ) );
      mainLayout->addMultiCellWidget( dnquery, row, row, 2, 3 );
    }
    row++;
  }

  if ( mFlags & W_FILTER ) {
    label = new TQLabel( i18n( "Filter:" ), this );
    mFilter = new KLineEdit( this, "kcfg_ldapfilter" );

    mainLayout->addWidget( label, row, 0 );
    mainLayout->addMultiCellWidget( mFilter, row, row, 1, 3 );
    row++;
  }

  if ( mFlags & W_SECBOX ) {
    TQHButtonGroup *btgroup = new TQHButtonGroup( i18n( "Security" ), this );
    mSecNO = new TQRadioButton( i18n( "No" ), btgroup, "kcfg_ldapnosec" );
    mSecTLS = new TQRadioButton( i18n( "TLS" ), btgroup, "kcfg_ldaptls" );
    mSecSSL = new TQRadioButton( i18n( "SSL" ), btgroup, "kcfg_ldapssl" );
    mainLayout->addMultiCellWidget( btgroup, row, row, 0, 3 );

    connect( mSecNO, TQ_SIGNAL( clicked() ), TQ_SLOT( setLDAPPort() ) );
    connect( mSecTLS, TQ_SIGNAL( clicked() ), TQ_SLOT( setLDAPPort() ) );
    connect( mSecSSL, TQ_SIGNAL( clicked() ), TQ_SLOT( setLDAPSPort( ) ) );

    mSecNO->setChecked( true );
    row++;
  }

  if ( mFlags & W_AUTHBOX ) {

    TQButtonGroup *authbox =
      new TQButtonGroup( 3, TQt::Horizontal, i18n( "Authentication" ), this );

    mAnonymous = new TQRadioButton( i18n( "Anonymous" ), authbox, "kcfg_ldapanon" );
    mSimple = new TQRadioButton( i18n( "Simple" ), authbox, "kcfg_ldapsimple" );
    mSASL = new TQRadioButton( i18n( "SASL" ), authbox, "kcfg_ldapsasl" );

    label = new TQLabel( i18n( "SASL mechanism:" ), authbox );
    mMech = new KComboBox( false, authbox, "kcfg_ldapsaslmech" );
    mMech->setEditable( true );
    mMech->insertItem( "DIGEST-MD5" );
    mMech->insertItem( "GSSAPI" );
    mMech->insertItem( "PLAIN" );

    //without host query doesn't make sense
    if ( mHost ) {
      mQueryMech = new TQPushButton( i18n( "Query Server" ), authbox );
      connect( mQueryMech, TQ_SIGNAL( clicked() ), TQ_SLOT( mQueryMechClicked() ) );
    }

    mainLayout->addMultiCellWidget( authbox, row, row+1, 0, 3 );

    connect( mAnonymous, TQ_SIGNAL( stateChanged(int) ), TQ_SLOT( setAnonymous(int) ) );
    connect( mSimple, TQ_SIGNAL( stateChanged(int) ), TQ_SLOT( setSimple(int) ) );
    connect( mSASL, TQ_SIGNAL( stateChanged(int) ), TQ_SLOT( setSASL(int) ) );

    mAnonymous->setChecked( true );
  }

}

void LdapConfigWidget::loadData( TDEIO::Job*, const TQByteArray& d )
{
  LDIF::ParseVal ret;

  if ( d.size() ) {
    mLdif.setLDIF( d );
  } else {
    mLdif.endLDIF();
  }
  do {
    ret = mLdif.nextItem();
    if ( ret == LDIF::Item && mLdif.attr().lower() == mAttr ) {
      mProg->progressBar()->advance( 1 );
      mQResult.push_back( TQString::fromUtf8( mLdif.val(), mLdif.val().size() ) );
    }
  } while ( ret != LDIF::MoreData );
}

void LdapConfigWidget::loadResult( TDEIO::Job* job)
{
  int error = job->error();
  if ( error && error != TDEIO::ERR_USER_CANCELED )
    mErrorMsg = job->errorString();
  else
    mErrorMsg = "";

  mCancelled = false;
  mProg->close();
}

void LdapConfigWidget::sendQuery()
{
  LDAPUrl _url;

  mQResult.clear();
  mCancelled = true;

  _url.setProtocol( ( mSecSSL && mSecSSL->isChecked() ) ? "ldaps" : "ldap" );
  if ( mHost ) _url.setHost( mHost->text() );
  if ( mPort ) _url.setPort( mPort->value() );
  _url.setDn( "" );
  _url.setAttributes( mAttr );
  _url.setScope( LDAPUrl::Base );
  if ( mVer ) _url.setExtension( "x-ver", TQString::number( mVer->value() ) );
  if ( mSecTLS && mSecTLS->isChecked() ) _url.setExtension( "x-tls", "" );

  kdDebug(5700) << "sendQuery url: " << _url.prettyURL() << endl;
  mLdif.startParsing();
  TDEIO::Job *job = TDEIO::get( _url, true, false );
  job->addMetaData("no-auth-prompt","true");
  connect( job, TQ_SIGNAL( data( TDEIO::Job*, const TQByteArray& ) ),
    this, TQ_SLOT( loadData( TDEIO::Job*, const TQByteArray& ) ) );
  connect( job, TQ_SIGNAL( result( TDEIO::Job* ) ),
    this, TQ_SLOT( loadResult( TDEIO::Job* ) ) );

  if ( mProg == NULL )
    mProg = new KProgressDialog( this, 0, i18n("LDAP Query"), _url.prettyURL(), true );
  else
    mProg->setLabel( _url.prettyURL() );
  mProg->progressBar()->setValue( 0 );
  mProg->progressBar()->setTotalSteps( 1 );
  mProg->exec();
  if ( mCancelled ) {
    kdDebug(5700) << "query cancelled!" << endl;
    job->kill( true );
  } else {
    if ( !mErrorMsg.isEmpty() ) KMessageBox::error( this, mErrorMsg );
  }
}

void LdapConfigWidget::mQueryMechClicked()
{
  mAttr = "supportedsaslmechanisms";
  sendQuery();
  if ( !mQResult.isEmpty() ) {
    mQResult.sort();
    mMech->clear();
    mMech->insertStringList( mQResult );
  }
}

void LdapConfigWidget::mQueryDNClicked()
{
  mAttr = "namingcontexts";
  sendQuery();
  if ( !mQResult.isEmpty() ) mDn->setText( mQResult.first() );
}

void LdapConfigWidget::setAnonymous( int state )
{
  if ( state == TQButton::Off ) return;
  if ( mUser ) mUser->setEnabled(false);
  if ( mPassword ) mPassword->setEnabled(false);
  if ( mBindDN ) mBindDN->setEnabled(false);
  if ( mRealm ) mRealm->setEnabled(false);
  if ( mMech ) mMech->setEnabled(false);
  if ( mQueryMech ) mQueryMech->setEnabled(false);
}

void LdapConfigWidget::setSimple( int state )
{
  if ( state == TQButton::Off ) return;
  if ( mUser ) mUser->setEnabled(true);
  if ( mPassword ) mPassword->setEnabled(true);
  if ( mBindDN ) mBindDN->setEnabled(false);
  if ( mRealm ) mRealm->setEnabled(false);
  if ( mMech ) mMech->setEnabled(false);
  if ( mQueryMech ) mQueryMech->setEnabled(false);
}

void LdapConfigWidget::setSASL( int state )
{
  if ( state == TQButton::Off ) return;
  if ( mUser ) mUser->setEnabled(true);
  if ( mPassword ) mPassword->setEnabled(true);
  if ( mBindDN ) mBindDN->setEnabled(true);
  if ( mRealm ) mRealm->setEnabled(true);
  if ( mMech ) mMech->setEnabled(true);
  if ( mQueryMech ) mQueryMech->setEnabled(true);
}

void LdapConfigWidget::setLDAPPort()
{
  mPort->setValue( 389 );
}

void LdapConfigWidget::setLDAPSPort()
{
  mPort->setValue( 636 );
}


LDAPUrl LdapConfigWidget::url() const
{
  LDAPUrl _url;
  if ( mSecSSL && mSecSSL->isChecked() )
    _url.setProtocol( "ldaps" );
  else
    _url.setProtocol( "ldap" );

  if ( mUser ) _url.setUser( mUser->text() );
  if ( mPassword ) _url.setPass( mPassword->text() );
  if ( mHost ) _url.setHost( mHost->text() );
  if ( mPort ) _url.setPort( mPort->value() );
  if ( mDn ) _url.setDn( mDn->text() );
  if ( mVer ) _url.setExtension( "x-ver", TQString::number( mVer->value() ) );
  if ( mSizeLimit && mSizeLimit->value() != 0 )
    _url.setExtension( "x-sizelimit", TQString::number( mSizeLimit->value() ) );
  if ( mTimeLimit && mTimeLimit->value() != 0 )
    _url.setExtension( "x-timelimit", TQString::number( mTimeLimit->value() ) );
  if ( mSecTLS && mSecTLS->isChecked() ) _url.setExtension( "x-tls","" );
  if ( mFilter && !mFilter->text().isEmpty() )
    _url.setFilter( mFilter->text() );
  if ( mSASL && mSASL->isChecked() ) {
    _url.setExtension( "x-sasl", "" );
    _url.setExtension( "x-mech", mMech->currentText() );
    if ( mBindDN && !mBindDN->text().isEmpty() )
      _url.setExtension( "bindname", mBindDN->text() );
    if ( mRealm && !mRealm->text().isEmpty() )
      _url.setExtension( "x-realm", mRealm->text() );
  }
  return ( _url );
}

void LdapConfigWidget::setUser( const TQString &user )
{
  if ( mUser ) mUser->setText( user );
}

TQString LdapConfigWidget::user() const
{
  return ( mUser ? mUser->text() : TQString::null );
}

void LdapConfigWidget::setPassword( const TQString &password )
{
  if ( mPassword ) mPassword->setText( password );
}

TQString LdapConfigWidget::password() const
{
  return ( mPassword ? mPassword->text() : TQString::null );
}

void LdapConfigWidget::setBindDN( const TQString &binddn )
{
  if ( mBindDN ) mBindDN->setText( binddn );
}

TQString LdapConfigWidget::bindDN() const
{
  return ( mBindDN ? mBindDN->text() : TQString::null );
}

void LdapConfigWidget::setRealm( const TQString &realm )
{
  if ( mRealm ) mRealm->setText( realm );
}

TQString LdapConfigWidget::realm() const
{
  return ( mRealm ? mRealm->text() : TQString::null );
}

void LdapConfigWidget::setHost( const TQString &host )
{
  if ( mHost ) mHost->setText( host );
}

TQString LdapConfigWidget::host() const
{
  return ( mHost ? mHost->text() : TQString::null );
}

void LdapConfigWidget::setPort( int port )
{
  if ( mPort ) mPort->setValue( port );
}

int LdapConfigWidget::port() const
{
  return ( mPort ? mPort->value() : 389 );
}

void LdapConfigWidget::setVer( int ver )
{
  if ( mVer ) mVer->setValue( ver );
}

int LdapConfigWidget::ver() const
{
  return ( mVer ? mVer->value() : 3 );
}

void LdapConfigWidget::setDn( const TQString &dn )
{
  if ( mDn ) mDn->setText( dn );
}

TQString LdapConfigWidget::dn() const
{
  return ( mDn ? mDn->text() : TQString::null );
}

void LdapConfigWidget::setFilter( const TQString &filter )
{
  if ( mFilter ) mFilter->setText( filter );
}

TQString LdapConfigWidget::filter() const
{
  return ( mFilter ? mFilter->text() : TQString::null );
}

void LdapConfigWidget::setMech( const TQString &mech )
{
  if ( mMech == 0 ) return;
  if ( !mech.isEmpty() ) {
    int i = 0;
    while ( i < mMech->count() ) {
      if ( mMech->text( i ) == mech ) break;
      i++;
    }
    if ( i == mMech->count() ) mMech->insertItem( mech );
    mMech->setCurrentItem( i );
  }
}

TQString LdapConfigWidget::mech() const
{
  return ( mMech ? mMech->currentText() : TQString::null );
}

void LdapConfigWidget::setSecNO( bool b )
{
  if ( mSecNO ) mSecNO->setChecked( b );
}

bool LdapConfigWidget::isSecNO() const
{
  return ( mSecNO ? mSecNO->isChecked() : true );
}

void LdapConfigWidget::setSecTLS( bool b )
{
  if ( mSecTLS ) mSecTLS->setChecked( b );
}

bool LdapConfigWidget::isSecTLS() const
{
  return ( mSecTLS ? mSecTLS->isChecked() : false );
}

void LdapConfigWidget::setSecSSL( bool b )
{
  if ( mSecSSL ) mSecSSL->setChecked( b );
}

bool LdapConfigWidget::isSecSSL() const
{
  return ( mSecSSL ? mSecSSL->isChecked() : false );
}

void LdapConfigWidget::setAuthAnon( bool b )
{
  if ( mAnonymous ) mAnonymous->setChecked( b );
}

bool LdapConfigWidget::isAuthAnon() const
{
  return ( mAnonymous ? mAnonymous->isChecked() : true );
}

void LdapConfigWidget::setAuthSimple( bool b )
{
  if ( mSimple ) mSimple->setChecked( b );
}

bool LdapConfigWidget::isAuthSimple() const
{
  return ( mSimple ? mSimple->isChecked() : false );
}

void LdapConfigWidget::setAuthSASL( bool b )
{
  if ( mSASL ) mSASL->setChecked( b );
}

bool LdapConfigWidget::isAuthSASL() const
{
  return ( mSASL ? mSASL->isChecked() : false );
}

void LdapConfigWidget::setSizeLimit( int sizelimit )
{
  if ( mSizeLimit ) mSizeLimit->setValue( sizelimit );
}

int LdapConfigWidget::sizeLimit() const
{
  return ( mSizeLimit ? mSizeLimit->value() : 0 );
}

void LdapConfigWidget::setTimeLimit( int timelimit )
{
  if ( mTimeLimit ) mTimeLimit->setValue( timelimit );
}

int LdapConfigWidget::timeLimit() const
{
  return ( mTimeLimit ? mTimeLimit->value() : 0 );
}

int LdapConfigWidget::flags() const
{
  return mFlags;
}

void LdapConfigWidget::setFlags( int flags )
{
  mFlags = flags;

  // First delete all the child widgets.
  // FIXME: I hope it's correct
  const TQObjectList ch = childrenListObject();
  TQObjectList ch2 = ch;
  TQObject *obj;
  TQWidget *widget;

  obj = ch2.first();
  while ( obj != 0 ) {
    widget = dynamic_cast<TQWidget*> (obj);
    if ( widget && widget->parent() == this ) {
      mainLayout->remove( widget );
      delete ( widget );
    }
    obj = ch2.next();
  }
  // Re-create child widgets according to the new flags
  initWidget();
}
