/* This file is part of the KDE libraries
  Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
  based on tdetoolbarhandler.cpp: Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

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

#include "tdemdiguiclient.h"
#include "tdemdiguiclient.moc"

#include <tqpopupmenu.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <tdetoolbar.h>
#include <tdemainwindow.h>
#include <tdelocale.h>
#include <tdeaction.h>
#include <tqstring.h>
#include <assert.h>
#include <kdebug.h>
#include <kdockwidget.h>
#include <tdeversion.h>
#include "tdemdimainfrm.h"
#include "tdemditoolviewaccessor.h"
#include "tdemditoolviewaccessor_p.h"
namespace
{
const char *actionListName = "show_tdemdi_document_tool_view_actions";

const char *guiDescription = ""
                             "<!DOCTYPE kpartgui><kpartgui name=\"KMDIViewActions\">"
                             "<MenuBar>"
                             "    <Menu name=\"view\">"
                             "        <ActionList name=\"%1\" />"
                             "    </Menu>"
                             "</MenuBar>"
                             "</kpartgui>";

const char *resourceFileName = "tdemdiviewactions.rc";

}


using namespace KMDIPrivate;



ToggleToolViewAction::ToggleToolViewAction( const TQString& text, const TDEShortcut& cut, KDockWidget *dw, KMdiMainFrm *mdiMainFrm,
        TQObject* parent, const char* name )
		: TDEToggleAction( text, cut, parent, name ), m_dw( dw ), m_mdiMainFrm( mdiMainFrm )
{
	if ( m_dw )
	{
		connect( this, TQ_SIGNAL( toggled( bool ) ), this, TQ_SLOT( slotToggled( bool ) ) );
		connect( m_dw->dockManager(), TQ_SIGNAL( change() ), this, TQ_SLOT( anDWChanged() ) );
		connect( m_dw, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slotWidgetDestroyed() ) );
		setChecked( m_dw->mayBeHide() );
	}
}


ToggleToolViewAction::~ToggleToolViewAction()
{
	unplugAll();
}

void ToggleToolViewAction::anDWChanged()
{
	if ( isChecked() && m_dw->mayBeShow() )
		setChecked( false );
	else if ( ( !isChecked() ) && m_dw->mayBeHide() )
		setChecked( true );
	else if ( isChecked() && ( m_dw->parentDockTabGroup() &&
	                           ( ( ::tqt_cast<KDockWidget*>( m_dw->parentDockTabGroup() ->
	                                                          parent() ) ) ->mayBeShow() ) ) )
		setChecked( false );
}


void ToggleToolViewAction::slotToggled( bool t )
{
	//  m_mw->mainDock->setDockSite( KDockWidget::DockCorner );

	if ( ( !t ) && m_dw->mayBeHide() )
		m_dw->undock();
	else
		if ( t && m_dw->mayBeShow() )
			m_mdiMainFrm->makeDockVisible( m_dw );

	//  m_mw->mainDock->setDockSite( KDockWidget::DockNone );
}

void ToggleToolViewAction::slotWidgetDestroyed()
{
	disconnect( m_dw->dockManager(), TQ_SIGNAL( change() ), this, TQ_SLOT( anDWChanged() ) );
	disconnect( this, TQ_SIGNAL( toggled( bool ) ), 0, 0 );
	unplugAll();
	deleteLater();
}


KMDIGUIClient::KMDIGUIClient( KMdiMainFrm* mdiMainFrm, bool showMDIModeAction, const char* name ) : TQObject( mdiMainFrm, name ),
		KXMLGUIClient( mdiMainFrm )
{
	m_mdiMode = KMdi::ChildframeMode;
	m_mdiMainFrm = mdiMainFrm;
	connect( mdiMainFrm->guiFactory(), TQ_SIGNAL( clientAdded( KXMLGUIClient * ) ),
	         this, TQ_SLOT( clientAdded( KXMLGUIClient * ) ) );

	/* re-use an existing resource file if it exists. can happen if the user launches the
	 * toolbar editor */
	/*
	setXMLFile( resourceFileName );
	*/

	if ( domDocument().documentElement().isNull() )
	{

		TQString completeDescription = TQString::fromLatin1( guiDescription )
		                              .arg( actionListName );

		setXML( completeDescription, false /*merge*/ );
	}

	if ( actionCollection() ->tdeaccel() == 0 )
		actionCollection() ->setWidget( mdiMainFrm );
	m_toolMenu = new TDEActionMenu( i18n( "Tool &Views" ), actionCollection(), "tdemdi_toolview_menu" );
	if ( showMDIModeAction )
	{
		m_mdiModeAction = new TDESelectAction( i18n( "MDI Mode" ), 0, actionCollection() );
		TQStringList modes;
		modes << i18n( "&Toplevel Mode" ) << i18n( "C&hildframe Mode" ) << i18n( "Ta&b Page Mode" ) << i18n( "I&DEAl Mode" );
		m_mdiModeAction->setItems( modes );
		connect( m_mdiModeAction, TQ_SIGNAL( activated( int ) ), this, TQ_SLOT( changeViewMode( int ) ) );
	}
	else
		m_mdiModeAction = 0;

	connect( m_mdiMainFrm, TQ_SIGNAL( mdiModeHasBeenChangedTo( KMdi::MdiMode ) ),
	         this, TQ_SLOT( mdiModeHasBeenChangedTo( KMdi::MdiMode ) ) );

	m_gotoToolDockMenu = new TDEActionMenu( i18n( "Tool &Docks" ), actionCollection(), "tdemdi_tooldock_menu" );
	m_gotoToolDockMenu->insert( new TDEAction( i18n( "Switch Top Dock" ), ALT + CTRL + SHIFT + Key_T, this, TQ_SIGNAL( toggleTop() ),
	                            actionCollection(), "tdemdi_activate_top" ) );
	m_gotoToolDockMenu->insert( new TDEAction( i18n( "Switch Left Dock" ), ALT + CTRL + SHIFT + Key_L, this, TQ_SIGNAL( toggleLeft() ),
	                            actionCollection(), "tdemdi_activate_left" ) );
	m_gotoToolDockMenu->insert( new TDEAction( i18n( "Switch Right Dock" ), ALT + CTRL + SHIFT + Key_R, this, TQ_SIGNAL( toggleRight() ),
	                            actionCollection(), "tdemdi_activate_right" ) );
	m_gotoToolDockMenu->insert( new TDEAction( i18n( "Switch Bottom Dock" ), ALT + CTRL + SHIFT + Key_B, this, TQ_SIGNAL( toggleBottom() ),
	                            actionCollection(), "tdemdi_activate_bottom" ) );
	m_gotoToolDockMenu->insert( new TDEActionSeparator( actionCollection(), "tdemdi_goto_menu_separator" ) );
	m_gotoToolDockMenu->insert( new TDEAction( i18n( "Previous Tool View" ), ALT + CTRL + Key_Left, m_mdiMainFrm, TQ_SLOT( prevToolViewInDock() ),
	                            actionCollection(), "tdemdi_prev_toolview" ) );
	m_gotoToolDockMenu->insert( new TDEAction( i18n( "Next Tool View" ), ALT + CTRL + Key_Right, m_mdiMainFrm, TQ_SLOT( nextToolViewInDock() ),
	                            actionCollection(), "tdemdi_next_toolview" ) );

	actionCollection() ->readShortcutSettings( "Shortcuts", kapp->config() );
}

