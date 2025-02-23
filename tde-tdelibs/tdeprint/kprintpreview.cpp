/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
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

#include "kprintpreview.h"
#include "kmfactory.h"

#include <tqfile.h>
#include <tqlayout.h>
#include <tqvbox.h>

#include <tdeparts/part.h>
#include <tdeaccel.h>
#include <tdeaction.h>
#include <klibloader.h>
#include <ktrader.h>
#include <kuserprofile.h>
#include <krun.h>
#include <tdeapplication.h>
#include <kstandarddirs.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kdebug.h>
#include <tdeconfig.h>
#include <tdetoolbar.h>
#include <kmimetype.h>

KPreviewProc::KPreviewProc()
: TDEProcess()
{
	m_bOk = false;
	connect(this, TQ_SIGNAL(processExited(TDEProcess*)), TQ_SLOT(slotProcessExited(TDEProcess*)));
}

KPreviewProc::~KPreviewProc()
{
}

bool KPreviewProc::startPreview()
{
	if (start())
	{
		kapp->enter_loop();
		return m_bOk;
	}
	else
		return false;
}

void KPreviewProc::slotProcessExited(TDEProcess* proc)
{
	kapp->exit_loop();
	if ( proc->normalExit() && proc->exitStatus() == 0 )
		m_bOk = true;
	else
		kdDebug(500) << "KPreviewProc::slotProcessExited: normalExit=" << proc->normalExit()
			<< " exitStatus=" << proc->exitStatus() << endl;
}

//*******************************************************************************************

class KPrintPreview::KPrintPreviewPrivate
{
public:
	KPrintPreviewPrivate(KPrintPreview *dlg) : gvpart_(0)
	{
		mainwidget_ = new TQWidget(dlg, "MainWidget");
		toolbar_ = new TDEToolBar(mainwidget_, "PreviewToolBar", true);
		actions_ = new TDEActionCollection(dlg);
		accel_ = new TDEAccel(dlg);
		previewonly_ = false;
	}
	~KPrintPreviewPrivate()
	{
		if (gvpart_) delete gvpart_;
	}
	void plugAction(TDEAction *act)
	{
		act->plug(toolbar_);
		act->plugAccel(accel_);
	}

	KParts::ReadOnlyPart	*gvpart_;
	TDEToolBar		*toolbar_;
	TDEActionCollection	*actions_;
	TQWidget		*mainwidget_;
	TDEAccel			*accel_;
	bool			previewonly_;
};

static KLibFactory* componentFactory()
{
	kdDebug(500) << "tdeprint: querying trader for 'application/postscript' service" << endl;
	KLibFactory	*factory(0);
	factory = KLibLoader::self()->factory("libkghostviewpart");
        if( factory )
            return factory;
	TDETrader::OfferList	offers = TDETrader::self()->query(TQString::fromLatin1("application/postscript"), TQString::fromLatin1("KParts/ReadOnlyPart"), TQString::null, TQString::null);
	for (TDETrader::OfferList::ConstIterator it = offers.begin(); it != offers.end(); ++it)
	{
		KService::Ptr	service = *it;
		factory = KLibLoader::self()->factory(TQFile::encodeName(service->library()));
		if (factory)
			break;
	}
	if (!factory)
	{
		// nothing has been found, try to load directly the KGhostview part
		factory = KLibLoader::self()->factory("libkghostviewpart");
	}
	return factory;
}

static bool continuePrint(const TQString& msg_, TQWidget *parent, bool previewOnly)
{
	TQString	msg(msg_);
	if (previewOnly)
	{
		KMessageBox::error(parent, msg);
		return false;
	}
	else
	{
		msg.append(" ").append(i18n("Do you want to continue printing anyway?"));
		return (KMessageBox::warningContinueCancel(parent, msg, TQString::null, KGuiItem(i18n("Print"),"document-print")) == KMessageBox::Continue);
	}
}

//*******************************************************************************************

KPrintPreview::KPrintPreview(TQWidget *parent, bool previewOnly)
: KDialogBase(parent, "PreviewDlg", true, i18n("Print Preview"), 0)
{
	kdDebug(500) << "tdeprint: creating preview dialog" << endl;
	d = new KPrintPreviewPrivate(this);
	d->previewonly_ = previewOnly;

	// create main view and actions
	setMainWidget(d->mainwidget_);
	if (previewOnly)
		KStdAction::close(this, TQ_SLOT(reject()), d->actions_, "close_print");
	else
	{
		new TDEAction(i18n("Print"), "document-print", TQt::Key_Return, this, TQ_SLOT(accept()), d->actions_, "continue_print");
		new TDEAction(i18n("Cancel"), "process-stop", TQt::Key_Escape, this, TQ_SLOT(reject()), d->actions_, "stop_print");
	}

}

KPrintPreview::~KPrintPreview()
{
	delete d;
}

