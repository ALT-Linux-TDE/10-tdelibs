/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
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

#include "kmwsocketutil.h"

#include <tqprogressbar.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqpushbutton.h>
#include <tdemessagebox.h>
#include <tqlayout.h>
#include <tqregexp.h>
#include <knumvalidator.h>

#include <tdeapplication.h>
#include <tdelocale.h>
#include <kextsock.h>
#include <kdebug.h>

#include <unistd.h>

TQString localRootIP();

//----------------------------------------------------------------------------------------

SocketConfig::SocketConfig(KMWSocketUtil *util, TQWidget *parent, const char *name)
: KDialogBase(parent, name, true, TQString::null, Ok|Cancel, Ok, true)
{
	TQWidget	*dummy = new TQWidget(this);
	setMainWidget(dummy);
        KIntValidator *val = new KIntValidator( this );
	TQLabel	*masklabel = new TQLabel(i18n("&Subnetwork:"),dummy);
	TQLabel	*portlabel = new TQLabel(i18n("&Port:"),dummy);
	TQLabel	*toutlabel = new TQLabel(i18n("&Timeout (ms):"),dummy);
	TQLineEdit	*mm = new TQLineEdit(dummy);
	mm->setText(TQString::fromLatin1(".[0-255]"));
	mm->setReadOnly(true);
	mm->setFixedWidth(fontMetrics().width(mm->text())+10);

	mask_ = new TQLineEdit(dummy);
	mask_->setAlignment(TQt::AlignRight);
	port_ = new TQComboBox(true,dummy);
        if ( port_->lineEdit() )
            port_->lineEdit()->setValidator( val );
	tout_ = new TQLineEdit(dummy);
        tout_->setValidator( val );

	masklabel->setBuddy(mask_);
	portlabel->setBuddy(port_);
	toutlabel->setBuddy(tout_);

	mask_->setText(util->root_);
	port_->insertItem("631");
	port_->insertItem("9100");
	port_->insertItem("9101");
	port_->insertItem("9102");
	port_->setEditText(TQString::number(util->port_));
	tout_->setText(TQString::number(util->timeout_));

	TQGridLayout	*main_ = new TQGridLayout(dummy, 3, 2, 0, 10);
	TQHBoxLayout	*lay1 = new TQHBoxLayout(0, 0, 5);
	main_->addWidget(masklabel, 0, 0);
	main_->addWidget(portlabel, 1, 0);
	main_->addWidget(toutlabel, 2, 0);
	main_->addLayout(lay1, 0, 1);
	main_->addWidget(port_, 1, 1);
	main_->addWidget(tout_, 2, 1);
	lay1->addWidget(mask_,1);
	lay1->addWidget(mm,0);

	resize(250,130);
	setCaption(i18n("Scan Configuration"));
}

SocketConfig::~SocketConfig()
{
}

void SocketConfig::slotOk()
{
	TQString	msg;
	TQRegExp	re("(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})");
	if (!re.exactMatch(mask_->text()))
		msg = i18n("Wrong subnetwork specification.");
	else
	{
		for (int i=1; i<=3; i++)
			if (re.cap(i).toInt() >= 255)
			{
				msg = i18n("Wrong subnetwork specification.");
				break;
			}
	}

	bool 	ok(false);
	int 	v = tout_->text().toInt(&ok);
	if (!ok || v <= 0)
		msg = i18n("Wrong timeout specification.");
	v = port_->currentText().toInt(&ok);
	if (!ok || v <= 0)
		msg = i18n("Wrong port specification.");
	if (!msg.isEmpty())
	{
		KMessageBox::error(this,msg);
		return;
	}

	KDialogBase::slotOk();
}

//----------------------------------------------------------------------------------------

KMWSocketUtil::KMWSocketUtil()
{
	printerlist_.setAutoDelete(true);
	root_ = localRootIP();
	port_ = 9100;
	timeout_ = 50;
}

bool KMWSocketUtil::checkPrinter(const TQString& IPstr, int port, TQString* hostname)
{
	KExtendedSocket	sock(IPstr, port, KExtendedSocket::inetSocket|KExtendedSocket::streamSocket);
	bool	result(false);
	sock.setTimeout(0, timeout_ * 1000);
	if (sock.connect() == 0)
	{
		if (hostname)
		{
			TQString	portname;
			KExtendedSocket::resolve((TDESocketAddress*)(sock.peerAddress()), *hostname, portname);
		}
		result = true;
	}
	sock.close();
	return result;
}

bool KMWSocketUtil::scanNetwork(TQProgressBar *bar)
{
	printerlist_.setAutoDelete(true);
	printerlist_.clear();
	int	n(256);
	if (bar)
		bar->setTotalSteps(n);
	for (int i=0; i<n; i++)
	{
		TQString	IPstr = root_ + "." + TQString::number(i);
		TQString	hostname;
		if (checkPrinter(IPstr, port_, &hostname))
		{ // we found a printer at this address, create SocketInfo entry in printer list
			SocketInfo	*info = new SocketInfo;
			info->IP = IPstr;
			info->Port = port_;
			info->Name = hostname;
			printerlist_.append(info);
		}
		if (bar)
		{
			bar->setProgress(i);
			kapp->flushX();
		}
	}
	return true;
}

void KMWSocketUtil::configureScan(TQWidget *parent)
{
	SocketConfig	*dlg = new SocketConfig(this,parent);
	if (dlg->exec())
	{
		root_ = dlg->mask_->text();
		port_ = dlg->port_->currentText().toInt();
		timeout_ = dlg->tout_->text().toInt();
	}
        delete dlg;
}

//----------------------------------------------------------------------------------------

TQString localRootIP()
{
	char	buf[256];
	buf[0] = '\0';
	if (!gethostname(buf, sizeof(buf)))
		buf[sizeof(buf)-1] = '\0';
	TQPtrList<KAddressInfo>	infos = KExtendedSocket::lookup(buf, TQString::null);
	infos.setAutoDelete(true);
	if (infos.count() > 0)
	{
		TQString	IPstr = infos.first()->address()->nodeName();
		int	p = IPstr.findRev('.');
		IPstr.truncate(p);
		return IPstr;
	}
	return TQString::null;
}

#include "kmwsocketutil.moc"
