//----------------------------------------------------------------------------
//    filename             : tdemdichildview.cpp
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create a
//                           -06/2000      stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                           02/2001       by Eva Brucherseifer (eva@rt.e-technik.tu-darmstadt.de)
//                           01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2003 by Szymon Stefanek (stefanek@tin.it)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
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

#include "tdemdichildview.h"
#include "tdemdichildview.moc"

#include <tqdatetime.h>
#include <tqobjectlist.h>

#include "tdemdimainfrm.h"
#include "tdemdichildfrm.h"
#include "tdemdidefines.h"
#include <kdebug.h>
#include <tdelocale.h>
#include <tqiconset.h>

//============ KMdiChildView ============//

KMdiChildView::KMdiChildView( const TQString& caption, TQWidget* parentWidget, const char* name, WFlags f )
	: TQWidget( parentWidget, name, f )
	, m_focusedChildWidget( 0L )
	, m_firstFocusableChildWidget( 0L )
	, m_lastFocusableChildWidget( 0L )
	, m_stateChanged( true )
	, m_bToolView( false )
	, m_bInterruptActivation( false )
	, m_bMainframesActivateViewIsPending( false )
	, m_bFocusInEventIsPending( false )
	, m_trackChanges( 0 )
{
	setGeometry( 0, 0, 0, 0 );  // reset
	if ( caption != 0L )
		m_szCaption = caption;
	else
		m_szCaption = i18n( "Unnamed" );
	
	m_sTabCaption = m_szCaption;
	setFocusPolicy( TQWidget::ClickFocus );
	installEventFilter( this );
	
	// store the current time
	updateTimeStamp();
}


//============ KMdiChildView ============//

KMdiChildView::KMdiChildView( TQWidget* parentWidget, const char* name, WFlags f )
	: TQWidget( parentWidget, name, f )
	, m_focusedChildWidget( 0L )
	, m_firstFocusableChildWidget( 0L )
	, m_lastFocusableChildWidget( 0L )
	, m_stateChanged( true )
	, m_bToolView( false )
	, m_bInterruptActivation( false )
	, m_bMainframesActivateViewIsPending( false )
	, m_bFocusInEventIsPending( false )
	, m_trackChanges( 0 )
{
	setGeometry( 0, 0, 0, 0 );  // reset
	m_szCaption = i18n( "Unnamed" );
	m_sTabCaption = m_szCaption;
	setFocusPolicy( TQWidget::ClickFocus );
	installEventFilter( this );

	// store the current time
	updateTimeStamp();
}

//============ ~KMdiChildView ============//

KMdiChildView::~KMdiChildView()
{
	kdDebug( 760 ) << k_funcinfo << endl;
}

void KMdiChildView::trackIconAndCaptionChanges( TQWidget *view )
{
	m_trackChanges = view;
}


//============== internal geometry ==============//

TQRect KMdiChildView::internalGeometry() const
{
	if ( mdiParent() )
	{ // is attached
		// get the client area coordinates inside the MDI child frame
		TQRect posInFrame = geometry();
		// map these values to the parent of the MDI child frame
		// (this usually is the MDI child area) and return
		TQPoint ptTopLeft = mdiParent() ->mapToParent( posInFrame.topLeft() );
		TQSize sz = size();
		return TQRect( ptTopLeft, sz );
	}
	else
	{
		TQRect geo = geometry();
		TQRect frameGeo = externalGeometry();
		return TQRect( frameGeo.x(), frameGeo.y(), geo.width(), geo.height() );
		//      return geometry();
	}
}

//============== set internal geometry ==============//

