/*
 * dialog.h
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
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
#ifndef TDESPELL_DIALOG_H
#define TDESPELL_DIALOG_H

#include <kdialogbase.h>

namespace KSpell2
{
    class Filter;
    class BackgroundChecker;
    class TDE_EXPORT Dialog : public KDialogBase
    {
        TQ_OBJECT
    public:
        Dialog( BackgroundChecker *checker,
                TQWidget *parent, const char *name=0 );
        ~Dialog();

        TQString originalBuffer() const;
        TQString buffer() const;

        void show();
        void activeAutoCorrect( bool _active );

    public slots:
        void setBuffer( const TQString& );
        void setFilter( Filter* filter );

    signals:
        void done( const TQString& newBuffer );
        void misspelling( const TQString& word, int start );
        void replace( const TQString& oldWord, int start,
                      const TQString& newWord );

        void stop();
        void cancel();
        void autoCorrect( const TQString & currentWord, const TQString & replaceWord );
    private slots:
        void slotMisspelling(const TQString& word, int start );
        void slotDone();

        void slotFinished();
        void slotCancel();

        void slotAddWord();
        void slotReplaceWord();
        void slotReplaceAll();
        void slotSkip();
        void slotSkipAll();
        void slotSuggest();
        void slotChangeLanguage( const TQString& );
        void slotSelectionChanged( TQListViewItem * );
        void slotAutocorrect();

    private:
        void updateDialog( const TQString& word );
        void fillSuggestions( const TQStringList& suggs );
        void initConnections();
        void initGui();
        void continueChecking();

    private:
        class Private;
        Private *d;
    };
}

#endif
