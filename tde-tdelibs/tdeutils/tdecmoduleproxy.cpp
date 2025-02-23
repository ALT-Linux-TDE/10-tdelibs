/*  This file is part of the KDE project
    Copyright (C) 2004 Frans Englich <frans.englich@telia.com>
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqapplication.h>
#include <tqcursor.h>
#include <tqdatastream.h>
#include <tqevent.h>
#include <tqfileinfo.h>
#include <tqframe.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpoint.h>
#include <tqscrollview.h>
#include <tqtextstream.h>
#include <tqvbox.h>
#include <tqwhatsthis.h>
#include <tqwidget.h>

#include <dcopclient.h>
#include <qxembed.h>

#include <tdeapplication.h>
#include <tdeaboutdata.h>
#include <tdecmodule.h>
#include <tdecmoduleinfo.h>
#include <tdecmoduleloader.h>
#include <kdebug.h>
#include <kdialog.h>
#include <tdelocale.h>
#include <kprocess.h>
#include <kservice.h>
#include <kstandarddirs.h>
#include <kuser.h>

#include <X11/Xlib.h>

#include "tdecmoduleproxy.h"
#include "tdecmoduleproxyIface.h"
#include "tdecmoduleproxyIfaceImpl.h"

/***************************************************************/
class TDECModuleProxy::TDECModuleProxyPrivate
{
	public:
		TDECModuleProxyPrivate( const TDECModuleInfo & info )
			: args( 0 )
			, kcm( 0 )
			//, view( 0 )
			, embedWidget( 0 )
			, rootProcess ( 0 )
			, embedFrame ( 0 )
			, dcopObject( 0 )
			, dcopClient( 0 )
			, topLayout( 0 )
			, rootCommunicator( 0 )
			, rootInfo( 0 )
			, modInfo( info )
			, withFallback( false )
			, changed( false )
			, rootMode( false )
			, bogusOccupier( false )
			, isInitialized( false )
		{}

		~TDECModuleProxyPrivate()
		{
			delete rootInfo; // Delete before embedWidget!
			delete embedWidget; // Delete before embedFrame!
			delete embedFrame;
			delete dcopClient;
			delete dcopObject;
			delete rootCommunicator;
			delete rootProcess;
			delete kcm;
		}

		TQStringList							args;
		TDECModule							*kcm;
		QXEmbed								*embedWidget;
		TDEProcess							*rootProcess;
		TQVBox								*embedFrame;
		TDECModuleProxyIfaceImpl  					*dcopObject;
		DCOPClient							*dcopClient;
		TQVBoxLayout							*topLayout; /* Contains TQScrollView view, and root stuff */
		TDECModuleProxyRootCommunicatorImpl				*rootCommunicator;
		TQLabel								*rootInfo;
		TQCString							dcopName;
		TDECModuleInfo 							modInfo;
		bool 								withFallback;
		bool 								changed;
		bool 								rootMode;
		bool								bogusOccupier;
		bool								isInitialized;
};
/***************************************************************/



/*
 TODO:

 - How TDECModuleProxy behaves wrt memory leaks and behavior, when exiting
 	from root mode is not tested, because no code make use of it. It needs 
	work, if it should be used.

 - Should write a document which outlines test cases, to avoid 
 	regressions. This class is a hazard.

 - Two Layout problems in runAsRoot:
 	* lblBusy doesn't show
 	* d->kcm/d->rootInfo doesn't get it right when the user 
		presses cancel in the tdesu dialog

 - Resizing horizontally is contrained; minimum size is set somewhere. 
 	It appears to be somehow derived from the module's size.

 - Prettify: set icon in KCMultiDialog.

 - Perhaps it's possible to link against tdesu such that 
 	the dialog is in process?

 */
