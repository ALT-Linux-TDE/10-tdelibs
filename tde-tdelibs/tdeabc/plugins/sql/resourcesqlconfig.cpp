/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#include <tqlabel.h>
#include <tqlayout.h>
#include <tqspinbox.h>
#include <tqvbox.h>

#include <klineedit.h>
#include <tdelocale.h>

#include "resource.h"
#include "resourcesqlconfig.h"

using namespace TDEABC;

ResourceSqlConfig::ResourceSqlConfig( TQWidget* parent,  const char* name )
    : ResourceConfigWidget( parent, name )
{
  resize( 290, 170 ); 

  TQGridLayout *mainLayout = new TQGridLayout( this, 4, 2 );

  TQLabel *label = new TQLabel( i18n( "Username:" ), this );
  mUser = new KLineEdit( this );

  mainLayout->addWidget( label, 0, 0 );
  mainLayout->addWidget( mUser, 0, 1 );

  label = new TQLabel( i18n( "Password:" ), this );
  mPassword = new KLineEdit( this );
  mPassword->setEchoMode( KLineEdit::Password );

  mainLayout->addWidget( label, 1, 0 );
  mainLayout->addWidget( mPassword, 1, 1 );

  label = new TQLabel( i18n( "Host:" ), this );
  mHost = new KLineEdit( this );

  mainLayout->addWidget( label, 2, 0 );
  mainLayout->addWidget( mHost, 2, 1 );

  label = new TQLabel( i18n( "Port:" ), this );
  TQVBox *box = new TQVBox(this);
  mPort = new TQSpinBox(0, 65535, 1, box );
  mPort->setSizePolicy(TQSizePolicy(TQSizePolicy::Maximum, TQSizePolicy::Preferred));
  mPort->setValue(389);
  new TQWidget(box, "dummy");

  mainLayout->addWidget( label, 3, 0 );
  mainLayout->addWidget( box, 3, 1 );

  label = new TQLabel( i18n( "Database:" ), this );
  mDbName = new KLineEdit( this );

  mainLayout->addWidget( label, 4, 0 );
  mainLayout->addWidget( mDbName, 4, 1 );
}

void ResourceSqlConfig::loadSettings( TDEConfig *config )
{
  mUser->setText( config->readEntry( "SqlUser" ) );
  mPassword->setText( TDEABC::Resource::cryptStr( config->readEntry( "SqlPassword" ) ) );
  mDbName->setText( config->readEntry( "SqlName" ) );
  mHost->setText( config->readEntry( "SqlHost" ) );
  mPort->setValue( config->readNumEntry( "SqlPort" ) );
}

void ResourceSqlConfig::saveSettings( TDEConfig *config )
{
  config->writeEntry( "SqlUser", mUser->text() );
  config->writeEntry( "SqlPassword", TDEABC::Resource::cryptStr( mPassword->text() ) );
  config->writeEntry( "SqlName", mDbName->text() );
  config->writeEntry( "SqlHost", mHost->text() );
  config->writeEntry( "SqlPort", mPort->value() );
}

#include "resourcesqlconfig.moc"
