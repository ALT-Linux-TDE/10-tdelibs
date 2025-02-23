//----------------------------------------------------------------------------
//    filename             : tdemdichildview.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : 02/2000       by Massimo Morin (mmorin@schedsys.com)
//                           */2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
//                           02/2001       by Eva Brucherseifer (eva@rt.e-technik.tu-darmstadt.de)
//                           01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
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
#ifndef _TDEMDI_CHILD_VIEW_H_
#define _TDEMDI_CHILD_VIEW_H_

#include <tqwidget.h>
#include <tqpixmap.h>
#include <tqrect.h>
#include <tqapplication.h>
#include <tqdatetime.h>

#include "tdemdichildfrm.h"

class KMdiChildViewPrivate;

/**
  * @short Base class for all your special view windows.
  *
  * Base class for all MDI view widgets. KMdi stores additional information in this class
  * to handle the attach/detach mechanism and such things.
  *
  * All such windows 'lives' attached to a KMdiChildFrm widget
  * managed by KMdiChildArea, or detached (managed by the window manager.)
  * So remember that the KMdiChildView::parent pointer may change, and may be 0L, too.
  *
  * There are 2 possibilities for you to put your widgets under MDI control:
  *
  * Either you inherit all the views from KMdiChildView:
  *   \code
  *   class MyMdiWidget : public KMdiChildView
  *   { .... };
  *   ...
  *   MyMdiWidget w;
  *   mainframe->addWindow(w, flags);
  *   \endcode
  *
  * or you wrap them by a KMdiChildView somehow like this:
  *
  * \code
  * void DocViewMan::addKMdiFrame(TQWidget* pNewView, bool bShow, const TQPixmap& icon)
  * {
  *   // cover it by a KMdi childview and add that MDI system
  *   KMdiChildView* pMDICover = new KMdiChildView( pNewView->caption());
  *   pMDICover->setIcon(icon);
  *   m_MDICoverList.append( pMDICover);
  *   TQBoxLayout* pLayout = new TQHBoxLayout( pMDICover, 0, -1, "layout");
  *   pNewView->reparent( pMDICover, TQPoint(0,0));
  *   pLayout->addWidget( pNewView);
  *   pMDICover->setName( pNewView->name());
  *   // captions
  *   TQString shortName = pNewView->caption();
  *   int length = shortName.length();
  *   shortName = shortName.right(length - (shortName.findRev('/') +1));
  *   pMDICover->setTabCaption( shortName);
  *   pMDICover->setCaption(pNewView->caption());
  *
  *   // fake a viewActivated to update the currentEditView/currentBrowserView pointers _before_ adding to MDI control
  *   slot_viewActivated( pMDICover);
  *
  *   // take it under MDI mainframe control (note: this triggers also a setFocus())
  *   int flags;
  *   if (bShow) {
  *     flags = KMdi::StandardAdd;
  *   }
  *   else {
  *     flags = KMdi::Hide;
  *   }
  *   // set the accelerators for Toplevel MDI mode (each toplevel window needs its own accels
  *   connect( m_pParent, TQ_SIGNAL(childViewIsDetachedNow(TQWidget*)), this, TQ_SLOT(initKeyAccel(TQWidget*)) );
  *
  *   m_pParent->addWindow( pMDICover, flags);
  *   // correct the default settings of KMdi ('cause we haven't a tab order for subwidget focuses)
  *   pMDICover->setFirstFocusableChildWidget(0L);
  *   pMDICover->setLastFocusableChildWidget(0L);
  * }
  * \endcode
  *
  */

class KMDI_EXPORT KMdiChildView : public TQWidget
{
	friend class KMdiMainFrm;
	friend class KMdiChildFrm;
	TQ_OBJECT

	// attributes
protected:
	/**
	 * See KMdiChildView::caption
	 */
	TQString m_szCaption;
	
	/**
	 * See KMdiChildView::tabCaption
	 */
	TQString m_sTabCaption;
	
	/**
	 * See KMdiChildView::focusedChildWidget
	 */
	TQWidget* m_focusedChildWidget;
	
	/**
	 * See KMdiChildView::setFirstFocusableChildWidget
	 */
	TQWidget* m_firstFocusableChildWidget;
	
	/**
	 * See KMdiChildView::setLastFocusableChildWidget
	 */
	TQWidget* m_lastFocusableChildWidget;
	
	/**
	 * Every child view window has an temporary ID in the Window menu of the main frame.
	 */
	int m_windowMenuID;
	
	/**
	 * Holds a temporary information about if the MDI view state has changed but is not processed yet (pending state).
	 * For example it could be that a 'maximize' is pending, if this variable is true.
	 */
	bool m_stateChanged;

