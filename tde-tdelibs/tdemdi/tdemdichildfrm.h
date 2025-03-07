//----------------------------------------------------------------------------
//    filename             : tdemdichildfrm.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 07/1999       by Szymon Stefanek as part of kvirc
//                                         (an IRC application)
//    changes              : 09/1999       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//    patches              : */2000        Lars Beikirch (Lars.Beikirch@gmx.net)
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
//------------------------------------------------------------------------------
#ifndef _TDEMDI_CHILD_FRM_H_
#define _TDEMDI_CHILD_FRM_H_

#include <tqptrlist.h>
#include <tqpixmap.h>
#include <tqpushbutton.h>
#include <tqlabel.h>
#include <tqdatetime.h>
#include <tqlayout.h>

#include <tqdict.h>

#include "tdemdichildfrmcaption.h"

class KMdiChildArea;
class KMdiChildView;
class TQPopupMenu;
class TQToolButton;

//==============================================================================
/**
  * @short Internal class, only used on Win32.
  * This class provides a label widget that can process mouse click events.
  */
class KMDI_EXPORT KMdiWin32IconButton : public TQLabel
{
	TQ_OBJECT
public:
	KMdiWin32IconButton( TQWidget* parent, const char* name = 0 );
	virtual void mousePressEvent( TQMouseEvent* );

signals:
	void pressed();
};

//==============================================================================
/* some special events, see tdemdidefines.h
*/ 
//------------------------------------------------------------------------------
/**
 * @short a TQCustomEvent for move
 * This special event will be useful, to inform view about child frame event.
 */
class KMDI_EXPORT KMdiChildFrmMoveEvent : public TQCustomEvent
{
public:
	KMdiChildFrmMoveEvent( TQMoveEvent *e ) : TQCustomEvent( TQEvent::Type( TQEvent::User + int( KMdi::EV_Move ) ), e ) {}

};

//------------------------------------------------------------------------------
/**
 * @short a TQCustomEvent for begin of dragging
 * This special event will be useful, to inform view about child frame event.
 */
class KMDI_EXPORT KMdiChildFrmDragBeginEvent : public TQCustomEvent
{
public:
	KMdiChildFrmDragBeginEvent( TQMouseEvent *e ) : TQCustomEvent( TQEvent::Type( TQEvent::User + int( KMdi::EV_DragBegin ) ), e ) {}
};

//------------------------------------------------------------------------------
/**
 * @short a TQCustomEvent for end of dragging
 * This special event will be useful, to inform view about child frame event.
 */
class KMDI_EXPORT KMdiChildFrmDragEndEvent : public TQCustomEvent
{
public:
	KMdiChildFrmDragEndEvent( TQMouseEvent *e ) : TQCustomEvent( TQEvent::Type( TQEvent::User + int( KMdi::EV_DragEnd ) ), e ) {}
};

//------------------------------------------------------------------------------
/**
 * @short a TQCustomEvent for begin of resizing
 * This special event will be useful, to inform view about child frame event.
 */
class KMDI_EXPORT KMdiChildFrmResizeBeginEvent : public TQCustomEvent
{
public:
	KMdiChildFrmResizeBeginEvent( TQMouseEvent *e ) : TQCustomEvent( TQEvent::Type( TQEvent::User + int( KMdi::EV_ResizeBegin ) ), e ) {}
};

//------------------------------------------------------------------------------
/**
 * @short a TQCustomEvent for end of resizing
 * This special event will be useful, to inform view about child frame event.
 */
class KMDI_EXPORT KMdiChildFrmResizeEndEvent : public TQCustomEvent
{
public:
	KMdiChildFrmResizeEndEvent( TQMouseEvent *e ) : TQCustomEvent( TQEvent::Type( TQEvent::User + int( KMdi::EV_ResizeEnd ) ), e ) {}
};


class KMdiChildFrmPrivate;
//==============================================================================
/**
  * @short Internal class.
  * It's an MDI child frame widget. It contains a view widget and a frame caption. Usually you derive from its view.
  */ 
//------------------------------------------------------------------------------
class KMDI_EXPORT KMdiChildFrm : public TQFrame
{
	friend class KMdiChildArea;
	friend class KMdiChildFrmCaption;

	TQ_OBJECT

