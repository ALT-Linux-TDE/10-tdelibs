/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>
    Copyright (c) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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
#ifndef KNEWSTUFF_ENGINE_H
#define KNEWSTUFF_ENGINE_H

#include <tqmap.h>
#include <tqobject.h>
#include <tqstring.h>

#include "entry.h"
#include "provider.h"

namespace TDEIO { class Job; }

class TDENewStuff;

namespace KNS {

class DownloadDialog;
class UploadDialog;
class ProviderDialog;

/**
 * @short Central class combining all possible TDENewStuff operations.
 *
 * In most cases, Engine objects are built and used internally.
 * Using this class explicitely does however give fine-grained control about the
 * upload and download operations.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class TDE_EXPORT Engine : public TQObject
{
    TQ_OBJECT
    struct Private;
  public:
    /**
      Constructor.

      @param newStuff a TDENewStuff object
      @param type the Hotstuff data type such as "korganizer/calendar"
      @param parentWidget the parent window
    */
    Engine( TDENewStuff *newStuff, const TQString &type, TQWidget *parentWidget = 0 );
    /**
      Constructor.

      @param newStuff a TDENewStuff object
      @param type the Hotstuff data type such as "korganizer/calendar"
      @param providerList the URL of the provider list
      @param parentWidget the parent window
    */
    Engine( TDENewStuff *newStuff, const TQString &type, const TQString &providerList, TQWidget *parentWidget = 0 );

    /**
      Destructor.
    */
    virtual ~Engine();

    /**
      Returns the previously set data type.

      @return the Hotstuff data type
    */
    TQString type() const { return mType; }

    /**
      Returns the previously set parent widget.

      @return parent widget
    */
    TQWidget *parentWidget() const { return mParentWidget; }

    /**
      Initiates the download process, retrieving provider lists and invoking
      the download dialog.
    */
    void download();

    /**
      Initiates the upload process, invoking the provider selection dialog
      and the file upload dialog.

      @param fileName name of the payload data file
      @param previewName name of the preview image file
    */
    void upload( const TQString &fileName = TQString::null, const TQString &previewName = TQString::null );

    /**
      Downloads the specified data file.

      @param entry the Hotstuff data object to be downloaded
    */
    void download( Entry *entry );

    /**
      Asynchronous lookup of provider information such as upload and
      download locations, icon etc.

      @param provider the Hotstuff provider to request information from
    */
    void requestMetaInformation( Provider *provider );

    /**
      Uploads the specified data file to the provider-dependent location.

      @param entry the Hotstuff data object to be uploaded
    */
    void upload( Entry *entry );
    
    /**
    Ignores the return value of the install method. Used internally to
    avoid showing of the success/failure dialog when installation is done
    in another place, like in @ref TDENewStuffSecure
     */
    void ignoreInstallResult(bool ignore);   

  signals:
    /** Emitted when the upload has finished.
      @param result indicates the success/failure of the upload
    */
    void uploadFinished( bool result );
  protected slots:
    void getMetaInformation( Provider::List *providers );
    void selectUploadProvider( Provider::List *providers );

    void slotNewStuffJobData( TDEIO::Job *job, const TQByteArray &data );
    void slotNewStuffJobResult( TDEIO::Job *job );

    void slotDownloadJobResult( TDEIO::Job *job );

    void slotUploadPayloadJobResult( TDEIO::Job *job );
    void slotUploadPreviewJobResult (TDEIO::Job *job );
    void slotUploadMetaJobResult( TDEIO::Job *job );

  protected:
    bool createMetaFile( Entry * );
    
  private:
    TQWidget *mParentWidget;

    ProviderLoader *mProviderLoader;

    TQMap<TDEIO::Job *,TQString> mNewStuffJobData;
    TQMap<TDEIO::Job *,Provider *> mProviderJobs;

    TQPtrList<Entry> mNewStuffList;

    DownloadDialog *mDownloadDialog;
    UploadDialog *mUploadDialog;
    ProviderDialog *mProviderDialog;

    TQString mDownloadDestination;

    Provider *mUploadProvider;

    TQString mUploadMetaFile;
    TQString mUploadFile;
    TQString mPreviewFile;
    TQString mProviderList;

    Private* d;

    TQString mType;
};

}

#endif
