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

#ifndef	CUPSDDIALOG_H
#define	CUPSDDIALOG_H

#include <kdialogbase.h>
#include <tqptrlist.h>

class CupsdPage;
struct CupsdConf;

class CupsdDialog : public KDialogBase
{
	TQ_OBJECT
public:
	CupsdDialog(TQWidget *parent = 0, const char *name = 0);
	~CupsdDialog();

	bool setConfigFile(const TQString& filename);

	static bool configure(const TQString& filename = TQString::null, TQWidget *parent = 0, TQString *errormsg = 0);
	static bool restartServer(TQString& msg);
	static int serverPid();
	static int serverOwner();

protected slots:
	void slotOk();
	void slotUser1();

protected:
	void addConfPage(CupsdPage*);
	void constructDialog();
	void restartServer();

private:
	TQPtrList<CupsdPage>	pagelist_;
	CupsdConf		*conf_;
	TQString			filename_;
};

#endif
