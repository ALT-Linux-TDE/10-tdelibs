//----------------------------------------------------------------------------
//    filename             : tdemdimainfrm.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                         : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           2000-2003     maintained by the KDevelop project
//    patches              : */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
//
//    copyright            : (C) 1999-2003 by Falk Brettschneider
//                                         and
//                                         Szymon Stefanek (stefanek@tin.it)
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

#ifndef _TDEMDIMAINFRM_H_
#define _TDEMDIMAINFRM_H_

#include <tdeparts/dockmainwindow.h>
#include <tdemenubar.h>
#include <tdepopupmenu.h>

#include <tqptrlist.h>
#include <tqrect.h>
#include <tqapplication.h>
#include <tqdom.h>
#include <tqguardedptr.h>

#include "tdemditaskbar.h"
#include "tdemdichildarea.h"
#include "tdemdichildview.h"
#include "tdemdiiterator.h"
#include "tdemdilistiterator.h"
#include "tdemdinulliterator.h"
#include "tdemditoolviewaccessor.h"

class TQTimer;
class TQPopupMenu;
class TQMenuBar;


class TQToolButton;

namespace KMDIPrivate
{
class KMDIGUIClient;
}

class KMdiDockContainer;
class KMdiMainFrmPrivate;

/**
 * @short Internal class
 *
 * This special event is needed because the view has to inform the main frame that it`s being closed.
 */
class KMDI_EXPORT KMdiViewCloseEvent : public TQCustomEvent
{
public:
	KMdiViewCloseEvent( KMdiChildView* pWnd ) : TQCustomEvent( TQEvent::User, pWnd ) {}
};

