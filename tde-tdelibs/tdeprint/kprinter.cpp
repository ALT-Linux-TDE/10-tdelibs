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

#include "kprinter.h"
#include "kprinterimpl.h"
#include "kprintdialog.h"
#include "kprintpreview.h"
#include "kmfactory.h"
#include "kmuimanager.h"
#include "kmmanager.h"
#include "driver.h"

#include <tqpaintdevicemetrics.h>
#include <tqfile.h>
#include <tqtl.h>
#include <tqdir.h>
#include <tqguardedptr.h>
#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <krun.h>
#include <knotifyclient.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <kprocess.h>
#include <klibloader.h>
#include <tdemessagebox.h>

static void dumpOptions(const TQMap<TQString,TQString>& opts);
static void reportError(KPrinter*);

//**************************************************************************************
// KPrinterWrapper class
//**************************************************************************************

class KPrinterWrapper : public TQPrinter
{
friend class KPrinter;
public:
	KPrinterWrapper(KPrinter*, PrinterMode m = ScreenResolution);
	~KPrinterWrapper();
protected:
	virtual bool cmd(int, TQPainter*, TQPDevCmdParam*);
	virtual int metric(int) const;
	int qprinterMetric(int) const;
private:
	KPrinter	*m_printer;
};

KPrinterWrapper::KPrinterWrapper(KPrinter *prt, TQPrinter::PrinterMode m)
: TQPrinter(m), m_printer(prt)
{
}

KPrinterWrapper::~KPrinterWrapper()
{
}

bool KPrinterWrapper::cmd(int c, TQPainter *painter, TQPDevCmdParam *p)
{
	return TQPrinter::cmd(c,painter,p);
}

int KPrinterWrapper::metric(int m) const
{
	return m_printer->metric(m);
}

int KPrinterWrapper::qprinterMetric(int m) const
{
	return TQPrinter::metric(m);
}

//**************************************************************************************
// KPrinterPrivate class
//**************************************************************************************

class KPrinterPrivate
{
public:
	TQGuardedPtr<KPrinterImpl>	m_impl;
	bool		m_restore;
	bool		m_previewonly;
	WId		m_parentId;
	TQString		m_docfilename;
	TQString m_docdirectory;
	KPrinterWrapper		*m_wrapper;
	TQMap<TQString,TQString>	m_options;
	TQString			m_tmpbuffer;
	TQString			m_printername;
	TQString			m_searchname;
	TQString			m_errormsg;
	bool			m_ready;
	int		m_pagenumber;
	DrPageSize *m_pagesize;
	bool m_useprinterres;
	int m_defaultres;
};

//**************************************************************************************
// KPrinter class
//**************************************************************************************

KPrinter::KPrinter(bool restore, TQPrinter::PrinterMode m)
: TQPaintDevice(TQInternal::Printer|TQInternal::ExternalDevice)
{
	init(restore, m);
}

KPrinter::~KPrinter()
{
	// delete Wrapper object
	delete d->m_wrapper;

	// save current options
	if (d->m_restore)
		saveSettings();

	// delete private data (along any data allocated internally)
	delete d->m_pagesize;
	delete d;
}

void KPrinter::init(bool restore, TQPrinter::PrinterMode m)
{
	// Private data initialization
	d = new KPrinterPrivate;
	d->m_impl = KMFactory::self()->printerImplementation();
	d->m_restore = restore;
	d->m_previewonly = false;
	d->m_parentId = 0;
	d->m_pagesize = 0;

	// initialize TQPrinter wrapper
	d->m_wrapper = new KPrinterWrapper(this, m);

	// other initialization
	d->m_tmpbuffer = d->m_impl->tempFile();
	d->m_ready = false;
	d->m_defaultres = d->m_wrapper->resolution();
	d->m_useprinterres = false;

	// reload options from implementation (static object)
	if (d->m_restore)
		loadSettings();
}

void KPrinter::loadSettings()
{
	d->m_options = d->m_impl->loadOptions();

	// load the last printer used in the current process (if any)
	// and remove the corresponding entry in the option map, as it
	// is not needed anymore
	setSearchName(option("kde-searchname"));
	d->m_options.remove("kde-searchname");

	TDEConfig	*conf = TDEGlobal::config(), *pconf = KMFactory::self()->printConfig();
	conf->setGroup("KPrinter Settings");
	pconf->setGroup("General");

	// load latest used printer from config file, if required in the options
	if (searchName().isEmpty() && pconf->readBoolEntry("UseLast", true))
		setSearchName(conf->readEntry("Printer"));

	// latest used print command
	setOption("kde-printcommand",conf->readPathEntry("PrintCommand"));

	// latest used document directory
	setDocDirectory( conf->readPathEntry( "DocDirectory" ) );
	setDocFileName( "print" );
}

