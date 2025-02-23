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

#ifndef TDEPRINTD_H
#define TDEPRINTD_H

#include <kdedmodule.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include <tqptrdict.h>
#include <tqguardedptr.h>
#include <tqintdict.h>

class KPrintProcess;
class TDEProcess;
class StatusWindow;

class KDEPrintd : public KDEDModule
{
	TQ_OBJECT
	K_DCOP

public:
	KDEPrintd(const TQCString& obj);
	~KDEPrintd();

k_dcop:
	int print(const TQString& cmd, const TQStringList& files, bool remove);
	TQString openPassDlg(const TQString& user);
	ASYNC statusMessage(const TQString& msg, int pid = -1, const TQString& appName = TQString::null);
	TQString requestPassword( const TQString& user, const TQString& host, int port, int seqNbr );
	void initPassword( const TQString& user, const TQString& passwd, const TQString& host, int port );

protected slots:
	void slotPrintTerminated( KPrintProcess* );
	void slotPrintError( KPrintProcess*, const TQString& );
	void slotClosed();
	void processRequest();

protected:
	bool checkFiles(TQString& cmd, const TQStringList& files);

private:
	class Request;
	TQPtrList<KPrintProcess>	m_processpool;
	TQIntDict<StatusWindow>	m_windows;
	TQPtrList<Request>       m_requestsPending;
};

#endif
