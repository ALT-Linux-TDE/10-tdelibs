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

#include "tdeprintd.h"
#include "kprintprocess.h"

#include <tqfile.h>
#include <tdelocale.h>
#include <knotifyclient.h>
#include <tdemessagebox.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <tdeio/passdlg.h>
#include <tdeio/authinfo.h>
#include <tqlabel.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kstandarddirs.h>
#include <twin.h>
#include <tdeapplication.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqregexp.h>

#include <unistd.h>

extern "C"
{
	TDE_EXPORT KDEDModule *create_tdeprintd(const TQCString& name)
	{
		return new KDEPrintd(name);
	}
}

class StatusWindow : public TQWidget
{
public:
	StatusWindow(int pid = -1);
	void setMessage(const TQString&);
	int pid() const { return m_pid; }

private:
	TQLabel		*m_label;
	TQPushButton	*m_button;
	int		m_pid;
	TQLabel		*m_icon;
};

StatusWindow::StatusWindow(int pid)
: TQWidget(NULL, "StatusWindow", (WFlags)(WType_TopLevel|WStyle_DialogBorder|WStyle_StaysOnTop|WDestructiveClose)), m_pid(pid)
{
	m_label = new TQLabel(this);
	m_label->setAlignment(AlignCenter);
	m_button = new KPushButton(KStdGuiItem::close(), this);
	m_icon = new TQLabel(this);
	m_icon->setPixmap(DesktopIcon("document-print"));
	m_icon->setAlignment(AlignCenter);
	KWin::setIcons(winId(), *(m_icon->pixmap()), SmallIcon("document-print"));
	TQGridLayout	*l0 = new TQGridLayout(this, 2, 3, 10, 10);
	l0->setRowStretch(0, 1);
	l0->setColStretch(1, 1);
	l0->addMultiCellWidget(m_label, 0, 0, 1, 2);
	l0->addWidget(m_button, 1, 2);
	l0->addMultiCellWidget(m_icon, 0, 1, 0, 0);
	connect(m_button, TQ_SIGNAL(clicked()), TQ_SLOT(hide()));
	resize(200, 50);
}

void StatusWindow::setMessage(const TQString& msg)
{
	//QSize	oldSz = size();
	m_label->setText(msg);
	//QSize	sz = m_label->sizeHint();
	//sz += TQSize(layout()->margin()*2, layout()->margin()*2+layout()->spacing()+m_button->sizeHint().height());
	// dialog will never be smaller
	//sz = sz.expandedTo(oldSz);
	//resize(sz);
	//setFixedSize(sz);
	//layout()->activate();
}

//*****************************************************************************************************

KDEPrintd::KDEPrintd(const TQCString& obj)
: KDEDModule(obj)
{
	m_processpool.setAutoDelete(true);
	m_windows.setAutoDelete(false);
	m_requestsPending.setAutoDelete( true );
}

KDEPrintd::~KDEPrintd()
{
}

int KDEPrintd::print(const TQString& cmd, const TQStringList& files, bool remflag)
{
	KPrintProcess *proc = new KPrintProcess;
	TQString	command(cmd);
	TQRegExp re( "\\$out\\{([^}]*)\\}" );

	connect(proc,TQ_SIGNAL(printTerminated(KPrintProcess*)),TQ_SLOT(slotPrintTerminated(KPrintProcess*)));
	connect(proc,TQ_SIGNAL(printError(KPrintProcess*,const TQString&)),TQ_SLOT(slotPrintError(KPrintProcess*,const TQString&)));
	proc->setCommand( command );
	if ( re.search( command ) != -1 )
	{
		KURL url( re.cap( 1 ) );
		if ( !url.isLocalFile() )
		{
			TQString tmpFilename = locateLocal( "tmp", "tdeprint_" + kapp->randomString( 8 ) );
			command.replace( re, TDEProcess::quote( tmpFilename ) );
			proc->setOutput( re.cap( 1 ) );
			proc->setTempOutput( tmpFilename );
		}
		else
			command.replace( re, TDEProcess::quote( re.cap( 1 ) ) );
	}

	if ( checkFiles( command, files ) )
	{
		*proc << command;
		if ( remflag )
			proc->setTempFiles( files );
		if ( proc->print() )
		{
			m_processpool.append( proc );
			return ( int )proc->pid();
		}
	}

	delete proc;
	return -1;
}

void KDEPrintd::slotPrintTerminated( KPrintProcess *proc )
{
	m_processpool.removeRef( proc );
}

void KDEPrintd::slotPrintError( KPrintProcess *proc, const TQString& msg )
{
	KNotifyClient::event("printerror",i18n("<p><nobr>A print error occurred. Error message received from system:</nobr></p><br>%1").arg(msg));
	m_processpool.removeRef( proc );
}

TQString KDEPrintd::openPassDlg(const TQString& user)
{
	TQString	user_(user), pass_, result;
	if (TDEIO::PasswordDialog::getNameAndPassword(user_, pass_, NULL) == KDialog::Accepted)
		result.append(user_).append(":").append(pass_);
	return result;
}

