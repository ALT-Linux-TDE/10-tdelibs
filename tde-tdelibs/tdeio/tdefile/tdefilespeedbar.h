/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef TDEFILESPEEDBAR_H
#define TDEFILESPEEDBAR_H

#include <kurlbar.h>

class TDEConfig;

class TDEIO_EXPORT KFileSpeedBar : public KURLBar
{
    TQ_OBJECT
public:
    KFileSpeedBar( TQWidget *parent = 0, const char *name = 0 );
    ~KFileSpeedBar();

    virtual void save( TDEConfig *config );
    virtual TQSize sizeHint() const;

private:
    bool m_initializeSpeedbar : 1;

};

#endif // TDEFILESPEEDBAR_H
