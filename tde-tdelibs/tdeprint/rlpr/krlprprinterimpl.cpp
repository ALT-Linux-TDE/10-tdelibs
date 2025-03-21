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

#include "krlprprinterimpl.h"
#include "kprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kmprinter.h"

#include <tqfile.h>
#include <kstandarddirs.h>
#include <tdeconfig.h>
#include <tdelocale.h>

KRlprPrinterImpl::KRlprPrinterImpl(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KPrinterImpl(parent,name)
{
}

KRlprPrinterImpl::~KRlprPrinterImpl()
{
}

bool KRlprPrinterImpl::setupCommand(TQString& cmd, KPrinter *printer)
{
	// retrieve the KMPrinter object, to get host and queue name
	KMPrinter	*rpr = KMFactory::self()->manager()->findPrinter(printer->printerName());
	if (!rpr)
		return false;

	TQString	host(rpr->option("host")), queue(rpr->option("queue"));
	if (!host.isEmpty() && !queue.isEmpty())
	{
		TQString		exestr = TDEStandardDirs::findExe("rlpr");
		if (exestr.isEmpty())
		{
			printer->setErrorMessage(i18n("The <b>%1</b> executable could not be found in your path. Check your installation.").arg("rlpr"));
			return false;
		}

		cmd = TQString::fromLatin1("%1 -H %2 -P %3 -\\#%4").arg(exestr).arg(quote(host)).arg(quote(queue)).arg(printer->numCopies());

		// proxy settings
		TDEConfig	*conf = KMFactory::self()->printConfig();
		conf->setGroup("RLPR");
		TQString	host = conf->readEntry("ProxyHost",TQString::null), port = conf->readEntry("ProxyPort",TQString::null);
		if (!host.isEmpty())
		{
			cmd.append(" -X ").append(quote(host));
			if (!port.isEmpty()) cmd.append(" --port=").append(port);
		}

		return true;
	}
	else
	{
		printer->setErrorMessage(i18n("The printer is incompletely defined. Try to reinstall it."));
		return false;
	}
}
