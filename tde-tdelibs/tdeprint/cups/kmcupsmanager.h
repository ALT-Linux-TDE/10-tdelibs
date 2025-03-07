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

#ifndef KMCUPSMANAGER_H
#define KMCUPSMANAGER_H

#include "kmmanager.h"

class IppRequest;
class KLibrary;
class KExtendedSocket;

namespace KNetwork {
    class KStreamSocket;
}

class KMCupsManager : public KMManager
{
	friend class KMWIppPrinter;
	friend class KMCupsJobManager;

	TQ_OBJECT
public:
	KMCupsManager(TQObject *parent, const char *name, const TQStringList & /*args*/);
	virtual ~KMCupsManager();

	// printer management functions
	bool createPrinter(KMPrinter *p);
	bool removePrinter(KMPrinter *p);
	bool enablePrinter(KMPrinter *p, bool state);
	bool startPrinter(KMPrinter *p, bool state);
	bool completePrinter(KMPrinter *p);
	bool completePrinterShort(KMPrinter *p);
	bool setDefaultPrinter(KMPrinter *p);
	bool testPrinter(KMPrinter *p);

	// printer listing functions
	// driver DB functions
	TQString driverDbCreationProgram();
	TQString driverDirectory();

	DrMain* loadPrinterDriver(KMPrinter *p, bool config = false);
	DrMain* loadFileDriver(const TQString& filename);
	bool savePrinterDriver(KMPrinter *p, DrMain *d);

	bool restartServer();
	bool configureServer(TQWidget *parent = 0);
	TQStringList detectLocalPrinters();

	void createPluginActions(TDEActionCollection*);
	void validatePluginActions(TDEActionCollection*, KMPrinter*);
	TQString stateInformation();

public slots:
	void exportDriver();
	void printerIppReport();

protected slots:
	void slotConnectionFailed( int );
	void slotConnectionSuccess();
	void slotAsyncConnect();

	void hostPingSlot();
	void hostPingFailedSlot();

protected:
	// the real printer listing job is done here
	void listPrinters();
	void loadServerPrinters();
	void processRequest(IppRequest*);
	bool setPrinterState(KMPrinter *p, int st);
	DrMain* loadDriverFile(const TQString& filename);
	DrMain* loadMaticDriver(const TQString& drname);
	void saveDriverFile(DrMain *driver, const TQString& filename);
	void reportIppError(IppRequest*);
	void* loadCupsdConfFunction(const char*);
	void unloadCupsdConf();
	TQString cupsInstallDir();
	void ippReport(IppRequest&, int, const TQString&);
	void checkUpdatePossibleInternal();

private:
	KLibrary	*m_cupsdconf;
	KMPrinter	*m_currentprinter;
        KNetwork::KStreamSocket   *m_socket;
	bool m_hostSuccess;
	bool m_lookupDone;
};

#endif
