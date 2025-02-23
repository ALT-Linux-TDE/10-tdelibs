/* This file is part of the KDE libraries
    Copyright (C) 2004 Frans Englich <frans.englich@telia.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqlayout.h>
#include <tqpixmap.h>
#include <tqstringlist.h>
#include <tqtabwidget.h>
#include <tqtooltip.h>
#include <tqvaluelist.h>

#include <tdecmodule.h>
#include <tdecmoduleinfo.h>
#include <tdecmoduleloader.h>
#include <tdecmoduleproxy.h>
#include <kdebug.h>
#include <kdialog.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <kpushbutton.h>
#include <kservice.h>
#include <kstdguiitem.h>

#include "tdecmodulecontainer.h"
#include "tdecmodulecontainer.moc"

/***********************************************************************/
class TDECModuleContainer::TDECModuleContainerPrivate
{
	public:
		TDECModuleContainerPrivate( const TQStringList& mods )
			: modules( mods )
			, tabWidget( 0 )
			, buttons( 0 )
			, hasRootKCM( false )
			, btnRootMode( 0 )
			, btnLayout( 0 )
			, topLayout( 0 )
			{}

		TQStringList modules;
		TQTabWidget *tabWidget;
		int buttons;
		bool hasRootKCM: 1;
		KPushButton *btnRootMode;
		TQHBoxLayout *btnLayout;
		TQVBoxLayout *topLayout;
		TQString handbookSection;


};
/***********************************************************************/





/***********************************************************************/
TDECModuleContainer::TDECModuleContainer( TQWidget* parent, const char* name, 
	const TQString& mods )
	: TDECModule( parent, name )
{
	d = new TDECModuleContainerPrivate( TQStringList::split( ",", TQString(mods).remove( " " )) );
	init();
}

TDECModuleContainer::TDECModuleContainer( TQWidget* parent, const char* name, 
	const TQStringList& mods )
	: TDECModule( parent, name ), d( new TDECModuleContainerPrivate( mods ) )
{
	init();
}

void TDECModuleContainer::init()
{
	d->topLayout = new TQVBoxLayout( this, 0, KDialog::spacingHint(), "topLayout" );
	d->tabWidget = new TQTabWidget(this, "tabWidget");
	d->tabWidget->setMargin(KDialog::marginHint());
	connect( d->tabWidget, TQ_SIGNAL( currentChanged( TQWidget* ) ), TQ_SLOT( tabSwitched( TQWidget* ) ));
	d->topLayout->addWidget( d->tabWidget );

	if ( !d->modules.isEmpty() )
	{
		/* Add our modules */
		for ( TQStringList::Iterator it = d->modules.begin(); it != d->modules.end(); ++it )
			addModule( (*it) );

		finalize();
	}

}

void TDECModuleContainer::finalize()
{
	setButtons( d->buttons );
	if ( d->hasRootKCM ) /* Add a root mode button */
	{
		if(!d->btnLayout) /* It could already be added */
		{
			d->btnLayout = new TQHBoxLayout(this, 0, 0, "btnLayout");
			d->btnRootMode = new KPushButton(KStdGuiItem::adminMode(), this, "btnRootMode");
					
			d->btnLayout->addWidget( d->btnRootMode );
			d->btnLayout->addStretch();
			d->topLayout->addLayout( d->btnLayout );
		}
	}
}

void TDECModuleContainer::addModule( const TQString& module )
{
	/* In case it doesn't exist we just silently drop it.
	 * This allows people to easily extend containers.
	 * For example, KCM monitor gamma can be in tdegraphics.
	 */
	if ( !KService::serviceByDesktopName( module ) )
	{
		kdDebug(713) << "TDECModuleContainer: module '" << 
			module << "' was not found and thus not loaded" << endl;
		return;
	}

	if( !TDECModuleLoader::testModule( module )) {
		return;
	}

	TDECModuleProxy* proxy = new TDECModuleProxy( module, false, d->tabWidget, module.latin1());
	if (allModules.count() < 1) {
		d->handbookSection = proxy->handbookSection();
	}
	allModules.append( proxy );

	d->tabWidget->addTab( proxy, TQIconSet(TDEGlobal::iconLoader()->loadIcon(
					proxy->moduleInfo().icon(), TDEIcon::Desktop)),
			/* QT eats ampersands for dinner. But not this time. */
			proxy->moduleInfo().moduleName().replace( "&", "&&" ));

	d->tabWidget->setTabToolTip( proxy, proxy->moduleInfo().comment() );

	connect( proxy, TQ_SIGNAL(changed(TDECModuleProxy *)), TQ_SLOT(moduleChanged(TDECModuleProxy *)));

	/* Collect our buttons - we go for the common deliminator */
	d->buttons = d->buttons | proxy->realModule()->buttons();

	/* If we should add an Administrator Mode button */
	if ( proxy->moduleInfo().needsRootPrivileges() ) {
		d->hasRootKCM=true;
	}
}

void TDECModuleContainer::tabSwitched( TQWidget * module )
{
	TDECModuleProxy* mod = (TDECModuleProxy *) module;
	d->handbookSection = mod->handbookSection();

	if ( !d->hasRootKCM ) {
		return;
	}

	/* Not like this. Not like this. */
	disconnect( d->btnRootMode, 0, 0, 0 );
	/* Welcome to the real world huh baby? */

	if ( mod->moduleInfo().needsRootPrivileges() && !mod->rootMode() ) {
		d->btnRootMode->setEnabled( true );
		connect( d->btnRootMode, TQ_SIGNAL( clicked() ), 
				TQ_SLOT( runAsRoot() ));
		connect( mod, TQ_SIGNAL( childClosed() ), 
				TQ_SLOT ( rootExited() ));
	}
	else {
		d->btnRootMode->setEnabled( false );
	}

	setQuickHelp( mod->quickHelp() );
	setAboutData( const_cast<TDEAboutData*>(mod->aboutData()) );
}

TQString TDECModuleContainer::handbookSection() const
{
	return d->handbookSection;
}

void TDECModuleContainer::runAsRoot()
{
	if ( d->tabWidget->currentPage() )
		( (TDECModuleProxy *) d->tabWidget->currentPage() )->runAsRoot();
	d->btnRootMode->setEnabled( false );
}

void TDECModuleContainer::rootExited()
{
	connect( d->btnRootMode, TQ_SIGNAL( clicked() ), TQ_SLOT( runAsRoot() ));
	d->btnRootMode->setEnabled( true );
}

void TDECModuleContainer::save()
{
	ModuleList list = changedModules;
	ModuleList::iterator it;
	for ( it = list.begin() ; it !=list.end() ; ++it )
	{
		(*it)->save();
	}

	emit changed( false );

}

void TDECModuleContainer::load()
{
	ModuleList list = allModules;
	ModuleList::iterator it;
	for ( it = list.begin() ; it !=list.end() ; ++it )
	{
		(*it)->load();
	}

	emit changed( false );
}

void TDECModuleContainer::defaults()
{
	ModuleList list = allModules;
	ModuleList::iterator it;
	for ( it = list.begin() ; it !=list.end() ; ++it )
	{
		(*it)->defaults();
	}

	emit changed( true );
}


void TDECModuleContainer::moduleChanged(TDECModuleProxy * proxy)
{
	changedModules.append( proxy );
	if( changedModules.isEmpty() )
		return;

	emit changed(true);
}

TDECModuleContainer::~TDECModuleContainer()
{
	delete d;
}

/***********************************************************************/




