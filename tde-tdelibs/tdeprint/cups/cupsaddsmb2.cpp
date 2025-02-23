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

#include "cupsaddsmb2.h"
#include "cupsinfos.h"
#include "sidepixmap.h"

#include <tqtimer.h>
#include <tqprogressbar.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tqmessagebox.h>
#include <tqfile.h>
#include <tdeio/passdlg.h>
#include <kdebug.h>
#include <kseparator.h>
#include <kactivelabel.h>
#include <tqwhatsthis.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include <cups/cups.h>
#include <cups/ppd.h>
#include <ctype.h>

CupsAddSmb::CupsAddSmb(TQWidget *parent, const char *name)
: KDialog(parent, name)
{
	m_state = None;
	m_status = false;
	m_actionindex = 0;
	connect(&m_proc, TQ_SIGNAL(receivedStdout(TDEProcess*,char*,int)), TQ_SLOT(slotReceived(TDEProcess*,char*,int)));
	connect(&m_proc, TQ_SIGNAL(receivedStderr(TDEProcess*,char*,int)), TQ_SLOT(slotReceived(TDEProcess*,char*,int)));
	connect(&m_proc, TQ_SIGNAL(processExited(TDEProcess*)), TQ_SLOT(slotProcessExited(TDEProcess*)));

	m_side = new SidePixmap(this);
	m_doit = new TQPushButton(i18n("&Export"), this);
	m_cancel = new KPushButton(KStdGuiItem::cancel(), this);
	connect(m_cancel, TQ_SIGNAL(clicked()), TQ_SLOT(reject()));
	connect(m_doit, TQ_SIGNAL(clicked()), TQ_SLOT(slotActionClicked()));
	m_bar = new TQProgressBar(this);
	m_text = new KActiveLabel(this);
	TQLabel	*m_title = new TQLabel(i18n("Export Printer Driver to Windows Clients"), this);
	setCaption(m_title->text());
	TQFont	f(m_title->font());
	f.setBold(true);
	m_title->setFont(f);
	KSeparator	*m_sep = new KSeparator(TQt::Horizontal, this);
	m_textinfo = new TQLabel( this );
	m_logined = new TQLineEdit( this );
	m_passwded = new TQLineEdit( this );
	m_passwded->setEchoMode( TQLineEdit::Password );
	m_servered = new TQLineEdit( this );
	TQLabel *m_loginlab = new TQLabel( i18n( "&Username:" ), this );
	TQLabel *m_serverlab = new TQLabel( i18n( "&Samba server:" ), this );
	TQLabel *m_passwdlab = new TQLabel( i18n( "&Password:" ), this );
	m_loginlab->setBuddy( m_logined );
	m_serverlab->setBuddy( m_servered );
	m_passwdlab->setBuddy( m_passwded );

	TQString txt = i18n( "<p><b>Samba server</b></p>"
						"Adobe Windows PostScript driver files plus the CUPS printer PPD will be "
						"exported to the <tt>[print$]</tt> special share of the Samba server (to change "
						"the source CUPS server, use the <nobr><i>Configure Manager -> CUPS server</i></nobr> first). "
						"The <tt>[print$]</tt> share must exist on the Samba side prior to clicking the "
						"<b>Export</b> button below." );
	TQWhatsThis::add( m_serverlab, txt );
	TQWhatsThis::add( m_servered, txt );

	txt = i18n( "<p><b>Samba username</b></p>"
				"User needs to have write access to the <tt>[print$]</tt> share on the Samba server. "
				"<tt>[print$]</tt> holds printer drivers prepared for download to Windows clients. "
				"This dialog does not work for Samba servers configured with <tt>security = share</tt> "
				"(but works fine with <tt>security = user</tt>)." );
	TQWhatsThis::add( m_loginlab, txt );
	TQWhatsThis::add( m_logined, txt );

	txt = i18n( "<p><b>Samba password</b></p>"
				"The Samba setting <tt>encrypt passwords = yes</tt> "
				"(default) requires prior use of <tt>smbpasswd -a [username]</tt> command, "
				"to create an encrypted Samba password and have Samba recognize it." );
	TQWhatsThis::add( m_passwdlab, txt );
	TQWhatsThis::add( m_passwded, txt );

	TQHBoxLayout	*l0 = new TQHBoxLayout(this, 10, 10);
	TQVBoxLayout	*l1 = new TQVBoxLayout(0, 0, 10);
	l0->addWidget(m_side);
	l0->addLayout(l1);
	l1->addWidget(m_title);
	l1->addWidget(m_sep);
	l1->addWidget(m_text);
	TQGridLayout *l3 = new TQGridLayout( 0, 3, 2, 0, 10 );
	l1->addLayout( l3 );
	l3->addWidget( m_loginlab, 1, 0 );
	l3->addWidget( m_passwdlab, 2, 0 );
	l3->addWidget( m_serverlab, 0, 0 );
	l3->addWidget( m_logined, 1, 1 );
	l3->addWidget( m_passwded, 2, 1 );
	l3->addWidget( m_servered, 0, 1 );
	l3->setColStretch( 1, 1 );
	l1->addSpacing( 10 );
	l1->addWidget(m_bar);
	l1->addWidget( m_textinfo );
	l1->addSpacing(30);
	TQHBoxLayout	*l2 = new TQHBoxLayout(0, 0, 10);
	l1->addLayout(l2);
	l2->addStretch(1);
	l2->addWidget(m_doit);
	l2->addWidget(m_cancel);

	m_logined->setText( CupsInfos::self()->login() );
	m_passwded->setText( CupsInfos::self()->password() );
	m_servered->setText( cupsServer() );

	setMinimumHeight(400);
}