/**
 * \short Base class for all your special main frames.
 *
 * It contains the child frame area (QMainWindow's central widget) and a child view taskbar
 * for switching the MDI views. Most methods are virtual functions for later overriding.
 *
 * Basically, this class provides functionality for docking/undocking view windows and
 * manages the taskbar. Usually a developer will only need to know about this class and
 * \ref KMdiChildView.
 *
 * \par General usage
 *
 * Your program mainwidget should inherit KMdiMainFrm. Then usually you'll just need
 * addWindow() and removeWindowFromMdi() to control the views.
 * \code
 *		class MyMainWindow : public KMdiMainFrm
 *		{ .... };
 * \endcode
 *
 * to define your main window class and
 *
 * \code
 *		MyMainWindow mainframe;
 *		tqApp->setMainWidget(&mainframe);
 *		mainframe->addWindow(view1); // put it under MDI control
 *		mainframe->addWindow(view2);
 * \endcode
 *
 * when you wish to use your main window class. The above example also adds a few windows
 * to the frame.
 *
 * KMdiMainFrm will provide the "Window" menu needed in common MDI applications. Just
 * insert it in your main menu:
 *
 * \code
 * 		if ( !isFakingSDIApplication() )
 * 		{
 * 			menuBar()->insertItem( i18n( "&Window" ), windowMenu() );
 * 		}
 * \endcode
 *
 * To synchronize the positions of the MDI control buttons inserted in your mainmenu:
 * \code
 * 		void B_MainModuleWidget::initMenuBar()
 * 		{
 * 			setMenuForSDIModeSysButtons( menuBar() );
 * 		}
 * 		...
 *		void B_MainModuleWidget::resizeEvent ( TQResizeEvent *e )
 *		{
 *			KMdiMainFrm::resizeEvent( e );
 *			setSysButtonsAtMenuPosition();
 *		}
 * \endcode
 *
 * \par Dynamic mode switching
 *
 * Dynamic switching of the MDI mode can be done via the following functions:
 * - switchToChildframeMode()
 * - switchToToplevelMode()
 * - switchToTabPageMode()
 * - switchToIDEAlMode()
 *
 * The MDI mode can be gotten using mdiMode(). If you need to fake the look of an SDI application
 * use fakeSDIApplication() to fake it and isFakingSDIApplication() to query whether or not an SDI
 * interface is being faked.
 *
 * You can dynamically change the shape of the attached MDI views using setFrameDecorOfAttachedViews().
 *
 * Additionally, here's a hint how to restore the mainframe's settings from config file:
 * \code
 *
 *    // restore MDI mode (toplevel, childframe, tabpage)
 *    int mdiMode = config->readIntEntry( "mainmodule session", "MDI mode", KMdi::ChildframeMode);
 *    switch (mdiMode) {
 *    case KMdi::ToplevelMode:
 *       {
 *          int childFrmModeHt = config->readIntEntry( "mainmodule session", "Childframe mode height", desktop()->height() - 50);
 *          mainframe->resize( m_pMdiMainFrm->width(), childFrmModeHt);
 *          mainframe->switchToToplevelMode();
 *       }
 *       break;
 *    case KMdi::ChildframeMode:
 *       break;
 *    case KMdi::TabPageMode:
 *       {
 *          int childFrmModeHt = m_pCfgFileManager->readIntEntry( "mainmodule session", "Childframe mode height", desktop()->height() - 50);
 *          mainframe->resize( m_pMdiMainFrm->width(), childFrmModeHt);
 *          mainframe->switchToTabPageMode();
 *       }
 *       break;
 *    default:
 *       break;
 *    }
 *
 *    // restore a possible maximized Childframe mode
 *    bool maxChildFrmMode = config->readBoolEntry( "mainmodule session", "maximized childframes", true);
 *    mainframe->setEnableMaximizedChildFrmMode(maxChildFrmMode);
 * \endcode
 * The maximized-Childframe mode means that currently all views are maximized in Childframe mode's application desktop.
 *
 * \par Managing views
 *
 * This class provides placing algorithms in Childframe mode. The following is a list of the window placement functions
 * - tilePragma() - Tile the windows and allow them to overlap
 * - tileAnodine() - Tile the windows but don't allow them to overlap
 * - tileVertically() - Tile the windows vertically
 * - cascadeWindows() - cascade windows
 * - cascadeMaximized() - cascade windows and maximize their viewing area
 * - expandVertical() - expand all the windows to use the most amount of vertical space
 * - expandHorizontal() - expand all the windows to use the most amount of horizontal space
 *
 * activateView(KMdiChildView*) and activateView(int index) set the appropriate MDI child view as the active
 * one. It will be raised, will get an active MDI frame and will get the focus. Call activeView() to find out what the
 * current MDI view is.
 *
 * Use detachWindow() and attachWindow() for docking the MDI views to desktop and back.
 *
 * Connect accels of your program with activatePrevWin(), activateNextWin() and activateView(int index).
 *
 * Note: KMdiChildViews can be added in 2 meanings: Either as a normal child view (usually containing
 * user document views) or as a tool-view (usually containing status, info or control widgets).
 * The tool-views can be added as floating dockwidgets or as stay-on-top desktop windows in tool style.
 *
 * Also, pay attention to the fact that when you click on the close button of MDI views that their
 * close event should be redirected to closeWindow(). Otherwise the mainframe class will
 * not get noticed about the deleted view and a dangling pointer will remain in the MDI control. The
 * closeWindow() or the removeWindowFromMdi() method is for that issue. The difference is closeWindow()
 * deletes the view object. So if your application wants to control that by itself, call removeWindowFromMdi()
 * and call delete by yourself. See also KMdiChildView::closeEvent() for that issue.
 *
 * Here's an example how you can suggest things for the adding of views to the MDI control via flags:
 * \code
 *		m_mapOfMdiWidgets.insert( pWnd, mh );
 *		unsigned int mdiFlags = KMdi::StandardAdd;
 *
 * 		if ( !show )
 *			mdiFlags |= KMdi::Hide;
 *
 * 		if ( !attach )
 *			mdiFlags |= KMdi::Detach;
 *
 * 		if ( minimize )
 *			mdiFlags |= KMdi::Minimize;
 *
 * 		if ( bToolWindow)
 *			mdiFlags |= KMdi::ToolWindow;
 *
 *		if ( m_pMdiMainFrm->isFakingSDIApplication() )
 *		{
 *			if ( attach ) //fake an SDI app
 *				mdiFlags |= KMdi::Maximize;
 *		}
 *		else
 *		{
 *			m_pMdiMainFrm->addWindow( pWnd, TQPoint(20, 20), KMdi::AddWindowFlags(mdiFlags));
 *			return;
 *		}
 *		m_pMdiMainFrm->addWindow( pWnd, KMdi::AddWindowFlags(mdiFlags));
 * \endcode
 */
