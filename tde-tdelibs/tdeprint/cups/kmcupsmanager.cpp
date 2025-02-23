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

#include "kmcupsmanager.h"
#include "kmprinter.h"
#include "ipprequest.h"
#include "cupsinfos.h"
#include "driver.h"
#include "kmfactory.h"
#include "kmdbentry.h"
#include "cupsaddsmb2.h"
#include "ippreportdlg.h"
#include "kpipeprocess.h"
#include "util.h"
#include "foomatic2loader.h"
#include "ppdloader.h"

#include <tqfile.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <tqtimer.h>
#include <tqsocket.h>
#include <tqdatetime.h>

#include <kdebug.h>
#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <tdesocketbase.h>
#include <klibloader.h>
#include <tdemessagebox.h>
#include <tdeaction.h>
#include <kdialogbase.h>
#include <kextendedsocket.h>
#include <kprocess.h>
#include <kbufferedsocket.h>
#include <kfilterdev.h>
#include <cups/cups.h>
#include <cups/ppd.h>
#include <math.h>

#define ppdi18n(s)	i18n(TQString::fromLocal8Bit(s).utf8())

static void extractMaticData(TQString& buf, const TQString& filename);
static TQString printerURI(KMPrinter *p, bool useExistingURI);
static TQString downloadDriver(KMPrinter *p);

static int trials = 5;

