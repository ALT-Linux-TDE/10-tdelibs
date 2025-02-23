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

#ifndef KABC_SOUND_H
#define KABC_SOUND_H

#include <tqcstring.h>
#include <tqstring.h>

#include <tdelibs_export.h>

namespace TDEABC {

/** @short Class that holds a Sound clip for a contact.
 *
 *  The sound can be played doing something like this:
 *
 *  \code
 *    KTempFile tmp;
 *    if(sound.isIntern()) {
 *      tmp.file()->writeBlock( sound.data() );
 *      tmp.close();
 *      KAudioPlayer::play( tmp.name() );
 *    } else if(!sound.url().isEmpty()) {
 *      TQString tmpFile;
 *      if(!TDEIO::NetAccess::download(KURL(themeURL.url()), tmpFile, NULL))
 *      {
 *        KMessageBox::error(0L,
 *                           TDEIO::NetAccess::lastErrorString(),
 *                           i18n("Failed to download sound file"),
 *                           KMessageBox::Notify
 *                          );
 *        return;
 *      }
 *      KAudioPlayer::play( tmpFile ); 
 *    }
 *  \endcode
 *       
 *  Unfortunetly KAudioPlayer::play is ASync, so to delete the temporary file, the best you can really do is set a timer.
 *
 */
class KABC_EXPORT Sound
{
  friend KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Sound & );
  friend KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Sound & );

public:

  /**
   * Consturctor. Creates an empty object.
   */
  Sound();

  /**
   * Consturctor.
   *
   * @param url  A URL that describes the position of the sound file.
   */
  Sound( const TQString &url );

  /**
   * Consturctor.
   *
   * @param data  The raw data of the sound.
   */
  Sound( const TQByteArray &data );

  /**
   * Destructor.
   */
  ~Sound();


  bool operator==( const Sound & ) const;
  bool operator!=( const Sound & ) const;

  /**
   * Sets a URL for the location of the sound file. When using this
   * function, isIntern() will return 'false' until you use
   * setData().
   *
   * @param url  The location URL of the sound file.
   */
  void setUrl( const TQString &url );

  /**
   * Test if this sound file has been set.
   * Just does:  !isIntern() && url.isEmpty()
   * @since 3.4
   */
  bool isEmpty() const;
  
  /**
   * Sets the raw data of the sound. When using this function,
   * isIntern() will return 'true' until you use setUrl().
   *
   * @param data  The raw data of the sound.
   */
  void setData( const TQByteArray &data );

  /**
   * Returns whether the sound is described by a URL (extern) or
   * by the raw data (intern).
   * When this method returns 'true' you can use data() to
   * get the raw data. Otherwise you can request the URL of this
   * sound by url() and load the raw data from that location.
   */
  bool isIntern() const;

  /**
   * Returns the location URL of this sound.
   */
  TQString url() const;

  /**
   * Returns the raw data of this sound.
   */
  TQByteArray data() const;

  /**
   * Returns string representation of the sound.
   */
  TQString asString() const;

private:
  TQString mUrl;
  TQByteArray mData;

  int mIntern;
};

KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Sound & );
KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Sound & );

}
#endif