void KMdiChildView::setInternalGeometry( const TQRect& newGeometry )
{
	if ( mdiParent() )
	{ // is attached
		// retrieve the frame size
		TQRect geo = internalGeometry();
		TQRect frameGeo = externalGeometry();
		int nFrameSizeTop = geo.y() - frameGeo.y();
		int nFrameSizeLeft = geo.x() - frameGeo.x();

		// create the new geometry that is accepted by the TQWidget::setGeometry() method
		TQRect newGeoQt;
		newGeoQt.setX( newGeometry.x() - nFrameSizeLeft );
		newGeoQt.setY( newGeometry.y() - nFrameSizeTop );

		newGeoQt.setWidth( newGeometry.width() + nFrameSizeLeft + KMDI_CHILDFRM_DOUBLE_BORDER / 2 );
		newGeoQt.setHeight( newGeometry.height() + nFrameSizeTop + KMDI_CHILDFRM_DOUBLE_BORDER / 2 );
		//      newGeoQt.setWidth(newGeometry.width()+KMDI_MDI_CHILDFRM_DOUBLE_BORDER);
		//      newGeoQt.setHeight(newGeometry.height()+mdiParent()->captionHeight()+KMDI_MDI_CHILDFRM_DOUBLE_BORDER);

		// set the geometry
		mdiParent()->setGeometry( newGeoQt );
	}
	else
	{
		// retrieve the frame size
		TQRect geo = internalGeometry();
		TQRect frameGeo = externalGeometry();
		int nFrameSizeTop = geo.y() - frameGeo.y();
		int nFrameSizeLeft = geo.x() - frameGeo.x();

		// create the new geometry that is accepted by the TQWidget::setGeometry() method
		TQRect newGeoQt;

		newGeoQt.setX( newGeometry.x() - nFrameSizeLeft );
		newGeoQt.setY( newGeometry.y() - nFrameSizeTop );

		newGeoQt.setWidth( newGeometry.width() );
		newGeoQt.setHeight( newGeometry.height() );

		// set the geometry
		setGeometry( newGeoQt );
	}
}

//============== external geometry ==============//

TQRect KMdiChildView::externalGeometry() const
{
	return mdiParent() ? mdiParent()->frameGeometry() : frameGeometry();
}

//============== set external geometry ==============//

void KMdiChildView::setExternalGeometry( const TQRect& newGeometry )
{
	if ( mdiParent() )
	{ // is attached
		mdiParent() ->setGeometry( newGeometry );
	}
	else
	{
		// retrieve the frame size
		TQRect geo = internalGeometry();
		TQRect frameGeo = externalGeometry();
		int nTotalFrameWidth = frameGeo.width() - geo.width();
		int nTotalFrameHeight = frameGeo.height() - geo.height();
		int nFrameSizeTop = geo.y() - frameGeo.y();
		int nFrameSizeLeft = geo.x() - frameGeo.x();

		// create the new geometry that is accepted by the TQWidget::setGeometry() method
		// not attached => the window system makes the frame
		TQRect newGeoQt;
		newGeoQt.setX( newGeometry.x() + nFrameSizeLeft );
		newGeoQt.setY( newGeometry.y() + nFrameSizeTop );
		newGeoQt.setWidth( newGeometry.width() - nTotalFrameWidth );
		newGeoQt.setHeight( newGeometry.height() - nTotalFrameHeight );

		// set the geometry
		setGeometry( newGeoQt );
	}
}

//============== minimize ==============//

void KMdiChildView::minimize( bool bAnimate )
{
	if ( mdiParent() )
	{
		if ( !isMinimized() )
		{
			mdiParent() ->setState( KMdiChildFrm::Minimized, bAnimate );
		}
	}
	else
		showMinimized();
}

void KMdiChildView::showMinimized()
{
	emit isMinimizedNow();
	TQWidget::showMinimized();
}

//slot:
void KMdiChildView::minimize()
{
	minimize( true );
}

//============= maximize ==============//

void KMdiChildView::maximize( bool bAnimate )
{
	if ( mdiParent() )
	{
		if ( !isMaximized() )
		{
			mdiParent() ->setState( KMdiChildFrm::Maximized, bAnimate );
			emit mdiParentNowMaximized( true );
		}
	}
	else
		showMaximized();
}

