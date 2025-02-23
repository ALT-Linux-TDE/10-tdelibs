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

#include "escpwidget.h"

#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqaccel.h>
#include <tdemacros.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kdialogbase.h>
#include <klibloader.h>
#include <kseparator.h>
#include <kdebug.h>

class EscpFactory : public KLibFactory
{
public:
	EscpFactory(TQObject *parent = 0, const char *name = 0) : KLibFactory(parent, name) {}
protected:
	TQObject* createObject(TQObject *parent = 0, const char *name = 0, const char * className = "TQObject", const TQStringList& args = TQStringList())
	{
               Q_UNUSED(className);
		KDialogBase	*dlg = new KDialogBase(static_cast<TQWidget*>(parent), name, true, i18n("EPSON InkJet Printer Utilities"), KDialogBase::Close);
		EscpWidget	*w = new EscpWidget(dlg);
		if (args.count() > 0)
			w->setDevice(args[0]);
		if (args.count() > 1)
			w->setPrinterName(args[1]);
		dlg->setMainWidget(w);
		return dlg;
	}
};

extern "C"
{
	void* init_tdeprint_tool_escputil() TDE_EXPORT;
	void* init_tdeprint_tool_escputil()
	{
		return new EscpFactory;
	}
}

EscpWidget::EscpWidget(TQWidget *parent, const char *name)
: TQWidget(parent, name)
{
	m_hasoutput = false;

	connect(&m_proc, TQ_SIGNAL(processExited(TDEProcess*)), TQ_SLOT(slotProcessExited(TDEProcess*)));
	connect(&m_proc, TQ_SIGNAL(receivedStdout(TDEProcess*,char*,int)), TQ_SLOT(slotReceivedStdout(TDEProcess*,char*,int)));
	connect(&m_proc, TQ_SIGNAL(receivedStderr(TDEProcess*,char*,int)), TQ_SLOT(slotReceivedStderr(TDEProcess*,char*,int)));

	TQPushButton	*cleanbtn = new TQPushButton(this, "-c");
	cleanbtn->setPixmap(DesktopIcon("application-x-executable"));
	TQPushButton	*nozzlebtn = new TQPushButton(this, "-n");
	nozzlebtn->setPixmap(DesktopIcon("application-x-executable"));
	TQPushButton	*alignbtn = new TQPushButton(this, "-a");
	alignbtn->setPixmap(DesktopIcon("application-x-executable"));
	TQPushButton	*inkbtn = new TQPushButton(this, "-i");
	inkbtn->setPixmap(DesktopIcon("tdeprint_inklevel"));
	TQPushButton	*identbtn = new TQPushButton(this, "-d");
	identbtn->setPixmap(DesktopIcon("application-x-executable"));

	TQFont	f(font());
	f.setBold(true);
	m_printer = new TQLabel(this);
	m_printer->setFont(f);
	m_device = new TQLabel(this);
	m_device->setFont(f);
	m_useraw = new TQCheckBox(i18n("&Use direct connection (might need root permissions)"), this);

	connect(cleanbtn, TQ_SIGNAL(clicked()), TQ_SLOT(slotButtonClicked()));
	connect(nozzlebtn, TQ_SIGNAL(clicked()), TQ_SLOT(slotButtonClicked()));
	connect(alignbtn, TQ_SIGNAL(clicked()), TQ_SLOT(slotButtonClicked()));
	connect(inkbtn, TQ_SIGNAL(clicked()), TQ_SLOT(slotButtonClicked()));
	connect(identbtn, TQ_SIGNAL(clicked()), TQ_SLOT(slotButtonClicked()));

	TQLabel	*printerlab = new TQLabel(i18n("Printer:"), this);
	printerlab->setAlignment(AlignRight|AlignVCenter);
	TQLabel	*devicelab = new TQLabel(i18n("Device:"), this);
	devicelab->setAlignment(AlignRight|AlignVCenter);
	TQLabel	*cleanlab = new TQLabel(i18n("Clea&n print head"), this);
	TQLabel	*nozzlelab = new TQLabel(i18n("&Print a nozzle test pattern"), this);
	TQLabel	*alignlab = new TQLabel(i18n("&Align print head"), this);
	TQLabel	*inklab = new TQLabel(i18n("&Ink level"), this);
	TQLabel	*identlab = new TQLabel(i18n("P&rinter identification"), this);

	cleanlab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
	nozzlelab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
	alignlab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
	inklab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);
	identlab->setAlignment(AlignLeft|AlignVCenter|ShowPrefix);

	cleanbtn->setAccel(TQAccel::shortcutKey(cleanlab->text()));
	nozzlebtn->setAccel(TQAccel::shortcutKey(nozzlelab->text()));
	alignbtn->setAccel(TQAccel::shortcutKey(alignlab->text()));
	inkbtn->setAccel(TQAccel::shortcutKey(inklab->text()));
	identbtn->setAccel(TQAccel::shortcutKey(identlab->text()));

	KSeparator	*sep = new KSeparator(this);
	sep->setFixedHeight(10);

	TQGridLayout	*l0 = new TQGridLayout(this, 8, 2, 10, 10);
	TQGridLayout	*l1 = new TQGridLayout(0, 2, 2, 0, 5);
	l0->addMultiCellLayout(l1, 0, 0, 0, 1);
	l1->addWidget(printerlab, 0, 0);
	l1->addWidget(devicelab, 1, 0);
	l1->addWidget(m_printer, 0, 1);
	l1->addWidget(m_device, 1, 1);
	l1->setColStretch(1, 1);
	l0->addMultiCellWidget(sep, 1, 1, 0, 1);
	l0->addWidget(cleanbtn, 2, 0);
	l0->addWidget(nozzlebtn, 3, 0);
	l0->addWidget(alignbtn, 4, 0);
	l0->addWidget(inkbtn, 5, 0);
	l0->addWidget(identbtn, 6, 0);
	l0->addWidget(cleanlab, 2, 1);
	l0->addWidget(nozzlelab, 3, 1);
	l0->addWidget(alignlab, 4, 1);
	l0->addWidget(inklab, 5, 1);
	l0->addWidget(identlab, 6, 1);
	l0->addMultiCellWidget(m_useraw, 7, 7, 0, 1);
	l0->setColStretch(1, 1);
}

