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

#include "klpdunixprinterimpl.h"
#include "kprinter.h"

#include <tqfile.h>
#include <kstandarddirs.h>
#include <tdelocale.h>
#include <kmacroexpander.h>

KLpdUnixPrinterImpl::KLpdUnixPrinterImpl(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KPrinterImpl(parent,name)
{
}

KLpdUnixPrinterImpl::~KLpdUnixPrinterImpl()
{
}

void KLpdUnixPrinterImpl::initLpPrint(TQString& cmd, KPrinter *printer)
{
	cmd += TQString::fromLatin1(" -d %1 -n%2").arg(quote(printer->printerName())).arg(printer->numCopies());
}

void KLpdUnixPrinterImpl::initLprPrint(TQString& cmd, KPrinter *printer)
{
	cmd += TQString::fromLatin1(" -P %1 '-#%2'").arg(quote(printer->printerName())).arg(printer->numCopies());
}

// look for executable, starting with "lpr"
TQString KLpdUnixPrinterImpl::executable()
{
	TQString	exe = TDEStandardDirs::findExe("lpr");
	if (exe.isEmpty())
		exe = TDEStandardDirs::findExe("lp");
	return exe;
}

bool KLpdUnixPrinterImpl::setupCommand(TQString& cmd, KPrinter *printer)
{
	TQString exe = printer->option( "kde-printcommand" );
	if ( exe.isEmpty() || exe == "<automatic>" )
	{
		exe = executable();
		if (!exe.isEmpty())
		{
			cmd = exe;
			if (exe.right(3) == "lpr")
				initLprPrint(cmd,printer);
			else
				initLpPrint(cmd,printer);
			return true;
		}
		else
			printer->setErrorMessage(i18n("No valid print executable was found in your path. Check your installation."));
		return false;
	}
	else
	{
		TQMap<TQString,TQString> map;
		map.insert( "printer", printer->printerName() );
		map.insert( "copies", TQString::number( printer->numCopies() ) );
		cmd = KMacroExpander::expandMacrosShellQuote( exe, map );
		return true;
	}
}