bool KDEPrintd::checkFiles(TQString& cmd, const TQStringList& files)
{
	for (TQStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
		if (::access(TQFile::encodeName(*it).data(), R_OK) != 0)
		{
			if (KMessageBox::warningContinueCancel(0,
				i18n("Some of the files to print are not readable by the TDE "
				     "print daemon. This may happen if you are trying to print "
				     "as a different user to the one currently logged in. To continue "
				     "printing, you need to provide root's password."),
				TQString::null,
				i18n("Provide root's Password"),
				"provideRootsPassword") == KMessageBox::Continue)
			{
				cmd = ("tdesu -c " + TDEProcess::quote(cmd));
				break;
			}
			else
				return false;
		}
	return true;
}

void KDEPrintd::statusMessage(const TQString& msg, int pid, const TQString& appName)
{
	StatusWindow	*w = m_windows.find(pid);
	if (!w && !msg.isEmpty())
	{
		w = new StatusWindow(pid);
		if (appName.isEmpty())
			w->setCaption(i18n("Printing Status - %1").arg("(pid="+TQString::number(pid)+")"));
		else
			w->setCaption(i18n("Printing Status - %1").arg(appName));
		connect(w, TQ_SIGNAL(destroyed()), TQ_SLOT(slotClosed()));
		w->show();
		m_windows.insert(pid, w);
	}
	if (w)
	{
		if (!msg.isEmpty())
			w->setMessage(msg);
		else
			w->close();
	}
}

void KDEPrintd::slotClosed()
{
	const StatusWindow	*w = static_cast<const StatusWindow*>(sender());
	if (w)
	{
		m_windows.remove(w->pid());
	}
}

//******************************************************************************************

class KDEPrintd::Request
{
public:
	DCOPClientTransaction *transaction;
	TQString user;
	TQString uri;
	int seqNbr;
};

TQString KDEPrintd::requestPassword( const TQString& user, const TQString& host, int port, int seqNbr )
{
	Request *req = new Request;
	req->user = user;
	req->uri = "print://" + user + "@" + host + ":" + TQString::number(port);
	req->seqNbr = seqNbr;
	req->transaction = callingDcopClient()->beginTransaction();
	m_requestsPending.append( req );
	if ( m_requestsPending.count() == 1 )
		TQTimer::singleShot( 0, this, TQ_SLOT( processRequest() ) );
	return "::";
}

void KDEPrintd::processRequest()
{
	if ( m_requestsPending.count() == 0 )
		return;

	Request *req = m_requestsPending.first();
	TDEIO::AuthInfo info;
	TQByteArray params, reply;
	TQCString replyType;
	TQString authString( "::" );

	info.username = req->user;
	info.keepPassword = true;
	info.url = req->uri;
	info.comment = i18n( "Printing system" );

	TQDataStream input( params, IO_WriteOnly );
	input << info << TQString(i18n( "Authentication failed (user name=%1)" ).arg( info.username )) << 0L << (long int) req->seqNbr;
	if ( callingDcopClient()->call( "kded", "kpasswdserver", "queryAuthInfo(TDEIO::AuthInfo,TQString,long int,long int)",
				params, replyType, reply ) )
	{
		if ( replyType == "TDEIO::AuthInfo" )
		{
			TQDataStream output( reply, IO_ReadOnly );
			TDEIO::AuthInfo result;
			int seqNbr;
			output >> result >> seqNbr;

			if ( result.isModified() )
				authString = result.username + ":" + result.password + ":" + TQString::number( seqNbr );
		}
		else
			kdWarning( 500 ) << "DCOP returned type error, expected TDEIO::AuthInfo, received " << replyType << endl;
	}
	else
		kdWarning( 500 ) << "Cannot communicate with kded_kpasswdserver" << endl;

	TQByteArray outputData;
	TQDataStream output( outputData, IO_WriteOnly );
	output << authString;
	replyType = "TQString";
	callingDcopClient()->endTransaction( req->transaction, replyType, outputData );

	m_requestsPending.remove( ( unsigned int )0 );
	if ( m_requestsPending.count() > 0 )
		TQTimer::singleShot( 0, this, TQ_SLOT( processRequest() ) );
}

void KDEPrintd::initPassword( const TQString& user, const TQString& passwd, const TQString& host, int port )
{
	TQByteArray params, reply;
	TQCString replyType;
	TDEIO::AuthInfo info;

	info.username = user;
	info.password = passwd;
	info.url = "print://" + user + "@" + host + ":" + TQString::number(port);

	TQDataStream input( params, IO_WriteOnly );
	input << info << ( long int )0;

	if ( !callingDcopClient()->call( "kded", "kpasswdserver", "addAuthInfo(TDEIO::AuthInfo,long int)",
			params, replyType, reply ) )
		kdWarning( 500 ) << "Unable to initialize password, cannot communicate with kded_kpasswdserver" << endl;
}

#include "tdeprintd.moc"