/***************************************************************/
TDECModule * TDECModuleProxy::realModule() const
{

	/*
	 * Note, don't call any function that calls realModule() since
	 * that leads to an infinite loop.
	 */

	kdDebug(711) << k_funcinfo << endl;

	/* Already loaded */
	if( d->kcm )
		return d->kcm;

	/* /We/ have no kcm, but tdecmshell running with root prevs does.. */
	if( d->rootMode )
		return 0;

	TQApplication::setOverrideCursor( TQt::WaitCursor );
	
	TDECModuleProxy * that = const_cast<TDECModuleProxy*>( this );

	if( !d->isInitialized )
	{
  		d->dcopName = TQString(moduleInfo().handle().prepend("TDECModuleProxy-")).utf8();
		d->topLayout = new TQVBoxLayout( that, 0, 0, "topLayout" );

		d->isInitialized = true;
	}

	if( !d->dcopClient )
		d->dcopClient = new DCOPClient();

	if( !d->dcopClient->isRegistered() )
  		d->dcopClient->registerAs( d->dcopName, false );

	d->dcopClient->setAcceptCalls( true );

	if( d->dcopClient->appId() == d->dcopName || d->bogusOccupier )
	{ /* We got the name we requested, because no one was before us, 
	   * or, it was an random application which had picked that name */
		kdDebug(711) << "Module not already loaded, loading module" << endl;

		d->dcopObject = new TDECModuleProxyIfaceImpl( d->dcopName, that );

		d->kcm = TDECModuleLoader::loadModule( moduleInfo(), TDECModuleLoader::Inline, d->withFallback,
			that, name(), d->args );

		connect( d->kcm, TQ_SIGNAL( changed( bool ) ),
				TQ_SLOT(moduleChanged(bool)) );
		connect( d->kcm, TQ_SIGNAL( destroyed() ),
				TQ_SLOT( moduleDestroyed() ) );
		connect( d->kcm, TQ_SIGNAL(quickHelpChanged()), 
				TQ_SIGNAL(quickHelpChanged()));
		TQWhatsThis::add( that, d->kcm->quickHelp() );

		d->topLayout->addWidget( d->kcm );

		if ( !d->rootInfo && /* If the message was not yet created */
				d->kcm->useRootOnlyMsg() /* and the module requests the message */ &&
				moduleInfo().needsRootPrivileges() /* and the module wants root access */ && 
				!KUser().isSuperUser() ) /* and we are not currently root */
		{

			d->rootInfo = new TQLabel( that, "rootInfo" );
			d->topLayout->insertWidget( 0, d->rootInfo );

			d->rootInfo->setFrameShape(TQFrame::Box);
			d->rootInfo->setFrameShadow(TQFrame::Raised);

			const TQString msg = d->kcm->rootOnlyMsg();
			if( msg.isEmpty() )
				d->rootInfo->setText(i18n(
   					  "<b>Changes in this section requires root access.</b><br />"
					  "Click the \"Administrator Mode\" button to "
					  "allow modifications."));
			else
				d->rootInfo->setText(msg);

			TQWhatsThis::add( d->rootInfo, i18n(
				  "This section requires special permissions, probably "
				  "for system-wide changes; therefore, it is "
				  "required that you provide the root password to be "
				  "able to change the module's properties. If "
				  "you do not provide the password, the module will be "
				  "disabled."));
		}
	}
	else
	{
		kdDebug(711) << "Module already loaded, loading KCMError" << endl;

		d->dcopClient->detach();
 		/* Re-register as anonymous */
		d->dcopClient->attach();

		d->dcopClient->setNotifications( true );
		connect( d->dcopClient, TQ_SIGNAL( applicationRemoved( const TQCString& )),
			TQ_SLOT( applicationRemoved( const TQCString& )));

		/* Figure out the name of where the module is already loaded */
		TQByteArray replyData, data;
		TQCString replyType;
		TQString result;
		TQDataStream arg, stream( replyData, IO_ReadOnly );

		if( d->dcopClient->call( d->dcopName, d->dcopName, "applicationName()", 
					data, replyType, replyData ))
		{
			stream >> result;

			d->kcm = TDECModuleLoader::reportError( TDECModuleLoader::Inline, 
					i18n( "Argument is application name", "This configuration section is "
						"already opened in %1" ).arg( result ), " ", that );

			d->topLayout->addWidget( d->kcm );
		}
		else
		{
			kdDebug(711) << "Calling TDECModuleProxy's DCOP interface for fetching the name failed." << endl;
			d->bogusOccupier = true;
			TQApplication::restoreOverrideCursor();
			return realModule();
		}
	}

	TQApplication::restoreOverrideCursor();

	return d->kcm;
}