class KMDI_EXPORT KMdiMainFrm : public KParts::DockMainWindow
{
	friend class KMdiChildView;
	friend class KMdiTaskBar;
	TQ_OBJECT

	friend class KMdiToolViewAccessor;
	// attributes
protected:
	KMdi::MdiMode m_mdiMode;
	KMdiChildArea *m_pMdi;
	KMdiTaskBar *m_pTaskBar;
	TQPtrList<KMdiChildView> *m_pDocumentViews;
	TQMap<TQWidget*, KMdiToolViewAccessor*> *m_pToolViews;
	KMdiChildView *m_pCurrentWindow;
	TQPopupMenu *m_pWindowPopup;
	TQPopupMenu *m_pTaskBarPopup;
	TQPopupMenu *m_pWindowMenu;
	TQPopupMenu *m_pDockMenu;
	TQPopupMenu *m_pMdiModeMenu;
	TQPopupMenu *m_pPlacingMenu;
	KMenuBar *m_pMainMenuBar;

	TQPixmap *m_pUndockButtonPixmap;
	TQPixmap *m_pMinButtonPixmap;
	TQPixmap *m_pRestoreButtonPixmap;
	TQPixmap *m_pCloseButtonPixmap;

	TQToolButton *m_pUndock;
	TQToolButton *m_pMinimize;
	TQToolButton *m_pRestore;
	TQToolButton *m_pClose;
	TQPoint m_undockPositioningOffset;
	bool m_bMaximizedChildFrmMode;
	int m_oldMainFrmHeight;
	int m_oldMainFrmMinHeight;
	int m_oldMainFrmMaxHeight;
	static KMdi::FrameDecor m_frameDecoration;
	bool m_bSDIApplication;
	KDockWidget* m_pDockbaseAreaOfDocumentViews;
	TQDomDocument* m_pTempDockSession;
	bool m_bClearingOfWindowMenuBlocked;

	TQTimer* m_pDragEndTimer;

	bool m_bSwitching;

	KDockWidget* m_leftContainer;
	KDockWidget* m_rightContainer;
	KDockWidget* m_topContainer;
	KDockWidget* m_bottomContainer;


private:
	KMdiMainFrmPrivate* d;
	KMDIPrivate::KMDIGUIClient* m_mdiGUIClient;
	bool m_managedDockPositionMode;

	// methods
public:
#ifdef qdoc
	KMdiMainFrm( TQWidget* parentWidget, const char* name = "", KMdi::MdiMode mdiMode = KMdi::ChildframeMode, WFlags flags = WType_TopLevel | WDestructiveClose );
#else
	KMdiMainFrm( TQWidget* parentWidget, const char* name = "", KMdi::MdiMode mdiMode = KMdi::ChildframeMode, WFlags flags = (WFlags)(WType_TopLevel | WDestructiveClose) );
#endif
	virtual ~KMdiMainFrm();

	/**
	 * Control whether or not the standard MDI menu is displayed
	 * when a context menu is displayed
	 */
	void setStandardMDIMenuEnabled( bool showModeMenu = true );

	void setManagedDockPositionModeEnabled( bool enabled );

	/**
	 * Returns whether the application's MDI views are in maximized state or not.
	 */
	bool isInMaximizedChildFrmMode() { return m_bMaximizedChildFrmMode; }

