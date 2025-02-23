/**
 * This file is part of the HTML rendering engine for KDE.
 *
 * Copyright (C) 2004-2005 Allan Sandfeld Jensen (kde@carewolf.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef ENUMERATE_H
#define ENUMERATE_H

class TQChar;
class TQString;

namespace tdehtml {

namespace Enumerate {

// Numeric
    TQString toArabicIndic( int number );
    TQString toLao( int number );
    TQString toPersianUrdu( int number );
    TQString toThai( int number );
    TQString toTibetan( int number );

// Alphabetic
    TQString toLowerLatin( int number );
    TQString toUpperLatin( int number );
    TQString toLowerGreek( int number );
    TQString toUpperGreek( int number );
    TQString toHiragana( int number );
    TQString toHiraganaIroha( int number );
    TQString toKatakana( int number );
    TQString toKatakanaIroha( int number );

// Algorithmic
    TQString toRoman( int number, bool upper );
    TQString toHebrew( int number );
    TQString toGeorgian( int number );
    TQString toArmenian( int number );

// Ideographic
    TQString toJapaneseFormal   ( int number );
    TQString toJapaneseInformal ( int number );
    TQString toSimpChineseFormal   ( int number );
    TQString toSimpChineseInformal ( int number );
    TQString toTradChineseFormal   ( int number );
    TQString toTradChineseInformal ( int number );

}} // namespaces

#endif