void KMdiChildView::showMaximized()
{
	emit isMaximizedNow();
	TQWidget::showMaximized();
}

//slot:
void KMdiChildView::maximize()
{
	maximize( true );
}

//============== restoreGeometry ================//

TQRect KMdiChildView::restoreGeometry()
{
	if ( mdiParent() )
		return mdiParent() ->restoreGeometry();
	else //FIXME not really supported, may be we must use Windows or X11 funtions
		return geometry();
}

//============== setRestoreGeometry ================//

void KMdiChildView::setRestoreGeometry( const TQRect& newRestGeo )
{
	if ( mdiParent() )
		mdiParent()->setRestoreGeometry( newRestGeo );
}

//============== attach ================//

void KMdiChildView::attach()
{
	emit attachWindow( this, true );
}

//============== detach =================//

void KMdiChildView::detach()
{
	emit detachWindow( this, true );
}

//=============== isMinimized ? =================//

bool KMdiChildView::isMinimized() const
{
	if ( mdiParent() )
		return ( mdiParent()->state() == KMdiChildFrm::Minimized );
	else
		return TQWidget::isMinimized();
}

//============== isMaximized ? ==================//

bool KMdiChildView::isMaximized() const
{
	if ( mdiParent() )
		return ( mdiParent()->state() == KMdiChildFrm::Maximized );
	else
		return TQWidget::isMaximized();
}

//============== restore ================//

void KMdiChildView::restore()
{
	if ( mdiParent() )
	{
		if ( isMaximized() )
			emit mdiParentNowMaximized( false );
		
		if ( isMinimized() || isMaximized() )
			mdiParent()->setState( KMdiChildFrm::Normal );
	}
	else
		showNormal();
}

void KMdiChildView::showNormal()
{
	emit isRestoredNow();
	TQWidget::showNormal();
}

//=============== youAreAttached ============//

void KMdiChildView::youAreAttached( KMdiChildFrm *lpC )
{
	lpC->setCaption( m_szCaption );
	emit isAttachedNow();
}

//================ youAreDetached =============//

void KMdiChildView::youAreDetached()
{
	setCaption( m_szCaption );

	setTabCaption( m_sTabCaption );
	if ( myIconPtr() )
		setIcon( *( myIconPtr() ) );
	
	setFocusPolicy( TQWidget::StrongFocus );

	emit isDetachedNow();
}

//================ setCaption ================//
// this set the caption of only the window
void KMdiChildView::setCaption( const TQString& szCaption )
{
	// this will work only for window
	m_szCaption = szCaption;
	if ( mdiParent() )
		mdiParent() ->setCaption( m_szCaption );
	else //have to call the parent one
		TQWidget::setCaption( m_szCaption );

	emit windowCaptionChanged( m_szCaption );
}

//============== closeEvent ================//

void KMdiChildView::closeEvent( TQCloseEvent *e )
{
	e->ignore(); //we ignore the event , and then close later if needed.
	emit childWindowCloseRequest( this );
}

//================ myIconPtr =================//

TQPixmap* KMdiChildView::myIconPtr()
{
	return 0;
}

//============= focusInEvent ===============//

void KMdiChildView::focusInEvent( TQFocusEvent *e )
{
	TQWidget::focusInEvent( e );

	// every widget get a focusInEvent when a popup menu is opened!?! -> maybe bug of QT
	if ( e && ( ( e->reason() ) == TQFocusEvent::Popup ) )
		return ;


	m_bFocusInEventIsPending = true;
	activate();
	m_bFocusInEventIsPending = false;

	emit gotFocus( this );
}

//============= activate ===============//

