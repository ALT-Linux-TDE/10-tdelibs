/**
 * backgroundchecker.h
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
#ifndef TDESPELL_BACKGROUNDCHECKER_H
#define TDESPELL_BACKGROUNDCHECKER_H

#include "broker.h"

class TQCustomEvent;

namespace KSpell2
{
    class Filter;

    /**
     *
     * BackgroundChecker is used to perform spell checking without
     * blocking the application. You can use it as is by calling
     * the checkText function or subclass it and reimplement
     * getMoreText function.
     *
     * The misspelling signal is emitted whenever a mispelled word
     * is found. The background checker stops right before emitting
     * the signal. So the parent has to call continueChecking function
     * to resume the checking.
     *
     * done signal is emitted when whole text is spell checked.
     *
     * @author Zack Rusin <zack@kde.org>
     * @short class used for spell checking in the background
     */
    class TDE_EXPORT BackgroundChecker : public TQObject
    {
        TQ_OBJECT
    public:
        BackgroundChecker( const Broker::Ptr& broker, TQObject *parent =0,
                           const char *name =0 );
        ~BackgroundChecker();

        /**
         * This method is used to spell check static text.
         * It automatically invokes start().
         *
         * Use getMoreText() with start() to spell check a stream.
         */
        void checkText( const TQString& );

        Filter *filter() const;

        Broker *broker() const;
        void changeLanguage( const TQString& lang );

        bool checkWord( const TQString& word );
        TQStringList suggest( const TQString& ) const;
        bool addWord( const TQString& word );
    public slots:
        virtual void setFilter( KSpell2::Filter *filter );
        virtual void start();
        virtual void stop();

        /**
         * After emitting misspelling signal the background
         * checker stops. The catcher is responsible for calling
         * continueChecking function to resume checking.
         */
        virtual void continueChecking();

    signals:
        /**
         * Emitted whenever a misspelled word is found
         */
        void misspelling( const TQString& word, int start );

        /**
         * Emitted after the whole text has been spell checked.
         */
        void done();

    protected:
        /**
         * This function is called to get the text to spell check.
         * It will be called continuesly until it returns TQString::null
         * in which case the done() singnal is emitted.
         * Note: the start parameter in mispelling() is not a combined
         * position but a position in the last string returned
         * by getMoreText. You need to store the state in the derivatives.
         */
        virtual TQString getMoreText();

        /**
         * This function will be called whenever the background checker
         * will be finished text which it got from getMoreText.
         */
        virtual void finishedCurrentFeed();

    protected slots:
        void slotEngineDone();
    protected:
        //void customEvent( TQCustomEvent *event );
    private:
        class Private;
        Private *d;
    };

}

#endif