void TDECModuleProxy::applicationRemoved( const TQCString& app )
{
	if( app == d->dcopName )
	{
		/* Violence: Get rid of KCMError & CO, so that 
		 * realModule() attempts to reload the module */
		delete d->kcm;
		d->kcm = 0;
		d->dcopClient->setNotifications( false );
		realModule();
		d->kcm->show();
	}
}

void TDECModuleProxy::showEvent( TQShowEvent * ev )
{

	kdDebug(711) << k_funcinfo << endl;
	( void )realModule();

	/* We have no kcm, if we're in root mode */
	if( d->kcm )
		d->kcm->show();

	TQWidget::showEvent( ev );

}

void TDECModuleProxy::runAsRoot()
{
	if ( !moduleInfo().needsRootPrivileges() )
		return;

	TQApplication::setOverrideCursor( TQt::WaitCursor );

	delete d->rootProcess;
	delete d->embedWidget;
	delete d->embedFrame;

	d->embedFrame = new TQVBox( this, "embedFrame" );
	d->embedFrame->setFrameStyle( TQFrame::Box | TQFrame::Raised );

	TQPalette pal( red );
	pal.setColor( TQColorGroup::Background, 
		colorGroup().background() );
	d->embedFrame->setPalette( pal );
	d->embedFrame->setLineWidth( 2 );
	d->embedFrame->setMidLineWidth( 2 );
	d->topLayout->addWidget(d->embedFrame,1);

	d->embedWidget = new QXEmbed( d->embedFrame, "embedWidget" );

	d->embedFrame->show();

	TQLabel *lblBusy = new TQLabel(i18n("<big>Loading...</big>"), d->embedWidget, "lblBusy" );
	lblBusy->setTextFormat(RichText);
	lblBusy->setAlignment(AlignCenter);
	lblBusy->setGeometry(0,0, d->kcm->width(), d->kcm->height());
	lblBusy->show();

	deleteClient();
	/* The DCOP registration is now gone, and it will occur again when tdecmshell soon 
	 * registers. Here's a race condition in other words, but how likely is that?
	 *
	 * - It's a user initiated action, which means the user have to do weird stuff, very
	 *   quick.
	 * - If the user _do_ manage to fsck up, the code will recover gracefully, see realModule().
	 *
	 * So no worry. At the end of this function, communication with 
	 * the DCOP object is established.
	 */

	/* Prepare the process to run the tdecmshell */
	TQString cmd = moduleInfo().service()->exec().stripWhiteSpace();
	if (cmd.left(5) == "tdesu")
	{
		cmd = TQString(cmd.remove(0,5)).stripWhiteSpace();

		/* Remove all tdesu switches */
		while( cmd.length() > 1 && cmd[ 0 ] == '-' )
			cmd = TQString(cmd.remove( 0, cmd.find( ' ' ) )).stripWhiteSpace();
	}

	if (cmd.left(8) == "tdecmshell")
		cmd = TQString(cmd.remove(0,8)).stripWhiteSpace();

	/* Run the process */
	TQString tdesu = TDEStandardDirs::findExe("tdesu");
	if (!tdesu.isEmpty())
	{

		d->rootProcess = new TDEProcess;

		*d->rootProcess << tdesu;
		*d->rootProcess << "--nonewdcop" << "-n" << "-d" << TQString( "-i%1" ).arg(moduleInfo().icon());

		*d->rootProcess << TQString("%1 %2 --embed-proxy %3 --lang %4").arg(locate("exe", "tdecmshell"))
			.arg(cmd).arg(d->embedWidget->winId()).arg(TDEGlobal::locale()->language());

		connect(d->rootProcess, TQ_SIGNAL(processExited(TDEProcess*)), TQ_SLOT(rootExited()));

		if ( !d->rootProcess->start( TDEProcess::NotifyOnExit ))
		{
			d->rootMode = false;
			rootExited();
		}
		else
		{
			d->rootMode = true;
			kapp->dcopClient();
			d->rootCommunicator = new TDECModuleProxyRootCommunicatorImpl( d->dcopName + "-RootCommunicator", this );
		}

		delete lblBusy;
		TQApplication::restoreOverrideCursor();
		return;
	}

	/* Clean up in case of failure */
	delete d->embedWidget;
	d->embedWidget = 0;
	delete d->embedFrame;
	d->embedFrame = 0;

	TQApplication::restoreOverrideCursor();
}

