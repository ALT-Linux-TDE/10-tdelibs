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

#ifndef CUPSDSECURITYPAGE_H
#define CUPSDSECURITYPAGE_H

#include "cupsdpage.h"
#include <tqptrlist.h>

class TQLineEdit;
class EditList;
class QDirLineEdit;
struct CupsLocation;

class CupsdSecurityPage : public CupsdPage
{
	TQ_OBJECT

public:
	CupsdSecurityPage(TQWidget *parent = 0, const char *name = 0);

	bool loadConfig(CupsdConf*, TQString&);
	bool saveConfig(CupsdConf*, TQString&);
	void setInfos(CupsdConf*);

protected slots:
	void slotAdd();
	void slotEdit(int);
	void slotDefaultList();
	void slotDeleted(int);

private:
	TQLineEdit	*remoteroot_, *systemgroup_;
	QDirLineEdit	*encryptcert_, *encryptkey_;
	EditList	*locations_;

	TQPtrList<CupsLocation>	locs_;
};

#endif
