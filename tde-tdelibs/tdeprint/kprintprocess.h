/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef KPRINTPROCESS_H
#define KPRINTPROCESS_H

#include <kprocess.h>
#include <tqstringlist.h>

class KPrintProcess : public KShellProcess
{
	TQ_OBJECT
public:
	KPrintProcess();
	~KPrintProcess();

	bool print();
	TQString errorMessage() const;

	void setOutput( const TQString& output );
	const TQString& output() const;
	void setTempOutput( const TQString& output );
	const TQString& tempOutput() const;
	void setTempFiles( const TQStringList& files );
	const TQStringList& tempFiles() const;
	void setCommand( const TQString& cmd );
	const TQString& command() const;

	enum State { None = 0, Printing, Finishing };
	int state() const;

signals:
	void printTerminated( KPrintProcess* );
	void printError( KPrintProcess*, const TQString& );

protected slots:
	void slotReceivedStderr(TDEProcess*, char*, int);
	void slotExited( TDEProcess* );

private:
	TQString	m_buffer;
	TQStringList m_tempfiles;
	TQString m_output, m_tempoutput, m_command;
	int m_state;
};

inline const TQString& KPrintProcess::output() const
{ return m_output; }

inline const TQString& KPrintProcess::tempOutput() const
{ return m_tempoutput; }

inline const TQStringList& KPrintProcess::tempFiles() const
{ return m_tempfiles; }

inline const TQString& KPrintProcess::command() const
{ return m_command; }

inline void KPrintProcess::setOutput( const TQString& s )
{ m_output = s; }

inline void KPrintProcess::setTempOutput( const TQString& s )
{ m_tempoutput = s; }

inline void KPrintProcess::setTempFiles( const TQStringList& l )
{ m_tempfiles = l; }

inline void KPrintProcess::setCommand( const TQString& c )
{ m_command = c; }

inline int KPrintProcess::state() const
{ return m_state; }

#endif
