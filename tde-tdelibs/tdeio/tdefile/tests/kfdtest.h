/****************************************************************************
** $Id$
**
** Copyright (C) 2003 Carsten Pfeiffer <pfeiffer@kde.org>
**
****************************************************************************/

#ifndef KFDTEST_H
#define KFDTEST_H

#include <tqobject.h>

class KFDTest : public TQObject
{
    TQ_OBJECT

public:
    KFDTest( const TQString& startDir, TQObject *parent = 0, const char *name = 0);

public slots:
    void doit();

private:
    TQString m_startDir;
};


#endif // KFDTEST_H