CupsAddSmb::~CupsAddSmb()
{
}

void CupsAddSmb::slotActionClicked()
{
	if (m_state == None)
		doExport();
	else if (m_proc.isRunning())
		m_proc.kill();
}

void CupsAddSmb::slotReceived(TDEProcess*, char *buf, int buflen)
{
	TQString	line;
	int		index(0);
	bool	partial(false);
	static bool incomplete(false);

	kdDebug(500) << "slotReceived()" << endl;
	while (1)
	{
		// read a line
		line = TQString::fromLatin1("");
		partial = true;
		while (index < buflen)
		{
			TQChar	c(buf[index++]);
			if (c == '\n')
			{
				partial = false;
				break;
			}
			else if (c.isPrint())
				line += c;
		}

		if (line.isEmpty())
		{
			kdDebug(500) << "NOTHING TO READ" << endl;
			return;
		}

		kdDebug(500) << "ANSWER = " << line << " (END = " << line.length() << ")" << endl;
		if (!partial)
		{
			if (incomplete && m_buffer.count() > 0)
				m_buffer[m_buffer.size()-1].append(line);
			else
				m_buffer << line;
			incomplete = false;
			kdDebug(500) << "COMPLETE LINE READ (" << m_buffer.count() << ")" << endl;
		}
		else
		{
			if (line.startsWith("smb:") || line.startsWith("rpcclient $"))
			{
				kdDebug(500) << "END OF ACTION" << endl;
				checkActionStatus();
				if (m_status)
					nextAction();
				else
				{
					// quit program
					kdDebug(500) << "EXITING PROGRAM..." << endl;
					m_proc.writeStdin("quit\n", 5);
					kdDebug(500) << "SENT" << endl;
				}
				return;
			}
			else
			{
				if (incomplete && m_buffer.count() > 0)
					m_buffer[m_buffer.size()-1].append(line);
				else
					m_buffer << line;
				incomplete = true;
				kdDebug(500) << "INCOMPLETE LINE READ (" << m_buffer.count() << ")" << endl;
			}
		}
	}
}

void CupsAddSmb::checkActionStatus()
{
	m_status = false;
	// when checking the status, we need to take into account the
	// echo of the command in the output buffer.
	switch (m_state)
	{
		case None:
		case Start:
			m_status = true;
			break;
		case Copy:
			m_status = (m_buffer.count() == 0);
			break;
		case MkDir:
			m_status = (m_buffer.count() == 1 || m_buffer[1].find("ERRfilexists") != -1);
			break;
		case AddDriver:
		case AddPrinter:
			m_status = (m_buffer.count() == 1 || !m_buffer[1].startsWith("result"));
			break;
	}
	kdDebug(500) << "ACTION STATUS = " << m_status << endl;
}

void CupsAddSmb::nextAction()
{
	if (m_actionindex < (int)(m_actions.count()))
		TQTimer::singleShot(1, this, TQ_SLOT(doNextAction()));
}

