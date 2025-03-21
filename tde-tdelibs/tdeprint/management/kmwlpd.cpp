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

#include <config.h>
#include "kmwlpd.h"
#include "kmwizard.h"
#include "kmprinter.h"

#include <kurl.h>
#include <tdelocale.h>
#include <tqlabel.h>
#include <kdebug.h>
#include <tqlineedit.h>
#include <tdemessagebox.h>
#include <kextsock.h>

static bool checkLpdQueue(const char *host, const char *queue);

//********************************************************************************************************

KMWLpd::KMWLpd(TQWidget *parent, const char *name)
: KMWInfoBase(2,parent,name)
{
	m_ID = KMWizard::LPD;
	m_title = i18n("LPD Queue Information");
	m_nextpage = KMWizard::Driver;

	setInfo(i18n("<p>Enter the information concerning the remote LPD queue; "
		     "this wizard will check it before continuing.</p>"));
	setLabel(0,i18n("Host:"));
	setLabel(1,i18n("Queue:"));
}

bool KMWLpd::isValid(TQString& msg)
{
	if (text(0).isEmpty() || text(1).isEmpty())
	{
		msg = i18n("Some information is missing.");
		return false;
	}

	// check LPD queue
	if (!checkLpdQueue(text(0).latin1(),text(1).latin1()))
	{
		if (KMessageBox::warningContinueCancel(this, i18n("Cannot find queue %1 on server %2; do you want to continue anyway?").arg(text(1)).arg(text(0))) == KMessageBox::Cancel)
			return false;
	}
	return true;
}

void KMWLpd::updatePrinter(KMPrinter *p)
{
	TQString	dev = TQString::fromLatin1("lpd://%1/%2").arg(text(0)).arg(text(1));
	p->setDevice(dev);
}

//*******************************************************************************************************

bool checkLpdQueue(const char *host, const char *queue)
{
	KExtendedSocket	sock(host, "printer", KExtendedSocket::streamSocket);
	sock.setBlockingMode(true);
	if (sock.connect() != 0)
		return false;

	char	res[64] = {0};
	snprintf(res,64,"%c%s\n",(char)4,queue);
	if (sock.writeBlock(res, strlen(res)) != (TQ_LONG)(strlen(res)))
		return false;

	char	buf[1024] = {0};
	int	n, tot(1);
	while ((n = sock.readBlock(res, 63)) > 0)
	{
		res[n] = 0;
		tot += n;
		if (tot >= 1024)
			break;
		else
			strcat(buf, res);
	}
	sock.close();
	if (strlen(buf) == 0 || strstr(buf, "unknown printer") != NULL)
		return false;
	return true;
}