//*****************************************************************************************************

	KMCupsManager::KMCupsManager(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KMManager(parent,name)
{
	// be sure to create the CupsInfos object -> password
	// management is handled correctly.
	CupsInfos::self();
	m_cupsdconf = 0;
	m_currentprinter = 0;
	m_socket = 0;

	setHasManagement(true);
	setPrinterOperationMask(KMManager::PrinterAll);
	setServerOperationMask(KMManager::ServerAll);

	// change LANG variable so that CUPS is always using
	// english language: translation may only come from the PPD
	// itself, or from KDE.
	setenv("LANG", "en_US.UTF-8", 1);
}

KMCupsManager::~KMCupsManager()
{
	delete m_socket;
}

TQString KMCupsManager::driverDbCreationProgram()
{
	return TQString(__TDE_BINDIR).append(TQString::fromLatin1("/make_driver_db_cups"));
}

TQString KMCupsManager::driverDirectory()
{
	TQString	d = cupsInstallDir();
	if (d.isEmpty()) {
#if defined(__OpenBSD__) || defined(__FreeBSD__)
		d = "/usr/local";
#else
		d = "/usr";
#endif
	}
	d.append("/share/cups/model");
	// raw foomatic support
#if defined(__OpenBSD__) || defined(__FreeBSD__)
	d.append(":/usr/local/share/foomatic/db/source");
#else
	d.append(":/usr/share/foomatic/db/source");
	// compressed foomatic support
	d.append(":/usr/lib/cups/driver/foomatic-db-compressed-ppds");
#endif
	return d;
}

TQString KMCupsManager::cupsInstallDir()
{
	TDEConfig	*conf=  KMFactory::self()->printConfig();
	conf->setGroup("CUPS");
	TQString	dir = conf->readPathEntry("InstallDir");
	return dir;
}

void KMCupsManager::reportIppError(IppRequest *req)
{
	setErrorMsg(req->statusMessage());
}

bool KMCupsManager::createPrinter(KMPrinter *p)
{
	bool isclass = p->isClass(false), result(false);
	IppRequest	req;
	TQString		uri;

	uri = printerURI(p,false);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	// needed to avoid problems when changing printer name
	p->setUri(KURL(uri));

	if (isclass)
	{
		req.setOperation(CUPS_ADD_CLASS);
		TQStringList	members = p->members(), uris;
		TQString		s;
		s = TQString::fromLocal8Bit("ipp://%1/printers/").arg(CupsInfos::self()->hostaddr());
		for (TQStringList::ConstIterator it=members.begin(); it!=members.end(); ++it)
			uris.append(s+(*it));
		req.addURI(IPP_TAG_PRINTER,"member-uris",uris);
	}
	else
	{
		req.setOperation(CUPS_ADD_PRINTER);
		// only set the device-uri if needed, otherwise you may loose authentification
		// data (login/password in URI's like smb or ipp).
		KMPrinter	*otherP = findPrinter(p->printerName());
		if (!otherP || otherP->device() != p->device())
		{
			/**
			 * As now the device is a TQString instead of KURL, special encoding
			 * required for SMB is not needed anymore. Use a unique mechanism
			 * for all backends.
			 */
			req.addURI(IPP_TAG_PRINTER,"device-uri",p->device());
		}
		if (!p->option("kde-banners").isEmpty())
		{
			TQStringList	bans = TQStringList::split(',',p->option("kde-banners"),false);
			while (bans.count() < 2)
				bans.append("none");
			req.addName(IPP_TAG_PRINTER,"job-sheets-default",bans);
		}
		req.addInteger(IPP_TAG_PRINTER,"job-quota-period",p->option("job-quota-period").toInt());
		req.addInteger(IPP_TAG_PRINTER,"job-k-limit",p->option("job-k-limit").toInt());
		req.addInteger(IPP_TAG_PRINTER,"job-page-limit",p->option("job-page-limit").toInt());
		if (!p->option("requesting-user-name-denied").isEmpty())
			req.addName(IPP_TAG_PRINTER,"requesting-user-name-denied",TQStringList::split(",",p->option("requesting-user-name-denied"),false));
		else if (!p->option("requesting-user-name-allowed").isEmpty())
			req.addName(IPP_TAG_PRINTER,"requesting-user-name-allowed",TQStringList::split(",",p->option("requesting-user-name-allowed"),false));
		else
			req.addName(IPP_TAG_PRINTER,"requesting-user-name-allowed",TQString::fromLatin1("all"));
	}
	req.addText(IPP_TAG_PRINTER,"printer-info",p->description());
	req.addText(IPP_TAG_PRINTER,"printer-location",p->location());

	if (req.doRequest("/admin/"))
	{
		result = true;
		if (p->driver())
			result = savePrinterDriver(p,p->driver());
		if (result)
			upPrinter(p, true);
	}
	else reportIppError(&req);

	return result;
}

bool KMCupsManager::removePrinter(KMPrinter *p)
{
	bool	result = setPrinterState(p,CUPS_DELETE_PRINTER);
	return result;
}

bool KMCupsManager::enablePrinter(KMPrinter *p, bool state)
{
	return setPrinterState(p, (state ? CUPS_ACCEPT_JOBS : CUPS_REJECT_JOBS));
}

bool KMCupsManager::startPrinter(KMPrinter *p, bool state)
{
	return setPrinterState(p, (state ? IPP_RESUME_PRINTER : IPP_PAUSE_PRINTER));
}

bool KMCupsManager::setDefaultPrinter(KMPrinter *p)
{
	return setPrinterState(p,CUPS_SET_DEFAULT);
}

bool KMCupsManager::setPrinterState(KMPrinter *p, int state)
{
	IppRequest	req;
	TQString		uri;

	req.setOperation(state);
	uri = printerURI(p, true);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	if (req.doRequest("/admin/"))
		return true;
	reportIppError(&req);
	return false;
}

bool KMCupsManager::completePrinter(KMPrinter *p)
{
	if (completePrinterShort(p))
	{
		// driver informations
		TQString	ppdname = downloadDriver(p);
		ppd_file_t	*ppd = (ppdname.isEmpty() ? NULL : ppdOpenFile(ppdname.local8Bit()));
		if (ppd)
		{
			KMDBEntry	entry;
			// use the validation mechanism of KMDBEntry to
			// fill possible missing entries like manufacturer
			// or model.
			entry.manufacturer = ppd->manufacturer;
			entry.model = ppd->shortnickname;
			entry.modelname = ppd->modelname;
			// do not check the driver regarding the manager
			entry.validate(false);
			// update the KMPrinter object
			p->setManufacturer(entry.manufacturer);
			p->setModel(entry.model);
			p->setDriverInfo(TQString::fromLocal8Bit(ppd->nickname));
			ppdClose(ppd);
		}
		if (!ppdname.isEmpty())
			TQFile::remove(ppdname);

		return true;
	}
	return false;
}

bool KMCupsManager::completePrinterShort(KMPrinter *p)
{
	IppRequest	req;
	TQStringList	keys;
	TQString		uri;

	req.setOperation(IPP_GET_PRINTER_ATTRIBUTES);
	uri = printerURI(p, true);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);

	/*
	// change host and port for remote stuffs
	if (!p->uri().isEmpty())
	{
	// THIS IS AN UGLY HACK!! FIXME
	// This attempts a "pre-connection" to see if the host is
	// actually reachable.  It times out after 2 seconds at most,
	// preventing application freezes.
	m_hostSuccess = false;
	m_lookupDone = false;
	// Give 2 seconds to connect to the printer, or abort
	KExtendedSocket *kes = new KExtendedSocket(p->uri().host(),
	p->uri().port());
	connect(kes, TQ_SIGNAL(connectionSuccess()), this, TQ_SLOT(hostPingSlot()));
	connect(kes, TQ_SIGNAL(connectionFailed(int)), this, TQ_SLOT(hostPingFailedSlot()));
	if (kes->startAsyncConnect() != 0) {
	delete kes;
	m_hostSuccess = false;
	} else {
	TQDateTime tm = TQDateTime::currentDateTime().addSecs(2);
	while (!m_lookupDone && (TQDateTime::currentDateTime() < tm))
	tqApp->processEvents();

	kes->cancelAsyncConnect();

	delete kes;

	if (!m_lookupDone)
	m_hostSuccess = false;
	}

	if (m_hostSuccess == true) {
	req.setHost(p->uri().host());
	req.setPort(p->uri().port());
	}
	}
	*/

	// disable location as it has been transferred to listing (for filtering)
	//keys.append("printer-location");
	keys.append("printer-info");
	keys.append("printer-make-and-model");
	keys.append("job-sheets-default");
	keys.append("job-sheets-supported");
	keys.append("job-quota-period");
	keys.append("job-k-limit");
	keys.append("job-page-limit");
	keys.append("requesting-user-name-allowed");
	keys.append("requesting-user-name-denied");
	if (p->isClass(true))
	{
		keys.append("member-uris");
		keys.append("member-names");
	}
	else
		keys.append("device-uri");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

	if (req.doRequest("/printers/"))
	{
		TQString	value;
		if (req.text("printer-info",value)) p->setDescription(value);
		// disabled location
		//if (req.text("printer-location",value)) p->setLocation(value);
		if (req.text("printer-make-and-model",value)) p->setDriverInfo(value);
		if (req.uri("device-uri",value))
		{
			/**
			 * No specific treatment required as the device is
			 * a normal TQString instead of a KURL
			 */
			p->setDevice( value );
		}
		TQStringList	values;
		/*		if (req.uri("member-uris",values))
				{
				TQStringList	members;
				for (TQStringList::ConstIterator it=values.begin(); it!=values.end(); ++it)
				{
				int	p = (*it).findRev('/');
				if (p != -1)
				members.append((*it).right((*it).length()-p-1));
				}
				p->setMembers(members);
				}*/
		if (req.name("member-names",values))
			p->setMembers(values);
		// banners
		req.name("job-sheets-default",values);
		while (values.count() < 2) values.append("none");
		p->setOption("kde-banners",values.join(TQString::fromLatin1(",")));
		if (req.name("job-sheets-supported",values)) p->setOption("kde-banners-supported",values.join(TQString::fromLatin1(",")));

		// quotas
		int	ival;
		if (req.integer("job-quota-period",ival)) p->setOption("job-quota-period",TQString::number(ival));
		if (req.integer("job-k-limit",ival)) p->setOption("job-k-limit",TQString::number(ival));
		if (req.integer("job-page-limit",ival)) p->setOption("job-page-limit",TQString::number(ival));

		// access permissions (allow and deny are mutually exclusives)
		if (req.name("requesting-user-name-allowed",values) && values.count() > 0)
		{
			p->removeOption("requesting-user-name-denied");
			p->setOption("requesting-user-name-allowed",values.join(","));
		}
		if (req.name("requesting-user-name-denied",values) && values.count() > 0)
		{
			p->removeOption("requesting-user-name-allowed");
			p->setOption("requesting-user-name-denied",values.join(","));
		}

		return true;
	}

	reportIppError(&req);
	return false;
}

