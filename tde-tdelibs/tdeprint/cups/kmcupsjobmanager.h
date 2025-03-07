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

#ifndef KMCUPSJOBMANAGER_H
#define KMCUPSJOBMANAGER_H

#include "kmjobmanager.h"
#include "ipprequest.h"
#include <tqstringlist.h>

class KMPrinter;

class KMCupsJobManager : public KMJobManager
{
	TQ_OBJECT

public:
	KMCupsJobManager(TQObject *parent, const char *name, const TQStringList &/*args*/);
	virtual ~KMCupsJobManager();

	int actions();
	TQValueList<TDEAction*> createPluginActions(TDEActionCollection*);
	void validatePluginActions(TDEActionCollection*, const TQPtrList<KMJob>&);
	bool doPluginAction(int, const TQPtrList<KMJob>&);

protected:
	bool jobIppReport(KMJob*);
	bool changePriority(const TQPtrList<KMJob>&, bool);
	bool editJobAttributes(KMJob*);

protected:
	bool listJobs(const TQString&, JobType, int = 0);
	bool sendCommandSystemJob(const TQPtrList<KMJob>& jobs, int action, const TQString& arg = TQString::null);
	void parseListAnswer(IppRequest& req, KMPrinter *pr);
};

#endif
