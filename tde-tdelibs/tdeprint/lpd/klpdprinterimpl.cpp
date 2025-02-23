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

#include "klpdprinterimpl.h"
#include "kprinter.h"

#include <tqfile.h>
#include <kstandarddirs.h>
#include <tdelocale.h>

KLpdPrinterImpl::KLpdPrinterImpl(TQObject *parent, const char *name)
: KPrinterImpl(parent,name)
{
}

KLpdPrinterImpl::~KLpdPrinterImpl()
{
}

TQString KLpdPrinterImpl::executable()
{
	return TDEStandardDirs::findExe("lpr");
}

bool KLpdPrinterImpl::setupCommand(TQString& cmd, KPrinter *printer)
{
	TQString	exestr = executable();
	if (exestr.isEmpty())
	{
		printer->setErrorMessage(i18n("The <b>%1</b> executable could not be found in your path. Check your installation.").arg("lpr"));
		return false;
	}
	cmd = TQString::fromLatin1("%1 -P %2 '-#%3'").arg(exestr).arg(quote(printer->printerName())).arg(printer->numCopies());
	return true;
}
