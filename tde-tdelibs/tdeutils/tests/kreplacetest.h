/*
    Copyright (C) 2002, David Faure <david@mandrakesoft.com>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KREPLACETEST_H
#define KREPLACETEST_H

#include <tqobject.h>
#include <tqstringlist.h>

class KReplace;

class KReplaceTest : public TQObject
{
    TQ_OBJECT
public:
    KReplaceTest( const TQStringList& text, int button )
        : TQObject( 0L ), m_text( text ), m_replace( 0 ), m_button( button ) {}

    void replace( const TQString &pattern, const TQString &replacement, long options );
    void print();
    const TQStringList& textLines() const { return m_text; }

public slots:
    void slotHighlight( const TQString &, int, int );
    void slotReplaceNext();
    void slotReplace(const TQString &text, int replacementIndex, int replacedLength, int matchedLength);

private:
    TQStringList::Iterator m_currentPos;
    TQStringList m_text;
    KReplace* m_replace;
    bool m_needEventLoop;
    int m_button;
};

#endif
