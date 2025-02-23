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

#ifndef KMMAINVIEW_H
#define KMMAINVIEW_H

#if !defined( _TDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a TDEPrint developer
#endif

#include <tqwidget.h>
#include <tdeprint/kpreloadobject.h>
#include <tdemainwindow.h>

class KMManager;
class KMPrinterView;
class KMPrinter;
class KMPages;
class TDEActionCollection;
class TQPopupMenu;
class TQTimer;
class TQSplitter;
class TDEToolBar;
class TDEAction;
class PluginComboBox;
class TQBoxLayout;
class MessageWindow;
class TQMenuBar;

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT KMMainView : public TQWidget, public KPReloadObject
{
	TQ_OBJECT
public:
	KMMainView(TQWidget *parent = 0, const char *name = 0, TDEActionCollection *coll = 0);
	~KMMainView();

	void setOrientation(int);
	int orientation() const;
	void setViewType(int);
	int viewType() const;
	void enableToolbar(bool on = true);
	TDEAction* action(const char *name);
	void showPrinterInfos(bool on);
	bool printerInfosShown() const;

public slots:
	void slotTimer();
	void slotShowPrinterInfos(bool);
	void slotChangePrinterState();
	void slotRemove();
	void slotConfigure();
	void slotAdd();
	void slotHardDefault();
	void slotSoftDefault();
	void slotTest();
	void slotServerRestart();
	void slotServerConfigure();
	void slotServerConfigureAccess();
	void slotManagerConfigure();
	void slotAddSpecial();
	void slotRefresh();
	void slotToolSelected(int);
	void slotToggleFilter(bool);

protected slots:
	void slotPrinterSelected(const TQString&);
	void slotRightButtonClicked(const TQString&, const TQPoint&);
	void slotToggleToolBar(bool);
	void slotToggleMenuBar(bool);
	void slotChangeView(int);
	void slotChangeDirection(int);
	void slotUpdatePossible( bool );
	void slotInit();

protected:
	void initActions();
	void showErrorMsg(const TQString& msg, bool usemgr = true);
	void restoreSettings();
	void saveSettings();
	void loadParameters();
	void reload();
	void configChanged();
	//void aboutToReload();
	void loadPluginActions();
	void removePluginActions();
	void createMessageWindow( const TQString&, int delay = 500 );
	void destroyMessageWindow();
	void reset( const TQString& msg = TQString::null, bool useDelay = true, bool holdTimer = true );

private:
	KMPrinterView	*m_printerview;
	KMPages		*m_printerpages;
	TQPopupMenu	*m_pop;
	TDEActionCollection	*m_actions;
	KMPrinter	*m_current;
	TDEToolBar	*m_toolbar;
	PluginComboBox	*m_plugin;
	int		m_pactionsindex;
	TQStringList	m_toollist;
	bool		m_first;
	TQBoxLayout	*m_boxlayout;
	class TDEMainWindowPrivate;
	TDEMainWindowPrivate *d;
	TDEToolBar *m_menubar;
};

TDEPRINT_EXPORT int tdeprint_management_add_printer_wizard( TQWidget* parent );

#endif
