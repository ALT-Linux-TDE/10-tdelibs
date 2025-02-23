/*
 * dialog.cpp
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
#include "dialog.h"
#include "tdespell2ui.h"

#include "backgroundchecker.h"
#include "broker.h"
#include "filter.h"
#include "dictionary.h"
#include "settings.h"

#include <tdeconfig.h>
#include <tdelocale.h>
#include <kdebug.h>

#include <tqlistview.h>
#include <tqpushbutton.h>
#include <tqcombobox.h>
#include <tqlineedit.h>
#include <tqlabel.h>
#include <tqtimer.h>
#include <tqdict.h>

namespace KSpell2
{

//to initially disable sorting in the suggestions listview
#define NONSORTINGCOLUMN 2

class Dialog::Private
{
public:
    KSpell2UI *ui;
    TQString   originalBuffer;
    BackgroundChecker *checker;

    Word   currentWord;
    TQMap<TQString, TQString> replaceAllMap;
};

Dialog::Dialog( BackgroundChecker *checker,
                TQWidget *parent, const char *name )
    : KDialogBase( parent, name, true,
                   i18n( "Check Spelling" ),
                   Help|Cancel|User1, Cancel,  true,
                   i18n( "&Finished" ) )
{
    d = new Private;

    d->checker = checker;

    initGui();
    initConnections();
    setMainWidget( static_cast<TQWidget*>(d->ui) );
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::initConnections()
{
    connect( d->ui->m_addBtn, TQ_SIGNAL(clicked()),
             TQ_SLOT(slotAddWord()) );
    connect( d->ui->m_replaceBtn, TQ_SIGNAL(clicked()),
             TQ_SLOT(slotReplaceWord()) );
    connect( d->ui->m_replaceAllBtn, TQ_SIGNAL(clicked()),
             TQ_SLOT(slotReplaceAll()) );
    connect( d->ui->m_skipBtn, TQ_SIGNAL(clicked()),
             TQ_SLOT(slotSkip()) );
    connect( d->ui->m_skipAllBtn, TQ_SIGNAL(clicked()),
             TQ_SLOT(slotSkipAll()) );
    connect( d->ui->m_suggestBtn, TQ_SIGNAL(clicked()),
             TQ_SLOT(slotSuggest()) );
    connect( d->ui->m_language, TQ_SIGNAL(activated(const TQString&)),
             TQ_SLOT(slotChangeLanguage(const TQString&)) );
    connect( d->ui->m_suggestions, TQ_SIGNAL(selectionChanged(TQListViewItem*)),
             TQ_SLOT(slotSelectionChanged(TQListViewItem*)) );
    connect( d->checker, TQ_SIGNAL(misspelling(const TQString&, int)),
             TQ_SIGNAL(misspelling(const TQString&, int)) );
    connect( d->checker, TQ_SIGNAL(misspelling(const TQString&, int)),
             TQ_SLOT(slotMisspelling(const TQString&, int)) );
    connect( d->checker, TQ_SIGNAL(done()),
             TQ_SLOT(slotDone()) );
    connect( d->ui->m_suggestions, TQ_SIGNAL(doubleClicked(TQListViewItem*, const TQPoint&, int)),
             TQ_SLOT( slotReplaceWord() ) );
    connect( this, TQ_SIGNAL(user1Clicked()), this, TQ_SLOT(slotFinished()) );
    connect( this, TQ_SIGNAL(cancelClicked()),this, TQ_SLOT(slotCancel()) );
    connect( d->ui->m_replacement, TQ_SIGNAL(returnPressed()), this, TQ_SLOT(slotReplaceWord()) );
    connect( d->ui->m_autoCorrect, TQ_SIGNAL(clicked()),
             TQ_SLOT(slotAutocorrect()) );
    // button use by kword/kpresenter
    // hide by default
    d->ui->m_autoCorrect->hide();
}

void Dialog::initGui()
{
    d->ui = new KSpell2UI( this );
    d->ui->m_suggestions->setSorting( NONSORTINGCOLUMN );
    d->ui->m_language->clear();
    d->ui->m_language->insertStringList( d->checker->broker()->languages() );
    for ( int i = 0; !d->ui->m_language->text( i ).isNull(); ++i ) {
        TQString ct = d->ui->m_language->text( i );
        if ( ct == d->checker->broker()->settings()->defaultLanguage() ) {
            d->ui->m_language->setCurrentItem( i );
            break;
        }
    }
}

void Dialog::activeAutoCorrect( bool _active )
{
    if ( _active )
        d->ui->m_autoCorrect->show();
    else
        d->ui->m_autoCorrect->hide();
}

void Dialog::slotAutocorrect()
{
    kdDebug()<<"void Dialog::slotAutocorrect()\n";
    emit autoCorrect(d->currentWord.word, d->ui->m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotFinished()
{
    kdDebug()<<"void Dialog::slotFinished() \n";
    emit stop();
    //FIXME: should we emit done here?
    emit done( d->checker->filter()->buffer() );
    accept();
}

void Dialog::slotCancel()
{
    kdDebug()<<"void Dialog::slotCancel() \n";
    emit cancel();
    reject();
}

TQString Dialog::originalBuffer() const
{
    return d->originalBuffer;
}

TQString Dialog::buffer() const
{
    return d->checker->filter()->buffer();
}

void Dialog::setBuffer( const TQString& buf )
{
    d->originalBuffer = buf;
}

void Dialog::setFilter( Filter *filter )
{
    filter->setBuffer( d->checker->filter()->buffer() );
    d->checker->setFilter( filter );
}

void Dialog::updateDialog( const TQString& word )
{
    d->ui->m_unknownWord->setText( word );
    d->ui->m_contextLabel->setText( d->checker->filter()->context() );
    TQStringList suggs = d->checker->suggest( word );
    d->ui->m_replacement->setText( suggs.first() );
    fillSuggestions( suggs );
}

void Dialog::show()
{
    kdDebug()<<"Showing dialog"<<endl;
    if ( d->originalBuffer.isEmpty() )
        d->checker->start();
    else
        d->checker->checkText( d->originalBuffer );
}

void Dialog::slotAddWord()
{
   d->checker->addWord( d->currentWord.word ); 
   d->checker->continueChecking();
}

void Dialog::slotReplaceWord()
{
    emit replace( d->currentWord.word, d->currentWord.start,
                  d->ui->m_replacement->text() );
    d->checker->filter()->replace( d->currentWord, d->ui->m_replacement->text() );
    d->checker->continueChecking();
}

void Dialog::slotReplaceAll()
{
    d->replaceAllMap.insert( d->currentWord.word,
                             d->ui->m_replacement->text() );
    slotReplaceWord();
}

void Dialog::slotSkip()
{
    d->checker->continueChecking();
}

void Dialog::slotSkipAll()
{
    //### do we want that or should we have a d->ignoreAll list?
    d->checker->broker()->settings()->addWordToIgnore( d->ui->m_replacement->text() );
    d->checker->continueChecking();
}

void Dialog::slotSuggest()
{
    TQStringList suggs = d->checker->suggest( d->ui->m_replacement->text() );
    fillSuggestions( suggs );
}

void Dialog::slotChangeLanguage( const TQString& lang )
{
    d->checker->changeLanguage( lang );
    slotSuggest();
}

void Dialog::slotSelectionChanged( TQListViewItem *item )
{
    d->ui->m_replacement->setText( item->text( 0 ) );
}

void Dialog::fillSuggestions( const TQStringList& suggs )
{
    d->ui->m_suggestions->clear();
    for ( TQStringList::ConstIterator it = suggs.begin(); it != suggs.end(); ++it ) {
        new TQListViewItem( d->ui->m_suggestions, d->ui->m_suggestions->firstChild(),
                           *it );
    }
}

void Dialog::slotMisspelling(const TQString& word, int start )
{
    kdDebug()<<"Dialog misspelling!!"<<endl;
    d->currentWord = Word( word, start );
    if ( d->replaceAllMap.contains( word ) ) {
        d->ui->m_replacement->setText( d->replaceAllMap[ word ] );
        slotReplaceWord();
    } else {
        updateDialog( word );
    }
    KDialogBase::show();
}

void Dialog::slotDone()
{
    kdDebug()<<"Dialog done!"<<endl;
    emit done( d->checker->filter()->buffer() );
    accept();
}

}

#include "dialog.moc"
