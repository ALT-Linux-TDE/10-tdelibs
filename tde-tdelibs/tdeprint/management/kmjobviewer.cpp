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

#include "kmjobviewer.h"
#include "kmjobmanager.h"
#include "kmfactory.h"
#include "kmjob.h"
#include "kmprinter.h"
#include "kmmanager.h"
#include "kmuimanager.h"
#include "jobitem.h"
#include "kmtimer.h"
#include "kmconfigjobs.h"
#include "kmconfigpage.h"
#include "kprinter.h"

#include <tdelistview.h>
#include <kstatusbar.h>
#include <tqpopupmenu.h>
#include <tdemessagebox.h>
#include <tdelocale.h>
#include <tdepopupmenu.h>
#include <tdeaction.h>
#include <kstdaction.h>
#include <kiconloader.h>
#include <tdeapplication.h>
#include <kcursor.h>
#include <tdemenubar.h>
#include <kdebug.h>
#include <twin.h>
#include <tdeio/netaccess.h>
#include <tqtimer.h>
#include <tqlayout.h>
#include <stdlib.h>
#include <tqlineedit.h>
#include <kdialogbase.h>
#include <tqcheckbox.h>
#include <kurldrag.h>
#include <tdeconfig.h>

#undef m_manager
#define	m_manager	KMFactory::self()->jobManager()

class KJobListView : public TDEListView
{
public:
	KJobListView( TQWidget *parent = 0, const char *name = 0 );

protected:
	bool acceptDrag( TQDropEvent* ) const;
};

KJobListView::KJobListView( TQWidget *parent, const char *name )
	: TDEListView( parent, name )
{
	setAcceptDrops( true );
	setDropVisualizer( false );
}

bool KJobListView::acceptDrag( TQDropEvent *e ) const
{
	if ( KURLDrag::canDecode( e ) )
		return true;
	else
		return TDEListView::acceptDrag( e );
}

KMJobViewer::KMJobViewer(TQWidget *parent, const char *name)
: TDEMainWindow(parent,name)
{
	m_view = 0;
	m_pop = 0;
	m_jobs.setAutoDelete(false);
	m_items.setAutoDelete(false);
	m_printers.setAutoDelete(false);
	m_type = KMJobManager::ActiveJobs;
	m_stickybox = 0;
	m_standalone = ( parent == NULL );

	setToolBarsMovable(false);
	init();

	if (m_standalone)
	{
		setCaption(i18n("No Printer"));
		TDEConfig *conf = KMFactory::self()->printConfig();
		TQSize defSize( 550, 250 );
		conf->setGroup( "Jobs" );
		resize( conf->readSizeEntry( "Size", &defSize ) );
	}

	connect(KMFactory::self()->manager(), TQ_SIGNAL(printerListUpdated()),TQ_SLOT(slotPrinterListUpdated()));
}

KMJobViewer::~KMJobViewer()
{
	if (m_standalone)
	{
		kdDebug( 500 ) << "Destroying stand-alone job viewer window" << endl;
		TDEConfig *conf = KMFactory::self()->printConfig();
		conf->setGroup( "Jobs" );
		conf->writeEntry( "Size", size() );
		emit viewerDestroyed(this);
	}
	removeFromManager();
}

void KMJobViewer::setPrinter(KMPrinter *p)
{
	setPrinter((p ? p->printerName() : TQString::null));
}

void KMJobViewer::setPrinter(const TQString& prname)
{
	// We need to trigger a refresh even if the printer
	// has not changed, some jobs may have been canceled
	// outside tdeprint. We can't return simply if
	// prname == m_prname.
	if (m_prname != prname)
	{
		removeFromManager();
		m_prname = prname;
		addToManager();
		m_view->setAcceptDrops( prname != i18n( "All Printers" ) );
	}
	triggerRefresh();
}

void KMJobViewer::updateCaption()
{
	if (!m_standalone)
		return;

	TQString	pixname("document-print");
	if (!m_prname.isEmpty())
	{
		setCaption(i18n("Print Jobs for %1").arg(m_prname));
		KMPrinter	*prt = KMManager::self()->findPrinter(m_prname);
		if (prt)
			pixname = prt->pixmap();
	}
	else
	{
		setCaption(i18n("No Printer"));
	}
	KWin::setIcons(winId(), DesktopIcon(pixname), SmallIcon(pixname));
}

