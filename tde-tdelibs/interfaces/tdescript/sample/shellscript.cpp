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

#include "shellscript.h"
#include <kdebug.h>
#include <tdeapplication.h>
#include <dcopclient.h>

#include <kgenericfactory.h>
#include <scriptclientinterface.h>
//using namespace KScriptInterface;
typedef KGenericFactory<ShellScript, KScriptClientInterface> ShellScriptFactory;
K_EXPORT_COMPONENT_FACTORY( libshellscript, ShellScriptFactory( "ShellScript" ) )

ShellScript::ShellScript(KScriptClientInterface *parent, const char *, const TQStringList & ) : ScriptClientInterface(parent)
{
	m_script =  new TDEProcess();
	connect ( m_script, TQ_SIGNAL(processExited(TDEProcess *)), TQ_SLOT(Exit(TDEProcess *)));
	connect ( m_script, TQ_SIGNAL(receivedStdout(TDEProcess *, char *, int)), TQ_SLOT(stdOut(TDEProcess *, char *, int )));
	connect ( m_script, TQ_SIGNAL(receivedStderr(TDEProcess *, char *, int)), TQ_SLOT(stdErr(TDEProcess *, char *, int )));
	// Connect feedback signals and slots
	//kdDebug() << "Building new script engine" << endl;
}

ShellScript::~ShellScript()
{
}

TQString ShellScript::script() const
{
	return m_scriptName;
}

void ShellScript::setScript( const TQString &scriptFile  )
{
	m_scriptName = scriptFile;
	*m_script << "sh" << m_scriptName << kapp->dcopClient()->appId();
}

void ShellScript::setScript( const TQString &, const TQString & )
{
    // ### what is this?
}

void ShellScript::run(TQObject *, const TQVariant &)
{
	 m_script->start(TDEProcess::NotifyOnExit,TDEProcess::All);
}
void ShellScript::kill()
{
	if (!m_script->kill())		// Kill the process
		m_script->kill(9);	// Kill it harder
}

void ShellScript::Exit(TDEProcess *proc)
{
	ScriptClientInterface->done((KScriptClientInterface::Result)proc->exitStatus(), "");
}

void ShellScript::stdErr(TDEProcess *, char *buffer, int)
{
	ScriptClientInterface->error(buffer);
}
void ShellScript::stdOut(TDEProcess *, char *buffer, int)
{
	ScriptClientInterface->output(buffer);
}

#include "shellscript.moc"
