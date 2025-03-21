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

#include "kmmainview.h"
#include "kmtimer.h"
#include "kmprinterview.h"
#include "kmpages.h"
#include "kmmanager.h"
#include "kmuimanager.h"
#include "kmfactory.h"
#include "kmvirtualmanager.h"
#include "kmprinter.h"
#include "driver.h"
#include "kmdriverdialog.h"
#include "kmwizard.h"
#include "kmconfigdialog.h"
#include "kmspecialprinterdlg.h"
#include "plugincombobox.h"
#include "kiconselectaction.h"
#include "messagewindow.h"

#include <tqdockarea.h>
#include <tdemenubar.h>
#include <tqtimer.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpopupmenu.h>
#include <tdemessagebox.h>
#include <tdeaction.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <tdetoolbar.h>
#include <tdetoolbarbutton.h>
#include <kdebug.h>
#include <tdepopupmenu.h>
#include <klibloader.h>
#include <kdialogbase.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <tdeapplication.h>
#include <kprocess.h>

#undef m_manager
#define	m_manager	KMFactory::self()->manager()

int tdeprint_management_add_printer_wizard( TQWidget* parent )
{
		KMWizard	dlg(parent);
		int		flag(0);
		if (dlg.exec())
		{
			flag = 1;
			// check if the printer already exists, and ask confirmation if needed.
			if (KMFactory::self()->manager()->findPrinter(dlg.printer()->name()) != 0)
				if (KMessageBox::warningContinueCancel(parent,i18n("The printer %1 already exists. Continuing will overwrite existing printer. Do you want to continue?").arg(dlg.printer()->name())) == KMessageBox::Cancel)
					flag = 0;
			// try to add printer only if flag is true.
			if (flag && !KMFactory::self()->manager()->createPrinter(dlg.printer()))
				flag = -1;
		}
		return flag;
}

KMMainView::KMMainView(TQWidget *parent, const char *name, TDEActionCollection *coll)
: TQWidget(parent, name)
{
	m_current = 0;
	m_first = true;

	// create widgets
	m_printerview = new KMPrinterView(this, "PrinterView");
	m_printerpages = new KMPages(this, "PrinterPages");
	m_pop = new TQPopupMenu(this);
	m_toolbar = new TDEToolBar(this, "ToolBar");
	m_toolbar->setMovingEnabled(false);
	m_plugin = new PluginComboBox(this, "Plugin");
	/*
	m_menubar = new KMenuBar( this );
	static_cast<KMenuBar*>( m_menubar )->setTopLevelMenu( false );
	*/
	m_menubar = new TDEToolBar( this, "MenuBar", false, false );
	m_menubar->setIconText( TDEToolBar::IconTextRight );
	m_menubar->setMovingEnabled( false );

	// layout
	TQVBoxLayout	*m_layout = new TQVBoxLayout(this, 0, 0);
	m_layout->addWidget(m_toolbar);
	m_layout->addWidget( m_menubar );
	m_boxlayout = new TQBoxLayout(TQBoxLayout::TopToBottom, 0, 0);
	m_layout->addLayout(m_boxlayout);
	m_boxlayout->addWidget(m_printerview);
	m_boxlayout->addWidget(m_printerpages);
	m_layout->addSpacing(5);
	m_layout->addWidget(m_plugin, 0);

	// connections
	connect(KMTimer::self(),TQ_SIGNAL(timeout()),TQ_SLOT(slotTimer()));
	connect(m_printerview,TQ_SIGNAL(printerSelected(const TQString&)),TQ_SLOT(slotPrinterSelected(const TQString&)));
	connect(m_printerview,TQ_SIGNAL(rightButtonClicked(const TQString&,const TQPoint&)),TQ_SLOT(slotRightButtonClicked(const TQString&,const TQPoint&)));
	connect(m_pop,TQ_SIGNAL(aboutToShow()),KMTimer::self(),TQ_SLOT(hold()));
	connect(m_pop,TQ_SIGNAL(aboutToHide()),KMTimer::self(),TQ_SLOT(release()));
	connect( m_manager, TQ_SIGNAL( updatePossible( bool ) ), TQ_SLOT( slotUpdatePossible( bool ) ) );

	// actions
    if (coll)
		m_actions = coll;
	else
		m_actions = new TDEActionCollection(this);
	initActions();

	// first update
	restoreSettings();
	loadParameters();

	// delay first update until KMManager is ready
	reset( i18n( "Initializing manager..." ), true, true );
}

KMMainView::~KMMainView()
{
	saveSettings();
	//KMFactory::release();
}