void KMJobViewer::updateStatusBar()
{
	if (!m_standalone)
		return;

	int	limit = m_manager->limit();
	if (limit == 0)
		statusBar()->changeItem(i18n("Max.: %1").arg(i18n("Unlimited")), 0);
	else
		statusBar()->changeItem(i18n("Max.: %1").arg(limit), 0);
}

void KMJobViewer::addToManager()
{
	if (m_prname == i18n("All Printers"))
	{
		loadPrinters();
		TQPtrListIterator<KMPrinter>	it(m_printers);
		for (; it.current(); ++it)
			m_manager->addPrinter(it.current()->printerName(), (KMJobManager::JobType)m_type, it.current()->isSpecial());
	}
	else if (!m_prname.isEmpty())
	{
		KMPrinter *prt = KMManager::self()->findPrinter( m_prname );
		bool isSpecial = ( prt ? prt->isSpecial() : false );
		m_manager->addPrinter(m_prname, (KMJobManager::JobType)m_type, isSpecial);
	}
}

void KMJobViewer::removeFromManager()
{
	if (m_prname == i18n("All Printers"))
	{
		TQPtrListIterator<KMPrinter>	it(m_printers);
		for (; it.current(); ++it)
			m_manager->removePrinter(it.current()->printerName(), (KMJobManager::JobType)m_type);
	}
	else if (!m_prname.isEmpty())
	{
		m_manager->removePrinter(m_prname, (KMJobManager::JobType)m_type);
	}
}

void KMJobViewer::refresh(bool reload)
{
	m_jobs.clear();
	TQPtrListIterator<KMJob>	it(m_manager->jobList(reload));
	bool	all = (m_prname == i18n("All Printers")), active = (m_type == KMJobManager::ActiveJobs);
	for (; it.current(); ++it)
		if ((all || it.current()->printer() == m_prname)
		    && ((it.current()->state() >= KMJob::Cancelled && !active)
			    || (it.current()->state() < KMJob::Cancelled && active))
		    && (m_username.isEmpty() || m_username == it.current()->owner()))
			m_jobs.append(it.current());
	updateJobs();


	// update the caption and icon (doesn't do anything if it has a parent widget)
	updateCaption();

	updateStatusBar();

	// do it last as this signal can cause this view to be destroyed. No
	// code can be executed safely after that
	emit jobsShown(this, (m_jobs.count() != 0));
}

void KMJobViewer::init()
{
	if (!m_view)
	{
		m_view = new KJobListView(this);
		m_view->addColumn(i18n("Job ID"));
		m_view->addColumn(i18n("Owner"));
		m_view->addColumn(i18n("Name"), 150);
		m_view->addColumn(i18n("Status", "State"));
		m_view->addColumn(i18n("Size (KB)"));
		m_view->addColumn(i18n("Page(s)"));
		m_view->setColumnAlignment(5,TQt::AlignRight|TQt::AlignVCenter);
		connect( m_view, TQ_SIGNAL( dropped( TQDropEvent*, TQListViewItem* ) ), TQ_SLOT( slotDropped( TQDropEvent*, TQListViewItem* ) ) );
		//m_view->addColumn(i18n("Printer"));
		//m_view->setColumnAlignment(6,TQt::AlignRight|TQt::AlignVCenter);
		KMFactory::self()->uiManager()->setupJobViewer(m_view);
		m_view->setFrameStyle(TQFrame::WinPanel|TQFrame::Sunken);
		m_view->setLineWidth(1);
		m_view->setSorting(0);
		m_view->setAllColumnsShowFocus(true);
		m_view->setSelectionMode(TQListView::Extended);
		connect(m_view,TQ_SIGNAL(selectionChanged()),TQ_SLOT(slotSelectionChanged()));
		connect(m_view,TQ_SIGNAL(rightButtonPressed(TQListViewItem*,const TQPoint&,int)),TQ_SLOT(slotRightClicked(TQListViewItem*,const TQPoint&,int)));
		setCentralWidget(m_view);
	}

	initActions();
}