void CupsAddSmb::doNextAction()
{
	m_buffer.clear();
	m_state = None;
	if (m_proc.isRunning())
	{
		TQCString	s = m_actions[m_actionindex++].latin1();
		m_bar->setProgress(m_bar->progress()+1);
		kdDebug(500) << "NEXT ACTION = " << s << endl;
		if (s == "quit")
		{
			// do nothing
		}
		else if (s == "mkdir")
		{
			m_state = MkDir;
			//m_text->setText(i18n("Creating directory %1").arg(m_actions[m_actionindex]));
			m_textinfo->setText(i18n("Creating folder %1").arg(m_actions[m_actionindex]));
			s.append(" ").append(m_actions[m_actionindex].latin1());
			m_actionindex++;
		}
		else if (s == "put")
		{
			m_state = Copy;
			//m_text->setText(i18n("Uploading %1").arg(m_actions[m_actionindex+1]));
			m_textinfo->setText(i18n("Uploading %1").arg(m_actions[m_actionindex+1]));
			s.append(" ").append(TQFile::encodeName(m_actions[m_actionindex]).data()).append(" ").append(m_actions[m_actionindex+1].latin1());
			m_actionindex += 2;
		}
		else if (s == "adddriver")
		{
			m_state = AddDriver;
			//m_text->setText(i18n("Installing driver for %1").arg(m_actions[m_actionindex]));
			m_textinfo->setText(i18n("Installing driver for %1").arg(m_actions[m_actionindex]));
			s.append(" \"").append(m_actions[m_actionindex].latin1()).append("\" \"").append(m_actions[m_actionindex+1].latin1()).append("\"");
			m_actionindex += 2;
		}
		else if (s == "addprinter" || s == "setdriver")
		{
			m_state = AddPrinter;
			//m_text->setText(i18n("Installing printer %1").arg(m_actions[m_actionindex]));
			m_textinfo->setText(i18n("Installing printer %1").arg(m_actions[m_actionindex]));
			TQCString	dest = m_actions[m_actionindex].local8Bit();
			if (s == "addprinter")
				s.append(" ").append(dest).append(" ").append(dest).append(" \"").append(dest).append("\" \"\"");
			else
				s.append(" ").append(dest).append(" ").append(dest);
			m_actionindex++;
		}
		else
		{
			kdDebug(500) << "ACTION = unknown action" << endl;
			m_proc.kill();
			return;
		}
		// send action
		kdDebug(500) << "ACTION = " << s << endl;
		s.append("\n");
		m_proc.writeStdin(s.data(), s.length());
	}
}

void CupsAddSmb::slotProcessExited(TDEProcess*)
{
	kdDebug(500) << "PROCESS EXITED (" << m_state << ")" << endl;
	if (m_proc.normalExit() && m_state != Start && m_status)
	{
		// last process went OK. If it was smbclient, then switch to rpcclient
		if (tqstrncmp(m_proc.args().first(), "smbclient", 9) == 0)
		{
			doInstall();
			return;
		}
		else
		{
			m_doit->setEnabled(false);
			m_cancel->setEnabled(true);
			m_cancel->setText(i18n("&Close"));
			m_cancel->setDefault(true);
			m_cancel->setFocus();
			m_logined->setEnabled( true );
			m_servered->setEnabled( true );
			m_passwded->setEnabled( true );
			m_text->setText(i18n("Driver successfully exported."));
			m_bar->reset();
			m_textinfo->setText( TQString::null );
			return;
		}
	}

	if (m_proc.normalExit())
	{
		showError(
				i18n("Operation failed. Possible reasons are: permission denied "
				     "or invalid Samba configuration (see <a href=\"man:/cupsaddsmb\">"
					 "cupsaddsmb</a> manual page for detailed information, you need "
					 "<a href=\"http://www.cups.org\">CUPS</a> version 1.1.11 or higher). "
					 "You may want to try again with another login/password."));

	}
	else
	{
		showError(i18n("Operation aborted (process killed)."));
	}
}

void CupsAddSmb::showError(const TQString& msg)
{
	m_text->setText(i18n("<h3>Operation failed.</h3><p>%1</p>").arg(msg));
	m_cancel->setEnabled(true);
	m_logined->setEnabled( true );
	m_servered->setEnabled( true );
	m_passwded->setEnabled( true );
	m_doit->setText(i18n("&Export"));
	m_state = None;
}

bool CupsAddSmb::exportDest(const TQString &dest, const TQString& datadir)
{
	CupsAddSmb	dlg;
	dlg.m_dest = dest;
	dlg.m_datadir = datadir;
	dlg.m_text->setText(
			i18n( "You are about to prepare the <b>%1</b> driver to be "
			      "shared out to Windows clients through Samba. This operation "
				  "requires the <a href=\"http://www.adobe.com/products/printerdrivers/main.html\">Adobe PostScript Driver</a>, a recent version of "
				  "Samba 2.2.x and a running SMB service on the target server. "
				  "Click <b>Export</b> to start the operation. Read the <a href=\"man:/cupsaddsmb\">cupsaddsmb</a> "
				  "manual page in Konqueror or type <tt>man cupsaddsmb</tt> in a "
				  "console window to learn more about this functionality." ).arg( dest ) );
	dlg.exec();
	return dlg.m_status;
}

