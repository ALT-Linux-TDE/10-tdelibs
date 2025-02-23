 /* This file is part of the KDE project
  Copyright (C) 2002 Christoph Cullmann <cullmann@kde.org>
  Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

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

#ifndef _TDEMDI_DOCK_CONTAINER_
#define _TDEMDI_DOCK_CONTAINER_

#include <tqwidget.h>
#include <tqstringlist.h>
#include <kdockwidget.h>
#include <tqmap.h>
#include <tqdom.h>

# include <kdockwidget_p.h>

#include <tqpushbutton.h>

class TQWidgetStack;
class KMultiTabBar;
class KDockButton_Private;

class KMDI_EXPORT KMdiDockContainer: public TQWidget, public KDockContainer
{
	TQ_OBJECT

public:
	KMdiDockContainer( TQWidget *parent, TQWidget *win, int position, int flags );
	virtual ~KMdiDockContainer();

	/** Get the KDockWidget that is our parent */
	KDockWidget *parentDockWidget();

	/**
	 * Add a widget to this container
	 * \param w the KDockWidget we are adding
	 */
	virtual void insertWidget ( KDockWidget *w, TQPixmap, const TQString &, int & );

	/**
	 * Show a widget.
	 *
	 * The widget has to belong to this container otherwise
	 * it will not be shown
	 * \param w the KDockWidget to show
	 */
	virtual void showWidget ( KDockWidget *w );

	/**
	 * Set the tooltip for the widget.
	 * Currently, this method does nothing
	 */
	virtual void setToolTip ( KDockWidget *, TQString & );

	/**
	 * Set the pixmap for the widget.
	 */
	virtual void setPixmap( KDockWidget* widget, const TQPixmap& pixmap );

	/**
	 * Undock the widget from the container.
	 */
	virtual void undockWidget( KDockWidget* dwdg );

	/**
	 * Remove a widget from the container. The caller of this function
	 * is responsible for deleting the widget
	 */
	virtual void removeWidget( KDockWidget* );

	/**
	 * Hide the the dock container if the number of items is 0
	 */
	void hideIfNeeded();

	/**
	 * Save the config using a TDEConfig object
	 *
	 * The combination of the group_or_prefix variable and the parent
	 * dockwidget's name will be the group the configuration is saved in
	 * \param group_or_prefix the prefix to append to the parent dockwidget's name
	 */
	virtual void save( TDEConfig *, const TQString& group_or_prefix );

	/**
	 * Load the config using a TDEConfig object
	 * 
	 * The combination of the group_or_prefix variable and the parent
	 * dockwidget's name will be the group the configuration is loaded from
	 * \param group_or_prefix the prefix to append to the parent dockwidget's name
	 */
	virtual void load( TDEConfig *, const TQString& group_or_prefix );

	/**
	 * Save the config to a QDomElement
	 */
	virtual void save( TQDomElement& );

	/**
	 * Load the config from a QDomElement
	 */
	virtual void load( TQDomElement& );

	/**
	 * Set the style for the tabbar
	 */
	void setStyle( int );

protected:
	bool eventFilter( TQObject*, TQEvent* );

public slots:
	void init();
	void collapseOverlapped();
	void toggle();
	void nextToolView();
	void prevToolView();
protected slots:
	void tabClicked( int );
	void delayedRaise();
	void changeOverlapMode();
private:
	TQWidget *m_mainWin;
	TQWidgetStack *m_ws;
	KMultiTabBar *m_tb;
	int mTabCnt;
	int oldtab;
	int m_previousTab;
	int m_position;
	int m_separatorPos;
	TQMap<KDockWidget*, int> m_map;
	TQMap<int, KDockWidget*> m_revMap;
	TQMap<KDockWidget*, KDockButton_Private*> m_overlapButtons;
	TQStringList itemNames;
	TQMap<TQString, TQString> tabCaptions;
	TQMap<TQString, TQString> tabTooltips;
	int m_inserted;
	int m_delayedRaise;
	bool m_horizontal;
	bool m_block;
	bool m_tabSwitching;
	TQObject *m_dragPanel;
	KDockManager *m_dockManager;
	TQMouseEvent *m_startEvent;
	enum MovingState {NotMoving = 0, WaitingForMoveStart, Moving} m_movingState;
signals:
	void activated( KMdiDockContainer* );
	void deactivated( KMdiDockContainer* );
};

#endif
