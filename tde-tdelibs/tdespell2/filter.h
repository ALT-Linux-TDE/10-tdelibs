/*
 * filter.h
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

#ifndef TDESPELL_FILTER_H
#define TDESPELL_FILTER_H

#include <tqstring.h>
#include <tdelibs_export.h>

namespace KSpell2
{
    class Settings;

    /**
     * Structure abstracts the word and its position in the
     * parent text.
     *
     * @author Zack Rusin <zack@kde.org>
     * @short struct represents word
     */
    struct Word
    {
        Word() : start( 0 ), end( true )
            {}

        Word( const TQString& w, int st, bool e = false )
            : word( w ), start( st ), end( e )
            {}
        Word( const Word& other )
            : word( other.word ), start( other.start ),
              end( other.end )
            {}

        TQString word;
        uint    start;
        bool    end;
    };

    /**
     * Filter is used to split text into words which
     * will be spell checked.
     *
     * @author Zack Rusin <zack@kde.org>
     * @short used to split text into words
     */
    class TDE_EXPORT Filter
    {
    public:
        static Filter *defaultFilter();
    public:
        Filter();
        virtual ~Filter();

        static Word end();

        /**
         * Sets the Settings object for this Filter
         */
        void setSettings( Settings* );

        /**
         * Returns currently used Settings object
         */
        Settings *settings() const;

        bool atEnd() const;

        void setBuffer( const TQString& buffer );
        TQString buffer() const;

        void restart();

        virtual Word nextWord() const;
        virtual Word previousWord() const;
        virtual Word wordAtPosition( unsigned int pos ) const;

        virtual void setCurrentPosition( int );
        virtual int currentPosition() const;
        virtual void replace( const Word& w, const TQString& newWord );

        /**
         * Should return the sentence containing the current word
         */
        virtual TQString context() const;
    protected:
        bool trySkipLinks() const;
        bool ignore( const TQString& word ) const;
        TQChar skipToLetter( uint &fromPosition ) const;
        bool shouldBeSkipped( bool wordWasUppercase, bool wordWasRunTogether,
                              const TQString& foundWord ) const;

    protected:
        TQString      m_buffer;
        mutable uint m_currentPosition;

    private:
        class Private;
        Private *d;
    };

}

#endif
