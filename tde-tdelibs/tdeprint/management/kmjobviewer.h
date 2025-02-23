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

#ifndef KMJOBVIEWER_H
#define KMJOBVIEWER_H

#if !defined( _TDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a TDEPrint developer
#endif

#include <tdemainwindow.h>
#include "kmprinterpage.h"
#include "kpreloadobject.h"

class KMJobManager;
class KMJob;
class TDEListView;
class JobItem;
class TQPopupMenu;
class TQListViewItem;
class KMPrinter;
class TQTimer;
class TQLineEdit;
class TQCheckBox;

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT KMJobViewer : public TDEMainWindow, public KMPrinterPage, public KPReloadObject
{
	TQ_OBJECT
public:
	KMJobViewer(TQWidget *parent = 0, const char *name = 0);
	~KMJobViewer();

	void setPrinter(const TQString& prname);
	void setPrinter(KMPrinter *p);
	void refresh(bool reload = false);
	TQString printer() const;
	bool isSticky() const;

signals:
	void jobsShown(KMJobViewer*, bool hasJobs);
	void refreshClicked();
	void printerChanged(KMJobViewer*, const TQString& prname);
	void viewerDestroyed(KMJobViewer*);

public slots:
	void pluginActionActivated(int);

protected slots:
	void slotSelectionChanged();
	void slotHold();
	void slotResume();
	void slotRemove();
	void slotRestart();
	void slotRightClicked(TQListViewItem*,const TQPoint&,int);
	void slotMove(int prID);
	void slotPrinterSelected(int);
	void slotShowCompleted(bool);
	void slotRefresh();
	void slotClose();
	void slotShowMoveMenu();
	void slotShowPrinterMenu();
	void slotUserOnly(bool);
	void slotUserChanged();
	void slotConfigure();
	void slotDropped( TQDropEvent*, TQListViewItem* );
	void slotKeepWindowChange(bool);
	void slotPrinterListUpdated();

protected:
	void init();
	void updateJobs();
	void initActions();
	JobItem* findItem(const TQString& uri);
	void jobSelection(TQPtrList<KMJob>& l);
	void send(int cmd, const TQString& name, const TQString& arg = TQString::null);
	void loadPrinters();
	void loadPluginActions();
	void removePluginActions();
	void reload();
	//void aboutToReload();
	void closeEvent(TQCloseEvent*);
	void triggerRefresh();
	void addToManager();
	void removeFromManager();
	void buildPrinterMenu(TQPopupMenu *menu, bool use_all = false, bool use_specials = false);
	void updateCaption();
	void updateStatusBar();

private:
	TDEListView		*m_view;
	TQPtrList<KMJob>		m_jobs;
	TQPtrList<JobItem>		m_items;
	TQPopupMenu		*m_pop;
	TQPtrList<KMPrinter>	m_printers;
	TQString	m_prname;
	int	m_type;
	TQString	m_username;
	TQLineEdit	*m_userfield;
	TQCheckBox	*m_stickybox;
	bool m_standalone;
};

inline TQString KMJobViewer::printer() const
{ return m_prname; }

#endif
