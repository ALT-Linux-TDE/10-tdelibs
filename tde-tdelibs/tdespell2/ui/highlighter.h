/*
 * highlighter.h
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
#ifndef TDESPELL_HIGHLIGHTER_H
#define TDESPELL_HIGHLIGHTER_H

#include "filter.h"

#include <tqsyntaxhighlighter.h>

class TQTextEdit;

namespace KSpell2
{
    class Highlighter : public TQSyntaxHighlighter
    {
    public:
        Highlighter( TQTextEdit *textEdit,
                     const TQString& configFile = TQString::null,
                     Filter *filter = Filter::defaultFilter() );
        ~Highlighter();

        virtual int highlightParagraph( const TQString& text,
                                        int endStateOfLastPara );

        Filter *currentFilter() const;
        void setCurrentFilter( Filter *filter );

        TQString currentLanguage() const;
        void setCurrentLanguage( const TQString& lang );

    protected:
        virtual void setMisspelled( int start, int count );
        virtual void unsetMisspelled( int start,  int count );
    private:
        class Private;
        Private *d;
    };

}

#endif
