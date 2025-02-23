//----------------------------------------------------------------------------
//    filename             : tdemditoolviewaccessor.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 08/2003       by Joseph Wenninger (jowenn@kde.org)
//    changes              : ---
//    patches              : ---
//
//    copyright            : (C) 2003 by Joseph Wenninger (jowenn@kde.org)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef NO_KDE
# include <kdebug.h>
#endif
#include "tdemditoolviewaccessor_p.h"
#include "tdemdiguiclient.h"
#include "tdemdimainfrm.h"

#include "tdemditoolviewaccessor.h"
#include "tdemditoolviewaccessor_p.h"

KMdiToolViewAccessor::KMdiToolViewAccessor( KMdiMainFrm *parent, TQWidget *widgetToWrap, const TQString& tabToolTip, const TQString& tabCaption )
		: TQObject( parent )
{
	mdiMainFrm = parent;
	d = new KMdiToolViewAccessorPrivate();
	if ( widgetToWrap->inherits( "KDockWidget" ) )
	{
		d->widgetContainer = dynamic_cast<KDockWidget*>( widgetToWrap );
		d->widget = d->widgetContainer->getWidget();
	}
	else
	{
		d->widget = widgetToWrap;
		TQString finalTabCaption;
		if ( tabCaption == 0 )
		{
			finalTabCaption = widgetToWrap->caption();
			if ( finalTabCaption.isEmpty() && !widgetToWrap->icon() )
			{
				finalTabCaption = widgetToWrap->name();
			}
		}
		else
		{
			finalTabCaption = tabCaption;
		}
		d->widgetContainer = parent->createDockWidget( widgetToWrap->name(),
		                     ( widgetToWrap->icon() ? ( *( widgetToWrap->icon() ) ) : TQPixmap() ),
		                     0L,   // parent
		                     widgetToWrap->caption(),
		                     finalTabCaption );
		d->widgetContainer->setWidget( widgetToWrap );
		if ( tabToolTip != 0 )
		{
			d->widgetContainer->setToolTipString( tabToolTip );
		} 
	}
	//mdiMainFrm->m_pToolViews->insert(d->widget,this);
	if ( mdiMainFrm->m_mdiGUIClient )
		mdiMainFrm->m_mdiGUIClient->addToolView( this );
	else
		kdDebug( 760 ) << "mdiMainFrm->m_mdiGUIClient == 0 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;

	d->widget->installEventFilter( this );
}

KMdiToolViewAccessor::KMdiToolViewAccessor( KMdiMainFrm *parent )
{
	mdiMainFrm = parent;
	d = new KMdiToolViewAccessorPrivate();
}

KMdiToolViewAccessor::~KMdiToolViewAccessor()
{
	if ( mdiMainFrm->m_pToolViews )
		mdiMainFrm->m_pToolViews->remove
		( d->widget );
	delete d;

}

TQWidget *KMdiToolViewAccessor::wrapperWidget()
{
	if ( !d->widgetContainer )
	{
		d->widgetContainer = mdiMainFrm->createDockWidget( "KMdiToolViewAccessor::null", TQPixmap() );
		connect( d->widgetContainer, TQ_SIGNAL( widgetSet( TQWidget* ) ), this, TQ_SLOT( setWidgetToWrap( TQWidget* ) ) );
	}
	return d->widgetContainer;
}

TQWidget *KMdiToolViewAccessor::wrappedWidget()
{
	return d->widget;
}


void KMdiToolViewAccessor::setWidgetToWrap( TQWidget *widgetToWrap, const TQString& tabToolTip, const TQString& tabCaption )
{
	Q_ASSERT( !( d->widget ) );
	Q_ASSERT( !widgetToWrap->inherits( "KDockWidget" ) );
	disconnect( d->widgetContainer, TQ_SIGNAL( widgetSet( TQWidget* ) ), this, TQ_SLOT( setWidgetToWrap( TQWidget* ) ) );
	delete d->widget;
	d->widget = widgetToWrap;
	KDockWidget *tmp = d->widgetContainer;

	TQString finalTabCaption;
	if ( tabCaption == 0 )
	{
		finalTabCaption = widgetToWrap->caption();
		if ( finalTabCaption.isEmpty() && !widgetToWrap->icon() )
		{
			finalTabCaption = widgetToWrap->name();
		}
	}
	else
	{
		finalTabCaption = tabCaption;
	}

	if ( !tmp )
	{
		tmp = mdiMainFrm->createDockWidget( widgetToWrap->name(),
		                                    widgetToWrap->icon() ? ( *( widgetToWrap->icon() ) ) : TQPixmap(),
		                                    0L,   // parent
		                                    widgetToWrap->caption(),
		                                    finalTabCaption );
		d->widgetContainer = tmp;
		if ( tabToolTip != 0 )
		{
			d->widgetContainer->setToolTipString( tabToolTip );
		}
	}
	else
	{
		tmp->setCaption( widgetToWrap->caption() );
		tmp->setTabPageLabel( finalTabCaption );
		tmp->setPixmap( widgetToWrap->icon() ? ( *( widgetToWrap->icon() ) ) : TQPixmap() );
		tmp->setName( widgetToWrap->name() );
		if ( tabToolTip != 0 )
		{
			d->widgetContainer->setToolTipString( tabToolTip );
		}
	}
	tmp->setWidget( widgetToWrap );
	mdiMainFrm->m_pToolViews->insert( widgetToWrap, this );
	if ( mdiMainFrm->m_mdiGUIClient )
		mdiMainFrm->m_mdiGUIClient->addToolView( this );
	else
		kdDebug( 760 ) << "mdiMainFrm->m_mdiGUIClient == 0 <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<" << endl;

	d->widget->installEventFilter( this );
}


