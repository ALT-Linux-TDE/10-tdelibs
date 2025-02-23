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

#include "kmvirtualmanager.h"
#include "kmprinter.h"
#include "kmfactory.h"
#include "kmmanager.h"
#include "kprinter.h"

#include <stdlib.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kdebug.h>
#include <tdemessagebox.h>

#include <unistd.h>

static TQString instanceName(const TQString& prname, const TQString& instname)
{
	TQString	str(prname);
	if (!instname.isEmpty())
		str.append("/"+instname);
	return str;
}

KMVirtualManager::KMVirtualManager(KMManager *parent, const char *name)
: TQObject(parent,name), m_manager(parent)
{
}

KMVirtualManager::~KMVirtualManager()
{
}

KMPrinter* KMVirtualManager::findPrinter(const TQString& name)
{
        return m_manager->findPrinter(name);
}

KMPrinter* KMVirtualManager::findInstance(KMPrinter *p, const TQString& name)
{
	TQString	instname(instanceName(p->printerName(),name));
	return findPrinter(instname);
}

void KMVirtualManager::addPrinter(KMPrinter *p)
{
	if (p && p->isValid())
	{
		KMPrinter	*other = findPrinter(p->name());
		if (other)
		{
			other->copy(*p);
			// Replace default options with the new loaded ones: this is needed
			// if we want to handle 2 lpoptions correctly (system-wide and local).
			// Anyway, the virtual printers will be reloaded only if something has
			// changed in one of the files, so it's better to reset everything, to
			// be sure to use the new changes. Edited options will be left unchanged.
			other->setDefaultOptions(p->defaultOptions());
			delete p;
		}
		else
                        m_manager->addPrinter(p);
	}
	else
		delete p;
}

void KMVirtualManager::setDefault(KMPrinter *p, bool saveflag)
{
        m_manager->setSoftDefault(p);
        m_defaultprinter = (p ? p->printerName() : TQString::null);
	if (saveflag) triggerSave();
}

bool KMVirtualManager::isDefault(KMPrinter *p, const TQString& name)
{
	TQString	instname(instanceName(p->printerName(),name));
	KMPrinter	*printer = findPrinter(instname);
	if (printer)
		return printer->isSoftDefault();
	else
		return false;
}

void KMVirtualManager::create(KMPrinter *p, const TQString& name)
{
	TQString	instname = instanceName(p->printerName(),name);
	if (findPrinter(instname) != NULL) return;
	KMPrinter	*printer = new KMPrinter;
	printer->setName(instname);
	printer->setPrinterName(p->printerName());
	printer->setInstanceName(name);
	if (!name.isEmpty())
		printer->setType(p->type()|KMPrinter::Virtual);
	// we need some options to know how to load the driver
	if (p->isSpecial())
		printer->setOptions(p->options());
	m_manager->addPrinter(printer);
	triggerSave();
}

void KMVirtualManager::copy(KMPrinter *p, const TQString& src, const TQString& name)
{
	TQString	instsrc(instanceName(p->printerName(),src)), instname(instanceName(p->printerName(),name));
	KMPrinter	*prsrc = findPrinter(instsrc);
	if (!prsrc || findPrinter(instname) != NULL) return;
	KMPrinter	*printer = new KMPrinter;
	printer->copy(*prsrc);
	printer->setName(instname);
	printer->setInstanceName(name);
        printer->setDefaultOptions(prsrc->defaultOptions());
        m_manager->addPrinter(printer);
	triggerSave();
}

void KMVirtualManager::remove(KMPrinter *p, const TQString& name)
{
        TQString	instname = instanceName(p->printerName(),name);
	KMPrinter	*printer = findPrinter(instname);
	if (!printer) return;
        if (name.isEmpty())
        { // remove default instance => only remove options, keep the KMPrinter object
                printer->setDefaultOptions(TQMap<TQString,TQString>());
                printer->setEditedOptions(TQMap<TQString,TQString>());
                printer->setEdited(false);
        }
        else
	        m_manager->m_printers.removeRef(printer);
	triggerSave();
}

void KMVirtualManager::setAsDefault(KMPrinter *p, const TQString& name, TQWidget *parent)
{
	TQString	instname(instanceName(p->printerName(),name));

	if ( p->isSpecial() )
	{
		if ( KMessageBox::warningContinueCancel( parent,
					i18n( "<qt>You are about to set a pseudo-printer as your personal default. "
						  "This setting is specific to TDE and will not be available outside TDE "
						  "applications. Note that this will only make your personal default printer "
						  "as undefined for non-TDE applications and should not prevent you from "
						  "printing normally. Do you really want to set <b>%1</b> as your personal default?</qt>" ).arg( instname ),
					TQString::null, i18n("Set as Default"), "setSpecialAsDefault" ) == KMessageBox::No )
			return;
	}

	KMPrinter	*printer = findPrinter(instname);
	if (!printer)
	{ // create it if necessary
		create(p,name);
		printer = findPrinter(instname);
	}
	if (printer)
		setDefault(printer,true);
}

void KMVirtualManager::refresh()
{
	TQFileInfo	fi(TQDir::homeDirPath() + TQFile::decodeName("/.cups/lpoptions"));
	TQFileInfo	fi2(TQFile::decodeName("/etc/cups/lpoptions"));

	// if root, then only use global file: trick -> use twice the same file
	if (getuid() == 0)
		fi.setFile(fi2.absFilePath());

	if (!m_checktime.isValid() || m_checktime < TQMAX(fi.lastModified(),fi2.lastModified()))
	{
                m_defaultprinter = TQString::null;
		if (fi2.exists())
			loadFile(fi2.absFilePath());
		if (fi.exists() && fi.absFilePath() != fi2.absFilePath())
                	loadFile(fi.absFilePath());
		m_checktime = TQMAX(fi.lastModified(),fi2.lastModified());
	}
        else
        { // parse printers looking for instances -> undiscarded them, real printers
          // are undiscarded by the manager itself. Also update printer status.
                TQPtrListIterator<KMPrinter>        it(m_manager->m_printers);
                for (;it.current();++it)
                        if (!it.current()->instanceName().isEmpty())
			{
				checkPrinter(it.current());
				if (it.current()->isValid()) it.current()->setDiscarded(false);
			}
        }
}