bool CupsAddSmb::doExport()
{
	m_status = false;
	m_state = None;

	if (!TQFile::exists(m_datadir+"/drivers/ADOBEPS5.DLL") ||
	    !TQFile::exists(m_datadir+"/drivers/ADOBEPS4.DRV"))
	{
		showError(
			i18n("Some driver files are missing. You can get them on "
			     "<a href=\"http://www.adobe.com\">Adobe</a> web site. "
			     "See <a href=\"man:/cupsaddsmb\">cupsaddsmb</a> manual "
				 "page for more details (you need <a href=\"http://www.cups.org\">CUPS</a> "
				 "version 1.1.11 or higher)."));
		return false;
	}

	m_bar->setTotalSteps(18);
	m_bar->setProgress(0);
	//m_text->setText(i18n("<p>Preparing to upload driver to host <b>%1</b>").arg(m_servered->text()));
	m_textinfo->setText(i18n("Preparing to upload driver to host %1").arg(m_servered->text()));
	m_cancel->setEnabled(false);
	m_logined->setEnabled( false );
	m_servered->setEnabled( false );
	m_passwded->setEnabled( false );
	m_doit->setText(i18n("&Abort"));

	const char	*ppdfile;

	if ((ppdfile = cupsGetPPD(m_dest.local8Bit())) == NULL)
	{
		showError(i18n("The driver for printer <b>%1</b> could not be found.").arg(m_dest));
		return false;
	}

	m_actions.clear();
	m_actions << "mkdir" << "W32X86";
	m_actions << "put" << ppdfile << "W32X86/"+m_dest+".PPD";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPS5.DLL" << "W32X86/ADOBEPS5.DLL";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPSU.DLL" << "W32X86/ADOBEPSU.DLL";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPSU.HLP" << "W32X86/ADOBEPSU.HLP";
	m_actions << "mkdir" << "WIN40";
	m_actions << "put" << ppdfile << "WIN40/"+m_dest+".PPD";
	m_actions << "put" << m_datadir+"/drivers/ADFONTS.MFM" << "WIN40/ADFONTS.MFM";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPS4.DRV" << "WIN40/ADOBEPS4.DRV";
	m_actions << "put" << m_datadir+"/drivers/ADOBEPS4.HLP" << "WIN40/ADOBEPS4.HLP";
	m_actions << "put" << m_datadir+"/drivers/DEFPRTR2.PPD" << "WIN40/DEFPRTR2.PPD";
	m_actions << "put" << m_datadir+"/drivers/ICONLIB.DLL" << "WIN40/ICONLIB.DLL";
	m_actions << "put" << m_datadir+"/drivers/PSMON.DLL" << "WIN40/PSMON.DLL";
	m_actions << "quit";

	m_proc.clearArguments();
	m_proc << "smbclient" << TQString::fromLatin1("//")+m_servered->text()+"/print$";
	return startProcess();
}

bool CupsAddSmb::doInstall()
{
	m_status = false;
	m_state = None;

	m_actions.clear();
	m_actions << "adddriver" << "Windows NT x86" << m_dest+":ADOBEPS5.DLL:"+m_dest+".PPD:ADOBEPSU.DLL:ADOBEPSU.HLP:NULL:RAW:NULL";
	// seems to be wrong with newer versions of Samba
	//m_actions << "addprinter" << m_dest;
	m_actions << "adddriver" << "Windows 4.0" << m_dest+":ADOBEPS4.DRV:"+m_dest+".PPD:NULL:ADOBEPS4.HLP:PSMON.DLL:RAW:ADFONTS.MFM,DEFPRTR2.PPD,ICONLIB.DLL";
	// seems to be ok with newer versions of Samba
	m_actions << "setdriver" << m_dest;
	m_actions << "quit";

	//m_text->setText(i18n("Preparing to install driver on host <b>%1</b>").arg(m_servered->text()));
	m_textinfo->setText(i18n("Preparing to install driver on host %1").arg(m_servered->text()));

	m_proc.clearArguments();
	m_proc << "rpcclient" << m_servered->text();
	return startProcess();
}

bool CupsAddSmb::startProcess()
{
	m_proc << "-d" << "0" << "-N" << "-U";
	if (m_passwded->text().isEmpty())
		m_proc << m_logined->text();
	else
		m_proc << m_logined->text()+"%"+m_passwded->text();
	m_state = Start;
	m_actionindex = 0;
	m_buffer.clear();
	kdDebug(500) << "PROCESS STARTED = " << m_proc.args()[0] << endl;
	return m_proc.start(TDEProcess::NotifyOnExit, TDEProcess::All);
}

#include "cupsaddsmb2.moc"
