/* This file is part of the KDE libraries

   Copyright (c) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KCURSOR_PRIVATE_H
#define KCURSOR_PRIVATE_H

#include <tqcursor.h>
#include <tqobject.h>
#include <tqptrdict.h>

class TQPoint;
class TQTimer;
class TQWidget;

/**
 * I don't want the eventFilter to be in KCursor, so we have another class
 * for that stuff
 * @internal
 * @author John Firebaugh <jfirebaugh@kde.org>
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
*/
class KCursorPrivateAutoHideEventFilter : public TQObject
{
    TQ_OBJECT

public:
    KCursorPrivateAutoHideEventFilter( TQWidget* widget );
    ~KCursorPrivateAutoHideEventFilter();

    virtual bool eventFilter( TQObject *o, TQEvent *e );
    
    void resetWidget();

private slots:
    void hideCursor();
    void unhideCursor();

private:
    TQWidget* actualWidget() const;

    TQTimer m_autoHideTimer;
    TQWidget* m_widget;
    bool m_wasMouseTracking;
    bool m_isCursorHidden;
    bool m_isOwnCursor;
    TQCursor m_oldCursor;
};

/**
 * @internal
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @author John Firebaugh <jfirebaugh@kde.org>
*/
class KCursorPrivate : public TQObject
{
    friend class KCursor; // to shut up the compiler
    TQ_OBJECT

public:
    static KCursorPrivate *self();

    void setAutoHideCursor( TQWidget *w, bool enable, bool customEventFilter );
    virtual bool eventFilter( TQObject *o, TQEvent *e );

    int hideCursorDelay;

private slots:
    void slotWidgetDestroyed( TQObject* );

private:
    KCursorPrivate();
    ~KCursorPrivate();

    bool enabled;
    static KCursorPrivate *s_self;

    TQPtrDict<KCursorPrivateAutoHideEventFilter> m_eventFilters;
};



#endif // KCURSOR_PRIVATE_H