void KMVirtualManager::checkPrinter(KMPrinter *p)
{
	KMPrinter	*realprinter = m_manager->findPrinter(p->printerName());
	if (!realprinter || realprinter->isDiscarded())
	{
		p->setType(KMPrinter::Invalid);
		p->setState(KMPrinter::Unknown);
	}
	else
	{
		if (!p->instanceName().isEmpty())
			p->setType(realprinter->type()|KMPrinter::Virtual);
		p->setState(realprinter->state());
	}
}

TQString KMVirtualManager::defaultPrinterName()
{
        return m_defaultprinter;
}

void KMVirtualManager::virtualList(TQPtrList<KMPrinter>& list, const TQString& prname)
{
	// load printers if necessary
	refresh();

	// then look for instances
	list.setAutoDelete(false);
	list.clear();
	kdDebug(500) << "KMVirtualManager::virtualList() prname=" << prname << endl;
	TQPtrListIterator<KMPrinter>	it(m_manager->m_printers);
	for (;it.current();++it)
		if (it.current()->printerName() == prname)
			list.append(it.current());
}

void KMVirtualManager::loadFile(const TQString& filename)
{
	TQFile	f(filename);
	if (f.exists() && f.open(IO_ReadOnly))
	{
		TQTextStream	t(&f);

		TQString		line;
		TQStringList	words;
		TQStringList	pair;
		KMPrinter	*printer, *realprinter;

		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.isEmpty()) continue;
			words = TQStringList::split(' ',line,false);
			if (words.count() < 2) continue;
			pair = TQStringList::split('/',words[1],false);
			realprinter = m_manager->findPrinter(KURL::decode_string(pair[0]));
			if (realprinter && !realprinter->isDiscarded())
			{ // keep only instances corresponding to an existing and
			  // non discarded printer.
			  	// "clone" the real printer and modify settings as needed
				printer = new KMPrinter(*realprinter);
				printer->setName(KURL::decode_string(words[1]));
				printer->setPrinterName(KURL::decode_string(pair[0]));
				if (pair.count() > 1)
				{
					printer->setInstanceName(KURL::decode_string(pair[1]));
					printer->addType(KMPrinter::Virtual);
				}
				// parse options
				for (uint i=2; i<words.count(); i++)
				{
					pair = TQStringList::split('=',words[i],false);
					printer->setDefaultOption(pair[0],(pair.count() > 1 ? pair[1] : TQString::null));
				}
				// add printer to the manager
				addPrinter(printer);	// don't use "printer" after this point !!!
				// check default state
				if (words[0].lower().startsWith("default"))
					setDefault(findPrinter(KURL::decode_string(words[1])),false);
			}
		}
	}
}

void KMVirtualManager::triggerSave()
{
	TQString	filename;
	if (getuid() == 0)
	{
		if (TDEStandardDirs::makeDir(TQFile::decodeName("/etc/cups")))
			filename = TQFile::decodeName("/etc/cups/lpoptions");
	}
	else
	{
		TQDir cupsDir(TQDir::home().absPath()+"/.cups");
		if (!cupsDir.exists())
			cupsDir.mkdir(TQDir::home().absPath()+"/.cups");
		filename = TQDir::homeDirPath() + TQFile::decodeName("/.cups/lpoptions");
	}

	if (!filename.isEmpty())
	{
		saveFile(filename);
		m_checktime = TQFileInfo(filename).lastModified();
	}
}

void KMVirtualManager::saveFile(const TQString& filename)
{
	TQFile	f(filename);
	if (f.open(IO_WriteOnly))
	{
		TQTextStream	t(&f);
		TQPtrListIterator<KMPrinter>	it(m_manager->m_printers);
		for (;it.current();++it)
		{
			if (it.current()->isSpecial())
			{
				t << ( it.current()->isSoftDefault() ? "DefaultSpecial " : "Special " );
				t << KURL::encode_string_no_slash( it.current()->printerName() );
				if ( !it.current()->instanceName().isEmpty() )
					t << "/" << KURL::encode_string_no_slash( it.current()->instanceName() );
			}
			else
				t << (it.current()->isSoftDefault() ? "Default " : "Dest ") << it.current()->name();
			TQMap<TQString,TQString>	opts = it.current()->defaultOptions();
			for (TQMap<TQString,TQString>::ConstIterator oit=opts.begin(); oit!=opts.end(); ++oit)
			{
				t << ' ' << oit.key();
				if (!oit.data().isEmpty())
					t << '=' << oit.data();
			}
			t << endl;
		}
	}
}

bool KMVirtualManager::testInstance(KMPrinter *p)
{
	TQString	testpage = KMManager::self()->testPage();
	if (testpage.isEmpty())
		return false;
	else
	{
		KPrinter	pr;
		pr.setPrinterName(p->printerName());
		pr.setSearchName(p->name());
		pr.setOptions(p->defaultOptions());
		return (pr.printFiles(testpage));
	}
}

void KMVirtualManager::reload()
{
	reset();
}

void KMVirtualManager::configChanged()
{
	reset();
}
