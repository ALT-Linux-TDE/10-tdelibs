/* This file is part of the KDE libraries
   Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2004 Christoph Cullmann <cullmann@kde.org>
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

#ifndef _TDEMDI_GUICLIENT_H_
#define _TDEMDI_GUICLIENT_H_

#include <tqobject.h>
#include <tqguardedptr.h>
#include <kxmlguiclient.h>
#include <tdeaction.h>

#include <tdemdi/global.h>

class TDEMainWindow;
class TDEToolBar;

namespace KMDI {
  class MainWindow;
  class ToolViewAccessor;
}

class KDockWidget;

namespace KMDIPrivate {

class GUIClientPrivate;

class GUIClient : public TQObject, public KXMLGUIClient
{
  TQ_OBJECT

  public:
    GUIClient( KMDI::MainWindow *mdiMainFrm, const char *name = 0 );
    virtual ~GUIClient();

    void addToolView(KMDI::ToolViewAccessor*);

  private slots:
    void clientAdded( KXMLGUIClient *client );
    void setupActions();
    void actionDeleted(TQObject*);

  signals:
    void toggleTop();
    void toggleLeft();
    void toggleRight();
    void toggleBottom();

  private:
    GUIClientPrivate *d;

    TQGuardedPtr<KMDI::MainWindow> m_mdiMainFrm;
    TQPtrList<TDEAction> m_toolViewActions;
    TQPtrList<TDEAction> m_documentViewActions;

    TDEActionMenu *m_docMenu;
    TDEActionMenu *m_toolMenu;

    TDEActionMenu *m_gotoToolDockMenu;
};

class ToggleToolViewAction:public TDEToggleAction
{
  TQ_OBJECT

  public:
    ToggleToolViewAction ( const TQString& text, const TDEShortcut& cut = TDEShortcut(),
                           KDockWidget *dw=0,KMDI::MainWindow *mdiMainFrm=0, TQObject* parent = 0, const char* name = 0 );

    virtual ~ToggleToolViewAction();

  protected slots:
    void slotToggled(bool);
    void anDWChanged();
    void slotWidgetDestroyed();

  private:
    KDockWidget *m_dw;
    KMDI::MainWindow *m_mdiMainFrm;
};

}

#endif
