/* This file is part of the KDE libraries
   Copyright (C) 2002 Simon Hausmann <hausmann@kde.org>

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

#ifndef TDEBARHANDLER_H
#define TDEBARHANDLER_H

#include <tqobject.h>
#include <tqguardedptr.h>
#include <kxmlguiclient.h>

class TDEMainWindow;
class TDEToolBar;

namespace KDEPrivate
{

/// @since 3.1
class ToolBarHandler : public TQObject,
                       public KXMLGUIClient
{
    TQ_OBJECT
public:
    ToolBarHandler( TDEMainWindow *mainWindow, const char *name = 0 );
    ToolBarHandler( TDEMainWindow *mainWindow, TQObject *parent, const char *name = 0 );
    virtual ~ToolBarHandler();

    TDEAction *toolBarMenuAction();

public slots:
    void setupActions();

private slots:
    void clientAdded( KXMLGUIClient *client );

private:
    void init( TDEMainWindow *mainWindow );
    void connectToActionContainers();
    void connectToActionContainer( TDEAction *action );
    void connectToActionContainer( TQWidget *container );

    struct Data;
    Data *d;

    TQGuardedPtr<TDEMainWindow> m_mainWindow;
    TQPtrList<TDEAction> m_actions;
    TQPtrList<TDEToolBar> m_toolBars;
};

} // namespace KDEPrivate

#endif // TDEBARHANDLER_H