void EscpWidget::startCommand(const TQString& arg)
{
	bool	useUSB(false);

	if (m_deviceURL.isEmpty())
	{
		KMessageBox::error(this, i18n("Internal error: no device set."));
		return;
	}
	else
	{
		TQString	protocol = m_deviceURL.protocol();
		if (protocol == "usb")
			useUSB = true;
		else if (protocol != "file" && protocol != "parallel" && protocol != "serial" && !protocol.isEmpty())
		{
			KMessageBox::error(this,
				i18n("Unsupported connection type: %1").arg(protocol));
			return;
		}
	}

	if (m_proc.isRunning())
	{
		KMessageBox::error(this, i18n("An escputil process is still running. "
		                              "You must wait until its completion before continuing."));
		return;
	}

	TQString	exestr = TDEStandardDirs::findExe("escputil");
	if (exestr.isEmpty())
	{
		KMessageBox::error(this, i18n("The executable escputil cannot be found in your "
		                              "PATH environment variable. Make sure gimp-print is "
		                              "installed and that escputil is in your PATH."));
		return;
	}

	m_proc.clearArguments();
	m_proc << exestr;
	if (m_useraw->isChecked() || arg == "-i")
		m_proc << "-r" << m_deviceURL.path();
	else
		m_proc << "-P" << m_printer->text();
	if (useUSB)
		m_proc << "-u";

	m_proc << arg << "-q";
	m_errorbuffer = m_outbuffer = TQString::null;
	m_hasoutput = ( arg == "-i" || arg == "-d" );
	for ( TQValueList<TQCString>::ConstIterator it=m_proc.args().begin(); it!=m_proc.args().end(); ++it )
		kdDebug() << "ARG: " << *it << endl;
	if (m_proc.start(TDEProcess::NotifyOnExit, TDEProcess::AllOutput))
		setEnabled(false);
	else
	{
		KMessageBox::error(this,
			i18n("Internal error: unable to start escputil process."));
		return;
	}
}

void EscpWidget::slotProcessExited(TDEProcess*)
{
	setEnabled(true);
	if (!m_proc.normalExit() || m_proc.exitStatus() != 0)
	{
		TQString	msg1 = "<qt>"+i18n("Operation terminated with errors.")+"</qt>";
		TQString	msg2;
		if (!m_outbuffer.isEmpty())
			msg2 += "<p><b><u>"+i18n("Output")+"</u></b></p><p>"+m_outbuffer+"</p>";
		if (!m_errorbuffer.isEmpty())
			msg2 += "<p><b><u>"+i18n("Error")+"</u></b></p><p>"+m_errorbuffer+"</p>";
		if (!msg2.isEmpty())
			KMessageBox::detailedError(this, msg1, msg2);
		else
			KMessageBox::error(this, msg1);
	}
	else if ( !m_outbuffer.isEmpty() && m_hasoutput )
	{
		KMessageBox::information( this, m_outbuffer );
	}
	m_hasoutput = false;
}

void EscpWidget::slotReceivedStdout(TDEProcess*, char *buf, int len)
{
	TQString	bufstr = TQCString(buf, len);
	m_outbuffer.append(bufstr);
}

void EscpWidget::slotReceivedStderr(TDEProcess*, char *buf, int len)
{
	TQString	bufstr = TQCString(buf, len);
	m_errorbuffer.append(bufstr);
}

void EscpWidget::slotButtonClicked()
{
	TQString	arg = sender()->name();
	startCommand(arg);
}

void EscpWidget::setPrinterName(const TQString& p)
{
	m_printer->setText(p);
}

void EscpWidget::setDevice(const TQString& dev)
{
	m_deviceURL = dev;
	m_device->setText(dev);
}

#include "escpwidget.moc"