bool KMCupsManager::testPrinter(KMPrinter *p)
{
	return KMManager::testPrinter(p);
	/*
	   TQString	testpage = testPage();
	   if (testpage.isEmpty())
	   {
	   setErrorMsg(i18n("Unable to locate test page."));
	   return false;
	   }

	   IppRequest	req;
	   TQString		uri;

	   req.setOperation(IPP_PRINT_JOB);
	   uri = printerURI(p);
	   req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	   req.addMime(IPP_TAG_OPERATION,"document-format","application/postscript");
	   if (!CupsInfos::self()->login().isEmpty()) req.addName(IPP_TAG_OPERATION,"requesting-user-name",CupsInfos::self()->login());
	   req.addName(IPP_TAG_OPERATION,"job-name",TQString::fromLatin1("TDE Print Test"));
	   if (req.doFileRequest("/printers/",testpage))
	   return true;
	   reportIppError(&req);
	   return false;
	   */
}

void KMCupsManager::listPrinters()
{
	loadServerPrinters();
}

void KMCupsManager::loadServerPrinters()
{
	IppRequest	req;
	TQStringList	keys;

	// get printers
	req.setOperation(CUPS_GET_PRINTERS);
	keys.append("printer-name");
	keys.append("printer-type");
	keys.append("printer-state");
	// location needed for filtering
	keys.append("printer-location");
	keys.append("printer-uri-supported");
	keys.append("printer-is-accepting-jobs");
	req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

	// filtering by username (hides printers user doesn't have allowance to use)
	req.addName(IPP_TAG_OPERATION, "requesting-user-name", TQString(cupsUser()));

	if (req.doRequest("/printers/"))
	{
		processRequest(&req);

		// get classes
		req.init();
		req.setOperation(CUPS_GET_CLASSES);
		req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",keys);

		if (req.doRequest("/classes/"))
		{
			processRequest(&req);

			// load default
			req.init();
			req.setOperation(CUPS_GET_DEFAULT);
			req.addKeyword(IPP_TAG_OPERATION,"requested-attributes",TQString::fromLatin1("printer-name"));
			if (req.doRequest("/printers/"))
			{
				TQString	s = TQString::null;
				req.name("printer-name",s);
				setHardDefault(findPrinter(s));
			}
			// This request may fails for example if no printer is defined. Just
			// discard the error message. Indeed as we successfully got printers
			// and classes, the most probable reason why this request may fail is
			// because of no printer defined. The best would be to actually check
			// there's no printer (TODO).
			return;
		}
	}

	// something went wrong if we get there, report the error
	reportIppError(&req);
}