	/**
	 * Holds the time when this view was activated (not only displayed) for the last time.
	 */
	TQDateTime m_time;

private:
	/**
	 * Internally used as indicator whether this KMdiChildView is treated as document view or as tool view.
	 */
	bool m_bToolView;
	
	/**
	 * Internally used by KMdiMainFrm to store a temporary information that the method
	 * activate() is unnecessary and that it can by escaped.
	 * This saves from unnecessary calls when activate is called directly.
	 */
	bool m_bInterruptActivation;
	
	/**
	 * Internally used to prevent cycles between KMdiMainFrm::activateView() and KMdiChildView::activate().
	 */
	bool m_bMainframesActivateViewIsPending;
	
	/**
	 * Internally used to check if there is a focus in event pending
	 */
	bool m_bFocusInEventIsPending;

	// methods
public:
	/**
	 * Constructor
	 */
	KMdiChildView( const TQString& caption, TQWidget* parentWidget = 0L, const char* name = 0L, WFlags f = 0 );
	
	/**
	 * Constructor 
	 * sets "Unnamed" as default caption
	 */
	KMdiChildView( TQWidget* parentWidget = 0L, const char* name = 0L, WFlags f = 0 );
	
	/**
	 * Destructor
	 */
	~KMdiChildView();
	
	/**
	 * This method does the same as focusInEvent(). That's why it is a replacement for the setFocus() call. It makes
	 * sense if you for instance want to focus (I mean raise and activate) this view although the real focus is
	 * in another toplevel widget. focusInEvent() will never get called in that case and your setFocus() call for this
	 * widget would fail without any effect.
	 * Use this method with caution, it always raises the view and pushes the taskbar button. Also when the focus is
	 * still on another MDI view in the same toplevel window where this is located!
	 */
	void activate();
	
	/**
	 * Memorizes the first focusable child widget of this widget
	 */
	void setFirstFocusableChildWidget( TQWidget* );
	
	/**
	 * Memorizes the last focusable child widget of this widget
	 */
	void setLastFocusableChildWidget( TQWidget* );
	
	/**
	 * Returns the current focused child widget of this widget
	 */
	TQWidget* focusedChildWidget();
	
	/**
	 * Returns true if the MDI view is a child window within the MDI mainframe widget
	 * or false if the MDI view is in toplevel mode
	 */
	bool isAttached() const { return ( mdiParent() != 0L ); }
	
	/**
	 * Returns the caption of the child window (different from the caption on the button in the taskbar)
	 */
	const TQString& caption() const { return m_szCaption; }
	
	/**
	 * Returns the caption of the button on the taskbar
	 */
	const TQString& tabCaption() const { return m_sTabCaption; }
	
	/**
	 * Sets the window caption string...
	 * Calls updateButton on the taskbar button if it has been set.
	 */
	virtual void setCaption( const TQString& szCaption );
	
	/**
	 * Sets the caption of the button referring to this window
	 */
	virtual void setTabCaption( const TQString& caption );
	
	/**
	 * Sets the caption of both the window and the button on the taskbar
	 */
	virtual void setMDICaption( const TQString &caption );
	
	/**
	 * Returns the KMdiChildFrm parent widget (or 0 if the window is not attached)
	 */
	KMdiChildFrm *mdiParent() const;
	
	/**
	 * Tells if the window is minimized when attached to the Mdi manager,
	 * or if it is VISIBLE when 'floating'.
	 */
	bool isMinimized() const;
	
	/**
	 * Tells if the window is minimized when attached to the Mdi manager,
	 * otherwise returns false.
	 */
	bool isMaximized() const;
	
	/**
	 * Returns the geometry of this MDI child window as TQWidget::geometry() does.
	 */
	TQRect internalGeometry() const;
	
	/**
	 * Sets the geometry of the client area of this MDI child window. The
	 * top left position of the argument is the position of the top left point
	 * of the client area in its parent coordinates and the arguments width
	 * and height is the width and height of the client area. Please note: This
	 * differs from the behavior of TQWidget::setGeometry()!
	 */
	void setInternalGeometry( const TQRect& newGeomety );
	
	/**
	 * Returns the frame geometry of this window or of the parent if there is any...
	 */
	TQRect externalGeometry() const;
	
	/**
	 * Sets the geometry of the frame of this MDI child window. The top left
	 * position of the argument is the position of the top left point of the
	 * frame in its parent coordinates and the arguments width and height is
	 * the width and height of the widget frame. Please note: This differs
	 * from the behavior of TQWidget::setGeometry()!
	 */
	void setExternalGeometry( const TQRect& newGeomety );
	
	/**
	 * You should override this function in the derived class.
	 */
	virtual TQPixmap* myIconPtr();
	
	/**
	 * Minimizes this window when it is attached to the Mdi manager.
	 * Otherwise has no effect
	 */
	virtual void minimize( bool bAnimate );
	