KMDIGUIClient::~KMDIGUIClient()
{

	//     actionCollection()->writeShortcutSettings( "KMDI Shortcuts", kapp->config() );
	for ( uint i = 0;i < m_toolViewActions.count();i++ )
		disconnect( m_toolViewActions.at( i ), 0, this, 0 );

	m_toolViewActions.setAutoDelete( false );
	m_toolViewActions.clear();
	m_documentViewActions.setAutoDelete( false );
	m_documentViewActions.clear();
}

void KMDIGUIClient::changeViewMode( int id )
{
	switch ( id )
	{
	case 0:
		m_mdiMainFrm->switchToToplevelMode();
		break;
	case 1:
		m_mdiMainFrm->switchToChildframeMode();
		break;
	case 2:
		m_mdiMainFrm->switchToTabPageMode();
		break;
	case 3:
		m_mdiMainFrm->switchToIDEAlMode();
		break;
	default:
		Q_ASSERT( 0 );
	}
}

void KMDIGUIClient::setupActions()
{
	if ( !factory() || !m_mdiMainFrm )
		return ;

	//    BarActionBuilder builder( actionCollection(), m_mainWindow, m_toolBars );

	//    if ( !builder.needsRebuild() )
	//        return;


	unplugActionList( actionListName );

	//    m_actions.setAutoDelete( true );
	//    m_actions.clear();
	//    m_actions.setAutoDelete( false );

	//    m_actions = builder.create();

	//    m_toolBars = builder.toolBars();

	//    m_toolViewActions.append(new TDEAction( "TESTKMDIGUICLIENT", TQString::null, 0,
	//             this, TQ_SLOT(blah()),actionCollection(),"nothing"));

	TQPtrList<TDEAction> addList;
	if ( m_toolViewActions.count() < 3 )
		for ( uint i = 0;i < m_toolViewActions.count();i++ )
			addList.append( m_toolViewActions.at( i ) );
	else
		addList.append( m_toolMenu );
	if ( m_mdiMode == KMdi::IDEAlMode )
		addList.append( m_gotoToolDockMenu );
	if ( m_mdiModeAction )
		addList.append( m_mdiModeAction );
	kdDebug( 760 ) << "KMDIGUIClient::setupActions: plugActionList" << endl;
	plugActionList( actionListName, addList );

	//    connectToActionContainers();
}

