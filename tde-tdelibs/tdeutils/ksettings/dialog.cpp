/*  This file is part of the KDE project
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

#include "ksettings/dialog.h"


#include <kcmultidialog.h>
#include <tdelocale.h>
#include <kservicegroup.h>
#include <kdebug.h>
#include <ktrader.h>
#include <kplugininfo.h>
#include "ksettings/dispatcher.h"
#include "ksettings/componentsdialog.h"
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <tqvbox.h>
#include <tqlabel.h>
#include "tdecmoduleinfo.h"

namespace KSettings
{

struct GroupInfo
{
	TQString id;
	TQString name;
	TQString comment;
	TQString icon;
	int weight;
	TQString parentid;
	TQWidget * page;
};

// The TreeList can get really complicated. That's why a tree data structure
// is necessary to make it suck less
class PageNode
{
	private:
		typedef TQValueList<PageNode*> List;
		enum Type { KCM, Group, Root };
		union Value
		{
			TDECModuleInfo * kcm;
			GroupInfo * group;
		};
		Type m_type;
		Value m_value;

		Dialog * m_dialog;
		List m_children;
		PageNode * m_parent;
		bool m_visible;
		bool m_dirty;

	protected:
		PageNode( TDECModuleInfo * info, PageNode * parent )
			: m_type( KCM )
			, m_parent( parent )
			, m_visible( true )
			, m_dirty( true )
		{
			m_value.kcm = info;
			m_dialog = parent->m_dialog;
		}

		PageNode( GroupInfo & group, PageNode * parent )
			: m_type( Group )
			, m_parent( parent )
			, m_visible( true )
			, m_dirty( true )
		{
			m_value.group = new GroupInfo( group );
			m_value.group->page = 0;
			m_dialog = parent->m_dialog;
		}

		void bubbleSort( List::Iterator begin, List::Iterator end )
		{
			--end;
			bool finished;
			List::Iterator lastswapped = begin;
			List::Iterator i;
			List::Iterator j;
			while( begin != end )
			{
				finished = true;
				i = j = end;
				do {
					--j;
					if( **i < **j )
					{
						finished = false;
						tqSwap( *i, *j );
						lastswapped = j;
					}
					--i;
				} while( j != begin );
				if( finished )
					return;
				++lastswapped;
				begin = lastswapped;
			}
		}

	public:
		PageNode( Dialog * dialog )
			: m_type( Root )
			, m_dialog( dialog )
			, m_parent( 0 )
			, m_visible( true )
			, m_dirty( true )
		{}

		~PageNode()
		{
			if( KCM == m_type )
				delete m_value.kcm;
			else if( Group == m_type )
				delete m_value.group;
			List::Iterator end = m_children.end();
			for( List::Iterator it = m_children.begin(); it != end; ++it )
				delete ( *it );
		}

		int weight() const
		{
			int w = ( KCM == m_type ) ? m_value.kcm->weight()
				: m_value.group->weight;
			kdDebug( 700 ) << k_funcinfo << name() << " " << w << endl;
			return w;
		}

		bool operator<( const PageNode & rhs ) const
		{
			return weight() < rhs.weight();
		}

		bool isVisible()
		{
			if( m_dirty )
			{
				if( KCM == m_type )
					m_visible = m_dialog->isPluginForKCMEnabled( m_value.kcm );
				else
				{
					m_visible = false;
					List::Iterator end = m_children.end();
					for( List::Iterator it = m_children.begin(); it != end;
							++it )
						if( ( *it )->isVisible() )
						{
							m_visible = true;
							break;
						}
				}
				m_dirty = false;
			}
			kdDebug( 700 ) << k_funcinfo << "returns " << m_visible << endl;
			return m_visible;
		}

		void makeDirty()
		{
			m_dirty = true;
			List::Iterator end = m_children.end();
			for( List::Iterator it = m_children.begin(); it != end; ++it )
				( *it )->makeDirty();
		}

		TQString name() const
		{
			if( Root == m_type )
				return TQString::fromAscii( "root node" );
			return ( KCM == m_type ) ? m_value.kcm->moduleName()
				: m_value.group->name;
		}

		TQStringList parentNames() const
		{
			TQStringList ret;
			PageNode * node = m_parent;
			while( node && node->m_type != Root )
			{
				ret.prepend( node->name() );
				node = node->m_parent;
			}
			return ret;
		}

		void addToDialog( KCMultiDialog * dlg )
		{
			kdDebug( 700 ) << k_funcinfo << "for " << name() << endl;
			if( ! isVisible() )
				return;

			if( KCM == m_type )
			{
				dlg->addModule( *m_value.kcm, parentNames() );
				return;
			}
			if( Group == m_type && 0 == m_value.group->page )
			{
				TQPixmap icon;
				if( ! m_value.group->icon.isNull() )
					icon = SmallIcon( m_value.group->icon,
							IconSize( TDEIcon::Small ) );
				TQVBox * page = dlg->addVBoxPage( m_value.group->name,
						TQString::null, icon );
				TQLabel * comment = new TQLabel( m_value.group->comment, page );
				comment->setTextFormat( TQt::RichText );
				m_value.group->page = page;
			}
			List::Iterator end = m_children.end();
			for( List::Iterator it = m_children.begin(); it != end; ++it )
				( *it )->addToDialog( dlg );
		}

		void removeFromDialog( KCMultiDialog * dlg )
		{
			kdDebug( 700 ) << k_funcinfo << "for " << name() << endl;
			if( KCM == m_type )
				return;
			if( Root == m_type )
				dlg->removeAllModules();
			List::Iterator end = m_children.end();
			for( List::Iterator it = m_children.begin(); it != end; ++it )
				( *it )->removeFromDialog( dlg );
			if( Group == m_type )
			{
				delete m_value.group->page;
				m_value.group->page = 0;
			}
		}

		void sort()
		{
			kdDebug( 700 ) << k_funcinfo << name() << endl;
			List::Iterator begin = m_children.begin();
			List::Iterator end = m_children.end();
			bubbleSort( begin, end );
			for( List::Iterator it = begin ; it != end; ++it )
				( *it )->sort();
		}

		bool insert( GroupInfo & group )
		{
			if( group.parentid.isNull() )
			{
				if( Root == m_type )
				{
					m_children.append( new PageNode( group, this ) );
					return true;
				}
				else
					kdFatal( 700 ) << "wrong PageNode insertion"
						<< kdBacktrace() << endl;
			}
			if( Group == m_type && group.parentid == m_value.group->id )
			{
				m_children.append( new PageNode( group, this ) );
				return true;
			}
			List::Iterator end = m_children.end();
			for( List::Iterator it = m_children.begin(); it != end; ++it )
				if( ( *it )->insert( group ) )
					return true;
			// no parent with the right parentid
			if( Root == m_type )
			{
				m_children.append( new PageNode( group, this ) );
				return true;
			}
			return false;
		}

		bool insert( TDECModuleInfo * info, const TQString & parentid )
		{
			if( parentid.isNull() )
			{
				if( Root == m_type )
				{
					m_children.append( new PageNode( info, this ) );
					return true;
				}
				else
					kdFatal( 700 ) << "wrong PageNode insertion"
						<< kdBacktrace() << endl;
			}
			if( Group == m_type && parentid == m_value.group->id )
			{
				m_children.append( new PageNode( info, this ) );
				return true;
			}
			List::Iterator end = m_children.end();
			for( List::Iterator it = m_children.begin(); it != end; ++it )
				if( ( *it )->insert( info, parentid ) )
					return true;
			// no parent with the right parentid
			if( Root == m_type )
			{
				m_children.append( new PageNode( info, this ) );
				return true;
			}
			return false;
		}

		bool needTree()
		{
			List::ConstIterator end = m_children.end();
			for( List::ConstIterator it = m_children.begin(); it != end; ++it )
				if( ( *it )->m_children.count() > 0 )
					return true;
			return false;
		}

		bool singleChild()
		{
			return ( m_children.count() == 1 );
		}
};

class Dialog::DialogPrivate
{
	public:
		DialogPrivate( Dialog * parent )
			: dlg( 0 )
			, pagetree( parent )
		{
		}

		bool staticlistview;
		KCMultiDialog * dlg;
		PageNode pagetree;
		TQWidget * parentwidget;
		TQStringList registeredComponents;
		TQValueList<KService::Ptr> services;
		TQMap<TQString, KPluginInfo*> plugininfomap;
};

Dialog::Dialog( TQWidget * parent, const char * name )
	: TQObject( parent, name )
	, d( new DialogPrivate( this ) )
{
	d->parentwidget = parent;
	d->staticlistview = true;
	d->services = instanceServices();
}

Dialog::Dialog( ContentInListView content,
		TQWidget * parent, const char * name )
	: TQObject( parent, name )
	, d( new DialogPrivate( this ) )
{
	d->parentwidget = parent;
	d->staticlistview = ( content == Static );
	d->services = instanceServices();
}

Dialog::Dialog( const TQStringList & components,
		TQWidget * parent, const char * name )
	: TQObject( parent, name )
	, d( new DialogPrivate( this ) )
{
	d->parentwidget = parent;
	d->staticlistview = true;
	d->services = instanceServices() + parentComponentsServices( components );
}

Dialog::Dialog( const TQStringList & components,
		ContentInListView content, TQWidget * parent, const char * name )
	: TQObject( parent, name )
	, d( new DialogPrivate( this ) )
{
	d->parentwidget = parent;
	d->staticlistview = ( content == Static );
	d->services = instanceServices() + parentComponentsServices( components );
}

Dialog::~Dialog()
{
	delete d;
}

void Dialog::addPluginInfos( const TQValueList<KPluginInfo*> & plugininfos )
{
	for( TQValueList<KPluginInfo*>::ConstIterator it = plugininfos.begin();
			it != plugininfos.end(); ++it )
	{
		d->registeredComponents.append( ( *it )->pluginName() );
		d->services += ( *it )->kcmServices();
		d->plugininfomap[ ( *it )->pluginName() ] = *it;
	}
}

void Dialog::show()
{
	if( 0 == d->dlg )
		createDialogFromServices();
	Dispatcher::self()->syncConfiguration();
	return d->dlg->show();
}

KCMultiDialog * Dialog::dialog()
{
	if( 0 == d->dlg )
		createDialogFromServices();
	return d->dlg;
}

TQValueList<KService::Ptr> Dialog::instanceServices() const
{
	kdDebug( 700 ) << k_funcinfo << endl;
	TQString instanceName = TDEGlobal::instance()->instanceName();
	d->registeredComponents.append( instanceName );
	kdDebug( 700 ) << "calling KServiceGroup::childGroup( " << instanceName
		<< " )" << endl;
	KServiceGroup::Ptr service = KServiceGroup::childGroup( instanceName );

	TQValueList<KService::Ptr> ret;

	if( service && service->isValid() )
	{
		kdDebug( 700 ) << "call was successfull" << endl;
		KServiceGroup::List list = service->entries();
		for( KServiceGroup::List::ConstIterator it = list.begin();
				it != list.end(); ++it )
		{
			KSycocaEntry * p = *it;
			if( p->isType( KST_KService ) )
			{
				kdDebug( 700 ) << "found service" << endl;
				ret << static_cast<KService *>( p );
			}
			else
				kdWarning( 700 ) << "KServiceGroup::childGroup returned"
					" something else than a KService (kinda)" << endl;
		}
	}

	return ret;
}

TQValueList<KService::Ptr> Dialog::parentComponentsServices(
		const TQStringList & kcdparents ) const
{
	d->registeredComponents += kcdparents;
	TQString constraint = kcdparents.join(
			"' in [X-TDE-ParentComponents]) or ('" );
	constraint = "('" + constraint + "' in [X-TDE-ParentComponents])";

	kdDebug( 700 ) << "constraint = " << constraint << endl;
	return TDETrader::self()->query( "TDECModule", constraint );
}

bool Dialog::isPluginForKCMEnabled( TDECModuleInfo * moduleinfo ) const
{
	// if the user of this class requested to hide disabled modules
	// we check whether it should be enabled or not
	bool enabled = true;
	kdDebug( 700 ) << "check whether the " << moduleinfo->moduleName()
		<< " KCM should be shown" << endl;
	// for all parent components
	TQStringList parentComponents = moduleinfo->service()->property(
			"X-TDE-ParentComponents" ).toStringList();
	for( TQStringList::ConstIterator pcit = parentComponents.begin();
			pcit != parentComponents.end(); ++pcit )
	{
		// if the parentComponent is not registered ignore it
		if( d->registeredComponents.find( *pcit ) ==
				d->registeredComponents.end() )
			continue;

		// we check if the parent component is a plugin
		if( ! d->plugininfomap.contains( *pcit ) )
		{
			// if not the TDECModule must be enabled
			enabled = true;
			// we're done for this TDECModuleInfo
			break;
		}
		// if it is a plugin we check whether the plugin is enabled
		KPluginInfo * pinfo = d->plugininfomap[ *pcit ];
		pinfo->load();
		enabled = pinfo->isPluginEnabled();
		kdDebug( 700 ) << "parent " << *pcit << " is "
			<< ( enabled ? "enabled" : "disabled" ) << endl;
		// if it is enabled we're done for this TDECModuleInfo
		if( enabled )
			break;
	}
	return enabled;
}

void Dialog::parseGroupFile( const TQString & filename )
{
	KSimpleConfig file( filename );
	TQStringList groups = file.groupList();
	for( TQStringList::ConstIterator it = groups.begin(); it != groups.end();
			++it )
	{
		GroupInfo group;
		TQString id = *it;
		file.setGroup( id.utf8() );
		group.id = id;
		group.name = file.readEntry( "Name" );
		group.comment = file.readEntry( "Comment" );
		group.weight = file.readNumEntry( "Weight", 100 );
		group.parentid = file.readEntry( "Parent" );
		group.icon = file.readEntry( "Icon" );
		d->pagetree.insert( group );
	}
}

void Dialog::createDialogFromServices()
{
	// read .setdlg files
	TQString setdlgpath = locate( "appdata",
			TDEGlobal::instance()->instanceName() + ".setdlg" );
	TQStringList setdlgaddon = TDEGlobal::dirs()->findAllResources( "appdata",
			"ksettingsdialog/*.setdlg" );
	if( ! setdlgpath.isNull() )
		parseGroupFile( setdlgpath );
	if( setdlgaddon.size() > 0 )
		for( TQStringList::ConstIterator it = setdlgaddon.begin();
				it != setdlgaddon.end(); ++it )
			parseGroupFile( *it );

	// now we process the TDECModule services
	for( TQValueList<KService::Ptr>::ConstIterator it = d->services.begin();
			it != d->services.end(); ++it )
	{
		// we create the TDECModuleInfo
		TDECModuleInfo * info = new TDECModuleInfo( *it );
		TQString parentid;
		TQVariant tmp = info->service()->property( "X-TDE-CfgDlgHierarchy",
			TQVariant::String );
		if( tmp.isValid() )
			parentid = tmp.toString();
		d->pagetree.insert( info, parentid );
	}

	// At this point d->pagetree holds a nice structure of the pages we want
	// to show. It's not going to change anymore so we can sort it now.
	d->pagetree.sort();

	int dialogface = KJanusWidget::IconList;
	if( d->pagetree.needTree() )
		dialogface = KJanusWidget::TreeList;
	else if( d->pagetree.singleChild() )
		dialogface = KJanusWidget::Plain;

	kdDebug( 700 ) << "creating KCMultiDialog" << endl;
	d->dlg = new KCMultiDialog( dialogface, i18n( "Configure" ),
			d->parentwidget );

	if( dialogface == KJanusWidget::TreeList )
		d->dlg->setShowIconsInTreeList( true );

	// TODO: Don't show the reset button until the issue with the
	// KPluginSelector::load() method is solved.
	// Problem:
	// KCMultiDialog::show() call TDECModule::load() to reset all KCMs
	// (KPluginSelector::load() resets all plugin selections and all plugin
	// KCMs).
	// The reset button calls TDECModule::load(), too but in this case we want the
	// KPluginSelector to only reset the current visible plugin KCM and not
	// touch the plugin selections.
	// I have no idea how to check that in KPluginSelector::load()...
	//d->dlg->showButton( KDialogBase::User1, true );

	if( ! d->staticlistview )
		d->dlg->addButtonBelowList( i18n( "Select Components..." ), this,
			TQ_SLOT( configureTree() ) );

	connect( d->dlg, TQ_SIGNAL( okClicked() ), Dispatcher::self(),
		TQ_SLOT( syncConfiguration() ) );
	connect( d->dlg, TQ_SIGNAL( applyClicked() ), Dispatcher::self(),
		TQ_SLOT( syncConfiguration() ) );
	connect( d->dlg, TQ_SIGNAL( configCommitted( const TQCString & ) ),
		Dispatcher::self(), TQ_SLOT( reparseConfiguration( const TQCString & ) ) );

	d->pagetree.addToDialog( d->dlg );

	if( dialogface == KJanusWidget::TreeList )
		d->dlg->unfoldTreeList();
}

void Dialog::configureTree()
{
	kdDebug( 700 ) << k_funcinfo << endl;
	ComponentsDialog * subdlg = new ComponentsDialog( d->dlg );
	subdlg->setPluginInfos( d->plugininfomap );
	subdlg->show();
	connect( subdlg, TQ_SIGNAL( okClicked() ), this, TQ_SLOT( updateTreeList() ) );
	connect( subdlg, TQ_SIGNAL( applyClicked() ), this, TQ_SLOT( updateTreeList() ) );
	connect( subdlg, TQ_SIGNAL( okClicked() ), this,
			TQ_SIGNAL( pluginSelectionChanged() ) );
	connect( subdlg, TQ_SIGNAL( applyClicked() ), this,
			TQ_SIGNAL( pluginSelectionChanged() ) );
	connect( subdlg, TQ_SIGNAL( finished() ), subdlg, TQ_SLOT( delayedDestruct() ) );
}

void Dialog::updateTreeList()
{
	kdDebug( 700 ) << k_funcinfo << endl;

	d->pagetree.makeDirty();

	// remove all pages from the dialog and then add them again. This is needed
	// because KDialogBase/KJanusWidget can only append to the end of the list
	// and we need to have a predefined order.

	d->pagetree.removeFromDialog( d->dlg );
	d->pagetree.addToDialog( d->dlg );

	if( d->pagetree.needTree() )
		d->dlg->unfoldTreeList( true );
}

} //namespace

#include "dialog.moc"