void KPrintPreview::initView(KLibFactory *factory)
{
	// load the component
	d->gvpart_ = (KParts::ReadOnlyPart*)factory->create(d->mainwidget_, "gvpart", "KParts::ReadOnlyPart");

	// populate the toolbar
	if (d->previewonly_)
		d->plugAction(d->actions_->action("close_print"));
	else
	{
		d->plugAction(d->actions_->action("continue_print"));
		d->plugAction(d->actions_->action("stop_print"));
	}
	if (d->gvpart_)
	{
		TQDomNodeList l = d->gvpart_->domDocument().elementsByTagName( "ToolBar" );
		if ( l.length() > 0 )
		{
			d->toolbar_->insertLineSeparator();
			TQDomNodeList acts = l.item( 0 ).toElement().elementsByTagName( "Action" );
			for ( uint i=0; i<acts.length(); i++ )
			{
				TQDomElement a = acts.item( i ).toElement();
				if ( a.attribute( "name" ) == "goToPage" )
					continue;
				TDEAction *act = d->gvpart_->action( a );
				if ( act != 0 )
					d->plugAction( act );
			}
		}
		/*
		TDEAction	*act;
		d->toolbar_->insertLineSeparator();
		if ((act = d->gvpart_->action("zoomIn")) != 0)
			d->plugAction(act);
		if ((act = d->gvpart_->action("zoomOut")) != 0)
			d->plugAction(act);
		d->toolbar_->insertSeparator();
		if ((act = d->gvpart_->action("prevPage")) != 0)
			d->plugAction(act);
		if ((act = d->gvpart_->action("nextPage")) != 0)
			d->plugAction(act);
			*/
	}
	d->toolbar_->setIconText(TDEToolBar::IconTextRight);
	d->toolbar_->setBarPos(TDEToolBar::Top);
	d->toolbar_->setMovingEnabled(false);
	//d->adjustSize();

	// construct the layout
	TQVBoxLayout	*l0 = new TQVBoxLayout(d->mainwidget_, 0, 0);
	l0->addWidget(d->toolbar_, AlignTop);
	if (d->gvpart_)
		l0->addWidget(d->gvpart_->widget());

	resize(855, 500);
	setCaption(i18n("Print Preview"));
}

void KPrintPreview::openFile(const TQString& file)
{
	d->gvpart_->openURL(KURL(file));
}

bool KPrintPreview::isValid() const
{
	return (d->gvpart_ != 0);
}

bool KPrintPreview::preview(const TQString& file, bool previewOnly, WId parentId)
{
	KMimeType::Ptr mime = KMimeType::findByPath( file );
	bool isPS = ( mime->name() == "application/postscript" );
	if ( !isPS )
		kdDebug( 500 ) << "Previewing a non PostScript file, built-in preview disabled" << endl;

	TDEConfig	*conf = KMFactory::self()->printConfig();
	conf->setGroup("General");
	KLibFactory	*factory(0);
	bool	externalPreview = conf->readBoolEntry("ExternalPreview", false);
	TQWidget	*parentW = TQWidget::find(parentId);
	TQString	exe;
	if (!externalPreview && isPS && (factory = componentFactory()) != 0)
	{
		KPrintPreview	dlg(parentW, previewOnly);
		dlg.initView(factory);

		if (dlg.isValid())
		{
			dlg.openFile(file);
			return dlg.exec();
		}
		else 
		{
			// do nothing at that point: try to use the other way around by
			// using an external PS viewer if possible
		}
	}

	// Either the PS viewer component was not found, or an external
	// preview program has been specified
	KPreviewProc	proc;
	if (externalPreview && isPS )
	{
		exe = conf->readPathEntry("PreviewCommand", "gv");
		if (TDEStandardDirs::findExe(exe).isEmpty())
		{
			TQString	msg = i18n("The preview program %1 cannot be found. "
						       "Check that the program is correctly installed and "
						       "located in a directory included in your PATH "
						       "environment variable.").arg(exe);
			return continuePrint(msg, parentW, previewOnly);
		}
		proc << exe << file;
	}
	else
	{
		KService::Ptr serv = KServiceTypeProfile::preferredService( mime->name(), TQString::null );
		if ( serv )
		{
			KURL url;
			url.setPath( file );
			TQStringList args = KRun::processDesktopExec( *serv, url, false );
			proc << args;
			exe = serv->name();
		}
		else
		{
			// in that case, the PS viewer component could not be loaded and no service
			// could be found to view PS
			TQString msg;
			if ( isPS )
				msg = i18n("Preview failed: neither the internal TDE PostScript "
			               "viewer (KGhostView) nor any other external PostScript "
			               "viewer could be found.");
			else
				msg = i18n( "Preview failed: TDE could not find any application "
						    "to preview files of type %1." ).arg( mime->name() );

			return continuePrint(msg, parentW, previewOnly);
		}
	}

	// start the preview process
	if (!proc.startPreview())
	{
		TQString	msg = i18n("Preview failed: unable to start program %1.").arg(exe);
		return continuePrint(msg, parentW, previewOnly);
	}
	else if (!previewOnly)
	{
		return (KMessageBox::questionYesNo(parentW, i18n("Do you want to continue printing?"), TQString::null, KGuiItem(i18n("Print"),"document-print"), KStdGuiItem::cancel(), "continuePrinting") == KMessageBox::Yes);
	}
	else
		return false;
}

#include "kprintpreview.moc"
