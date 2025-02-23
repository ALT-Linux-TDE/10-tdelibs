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
#ifndef __shellscript_h__
#define __shellscript_h__

#include <scriptinterface.h>
#include <tqvariant.h>
#include <tqobject.h>
#include <kprocess.h>
//using namespace KScriptInterface;
class ShellScript :  public KScriptInterface
{
	TQ_OBJECT
public:
	ShellScript(KScriptClientInterface *parent, const char *name, const TQStringList &args);
	virtual ~ShellScript();
	TQString script() const;
	void setScript( const TQString &scriptFile );
	void setScript( const TQString &scriptLibFile, const TQString &method );
	void run(TQObject *context = 0, const TQVariant &arg = 0);
	void kill();
private slots:
	void Exit(TDEProcess *proc);
	void stdErr(TDEProcess *proc, char *buffer, int buflen);
	void stdOut(TDEProcess *proc, char *buffer, int buflen);
private:
	TDEProcess *m_script;
	KScriptClientInterface *ScriptClientInterface;
	TQString m_scriptName;
};

#endif
