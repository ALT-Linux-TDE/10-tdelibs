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

#include "kfoomaticprinterimpl.h"
#include "kprinter.h"

#include <kstandarddirs.h>
#include <tdelocale.h>

KFoomaticPrinterImpl::KFoomaticPrinterImpl(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KPrinterImpl(parent,name)
{
}

KFoomaticPrinterImpl::~KFoomaticPrinterImpl()
{
}

// look for executable
TQString KFoomaticPrinterImpl::executable()
{
	TQString	exe = TDEStandardDirs::findExe("foomatic-printjob");
	return exe;
}

bool KFoomaticPrinterImpl::setupCommand(TQString& cmd, KPrinter *printer)
{
	TQString		exe = executable();
	if (!exe.isEmpty())
	{
		cmd = exe + TQString::fromLatin1(" -P %1 -# %2").arg(quote(printer->printerName())).arg(printer->numCopies());
		return true;
	}
	else
		printer->setErrorMessage(i18n("No valid print executable was found in your path. Check your installation."));
	return false;
}
