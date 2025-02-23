/* This file is part of the KDE project
   Copyright (C) 2002, 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002, 2004 Christoph Cullmann <cullmann@kde.org>

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

# include <kdockwidget_p.h>

#include <tqpushbutton.h>

class TQWidgetStack;
class KMultiTabBar;
class KDockButton_Private;

namespace KMDI
{

class DockContainer: public TQWidget, public KDockContainer
{
  TQ_OBJECT

  public:
    DockContainer(TQWidget *parent, TQWidget *win, int position, int flags);
    virtual ~DockContainer();

    /** Get the KDockWidget that is our parent */
    KDockWidget *parentDockWidget();

    /**
     * Add a widget to this container.
     * \param w the KDockWidget object to add
     * \todo Remove the extra parameters that we don't use?
     */
    virtual void insertWidget (KDockWidget *w, TQPixmap, const TQString &, int &);

    /**
     * Show a KDockWidget in our DockContainer
     * \param w the KDockWidget to show
     */
    virtual void showWidget (KDockWidget *w);

    /**
     * Set a tooltip for a widget
     *
     * \todo Actually implement it? Right now, it looks just it
     * does exactly nothing
     */
    virtual void setToolTip (KDockWidget *, TQString &);

    /**
     * Set a pixmap for one of our dock widgets
     * \param widget the KDockWidget to set the pixmap for
     * \param pixmap the pixmap you want to give the widget
     */
    virtual void setPixmap(KDockWidget* widget, const TQPixmap& pixmap);

    /**
     * Undock a widget from the container. This function is called
     * when you've dragged a tab off the dock it's attached to. 
     * \param dwdg the KDockWidget to undock
     */
    virtual void undockWidget(KDockWidget* dwdg);

    /**
     * Remove a widget from the container. The caller
     * of this function is responsible for deleting the widget after calling
     * this function.
     */
    virtual void removeWidget(KDockWidget*);

    void hideIfNeeded();

    virtual void save(TDEConfig *,const TQString& group_or_prefix);
    virtual void load(TDEConfig *,const TQString& group_or_prefix);

    void setStyle(int);
  protected:
    bool eventFilter(TQObject*,TQEvent*);

  public slots:
    void init();
    void collapseOverlapped();
    void toggle();
    void nextToolView();
    void prevToolView();
  protected slots:
    void tabClicked(int);
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
    TQMap<KDockWidget*,int> m_map;
    TQMap<int,KDockWidget*> m_revMap;
    TQMap<KDockWidget*,KDockButton_Private*> m_overlapButtons;
    TQStringList itemNames;
    TQMap<TQString,TQString> tabCaptions;
    TQMap<TQString,TQString> tabTooltips;
    int m_inserted;
    int m_delayedRaise;
    bool m_vertical;
    bool m_block;
    bool m_tabSwitching;
    TQObject *m_dragPanel;
    KDockManager *m_dockManager;
    TQMouseEvent *m_startEvent;
    enum MovingState {NotMoving=0,WaitingForMoveStart,MovingInternal,Moving} m_movingState;
  signals:
        void activated(DockContainer*);
        void deactivated(DockContainer*);
};

}

#endif