	/**
	 * Returns the MDI mode. This can be one of the enumerations KMdi::MdiMode.
	 */
	KMdi::MdiMode mdiMode() { return m_mdiMode; }

	/**
	 * Returns the focused attached MDI view.
	 */
	KMdiChildView* activeWindow();

	/**
	 * Returns a popup menu filled according to the MDI view state. You can override this
	 * method to insert additional entries there. The popup menu is usually popuped when the user
	 * clicks with the right mouse button on a taskbar entry. The default entries are:
	 * Undock/Dock, Restore/Maximize/Minimize, Close and an empty sub-popup ( windowPopup() )
	 * menu called Operations.
	 */
	virtual TQPopupMenu * taskBarPopup( KMdiChildView *pWnd, bool bIncludeWindowPopup = false );

	/**
	 * Returns a popup menu with only a title "Window". You can fill it with own operations entries
	 * on the MDI view. This popup menu is inserted as last menu item in taskBarPopup() .
	 */
	virtual TQPopupMenu * windowPopup( KMdiChildView *pWnd, bool bIncludeTaskbarPopup = true );

	/**
	 * Called in the constructor (forces a resize of all MDI views)
	 */
	virtual void applyOptions();

	/**
	 * Returns the KMdiChildView belonging to the given caption string.
	 */
	KMdiChildView * findWindow( const TQString& caption );

	enum ExistsAs {DocumentView, ToolView, AnyView};
	/**
	 * Returns whether this MDI child view is under MDI control (using addWindow() ) or not.
	 */
	bool windowExists( KMdiChildView *pWnd, ExistsAs as );

	/**
	 * Catches certain Qt events and processes it here.
	 * Currently, here this catches only the KMdiViewCloseEvent (a KMdi user event) which is sent
	 * from itself in childWindowCloseRequest() right after a KMdiChildView::closeEvent() .
	 * The reason for this event to itself is simple: It just wants to break the function call stack.
	 * It continues the processing with calling closeWindow() .
	 * You see, a close() is translated to a closeWindow() .
	 * It is necessary that the main frame has to start an MDI view close action because it must
	 * remove the MDI view from MDI control, additionally.
	 *
	 * This method calls TQMainWindow::event , additionally.
	 */
	virtual bool event( TQEvent* e );

	/**
	 * If there's a main menubar given, it will create the 4 maximize mode buttons there (undock, minimize, restore, close).
	 */
	virtual void setSysButtonsAtMenuPosition();

	/**
	 * Returns the height of the taskbar.
	 */
	virtual int taskBarHeight() { return m_pTaskBar ? m_pTaskBar->height() : 0; }

	/**
	 * Sets an offset value that is used on detachWindow() . The undocked window
	 * is visually moved on the desktop by this offset.
	 */
	virtual void setUndockPositioningOffset( TQPoint offset ) { m_undockPositioningOffset = offset; }

	/**
	 * If you don't want to know about the inner structure of the KMdi system, you can use
	 * this iterator to handle with the MDI view list in a more abstract way.
	 * The iterator hides what special data structure is used in KMdi.
	 * The caller must delete the iterator once he does not need it anymore.	
	 */
	// FIXME And what exactly are we supposed to fix? -mattr
	KMdiIterator<KMdiChildView*>* createIterator()
	{
		if ( m_pDocumentViews == 0L )
		{
			return new KMdiNullIterator<KMdiChildView*>();
		}
		else
		{
			return new KMdiListIterator<KMdiChildView>( *m_pDocumentViews );
		}
	}

	/**
	 * Deletes an KMdiIterator created in the KMdi library (needed for the windows dll problem).
	 */
	void deleteIterator( KMdiIterator<KMdiChildView*>* pIt )
	{
		delete pIt;
	}

	/**
	 * Returns a popup menu that contains the MDI controlled view list.
	 * Additionally, this menu provides some placing actions for these views.
	 * Usually, you insert this popup menu in your main menubar as "Window" menu.
	 */
	TQPopupMenu* windowMenu() const  { return m_pWindowMenu; };

