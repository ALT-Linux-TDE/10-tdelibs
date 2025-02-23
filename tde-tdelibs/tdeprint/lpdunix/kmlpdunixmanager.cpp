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

#include "kmlpdunixmanager.h"
#include "kmfactory.h"
#include "kmprinter.h"

#include <tqfile.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <stdlib.h>

/*****************
 * Utility class *
 *****************/
class KTextBuffer
{
public:
	KTextBuffer(TQIODevice *dev) : m_stream(dev) {}
	bool eof() const { return (m_stream.eof() && m_linebuf.isEmpty()); }
	TQString readLine();
	void unreadLine(const TQString& l) { m_linebuf = l; }
private:
	TQTextStream	m_stream;
	TQString		m_linebuf;
};

TQString KTextBuffer::readLine()
{
	TQString	line;
	if (!m_linebuf.isEmpty())
	{
		line = m_linebuf;
		m_linebuf = TQString::null;
	}
	else
		line = m_stream.readLine();
	return line;
}

/*****************************
 * Various parsing functions *
 *****************************/

// Extract a line from a KTextBuffer:
//	'#' -> comments
//	'\' -> line continue
//	':' or '|' -> line continue (LPRng)
//
// New entry is detected by a line which have first character different from
// '#', '|', ':'. The line is then put back in the IODevice.
TQString readLine(KTextBuffer& t)
{
	TQString	line, buffer;
	bool	lineContinue(false);

	while (!t.eof())
	{
		buffer = t.readLine().stripWhiteSpace();
		if (buffer.isEmpty() || buffer[0] == '#')
			continue;
		if (buffer[0] == '|' || buffer[0] == ':' || lineContinue || line.isEmpty())
		{
			line.append(buffer);
			if (line.right(1) == "\\")
			{
				line.truncate(line.length()-1);
				line = line.stripWhiteSpace();
				lineContinue = true;
			}
			else
				lineContinue = false;
		}
		else
		{
			t.unreadLine(buffer);
			break;
		}
	}
	return line;
}

// extact an entry (printcap-like)
TQMap<TQString,TQString> readEntry(KTextBuffer& t)
{
	TQString	line = readLine(t);
	TQMap<TQString,TQString>	entry;

	if (!line.isEmpty())
	{
		TQStringList	l = TQStringList::split(':',line,false);
		if (l.count() > 0)
		{
			int 	p(-1);
			if ((p=l[0].find('|')) != -1)
				entry["printer-name"] = l[0].left(p);	// only keep first name (discard aliases
			else
				entry["printer-name"] = l[0];
			for (uint i=1; i<l.count(); i++)
				if ((p=l[i].find('=')) != -1)
					entry[l[i].left(p).stripWhiteSpace()] = l[i].right(l[i].length()-p-1).stripWhiteSpace();
				else
					entry[l[i].stripWhiteSpace()] = TQString::null;
		}
	}
	return entry;
}

// create basic printer from entry
KMPrinter* createPrinter(const TQMap<TQString,TQString>& entry)
{
	KMPrinter	*printer = new KMPrinter();
	printer->setName(entry["printer-name"]);
	printer->setPrinterName(entry["printer-name"]);
	printer->setType(KMPrinter::Printer);
	printer->setState(KMPrinter::Idle);
	return printer;
}
KMPrinter* createPrinter(const TQString& prname)
{
	TQMap<TQString,TQString>	map;
	map["printer-name"] = prname;
	return createPrinter(map);
}