void KMMainView::loadParameters()
{
}

void KMMainView::restoreSettings()
{
	TDEConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	setViewType((KMPrinterView::ViewType)conf->readNumEntry("ViewType",KMPrinterView::Icons));
	setOrientation(conf->readNumEntry("Orientation", TQt::Vertical));
	bool 	view = conf->readBoolEntry("ViewToolBar",false);
	slotToggleToolBar(view);
	((TDEToggleAction*)m_actions->action("view_toolbar"))->setChecked(view);
	view = conf->readBoolEntry( "ViewMenuBar", true );
	slotToggleMenuBar( view );
	static_cast<TDEToggleAction*>( m_actions->action( "view_menubar" ) )->setChecked( view );
	view = conf->readBoolEntry("ViewPrinterInfos",true);
	slotShowPrinterInfos(view);
	((TDEToggleAction*)m_actions->action("view_printerinfos"))->setChecked(view);
}

void KMMainView::saveSettings()
{
	TDEConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	conf->writeEntry("ViewType",(int)m_printerview->viewType());
	conf->writeEntry("Orientation",(int)orientation());
	conf->writeEntry("ViewToolBar",((TDEToggleAction*)m_actions->action("view_toolbar"))->isChecked());
	conf->writeEntry("ViewMenuBar",static_cast<TDEToggleAction*>( m_actions->action("view_menubar") )->isChecked());
	conf->writeEntry("ViewPrinterInfos",((TDEToggleAction*)m_actions->action("view_printerinfos"))->isChecked());
	conf->sync();
}