void KPrinter::saveSettings()
{
	if (d->m_impl)
	{
		setOption("kde-searchname", searchName());
		d->m_impl->saveOptions(d->m_options);
	}

	// save latest used printer to config file
	TDEConfig	*conf = TDEGlobal::config();
	conf->setGroup("KPrinter Settings");
	conf->writeEntry("Printer",searchName());
	// latest used print command
	conf->writePathEntry("PrintCommand",option("kde-printcommand"));

	// latest used document directory
	if ( d->m_docdirectory.isEmpty() )
	{
		KURL url( outputFileName() );
		if ( url.isValid() )
			conf->writePathEntry( "DocDirectory", url.directory() );
	}
	else
		conf->writePathEntry( "DocDirectory", d->m_docdirectory );
}

bool KPrinter::setup(TQWidget *parent, const TQString& caption, bool forceExpand)
{
	if (!kapp->authorize("print/dialog"))
	{
		autoConfigure(TQString::null, parent);
		return true; // Just print it
	}

	if (parent)
		d->m_parentId = parent->winId();

	KPrintDialog	*dlg = KPrintDialog::printerDialog(this, parent, caption, forceExpand);
	bool	state = false;
	if (dlg)
	{
		state = dlg->exec();
		delete dlg;
	}
	return state;
}

void KPrinter::addStandardPage(int p)
{
	KMFactory::self()->settings()->standardDialogPages |= p;
}

void KPrinter::removeStandardPage(int p)
{
	KMFactory::self()->settings()->standardDialogPages &= (~p);
}

void KPrinter::addDialogPage(KPrintDialogPage *page)
{
	KMFactory::self()->uiManager()->addPrintDialogPage(page);
}

void KPrinter::setPageSelection(PageSelectionType t)
{
	KMFactory::self()->settings()->pageSelection = t;
}

KPrinter::PageSelectionType KPrinter::pageSelection()
{
	return (PageSelectionType)KMFactory::self()->settings()->pageSelection;
}

void KPrinter::setApplicationType(ApplicationType t)
{
	KMFactory::self()->settings()->application = t;
}

KPrinter::ApplicationType KPrinter::applicationType()
{
	return (ApplicationType)KMFactory::self()->settings()->application;
}

bool KPrinter::cmd(int c, TQPainter *painter, TQPDevCmdParam *p)
{
	bool value(true);
	if (c == TQPaintDevice::PdcBegin)
	{
		d->m_impl->statusMessage(i18n("Initialization..."), this);
		d->m_pagenumber = 1;
		preparePrinting();
		d->m_impl->statusMessage(i18n("Generating print data: page %1").arg(d->m_pagenumber), this);
	}
	value = d->m_wrapper->cmd(c,painter,p);
	if (c == TQPaintDevice::PdcEnd)
	{
		// this call should take care of everything (preview, output-to-file, filtering, ...)
		value = value && printFiles(TQStringList(d->m_wrapper->outputFileName()),true);
		// reset "ready" state
		finishPrinting();
	}
	return value;
}

