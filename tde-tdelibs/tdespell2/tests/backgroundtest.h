/**
 * backgroundtest.h
 *
 * Copyright (C)  2004  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef BACKGROUNDTEST_H
#define BACKGROUNDTEST_H

#include <tqobject.h>
#include <tqdatetime.h>

#include "backgroundchecker.h"

class BackgroundTest : public TQObject
{
    TQ_OBJECT
public:
    BackgroundTest();

protected slots:
    void slotDone();
    void slotMisspelling( const TQString& word, int start );

private:
    KSpell2::BackgroundChecker *m_checker;
    TQTime m_timer;
    int m_len;
};

#endif