void KMMainView::initActions()
{
	TDEIconSelectAction	*vact = new TDEIconSelectAction(i18n("&View"),0,m_actions,"view_change");
	TQStringList	iconlst;
	iconlst << "view_icon" << "view_detailed" << "view_tree";
	vact->setItems(TQStringList::split(',',i18n("&Icons,&List,&Tree"),false), iconlst);
	vact->setCurrentItem(0);
	connect(vact,TQ_SIGNAL(activated(int)),TQ_SLOT(slotChangeView(int)));

	TDEActionMenu	*stateAct = new TDEActionMenu(i18n("Start/Stop Printer"), "tdeprint_printstate", m_actions, "printer_state_change");
	stateAct->setDelayed(false);
	stateAct->insert(new TDEAction(i18n("&Start Printer"),"tdeprint_enableprinter",0,this,TQ_SLOT(slotChangePrinterState()),m_actions,"printer_start"));
	stateAct->insert(new TDEAction(i18n("Sto&p Printer"),"tdeprint_stopprinter",0,this,TQ_SLOT(slotChangePrinterState()),m_actions,"printer_stop"));

	stateAct = new TDEActionMenu(i18n("Enable/Disable Job Spooling"), "tdeprint_queuestate", m_actions, "printer_spool_change");
	stateAct->setDelayed(false);
	stateAct->insert(new TDEAction(i18n("&Enable Job Spooling"),"tdeprint_enableprinter",0,this,TQ_SLOT(slotChangePrinterState()),m_actions,"printer_enable"));
	stateAct->insert(new TDEAction(i18n("&Disable Job Spooling"),"tdeprint_stopprinter",0,this,TQ_SLOT(slotChangePrinterState()),m_actions,"printer_disable"));

	new TDEAction(i18n("&Remove"),"edittrash",0,this,TQ_SLOT(slotRemove()),m_actions,"printer_remove");
	new TDEAction(i18n("&Configure..."),"configure",0,this,TQ_SLOT(slotConfigure()),m_actions,"printer_configure");
	new TDEAction(i18n("Add &Printer/Class..."),"tdeprint_addprinter",0,this,TQ_SLOT(slotAdd()),m_actions,"printer_add");
	new TDEAction(i18n("Add &Special (pseudo) Printer..."),"tdeprint_addpseudo",0,this,TQ_SLOT(slotAddSpecial()),m_actions,"printer_add_special");
	new TDEAction(i18n("Set as &Local Default"),"tdeprint_defaulthard",0,this,TQ_SLOT(slotHardDefault()),m_actions,"printer_hard_default");
	new TDEAction(i18n("Set as &User Default"),"tdeprint_defaultsoft",0,this,TQ_SLOT(slotSoftDefault()),m_actions,"printer_soft_default");
	new TDEAction(i18n("&Test Printer..."),"tdeprint_testprinter",0,this,TQ_SLOT(slotTest()),m_actions,"printer_test");
	new TDEAction(i18n("Configure &Manager..."),"tdeprint_configmgr",0,this,TQ_SLOT(slotManagerConfigure()),m_actions,"manager_configure");
	new TDEAction(i18n("Initialize Manager/&View"),"reload",0,this,TQ_SLOT(slotInit()),m_actions,"view_refresh");

	TDEIconSelectAction	*dact = new TDEIconSelectAction(i18n("&Orientation"),0,m_actions,"orientation_change");
	iconlst.clear();
	iconlst << "view_top_bottom" << "view_left_right";
	dact->setItems(TQStringList::split(',',i18n("&Vertical,&Horizontal"),false), iconlst);
	dact->setCurrentItem(0);
	connect(dact,TQ_SIGNAL(activated(int)),TQ_SLOT(slotChangeDirection(int)));

	new TDEAction(i18n("R&estart Server"),"tdeprint_restartsrv",0,this,TQ_SLOT(slotServerRestart()),m_actions,"server_restart");
	new TDEAction(i18n("Configure &Server..."),"tdeprint_configsrv",0,this,TQ_SLOT(slotServerConfigure()),m_actions,"server_configure");
	new TDEAction(i18n("Configure Server Access..."),"tdeprint_configsrv",0,this,TQ_SLOT(slotServerConfigureAccess()),m_actions,"server_access_configure");

	TDEToggleAction	*tact = new TDEToggleAction(i18n("Show &Toolbar"),0,m_actions,"view_toolbar");
	tact->setCheckedState(i18n("Hide &Toolbar"));
	connect(tact,TQ_SIGNAL(toggled(bool)),TQ_SLOT(slotToggleToolBar(bool)));
	tact = new TDEToggleAction( i18n( "Show Me&nu Toolbar" ), 0, m_actions, "view_menubar" );
	tact->setCheckedState(i18n("Hide Me&nu Toolbar"));
	connect( tact, TQ_SIGNAL( toggled( bool ) ), TQ_SLOT( slotToggleMenuBar( bool ) ) );
	tact = new TDEToggleAction(i18n("Show Pr&inter Details"),"tdeprint_printer_infos", 0,m_actions,"view_printerinfos");
	tact->setCheckedState(KGuiItem(i18n("Hide Pr&inter Details"),"tdeprint_printer_infos"));
	tact->setChecked(true);
	connect(tact,TQ_SIGNAL(toggled(bool)),TQ_SLOT(slotShowPrinterInfos(bool)));

	tact = new TDEToggleAction(i18n("Toggle Printer &Filtering"), "filter", 0, m_actions, "view_pfilter");
	tact->setChecked(KMManager::self()->isFilterEnabled());
	connect(tact, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotToggleFilter(bool)));

	TDEActionMenu	*mact = new TDEActionMenu(i18n("Pri&nter Tools"), "applications-utilities", m_actions, "printer_tool");
	mact->setDelayed(false);
	connect(mact->popupMenu(), TQ_SIGNAL(activated(int)), TQ_SLOT(slotToolSelected(int)));
	TQStringList	files = TDEGlobal::dirs()->findAllResources("data", "tdeprint/tools/*.desktop");
	for (TQStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
	{
		KSimpleConfig	conf(*it);
		conf.setGroup("Desktop Entry");
		mact->popupMenu()->insertItem(conf.readEntry("Name", "Unnamed"), mact->popupMenu()->count());
		m_toollist << conf.readEntry("X-TDE-Library");
	}

	// add actions to the toolbar
	m_actions->action("printer_add")->plug(m_toolbar);
	m_actions->action("printer_add_special")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("printer_state_change")->plug(m_toolbar);
	m_actions->action("printer_spool_change")->plug(m_toolbar);
	m_toolbar->insertSeparator();
	m_actions->action("printer_hard_default")->plug(m_toolbar);
	m_actions->action("printer_soft_default")->plug(m_toolbar);
	m_actions->action("printer_remove")->plug(m_toolbar);
	m_toolbar->insertSeparator();
	m_actions->action("printer_configure")->plug(m_toolbar);
	m_actions->action("printer_test")->plug(m_toolbar);
	m_actions->action("printer_tool")->plug(m_toolbar);
	m_pactionsindex = m_toolbar->insertSeparator();
	m_toolbar->insertLineSeparator();
	m_actions->action("server_restart")->plug(m_toolbar);
	m_actions->action("server_configure")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("manager_configure")->plug(m_toolbar);
	m_actions->action("view_refresh")->plug(m_toolbar);
	m_toolbar->insertLineSeparator();
	m_actions->action("view_printerinfos")->plug(m_toolbar);
	m_actions->action("view_change")->plug(m_toolbar);
	m_actions->action("orientation_change")->plug(m_toolbar);
	m_actions->action("view_pfilter")->plug(m_toolbar);

	// add actions to the menu bar
	TQPopupMenu *menu = new TQPopupMenu( this );
	m_actions->action( "printer_add" )->plug( menu );
	m_actions->action( "printer_add_special" )->plug( menu );
	//m_menubar->insertItem( i18n( "Add" ), menu );
	m_menubar->insertButton( "wizard", 0, true, i18n( "Add" ) );
	m_menubar->getButton( 0 )->setPopup( menu, true );
	menu = new TQPopupMenu( this );
	m_actions->action("printer_state_change")->plug( menu );
	m_actions->action("printer_spool_change")->plug( menu );
	menu->insertSeparator();
	m_actions->action("printer_hard_default")->plug( menu );
	m_actions->action("printer_soft_default")->plug( menu );
	m_actions->action("printer_remove")->plug( menu );
	menu->insertSeparator();
	m_actions->action("printer_configure")->plug( menu );
	m_actions->action("printer_test")->plug( menu );
	m_actions->action("printer_tool")->plug( menu );
	menu->insertSeparator();
	//m_menubar->insertItem( i18n( "Printer" ), menu );
	m_menubar->insertButton( "printer", 1, true, i18n( "Printer" ) );
	m_menubar->getButton( 1 )->setPopup( menu, true );
	menu = new TQPopupMenu( this );
	m_actions->action("server_restart")->plug( menu );
	m_actions->action("server_configure")->plug( menu );
	//m_menubar->insertItem( i18n( "Server" ), menu );
	m_menubar->insertButton( "misc", 2, true, i18n( "Print Server" ) );
	m_menubar->getButton( 2 )->setPopup( menu, true );
	menu = new TQPopupMenu( this );
	m_actions->action("manager_configure")->plug( menu );
	m_actions->action("view_refresh")->plug( menu );
	//m_menubar->insertItem( i18n( "Manager" ), menu );
	m_menubar->insertButton( "tdeprint_configmgr", 3, true, i18n( "Print Manager" ) );
	m_menubar->getButton( 3 )->setPopup( menu, true );
	menu = new TQPopupMenu( this );
	m_actions->action("view_printerinfos")->plug( menu );
	m_actions->action("view_change")->plug( menu );
	m_actions->action("orientation_change")->plug( menu );
	m_actions->action( "view_toolbar" )->plug ( menu );
	m_actions->action( "view_menubar" )->plug ( menu );
	menu->insertSeparator();
	m_actions->action("view_pfilter")->plug( menu );
	//m_menubar->insertItem( i18n( "View" ), menu );
	m_menubar->insertButton( "view_remove", 4, true, i18n( "View" ) );
	m_menubar->getButton( 4 )->setPopup( menu, true );
	//m_menubar->setMinimumHeight( m_menubar->heightForWidth( 1000 ) );

	loadPluginActions();
	slotPrinterSelected(TQString::null);
}