void KMJobViewer::initActions()
{
	// job actions
	TDEAction	*hact = new TDEAction(i18n("&Hold"),"process-stop",0,this,TQ_SLOT(slotHold()),actionCollection(),"job_hold");
	TDEAction	*ract = new TDEAction(i18n("&Resume"),"system-run",0,this,TQ_SLOT(slotResume()),actionCollection(),"job_resume");
	TDEAction	*dact = new TDEAction(i18n("Remo&ve"),"edittrash",TQt::Key_Delete,this,TQ_SLOT(slotRemove()),actionCollection(),"job_remove");
	TDEAction *sact = new TDEAction(i18n("Res&tart"),"edit-redo",0,this,TQ_SLOT(slotRestart()),actionCollection(),"job_restart");
	TDEActionMenu *mact = new TDEActionMenu(i18n("&Move to Printer"),"document-print",actionCollection(),"job_move");
	mact->setDelayed(false);
	connect(mact->popupMenu(),TQ_SIGNAL(activated(int)),TQ_SLOT(slotMove(int)));
	connect(mact->popupMenu(),TQ_SIGNAL(aboutToShow()),KMTimer::self(),TQ_SLOT(hold()));
	connect(mact->popupMenu(),TQ_SIGNAL(aboutToHide()),KMTimer::self(),TQ_SLOT(release()));
	connect(mact->popupMenu(),TQ_SIGNAL(aboutToShow()),TQ_SLOT(slotShowMoveMenu()));
	TDEToggleAction	*tact = new TDEToggleAction(i18n("&Toggle Completed Jobs"),"history",0,actionCollection(),"view_completed");
	tact->setEnabled(m_manager->actions() & KMJob::ShowCompleted);
	connect(tact,TQ_SIGNAL(toggled(bool)),TQ_SLOT(slotShowCompleted(bool)));
	TDEToggleAction	*uact = new TDEToggleAction(i18n("Show Only User Jobs"), "preferences-desktop-personal", 0, actionCollection(), "view_user_jobs");
	uact->setCheckedState(KGuiItem(i18n("Hide Only User Jobs"),"preferences-desktop-personal"));
	connect(uact, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotUserOnly(bool)));
	m_userfield = new TQLineEdit(0);
	m_userfield->setText(getenv("USER"));
	connect(m_userfield, TQ_SIGNAL(returnPressed()), TQ_SLOT(slotUserChanged()));
	connect(uact, TQ_SIGNAL(toggled(bool)), m_userfield, TQ_SLOT(setEnabled(bool)));
	m_userfield->setEnabled(false);
	m_userfield->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed, TQSizePolicy::Fixed));
	KWidgetAction	*ufact = new KWidgetAction(m_userfield, i18n("User Name"), 0, 0, 0, actionCollection(), "view_username");

	if (!m_pop)
	{
		m_pop = new TQPopupMenu(this);
		connect(m_pop,TQ_SIGNAL(aboutToShow()),KMTimer::self(),TQ_SLOT(hold()));
		connect(m_pop,TQ_SIGNAL(aboutToHide()),KMTimer::self(),TQ_SLOT(release()));
		hact->plug(m_pop);
		ract->plug(m_pop);
		m_pop->insertSeparator();
		dact->plug(m_pop);
		mact->plug(m_pop);
		m_pop->insertSeparator();
		sact->plug(m_pop);
	}

	// Filter actions
	TDEActionMenu	*fact = new TDEActionMenu(i18n("&Select Printer"), "tdeprint_printer", actionCollection(), "filter_modify");
	fact->setDelayed(false);
	connect(fact->popupMenu(),TQ_SIGNAL(activated(int)),TQ_SLOT(slotPrinterSelected(int)));
	connect(fact->popupMenu(),TQ_SIGNAL(aboutToShow()),KMTimer::self(),TQ_SLOT(hold()));
	connect(fact->popupMenu(),TQ_SIGNAL(aboutToHide()),KMTimer::self(),TQ_SLOT(release()));
	connect(fact->popupMenu(),TQ_SIGNAL(aboutToShow()),TQ_SLOT(slotShowPrinterMenu()));

	if (!m_standalone)
	{
		TDEToolBar	*toolbar = toolBar();
		hact->plug(toolbar);
		ract->plug(toolbar);
		toolbar->insertSeparator();
		dact->plug(toolbar);
		mact->plug(toolbar);
		toolbar->insertSeparator();
		sact->plug(toolbar);
		toolbar->insertSeparator();
		tact->plug(toolbar);
		uact->plug(toolbar);
		ufact->plug(toolbar);
	}
	else
	{// stand-alone application
		KStdAction::quit(kapp,TQ_SLOT(quit()),actionCollection());
		KStdAction::close(this,TQ_SLOT(slotClose()),actionCollection());
		KStdAction::preferences(this, TQ_SLOT(slotConfigure()), actionCollection());

		// refresh action
		new TDEAction(i18n("Refresh"),"reload",0,this,TQ_SLOT(slotRefresh()),actionCollection(),"refresh");

		// create status bar
		KStatusBar	*statusbar = statusBar();
		m_stickybox = new TQCheckBox( i18n( "Keep window permanent" ), statusbar );

		TDEConfig *conf = KMFactory::self()->printConfig();
		conf->setGroup("Jobs");
		m_stickybox->setChecked(conf->readBoolEntry("KeepWindow",false));
		connect(m_stickybox, TQ_SIGNAL(toggled(bool)), TQ_SLOT(slotKeepWindowChange(bool)));
		statusbar->addWidget( m_stickybox, 1, false );
		statusbar->insertItem(" " + i18n("Max.: %1").arg(i18n("Unlimited"))+ " ", 0, 0, true);
		statusbar->setItemFixed(0);
		updateStatusBar();

		createGUI();
	}

	loadPluginActions();
	slotSelectionChanged();
}

