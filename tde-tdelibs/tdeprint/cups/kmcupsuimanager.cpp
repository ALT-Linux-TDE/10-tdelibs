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

#include "kmcupsuimanager.h"
#include "kmpropertypage.h"
#include "kmwizard.h"
#include "kmconfigdialog.h"
#include "kmwbackend.h"
#include "kmfactory.h"
#include "kprinter.h"

#include "kmpropbanners.h"
#include "kmpropmembers.h"
#include "kmpropbackend.h"
#include "kmpropdriver.h"
#include "kmwbanners.h"
#include "kmwipp.h"
#include "kmwippselect.h"
#include "kmwippprinter.h"
#include "kmconfigcups.h"
#include "kmconfigcupsdir.h"
#include "kmwfax.h"
#include "kmwother.h"
#include "kmwquota.h"
#include "kmpropquota.h"
#include "kmwusers.h"
#include "kmpropusers.h"
#include "kpschedulepage.h"
#include "kptagspage.h"

#include "kprinterpropertydialog.h"
#include "kpgeneralpage.h"
#include "kpimagepage.h"
#include "kptextpage.h"
#include "kphpgl2page.h"

#include "ipprequest.h"
#include "cupsinfos.h"

#include <tqlistview.h>
#include <tqwhatsthis.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <tdeaction.h>
#include <tdemessagebox.h>

#include "config.h"

KMCupsUiManager::KMCupsUiManager(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KMUiManager(parent,name)
{
}

KMCupsUiManager::~KMCupsUiManager()
{
}

void KMCupsUiManager::setupPropertyPages(KMPropertyPage *p)
{
	p->addPropPage(new KMPropMembers(p, "Members"));
	p->addPropPage(new KMPropBackend(p, "Backend"));
	p->addPropPage(new KMPropDriver(p, "Driver"));
	p->addPropPage(new KMPropBanners(p, "Banners"));
	p->addPropPage(new KMPropQuota(p, "Quotas"));
	p->addPropPage(new KMPropUsers(p, "Users"));
}

void KMCupsUiManager::setupWizard(KMWizard *wizard)
{
	TQString whatsThisRemoteCUPSIPPBackend =
		i18n( "<qt><p>Print queue on remote CUPS server</p>"
			  "<p>Use this for a print queue installed on a remote "
			  "machine running a CUPS server. This allows to use "
			  "remote printers when CUPS browsing is turned off.</p></qt>"
			);

	TQString whatsThisRemotePrinterIPPBackend =
		i18n( "<qt><p>Network IPP printer</p>"
			  "<p>Use this for a network-enabled printer using the "
			  "IPP protocol. Modern high-end printers can use this mode. "
			  "Use this mode instead of TCP if your printer can do both.</p></qt>"
			);

	TQString whatsThisSerialFaxModemBackend =
		i18n( "<qt><p>Fax/Modem printer</p>"
			  "<p>Use this for a fax/modem printer. This requires the installation "
			  "of the <a href=\"http://vigna.dsi.unimi.it/fax4CUPS/\">fax4CUPS</a> backend. Documents sent on this printer will be faxed "
			  "to the given target fax number.</p></qt>"
			);

	TQString whatsThisOtherPrintertypeBackend =
		i18n( "<qt><p>Other printer</p>"
			  "<p>Use this for any printer type. To use this option, you must know "
			  "the URI of the printer you want to install. Refer to the CUPS documentation "
			  "for more information about the printer URI. This option is mainly useful for "
			  "printer types using 3rd party backends not covered by the other possibilities.</p></qt>"
			);

	TQString whatsThisClassOfPrinters =
		i18n( "<qt><p>Class of printers</p>"
			  "<p>Use this to create a class of printers. When sending a document to a class, "
			  "the document is actually sent to the first available (idle) printer in the class. "
			  "Refer to the CUPS documentation for more information about class of printers.</p></qt>"
			);

	KMWBackend	*backend = wizard->backendPage();
	if (!backend)
		return;
	backend->addBackend(KMWizard::Local,false);
	backend->addBackend(KMWizard::LPD,false);
	backend->addBackend(KMWizard::SMB,false,KMWizard::Password);
	backend->addBackend(KMWizard::TCP,false);
	backend->addBackend(KMWizard::IPP,i18n("Re&mote CUPS server (IPP/HTTP)"),false,whatsThisRemoteCUPSIPPBackend,KMWizard::Password);
	backend->addBackend(KMWizard::Custom+1,i18n("Network printer w/&IPP (IPP/HTTP)"),false,whatsThisRemotePrinterIPPBackend);
	backend->addBackend(KMWizard::Custom+2,i18n("S&erial Fax/Modem printer"),false,whatsThisSerialFaxModemBackend);
	backend->addBackend(KMWizard::Custom+5,i18n("Other &printer type"),false,whatsThisOtherPrintertypeBackend);
	backend->addBackend();
	backend->addBackend(KMWizard::Class,i18n("Cl&ass of printers"),false,whatsThisClassOfPrinters);

	IppRequest	req;
	TQString		uri;

	req.setOperation(CUPS_GET_DEVICES);
	uri = TQString::fromLocal8Bit("ipp://%1/printers/").arg(CupsInfos::self()->hostaddr());
	req.addURI(IPP_TAG_OPERATION,"printer-uri",uri);

	if (req.doRequest("/"))
	{
		ipp_attribute_t	*attr = req.first();
		while (attr)
		{
#ifdef HAVE_CUPS_1_6
			if (ippGetName(attr) && strcmp(ippGetName(attr),"device-uri") == 0)
			{
				if (strncmp(ippGetString(attr, 0, NULL),"socket",6) == 0) backend->enableBackend(KMWizard::TCP,true);
				else if (strncmp(ippGetString(attr, 0, NULL),"parallel",8) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(ippGetString(attr, 0, NULL),"serial",6) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(ippGetString(attr, 0, NULL),"smb",3) == 0) backend->enableBackend(KMWizard::SMB,true);
				else if (strncmp(ippGetString(attr, 0, NULL),"lpd",3) == 0) backend->enableBackend(KMWizard::LPD,true);
				else if (strncmp(ippGetString(attr, 0, NULL),"usb",3) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(ippGetString(attr, 0, NULL),"http",4) == 0 || strncmp(ippGetString(attr, 0, NULL),"ipp",3) == 0)
				{
					backend->enableBackend(KMWizard::IPP,true);
					backend->enableBackend(KMWizard::Custom+1,true);
				}
				else if (strncmp(ippGetString(attr, 0, NULL),"fax",3) == 0) backend->enableBackend(KMWizard::Custom+2,true);
			}
			attr = ippNextAttribute(req.request());
#else // HAVE_CUPS_1_6
			if (attr->name && strcmp(attr->name,"device-uri") == 0)
			{
				if (strncmp(attr->values[0].string.text,"socket",6) == 0) backend->enableBackend(KMWizard::TCP,true);
				else if (strncmp(attr->values[0].string.text,"parallel",8) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"serial",6) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"smb",3) == 0) backend->enableBackend(KMWizard::SMB,true);
				else if (strncmp(attr->values[0].string.text,"lpd",3) == 0) backend->enableBackend(KMWizard::LPD,true);
				else if (strncmp(attr->values[0].string.text,"usb",3) == 0) backend->enableBackend(KMWizard::Local,true);
				else if (strncmp(attr->values[0].string.text,"http",4) == 0 || strncmp(attr->values[0].string.text,"ipp",3) == 0)
				{
					backend->enableBackend(KMWizard::IPP,true);
					backend->enableBackend(KMWizard::Custom+1,true);
				}
				else if (strncmp(attr->values[0].string.text,"fax",3) == 0) backend->enableBackend(KMWizard::Custom+2,true);
			}
			attr = attr->next;
#endif // HAVE_CUPS_1_6
		}
		backend->enableBackend(KMWizard::Class, true);
		backend->enableBackend(KMWizard::Custom+5, true);
	}
	else
		KMessageBox::error(wizard,
			"<qt><nobr>" +
			i18n("An error occurred while retrieving the list of available backends:") +
			"</nobr><br><br>" + req.statusMessage() + "</qt>");

	// banners page
	wizard->addPage(new KMWBanners(wizard));
	wizard->setNextPage(KMWizard::DriverTest,KMWizard::Banners);
	wizard->addPage(new KMWIpp(wizard));
	wizard->addPage(new KMWIppSelect(wizard));
	wizard->addPage(new KMWIppPrinter(wizard));
	wizard->addPage(new KMWFax(wizard));
	wizard->addPage(new KMWQuota(wizard));
	wizard->addPage(new KMWUsers(wizard));
	wizard->addPage(new KMWOther(wizard));
}