void KMdiChildView::activate()
{
	// avoid circularity
	static bool s_bActivateIsPending = false;
	if ( s_bActivateIsPending )
		return ;
	
	s_bActivateIsPending = true;

	// raise the view and push the taskbar button
	if ( !m_bMainframesActivateViewIsPending )
		emit focusInEventOccurs( this );

	// if this method was called directly, check if the mainframe wants that we interrupt
	if ( m_bInterruptActivation )
		m_bInterruptActivation = false;
	else
	{
		if ( !m_bFocusInEventIsPending )
			setFocus();
		
		kdDebug( 760 ) << k_funcinfo << endl;
		emit activated( this );
	}

	if ( m_focusedChildWidget != 0L )
		m_focusedChildWidget->setFocus();
	else
	{
		if ( m_firstFocusableChildWidget != 0L )
		{
			m_firstFocusableChildWidget->setFocus();
			m_focusedChildWidget = m_firstFocusableChildWidget;
		}
	}
	s_bActivateIsPending = false;
}

//============= focusOutEvent ===============//

void KMdiChildView::focusOutEvent( TQFocusEvent* e )
{
	TQWidget::focusOutEvent( e );
	emit lostFocus( this );
}

//============= resizeEvent ===============//

void KMdiChildView::resizeEvent( TQResizeEvent* e )
{
	TQWidget::resizeEvent( e );

	if ( m_stateChanged )
	{
		m_stateChanged = false;
		if ( isMaximized() )
		{ //maximized
			emit isMaximizedNow();
		}
		else if ( isMinimized() )
		{ //minimized
			emit isMinimizedNow();
		}
		else
		{ //is restored
			emit isRestoredNow();
		}
	}
}

void KMdiChildView::slot_childDestroyed()
{
	// do what we do if a child is removed

	// if we lost a child we uninstall ourself as event filter for the lost
	// child and its children
	const TQObject * pLostChild = sender();
	if ( pLostChild && ( pLostChild->isWidgetType() ) )
	{
		TQObjectList* list = ( ( TQObject* ) ( pLostChild ) ) ->queryList( "TQWidget" );
		list->insert( 0, pLostChild );        // add the lost child to the list too, just to save code
		TQObjectListIt it( *list );          // iterate over all lost child widgets
		TQObject* obj;
		while ( ( obj = it.current() ) != 0 )
		{ // for each found object...
			TQWidget * widg = ( TQWidget* ) obj;
			++it;
			widg->removeEventFilter( this );
			if ( m_firstFocusableChildWidget == widg )
				m_firstFocusableChildWidget = 0L;   // reset first widget
			
			if ( m_lastFocusableChildWidget == widg )
				m_lastFocusableChildWidget = 0L;    // reset last widget
			
			if ( m_focusedChildWidget == widg )
				m_focusedChildWidget = 0L;          // reset focused widget
		}
		delete list;                        // delete the list, not the objects
	}
}