void KMCupsManager::processRequest(IppRequest* req)
{
	ipp_attribute_t	*attr = req->first();
	ipp_attribute_t	*nextAttr;
	KMPrinter	*printer = new KMPrinter();
	while (attr)
	{
#ifdef HAVE_CUPS_1_6
		TQString	attrname(ippGetName(attr));
		if (attrname == "printer-name")
		{
			TQString	value = TQString::fromLocal8Bit(ippGetString(attr, 0, NULL));
			printer->setName(value);
			printer->setPrinterName(value);
		}
		else if (attrname == "printer-type")
		{
			int	value = ippGetInteger(attr, 0);
			printer->setType(0);
			printer->addType(((value & CUPS_PRINTER_CLASS) || (value & CUPS_PRINTER_IMPLICIT) ? KMPrinter::Class : KMPrinter::Printer));
			if ((value & CUPS_PRINTER_REMOTE)) printer->addType(KMPrinter::Remote);
			if ((value & CUPS_PRINTER_IMPLICIT)) printer->addType(KMPrinter::Implicit);

			// convert printer-type attribute
			printer->setPrinterCap( ( value & CUPS_PRINTER_OPTIONS ) >> 2 );
		}
		else if (attrname == "printer-state")
		{
			switch (ippGetInteger(attr, 0))
			{
				case IPP_PRINTER_IDLE: printer->setState(KMPrinter::Idle); break;
				case IPP_PRINTER_PROCESSING: printer->setState(KMPrinter::Processing); break;
				case IPP_PRINTER_STOPPED: printer->setState(KMPrinter::Stopped); break;
			}
		}
		else if (attrname == "printer-uri-supported")
		{
			printer->setUri(KURL(ippGetString(attr, 0, NULL)));
		}
		else if (attrname == "printer-location")
		{
			printer->setLocation(TQString::fromLocal8Bit(ippGetString(attr, 0, NULL)));
		}
		else if (attrname == "printer-is-accepting-jobs")
		{
			printer->setAcceptJobs(ippGetBoolean(attr, 0));
		}

		nextAttr = ippNextAttribute(req->request());
		if (attrname.isEmpty() || (!nextAttr))
		{
			addPrinter(printer);
			printer = new KMPrinter();
		}
		attr = nextAttr;
#else // HAVE_CUPS_1_6
		TQString	attrname(attr->name);
		if (attrname == "printer-name")
		{
			TQString	value = TQString::fromLocal8Bit(attr->values[0].string.text);
			printer->setName(value);
			printer->setPrinterName(value);
		}
		else if (attrname == "printer-type")
		{
			int	value = attr->values[0].integer;
			printer->setType(0);
			printer->addType(((value & CUPS_PRINTER_CLASS) || (value & CUPS_PRINTER_IMPLICIT) ? KMPrinter::Class : KMPrinter::Printer));
			if ((value & CUPS_PRINTER_REMOTE)) printer->addType(KMPrinter::Remote);
			if ((value & CUPS_PRINTER_IMPLICIT)) printer->addType(KMPrinter::Implicit);

			// convert printer-type attribute
			printer->setPrinterCap( ( value & CUPS_PRINTER_OPTIONS ) >> 2 );
		}
		else if (attrname == "printer-state")
		{
			switch (attr->values[0].integer)
			{
				case IPP_PRINTER_IDLE: printer->setState(KMPrinter::Idle); break;
				case IPP_PRINTER_PROCESSING: printer->setState(KMPrinter::Processing); break;
				case IPP_PRINTER_STOPPED: printer->setState(KMPrinter::Stopped); break;
			}
		}
		else if (attrname == "printer-uri-supported")
		{
			printer->setUri(KURL(attr->values[0].string.text));
		}
		else if (attrname == "printer-location")
		{
			printer->setLocation(TQString::fromLocal8Bit(attr->values[0].string.text));
		}
		else if (attrname == "printer-is-accepting-jobs")
		{
			printer->setAcceptJobs(attr->values[0].boolean);
		}
		if (attrname.isEmpty() || attr == req->last())
		{
			addPrinter(printer);
			printer = new KMPrinter();
		}
		attr = attr->next;
#endif // HAVE_CUPS_1_6
	}
	delete printer;
}