void KPrinter::translateQtOptions()
{
	d->m_wrapper->setCreator(creator());
	d->m_wrapper->setDocName(docName());
	d->m_wrapper->setFullPage(fullPage());
	d->m_wrapper->setColorMode((TQPrinter::ColorMode)colorMode());
	d->m_wrapper->setOrientation((TQPrinter::Orientation)orientation());
	if ( !option( "kde-printsize" ).isEmpty() )
		d->m_wrapper->setPageSize( ( TQPrinter::PageSize )option( "kde-printsize" ).toInt() );
	else
		d->m_wrapper->setPageSize((TQPrinter::PageSize)pageSize());
	d->m_wrapper->setOutputToFile(true);
	d->m_wrapper->setOutputFileName(d->m_tmpbuffer);
	d->m_wrapper->setNumCopies(option("kde-qtcopies").isEmpty() ? 1 : option("kde-qtcopies").toInt());
	if (!option("kde-margin-top").isEmpty())
	{
		/**
		 * Scale margings as the margin widget always stores values
		 * in dot units ( 1/72th in ), to be resolution independent
		 * when specified by the user ( who usually specifies margins
		 * in metric units ).
		 */
		int res = resolution();
		d->m_wrapper->setMargins(
				( int )( ( option("kde-margin-top").toFloat() * res + 71 ) / 72 ),
				( int )( ( option("kde-margin-left").toFloat() * res + 71 ) / 72 ),
				( int )( ( option("kde-margin-bottom").toFloat() * res + 71 ) / 72 ),
				( int )( ( option("kde-margin-right").toFloat() * res + 71 ) / 72 ) );
	}
	else if ( d->m_pagesize != NULL )
	{
		int res = resolution();
		DrPageSize *ps = d->m_pagesize;
		int top = ( int )( ps->topMargin() * res + 71 ) / 72;
		int left = ( int )( ps->leftMargin() * res + 71 ) / 72;
		int bottom = ( int )( ps->bottomMargin() * res + 71 ) / 72;
		int right = ( int )( ps->rightMargin() * res + 71 ) / 72;
		if ( !fullPage() )
		{
			// Printers can often print very close to the edges (PPD files say ImageArea==PaperDimension).
			// But that doesn't mean it looks good. Apps which use setFullPage(false) assume that
			// KPrinter will give them reasonable margins, so let's TQMAX with defaults from Qt in that case.
			// Keep this in sync with KPMarginPage::initPageSize
			unsigned int it, il, ib, ir;
			d->m_wrapper->margins( &it, &il, &ib, &ir );
			top = TQMAX( top, (int)it );
			left = TQMAX( left, (int)il );
			bottom = TQMAX( bottom, (int)ib );
			right = TQMAX( right, (int)ir );
		}
		d->m_wrapper->setMargins( top, left, bottom, right );
	}
	/*else
	{
		int res = d->m_wrapper->resolution();
		d->m_wrapper->setMargins( res/3, res/2, res/3, res/2 );
	}*/
	// for special printers, copies are handled by Qt
	if (option("kde-isspecial") == "1")
		d->m_wrapper->setNumCopies(numCopies());
}

bool KPrinter::printFiles(const TQStringList& l, bool flag, bool startviewer)
{
	TQStringList	files(l);
	bool		status(true);

	// First apply possible filters, and update "remove" flag if filters has
	// been applied (result == 0, means nothing happened).
	int	fresult = d->m_impl->filterFiles(this, files, flag);
	if (fresult == -1)
	{
		reportError(this);
		status = false;
	}
	else if (fresult == 1)
		flag = true;

	if (status)
	{
		// Automatic conversion to format supported by print system
		fresult = d->m_impl->autoConvertFiles(this, files, flag);
		if (fresult == -1)
		{
			reportError(this);
			status = false;
		}
		else if (fresult == 1)
			flag = true;
	}

	// Continue if status is OK (filtering succeeded) and no output-to-file
	if (status && files.count() > 0)
	{
		// Show preview if needed (only possible for a single file !), and stop
		// if the user requested it. Force preview if preview-only mode has been set: it
		// then use by default the first file in the list.
		if (((files.count() != 1 || option("kde-preview") != "1") && !d->m_previewonly) || doPreview(files[0]))
		{
			// check if printing has been prepared (it may be not prepared if the KPrinter object is not
			// use as a TQPaintDevice object)
			preparePrinting();

			if (!d->m_impl->printFiles(this, files, flag))
			{
				reportError(this);
				status = false;
			}
			else
			{
				if (/* !outputToFile() && */ startviewer && !TQFile::exists("/etc/xdg/autostart/system-config-printer-applet-kde.desktop") )
				{
					TQStringList args;
					args << "-d";
					args << printerName();
					args << "--noshow";
					kapp->tdeinitExec("kjobviewer", args);
				}
			}
		}
		else if (flag)
		// situation: only one file, it has been previewed and printing has been canceled, then
		//            we should remove the file ourself
		{
			TQFile::remove(files[0]);
		}
	}
	finishPrinting();
	return status;
}

bool KPrinter::doPreview(const TQString& file)
{
	d->m_impl->statusMessage(i18n("Previewing..."), this);
	d->m_impl->statusMessage(TQString::null, this);
	return KPrintPreview::preview(file, d->m_previewonly, d->m_parentId);
}

