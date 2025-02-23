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

#ifndef KMMANAGER_H
#define KMMANAGER_H

#if !defined( _TDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a TDEPrint developer
#endif

#include <tdeprint/kmprinter.h>

#include <tqobject.h>
#include <tqstring.h>
#include <tqptrlist.h>

class DrMain;
class KMDBEntry;
class KMVirtualManager;
class KMSpecialManager;
class TQWidget;
class TDEActionCollection;
class PrinterFilter;

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT KMManager : public TQObject
{
	TQ_OBJECT

friend class KMVirtualManager;
friend class KMSpecialManager;
friend class KMFactory;

public:
	enum PrinterOperations {
		PrinterEnabling  = 0x01,
		PrinterCreation  = 0x02,
		PrinterDefault   = 0x04,
		PrinterTesting   = 0x08,
		PrinterConfigure = 0x10,
		PrinterRemoval   = 0x20,
		PrinterAll       = 0xFF
	};
	enum ServerOperations {
		ServerRestarting = 0x1,
		ServerConfigure  = 0x2,
		ServerAll        = 0xF
	};

	KMManager(TQObject *parent = 0, const char *name = 0);
	virtual ~KMManager();

    static KMManager* self();

	// error management functions
	TQString errorMsg() const		{ return m_errormsg; }
	void setErrorMsg(const TQString& s)	{ m_errormsg = s; }

	// support management ?
	bool hasManagement() const 		{ return m_hasmanagement; }

	// printer management functions
	virtual bool createPrinter(KMPrinter *p);
	virtual bool removePrinter(KMPrinter *p);
	virtual bool enablePrinter(KMPrinter *p, bool on);
	virtual bool startPrinter(KMPrinter *p, bool on);
	virtual bool completePrinter(KMPrinter *p);
	virtual bool completePrinterShort(KMPrinter *p);
	virtual bool setDefaultPrinter(KMPrinter *p);
	virtual bool testPrinter(KMPrinter *p);
	bool upPrinter(KMPrinter *p, bool state);
	bool modifyPrinter(KMPrinter *oldp, KMPrinter *newp);
	bool removePrinter(const TQString& name);
	bool enablePrinter(const TQString& name, bool state);
	bool startPrinter(const TQString& name, bool state);
	bool completePrinter(const TQString& name);
	bool setDefaultPrinter(const TQString& name);
	int printerOperationMask() const 	{ return m_printeroperationmask; }
	int addPrinterWizard(TQWidget *parent = 0);

	// special printer management functions
	bool createSpecialPrinter(KMPrinter *p);
	bool removeSpecialPrinter(KMPrinter *p);

	// printer listing functions
	KMPrinter* findPrinter(const TQString& name);
	TQPtrList<KMPrinter>* printerList(bool reload = true);
	TQPtrList<KMPrinter>* printerListComplete(bool reload = true);
	KMPrinter* defaultPrinter();
	void enableFilter(bool on);
	bool isFilterEnabled() const;

	// driver DB functions
	virtual TQString driverDbCreationProgram();
	virtual TQString driverDirectory();

	// driver functions
	virtual DrMain* loadPrinterDriver(KMPrinter *p, bool config = false);
	virtual DrMain* loadDbDriver(KMDBEntry *entry);
	virtual DrMain* loadFileDriver(const TQString& filename);
	DrMain* loadDriver(KMPrinter *p, bool config = false);
	virtual bool savePrinterDriver(KMPrinter *p, DrMain *d);
	virtual bool validateDbDriver(KMDBEntry *entry);

	// configuration functions
	bool invokeOptionsDialog(TQWidget *parent = 0);
	virtual TQString stateInformation();

	// server functions
	int serverOperationMask() const 	{ return m_serveroperationmask; }
	virtual bool restartServer();
	virtual bool configureServer(TQWidget *parent = 0);
	virtual TQStringList detectLocalPrinters();

	// additional actions (for print manager)
	virtual void createPluginActions(TDEActionCollection*);
	virtual void validatePluginActions(TDEActionCollection*, KMPrinter*);

	// utility function
	void checkUpdatePossible();

signals:
	void updatePossible(bool);
	void printerListUpdated();

protected:
	// the real printer listing job is done here
	virtual void listPrinters();

	// utility functions
	void addPrinter(KMPrinter *p);	// in any case, the pointer given MUST not be used after
					// calling this function. Useful when listing printers.
	void setHardDefault(KMPrinter*);
	void setSoftDefault(KMPrinter*);
	KMPrinter* softDefault() const;
	KMPrinter* hardDefault() const;
	// this function uncompress the given file (or does nothing
	// if the file is not compressed). Returns wether the file was
	// compressed or not.
	bool uncompressFile(const TQString& srcname, TQString& destname);
	bool notImplemented();
	void setHasManagement(bool on)		{ m_hasmanagement = on; }
	void setPrinterOperationMask(int m)	{ m_printeroperationmask = m; }
	void setServerOperationMask(int m)	{ m_serveroperationmask = m; }
	TQString testPage();
	void discardAllPrinters(bool);
	void setUpdatePossible( bool );
	virtual void checkUpdatePossibleInternal();

protected:
	TQString			m_errormsg;
	KMPrinterList		m_printers, m_fprinters;	// filtered printers
	bool 			m_hasmanagement;
	int			m_printeroperationmask;
	int 			m_serveroperationmask;
	KMSpecialManager	*m_specialmgr;
	KMVirtualManager	*m_virtualmgr;
	PrinterFilter	*m_printerfilter;
	bool m_updatepossible;
};

#endif
