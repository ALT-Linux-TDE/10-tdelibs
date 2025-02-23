/* This file is part of the KDE libraries
    Copyright (C) 1997 Alexander Sanda (alex@darkstar.ping.at)

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
*/
/*
 * $Id$
*/

#ifndef KTABCTL_H
#define KTABCTL_H

#include <tqwidget.h>
#include <tqtabbar.h>
#include <tqmemarray.h>

#include <tdelibs_export.h>

/**
 * Tabbed dialog with extended features.
 * KTabCtl is very similar to TQTabDialog, with the following differences:
 * 
 * @li To avoid confusion, the API is almost identical with TQTabDialog.
 * @li Does not create any buttons, therefore KTabCtl is not limited
 * to dialog boxes. You can use it whereever you want.
 * @li emits the signal tabSelected(int pagenumber) when the user
 * selects one of the tabs. This gives you the chance to update the
 * widget contents of a single page. The signal is emitted _before_ the
 * page is shown.  This is very useful if the contents of some widgets
 * on page A depend on the contents of some other widgets on page B.
 *
 * @author Alexander Sanda (alex@darkstar.ping.at)
 * @version $Id$
*/
class TDEUI_EXPORT KTabCtl : public TQWidget
{
    TQ_OBJECT

public:
    KTabCtl(TQWidget *parent = 0, const char *name = 0);
   ~KTabCtl();

    void show();
    void setFont(const TQFont & font);
    void setTabFont( const TQFont &font );

    void addTab(TQWidget *, const TQString&);
    bool isTabEnabled(const TQString& );
    void setTabEnabled(const TQString&, bool);
    void setBorder(bool);
    void setShape( TQTabBar::Shape shape );
    virtual TQSize sizeHint() const;

protected:
    void paintEvent(TQPaintEvent *);
    void resizeEvent(TQResizeEvent *);

signals:
    void tabSelected(int);
    
protected slots:
    void showTab(int i);

protected:
    void setSizes();
    TQRect getChildRect() const;

    TQTabBar * tabs;
    TQMemArray<TQWidget *> pages;
    int bh;
    bool blBorder;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KTabCtrlPrivate* d;
};
#endif