DrMain* KMCupsManager::loadPrinterDriver(KMPrinter *p, bool)
{
	if (!p) 
		return NULL;

	if (p->isClass(true)) 
	{
		KMPrinter *first_class_member = NULL;
		/* find the first printer in the class */
 		first_class_member = findPrinter(p->members().first());
	  
		if (first_class_member == NULL) 
		{
			/* we didn't find a printer in the class */
			return NULL;
		}
		else
		{
			p = first_class_member;
		}
	}

	TQString	fname = downloadDriver(p);
	DrMain	*driver(0);
	if (!fname.isEmpty())
	{
		driver = loadDriverFile(fname);
		if (driver)
			driver->set("temporary",fname);
	}

	return driver;
}

DrMain* KMCupsManager::loadFileDriver(const TQString& filename)
{
	if (filename.startsWith("ppd:"))
		return loadDriverFile(filename.mid(4));
	else if (filename.startsWith("compressed-ppd:"))
		return loadDriverFile(filename);
	else if (filename.startsWith("foomatic/"))
		return loadMaticDriver(filename);
	else
		return loadDriverFile(filename);
}

DrMain* KMCupsManager::loadMaticDriver(const TQString& drname)
{
	TQStringList	comps = TQStringList::split('/', drname, false);
	TQString	tmpFile = locateLocal("tmp", "foomatic_" + kapp->randomString(8));
#if defined(__OpenBSD__) || defined(__FreeBSD__)
	TQString	PATH = getenv("PATH") + TQString::fromLatin1(":/usr/local/bin:/usr/sbin:/usr/local/sbin:/opt/sbin:/opt/local/sbin");
#else
	TQString	PATH = getenv("PATH") + TQString::fromLatin1(":/usr/sbin:/usr/local/sbin:/opt/sbin:/opt/local/sbin");
#endif
	TQString	exe = TDEStandardDirs::findExe("foomatic-datafile", PATH);
	if (exe.isEmpty())
	{
		setErrorMsg(i18n("Unable to find the executable foomatic-datafile "
					"in your PATH. Check that Foomatic is correctly installed."));
		return NULL;
	}

	KPipeProcess	in;
	TQFile		out(tmpFile);
	TQString cmd = TDEProcess::quote(exe);
	cmd += " -t cups -d ";
	cmd += TDEProcess::quote(comps[2]);
	cmd += " -p ";
	cmd += TDEProcess::quote(comps[1]);
	if (in.open(cmd) && out.open(IO_WriteOnly))
	{
		TQTextStream	tin(&in), tout(&out);
		TQString	line;
		while (!tin.atEnd())
		{
			line = tin.readLine();
			tout << line << endl;
		}
		in.close();
		out.close();

		DrMain	*driver = loadDriverFile(tmpFile);
		if (driver)
		{
			driver->set("template", tmpFile);
			driver->set("temporary", tmpFile);
			return driver;
		}
	}
	setErrorMsg(i18n("Unable to create the Foomatic driver [%1,%2]. "
				"Either that driver does not exist, or you don't have "
				"the required permissions to perform that operation.").arg(comps[1]).arg(comps[2]));
	TQFile::remove(tmpFile);
	return NULL;
}

DrMain* KMCupsManager::loadDriverFile(const TQString& fname)
{
	if ((fname.startsWith("compressed-ppd:")) || TQFile::exists(fname))
	{
		TQString msg; /* possible error message */
		DrMain *driver = PPDLoader::loadDriver( fname, &msg );
		if ( driver )
		{
			driver->set( "template", fname );
			// FIXME: should fix option in group "install"
		}
		else
			setErrorMsg( msg );
		return driver;
	}
	return NULL;
}

