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

#ifndef SMBVIEW_H
#define SMBVIEW_H

#include <tdelistview.h>

class TDEProcess;
class KTempFile;

class SmbView : public TDEListView
{
	TQ_OBJECT
public:
	SmbView(TQWidget *parent = 0, const char *name = 0);
	~SmbView();

	void setLoginInfos(const TQString& login, const TQString& password);
	void setOpen(TQListViewItem*, bool);
	void init();
	void abort();

signals:
	void printerSelected(const TQString& work, const TQString& server, const TQString& printer);
	void running(bool);

protected:
	void startProcess(int);
	void endProcess();
	void processGroups();
	void processServers();
	void processShares();

protected slots:
	void slotReceivedStdout(TDEProcess*, char*, int);
	void slotProcessExited(TDEProcess*);
	void slotSelectionChanged(TQListViewItem*);

private:
	enum State { GroupListing, ServerListing, ShareListing, Idle };
	int 		m_state;
	TQListViewItem	*m_current;
	TDEProcess	*m_proc;
	TQString	m_buffer;
	TQString	m_login, m_password;
	KTempFile	*m_passwdFile;
	TQString	m_wins_server;
};

#endif