	/**
	 * Maximizes this window when it is attached to the Mdi manager.
	 * Otherwise has no effect
	 */
	virtual void maximize( bool bAnimate );
	
	/**
	 * Returns the geometry that will be restored by calling restore().
	 */
	TQRect restoreGeometry();
	
	/**
	 * Sets the geometry that will be restored by calling restore().
	 */
	void setRestoreGeometry( const TQRect& newRestGeo );
	
	/**
	 * Switches interposing in event loop of all current child widgets off.
	 */
	void removeEventFilterForAllChildren();
	
	/**
	 * Internally used for setting an ID for the 'Window' menu entry
	 */
	void setWindowMenuID( int id );
	
	/**
	 * Sets the minimum size of the widget to w by h pixels.
	 * It extends it base clase method in a way that the minimum size of
	 * its childframe (if there is one) will be set, additionally.
	 */
	virtual void setMinimumSize ( int minw, int minh );
	
	/**
	 * Sets the maximum size of the widget to w by h pixels.
	 * It extends it base clase method in a way that the maximum size of
	 * its childframe (if there is one) will be set, additionally.
	 */
	virtual void setMaximumSize ( int maxw, int maxh );
	
	/**
	 * Returns if this is added as MDI tool-view
	 */
	inline bool isToolView() const { return m_bToolView; }
	
	/**
	 * Remember the current time
	 */
	inline void updateTimeStamp()
	{
		m_time.setDate( TQDate::currentDate() );
		m_time.setTime( TQTime::currentTime() );
	}
	
	/**
	 * Recall a previously remembered time, i.e. the value of m_time
	 */
	inline const TQDateTime& getTimeStamp() const { return m_time; }

public slots:
	/**
	* Attaches this window to the Mdi manager.
	* It calls the KMdiMainFrm attachWindow function , so if you have a pointer
	* to this KMdiMainFrm you'll be faster calling that function.
	*/
	virtual void attach();
	
	/**
	 * Detaches this window from the Mdi manager.
	 * It calls the KMdiMainFrm detachWindow function , so if you have a pointer
	 * to this KMdiMainFrm you'll be faster calling that function.
	 */
	virtual void detach();
	
	/**
	 * Mimimizes the MDI view. If attached, the covering childframe widget is minimized (only a mini widget
	 * showing the caption bar and the system buttons will remain visible). If detached, it will use the
	 * minimize of the underlying system ( TQWidget::showMinimized ).
	 */
	virtual void minimize();
	
	/**
	 * Maximizes the MDI view. If attached, this widget will fill the whole MDI view area widget. The system buttons
	 * move to the main menubar (if set by KMdiMainFrm::setMenuForSDIModeSysButtons ).
	 * If detached, it will use the minimize of the underlying system ( TQWidget::showMaximized ).
	 */
	virtual void maximize();
	
	/**
	 * Restores this window to its normal size. Also known as 'normalize'.
	 */
	virtual void restore();
	
	/**
	 * Internally called, if KMdiMainFrm::attach is called.
	 * Actually, only the caption of the covering childframe is set.
	 */
	virtual void youAreAttached( KMdiChildFrm *lpC );
	
	/**
	 * Internally called, if KMdiMainFrm::detach is called.
	 * Some things for going toplevel will be done here.
	 */
	virtual void youAreDetached();
	
	/**
	 * Called if someone click on the "Window" menu item for this child frame window
	 */
	virtual void slot_clickedInWindowMenu();
	
	/**
	 * Called if someone click on the "Dock/Undock..." menu item for this child frame window
	 */
	virtual void slot_clickedInDockMenu();
	
	/**
	 * Calls TQWidget::show but also for it's parent widget if attached
	 */
	virtual void show();
	
	/**
	 * Calls TQWidget::hide() or it's parent widget hide() if attached
	 */
	virtual void hide();
	
	/**
	 * Calls TQWidget::raise() or it's parent widget raise() if attached
	 */
	virtual void raise();
	
	/**
	 * Overridden from its base class method. Emits a signal KMdiChildView::isMinimizedNow , additionally.
	 * Note that this method is not used by an external windows manager call on system minimizing.
	 */
	virtual void showMinimized();
	
	/**
	 * Overridden from its base class method. Emits a signal KMdiChildView::isMaximizedNow , additionally.
	 * Note that this method is not used by an external windows manager call on system maximizing.
	 */
	virtual void showMaximized();
	
	/**
	 * Overridden from its base class method. Emits a signal KMdiChildView::isRestoredNow , additionally.
	 * Note that this method is not used by an external windows manager call on system normalizing.
	 */
	virtual void showNormal();
	

protected:
	/**
	 * Ignores the event and calls KMdiMainFrm::childWindowCloseRequest instead. This is because the
	 * mainframe has control over the views. Therefore the MDI view has to request the mainframe for a close.
	 */
	virtual void closeEvent( TQCloseEvent *e );
	
