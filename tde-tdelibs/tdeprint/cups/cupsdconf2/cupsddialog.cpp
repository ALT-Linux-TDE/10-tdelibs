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

#include "cupsddialog.h"

#include "cupsdpage.h"
#include "cupsdconf.h"
#include "cupsdsplash.h"
#include "cupsdserverpage.h"
#include "cupsdlogpage.h"
#include "cupsdjobspage.h"
#include "cupsdfilterpage.h"
#include "cupsddirpage.h"
#include "cupsdnetworkpage.h"
#include "cupsdbrowsingpage.h"
#include "cupsdsecuritypage.h"

#include <tqdir.h>
#include <tqvbox.h>
#include <tdemessagebox.h>
#include <tdelocale.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <tqstringlist.h>
#include <tqwhatsthis.h>
#include <tdeio/passdlg.h>
#include <kguiitem.h>
#include <kprocess.h>
#include <tqprocess.h>

#include <stdlib.h>
#include <signal.h>
#include <cups/cups.h>

static bool	dynamically_loaded = false;
static TQString	pass_string;

extern "C"
{
#include "cups-util.h"
	TDEPRINT_EXPORT bool restartServer(TQString& msg)
	{
		return CupsdDialog::restartServer(msg);
	}
	TDEPRINT_EXPORT bool configureServer(TQWidget *parent, TQString& msg)
	{
		dynamically_loaded = true;
		bool result = CupsdDialog::configure(TQString::null, parent, &msg);
		dynamically_loaded = false;
		return result;
	}
}

int getServerPid()
{
#if defined(__OpenBSD__) || defined(__FreeBSD__)
	TQProcess *proc = new TQProcess();
	proc->addArgument("pgrep");
	proc->addArgument("cupsd");
	proc->start();
	while (proc->isRunning()); //Wait for process to exit
	TQString pidString = proc->readLineStdout();
	bool ok;
	int pid = pidString.toInt(&ok);
	if (ok) return pid;
	return (-1);
#else
	TQDir	dir("/proc",TQString::null,TQDir::Name,TQDir::Dirs);
	for (uint i=0;i<dir.count();i++)
	{
		if (dir[i] == "." || dir[i] == ".." || dir[i] == "self") continue;
		TQFile	f("/proc/" + dir[i] + "/cmdline");
		if (f.exists() && f.open(IO_ReadOnly))
		{
			TQTextStream	t(&f);
			TQString	line;
			t >> line;
			f.close();
			if (line.right(5) == "cupsd" ||
			    line.right(6).left(5) == "cupsd")	// second condition for 2.4.x kernels
								// which add a null byte at the end
				return dir[i].toInt();
		}
	}
	return (-1);
#endif
}

const char* getPassword(const char*)
{
	TQString	user(cupsUser());
	TQString	pass;

	if (TDEIO::PasswordDialog::getNameAndPassword(user, pass, NULL) == TQDialog::Accepted)
	{
		cupsSetUser(user.latin1());
		pass_string = pass;
		if (pass_string.isEmpty())
			return "";
		else
			return pass_string.latin1();
	}
	else
		return NULL;
}

//---------------------------------------------------

CupsdDialog::CupsdDialog(TQWidget *parent, const char *name)
	: KDialogBase(IconList, "", Ok|Cancel|User1, Ok, parent, name, true, true, KGuiItem(i18n("Short Help"), "help"))
{
	TDEGlobal::iconLoader()->addAppDir("tdeprint");
	TDEGlobal::locale()->insertCatalogue("cupsdconf");

	setShowIconsInTreeList(true);
	setRootIsDecorated(false);

	pagelist_.setAutoDelete(false);
	filename_ = "";
	conf_ = 0;
	constructDialog();

        setCaption(i18n("CUPS Server Configuration"));

        //resize(500, 400);
}

CupsdDialog::~CupsdDialog()
{
        delete conf_;
}

void CupsdDialog::addConfPage(CupsdPage *page)
{
	TQPixmap icon = TDEGlobal::instance()->iconLoader()->loadIcon(
	                                                           page->pixmap(),
                                                                   TDEIcon::NoGroup,
                                                                   TDEIcon::SizeMedium
	                                                          );

	TQVBox	*box = addVBoxPage(page->pageLabel(), page->header(), icon);
	page->reparent(box, TQPoint(0,0));
	pagelist_.append(page);
}

void CupsdDialog::constructDialog()
{
	addConfPage(new CupsdSplash(0));
	addConfPage(new CupsdServerPage(0));
	addConfPage(new CupsdNetworkPage(0));
	addConfPage(new CupsdSecurityPage(0));
	addConfPage(new CupsdLogPage(0));
	addConfPage(new CupsdJobsPage(0));
	addConfPage(new CupsdFilterPage(0));
	addConfPage(new CupsdDirPage(0));
	addConfPage(new CupsdBrowsingPage(0));

	conf_ = new CupsdConf();
	for (pagelist_.first();pagelist_.current();pagelist_.next())
        {
                pagelist_.current()->setInfos(conf_);
        }
}

