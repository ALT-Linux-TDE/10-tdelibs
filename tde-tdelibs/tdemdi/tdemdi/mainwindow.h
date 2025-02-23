/* This file is part of the KDE libraries
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2002,2003 Joseph Wenninger <jowenn@kde.org>

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

   Based on:

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
   //    patches              : -/2000        by Lars Beikirch (Lars.Beikirch@gmx.net)
   //                         : 01/2003       by Jens Zurheide (jens.zurheide@gmx.de)
   //
   //    copyright            : (C) 1999-2003 by Falk Brettschneider
   //                                         and
   //                                         Szymon Stefanek (stefanek@tin.it)
   //    email                :  falkbr@kdevelop.org (Falk Brettschneider)
   //----------------------------------------------------------------------------
*/

#ifndef _TDEMDI_MAINWINDOW_H_
#define _TDEMDI_MAINWINDOW_H_

#include <tdeparts/dockmainwindow.h>
#include <tdemenubar.h>
#include <tdepopupmenu.h>

#include <tqptrlist.h>
#include <tqrect.h>
#include <tqapplication.h>
#include <tqguardedptr.h>

#include <tdemdi/global.h>

class TQPopupMenu;
class TQMenuBar;

#include <tdemdi/toolviewaccessor.h>

namespace KMDIPrivate {
  class MainWindowPrivate;
  class GUIClient;
}

namespace KMDI
{

class DockContainer;
class TabWidget;

class KMDI_EXPORT MainWindow : public KParts::DockMainWindow
{
  TQ_OBJECT

  friend class KMDI::ToolViewAccessor;

  public:
    /**
     * Constructor.
     */
    MainWindow ( TQWidget* parentWidget, const char* name = "");

    /**
     * Destructor.
     */
    virtual ~MainWindow ();

  public:
    /**
     * get the central tabwidget
     * add all your content as child to this widget
     * or as multiple tabs
     * @return tabwidget in central of window
     */
    TabWidget *tabWidget ();

    /**
     * Using this method you have to use the setWidget method of the access object, and it is very recommendet, that you use
     * the widgetContainer() method for the parent of your newly created widget
     * @return created toolview
     */
    KMDI::ToolViewAccessor *createToolWindow();

    /**
     * Usually called from addWindow() when adding a tool view window. It reparents the given widget
     * as toplevel and stay-on-top on the application's main widget.
     * @param pWnd widget for the toolview
     * @param pos docking position
     * @param pTargetWnd
     * @param percent
     * @param tabToolTip
     * @param tabCaption
     * @return created toolview
     */
    KMDI::ToolViewAccessor *addToolWindow( TQWidget* pWnd, KDockWidget::DockPosition pos = KDockWidget::DockNone, TQWidget* pTargetWnd = 0L, int percent = 50, const TQString& tabToolTip = 0, const TQString& tabCaption = 0);

    /**
     * delete the toolview belonging to the given accessor
     * @param accessor toolview to delete
     */
    void deleteToolWindow( KMDI::ToolViewAccessor *accessor);

    /**
     * delete the toolview belonging to the given pWnd
     * @param pWnd toolview to be deleted
     */
    void deleteToolWindow( TQWidget* pWnd);

    /**
     * Sets the appearance of the toolview tabs.
     * @param flags See KMDI::ToolviewStyle.
     */
    void setToolViewStyle(int flags);

  public slots:
    void prevToolViewInDock();
    void nextToolViewInDock();

  signals:
    void collapseOverlapContainers();

  protected:
    void findToolViewsDockedToMain(TQPtrList<KDockWidget>* list,KDockWidget::DockPosition dprtmw);
    void dockToolViewsIntoContainers(TQPtrList<KDockWidget>& widgetsToReparent,KDockWidget *container);

  private:
    /**
     * setup main dock + tab widget
     */
    void setupMainDock ();

    /**
     * setup the sidebars for the toolviews
     * and other internals to get them working
     */
    void setupToolViews ();

    /**
     * setup the menuentries
     * must be done AFTER setupToolViews ()
     */
    void setupGUIClient ();

  private slots:
    void setActiveToolDock(DockContainer*);
    void removeFromActiveDockList(DockContainer*);

  #define protected public
  signals:
  #undef protected
    void toggleTop();
    void toggleLeft();
    void toggleRight();
    void toggleBottom();

  protected:
    KMDIPrivate::GUIClient *m_guiClient;
    TQMap <TQWidget*, KMDI::ToolViewAccessor*> *m_toolViews;

    KDockWidget *m_leftContainer;
    KDockWidget *m_rightContainer;
    KDockWidget *m_topContainer;
    KDockWidget *m_bottomContainer;

    KMDI::TabWidget *m_tabWidget;

  private:
    /**
     * private d-pointer for BC
     */
    KMDIPrivate::MainWindowPrivate *d;
};

}

#endif //_MainWindow_H_