void KMMainView::slotRefresh()
{
	// TODO: remove me
}

void KMMainView::slotTimer()
{
	kdDebug() << "KMMainView::slotTimer" << endl;
	TQPtrList<KMPrinter>	*printerlist = m_manager->printerList();
	bool ok = m_manager->errorMsg().isEmpty();
	m_printerview->setPrinterList(printerlist);
	if ( m_first )
	{
		if ( !ok )
			showErrorMsg(i18n("An error occurred while retrieving the printer list."));
		else
		{
			/* try to select the most appropriate printer:
			 *    - soft default owner printer
			 *    - hard default printer
			 *    - first printer
			 */
			TQPtrListIterator<KMPrinter> it( *printerlist );
			KMPrinter *p1 = 0, *p2 = 0, *p3 = 0;
			while ( it.current() )
			{
				if ( !it.current()->isVirtual() )
				{
					if ( it.current()->ownSoftDefault() )
					{
						p1 = it.current();
						break;
					}
					else if ( it.current()->isHardDefault() )
						p2 = it.current();
					else if ( !p3 )
						p3 = it.current();
				}
				++it;
			}
			if ( p1 || p2 || p3 )
				m_printerview->setPrinter( p1 ? p1 : ( p2 ? p2 : p3 ) );
		}
		m_first = false;
	}
}