// this function support LPRng piping feature, it defaults to
// /etc/printcap in any other cases (basic support)
TQString getPrintcapFileName()
{
	// check if LPRng system
	TQString	printcap("/etc/printcap");
	TQFile	f("/etc/lpd.conf");
	if (f.exists() && f.open(IO_ReadOnly))
	{
		kdDebug() << "/etc/lpd.conf found: probably LPRng system" << endl;
		TQTextStream	t(&f);
		TQString		line;
		while (!t.eof())
		{
			line = t.readLine().stripWhiteSpace();
			if (line.startsWith("printcap_path="))
			{
				kdDebug() << "printcap_path entry found: " << line << endl;
				TQString	pcentry = line.mid(14).stripWhiteSpace();
				kdDebug() << "printcap_path value: " << pcentry << endl;
				if (pcentry[0] == '|')
				{ // printcap through pipe
					printcap = locateLocal("tmp","printcap");
					TQString	cmd = TQString::fromLatin1("echo \"all\" | %1 > %2").arg(pcentry.mid(1)).arg(printcap);
					kdDebug() << "printcap obtained through pipe" << endl << "executing: " << cmd << endl;
					::system(cmd.local8Bit());
				}
				break;
			}
		}
	}
	kdDebug() << "printcap file returned: " << printcap << endl;
	return printcap;
}

// "/etc/printcap" file parsing (Linux/LPR)
void KMLpdUnixManager::parseEtcPrintcap()
{
	TQFile	f(getPrintcapFileName());
	if (f.exists() && f.open(IO_ReadOnly))
	{
		KTextBuffer	t(&f);
		TQMap<TQString,TQString>	entry;

		while (!t.eof())
		{
			entry = readEntry(t);
			if (entry.isEmpty() || !entry.contains("printer-name") || entry.contains("server"))
				continue;
			if (entry["printer-name"] == "all")
			{
				if (entry.contains("all"))
				{
					// find separator
					int	p = entry["all"].find(TQRegExp("[^a-zA-Z0-9_\\s-]"));
					if (p != -1)
					{
						TQChar	c = entry["all"][p];
						TQStringList	prs = TQStringList::split(c,entry["all"],false);
						for (TQStringList::ConstIterator it=prs.begin(); it!=prs.end(); ++it)
						{
							KMPrinter	*printer = ::createPrinter(*it);
							printer->setDescription(i18n("Description unavailable"));
							addPrinter(printer);
						}
					}
				}
			}
			else
			{
				KMPrinter	*printer = ::createPrinter(entry);
				if (entry.contains("rm"))
					printer->setDescription(i18n("Remote printer queue on %1").arg(entry["rm"]));
				else
					printer->setDescription(i18n("Local printer"));
				addPrinter(printer);
			}
		}
	}
}

// helper function for NIS support in Solaris-2.6 (use "ypcat printers.conf.byname")
TQString getEtcPrintersConfName()
{
	TQString	printersconf("/etc/printers.conf");
	if (!TQFile::exists(printersconf) && !TDEStandardDirs::findExe( "ypcat" ).isEmpty())
	{
		// standard file not found, try NIS
		printersconf = locateLocal("tmp","printers.conf");
		TQString	cmd = TQString::fromLatin1("ypcat printers.conf.byname > %1").arg(printersconf);
		kdDebug() << "printers.conf obtained from NIS server: " << cmd << endl;
		::system(TQFile::encodeName(cmd));
	}
	return printersconf;
}

// "/etc/printers.conf" file parsing (Solaris 2.6)
void KMLpdUnixManager::parseEtcPrintersConf()
{
	TQFile	f(getEtcPrintersConfName());
	if (f.exists() && f.open(IO_ReadOnly))
	{
		KTextBuffer	t(&f);
		TQMap<TQString,TQString>	entry;
		TQString		default_printer;

		while (!t.eof())
		{
			entry = readEntry(t);
			if (entry.isEmpty() || !entry.contains("printer-name"))
				continue;
			TQString	prname = entry["printer-name"];
			if (prname == "_default")
			{
				if (entry.contains("use"))
					default_printer = entry["use"];
			}
			else if (prname != "_all")
			{
				KMPrinter	*printer = ::createPrinter(entry);
				if (entry.contains("bsdaddr"))
				{
					TQStringList	l = TQStringList::split(',',entry["bsdaddr"],false);
					printer->setDescription(i18n("Remote printer queue on %1").arg(l[0]));
				}
				else
					printer->setDescription(i18n("Local printer"));
				addPrinter(printer);
			}
		}

		if (!default_printer.isEmpty())
			setSoftDefault(findPrinter(default_printer));
	}
}