	// attributes
public:
	enum MdiWindowState { Normal, Maximized, Minimized };
	//positions same in h and cpp for fast order check
	KMdiChildView* m_pClient;

protected:
	KMdiChildArea* m_pManager;
	KMdiChildFrmCaption* m_pCaption;
	KMdiWin32IconButton* m_pWinIcon;
	TQToolButton* m_pUnixIcon;
	TQToolButton* m_pMinimize;
	TQToolButton* m_pMaximize;
	TQToolButton* m_pClose;
	TQToolButton* m_pUndock;
	MdiWindowState m_state;
	TQRect m_restoredRect;
	int m_iResizeCorner;
	int m_iLastCursorCorner;
	bool m_bResizing;
	bool m_bDragging;
	TQPixmap* m_pIconButtonPixmap;
	TQPixmap* m_pMinButtonPixmap;
	TQPixmap* m_pMaxButtonPixmap;
	TQPixmap* m_pRestoreButtonPixmap;
	TQPixmap* m_pCloseButtonPixmap;
	TQPixmap* m_pUndockButtonPixmap;
	
	/**
	* Every child frame window has an temporary ID in the Window menu of the child area.
	*/
	int m_windowMenuID;
	
	/**
	* Imitates a system menu for child frame windows
	*/
	TQPopupMenu* m_pSystemMenu;
	
	TQSize m_oldClientMinSize;
	TQSize m_oldClientMaxSize;
	TQLayout::ResizeMode m_oldLayoutResizeMode;
	TQTime m_timeMeasure;

	// methods
public:
	
	/**
	 * Creates a new KMdiChildFrm class.
	 */
	KMdiChildFrm( KMdiChildArea *parent );
	
	/**
	 * Destroys this KMdiChildFrm
	 * If a child is still here managed (no recreation was made) it is destroyed too.
	 */
	~KMdiChildFrm();
	
	/**
	 * Reparents the widget w to this KMdiChildFrm (if this is not already done)
	 * Installs an event filter to catch focus events.
	 * Resizes this mdi child in a way that the child fits perfectly in.
	 */
	void setClient( KMdiChildView *w, bool bAutomaticResize = false );
	
	/**
	 * Reparents the client widget to 0 (desktop), moves with an offset from the original position
	 * Removes the event filter.
	 */
	void unsetClient( TQPoint positionOffset = TQPoint( 0, 0 ) );
	
	/**
	 * Sets the window icon pointer.
	 */
	void setIcon( const TQPixmap &pxm );
	
	/**
	 * Returns the child frame icon.
	 */
	TQPixmap* icon() const;
	
	/**
	 * Enables or disables the close button
	 */
	void enableClose( bool bEnable );
	
	/**
	 * Sets the caption of this window
	 */
	void setCaption( const TQString& text );
	
	/**
 	 * Gets the caption of this mdi child.
	 */
	const TQString& caption() { return m_pCaption->m_szCaption; }
	
	/**
	 * Minimizes, Maximizes, or restores the window.
	 */
	void setState( MdiWindowState state, bool bAnimate = true );
	
	/**
	 * Returns the current state of the window
	 */
	inline MdiWindowState state() const { return m_state; }
	
	/**
	 * Returns the inner client area of the parent of this (which is KMdiChildArea).
	 */
	TQRect mdiAreaContentsRect() const;
	
	/**
	 * Returns the geometry that will be restored by calling restore().
	 */
	TQRect restoreGeometry() const;
	
	/**
	 * Sets the geometry that will be restored by calling restore().
	 */
	void setRestoreGeometry( const TQRect& newRestGeo );
	
	/**
	 * Forces updating the rects of the caption and so...
	 * It may be useful when setting the mdiCaptionFont of the MdiManager
	 */
	void updateRects() { resizeEvent( 0 ); }
	
	/**
	 * Returns the system menu.
	 */
	TQPopupMenu* systemMenu() const;
	
	/**
	 * Returns the caption bar height 
	 */
	inline int captionHeight() const { return m_pCaption->height(); }
	
	/**
	 * sets new raise behavior and pixmaps of the buttons depending on the current decoration style
	 */
	void redecorateButtons();

	/**
	 * returns the mouse state "In Drag"
	 */
	bool isInDrag() const { return m_bDragging; }
	
	/**
	 * returns the mouse state "In Resize"
	 */
	bool isInResize() const { return m_bResizing; }
	
	/**
	 * Internally called from the signal focusInEventOccurs.
	 * It raises the MDI childframe to the top of all other MDI child frames and sets the focus on it.
	 */
	void raiseAndActivate();

	/**
	 * Sets the minimum size of the widget to w by h pixels.
	 * It extends it's base clase method in a way that the minimum size of
	 * the child area will be set additionally if the view is maximized.
	 */
	virtual void setMinimumSize ( int minw, int minh );

public slots:
	
