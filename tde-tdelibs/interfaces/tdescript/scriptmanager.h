/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser  (geiseri@kde.org)

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
#ifndef __scriptmanager_h__
#define __scriptmanager_h__

#include <tqvariant.h>
#include <scriptclientinterface.h>
#include <scriptinterface.h>
#include <tqdict.h>
#include <tqobject.h>

#include <tdelibs_export.h>

class ScriptInfo;
//namespace KScriptInterface
//{

	/**
	*	This class is the base for all script engines.
	*	@author Ian Reinhart Geiser <geiseri@kde.org>
	*
	**/
	class TDE_EXPORT KScriptManager : public TQObject, public KScriptClientInterface
	{
	TQ_OBJECT
	friend class KScriptInterface;
	public:
		/**
		*	Create a new instance of the script engine.
		*/
		KScriptManager(TQObject *parent, const char *name);
		/**
		*	Destroy the current script engine.
		*/
		virtual ~KScriptManager();
		/**
		*	Add a new script instance to the script engine.
		*	This should be the full name and path to the desktop
		*	file.
		*/
		bool addScript( const TQString &scriptDesktopFile);
		/**
		*	Remove a script instance from the script engine.
		*	@returns the success of the operation.
		*/
		bool removeScript( const TQString &scriptName );
		/**
		*	Access the names of script instances from the script engine.
		*	@returns a TQStringList of the current scripts.
		*/
		TQStringList scripts();
		/**
		*	Clear all script intstances in memory
		*/
		void clear();
		/**
		*	This function will allow the main application of any errors
		*	that have occurred during processing of the script.
		*/
		void error( const TQString &msg ) {emit scriptError(msg);}
		/**
		*	This function will allow the main application of any warnings
		*	that have occurred during the processing of the script.
		*/
		void warning( const TQString &msg ) {emit scriptWarning(msg);}
		/**
		*	This function will allow the main application of any normal
		*	output that has occurred during the processing of the script.
		*/
		void output( const TQString &msg ) {emit scriptOutput(msg);}
		/**
		*	This function will allow feedback to any progress bars in the main
		*	application as to how far along the script is.  This is very useful when
		*	a script is processing files or doing some long operation that is of a
		*	known duration.
		*/
		void progress( int percent ) {emit scriptProgress(percent);}
		/**
		*	This function will allow feedback on completion of the script.
		*	It turns the result as a KScriptInteface::Result, and a return
		*	value as a QVariant
		*/
		void done( KScriptClientInterface::Result result, const TQVariant &returned )  {emit scriptDone(result, returned);}

	public slots:
		/**
		*	Run the selected script
		*/
		void runScript( const TQString &scriptName, TQObject *context = 0, const TQVariant &arg = 0 );
	signals:
		/**
		*	Send out a signal of the error message from the current running
		*	script.
		*/
		void scriptError( const TQString &msg );
		/**
		*	Send out a signal of the warning message from the current running
		*	script.
		*/
		void scriptWarning( const TQString &msg );
		/**
		*	Send out a signal of the output message from the current running
		*	script.
		*/
		void scriptOutput( const TQString &msg );
		/**
		*	Send out a signal of the progress of the current running
		*	script.
		*/
		void scriptProgress( int percent);
		/**
		*	Send out a signal of the exit status of the script
		*
		*/
		void scriptDone( KScriptClientInterface::Result result, const TQVariant &returned);
	protected:
		TQDict<ScriptInfo> m_scripts;
		TQDict<KScriptInterface> m_scriptCache;
		//TQStringList m_scriptNames;
		TQString m_currentScript;
	};
//};
#endif