// "/etc/lp/printers/" directory parsing (Solaris non-2.6)
void KMLpdUnixManager::parseEtcLpPrinters()
{
	TQDir	d("/etc/lp/printers");
	const TQFileInfoList	*prlist = d.entryInfoList(TQDir::Dirs);
	if (!prlist)
		return;

	TQFileInfoListIterator	it(*prlist);
	for (;it.current();++it)
	{
		if (it.current()->fileName() == "." || it.current()->fileName() == "..")
			continue;
		TQFile	f(it.current()->absFilePath() + "/configuration");
		if (f.exists() && f.open(IO_ReadOnly))
		{
			KTextBuffer	t(&f);
			TQString		line, remote;
			while (!t.eof())
			{
				line = readLine(t);
				if (line.isEmpty()) continue;
				if (line.startsWith("Remote:"))
				{
					TQStringList	l = TQStringList::split(':',line,false);
					if (l.count() > 1) remote = l[1];
				}
			}
			KMPrinter	*printer = new KMPrinter;
			printer->setName(it.current()->fileName());
			printer->setPrinterName(it.current()->fileName());
			printer->setType(KMPrinter::Printer);
			printer->setState(KMPrinter::Idle);
			if (!remote.isEmpty())
				printer->setDescription(i18n("Remote printer queue on %1").arg(remote));
			else
				printer->setDescription(i18n("Local printer"));
			addPrinter(printer);
		}
	}
}

// "/etc/lp/member/" directory parsing (HP-UX)
void KMLpdUnixManager::parseEtcLpMember()
{
	TQDir	d("/etc/lp/member");
	const TQFileInfoList	*prlist = d.entryInfoList(TQDir::Files);
	if (!prlist)
		return;

	TQFileInfoListIterator	it(*prlist);
	for (;it.current();++it)
	{
		KMPrinter	*printer = new KMPrinter;
		printer->setName(it.current()->fileName());
		printer->setPrinterName(it.current()->fileName());
		printer->setType(KMPrinter::Printer);
		printer->setState(KMPrinter::Idle);
		printer->setDescription(i18n("Local printer"));
		addPrinter(printer);
	}
}

// "/usr/spool/lp/interfaces/" directory parsing (IRIX 6.x)
void KMLpdUnixManager::parseSpoolInterface()
{
	TQDir	d("/usr/spool/interfaces/lp");
	const TQFileInfoList	*prlist = d.entryInfoList(TQDir::Files);
	if (!prlist)
		return;

	TQFileInfoListIterator	it(*prlist);
	for (;it.current();++it)
	{
		TQFile	f(it.current()->absFilePath());
		if (f.exists() && f.open(IO_ReadOnly))
		{
			KTextBuffer	t(&f);
			TQString		line, remote;

			while (!t.eof())
			{
				line = t.readLine().stripWhiteSpace();
				if (line.startsWith("HOSTNAME"))
				{
					TQStringList	l = TQStringList::split('=',line,false);
					if (l.count() > 1) remote = l[1];
				}
			}

			KMPrinter	*printer = new KMPrinter;
			printer->setName(it.current()->fileName());
			printer->setPrinterName(it.current()->fileName());
			printer->setType(KMPrinter::Printer);
			printer->setState(KMPrinter::Idle);
			if (!remote.isEmpty())
				printer->setDescription(i18n("Remote printer queue on %1").arg(remote));
			else
				printer->setDescription(i18n("Local printer"));
			addPrinter(printer);
		}
	}
}

//*********************************************************************************************************

KMLpdUnixManager::KMLpdUnixManager(TQObject *parent, const char *name, const TQStringList & /*args*/)
: KMManager(parent,name)
{
	m_loaded = false;
}

void KMLpdUnixManager::listPrinters()
{
	// load only once, if already loaded, just keep them (remove discard flag)
	if (!m_loaded)
	{
		parseEtcPrintcap();
		parseEtcPrintersConf();
		parseEtcLpPrinters();
		parseEtcLpMember();
		parseSpoolInterface();
		m_loaded = true;
	}
	else
		discardAllPrinters(false);
}