	/**
	 * It only catches TQEvent::KeyPress events there. If a TQt::Key_Tab is pressed, the internal MDI focus
	 * handling is called. That means if the last focusable child widget of this is called, it will jump to the
	 * first focusable child widget of this.
	 * See KMdiChildView::setFirstFocusableChildWidget and KMdiChildView::lastFirstFocusableChildWidget
	 */
	virtual bool eventFilter( TQObject *obj, TQEvent *e );
	
	/**
	 * If attached, the childframe will be activated and the MDI taskbar button will be pressed. Additionally, the
	 * memorized old focused child widget of this is focused again.
	 * Sends the focusInEventOccurs signal before changing the focus and the
	 * gotFocus signal after changing the focus.
	 */
	virtual void focusInEvent( TQFocusEvent *e );
	
	/**
	 * Send the lostFocus signal
	 */
	virtual void focusOutEvent( TQFocusEvent *e );
	
	/**
	 * Internally used for the minimize/maximize/restore mechanism when in attach mode.
	 */
	virtual void resizeEvent( TQResizeEvent *e );

	void trackIconAndCaptionChanges( TQWidget *view );

protected slots:
	void slot_childDestroyed();

signals:
	/**
	 * Internally used by KMdiChildView::attach to send it as command to the mainframe.
	 */
	void attachWindow( KMdiChildView*, bool );
	
	/**
	 * Internally used by KMdiChildView::detach to send it as command to the mainframe.
	 */
	void detachWindow( KMdiChildView*, bool );
	
	/**
	 * Is sent when this MDI child view is going to receive focus (before actually changing the focus).
	 * Internally used to send information to the mainframe that this MDI child view is focused.
	 * See KMdiChildView::focusInEvent
	 */
	void focusInEventOccurs( KMdiChildView* );
	
	/**
	 * Is sent when this MDI child has received the focus (after actually changing the focus).
	 * See KMdiChildView::focusInEvent
	 */
	void gotFocus( KMdiChildView* );
	
	/**
	 * Is sent when this MDI child was set to the activate view of all MDI views (after actually changing the focus).
	 * See KMdiChildView::activate
	 */
	void activated( KMdiChildView* );
	
	/** Is sent when this MDI child view has lost the focus (after actually changing the focus).
	 *  See KMdiChildView::focusOutEvent
	 */
	void lostFocus( KMdiChildView* );
	
	/** Is sent when this MDI child view was deactivated (after actually changing the focus).
	 *  See KMdiChildView::focusOutEvent
	 */
	void deactivated( KMdiChildView* );
	
	/**
	 * Internally used to send information to the mainframe that this MDI child view wants to be closed.
	 * See KMdiChildView::closeEvent and KMdiMainFrm::closeWindow
	 */
	void childWindowCloseRequest( KMdiChildView* );
	
	/**
	 * Emitted when the window caption is changed via KMdiChildView::setCaption or KMdiChildView::setMDICaption
	 */
	void windowCaptionChanged( const TQString& );
	
	/**
	 * Emitted  when the window caption is changed via KMdiChildView::setTabCaption or KMdiChildView::setMDICaption
	 */
	void tabCaptionChanged( const TQString& );
	
	/**
	 * Internally used to send information to the mainframe that this MDI view is maximized now.
	 * Usually, the mainframe switches system buttons.
	 */
	void mdiParentNowMaximized( bool );
	
	/**
	 * Is automatically emitted when slot_clickedInWindowMenu is called
	 */
	void clickedInWindowMenu( int );
	
	/**
	 * Is automatically emitted when slot_clickedInDockMenu is called
	 */
	void clickedInDockMenu( int );
	
	/**
	 * Signals this has been maximized
	 */
	void isMaximizedNow();
	
	/**
	 * Signals this has been minimized
	 */
	void isMinimizedNow();
	
	/**
	 * Signals this has been restored (normalized)
	 */
	void isRestoredNow();
	
	/**
	 * Signals this has been attached
	 */
	void isAttachedNow();
	
	/**
	 * Signals this has been detached
	 */
	void isDetachedNow();

	void iconUpdated( TQWidget*, TQPixmap );
	void captionUpdated( TQWidget*, const TQString& );


private:
	KMdiChildViewPrivate *d;
	TQWidget *m_trackChanges;
};

inline KMdiChildFrm *KMdiChildView::mdiParent() const
{
	TQWidget * pw = parentWidget();
	if ( pw != 0L )
		if ( pw->inherits( "KMdiChildFrm" ) )
			return ( KMdiChildFrm * ) pw;
	return 0L;
}

#endif //_TDEMDI_CHILD_VIEW_H_