void KPrinter::preparePrinting()
{
	// check if already prepared (-> do nothing)
	if (d->m_ready) return;

	// re-initialize error
	setErrorMessage(TQString::null);

	// re-initialize margins and page size (by default, use Qt mechanism)
	setRealPageSize(NULL);

	// print-system-specific setup, only if not printing to file
	if (option("kde-isspecial") != "1")
		d->m_impl->preparePrinting(this);

	// set the correct resolution, if needed (or reset it)
	int res = option( "kde-resolution" ).toInt();
	if ( d->m_useprinterres && res > 0 )
		d->m_wrapper->setResolution( res );
	else
		d->m_wrapper->setResolution( d->m_defaultres );

	// standard Qt settings
	translateQtOptions();

	d->m_ready = true;
dumpOptions(d->m_options);
}

void KPrinter::finishPrinting()
{
	d->m_ready = false;
	// close the status window
	d->m_impl->statusMessage(TQString::null, this);
}

TQValueList<int> KPrinter::pageList() const
{
	TQValueList<int>	list;
	int	mp(minPage()), MP(maxPage());
	if (mp > 0 && MP > 0 && MP >= mp)
	{ // do something only if bounds specified
		if (option("kde-current") == "1")
		{ // print only current page
			int	pp = currentPage();
			if (pp >= mp && pp <= MP) list.append(pp);
		}
		else
		{
			// process range specification
			if (!option("kde-range").isEmpty())
			{
				TQStringList	ranges = TQStringList::split(',',option("kde-range"),false);
				for (TQStringList::ConstIterator it=ranges.begin();it!=ranges.end();++it)
				{
					int	p = (*it).find('-');
					bool	ok;
					if (p == -1)
					{
						int	pp = (*it).toInt(&ok);
						if (ok && pp >= mp && pp <= MP)
							list.append(pp);
					}
					else
					{
						int	p1(0), p2(0);
						p1 = (*it).left(p).toInt(&ok);
						if (ok) p2 = (*it).right((*it).length()-p-1).toInt(&ok);
						if (ok && p1 <= p2)
						{
							// clip to min/max
							p1 = TQMAX(mp,p1);
							p2 = TQMIN(MP,p2);
							for (int i=p1;i<=p2;i++)
								list.append(i);
						}
					}
				}
			}
			else
			{ // add all pages between min and max
				for (int i=mp;i<=MP;i++) list.append(i);
			}

			// revert the list if needed
			if (pageOrder() == LastPageFirst)
			{
				for (uint i=0;i<(list.count()/2);i++)
					tqSwap(list[i],list[list.count()-1-i]);
			}

			// select page set if needed
			if (pageSet() != AllPages)
			{
				bool	keepEven = (pageSet() == EvenPages);
				for (TQValueList<int>::Iterator it=list.begin();it!=list.end();)
					if ((((*it) % 2) != 0 && keepEven) ||
					    (((*it) % 2) == 0 && !keepEven)) it = list.remove(it);
					else ++it;
			}
		}
	}
	return list;
}

//**************************************************************************************
// TQPrinter interface
//**************************************************************************************

int KPrinter::numCopies() const
{
	bool	ok;
	int	p = option("kde-copies").toInt(&ok);
	return (ok ? p : 1);
}

TQSize KPrinter::margins() const
{
	return d->m_wrapper->margins();
}

void KPrinter::margins( uint *top, uint *left, uint *bottom, uint *right ) const
{
	d->m_wrapper->margins( top, left, bottom, right );
}

int KPrinter::metric(int m) const
{
	if (d->m_pagesize == NULL || !option( "kde-printsize" ).isEmpty())
		return d->m_wrapper->qprinterMetric(m);

	int	val(0);
	bool	land = (orientation() == KPrinter::Landscape);
	uint	res(d->m_wrapper->resolution()), top = res/2, left = res/2, bottom = res/3, right = res/2;
	margins( &top, &left, &bottom, &right );
	switch ( m )
	{
		case TQPaintDeviceMetrics::PdmWidth:
			val = (land ? ( int )d->m_pagesize->pageHeight() : ( int )d->m_pagesize->pageWidth());
			if ( res != 72 )
				val = (val * res + 36) / 72;
			if ( !fullPage() )
				val -= ( left + right );
			break;
		case TQPaintDeviceMetrics::PdmHeight:
			val = (land ? ( int )d->m_pagesize->pageWidth() : ( int )d->m_pagesize->pageHeight());
			if ( res != 72 )
				val = (val * res + 36) / 72;
			if ( !fullPage() )
				val -= ( top + bottom );
			break;
		case TQPaintDeviceMetrics::PdmWidthMM:
			val = metric( TQPaintDeviceMetrics::PdmWidth );
			val = (val * 254 + 5*res) / (10*res); // +360 to get the right rounding
			break;
		case TQPaintDeviceMetrics::PdmHeightMM:
			val = metric( TQPaintDeviceMetrics::PdmHeight );
			val = (val * 254 + 5*res) / (10*res);
			break;
		default:
			val = d->m_wrapper->qprinterMetric(m);
			break;
	}
	return val;
}

