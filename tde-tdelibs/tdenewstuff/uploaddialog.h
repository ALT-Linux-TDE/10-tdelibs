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
#ifndef KNEWSTUFF_UPLOADDIALOG_H
#define KNEWSTUFF_UPLOADDIALOG_H

#include <kdialogbase.h>

class TQLineEdit;
class TQSpinBox;
class KURLRequester;
class TQTextEdit;
class TQComboBox;

namespace KNS {

class Engine;
class Entry;

/**
 * @short TDENewStuff file upload dialog.
 *
 * Using this dialog, data can easily be uploaded to the Hotstuff servers.
 * It should however not be used on its own, instead a TDENewStuff (or
 * TDENewStuffGeneric) object invokes it.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class UploadDialog : public KDialogBase
{
    TQ_OBJECT
  public:
    /**
      Constructor.

      @param engine a TDENewStuff engine object to be used for uploads
      @param parent the parent window
    */
    UploadDialog( Engine *engine, TQWidget *parent );

    /**
      Destructor.
    */
    ~UploadDialog();

    /**
      Sets the preview filename.
      This is only meaningful if the application supports previews.

      @param previewFile the preview image file
    */
    void setPreviewFile( const TQString &previewFile );

    /**
      Sets the payload filename.
      This is optional, but necessary if the application wants to support
      reusing previously filled out form data based on the filename.

      @param payloadFile the payload data file
    */
    void setPayloadFile( const TQString &payloadFile );

  protected slots:
    void slotOk();

  private:
    Engine *mEngine;

    TQLineEdit *mNameEdit;
    TQLineEdit *mAuthorEdit;
    TQLineEdit *mEmailEdit;
    TQLineEdit *mVersionEdit;
    TQSpinBox *mReleaseSpin;
    KURLRequester *mPreviewUrl;
    TQTextEdit *mSummaryEdit;
    TQComboBox *mLanguageCombo;
    TQComboBox *mLicenceCombo;

    TQPtrList<Entry> mEntryList;
    KURL mPayloadUrl;
};

}

#endif
