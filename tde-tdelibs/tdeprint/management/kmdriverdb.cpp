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

#include "kmdriverdb.h"
#include "kmdbentry.h"
#include "kmdbcreator.h"
#include "kmmanager.h"
#include "kmfactory.h"
#include <kdebug.h>

#include <tqfile.h>
#include <tqtextstream.h>
#include <tqfileinfo.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>
#include <tdemessagebox.h>

KMDriverDB* KMDriverDB::m_self = 0;

KMDriverDB* KMDriverDB::self()
{
	if (!m_self)
	{
		m_self = new KMDriverDB();
		TQ_CHECK_PTR(m_self);
	}
	return m_self;
}

KMDriverDB::KMDriverDB(TQObject *parent, const char *name)
: TQObject(parent,name)
{
	m_creator = new KMDBCreator(this,"db-creator");
	connect(m_creator,TQ_SIGNAL(dbCreated()),TQ_SLOT(slotDbCreated()));

	m_entries.setAutoDelete(true);
	m_pnpentries.setAutoDelete(true);
}

KMDriverDB::~KMDriverDB()
{
}

TQString KMDriverDB::dbFile()
{
	// this calls insure missing directories creation
	TQString	filename = locateLocal("data",TQString::fromLatin1("tdeprint/printerdb_%1.txt").arg(KMFactory::self()->printSystem()));
	return filename;
}

void KMDriverDB::init(TQWidget *parent)
{
	TQFileInfo	dbfi(dbFile());
	TQString		dirname = KMFactory::self()->manager()->driverDirectory();
	TQStringList	dbDirs = TQStringList::split(':', dirname, false);
	bool	createflag(false);

	for (TQStringList::ConstIterator it=dbDirs.begin(); it!=dbDirs.end() && !createflag; ++it)
		if (!(*it).startsWith("module:") && !m_creator->checkDriverDB(*it, dbfi.lastModified()))
			createflag = true;

	if (createflag)
	{
		// starts DB creation and wait for creator signal
		if (!m_creator->createDriverDB(dirname,dbfi.absFilePath(),parent))
			KMessageBox::error(parent, KMFactory::self()->manager()->errorMsg().prepend("<qt>").append("</qt>"));
	}
	else if (m_entries.count() == 0)
	{
		// call directly the slot as the DB won't be re-created
		// this will (re)load the driver DB
		slotDbCreated();
	}
	else
		// no need to refresh, and already loaded, just emit signal
		emit dbLoaded(false);
}

void KMDriverDB::slotDbCreated()
{
	// DB should be created, check creator status
	if (m_creator->status())
	{
		// OK, load DB and emit signal
		loadDbFile();
		emit dbLoaded(true);
	}
	else
		// error while creating DB, notify the DB widget
		emit error(KMManager::self()->errorMsg());
	// be sure to emit this signal to notify the DB widget
	//emit dbLoaded(true);
}

KMDBEntryList* KMDriverDB::findEntry(const TQString& manu, const TQString& model)
{
	TQDict<KMDBEntryList>	*models = m_entries.find(manu);
	if (models)
		return models->find(model);
	return 0;
}

KMDBEntryList* KMDriverDB::findPnpEntry(const TQString& manu, const TQString& model)
{
	TQDict<KMDBEntryList>	*models = m_pnpentries.find(manu);
	if (models)
		return models->find(model);
	return 0;
}

TQDict<KMDBEntryList>* KMDriverDB::findModels(const TQString& manu)
{
	return m_entries.find(manu);
}

void KMDriverDB::insertEntry(KMDBEntry *entry)
{
	// first check entry
	if (!entry->validate())
	{
		kdDebug() << "Incorrect entry, skipping...(" << entry->file << ")" << endl;
		delete entry;
		return;
	}

	// insert it in normal entries
	TQDict<KMDBEntryList>	*models = m_entries.find(entry->manufacturer);
	if (!models)
	{
		models = new TQDict<KMDBEntryList>(17,false);
		models->setAutoDelete(true);
		m_entries.insert(entry->manufacturer,models);
	}
	KMDBEntryList	*list = models->find(entry->model);
	if (!list)
	{
		list = new KMDBEntryList;
		list->setAutoDelete(true);
		models->insert(entry->model,list);
	}
	list->append(entry);

	if (!entry->pnpmanufacturer.isEmpty() && !entry->pnpmodel.isEmpty())
	{
		// insert it in PNP entries
		models = m_pnpentries.find(entry->manufacturer);
		if (!models)
		{
			models = new TQDict<KMDBEntryList>(17,false);
			models->setAutoDelete(true);
			m_pnpentries.insert(entry->manufacturer,models);
		}
		list = models->find(entry->model);
		if (!list)
		{
			list = new KMDBEntryList;
			list->setAutoDelete(true);
			models->insert(entry->model,list);
		}
		list->append(entry);
	}

	// don't block GUI
	kapp->processEvents();
}

/*
  Driver DB file format:
	FILE=<path>
	MANUFACTURER=<string>
	MODEL=<string>
	PNPMANUFACTURER=<string>
	PNPMODEL=<string>
	DESCRIPTION=<string>
*/

void KMDriverDB::loadDbFile()
{
	// first clear everything
	m_entries.clear();
	m_pnpentries.clear();

	TQFile	f(dbFile());
	if (f.exists() && f.open(IO_ReadOnly))
	{
		TQTextStream	t(&f);
		TQString		line;
		TQStringList	words;
		KMDBEntry	*entry(0);

		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.isEmpty())
				continue;
			int	p = line.find('=');
			if (p == -1)
				continue;
			words.clear();
			words << line.left(p) << line.mid(p+1);
			if (words[0] == "FILE")
			{
				if (entry) insertEntry(entry);
				entry = new KMDBEntry;
				entry->file = words[1];
			}
			else if (words[0] == "MANUFACTURER" && entry)
				entry->manufacturer = words[1].upper();
			else if (words[0] == "MODEL" && entry)
				entry->model = words[1];
			else if (words[0] == "MODELNAME" && entry)
				entry->modelname = words[1];
			else if (words[0] == "PNPMANUFACTURER" && entry)
				entry->pnpmanufacturer = words[1].upper();
			else if (words[0] == "PNPMODEL" && entry)
				entry->pnpmodel = words[1];
			else if (words[0] == "DESCRIPTION" && entry)
				entry->description = words[1];
			else if (words[0] == "RECOMMANDED" && entry && words[1].lower() == "yes")
				entry->recommended = true;
			else if (words[0] == "DRIVERCOMMENT" && entry)
				entry->drivercomment = ("<qt>"+words[1].replace("&lt;", "<").replace("&gt;", ">")+"</qt>");
		}
		if (entry)
			insertEntry(entry);
	}
}
#include "kmdriverdb.moc"