void KPrinter::setOrientation(Orientation o, bool locking)
{
	KMFactory::self()->settings()->orientation = o;
	setOption("kde-orientation",(o == Landscape ? "Landscape" : "Portrait"));
	d->m_impl->broadcastOption("kde-orientation",(o == Landscape ? "Landscape" : "Portrait"));
	if (locking)
	{
		d->m_impl->broadcastOption( "kde-orientation-fixed", "1" );
	}
}

void KPrinter::setOption( const TQString& key, const TQString& value, bool broadcast )
{
	setOption( key, value );
	if ( broadcast )
		d->m_impl->broadcastOption( key, value );
}

void KPrinter::setPageSize(PageSize s, bool locking)
{
	KMFactory::self()->settings()->pageSize = s;
	setOption("kde-pagesize",TQString::number((int)s),true);
	if (locking)
	{
		d->m_impl->broadcastOption( "kde-pagesize-fixed", "1" );
	}
}

void KPrinter::setOptions(const TQMap<TQString,TQString>& opts)
{ // This functions remove all options except those with "kde-..."
  // which correspond to externally-sets options (use the value
  // from "opts" if specified
	TQMap<TQString,TQString>	tmpset = d->m_options;
	d->m_options = opts;
	// remove some problematic options that may not be overwritten (ugly hack).
	// Default values will be used instead, except if the dialog has set new ones.
	tmpset.remove("kde-pagesize");
	tmpset.remove( "kde-printsize" );
	tmpset.remove("kde-orientation");
	tmpset.remove("kde-colormode");
	tmpset.remove("kde-margin-top");
	tmpset.remove("kde-margin-left");
	tmpset.remove("kde-margin-bottom");
	tmpset.remove("kde-margin-right");
	tmpset.remove( "kde-resolution" );
	tmpset.remove( "kde-fonts" );
	for (TQMap<TQString,TQString>::ConstIterator it=tmpset.begin();it!=tmpset.end();++it)
		if (it.key().left(4) == "kde-" && !(d->m_options.contains(it.key())))
			d->m_options[it.key()] = it.data();
}