void KMCupsUiManager::setupPrinterPropertyDialog(KPrinterPropertyDialog *dlg)
{
	// add general page
	dlg->addPage(new KPGeneralPage(dlg->printer(),dlg->driver(),dlg,"GeneralPage"));
	if (KMFactory::self()->settings()->application != KPrinter::Dialog)
	{
		dlg->addPage(new KPImagePage(dlg->driver(), dlg, "ImagePage"));
		dlg->addPage(new KPTextPage(dlg->driver(), dlg, "TextPage"));
		dlg->addPage(new KPHpgl2Page(dlg, "Hpgl2Page"));
	}
}

void KMCupsUiManager::setupConfigDialog(KMConfigDialog *dlg)
{
	dlg->addConfigPage(new KMConfigCups(dlg));
	dlg->addConfigPage(new KMConfigCupsDir(dlg));
}

int KMCupsUiManager::pluginPageCap()
{
	return (KMUiManager::CopyAll & ~KMUiManager::Current);
}

void KMCupsUiManager::setupPrintDialogPages(TQPtrList<KPrintDialogPage>* pages)
{
	pages->append(new KPSchedulePage());
	pages->append(new KPTagsPage());
}

void KMCupsUiManager::setupJobViewer(TQListView *lv)
{
	lv->addColumn(i18n("Priority"));
	lv->setColumnAlignment(lv->columns()-1, TQt::AlignRight|TQt::AlignVCenter);
	lv->addColumn(i18n("Billing Information"));
	lv->setColumnAlignment(lv->columns()-1, TQt::AlignRight|TQt::AlignVCenter);
}