	/**
	 * Sets a background color for the MDI view area widget.
	 */
	virtual void setBackgroundColor( const TQColor &c ) { m_pMdi->setBackgroundColor( c ); }

	/**
	 * Sets a background pixmap for the MDI view area widget.
	 */
	virtual void setBackgroundPixmap( const TQPixmap &pm ) { m_pMdi->setBackgroundPixmap( pm ); }

	/**
	 * Sets a size that is used as the default size for a newly to the MDI system added KMdiChildView .
	 *  By default this size is 600x400. So all non-resized added MDI views appear in that size.
	 */
	void setDefaultChildFrmSize( const TQSize& sz ) { m_pMdi->m_defaultChildFrmSize = sz; }

	/**
	 * Returns the default size for a newly added KMdiChildView. See setDefaultChildFrmSize() .
	 */
	TQSize defaultChildFrmSize() { return m_pMdi->m_defaultChildFrmSize; }

	/**
	 * Do nothing when in Toplevel mode
	 */
	virtual void setMinimumSize( int minw, int minh );

	/**
	 * Returns the Childframe mode height of this. Makes only sense when in Toplevel mode.
	 */
	int childFrameModeHeight() { return m_oldMainFrmHeight; };
	/**
	 * Tells the MDI system a QMenu where it can insert buttons for
	 * the system menu, undock, minimize, restore actions.
	 * If no such menu is given, KMdi simply overlays the buttons
	 * at the upper right-hand side of the main widget.
	 */
	virtual void setMenuForSDIModeSysButtons( KMenuBar* menuBar = 0 );

	/**
	 * @return the decoration of the window frame of docked (attached) MDI views
	 */
	static int frameDecorOfAttachedViews() { return m_frameDecoration; }

	/**
	 * An SDI application user interface is faked:
	 * @li an opened view is always maximized
	 * @li buttons for maximized childframe mode aren't inserted in the main menubar
	 * @li taskbar and windowmenu are not created/updated
	 */
	void fakeSDIApplication();

	/**
	 * @returns if we are faking an SDI application (fakeSDIApplication())
	 */
	bool isFakingSDIApplication() const { return m_bSDIApplication; }

	virtual bool eventFilter( TQObject *obj, TQEvent *e );
	void findRootDockWidgets( TQPtrList<KDockWidget>* pRootDockWidgetList, TQValueList<TQRect>* pPositionList );

	/** We're switching something.*/
	void setSwitching( const bool switching ) { m_bSwitching = switching; }
	bool switching( void ) const { return m_bSwitching; }

public slots:
	/**
	 * addWindow demands a KMdiChildView. This method wraps every TQWidget in such an object and
	 * this way you can put every widget under MDI control.
	 */
	KMdiChildView* createWrapper( TQWidget *view, const TQString& name, const TQString& shortName );

	/**
	 * Adds a KMdiChildView to the MDI system. The main frame takes control of it.
	 * \param pWnd the parent view.
	 * \param flags the flags for the view such as:
	 * \li whether the view should be attached or detached.
	 * \li whether the view should be shown or hidden
	 * \li whether the view should be maximized, minimized or restored (normalized)
	 * \li whether the view should be added as tool view (stay-on-top and toplevel) or added as document-type view.
	 */
	virtual void addWindow( KMdiChildView* pWnd, int flags = KMdi::StandardAdd );

	//KDE4: merge the two methods
	/**
	 * Adds a KMdiChildView to the MDI system. The main frame takes control of it.
	 * \param pWnd the parent view.
	 * \param flags
	 * You can specify here whether:
	 * \li the view should be attached or detached.
	 * \li shown or hidden
	 * \li maximized, minimized or restored (normalized)
	 * \li added as tool view (stay-on-top and toplevel) or added as
	 * document-type view.
	 * \param index the index of the tab we should insert the new tab after.
	 * If index == -1 then the tab will just be appended to the end.
	 * Using this parameter in childview mode has no effect.
	 * \since 3.3
	 */
	void addWindow( KMdiChildView* pWnd, int flags, int index );

