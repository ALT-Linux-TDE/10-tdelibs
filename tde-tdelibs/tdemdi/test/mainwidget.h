/***************************************************************************
                          mainwidget.h  -  description
                             -------------------
    begin                : Mon Nov 8 1999
    copyright            : (C) 1999 by Falk Brettschneider
    email                : falkbr@kdevelop.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqdom.h>
#include <tdemdimainfrm.h>

/**
  *@author Falk Brettschneider
  */
class MainWidget : public KMdiMainFrm
{
    TQ_OBJECT
public:
    MainWidget(TQDomElement& dockConfig,KMdi::MdiMode mode);
    virtual ~MainWidget();
    void initMenu();

protected: // Protected methods
    virtual void resizeEvent( TQResizeEvent *pRSE);
private:
    TQDomElement m_dockConfig;
};



/**
  *@author Falk Brettschneider
  * This allows me to test KMdiMainFrm::read/writeDockConfig by
  * closing and restarting the MainWidget via checkbox click.
  */
class RestartWidget : public TDEMainWindow
{
    TQ_OBJECT
// methods
public:
    RestartWidget();
    void setWindow(MainWidget *w);

private slots:
    void onStateChanged(int on);

// attributes
public:
    TQDomDocument domDoc;
    TQDomElement dockConfig;
    KMdi::MdiMode mdimode;
private:
    MainWidget *m_w;
};

#endif