void KMJobViewer::buildPrinterMenu(TQPopupMenu *menu, bool use_all, bool use_specials)
{
	loadPrinters();
	menu->clear();

	TQPtrListIterator<KMPrinter>	it(m_printers);
	int	i(0);
	if (use_all)
	{
		menu->insertItem(SmallIcon("document-print"), i18n("All Printers"), i++);
		menu->insertSeparator();
	}
	for (; it.current(); ++it, i++)
	{
		if ( !it.current()->instanceName().isEmpty() ||
				( it.current()->isSpecial() && !use_specials ) )
			continue;
		menu->insertItem(SmallIcon(it.current()->pixmap()), it.current()->printerName(), i);
	}
}

void KMJobViewer::slotKeepWindowChange( bool val )
{
    TDEConfig *conf = KMFactory::self()->printConfig();
    conf->setGroup("Jobs");
    conf->writeEntry("KeepWindow",val);
}

void KMJobViewer::slotShowMoveMenu()
{
	TQPopupMenu	*menu = static_cast<TDEActionMenu*>(actionCollection()->action("job_move"))->popupMenu();
	buildPrinterMenu(menu, false, false);
}

void KMJobViewer::slotShowPrinterMenu()
{
	TQPopupMenu	*menu = static_cast<TDEActionMenu*>(actionCollection()->action("filter_modify"))->popupMenu();
	buildPrinterMenu(menu, true, true);
}

void KMJobViewer::updateJobs()
{
	TQPtrListIterator<JobItem>	jit(m_items);
	for (;jit.current();++jit)
		jit.current()->setDiscarded(true);

	TQPtrListIterator<KMJob>	it(m_jobs);
	for (;it.current();++it)
	{
		KMJob	*j(it.current());
		JobItem	*item = findItem(j->uri());
		if (item)
		{
			item->setDiscarded(false);
			item->init(j);
		}
		else
			m_items.append(new JobItem(m_view,j));
	}

	for (uint i=0; i<m_items.count(); i++)
		if (m_items.at(i)->isDiscarded())
		{
			delete m_items.take(i);
			i--;
		}

	slotSelectionChanged();
}

JobItem* KMJobViewer::findItem(const TQString& uri)
{
	TQPtrListIterator<JobItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->jobUri() == uri) return it.current();
	return 0;
}

void KMJobViewer::slotSelectionChanged()
{
	int	acts = m_manager->actions();
	int	state(-1);
	int	thread(0);
	bool	completed(true), remote(false);

	TQPtrListIterator<JobItem>	it(m_items);
	TQPtrList<KMJob>	joblist;

	joblist.setAutoDelete(false);
	for (;it.current();++it)
	{
		if (it.current()->isSelected())
		{
			// check if threaded job. "thread" value will be:
			//	0 -> no jobs
			//	1 -> only thread jobs
			//	2 -> only system jobs
			//	3 -> thread and system jobs
			if (it.current()->job()->type() == KMJob::Threaded) thread |= 0x1;
			else thread |= 0x2;

			if (state == -1) state = it.current()->job()->state();
			else if (state != 0 && state != it.current()->job()->state()) state = 0;

			completed = (completed && it.current()->job()->isCompleted());
			joblist.append(it.current()->job());
			if (it.current()->job()->isRemote())
				remote = true;
		}
	}
	if (thread != 2)
		joblist.clear();

	actionCollection()->action("job_remove")->setEnabled((thread == 1) || (/*!remote &&*/ !completed && (state >= 0) && (acts & KMJob::Remove)));
	actionCollection()->action("job_hold")->setEnabled(/*!remote &&*/ !completed && (thread == 2) && (state > 0) && (state != KMJob::Held) && (acts & KMJob::Hold));
	actionCollection()->action("job_resume")->setEnabled(/*!remote &&*/ !completed && (thread == 2) && (state > 0) && (state == KMJob::Held) && (acts & KMJob::Resume));
	actionCollection()->action("job_move")->setEnabled(!remote && !completed && (thread == 2) && (state >= 0) && (acts & KMJob::Move));
	actionCollection()->action("job_restart")->setEnabled(!remote && (thread == 2) && (state >= 0) && (completed) && (acts & KMJob::Restart));

	m_manager->validatePluginActions(actionCollection(), joblist);
}