void KMDIGUIClient::addToolView( KMdiToolViewAccessor* mtva )
{
	kdDebug( 760 ) << "*****void KMDIGUIClient::addToolView(KMdiToolViewAccessor* mtva)*****" << endl;
	//	kdDebug()<<"name: "<<mtva->wrappedWidget()->name()<<endl;
	TQString aname = TQString( "tdemdi_toolview_" ) + mtva->wrappedWidget() ->name();

	// try to read the action shortcut
	TDEShortcut sc;
	TDEConfig *cfg = kapp->config();
	TQString _grp = cfg->group();
	cfg->setGroup( "Shortcuts" );
	// 	if ( cfg->hasKey( aname ) )
	sc = TDEShortcut( cfg->readEntry( aname, "" ) );
	cfg->setGroup( _grp );
	TDEAction *a = new ToggleToolViewAction( i18n( "Show %1" ).arg( mtva->wrappedWidget() ->caption() ),
	                                       /*TQString::null*/sc, dynamic_cast<KDockWidget*>( mtva->wrapperWidget() ),
	                                       m_mdiMainFrm, actionCollection(), aname.latin1() );
#if KDE_IS_VERSION(3,2,90)

	( ( ToggleToolViewAction* ) a ) ->setCheckedState( TQString(i18n( "Hide %1" ).arg( mtva->wrappedWidget() ->caption() )) );
#endif

	connect( a, TQ_SIGNAL( destroyed( TQObject* ) ), this, TQ_SLOT( actionDeleted( TQObject* ) ) );
	m_toolViewActions.append( a );
	m_toolMenu->insert( a );
	mtva->d->action = a;

	setupActions();
}

void KMDIGUIClient::actionDeleted( TQObject* a )
{
	m_toolViewActions.remove( static_cast<TDEAction*>( a ) );
	/*	if (!m_toolMenu.isNull()) m_toolMenu->remove(static_cast<TDEAction*>(a));*/
	setupActions();
}


void KMDIGUIClient::clientAdded( KXMLGUIClient *client )
{
	if ( client == this )
		setupActions();
}


void KMDIGUIClient::mdiModeHasBeenChangedTo( KMdi::MdiMode mode )
{
	kdDebug( 760 ) << "KMDIGUIClient::mdiModeHasBennChangeTo" << endl;
	m_mdiMode = mode;
	if ( m_mdiModeAction )
	{
		switch ( mode )
		{
		case KMdi::ToplevelMode:
			m_mdiModeAction->setCurrentItem( 0 );
			break;
		case KMdi::ChildframeMode:
			m_mdiModeAction->setCurrentItem( 1 );
			break;
		case KMdi::TabPageMode:
			m_mdiModeAction->setCurrentItem( 2 );
			break;
		case KMdi::IDEAlMode:
			m_mdiModeAction->setCurrentItem( 3 );
			break;
		default:
			Q_ASSERT( 0 );
		}
	}
	setupActions();

}