void TDECModuleProxy::rootExited()
{
	kdDebug(711) << k_funcinfo << endl;

	if ( d->embedWidget->embeddedWinId() )
		XDestroyWindow(tqt_xdisplay(), d->embedWidget->embeddedWinId());

	delete d->embedWidget;
	d->embedWidget = 0;

	delete d->rootProcess;
	d->rootProcess = 0;

	delete d->embedFrame;
	d->embedFrame=0;

	delete d->rootCommunicator;
	d->rootCommunicator = 0;

	/* Such that the "ordinary" module loads again */
	d->rootMode = false;

	d->topLayout->invalidate();

	TQShowEvent ev;
	showEvent( &ev ); 

	moduleChanged( false );
	emit childClosed();
}

TDECModuleProxy::~TDECModuleProxy()
{
	deleteClient();
	TDECModuleLoader::unloadModule(moduleInfo());

	delete d;
}

void TDECModuleProxy::deleteClient()
{
	if( d->embedWidget )
		XKillClient(tqt_xdisplay(), d->embedWidget->embeddedWinId());


	delete d->kcm;
	d->kcm = 0;

	delete d->dcopObject;
	d->dcopObject = 0;

	if( d->dcopClient && !d->dcopClient->detach() )
		kdDebug(711) << "Unregistering from DCOP failed." << endl;

	delete d->dcopClient;
	d->dcopClient = 0;

	kapp->syncX();

}

void TDECModuleProxy::moduleChanged( bool c )
{
	if(  d->changed == c )
		return;

	d->changed = c;
	emit changed( c );
	emit changed( this );
}

void TDECModuleProxy::moduleDestroyed()
{
	d->kcm = 0;
}

TDECModuleProxy::TDECModuleProxy( const KService::Ptr & service, bool withFallback, 
		TQWidget  * parent, const char * name, const TQStringList & args)
	: TQWidget( parent, name )
{
	init( TDECModuleInfo( service ));
	d->args = args;
	d->withFallback = withFallback;
}

TDECModuleProxy::TDECModuleProxy( const TDECModuleInfo & info, bool withFallback,
		TQWidget * parent, const char * name, const TQStringList & args )
	: TQWidget( parent, name )
{
	init( info );
	d->args = args;
	d->withFallback = withFallback;
}

TDECModuleProxy::TDECModuleProxy( const TQString& serviceName, bool withFallback, 
		TQWidget * parent, const char * name, 
		const TQStringList & args)
	: TQWidget( parent, name )
{
	init( TDECModuleInfo( serviceName ));
	d->args = args;
	d->withFallback = withFallback;
}

void TDECModuleProxy::init( const TDECModuleInfo& info )
{ 
	kdDebug(711) << k_funcinfo << endl;

	d = new TDECModuleProxyPrivate( info );

	/* This is all we do for now; all the heavy work is 
	 * done in realModule(). It's called when the module
	 * _actually_ is needed, in for example showEvent().
	 * The module is loaded "on demand" -- lazy loading.
	 */

}