void KMJobViewer::jobSelection(TQPtrList<KMJob>& l)
{
	l.setAutoDelete(false);
	TQPtrListIterator<JobItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->isSelected())
			l.append(it.current()->job());
}

void KMJobViewer::send(int cmd, const TQString& name, const TQString& arg)
{
	KMTimer::self()->hold();

	TQPtrList<KMJob>	l;
	jobSelection(l);
	if (!m_manager->sendCommand(l,cmd,arg))
	{
		KMessageBox::error(this,"<qt>"+i18n("Unable to perform action \"%1\" on selected jobs. Error received from manager:").arg(name)+"<p>"+KMManager::self()->errorMsg()+"</p></qt>");
		// error reported, clean it
		KMManager::self()->setErrorMsg(TQString::null);
	}

	triggerRefresh();

	KMTimer::self()->release();
}

void KMJobViewer::slotHold()
{
	send(KMJob::Hold,i18n("Hold"));
}

void KMJobViewer::slotResume()
{
	send(KMJob::Resume,i18n("Resume"));
}

void KMJobViewer::slotRemove()
{
	send(KMJob::Remove,i18n("Remove"));
}

void KMJobViewer::slotRestart()
{
	send(KMJob::Restart,i18n("Restart"));
}

void KMJobViewer::slotMove(int prID)
{
	if (prID >= 0 && prID < (int)(m_printers.count()))
	{
		KMPrinter	*p = m_printers.at(prID);
		send(KMJob::Move,i18n("Move to %1").arg(p->printerName()),p->printerName());
	}
}

void KMJobViewer::slotRightClicked(TQListViewItem*,const TQPoint& p,int)
{
	if (m_pop) m_pop->popup(p);
}

void KMJobViewer::slotPrinterListUpdated()
{
	loadPrinters();
}

void KMJobViewer::loadPrinters()
{
	m_printers.clear();

	// retrieve printer list without reloading it (faster)
	TQPtrListIterator<KMPrinter>	it(*(KMFactory::self()->manager()->printerList(false)));
	for (;it.current();++it)
	{
		// keep only real printers (no instance, no implicit) and special printers
		if ((it.current()->isPrinter() || it.current()->isClass(false) ||
					( it.current()->isSpecial() && it.current()->isValid() ) )
				&& (it.current()->name() == it.current()->printerName()))
			m_printers.append(it.current());
	}
}

void KMJobViewer::slotPrinterSelected(int prID)
{
	if (prID >= 0 && prID < (int)(m_printers.count()+1))
	{
		TQString	prname = (prID == 0 ? i18n("All Printers") : m_printers.at(prID-1)->printerName());
		emit printerChanged(this, prname);
	}
}

void KMJobViewer::slotRefresh()
{
	triggerRefresh();
}

void KMJobViewer::triggerRefresh()
{
	// parent widget -> embedded in KControl and needs
	// to update itself. Otherwise, it's standalone
	// kjobviewer and we need to synchronize all possible
	// opened windows -> do the job on higher level.
	if (!m_standalone)
		refresh(true);
	else
		emit refreshClicked();
}

void KMJobViewer::slotShowCompleted(bool on)
{
	removeFromManager();
	m_type = (on ? KMJobManager::CompletedJobs : KMJobManager::ActiveJobs);
	addToManager();
	triggerRefresh();
}

void KMJobViewer::slotClose()
{
	delete this;
}