//============= eventFilter ===============//
bool KMdiChildView::eventFilter( TQObject *obj, TQEvent *e )
{
	if ( e->type() == TQEvent::KeyPress && isAttached() )
	{
		TQKeyEvent* ke = ( TQKeyEvent* ) e;
		if ( ke->key() == TQt::Key_Tab )
		{
			TQWidget* w = ( TQWidget* ) obj;
			TQWidget::FocusPolicy wfp = w->focusPolicy();
			if ( wfp == TQWidget::StrongFocus || wfp == TQWidget::TabFocus || w->focusPolicy() == TQWidget::WheelFocus )
			{
				if ( m_lastFocusableChildWidget != 0 )
				{
					if ( w == m_lastFocusableChildWidget )
					{
						if ( w != m_firstFocusableChildWidget )
							m_firstFocusableChildWidget->setFocus();
					}
				}
			}
		}
	}
	else if ( e->type() == TQEvent::FocusIn )
	{
		if ( obj->isWidgetType() )
		{
			TQObjectList * list = queryList( "TQWidget" );
			if ( list->find( obj ) != -1 )
				m_focusedChildWidget = ( TQWidget* ) obj;

			delete list;   // delete the list, not the objects
		}
		if ( !isAttached() )
		{   // is toplevel, for attached views activation is done by main frame event filter
			static bool m_bActivationIsPending = false;
			if ( !m_bActivationIsPending )
			{
				m_bActivationIsPending = true;
				activate(); // sets the focus
				m_bActivationIsPending = false;
			}
		}
	}
	else if ( e->type() == TQEvent::ChildRemoved )
	{
		// if we lost a child we uninstall ourself as event filter for the lost
		// child and its children
		TQObject * pLostChild = ( ( TQChildEvent* ) e ) ->child();
		if ( ( pLostChild != 0L ) && ( pLostChild->isWidgetType() ) )
		{
			TQObjectList * list = pLostChild->queryList( "TQWidget" );
			list->insert( 0, pLostChild );        // add the lost child to the list too, just to save code
			TQObjectListIt it( *list );          // iterate over all lost child widgets
			TQObject * o;
			while ( ( o = it.current() ) != 0 )
			{ // for each found object...
				TQWidget * widg = ( TQWidget* ) o;
				++it;
				widg->removeEventFilter( this );
				TQWidget::FocusPolicy wfp = widg->focusPolicy();
				if ( wfp == TQWidget::StrongFocus || wfp == TQWidget::TabFocus || widg->focusPolicy() == TQWidget::WheelFocus )
				{
					if ( m_firstFocusableChildWidget == widg )
						m_firstFocusableChildWidget = 0L;   // reset first widget
					
					if ( m_lastFocusableChildWidget == widg )
						m_lastFocusableChildWidget = 0L;    // reset last widget
				}
			}
			delete list;                        // delete the list, not the objects
		}
	}
	else if ( e->type() == TQEvent::ChildInserted )
	{
		// if we got a new child and we are attached to the MDI system we
		// install ourself as event filter for the new child and its children
		// (as we did when we were added to the MDI system).
		TQObject * pNewChild = ( ( TQChildEvent* ) e ) ->child();
		if ( ( pNewChild != 0L ) && ( pNewChild->isWidgetType() ) )
		{
			TQWidget * pNewWidget = ( TQWidget* ) pNewChild;
			if ( pNewWidget->testWFlags( (WFlags)(WType_Dialog | WShowModal) ) )
				return false;
			TQObjectList *list = pNewWidget->queryList( "TQWidget" );
			list->insert( 0, pNewChild );         // add the new child to the list too, just to save code
			TQObjectListIt it( *list );          // iterate over all new child widgets
			TQObject * o;
			while ( ( o = it.current() ) != 0 )
			{ // for each found object...
				TQWidget * widg = ( TQWidget* ) o;
				++it;
				widg->installEventFilter( this );
				connect( widg, TQ_SIGNAL( destroyed() ), this, TQ_SLOT( slot_childDestroyed() ) );
				TQWidget::FocusPolicy wfp = widg->focusPolicy();
				if ( wfp == TQWidget::StrongFocus || wfp == TQWidget::TabFocus || widg->focusPolicy() == TQWidget::WheelFocus )
				{
					if ( m_firstFocusableChildWidget == 0 )
						m_firstFocusableChildWidget = widg;  // first widge
					
					m_lastFocusableChildWidget = widg; // last widget
				}
			}
			delete list;                        // delete the list, not the objects
		}
	}
	else
	{
		if ( e->type() == TQEvent::IconChange )
		{
			//            tqDebug("KMDiChildView:: TQEvent:IconChange intercepted\n");
			if ( obj == this )
				iconUpdated( this, icon() ? ( *icon() ) : TQPixmap() );
			else if ( obj == m_trackChanges )
				setIcon( m_trackChanges->icon() ? ( *( m_trackChanges->icon() ) ) : TQPixmap() );
		}
		if ( e->type() == TQEvent::CaptionChange )
		{
			if ( obj == this )
				captionUpdated( this, caption() );
		}
	}

	return false;                           // standard event processing
}