bool KMdiToolViewAccessor::eventFilter( TQObject *, TQEvent *e )
{
	if ( e->type() == TQEvent::IconChange )
	{
		d->widgetContainer->setPixmap( d->widget->icon() ? ( *d->widget->icon() ) : TQPixmap() );
	}
	return false;
}

void KMdiToolViewAccessor::placeAndShow( KDockWidget::DockPosition pos, TQWidget* pTargetWnd , int percent )
{
	place( pos, pTargetWnd, percent );
	show();
}
void KMdiToolViewAccessor::place( KDockWidget::DockPosition pos, TQWidget* pTargetWnd , int percent )
{
	Q_ASSERT( d->widgetContainer );
	if ( !d->widgetContainer )
		return ;
	if ( pos == KDockWidget::DockNone )
	{
		d->widgetContainer->setEnableDocking( KDockWidget::DockNone );
		d->widgetContainer->reparent( mdiMainFrm, (WFlags)(WType_TopLevel | WType_Dialog), TQPoint( 0, 0 ), true ); //pToolView->isVisible());
	}
	else
	{   // add (and dock) the toolview as DockWidget view

		KDockWidget* pCover = d->widgetContainer;

		KDockWidget* pTargetDock = 0L;
		if ( pTargetWnd->inherits( "KDockWidget" ) || pTargetWnd->inherits( "KDockWidget_Compat::KDockWidget" ) )
		{
			pTargetDock = ( KDockWidget* ) pTargetWnd;
		}

		// Should we dock to ourself?
		bool DockToOurself = false;
		if ( mdiMainFrm->m_pDockbaseAreaOfDocumentViews )
		{
			if ( pTargetWnd == mdiMainFrm->m_pDockbaseAreaOfDocumentViews->getWidget() )
			{
				DockToOurself = true;
				pTargetDock = mdiMainFrm->m_pDockbaseAreaOfDocumentViews;
			}
			else if ( pTargetWnd == mdiMainFrm->m_pDockbaseAreaOfDocumentViews )
			{
				DockToOurself = true;
				pTargetDock = mdiMainFrm->m_pDockbaseAreaOfDocumentViews;
			}
		}
		// this is not inheriting TQWidget*, its plain impossible that this condition is true
		//if (pTargetWnd == this) DockToOurself = true;
		if ( !DockToOurself )
			if ( pTargetWnd != 0L )
			{
				pTargetDock = mdiMainFrm->dockManager->findWidgetParentDock( pTargetWnd );
				if ( !pTargetDock )
				{
					if ( pTargetWnd->parentWidget() )
					{
						pTargetDock = mdiMainFrm->dockManager->findWidgetParentDock( pTargetWnd->parentWidget() );
					}
				}
			}
		if ( !pTargetDock || pTargetWnd == mdiMainFrm->getMainDockWidget() )
		{
			if ( mdiMainFrm->m_managedDockPositionMode && ( mdiMainFrm->m_pMdi || mdiMainFrm->m_documentTabWidget ) )
			{
				KDockWidget * dw1 = pTargetDock->findNearestDockWidget( pos );
				if ( dw1 )
					pCover->manualDock( dw1, KDockWidget::DockCenter, percent );
				else
					pCover->manualDock ( pTargetDock, pos, 20 );
				return ;
			}
		}
		pCover->manualDock( pTargetDock, pos, percent );
		//check      pCover->show();
	}
}

void KMdiToolViewAccessor::hide()
{
	Q_ASSERT( d->widgetContainer );
	if ( !d->widgetContainer )
		return ;
	d->widgetContainer->undock();
}

void KMdiToolViewAccessor::show()
{
	Q_ASSERT( d->widgetContainer );
	if ( !d->widgetContainer )
		return ;
	d->widgetContainer->makeDockVisible();
}


#ifndef NO_INCLUDE_MOCFILES
#include "tdemditoolviewaccessor.moc"
#endif 