void TDECModuleProxy::load()
{

	if( d->rootMode )
		callRootModule( "load()" );
	else if( realModule() )
	{
		d->kcm->load();
		moduleChanged( false );
	}
}

void TDECModuleProxy::save()
{
	if( d->rootMode )
		callRootModule( "save()" );
	else if( d->changed && realModule() )
	{
		d->kcm->save();
		moduleChanged( false );
	}
}

void TDECModuleProxy::callRootModule( const TQCString& function )
{
	TQByteArray sendData, replyData;
	TQCString replyType;

	/* Note, we don't use d->dcopClient here, because it's used for 
	 * the loaded module(and it's not "us" when this function is called) */
	if( !kapp->dcopClient()->call( d->dcopName, d->dcopName, function, sendData,
			replyType, replyData, true, -1 ))
		kdDebug(711) << "Calling function '" << function << "' failed." << endl;

}

void TDECModuleProxy::defaults()
{
	if( d->rootMode )
		callRootModule( "defaults()" );
	if( realModule() )
		d->kcm->defaults();
}

TQString TDECModuleProxy::quickHelp() const
{

	if( !d->rootMode )
		return realModule() ? realModule()->quickHelp() : TQString::null;
	else
	{
		TQByteArray data, replyData;
		TQCString replyType;

		if (kapp->dcopClient()->call(d->dcopName, d->dcopName, "quickHelp()",
				  data, replyType, replyData))
			kdDebug(711) << "Calling DCOP function bool changed() failed." << endl;
		else
		{
			TQDataStream reply(replyData, IO_ReadOnly);
			if (replyType == "TQString")
			{
				TQString result;
				reply >> result;
				return result;
			}
			else
				kdDebug(711) << "DCOP function changed() returned mumbo jumbo." << endl;
		}
		return TQString::null;
	}
}

const TDEAboutData * TDECModuleProxy::aboutData() const
{
	if( !d->rootMode ) {
		return realModule() ? realModule()->aboutData() : 0;
	}
	else {
	/* This needs fixing, perhaps cache a TDEAboutData copy
	 * while in root mode? */
		return 0;
	}
}

TQString TDECModuleProxy::handbookDocPath() const
{
	if( !d->rootMode ) {
		return realModule() ? realModule()->handbookDocPath() : TQString::null;
	}
	else {
	/* This needs fixing, perhaps cache a handbookSection() copy
	 * while in root mode? */
		return TQString::null;
	}
}

TQString TDECModuleProxy::handbookSection() const
{
	if( !d->rootMode ) {
		return realModule() ? realModule()->handbookSection() : TQString::null;
	}
	else {
	/* This needs fixing, perhaps cache a handbookSection() copy
	 * while in root mode? */
		return TQString::null;
	}
}

int TDECModuleProxy::buttons() const
{
	return realModule() ? realModule()->buttons() :
		TDECModule::Help | TDECModule::Default | TDECModule::Apply ;
}

TQString TDECModuleProxy::rootOnlyMsg() const
{
	return realModule() ? realModule()->rootOnlyMsg() : TQString::null;
}

bool TDECModuleProxy::useRootOnlyMsg() const
{
	return realModule() ? realModule()->useRootOnlyMsg() : true;
}

TDEInstance * TDECModuleProxy::instance() const
{
	return realModule() ? realModule()->instance() : 0;
}

bool TDECModuleProxy::changed() const
{
	return d->changed;
}

const TDECModuleInfo& TDECModuleProxy::moduleInfo() const
{
	return d->modInfo;
}

bool TDECModuleProxy::rootMode() const
{
	return d->rootMode;
}

TQCString TDECModuleProxy::dcopName() const
{
	return d->dcopName;
}

void TDECModuleProxy::emitQuickHelpChanged()
{
	emit quickHelpChanged();
}

/***************************************************************/
#include "tdecmoduleproxy.moc"
