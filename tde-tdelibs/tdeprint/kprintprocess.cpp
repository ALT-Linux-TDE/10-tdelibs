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

#include "kprintprocess.h"
#include <tdeapplication.h>
#include <tdelocale.h>
#include <tqfile.h>

KPrintProcess::KPrintProcess()
: KShellProcess()
{
	// redirect everything to a single buffer
	connect(this,TQ_SIGNAL(receivedStdout(TDEProcess*,char*,int)),TQ_SLOT(slotReceivedStderr(TDEProcess*,char*,int)));
	connect(this,TQ_SIGNAL(receivedStderr(TDEProcess*,char*,int)),TQ_SLOT(slotReceivedStderr(TDEProcess*,char*,int)));
	connect( this, TQ_SIGNAL( processExited( TDEProcess* ) ), TQ_SLOT( slotExited( TDEProcess* ) ) );
	m_state = None;
}

KPrintProcess::~KPrintProcess()
{
	if ( !m_tempoutput.isEmpty() )
		TQFile::remove( m_tempoutput );
	if ( m_tempfiles.size() > 0 )
		for ( TQStringList::ConstIterator it=m_tempfiles.begin(); it!=m_tempfiles.end(); ++it )
			TQFile::remove( *it );
}

TQString KPrintProcess::errorMessage() const
{
	return m_buffer;
}

bool KPrintProcess::print()
{
	m_buffer = TQString();
	m_state = Printing;
	return start(NotifyOnExit,All);
}

void KPrintProcess::slotReceivedStderr(TDEProcess *proc, char *buf, int len)
{
	if (proc == this)
	{
		TQCString	str = TQCString(buf,len).stripWhiteSpace();
		m_buffer.append(str.append("\n"));
	}
}

void KPrintProcess::slotExited( TDEProcess* )
{
	switch ( m_state )
	{
		case Printing:
			if ( !m_output.isEmpty() )
			{
				clearArguments();
				*this << "kfmclient" << "copy" << m_tempoutput << m_output;
				m_state = Finishing;
				m_buffer = i18n( "File transfer failed." );
				if ( start( NotifyOnExit ) )
					return;
			}
		case Finishing:
			if ( !normalExit() )
				emit printError( this, i18n( "Abnormal process termination (<b>%1</b>)." ).arg( m_command ) );
			else if ( exitStatus() != 0 )
				emit printError( this, i18n( "<b>%1</b>: execution failed with message:<p>%2</p>" ).arg( m_command ).arg( m_buffer ) );
			else
				emit printTerminated( this );
			break;
		default:
			emit printError( this, "Internal error, printing terminated in unexpected state. "
					"Report bug at <a href=\"http://bugs.trinitydesktop.org\">http://bugs.trinitydesktop.org</a>." );
			break;
	}
}

#include "kprintprocess.moc"
