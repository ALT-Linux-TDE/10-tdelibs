    /*

    Copyright (C) 2000 Stefan Westerfeld
                       stefan@space.twc.de

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

#ifndef __KAUDIOPLAYER_H__
#define __KAUDIOPLAYER_H__

#include <tqobject.h>
#include "tdelibs_export.h"

class KAudioPlayerPrivate;
/**
 * This class provides one-shot-and-forget audio playing. You will never
 * know if what you wanted to play really got played.
 *
 * It doesn't require linking any special libraries, as it operates over
 * DCOP. In the current implementation, it only indirectly communicates
 * with the aRts soundserver, using knotify as DCOP -> MCOP bridge.
 *
 * Due to that fact, if you need "fast" response times, more control or
 * feedback, use the MCOP interfaces rather than this.
 *
 * An example of using this class is:
 *
 * \code
 *   KAudioPlayer::play("/var/share/foo.wav");
 * \endcode
 *
 * If you want to use signals & slots, you can do something like:
 *
 * \code
 *   KAudioPlayer player("/var/share/foo.wav");
 *   connect(&button, TQ_SIGNAL(clicked()), &player, TQ_SLOT(play()));
 * \endcode
 *
 */
//REVISED: hausmann
class TDECORE_EXPORT KAudioPlayer : public TQObject {
TQ_OBJECT
public:
	/**
	 * Constructor.
	 *
	 * @param filename Absolute path to the filename of the sound file to play
	 * @param parent A parent TQObject for this KAudioPlayer
	 * @param name An internal name for this KAudioPlayer
	 */
	KAudioPlayer( const TQString& filename,
			TQObject* parent = 0, const char* name = 0 );

	/**
	 * Destructor.
	 */
	~KAudioPlayer();

	/**
	 * Static play function.
	 *
	 * @param filename Absolute path to the filename of the sound file to play.
	 *                if not absolute, goes off TDEDIR/share/sounds/ (preferred)
	 */
	static void play(const TQString &filename);

public slots:
	/**
	 * Play function as slot.
	 *
	 * Plays the soundfile given to the constructor.
	 */
	void play();
private:
	KAudioPlayerPrivate *d;
};

#endif // __KAUDIOPLAYER_H__