void KMJobViewer::loadPluginActions()
{
	int	mpopindex(7), toolbarindex(!m_standalone?7:8), menuindex(7);
	TQMenuData	*menu(0);

	if (m_standalone)
	{
		// standalone window, insert actions into main menubar
		TDEAction	*act = actionCollection()->action("job_restart");
		for (int i=0;i<act->containerCount();i++)
		{
			if (menuBar()->findItem(act->itemId(i), &menu))
			{
				menuindex = mpopindex = menu->indexOf(act->itemId(i))+1;
				break;
			}
		}
	}

	TQValueList<TDEAction*>	acts = m_manager->createPluginActions(actionCollection());
	for (TQValueListIterator<TDEAction*> it=acts.begin(); it!=acts.end(); ++it)
	{
		// connect the action to this
		connect((*it), TQ_SIGNAL(activated(int)), TQ_SLOT(pluginActionActivated(int)));

		// should add it to the toolbar and menubar
		(*it)->plug(toolBar(), toolbarindex++);
		if (m_pop)
			(*it)->plug(m_pop, mpopindex++);
		if (menu)
			(*it)->plug(static_cast<TQPopupMenu*>(menu), menuindex++);
	}
}

void KMJobViewer::removePluginActions()
{
	TQValueList<TDEAction*>	acts = actionCollection()->actions("plugin");
	for (TQValueListIterator<TDEAction*> it=acts.begin(); it!=acts.end(); ++it)
	{
		(*it)->unplugAll();
		delete (*it);
	}
}

/*
void KMJobViewer::aboutToReload()
{
	if (m_view)
	{
		m_view->clear();
		m_items.clear();
	}
	m_jobs.clear();
}
*/

void KMJobViewer::reload()
{
	removePluginActions();
	loadPluginActions();
	// re-add the current printer to the job manager: the job
	// manager has been destroyed, so the new one doesn't know
	// which printer it has to list
	addToManager();
	// no refresh needed: view has been cleared before reloading
	// and the actual refresh will be triggered either by the KControl
	// module, or by KJobViewerApp using timer.

	// reload the columns needed: remove the old one
	for (int c=m_view->columns()-1; c>5; c--)
		m_view->removeColumn(c);
	KMFactory::self()->uiManager()->setupJobViewer(m_view);

	// update the "History" action state
	actionCollection()->action("view_completed")->setEnabled(m_manager->actions() & KMJob::ShowCompleted);
	static_cast<TDEToggleAction*>(actionCollection()->action("view_completed"))->setChecked(false);
}

void KMJobViewer::closeEvent(TQCloseEvent *e)
{
	if (m_standalone && !kapp->sessionSaving())
	{
		hide();
		e->ignore();
	}
	else
		e->accept();
}

void KMJobViewer::pluginActionActivated(int ID)
{
	KMTimer::self()->hold();

	TQPtrList<KMJob>	joblist;
	jobSelection(joblist);
	if (!m_manager->doPluginAction(ID, joblist))
		KMessageBox::error(this, "<qt>"+i18n("Operation failed.")+"<p>"+KMManager::self()->errorMsg()+"</p></qt>");

	triggerRefresh();
	KMTimer::self()->release();
}

void KMJobViewer::slotUserOnly(bool on)
{
	m_username = (on ? m_userfield->text() : TQString::null);
	refresh(false);
}

void KMJobViewer::slotUserChanged()
{
	if (m_userfield->isEnabled())
	{
		m_username = m_userfield->text();
		refresh(false);
	}
}

void KMJobViewer::slotConfigure()
{
	KMTimer::self()->hold();

	KDialogBase	dlg(this, 0, true, i18n("Print Job Settings"), KDialogBase::Ok|KDialogBase::Cancel);
	KMConfigJobs	*w = new KMConfigJobs(&dlg);
	dlg.setMainWidget(w);
	dlg.resize(300, 10);
	TDEConfig	*conf = KMFactory::self()->printConfig();
	w->loadConfig(conf);
	if (dlg.exec())
	{
		w->saveConfig(conf);
		updateStatusBar();
		refresh(true);
	}

	KMTimer::self()->release();
}

bool KMJobViewer::isSticky() const
{
	return ( m_stickybox ? m_stickybox->isChecked() : false );
}

void KMJobViewer::slotDropped( TQDropEvent *e, TQListViewItem* )
{
	TQStringList files;
	TQString target;

        KURL::List uris;
	KURLDrag::decode( e, uris );
	for ( KURL::List::ConstIterator it = uris.begin();
	      it != uris.end(); ++it)
	{
		if ( TDEIO::NetAccess::download( *it, target, 0 ) )
			files << target;
	}

	if ( files.count() > 0 )
	{
		KPrinter prt;
		if ( prt.autoConfigure( m_prname, this ) )
			prt.printFiles( files, false, false );
	}
}

#include "kmjobviewer.moc"