void KPrinter::initOptions(const TQMap<TQString,TQString>& opts)
{ // This function can be used to initialize the KPrinter object just after
  // creation to set some options. Non global options will be propagated to
  // all listed printers (non-global => start with "kde-...")
	for (TQMap<TQString,TQString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
	{
		setOption(it.key(), it.data());
		if (it.key().left(4) != "kde-")
			d->m_impl->broadcastOption(it.key(),it.data());
	}
}

void KPrinter::reload()
{
	d->m_impl = KMFactory::self()->printerImplementation();
	int	global = KMFactory::self()->settings()->orientation;
	if (global != -1) setOrientation((KPrinter::Orientation)global);
	global = KMFactory::self()->settings()->pageSize;
	if (global != -1) setPageSize((KPrinter::PageSize)global);
	//initOptions(d->m_options);
}

bool KPrinter::autoConfigure(const TQString& prname, TQWidget *parent)
{
	KMManager	*mgr = KMManager::self();
	KMPrinter	*mprt(0);

	mgr->printerList(false);
	if (prname.isEmpty())
		mprt = mgr->defaultPrinter();
	else
		mprt = mgr->findPrinter(prname);

	if (mprt)
		return mprt->autoConfigure(this, parent);
	else
		return false;
}

//**************************************************************************************
// Util functions
//**************************************************************************************

void reportError(KPrinter *p)
{
	if (!KNotifyClient::event(0,"printerror",i18n("<p><nobr>A print error occurred. Error message received from system:</nobr></p><br>%1").arg(p->errorMessage())))
		kdDebug(500) << "could not send notify event" << endl;
}

KPrinter::PageSize pageNameToPageSize(const TQString& _name)
{
	TQString name = _name.upper();
	if (name == "LETTER") return KPrinter::Letter;
	else if (name == "LEGAL") return KPrinter::Legal;
	else if (name == "A4") return KPrinter::A4;
	else if (name == "A3") return KPrinter::A3;
	else if (name == "EXECUTIVE") return KPrinter::Executive;
	else if (name == "LEDGER") return KPrinter::Ledger;
	else if (name == "TABLOID") return KPrinter::Tabloid;
	else if (name == "FOLIO") return KPrinter::Folio;
	else if (name == "A5") return KPrinter::A5;
	else if (name == "A6") return KPrinter::A6;
	else if (name == "A7") return KPrinter::A7;
	else if (name == "A8") return KPrinter::A8;
	else if (name == "A9") return KPrinter::A9;
	else if (name == "A2") return KPrinter::A2;
	else if (name == "A1") return KPrinter::A1;
	else if (name == "A0") return KPrinter::A0;
	else if (name == "B0" || name == "B0ISO") return KPrinter::B0;
	else if (name == "B1" || name == "B1ISO") return KPrinter::B1;
	else if (name == "B2" || name == "B2ISO") return KPrinter::B2;
	else if (name == "B3" || name == "B3ISO") return KPrinter::B3;
	else if (name == "B4" || name == "B4ISO") return KPrinter::B4;
	else if (name == "B5" || name == "B5ISO") return KPrinter::B5;
	else if (name == "B6" || name == "B6ISO") return KPrinter::B6;
	else if (name == "B7" || name == "B7ISO") return KPrinter::B7;
	else if (name == "B8" || name == "B8ISO") return KPrinter::B8;
	else if (name == "B9" || name == "B9ISO") return KPrinter::B9;
	else if (name == "B10" || name == "B10ISO") return KPrinter::B10;
	else if (name == "C5" || name == "C5E" || name == "ENVC5") return KPrinter::C5E;
	else if (name == "DL" || name == "DLE" || name == "ENVDL") return KPrinter::DLE;
	else if (name == "COMM10" || name == "COM10" || name == "ENV10") return KPrinter::Comm10E;
	else return KPrinter::A4;
}

const char* pageSizeToPageName(KPrinter::PageSize s)
{
	switch(s)
	{
		case KPrinter::Letter: return "Letter";
		case KPrinter::Legal: return "Legal";
		case KPrinter::A4: return "A4";
		case KPrinter::A3: return "A3";
		case KPrinter::Executive: return "Executive";
		case KPrinter::Ledger: return "Ledger";
		case KPrinter::Tabloid: return "Tabloid";
		case KPrinter::Folio: return "Folio";
		case KPrinter::A5: return "A5";
		case KPrinter::A6: return "A6";
		case KPrinter::A7: return "A7";
		case KPrinter::A8: return "A8";
		case KPrinter::A9: return "A9";
		case KPrinter::A2: return "A2";
		case KPrinter::A1: return "A1";
		case KPrinter::A0: return "A0";
		case KPrinter::B0: return "B0";
		case KPrinter::B1: return "B1";
		case KPrinter::B2: return "B2";
		case KPrinter::B3: return "B3";
		case KPrinter::B4: return "B4";
		case KPrinter::B5: return "B5";
		case KPrinter::B6: return "B6";
		case KPrinter::B7: return "B7";
		case KPrinter::B8: return "B8";
		case KPrinter::B9: return "B9";
		case KPrinter::B10: return "B10";
		case KPrinter::C5E: return "C5";
		case KPrinter::DLE: return "DL";
		case KPrinter::Comm10E: return "Comm10";
		default: return "A4";
	}
}

// FIXME: remove for 4.0
TQSize rangeToSize( const TQString& )
{
	kdWarning( 500 ) << "rangeToSize(TQString) is obsolete, do not use (no effect)" << endl;
	return TQSize();
}

static void dumpOptions(const TQMap<TQString,TQString>& opts)
{
	kdDebug(500) << "********************" << endl;
	for (TQMap<TQString,TQString>::ConstIterator it=opts.begin(); it!=opts.end(); ++it)
		kdDebug(500) << it.key() << " = " << it.data() << endl;
}

KPrinterImpl* KPrinter::implementation() const
{ return d->m_impl; }

const TQString& KPrinter::option(const TQString& key) const
{ return ((const KPrinterPrivate*)(d))->m_options[key]; }

void KPrinter::setOption(const TQString& key, const TQString& value)
{ d->m_options[key] = value; }

TQString KPrinter::docName() const
{ return option("kde-docname"); }

void KPrinter::setDocName(const TQString& d)
{ setOption("kde-docname",d); }

TQString KPrinter::creator() const
{ return option("kde-creator"); }

void KPrinter::setCreator(const TQString& d)
{ setOption("kde-creator",d); }

bool KPrinter::fullPage() const
{ return (option("kde-fullpage") == "1"); }

void KPrinter::setFullPage(bool on)
{ setOption("kde-fullpage",(on ? "1" : "0")); }

KPrinter::ColorMode KPrinter::colorMode() const
{ return (KPrinter::ColorMode)(option("kde-colormode") == "GrayScale" ? GrayScale : Color); }

void KPrinter::setColorMode(ColorMode m)
{ setOption("kde-colormode",(m == Color ? "Color" : "GrayScale")); }

void KPrinter::setNumCopies(int n)
{ setOption("kde-copies",TQString::number(n)); }

KPrinter::Orientation KPrinter::orientation() const
{ return (option("kde-orientation") == "Landscape" ? Landscape : Portrait); }

KPrinter::PageOrder KPrinter::pageOrder() const
{ return (option("kde-pageorder") == "Reverse" ? LastPageFirst : FirstPageFirst); }

void KPrinter::setPageOrder(PageOrder o)
{ setOption("kde-pageorder",(o == LastPageFirst ? "Reverse" : "Forward")); }

KPrinter::CollateType KPrinter::collate() const
{ return (option("kde-collate") == "Collate" ? Collate : Uncollate); }

void KPrinter::setCollate(CollateType c)
{ setOption("kde-collate",(c == Collate ? "Collate" : "Uncollate")); }

int KPrinter::minPage() const
{ return (option("kde-minpage").isEmpty() ? 0 : option("kde-minpage").toInt()); }

int KPrinter::maxPage() const
{ return (option("kde-maxpage").isEmpty() ? 0 : option("kde-maxpage").toInt()); }

void KPrinter::setMinMax(int m, int M)
{ setOption("kde-minpage",TQString::number(m)); setOption("kde-maxpage",TQString::number(M)); }

int KPrinter::fromPage() const
{ return (option("kde-frompage").isEmpty() ? 0 : option("kde-frompage").toInt()); }

int KPrinter::toPage() const
{ return (option("kde-topage").isEmpty() ? 0 : option("kde-topage").toInt()); }

void KPrinter::setFromTo(int m, int M)
{ setOption("kde-frompage",TQString::number(m)); setOption("kde-topage",TQString::number(M)); setOption("kde-range",(m>0 && M>0 ? TQString("%1-%2").arg(m).arg(M) : TQString::fromLatin1(""))); }

// if no page size defined, use the localized one
KPrinter::PageSize KPrinter::pageSize() const
{ return (option("kde-pagesize").isEmpty() ? (PageSize)TDEGlobal::locale()->pageSize() : (PageSize)option("kde-pagesize").toInt()); }

KPrinter::PageSetType KPrinter::pageSet() const
{ return (option("kde-pageset").isEmpty() ? AllPages : (PageSetType)(option("kde-pageset").toInt())); }

int KPrinter::currentPage() const
{ return (option("kde-currentpage").isEmpty() ? 0 : option("kde-currentpage").toInt()); }

void KPrinter::setCurrentPage(int p)
{ setOption("kde-currentpage",TQString::number(p)); }

TQString KPrinter::printerName() const
{ return d->m_printername; }

void KPrinter::setPrinterName(const TQString& s)
{ d->m_printername = s; }

TQString KPrinter::printProgram() const
{ return (option("kde-isspecial") == "1" ? option("kde-special-command") : TQString::null); }

void KPrinter::setPrintProgram(const TQString& prg)
{
	if (prg.isNull())
	{
		setOption("kde-isspecial", "0");
		d->m_options.remove("kde-special-command");
	}
	else
	{
		TQString	s(prg);
		if (s.find("%in") == -1)
			s.append(" %in");
		setOutputToFile( s.find( "%out" ) != -1 );
		setOption("kde-isspecial", "1");
		setOption("kde-special-command", s);
	}
}

TQString KPrinter::printerSelectionOption() const
{ return TQString::fromLatin1(""); }

void KPrinter::setPrinterSelectionOption(const TQString&)
{}

const TQMap<TQString,TQString>& KPrinter::options() const
{ return d->m_options; }

TQString KPrinter::searchName() const
{ return d->m_searchname; }

void KPrinter::setSearchName(const TQString& s)
{ d->m_searchname = s; }

bool KPrinter::newPage()
{
	d->m_pagenumber++;
	d->m_impl->statusMessage(i18n("Generating print data: page %1").arg(d->m_pagenumber), this);
	return d->m_wrapper->newPage();
}

TQString KPrinter::outputFileName() const
{ return option("kde-outputfilename"); }

void KPrinter::setOutputFileName(const TQString& f)
{ setOption("kde-outputfilename",f); setOutputToFile(!f.isEmpty()); }

bool KPrinter::outputToFile() const
{ return (option("kde-outputtofile") == "1" || (option("kde-isspecial") == "1" && option("kde-special-command").isEmpty())); }

void KPrinter::setOutputToFile(bool on)
{
	setOption("kde-outputtofile",(on ? "1" : "0"));
	if (on)
	{
		setOption("kde-special-command",TQString::null);
		setOption("kde-isspecial","1");
	}
}

bool KPrinter::abort()
{ return d->m_wrapper->abort(); }

bool KPrinter::aborted() const
{ return d->m_wrapper->aborted(); }

void KPrinter::setMargins(TQSize m)
{
	setMargins( m.height(), m.width(), m.height(), m.width() );
}

void KPrinter::setMargins( uint top, uint left, uint bottom, uint right )
{
	d->m_wrapper->setMargins( top, left, bottom, right );
	setOption( "kde-margin-top", TQString::number( top ), true );
	setOption( "kde-margin-left", TQString::number( left ), true );
	setOption( "kde-margin-bottom", TQString::number( bottom ), true );
	setOption( "kde-margin-right", TQString::number( right ), true );
}

// FIXME: remove for 4.0
TQSize KPrinter::realPageSize() const
{
	kdWarning( 500 ) << "KPrinter::realPageSize() is obsolete, do not use" << endl;
	if ( d->m_pagesize )
		return d->m_pagesize->pageSize();
	else
		return TQSize();
}

void KPrinter::setRealPageSize(DrPageSize *p)
{
	if ( p )
	{
		kdDebug( 500 ) << "Page size:  width =" << p->pageWidth() << endl;
		kdDebug( 500 ) << "Page size: height =" << p->pageHeight() << endl;
		kdDebug( 500 ) << "Page size:   left =" << p->leftMargin() << endl;
		kdDebug( 500 ) << "Page size:    top =" << p->topMargin() << endl;
		kdDebug( 500 ) << "Page size:  right =" << p->rightMargin() << endl;
		kdDebug( 500 ) << "Page size: bottom =" << p->bottomMargin() << endl;
	}
	else
		kdDebug( 500 ) << "Resetting page size" << endl;

	/* we copy the page size structure internally
	 * as the original object is owned by the driver
	 * that control its destrution */
	delete d->m_pagesize;
	d->m_pagesize = 0;
	if ( p )
		d->m_pagesize = new DrPageSize( *p );
}

// FIXME: remove for 4.0
void KPrinter::setRealPageSize( TQSize )
{
	kdWarning( 500 ) << "KPrinter::setRealPageSize(TQSize) is obsolete, do not use (no effect)" << endl;
}

// FIXME: remove for 4.0
void KPrinter::setRealDrawableArea( const TQRect& )
{
	kdWarning( 500 ) << "KPrinter::setRealDrawableArea(TQRect) is obsolete, do not use (no effect)" << endl;
}

// FIXME: remove for 4.0
TQRect KPrinter::realDrawableArea() const
{
	kdWarning( 500 ) << "KPrinter::realDrawableArea() is obsolete, do not use" << endl;
	if ( d->m_pagesize )
		return d->m_pagesize->pageRect();
	else
		return TQRect();
}

TQString KPrinter::errorMessage() const
{ return d->m_errormsg; }

void KPrinter::setErrorMessage(const TQString& msg)
{ d->m_errormsg = msg; }

/* we're using a builtin member to store this state because we don't
 * want to keep it from object to object. So there's no need to use
 * the TQMap structure to store this
 */
void KPrinter::setPreviewOnly(bool on)
{ d->m_previewonly = on; }

bool KPrinter::previewOnly() const
{ return d->m_previewonly; }

void KPrinter::setDocFileName(const TQString& s)
{ d->m_docfilename = s; }

TQString KPrinter::docFileName() const
{ return d->m_docfilename; }

void KPrinter::setDocDirectory( const TQString& s )
{ d->m_docdirectory = s; }

TQString KPrinter::docDirectory() const
{ return ( d->m_docdirectory.isEmpty() ? TQDir::homeDirPath() : d->m_docdirectory ); }

void KPrinter::setResolution(int dpi)
{
	d->m_wrapper->setResolution(dpi);
	d->m_defaultres = dpi;
}

int KPrinter::resolution() const
{ return d->m_wrapper->resolution(); }

void KPrinter::setUsePrinterResolution( bool on )
{ d->m_useprinterres = on; }
