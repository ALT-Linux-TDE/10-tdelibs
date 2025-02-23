/****************************************************************************
** $Id$
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef HELLO_H
#define HELLO_H

#include <tqstring.h>
#include <tdemdichildview.h>

class TQWidget;
class TQMouseEvent;
class TQPaintEvent;

class Hello : public KMdiChildView
{
    TQ_OBJECT
public:
    Hello( const char *title, const char *text, TQWidget* parentWidget = 0 );
signals:
    void clicked();
protected:
    void mouseReleaseEvent( TQMouseEvent * );
    void paintEvent( TQPaintEvent * );
private slots:
    void animate();
private:
    TQString t;
    int     b;
};

#endif