	/**
	 * Adds a KMdiChildView to the MDI system. The main frame takes control of it.
	 * \param pWnd the parent view.
	 * \param pos move the child view to the specified position
	 * \param flags the flags for the view such as:
	 * \li whether the view should be attached or detached.
	 * \li whether the view should be shown or hidden
	 * \li whether the view should be maximized, minimized or restored (normalized)
	 * \li whether the view should be added as tool view (stay-on-top and toplevel) or
	 * added as document-type view.
	 */
	virtual void addWindow( KMdiChildView* pWnd, TQPoint pos, int flags = KMdi::StandardAdd );

	/**
	 * Adds a KMdiChildView to the MDI system. The main frame takes control of it.
	 * \param pWnd the parent view.
	 * \param rectNormal Sets the geometry for this child view
	 * \param flags the flags for the view such as:
	 * \li whether the view should be attached or detached.
	 * \li whether the view should be shown or hidden
	 * \li whether the view should be maximized, minimized or restored (normalized)
	 * \li whether the view should be added as tool view (stay-on-top and toplevel) or
	 * added as document-type view.
	 */
	virtual void addWindow( KMdiChildView* pWnd, TQRect rectNormal, int flags = KMdi::StandardAdd );

	/**
	 * Usually called from addWindow() when adding a tool view window. It reparents the given widget
	 * as toplevel and stay-on-top on the application's main widget.
	 */
	virtual KMdiToolViewAccessor *addToolWindow( TQWidget* pWnd, KDockWidget::DockPosition pos = KDockWidget::DockNone,
	                                             TQWidget* pTargetWnd = 0L, int percent = 50, const TQString& tabToolTip = 0,
	                                             const TQString& tabCaption = 0 );

	virtual void deleteToolWindow( TQWidget* pWnd );
	virtual void deleteToolWindow( KMdiToolViewAccessor *accessor );

	/**
	 * Using this method you have to use the setWidget method of the access object, and it is very recommendet, that you use
	 * the widgetContainer() method for the parent of your newly created widget
	 */
	KMdiToolViewAccessor *createToolWindow();

	/**
	 * Removes a KMdiChildView from the MDI system and from the main frame`s control.
	 * The caller is responsible for deleting the view. If the view is not deleted it will
	 * be reparented to 0
	 */
	virtual void removeWindowFromMdi( KMdiChildView *pWnd );

	/**
	 * Removes a KMdiChildView from the MDI system and from the main frame`s control.
	 * Note: The view will be deleted!
	 */
	virtual void closeWindow( KMdiChildView *pWnd, bool layoutTaskBar = true );

	/**
	 * Switches the KMdiTaskBar on and off.
	 */
	virtual void slot_toggleTaskBar();

	/**
	 * Makes a main frame controlled undocked KMdiChildView docked.
	 * Doesn't work on KMdiChildView which aren't added to the MDI system.
	 * Use addWindow() for that.
	 */
	virtual void attachWindow( KMdiChildView *pWnd, bool bShow = true, bool bAutomaticResize = false );

	/**
	 * Makes a docked KMdiChildView undocked.
	 * The view window still remains under the main frame's MDI control.
	 */
	virtual void detachWindow( KMdiChildView *pWnd, bool bShow = true );

	/**
	 * Someone wants that the MDI view to be closed. This method sends a KMdiViewCloseEvent to itself
	 * to break the function call stack. See also event() .
	 */
	virtual void childWindowCloseRequest( KMdiChildView *pWnd );

	/**
	 * Close all views
	 */
	virtual void closeAllViews();

	/**
	 * Iconfiy all views
	 */
	virtual void iconifyAllViews();

	/**
	 * Closes the view of the active (topchild) window
	 */
	virtual void closeActiveView();