/** Switches interposing in event loop of all current child widgets off. */
void KMdiChildView::removeEventFilterForAllChildren()
{
	TQObjectList* list = queryList( "TQWidget" );
	TQObjectListIt it( *list );          // iterate over all child widgets
	TQObject* obj;
	while ( ( obj = it.current() ) != 0 )
	{ // for each found object...
		TQWidget* widg = ( TQWidget* ) obj;
		++it;
		widg->removeEventFilter( this );
	}
	delete list;                        // delete the list, not the objects
}

TQWidget* KMdiChildView::focusedChildWidget()
{
	return m_focusedChildWidget;
}

void KMdiChildView::setFirstFocusableChildWidget( TQWidget* firstFocusableChildWidget )
{
	m_firstFocusableChildWidget = firstFocusableChildWidget;
}

void KMdiChildView::setLastFocusableChildWidget( TQWidget* lastFocusableChildWidget )
{
	m_lastFocusableChildWidget = lastFocusableChildWidget;
}

/** Set a new value of  the task bar button caption  */
void KMdiChildView::setTabCaption ( const TQString& stbCaption )
{
	m_sTabCaption = stbCaption;
	emit tabCaptionChanged( m_sTabCaption );
}

void KMdiChildView::setMDICaption ( const TQString& caption )
{
	setCaption( caption );
	setTabCaption( caption );
}

/** sets an ID  */
void KMdiChildView::setWindowMenuID( int id )
{
	m_windowMenuID = id;
}

//============= slot_clickedInWindowMenu ===============//

/** called if someone click on the "Window" menu item for this child frame window */
void KMdiChildView::slot_clickedInWindowMenu()
{
	updateTimeStamp();
	emit clickedInWindowMenu( m_windowMenuID );
}

//============= slot_clickedInDockMenu ===============//

/** called if someone click on the "Dock/Undock..." menu item for this child frame window */
void KMdiChildView::slot_clickedInDockMenu()
{
	emit clickedInDockMenu( m_windowMenuID );
}

//============= setMinimumSize ===============//

void KMdiChildView::setMinimumSize( int minw, int minh )
{
	TQWidget::setMinimumSize( minw, minh );
	if ( mdiParent() && mdiParent()->state() != KMdiChildFrm::Minimized )
	{
		mdiParent() ->setMinimumSize( minw + KMDI_CHILDFRM_DOUBLE_BORDER,
		                              minh + KMDI_CHILDFRM_DOUBLE_BORDER + KMDI_CHILDFRM_SEPARATOR + mdiParent() ->captionHeight() );
	}
}

//============= setMaximumSize ===============//

void KMdiChildView::setMaximumSize( int maxw, int maxh )
{
	if ( mdiParent() && mdiParent()->state() == KMdiChildFrm::Normal )
	{
		int w = maxw + KMDI_CHILDFRM_DOUBLE_BORDER;
		if ( w > TQWIDGETSIZE_MAX )
			w = TQWIDGETSIZE_MAX;

		int h = maxh + KMDI_CHILDFRM_DOUBLE_BORDER + KMDI_CHILDFRM_SEPARATOR + mdiParent() ->captionHeight();
		if ( h > TQWIDGETSIZE_MAX )
			h = TQWIDGETSIZE_MAX;

		mdiParent()->setMaximumSize( w, h );
	}
	TQWidget::setMaximumSize( maxw, maxh );
}

//============= show ===============//

void KMdiChildView::show()
{
	if ( mdiParent() )
		mdiParent()->show();

	TQWidget::show();
}

//============= hide ===============//

void KMdiChildView::hide()
{
	if ( mdiParent() )
		mdiParent()->hide();
	
	TQWidget::hide();
}

//============= raise ===============//

void KMdiChildView::raise()
{
	if ( mdiParent() )  //TODO Check Z-order
		mdiParent()->raise();

	TQWidget::raise();
}