void KMMainView::slotPrinterSelected(const TQString& prname)
{
	KMPrinter	*p = KMManager::self()->findPrinter(prname);
	m_current = p;
	if (p && !p->isSpecial())
		KMFactory::self()->manager()->completePrinter(p);
	m_printerpages->setPrinter(p);

	// update actions state (only if toolbar enabled, workaround for toolbar
	// problem).
	//if (m_toolbar->isEnabled())
	//{
		int 	mask = (m_manager->hasManagement() ? m_manager->printerOperationMask() : 0);
		bool	sp = !(p && p->isSpecial());
//		m_actions->action("printer_remove")->setEnabled(!sp || ((mask & KMManager::PrinterRemoval) && p && p->isLocal() && !p->isImplicit()));
		m_actions->action("printer_remove")->setEnabled(!sp || ((mask & KMManager::PrinterRemoval) && p && !p->isImplicit()));
		m_actions->action("printer_configure")->setEnabled(!sp || ((mask & KMManager::PrinterConfigure) && p && !p->isClass(true) /*&& p->isLocal()*/));
		m_actions->action("printer_hard_default")->setEnabled((sp && (mask & KMManager::PrinterDefault) && p && !p->isClass(true) && !p->isHardDefault() && p->isLocal()));
		m_actions->action("printer_soft_default")->setEnabled((p && !p->isSoftDefault()));
		m_actions->action("printer_test")->setEnabled((sp && (mask & KMManager::PrinterTesting) && p && !p->isClass(true)));
		bool	stmask = (sp && (mask & KMManager::PrinterEnabling) && p);
		m_actions->action("printer_state_change")->setEnabled(stmask && p->isLocal());
		m_actions->action("printer_spool_change")->setEnabled(stmask);
		m_actions->action("printer_start")->setEnabled((stmask && p->state() == KMPrinter::Stopped));
		m_actions->action("printer_stop")->setEnabled((stmask && p->state() != KMPrinter::Stopped));
		m_actions->action("printer_enable")->setEnabled((stmask && !p->acceptJobs()));
		m_actions->action("printer_disable")->setEnabled((stmask && p->acceptJobs()));

		m_actions->action("printer_add")->setEnabled((mask & KMManager::PrinterCreation));
		mask = m_manager->serverOperationMask();
		m_actions->action("server_restart")->setEnabled((mask & KMManager::ServerRestarting));
		m_actions->action("server_configure")->setEnabled((mask & KMManager::ServerConfigure));

		KMFactory::self()->manager()->validatePluginActions(m_actions, p);
	//}
	m_actions->action("printer_tool")->setEnabled(p && !p->isClass(true) && !p->isRemote() && !p->isSpecial());
}

void KMMainView::setViewType(int ID)
{
	((TDESelectAction*)m_actions->action("view_change"))->setCurrentItem(ID);
	slotChangeView(ID);
}

int KMMainView::viewType() const
{ return m_printerview->viewType(); }

void KMMainView::slotChangeView(int ID)
{
	kdDebug() << "KMMainView::slotChangeView" << endl;
	if (ID >= KMPrinterView::Icons && ID <= KMPrinterView::Tree)
		m_printerview->setViewType((KMPrinterView::ViewType)ID);
}