	/**
	 * Undocks all view windows (unix-like)
	 */
	virtual void switchToToplevelMode();
	virtual void finishToplevelMode();

	/**
	 * Docks all view windows (Windows-like)
	 */
	virtual void switchToChildframeMode();
	virtual void finishChildframeMode();

	/**
	 * Docks all view windows (Windows-like)
	 */
	virtual void switchToTabPageMode();
	virtual void finishTabPageMode();

	/**
	 * Docks all view windows. Toolviews use dockcontainers
	 */
	virtual void switchToIDEAlMode();
	virtual void finishIDEAlMode( bool full = true );

	/**
	 * Sets the appearance of the IDEAl mode. See KMultiTabBar styles for the first 3 bits.
	 * @deprecated use setToolviewStyle(int flags) instead
	 */
	void setIDEAlModeStyle( int flags ) TDE_DEPRECATED;
	//KDE4: Get rid of the above.
	/**
	 * Sets the appearance of the toolview tabs.
	 * @param flags See KMdi::ToolviewStyle.
	 * @since 3.3
	 */
	void setToolviewStyle( int flags );
	/**
	 * @return if the view taskbar should be shown if there are MDI views
	 */
	bool isViewTaskBarOn();

	/**
	 * Shows the view taskbar. This should be connected with your "View" menu.
	 */
	virtual void showViewTaskBar();

	/**
	 * Hides the view taskbar. This should be connected with your "View" menu.
	 */
	virtual void hideViewTaskBar();

	/**
	 * Update of the window menu contents.
	 */
	virtual void fillWindowMenu();

	/**
	 * Cascades the windows without resizing them.
	 */
	virtual void cascadeWindows() { m_pMdi->cascadeWindows(); }

	/**
	 * Cascades the windows resizing them to the maximum available size.
	 */
	virtual void cascadeMaximized() { m_pMdi->cascadeMaximized(); }

	/**
	 * Maximizes only in vertical direction.
	 */
	virtual void expandVertical() { m_pMdi->expandVertical(); }

	/**
	* Maximizes only in horizontal direction.
	*/
	virtual void expandHorizontal() { m_pMdi->expandHorizontal(); }

	/**
	 * Tile Pragma
	 */
	virtual void tilePragma() { m_pMdi->tilePragma(); }

	/**
	 * Tile Anodine
	 */
	virtual void tileAnodine() { m_pMdi->tileAnodine(); }

	/**
	 * Tile Vertically
	 */
	virtual void tileVertically() { m_pMdi->tileVertically(); }

	/**
	 * Sets the decoration of the window frame of docked (attached) MDI views
	 * @deprecated Will be removed in KDE 4
	 */
	virtual void setFrameDecorOfAttachedViews( int frameDecor );

	/**
	 * If in Childframe mode, we can switch between maximized or restored shown MDI views
	 */
	virtual void setEnableMaximizedChildFrmMode( bool bEnable );

	/**
	 * Activates the next open view
	 */
	virtual void activateNextWin();

	/**
	 * Activates the previous open view
	 */
	virtual void activatePrevWin();

	/**
	 * Activates the view first viewed concerning to the access time.
	 */
	virtual void activateFirstWin();

	/**
	 * Activates the view last viewed concerning to the access time.
	 */
	virtual void activateLastWin();

	/**
	 * Activates the view with the tab page index (TabPage mode only)
	 */
	virtual void activateView( int index );

private:
	void setupToolViewsForIDEALMode();
	void setupTabbedDocumentViewSpace();
	class KMdiDocumentViewTabWidget * m_documentTabWidget;

protected:

	virtual void resizeEvent( TQResizeEvent * );

	/**
	 * Creates a new MDI taskbar (showing the MDI views as taskbar entries) and shows it.
	 */
	virtual void createTaskBar();

	/**
	 * Creates the MDI view area and connects some signals and slots with the KMdiMainFrm widget.
	 */
	virtual void createMdiManager();