bool CupsdDialog::setConfigFile(const TQString& filename)
{
	filename_ = filename;
	if (!conf_->loadFromFile(filename_))
	{
		KMessageBox::error(this, i18n("Error while loading configuration file!"), i18n("CUPS Configuration Error"));
		return false;
	}
	if (conf_->unknown_.count() > 0)
	{
		// there were some unknown options, warn the user
		TQString	msg;
		for (TQValueList< TQPair<TQString,TQString> >::ConstIterator it=conf_->unknown_.begin(); it!=conf_->unknown_.end(); ++it)
			msg += ((*it).first + " = " + (*it).second + "<br>");
		msg.prepend("<p>" + i18n("Some options were not recognized by this configuration tool. "
		                          "They will be left untouched and you won't be able to change them.") + "</p>");
		KMessageBox::sorry(this, msg, i18n("Unrecognized Options"));
	}
	bool	ok(true);
	TQString	msg;
	for (pagelist_.first();pagelist_.current() && ok;pagelist_.next())
		ok = pagelist_.current()->loadConfig(conf_, msg);
	if (!ok)
	{
		KMessageBox::error(this, msg.prepend("<qt>").append("</qt>"), i18n("CUPS Configuration Error"));
		return false;
	}
	return true;
}

bool CupsdDialog::restartServer(TQString& msg)
{
	int	serverPid = getServerPid();
        msg.truncate(0);
	if (serverPid <= 0)
	{
		msg = i18n("Unable to find a running CUPS server");
	}
	else
	{
                bool success = false;
		TDEProcess proc;
		proc << "tdesu" << "-c" << "/etc/init.d/cupsys restart";
		success = proc.start( TDEProcess::Block ) && proc.normalExit();
                if( !success )    
			msg = i18n("Unable to restart CUPS server (pid = %1)").arg(serverPid);
	}
        return (msg.isEmpty());
}

bool CupsdDialog::configure(const TQString& filename, TQWidget *parent, TQString *msg)
{
	bool needUpload(false);
	TQString errormsg;
	bool result = true;

	// init password dialog if needed
	if (!dynamically_loaded)
		cupsSetPasswordCB(getPassword);

	// load config file from server
	TQString	fn(filename);
	if (fn.isEmpty())
	{
		fn = cupsGetConf();
		if (fn.isEmpty())
			errormsg = i18n("Unable to retrieve configuration file from the CUPS server. "
				        "You probably don't have the access permissions to perform this operation.");
		else needUpload = true;
	}

	// check read state (only if needed)
	if (!fn.isEmpty())
	{
		TQFileInfo	fi(fn);
		if (!fi.exists() || !fi.isReadable() || !fi.isWritable())
			errormsg = i18n("Internal error: file '%1' not readable/writable!").arg(fn);
		// check file size
		if (fi.size() == 0)
			errormsg = i18n("Internal error: empty file '%1'!").arg(fn);
	}

	if (!errormsg.isEmpty())
	{
		if ( !dynamically_loaded )
			KMessageBox::error(parent, errormsg.prepend("<qt>").append("</qt>"), i18n("CUPS Configuration Error"));
		result = false;
	}
	else
	{
		TDEGlobal::locale()->insertCatalogue("cupsdconf"); // Must be before dialog is created to translate "Short Help"
		CupsdDialog	dlg(parent);
		if (dlg.setConfigFile(fn) && dlg.exec())
		{
			TQCString	encodedFn = TQFile::encodeName(fn);
			if (!needUpload)
				KMessageBox::information(parent,
					i18n("The config file has not been uploaded to the "
					     "CUPS server. The daemon will not be restarted."));
			else if (!cupsPutConf(encodedFn.data()))
			{
				errormsg = i18n("Unable to upload the configuration file to CUPS server. "
					     "You probably don't have the access permissions to perform this operation.");
				if ( !dynamically_loaded )
					KMessageBox::error(parent, errormsg, i18n("CUPS configuration error"));
				result = false;
			}
		}

	}
	if (needUpload)
		TQFile::remove(fn);

	if ( msg )
		*msg = errormsg;
	return result;
}

void CupsdDialog::slotOk()
{
	if (conf_ && !filename_.isEmpty())
	{ // try to save the file
		bool	ok(true);
		TQString	msg;
		CupsdConf	newconf_;
		for (pagelist_.first();pagelist_.current() && ok;pagelist_.next())
			ok = pagelist_.current()->saveConfig(&newconf_, msg);
		// copy unknown options
		newconf_.unknown_ = conf_->unknown_;
		if (!ok)
		{
			; // do nothing
		}
		else if (!newconf_.saveToFile(filename_))
		{
			msg = i18n("Unable to write configuration file %1").arg(filename_);
				ok = false;
		}
		if (!ok)
		{
			KMessageBox::error(this, msg.prepend("<qt>").append("</qt>"), i18n("CUPS Configuration Error"));
		}
		else
			KDialogBase::slotOk();
	}
}

void CupsdDialog::slotUser1()
{
	TQWhatsThis::enterWhatsThisMode();
}

int CupsdDialog::serverPid()
{
	return getServerPid();
}

int CupsdDialog::serverOwner()
{
	int	pid = getServerPid();
	if (pid > 0)
	{
		TQString	str;
		str.sprintf("/proc/%d/status",pid);
		TQFile	f(str);
		if (f.exists() && f.open(IO_ReadOnly))
		{
			TQTextStream	t(&f);
			while (!t.eof())
			{
				str = t.readLine();
				if (str.find("Uid:",0,false) == 0)
				{
					TQStringList	list = TQStringList::split('\t', str, false);
					if (list.count() >= 2)
					{
						bool	ok;
						int	u = list[1].toInt(&ok);
						if (ok) return u;
					}
				}
			}
		}
	}
	return (-1);
}

#include "cupsddialog.moc"