void KMCupsManager::saveDriverFile(DrMain *driver, const TQString& filename)
{
	kdDebug( 500 ) << "Saving PPD file with template=" << driver->get( "template" ) << endl;
	TQString templateFile = driver->get( "template" );
	if (templateFile.startsWith("compressed-ppd:")) {
		templateFile = driver->get( "temporary-cppd" );
	}
	TQIODevice *in = KFilterDev::deviceForFile( templateFile );
	TQFile	out(filename);
	if (in && in->open(IO_ReadOnly) && out.open(IO_WriteOnly))
	{
		TQTextStream	tin(in), tout(&out);
		TQString		line, keyword;
		bool 		isnumeric(false);
		DrBase		*opt(0);

		while (!tin.eof())
		{
			line = tin.readLine();
			if (line.startsWith("*% COMDATA #"))
			{
				int	p(-1), q(-1);
				if ((p=line.find("'name'")) != -1)
				{
					p = line.find('\'',p+6)+1;
					q = line.find('\'',p);
					keyword = line.mid(p,q-p);
					opt = driver->findOption(keyword);
					if (opt && (opt->type() == DrBase::Integer || opt->type() == DrBase::Float))
						isnumeric = true;
					else
						isnumeric = false;
				}
				/*else if ((p=line.find("'type'")) != -1)
				{
					p = line.find('\'',p+6)+1;
					if (line.find("float",p) != -1 || line.find("int",p) != -1)
						isnumeric = true;
					else
						isnumeric = false;
				}*/
				else if ((p=line.find("'default'")) != -1 && !keyword.isEmpty() && opt && isnumeric)
				{
					TQString	prefix = line.left(p+9);
					tout << prefix << " => '" << opt->valueText() << '\'';
					if (line.find(',',p) != -1)
						tout << ',';
					tout << endl;
					continue;
				}
				tout << line << endl;
			}
			else if (line.startsWith("*Default"))
			{
				int	p = line.find(':',8);
				keyword = line.mid(8,p-8);
				DrBase *bopt = 0;
				if ( keyword == "PageRegion" || keyword == "ImageableArea" || keyword == "PaperDimension" )
					bopt = driver->findOption( TQString::fromLatin1( "PageSize" ) );
				else
					bopt = driver->findOption( keyword );
				if (bopt)
					switch (bopt->type())
					{
						case DrBase::List:
						case DrBase::Boolean:
							{
								DrListOption	*opt = static_cast<DrListOption*>(bopt);
								if (opt && opt->currentChoice())
									tout << "*Default" << keyword << ": " << opt->currentChoice()->name() << endl;
								else
									tout << line << endl;
							}
							break;
						case DrBase::Integer:
							{
								DrIntegerOption	*opt = static_cast<DrIntegerOption*>(bopt);
								tout << "*Default" << keyword << ": " << opt->fixedVal() << endl;
							}
							break;
						case DrBase::Float:
							{
								DrFloatOption	*opt = static_cast<DrFloatOption*>(bopt);
								tout << "*Default" << keyword << ": " << opt->fixedVal() << endl;
							}
							break;
						default:
							tout << line << endl;
							break;
					}
				else
					tout << line << endl;
			}
			else
				tout << line << endl;
		}
	}
	delete in;
}

bool KMCupsManager::savePrinterDriver(KMPrinter *p, DrMain *d)
{
	TQString	tmpfilename = locateLocal("tmp","print_") + kapp->randomString(8);

	// first save the driver in a temporary file
	saveDriverFile(d,tmpfilename);

	// then send a request
	IppRequest	req;
	TQString		uri;
	bool		result(false);

	req.setOperation(CUPS_ADD_PRINTER);
	uri = printerURI(p, true);
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
	result = req.doFileRequest("/admin/",tmpfilename);

	// remove temporary file
	TQFile::remove(tmpfilename);

	if (!result)
		reportIppError(&req);
	return result;
}

void* KMCupsManager::loadCupsdConfFunction(const char *name)
{
	if (!m_cupsdconf)
	{
		m_cupsdconf = KLibLoader::self()->library("cupsdconf");
		if (!m_cupsdconf)
		{
			setErrorMsg(i18n("Library cupsdconf not found. Check your installation."));
			return NULL;
		}
	}
	void*	func = m_cupsdconf->symbol(name);
	if (!func)
		setErrorMsg(i18n("Symbol %1 not found in cupsdconf library.").arg(name));
	return func;
}

void KMCupsManager::unloadCupsdConf()
{
	if (m_cupsdconf)
	{
		KLibLoader::self()->unloadLibrary("libcupsdconf");
		m_cupsdconf = 0;
	}
}

bool KMCupsManager::restartServer()
{
	TQString	msg;
	bool (*f1)(TQString&) = (bool(*)(TQString&))loadCupsdConfFunction("restartServer");
	bool 	result(false);
	if (f1)
	{
		result = f1(msg);
		if (!result) setErrorMsg(msg);
	}
	unloadCupsdConf();
	return result;
}

bool KMCupsManager::configureServer(TQWidget *parent)
{
	TQString msg;
	bool (*f2)(TQWidget*, TQString&) = (bool(*)(TQWidget*, TQString&))loadCupsdConfFunction("configureServer");
	bool 	result(false);
	if (f2)
	{
		result = f2(parent, msg);
		if ( !result )
			setErrorMsg( msg );
	}
	unloadCupsdConf();
	return result;
}

