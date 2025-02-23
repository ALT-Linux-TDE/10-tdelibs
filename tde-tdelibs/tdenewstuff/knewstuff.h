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
#ifndef KNEWSTUFF_H
#define KNEWSTUFF_H

#include <tqstring.h>

#include <tdemacros.h>

class TQObject;
class TQWidget;
class TDEAction;
class TDEActionCollection;

namespace KNS {
class Engine;
class Entry;

TDE_EXPORT TDEAction* standardAction(const TQString& what,
                        const TQObject *recvr,
                        const char *slot,
                        TDEActionCollection* parent,
                        const char *name = 0);
}

/**
 * @short This class provides the functionality to download and upload "new stuff".
 *
 * Applications have to subclass TDENewStuff, implement the pure virtual functions
 * and link to against libknewstuff.
 *
 * By calling download() the download process is started which means that a list
 * of "providers" is fetched from a "master server", information about new stuff
 * is collected from the providers and presented to the user. Selected entries
 * get downloaded and installed to the application. The required functions to
 * install new stuff are provided by implementing install(). The location where
 * the downloaded files are stored can be customized by reimplementing
 * downloadDestination().
 *
 * By calling upload() the upload process is started which means the user has to
 * select a provider from the list fetched from the master server and to put in
 * information about the entry to be uploaded. Then the file to be uploaded is
 * fetched from the application by calling createUploadFile() and transfered to
 * the upload destination specified in the provider list.
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 *
 * @since 3.3
 */
class TDE_EXPORT TDENewStuff
{
  public:
    /**
      Constructor.

      @param type type of data to be handled, should be something like
                  korganizer/calendar, kword/template, kdesktop/wallpaper
      @param parentWidget parent widget of dialogs opened by the TDENewStuff
                          engine
    */
    TDENewStuff( const TQString &type, TQWidget *parentWidget = 0 );
    
    /**
      Constructor.

      @param type type of data to be handled, should be something like
                  korganizer/calendar, kword/template, kdesktop/wallpaper
      @param providerList the URL of the provider list
      @param parentWidget parent widget of dialogs opened by the TDENewStuff
                          engine
    */
    TDENewStuff( const TQString &type, const TQString &providerList, TQWidget *parentWidget = 0 );
    virtual ~TDENewStuff();

    /**
      Return type of data.
    */
    TQString type() const;

    /**
      Return parent widget.
    */
    TQWidget *parentWidget() const;

    /**
      Start download process.
    */
    void download();

    /**
      Start upload process.
    */
    void upload();

    /**
      Upload with pre-defined files.
    */
    void upload( const TQString &fileName, const TQString previewName );

    /**
      Install file to application. The given fileName points to the file
      downloaded by the TDENewStuff engine. This is a temporary file by default.
      The application can do whatever is needed to handle the information
      contained in the file.

      The function returns true, when the installation
      was successful and false if were errors.

      @param fileName name of downloaded file
    */
    virtual bool install( const TQString &fileName ) = 0;
    /**
      Create a file to be uploaded to a "new stuff provider" and return the
      filename. The format of the file is application specific. The only
      constraint is that the corresponding install() implementation is able to
      use the file.

      @param fileName name of the file to be written
      @return @c true on success, @c false on error.
    */
    virtual bool createUploadFile( const TQString &fileName ) = 0;

    /**
      Return a filename which should be used as destination for downloading the
      specified new stuff entry. Reimplement this function, if you don't want
      the new stuff to be downloaded to a temporary file.
    */
    virtual TQString downloadDestination( KNS::Entry *entry );
    
    
  protected:
    /**
      Get the pointer to the engine. Needed by subclasses to access the KNS::Engine object.
     */  
    KNS::Engine *engine() { return mEngine; }  

    
  private:
    KNS::Engine *mEngine;
};

#endif
