/*
    This file is part of KDE.

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
#ifndef KNEWSTUFFGENERIC_H
#define KNEWSTUFFGENERIC_H

#include "knewstuff.h"

class TDEConfig;

/**
 * @short Basic TDENewStuff class with predefined actions.
 *
 * This class is used for data uploads and installation.
 * \code
 * TQString payload, preview;
 * TDENewStuffGeneric *ns = new TDENewStuffGeneric("kamikaze/level", this);
 * ns->upload(payload, preview);
 * \endcode
 *
 * @author Cornelius Schumacher (schumacher@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class TDE_EXPORT TDENewStuffGeneric : public TDENewStuff
{
  public:
    /**
      Constructor.

      @param type a Hotstuff data type such as "korganizer/calendar"
      @param parent the parent window.
    */
    TDENewStuffGeneric( const TQString &type, TQWidget *parent = 0 );
    ~TDENewStuffGeneric();

    /**
      Installs a downloaded file according to the application's configuration.

      @param fileName filename of the donwloaded file
      @return @c true in case of installation success, @c false otherwise
    */
    bool install( const TQString &fileName );

    /**
      Creates a file suitable for upload.
      Note that this method always fails, since using TDENewStuffGeneric
      means that the provided file must already be in a usable format.

      @param fileName the name of the file to upload after its creation
      @return @c true in case of creation success, @c false otherwise
    */
    bool createUploadFile( const TQString &fileName );

    /**
      Queries the preferred destination file for a download.

      @param entry a Hotstuff data entry
      @return destination filename, or 0 to return directory only
    */
    TQString downloadDestination( KNS::Entry *entry );

  private:
    TQString destinationPath( KNS::Entry *entry );

    TDEConfig *mConfig;
};

#endif