	/**
	 * prevents fillWindowMenu() from m_pWindowMenu->clear(). You have to care for it by yourself.
	 * This is useful if you want to add some actions in your overridden fillWindowMenu() method.
	 */
	void blockClearingOfWindowMenu( bool bBlocked ) { m_bClearingOfWindowMenuBlocked = bBlocked; }

	void findToolViewsDockedToMain( TQPtrList<KDockWidget>* list, KDockWidget::DockPosition dprtmw );
	void dockToolViewsIntoContainers( TQPtrList<KDockWidget>& widgetsToReparent, KDockWidget *container );
	TQStringList prepareIdealToTabs( KDockWidget* container );
	void idealToolViewsToStandardTabs( TQStringList widgetNames, KDockWidget::DockPosition pos, int sizee );

	/** Get tabwidget visibility */
	KMdi::TabWidgetVisibility tabWidgetVisibility();

	/** Set tabwidget visibility */
	void setTabWidgetVisibility( KMdi::TabWidgetVisibility );

	/** Returns the tabwidget used in IDEAl and Tabbed modes. Returns 0 in other modes. */
	class KTabWidget * tabWidget() const;


protected slots:  // Protected slots
	/**
	 * Sets the focus to this MDI view, raises it, activates its taskbar button and updates
	 * the system buttons in the main menubar when in maximized (Maximize mode).
	 */
	virtual void activateView( KMdiChildView *pWnd );

	/**
	 * Activates the MDI view (see activateView() ) and popups the taskBar popup menu (see taskBarPopup() ).
	 */
	virtual void taskbarButtonRightClicked( KMdiChildView *pWnd );

	/**
	 * Turns the system buttons for maximize mode (SDI mode) off, and disconnects them
	 */
	void switchOffMaximizeModeForMenu( KMdiChildFrm* oldChild );

	/**
	 * Reconnects the system buttons form maximize mode (SDI mode) with the new child frame
	 */
	void updateSysButtonConnections( KMdiChildFrm* oldChild, KMdiChildFrm* newChild );

	/**
	 * Usually called when the user clicks an MDI view item in the "Window" menu.
	 */
	void windowMenuItemActivated( int id );

	/**
	 * Usually called when the user clicks an MDI view item in the sub-popup menu "Docking" of the "Window" menu.
	 */
	void dockMenuItemActivated( int id );

	/**
	 * Popups the "Window" menu. See also windowPopup() .
	 */
	void popupWindowMenu( TQPoint p );

	/**
	 * The timer for main widget moving has elapsed -> send drag end to all concerned views.
	 */
	void dragEndTimeOut();

	/**
	 * internally used to handle click on view close button (TabPage mode, only)
	 */
	void closeViewButtonPressed();

signals:
	/**
	 * Signals the last attached KMdiChildView has been closed
	 */
	void lastChildFrmClosed();

	/**
	 * Signals the last KMdiChildView (that is under MDI control) has been closed
	 */
	void lastChildViewClosed();

	/**
	 * Signals that the Toplevel mode has been left
	 */
	void leftTopLevelMode();

	/**
	 * Signals that a child view has been detached (undocked to desktop)
	 */
	void childViewIsDetachedNow( TQWidget* );

	/** Signals we need to collapse the overlapped containers */
	void collapseOverlapContainers();

	/** Signals the MDI mode has been changed */
	void mdiModeHasBeenChangedTo( KMdi::MdiMode );

	void viewActivated( KMdiChildView* );
	void viewDeactivated( KMdiChildView* );

public slots:
	void prevToolViewInDock();
	void nextToolViewInDock();

private slots:
	void setActiveToolDock( KMdiDockContainer* );
	void removeFromActiveDockList( KMdiDockContainer* );
	void slotDocCurrentChanged( TQWidget* );
	void verifyToplevelHeight();
#define protected public
signals:
#undef protected

	void toggleTop();
	void toggleLeft();
	void toggleRight();
	void toggleBottom();
};

#endif //_TDEMDIMAINFRM_H_