void KMMainView::slotRightButtonClicked(const TQString& prname, const TQPoint& p)
{
	KMPrinter	*printer = KMManager::self()->findPrinter(prname);
	// construct popup menu
	m_pop->clear();
	if (printer)
	{
		m_current = printer;
		if (!printer->isSpecial())
		{
			if (printer->isLocal())
				m_actions->action((printer->state() == KMPrinter::Stopped ? "printer_start" : "printer_stop"))->plug(m_pop);
			m_actions->action((printer->acceptJobs() ? "printer_disable" : "printer_enable"))->plug(m_pop);
			m_pop->insertSeparator();
		}
		if (!printer->isSoftDefault()) m_actions->action("printer_soft_default")->plug(m_pop);
		if (printer->isLocal() && !printer->isImplicit())
		{
			if (!printer->isHardDefault()) m_actions->action("printer_hard_default")->plug(m_pop);
			m_actions->action("printer_remove")->plug(m_pop);
			m_pop->insertSeparator();
			if (!printer->isClass(true))
			{
				m_actions->action("printer_configure")->plug(m_pop);
				m_actions->action("printer_test")->plug(m_pop);
				m_actions->action("printer_tool")->plug(m_pop);
				m_pop->insertSeparator();
			}
		}
		else
		{
			m_actions->action("printer_remove")->plug(m_pop);
			m_pop->insertSeparator();
			if (!printer->isClass(true))
			{
				m_actions->action("printer_configure")->plug(m_pop);
				m_actions->action("printer_test")->plug(m_pop);
			}
			m_pop->insertSeparator();
		}
		if (!printer->isSpecial())
		{
			TQValueList<TDEAction*>	pactions = m_actions->actions("plugin");
			for (TQValueList<TDEAction*>::Iterator it=pactions.begin(); it!=pactions.end(); ++it)
				(*it)->plug(m_pop);
			if (pactions.count() > 0)
				m_pop->insertSeparator();
		}
	}
	else
	{
		m_actions->action("printer_add")->plug(m_pop);
		m_actions->action("printer_add_special")->plug(m_pop);
		m_pop->insertSeparator();
		m_actions->action("server_restart")->plug(m_pop);
		m_actions->action("server_configure")->plug(m_pop);
		m_pop->insertSeparator();
		m_actions->action("manager_configure")->plug(m_pop);
		m_actions->action("view_refresh")->plug(m_pop);
		m_pop->insertSeparator();
	}
	m_actions->action("view_printerinfos")->plug(m_pop);
	m_actions->action("view_change")->plug(m_pop);
	m_actions->action("orientation_change")->plug(m_pop);
	m_actions->action("view_toolbar")->plug(m_pop);
	m_actions->action("view_menubar")->plug(m_pop);
	m_pop->insertSeparator();
	m_actions->action("view_pfilter")->plug(m_pop);

	// pop the menu
	m_pop->popup(p);
}

void KMMainView::slotChangePrinterState()
{
	TQString	opname = sender()->name();
	if (m_current && opname.startsWith("printer_"))
	{
		opname = opname.mid(8);
		KMTimer::self()->hold();
		bool	result(false);
		if (opname == "enable")
			result = m_manager->enablePrinter(m_current, true);
		else if (opname == "disable")
			result = m_manager->enablePrinter(m_current, false);
		else if (opname == "start")
			result = m_manager->startPrinter(m_current, true);
		else if (opname == "stop")
			result = m_manager->startPrinter(m_current, false);
		if (!result)
			showErrorMsg(i18n("Unable to modify the state of printer %1.").arg(m_current->printerName()));
		KMTimer::self()->release(result);
	}
}

void KMMainView::slotRemove()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		bool	result(false);
		if (KMessageBox::warningYesNo(this,i18n("Do you really want to remove %1?").arg(m_current->printerName())) == KMessageBox::Yes)
			if (m_current->isSpecial())
			{
				if (!(result=m_manager->removeSpecialPrinter(m_current)))
					showErrorMsg(i18n("Unable to remove special printer %1.").arg(m_current->printerName()));
			}
			else if (!(result=m_manager->removePrinter(m_current)))
				showErrorMsg(i18n("Unable to remove printer %1.").arg(m_current->printerName()));
		KMTimer::self()->release(result);
	}
}

void KMMainView::slotConfigure()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		bool	needRefresh(false);
		if (m_current->isSpecial())
		{
			KMSpecialPrinterDlg	dlg(this);
			dlg.setPrinter(m_current);
			if (dlg.exec())
			{
				KMPrinter	*prt = dlg.printer();
				if (prt->name() != m_current->name())
					m_manager->removeSpecialPrinter(m_current);
				m_manager->createSpecialPrinter(prt);
				needRefresh = true;
			}
		}
		else
		{
			DrMain	*driver = m_manager->loadPrinterDriver(m_current, true);
			if (driver)
			{
				KMDriverDialog	dlg(this);
				dlg.setCaption(i18n("Configure %1").arg(m_current->printerName()));
				dlg.setDriver(driver);
				// disable OK button for remote printer (read-only dialog)
				if (m_current->isRemote())
					dlg.enableButtonOK(false);
				if (dlg.exec())
					if (!m_manager->savePrinterDriver(m_current,driver))
						showErrorMsg(i18n("Unable to modify settings of printer %1.").arg(m_current->printerName()));
				delete driver;
			}
			else
				showErrorMsg(i18n("Unable to load a valid driver for printer %1.").arg(m_current->printerName()));
		}
		KMTimer::self()->release(needRefresh);
	}
}

void KMMainView::slotAdd()
{
	KMTimer::self()->hold();

	int	result(0);
	if ((result=tdeprint_management_add_printer_wizard(this)) == -1)
		showErrorMsg(i18n("Unable to create printer."));

	KMTimer::self()->release((result == 1));
}

