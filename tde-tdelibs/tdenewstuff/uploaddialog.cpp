/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqspinbox.h>
#include <tqstring.h>
#include <ktextedit.h>

#include <tdelistview.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <kurlrequester.h>
#include <tdemessagebox.h>
#include <tdeconfig.h>
#include <tdeapplication.h>
#include <kuser.h>

#include "engine.h"
#include "entry.h"

#include "uploaddialog.h"
#include "uploaddialog.moc"

using namespace KNS;

UploadDialog::UploadDialog( Engine *engine, TQWidget *parent ) :
  KDialogBase( Plain, i18n("Share Hot New Stuff"), Ok | Cancel, Cancel,
               parent, 0, false, true ),
  mEngine( engine )
{
  mEntryList.setAutoDelete( true );

  TQFrame *topPage = plainPage();

  TQGridLayout *topLayout = new TQGridLayout( topPage );
  topLayout->setSpacing( spacingHint() );

  TQLabel *nameLabel = new TQLabel( i18n("Name:"), topPage );
  topLayout->addWidget( nameLabel, 0, 0 );  
  mNameEdit = new TQLineEdit( topPage );
  topLayout->addWidget( mNameEdit, 0, 1 );

  TQLabel *authorLabel = new TQLabel( i18n("Author:"), topPage );
  topLayout->addWidget( authorLabel, 1, 0 );
  mAuthorEdit = new TQLineEdit( topPage );
  topLayout->addWidget( mAuthorEdit, 1, 1 );

  TQLabel *emailLabel = new TQLabel( i18n("Email:"), topPage );
  topLayout->addWidget( emailLabel, 2, 0 );
  mEmailEdit = new TQLineEdit( topPage );
  topLayout->addWidget( mEmailEdit, 2, 1 );

  TQLabel *versionLabel = new TQLabel( i18n("Version:"), topPage );
  topLayout->addWidget( versionLabel, 3, 0 );  
  mVersionEdit = new TQLineEdit( topPage );
  topLayout->addWidget( mVersionEdit, 3, 1 );

  TQLabel *releaseLabel = new TQLabel( i18n("Release:"), topPage );
  topLayout->addWidget( releaseLabel, 4, 0 );  
  mReleaseSpin = new TQSpinBox( topPage );
  mReleaseSpin->setMinValue( 1 );
  topLayout->addWidget( mReleaseSpin, 4, 1 );

  TQLabel *licenceLabel = new TQLabel( i18n("License:"), topPage );
  topLayout->addWidget( licenceLabel, 5, 0 );
  mLicenceCombo = new TQComboBox( topPage );
  mLicenceCombo->setEditable( true );
  mLicenceCombo->insertItem( i18n("GPL") );
  mLicenceCombo->insertItem( i18n("LGPL") );
  mLicenceCombo->insertItem( i18n("BSD") );
  topLayout->addWidget( mLicenceCombo, 5, 1 );

  TQLabel *languageLabel = new TQLabel( i18n("Language:"), topPage );
  topLayout->addWidget( languageLabel, 6, 0 );
  mLanguageCombo = new TQComboBox( topPage );
  topLayout->addWidget( mLanguageCombo, 6, 1 );
  mLanguageCombo->insertStringList( TDEGlobal::locale()->languageList() );

  TQLabel *previewLabel = new TQLabel( i18n("Preview URL:"), topPage );
  topLayout->addWidget( previewLabel, 7, 0 );
  mPreviewUrl = new KURLRequester( topPage );
  topLayout->addWidget( mPreviewUrl, 7, 1 );

  TQLabel *summaryLabel = new TQLabel( i18n("Summary:"), topPage );
  topLayout->addMultiCellWidget( summaryLabel, 8, 8, 0, 1 );
  mSummaryEdit = new KTextEdit( topPage );
  topLayout->addMultiCellWidget( mSummaryEdit, 9, 9, 0, 1 );

  KUser user;
  mAuthorEdit->setText(user.fullName());
}

UploadDialog::~UploadDialog()
{
  mEntryList.clear();
}

void UploadDialog::slotOk()
{
  if ( mNameEdit->text().isEmpty() ) {
    KMessageBox::error( this, i18n("Please put in a name.") );
    return;
  }

  Entry *entry = new Entry;

  mEntryList.append( entry );

  entry->setName( mNameEdit->text() );
  entry->setAuthor( mAuthorEdit->text() );
  entry->setAuthorEmail( mEmailEdit->text() );
  entry->setVersion( mVersionEdit->text() );
  entry->setRelease( mReleaseSpin->value() );
  entry->setLicence( mLicenceCombo->currentText() );
  entry->setPreview( KURL( mPreviewUrl->url().section("/", -1) ), mLanguageCombo->currentText() );
  entry->setSummary( mSummaryEdit->text(), mLanguageCombo->currentText() );

  if ( mPayloadUrl.isValid() ) {
    TDEConfig *conf = kapp->config();
    conf->setGroup( TQString("TDENewStuffUpload:%1").arg(mPayloadUrl.fileName()) );
    conf->writeEntry("name", mNameEdit->text());
    conf->writeEntry("author", mAuthorEdit->text());
    conf->writeEntry("email", mEmailEdit->text());
    conf->writeEntry("version", mVersionEdit->text());
    conf->writeEntry("release", mReleaseSpin->value());
    conf->writeEntry("licence", mLicenceCombo->currentText());
    conf->writeEntry("preview", mPreviewUrl->url());
    conf->writeEntry("summary", mSummaryEdit->text());
    conf->writeEntry("language", mLanguageCombo->currentText());
    conf->sync();
  }

  mEngine->upload( entry );

  accept();
}

void UploadDialog::setPreviewFile( const TQString &previewFile )
{
  mPreviewUrl->setURL( previewFile );
}

void UploadDialog::setPayloadFile( const TQString &payloadFile )
{
  mPayloadUrl = payloadFile;

  TDEConfig *conf = kapp->config();
  conf->setGroup( TQString("TDENewStuffUpload:%1").arg(mPayloadUrl.fileName()) );
  TQString name = conf->readEntry("name");
  TQString author = conf->readEntry("author");
  TQString email = conf->readEntry("email");
  TQString version = conf->readEntry("version");
  TQString release = conf->readEntry("release");
  TQString preview = conf->readEntry("preview");
  TQString summary = conf->readEntry("summary");
  TQString lang = conf->readEntry("language");
  TQString licence = conf->readEntry("licence");

  mNameEdit->clear();
  mAuthorEdit->clear();
  mEmailEdit->clear();
  mVersionEdit->clear();
  mReleaseSpin->setValue(1);
  mPreviewUrl->clear();
  mSummaryEdit->clear();
  mLanguageCombo->setCurrentItem(0);
  mLicenceCombo->setCurrentItem(0);

  if(!name.isNull())
  {
    int prefill = KMessageBox::questionYesNo(this, i18n("Old upload information found, fill out fields?"),TQString::null,i18n("Fill Out"),i18n("Do Not Fill Out"));
    if(prefill == KMessageBox::Yes)
    {
      mNameEdit->setText(name);
      mAuthorEdit->setText(author);
      mEmailEdit->setText(email);
      mVersionEdit->setText(version);
      mReleaseSpin->setValue(release.toInt());
      mPreviewUrl->setURL(preview);
      mSummaryEdit->setText(summary);
      if(!lang.isEmpty()) mLanguageCombo->setCurrentText(lang);
      if(!licence.isEmpty()) mLicenceCombo->setCurrentText(licence);
    }
  }
}

