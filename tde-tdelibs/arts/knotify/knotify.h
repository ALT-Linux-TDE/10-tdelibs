/*
   Copyright (c) 1997 Christian Esken (esken@kde.org)

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#ifndef KNOTIFY_H
#define KNOTIFY_H

#include <tqobject.h>
#include <knotifyclient.h>
#include <dcopobject.h>

class KNotifyPrivate;
class TDEProcess;
class TDEConfig;

class KNotify : public TQObject, public DCOPObject
{
	TQ_OBJECT
	K_DCOP
	
	public:
		KNotify( bool useArts );
		~KNotify();
	
		enum PlayingFinishedStatus {
			PlayedOK = 0,        // success, all following mean failure
			NoSoundFile,
			FileAlreadyPlaying,
			NoSoundSupport,
			PlayerBusy,
			Aborted,
			Unknown = 5000
		};
	
	protected:
	k_dcop:
		// deprecated
		void notify(const TQString &event, const TQString &fromApp, const TQString &text, TQString sound, TQString file, int present, int level);
	
		// deprecated
		void notify(const TQString &event, const TQString &fromApp, const TQString &text, TQString sound, TQString file, int present, int level, int winId);
	
		void notify(const TQString &event, const TQString &fromApp, const TQString &text, TQString sound, TQString file, int present, int level, int winId, int eventId);
	
	
		void reconfigure();
		void setVolume( int volume );
		void sessionReady(); // from ksmserver
	
	private:
		bool notifyBySound(const TQString &sound, const TQString &appname, int eventId);
		bool notifyByMessagebox(const TQString &text, int level, WId winId);
		bool notifyByLogfile(const TQString &text, const TQString &file);
		bool notifyByStderr(const TQString &text);
		bool notifyByPassivePopup(const TQString &text, const TQString &appName, TDEConfig* eventsFile, WId winId );
		bool notifyByExecute(const TQString &command, const TQString& event, const TQString& fromApp, const TQString& text, int winId, int eventId );
		bool notifyByTaskbar( WId winId );
		
		bool isPlaying( const TQString& soundFile ) const;
	
		void soundFinished( int eventId, PlayingFinishedStatus reason );
		void abortFirstPlayObject();
		
		WId checkWinId( const TQString& appName, WId senderWinId );
	
		/**
		* checks if eventname is a global event (exists in config/eventsrc)
		**/
		bool isGlobal(const TQString &eventname);
	
	private slots:
		void playTimeout();
		void slotPlayerProcessExited( TDEProcess *proc );
		void restartedArtsd();
	
	private:
		KNotifyPrivate* d;
		void loadConfig();
};


#endif