void KMMainView::slotHardDefault()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		bool	result = m_manager->setDefaultPrinter(m_current);
		if (!result)
			showErrorMsg(i18n("Unable to define printer %1 as default.").arg(m_current->printerName()));
		KMTimer::self()->release(result);
	}
}

void KMMainView::slotSoftDefault()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		KMFactory::self()->virtualManager()->setAsDefault(m_current,TQString::null);
		KMTimer::self()->release(true);
	}
}

void KMMainView::setOrientation(int o)
{
	int 	ID = (o == TQt::Horizontal ? 1 : 0);
	((TDESelectAction*)m_actions->action("orientation_change"))->setCurrentItem(ID);
	slotChangeDirection(ID);
}

int KMMainView::orientation() const
{ return (m_boxlayout->direction() == TQBoxLayout::LeftToRight ? TQt::Horizontal : TQt::Vertical);  }

void KMMainView::slotChangeDirection(int d)
{
	m_boxlayout->setDirection(d == 1 ? TQBoxLayout::LeftToRight : TQBoxLayout::TopToBottom);
}

void KMMainView::slotTest()
{
	if (m_current)
	{
		KMTimer::self()->hold();
		if (KMessageBox::warningContinueCancel(this, i18n("You are about to print a test page on %1. Do you want to continue?").arg(m_current->printerName()), TQString::null, i18n("Print Test Page"), "printTestPage") == KMessageBox::Continue)
		{
			if (KMFactory::self()->manager()->testPrinter(m_current))
				KMessageBox::information(this,i18n("Test page successfully sent to printer %1.").arg(m_current->printerName()));
			else
				showErrorMsg(i18n("Unable to test printer %1.").arg(m_current->printerName()));
		}
		KMTimer::self()->release(true);
	}
}

void KMMainView::showErrorMsg(const TQString& msg, bool usemgr)
{
	TQString	s(msg);
	if (usemgr)
	{
		s.prepend("<p>");
		s.append(" ");
		s += i18n("Error message received from manager:</p><p>%1</p>");
		if (m_manager->errorMsg().isEmpty())
			s = s.arg(i18n("Internal error (no error message)."));
		else
			s = s.arg(m_manager->errorMsg());
		// clean up error message
		m_manager->setErrorMsg(TQString::null);
	}
	s.prepend("<qt>").append("</qt>");
	KMTimer::self()->hold();
	KMessageBox::error(this,s);
	KMTimer::self()->release();
}

void KMMainView::slotServerRestart()
{
	KMTimer::self()->hold();
	bool	result = m_manager->restartServer();
	if (!result)
	{
		showErrorMsg(i18n("Unable to restart print server."));
		KMTimer::self()->release( false );
	}
	else
	{
		reset( i18n( "Restarting server..." ), false, false );
	}
}

void KMMainView::slotServerConfigure()
{
	KMTimer::self()->hold();
	bool	result = m_manager->configureServer(this);
	if (!result)
	{
		showErrorMsg(i18n("Unable to configure print server."));
		KMTimer::self()->release( false );
	}
	else
	{
		reset( i18n( "Configuring server..." ), false, false );
	}
}

void KMMainView::slotServerConfigureAccess()
{
	TDEProcess *proc = new TDEProcess;
	*proc << "/usr/bin/system-config-printer-kde";
	proc->start(TDEProcess::DontCare);
}

void KMMainView::slotToggleToolBar(bool on)
{
	if (on) m_toolbar->show();
	else m_toolbar->hide();
}

void KMMainView::slotToggleMenuBar( bool on )
{
	if ( on )
		m_menubar->show();
	else
		m_menubar->hide();
}

void KMMainView::slotManagerConfigure()
{
	KMTimer::self()->hold();
	KMConfigDialog	dlg(this,"ConfigDialog");
	if ( dlg.exec() )
	{
		loadParameters();
	}
	/* when "OK":
	 *  => the config file is saved
	 *  => triggering a DCOP signal
	 *  => configChanged() called
	 * hence no need to refresh, just release the timer
	 */
	KMTimer::self()->release( false );
}

void KMMainView::slotAddSpecial()
{
	KMTimer::self()->hold();
	KMSpecialPrinterDlg	dlg(this);
	if (dlg.exec())
	{
		KMPrinter	*prt = dlg.printer();
		m_manager->createSpecialPrinter(prt);
	}
	KMTimer::self()->release(true);
}

void KMMainView::slotShowPrinterInfos(bool on)
{
	if (on)
		m_printerpages->show();
	else
		m_printerpages->hide();
	m_actions->action("orientation_change")->setEnabled(on);
}