	void slot_resizeViaSystemMenu();

protected:
	
	/**
	 * Reimplemented from the base class.
	 * Resizes the captionbar, relayouts the position of the system buttons,
	 * and calls resize for its embedded KMdiChildView with the proper size
	 */
	virtual void resizeEvent( TQResizeEvent * );
	
	/**
	 * Reimplemented from the base class.
	 * Detects if the mouse is on the edge of window and what resize cursor must be set.
	 * Calls KMdiChildFrm::resizeWindow if it is in m_bResizing.
	 */
	virtual void mouseMoveEvent( TQMouseEvent *e );
	
	/**
	 * Reimplemented from the base class.
	 * Colours the caption, raises the childfrm widget and
	 * turns to resize mode if it is on the edge (resize-sensitive area)
	 */
	virtual void mousePressEvent( TQMouseEvent *e );
	
	/**
	 * Reimplemented from the base class.
	 * Sets a normal cursor and leaves the resize mode.
	 */
	virtual void mouseReleaseEvent( TQMouseEvent * );
	
	/**
	 * Reimplemented from the base class.
	 * give its child view the chance to notify a childframe move... that's why it sends
	 * a KMdiChildMovedEvent to the embedded KMdiChildView .
	 */
	virtual void moveEvent( TQMoveEvent* me );
	
	/**
	 * Reimplemented from the base class. If not in resize mode, it sets the mouse cursor to normal appearance.
	 */
	virtual void leaveEvent( TQEvent * );
	
	/** 
	 * Reimplemented from the base class.
	 * In addition, the following are caught
	 * -the client's mousebutton press events which raises and activates the childframe
	 * -the client's resize event which resizes this widget as well
	 */
	virtual bool eventFilter( TQObject*, TQEvent* ); 
	
	/**
	 * Calculates the new geometry from the new mouse position given as parameters
	 * and calls KMdiChildFrm::setGeometry
	 */
	void resizeWindow( int resizeCorner, int x, int y );
	
	/**
	 * Override the cursor appearance depending on the widget corner given as parameter
	 */
	void setResizeCursor( int resizeCorner );
	
	/**
	 * Changes from the resize cursor to the normal (previous) cursor
	 */
	void unsetResizeCursor();
	
	/**
	 * That means to show a mini window showing the childframe's caption bar, only.
	 * It cannot be resized.
	 */
	virtual void switchToMinimizeLayout();
	
	/**
	 * Does the actual resize. Called from various places but from resizeEvent in general.
	 */
	void doResize();
	
	/**
	 * Does the actual resize, like doResize() but skips resize of the client if \a captionOnly is true.
	 * @todo: merge with doResize()
	 */
	void doResize( bool captionOnly );

protected slots:
	
	/**
	 * Handles a click on the Maximize button
	 */
	void maximizePressed();
	
	/**
	 * Handles a click on the Restore (Normalize) button
	 */
	void restorePressed();
	
	/**
	 * Handles a click on the Minimize button.
	 */
	void minimizePressed();
	
	/**
	 * Handles a click on the Close button.
	 */
	void closePressed();
	
	/**
	 * Handles a click on the Undock (Detach) button
	 */
	void undockPressed();
	
	/**
	 * Shows a system menu for child frame windows.
	 */
	void showSystemMenu();

protected:

	/**
	 * Restore the focus policies for _all_ widgets in the view using the list given as parameter.
	 * Install the event filter for all direct child widgets of this. (See KMdiChildFrm::eventFilter)
	 */
	void linkChildren( TQDict<TQWidget::FocusPolicy>* pFocPolDict );

	/**
	 * Backups all focus policies of _all_ child widgets in the MDI childview since they get lost during a reparent.
	 * Remove all event filters for all direct child widgets of this. (See KMdiChildFrm::eventFilter)
	 */
	TQDict<TQWidget::FocusPolicy>* unlinkChildren();
	
	/**
	 * Calculates the corner id for the resize cursor. The return value can be tested for:
	 * KMDI_RESIZE_LEFT, KMDI_RESIZE_RIGHT, KMDI_RESIZE_TOP, KMDI_RESIZE_BOTTOM
	 * or an OR'd variant of them for the corners.
	 */
	int getResizeCorner( int ax, int ay );

private:
	KMdiChildFrmPrivate *d;
};

#endif //_TDEMDI_CHILDFRM_H_