TQStringList KMCupsManager::detectLocalPrinters()
{
	TQStringList	list;
	IppRequest	req;
	ipp_attribute_t	*nextAttr;
	req.setOperation(CUPS_GET_DEVICES);
	if (req.doRequest("/"))
	{
		TQString	desc, uri, printer, cl;
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
#ifdef HAVE_CUPS_1_6
			TQString	attrname(ippGetName(attr));
			if (attrname == "device-info") desc = ippGetString(attr, 0, NULL);
			else if (attrname == "device-make-and-model") printer = ippGetString(attr, 0, NULL);
			else if (attrname == "device-uri") uri = ippGetString(attr, 0, NULL);
			else if ( attrname == "device-class" ) cl = ippGetString(attr, 0, NULL);
			nextAttr = ippNextAttribute(req.request());
			if (attrname.isEmpty() || (!nextAttr))
			{
				if (!uri.isEmpty())
				{
					if (printer == "Unknown") printer = TQString::null;
					list << cl << uri << desc << printer;
				}
				uri = desc = printer = cl = TQString::null;
			}
			attr = nextAttr;
#else // HAVE_CUPS_1_6
			TQString	attrname(attr->name);
			if (attrname == "device-info") desc = attr->values[0].string.text;
			else if (attrname == "device-make-and-model") printer = attr->values[0].string.text;
			else if (attrname == "device-uri") uri = attr->values[0].string.text;
			else if ( attrname == "device-class" ) cl = attr->values[ 0 ].string.text;
			if (attrname.isEmpty() || attr == req.last())
			{
				if (!uri.isEmpty())
				{
					if (printer == "Unknown") printer = TQString::null;
					list << cl << uri << desc << printer;
				}
				uri = desc = printer = cl = TQString::null;
			}
			attr = attr->next;
#endif // HAVE_CUPS_1_6
		}
	}
	return list;
}

void KMCupsManager::createPluginActions(TDEActionCollection *coll)
{
	TDEAction	*act = new TDEAction(i18n("&Export Driver..."), "tdeprint_uploadsmb", 0, this, TQ_SLOT(exportDriver()), coll, "plugin_export_driver");
	act->setGroup("plugin");
	act = new TDEAction(i18n("&Printer IPP Report"), "tdeprint_report", 0, this, TQ_SLOT(printerIppReport()), coll, "plugin_printer_ipp_report");
	act->setGroup("plugin");
}

void KMCupsManager::validatePluginActions(TDEActionCollection *coll, KMPrinter *pr)
{
	// save selected printer for future use in slots
	m_currentprinter = pr;
	coll->action("plugin_export_driver")->setEnabled(pr && pr->isLocal() && !pr->isClass(true) && !pr->isSpecial());
	coll->action("plugin_printer_ipp_report")->setEnabled(pr && !pr->isSpecial());
}

void KMCupsManager::exportDriver()
{
	if (m_currentprinter && m_currentprinter->isLocal() &&
	    !m_currentprinter->isClass(true) && !m_currentprinter->isSpecial())
	{
		TQString	path = cupsInstallDir();
		if (path.isEmpty()) {
#if defined(__OpenBSD__) || defined(__FreeBSD__)
			path = "/usr/local/share/cups";
#else
			path = "/usr/share/cups";
#endif
		} else {
			path += "/share/cups";
		}
		CupsAddSmb::exportDest(m_currentprinter->printerName(), path);
	}
}

void KMCupsManager::printerIppReport()
{
	if (m_currentprinter && !m_currentprinter->isSpecial())
	{
		IppRequest	req;
		TQString	uri;

		req.setOperation(IPP_GET_PRINTER_ATTRIBUTES);
		uri = printerURI(m_currentprinter, true);
		req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);
		/*
		if (!m_currentprinter->uri().isEmpty())
		{
			req.setHost(m_currentprinter->uri().host());
			req.setPort(m_currentprinter->uri().port());
		}
		*/
		req.dump(2);
		if (req.doRequest("/printers/"))
		{
			ippReport(req, IPP_TAG_PRINTER, i18n("IPP Report for %1").arg(m_currentprinter->printerName()));
		}
		else
		{
			KMessageBox::error(0, "<p>"+i18n("Unable to retrieve printer information. Error received:")+"</p>"+req.statusMessage());
		}
	}
}

void KMCupsManager::ippReport(IppRequest& req, int group, const TQString& caption)
{
	IppReportDlg::report(&req, group, caption);
}

TQString KMCupsManager::stateInformation()
{
	return TQString("%1: %2")
		.arg(i18n("Server"))
		.arg(CupsInfos::self()->host()[0] != '/' ?
			TQString(TQString("%1:%2").arg(CupsInfos::self()->host()).arg(CupsInfos::self()->port()))
			: CupsInfos::self()->host());
}

void KMCupsManager::checkUpdatePossibleInternal()
{
	kdDebug(500) << "Checking for update possible" << endl;
	delete m_socket;
        m_socket = new KNetwork::TDEBufferedSocket;
	m_socket->setTimeout( 1500 );
	connect( m_socket, TQ_SIGNAL( connected(const KResolverEntry&) ), 
                TQ_SLOT( slotConnectionSuccess() ) );
	connect( m_socket, TQ_SIGNAL( gotError( int ) ), TQ_SLOT( slotConnectionFailed( int ) ) );

        trials = 5;
        TQTimer::singleShot( 1, this, TQ_SLOT( slotAsyncConnect() ) );
}