void KMMainView::enableToolbar(bool on)
{
	TDEToggleAction	*act = (TDEToggleAction*)m_actions->action("view_toolbar");
	m_toolbar->setEnabled(on);
	act->setEnabled(on);
	if (on && act->isChecked())
		m_toolbar->show();
	else
		m_toolbar->hide();
}

TDEAction* KMMainView::action(const char *name)
{
	return m_actions->action(name);
}

/*
void KMMainView::aboutToReload()
{
	m_printerview->setPrinterList(0);
}
*/

void KMMainView::reload()
{
	removePluginActions();
	loadPluginActions();

	// redo the connection as the old manager object has been removed
	connect( m_manager, TQ_SIGNAL( updatePossible( bool ) ), TQ_SLOT( slotUpdatePossible( bool ) ) );

	// We must delay the refresh such that all objects has been
	// correctly reloaded (otherwise, crash in KMJobViewer).
	reset( i18n( "Initializing manager..." ), true, true );
}

void KMMainView::showPrinterInfos(bool on)
{
	static_cast<TDEToggleAction*>(m_actions->action("view_printerinfos"))->setChecked(on);
	slotShowPrinterInfos(on);
}

bool KMMainView::printerInfosShown() const
{
	return (static_cast<TDEToggleAction*>(m_actions->action("view_printerinfos"))->isChecked());
}

void KMMainView::loadPluginActions()
{
	KMFactory::self()->manager()->createPluginActions(m_actions);
	TQValueList<TDEAction*>	pactions = m_actions->actions("plugin");
	int	index = m_pactionsindex;
	//TQPopupMenu *menu = m_menubar->findItem( m_menubar->idAt( 1 ) )->popup();
	TQPopupMenu *menu = m_menubar->getButton( 1 )->popup();
	for (TQValueList<TDEAction*>::Iterator it=pactions.begin(); it!=pactions.end(); ++it)
	{
		(*it)->plug(m_toolbar, index++);
		( *it )->plug( menu );
	}
}

void KMMainView::removePluginActions()
{
	TQValueList<TDEAction*>	pactions = m_actions->actions("plugin");
	for (TQValueList<TDEAction*>::Iterator it=pactions.begin(); it!=pactions.end(); ++it)
	{
		(*it)->unplugAll();
		delete (*it);
	}
}

void KMMainView::slotToolSelected(int ID)
{
	KMTimer::self()->hold();

	TQString	libname = m_toollist[ID];
	libname.prepend("tdeprint_tool_");
	if (m_current && !m_current->device().isEmpty() && !libname.isEmpty())
	{
		KLibFactory	*factory = KLibLoader::self()->factory(libname.local8Bit());
		if (factory)
		{
			TQStringList	args;
			args << m_current->device() << m_current->printerName();
			KDialogBase	*dlg = static_cast<KDialogBase*>(factory->create(this, "Tool", 0, args));
			if (dlg)
				dlg->exec();
			delete dlg;
		}
	}
	else
		KMessageBox::error(this,
			i18n("Unable to start printer tool. Possible reasons are: "
			     "no printer selected, the selected printer doesn't have "
			     "any local device defined (printer port), or the tool library "
			     "could not be found."));

	KMTimer::self()->release();
}

void KMMainView::slotToggleFilter(bool on)
{
	KMTimer::self()->hold();
	KMManager::self()->enableFilter(on);
	KMTimer::self()->release(true);
}

void KMMainView::configChanged()
{
	reset( i18n( "Initializing manager..." ), false, true );
}

void KMMainView::slotUpdatePossible( bool flag )
{
	destroyMessageWindow();
	if ( !flag )
		showErrorMsg( i18n( "Unable to retrieve the printer list." ) );
	KMTimer::self()->release( true );
}

void KMMainView::createMessageWindow( const TQString& txt, int delay )
{
	destroyMessageWindow();
	MessageWindow::add( m_printerview, txt, delay );
}

void KMMainView::destroyMessageWindow()
{
	MessageWindow::remove( m_printerview );
}

void KMMainView::slotInit()
{
	reset( i18n( "Initializing manager..." ), true, true );
}

void KMMainView::reset( const TQString& msg, bool useDelay, bool holdTimer )
{
	if ( holdTimer )
		KMTimer::self()->hold();
	m_printerview->setPrinterList( 0 );
	if ( !msg.isEmpty() )
		createMessageWindow( msg, ( useDelay ? 500 : 0 ) );
	m_first = true;
	m_manager->checkUpdatePossible();
}

#include "kmmainview.moc"