void KMCupsManager::slotConnectionSuccess()
{
	kdDebug(500) << "Connection success, trying to send a request..." << endl;
	m_socket->close();

	IppRequest req;
	req.setOperation( CUPS_GET_PRINTERS );
	req.addKeyword( IPP_TAG_OPERATION, "requested-attributes", TQString::fromLatin1( "printer-name" ) );
	if ( req.doRequest( "/printers/" ) )
		setUpdatePossible( true );
	else
	{
		kdDebug(500) << "Unable to get printer list" << endl;
		if ( trials > 0 )
		{
			trials--;
			TQTimer::singleShot( 1000, this, TQ_SLOT( slotAsyncConnect() ) );
		}
		else
		{
			setErrorMsg( i18n( "Connection to CUPS server failed. Check that the CUPS server is correctly installed and running. "
				"Error: %1." ).arg( i18n( "the IPP request failed for an unknown reason" ) ) );
			setUpdatePossible( false );
		}
	}
}

void KMCupsManager::slotAsyncConnect()
{
	kdDebug(500) << "Starting async connect to " << CupsInfos::self()->hostaddr() << endl;
	//m_socket->startAsyncConnect();
        if (CupsInfos::self()->host().startsWith("/"))
            m_socket->connect( TQString(), CupsInfos::self()->host());
        else
            m_socket->connectToHost( CupsInfos::self()->host(), CupsInfos::self()->port() );
}

void KMCupsManager::slotConnectionFailed( int errcode )
{
	kdDebug(500) << "Connection failed trials=" << trials << endl;
	if ( trials > 0 )
	{
		//m_socket->setTimeout( ++to );
		//m_socket->cancelAsyncConnect();
		trials--;
		m_socket->close();
		TQTimer::singleShot( 1000, this, TQ_SLOT( slotAsyncConnect() ) );
		return;
	}

    TQString einfo;

    switch (errcode) {
    case KNetwork::TDESocketBase::ConnectionRefused:
    case KNetwork::TDESocketBase::ConnectionTimedOut:
        einfo = i18n("connection refused") + TQString(" (%1)").arg(errcode);
        break;
    case KNetwork::TDESocketBase::LookupFailure:
        einfo = i18n("host not found") + TQString(" (%1)").arg(errcode);
        break;
    case KNetwork::TDESocketBase::WouldBlock:
    default:
        einfo = i18n("read failed (%1)").arg(errcode);
        break;
    }

    setErrorMsg( i18n( "Connection to CUPS server failed. Check that the CUPS server is correctly installed and running. "
                "Error: %2: %1." ).arg( einfo, CupsInfos::self()->host()));
    setUpdatePossible( false );
}

void KMCupsManager::hostPingSlot() {
	m_hostSuccess = true;
	m_lookupDone = true;
}

void KMCupsManager::hostPingFailedSlot() {
	m_hostSuccess = false;
	m_lookupDone = true;
}

//*****************************************************************************************************

static void extractMaticData(TQString& buf, const TQString& filename)
{
	TQFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		TQTextStream	t(&f);
		TQString		line;
		while (!t.eof())
		{
			line = t.readLine();
			if (line.startsWith("*% COMDATA #"))
				buf.append(line.right(line.length()-12)).append('\n');
		}
	}
}

static TQString printerURI(KMPrinter *p, bool use)
{
	TQString	uri;
	if (use && !p->uri().isEmpty())
		uri = p->uri().prettyURL();
	else
		uri = TQString("ipp://%1/%3/%2").arg(CupsInfos::self()->hostaddr()).arg(p->printerName()).arg((p->isClass(false) ? "classes" : "printers"));
	return uri;
}

static TQString downloadDriver(KMPrinter *p)
{
	TQString	driverfile, prname = p->printerName();
	bool	changed(false);

	/*
	if (!p->uri().isEmpty())
	{
		// try to load the driver from the host:port
		// specified in its URI. Doing so may also change
		// the printer name to use. Note that for remote
		// printer, this operation is read-only, no counterpart
		// for saving operation.
		cupsSetServer(p->uri().host().local8Bit());
		ippSetPort(p->uri().port());
		// strip any "@..." from the printer name
		prname = prname.replace(TQRegExp("@.*"), "");
		changed = true;
	}
	*/

	// download driver
	driverfile = cupsGetPPD(prname.local8Bit());

	// restore host:port (if they have changed)
	if (changed)
	{
		cupsSetServer(CupsInfos::self()->host().local8Bit());
		ippSetPort(CupsInfos::self()->port());
	}

	return driverfile;
}

#include "kmcupsmanager.moc"
