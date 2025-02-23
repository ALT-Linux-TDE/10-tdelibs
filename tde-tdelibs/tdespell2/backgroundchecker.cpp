/**
 * backgroundchecker.cpp
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
#include "backgroundchecker.h"

#include "broker.h"
#include "backgroundengine.h"
//#include "backgroundthread.h"
//#include "threadevents.h"

#include <kdebug.h>

using namespace KSpell2;

class BackgroundChecker::Private
{
public:
    //BackgroundThread thread;
    BackgroundEngine *engine;
    TQString currentText;
};

BackgroundChecker::BackgroundChecker( const Broker::Ptr& broker, TQObject* parent,
                                      const char *name )
    : TQObject( parent, name )
{
    d = new Private;
    //d->thread.setReceiver( this );
    //d->thread.setBroker( broker );
    d->engine = new BackgroundEngine( this );
    d->engine->setBroker( broker );
    connect( d->engine, TQ_SIGNAL(misspelling( const TQString&, int )),
             TQ_SIGNAL(misspelling( const TQString&, int )) );
    connect( d->engine, TQ_SIGNAL(done()),
             TQ_SLOT(slotEngineDone()) );
}

BackgroundChecker::~BackgroundChecker()
{
    delete d;
}

void BackgroundChecker::checkText( const TQString& text )
{
    d->currentText = text;
    //d->thread.setText( text );
    d->engine->setText( text );
    d->engine->start();
}

void BackgroundChecker::start()
{
    d->currentText = getMoreText();
    // ## what if d->currentText.isEmpty()?
    //kdDebug()<<"KSpell BackgroundChecker: starting with : \"" << d->currentText << "\""<<endl;
    //d->thread.setText( d->currentText );
    d->engine->setText( d->currentText );
    d->engine->start();
}

void BackgroundChecker::stop()
{
    //d->thread.stop();
    d->engine->stop();
}

TQString BackgroundChecker::getMoreText()
{
    return TQString::null;
}

void BackgroundChecker::finishedCurrentFeed()
{
}

void BackgroundChecker::setFilter( Filter *filter )
{
    //d->thread.setFilter( filter );
    d->engine->setFilter( filter );
}

Filter *BackgroundChecker::filter() const
{
    //return d->thread.filter();
    return d->engine->filter();
}

Broker *BackgroundChecker::broker() const
{
    //return d->thread.broker();
    return d->engine->broker();
}

bool BackgroundChecker::checkWord( const TQString& word )
{
    //kdDebug()<<"checking word \""<<word<< "\""<<endl;
    return d->engine->checkWord( word );
}

bool BackgroundChecker::addWord( const TQString& word )
{
    return d->engine->addWord( word );
}

TQStringList BackgroundChecker::suggest( const TQString& word ) const
{
    //return d->thread.suggest( word );
    return d->engine->suggest( word );
}

void BackgroundChecker::changeLanguage( const TQString& lang )
{
    //d->thread.changeLanguage( lang );
    d->engine->changeLanguage( lang );
}

void BackgroundChecker::continueChecking()
{
    d->engine->continueChecking();
}

void BackgroundChecker::slotEngineDone()
{
    finishedCurrentFeed();
    d->currentText = getMoreText();

    if ( d->currentText.isNull() ) {
        emit done();
    } else {
        //d->thread.setText( d->currentText );
        d->engine->setText( d->currentText );
        d->engine->start();
    }
}

//////////////////////////////////////////////////////////////////
#if 0
void BackgroundChecker::customEvent( TQCustomEvent *event )
{
    if ( (int)event->type() == FoundMisspelling ) {
        MisspellingEvent *me = static_cast<MisspellingEvent*>( event );
        kdDebug()<<"Found misspelling of \"" << me->word() << "\"" <<endl;
        TQString currentWord = d->currentText.mid( me->position(), me->word().length() );
        if ( currentWord == me->word() )
            emit misspelling( me->word(), me->position() );
        else {
            kdDebug()<<"Cleaning up misspelling for old text which is \""<<currentWord
                     <<"\" and should be \""<<me->word()<<"\""<<endl;
        }
    } else if ( (int)event->type() == FinishedChecking ) {
        d->currentText = getMoreText();
        if ( d->currentText.isEmpty() )
            emit done();
        else
            d->thread.setText( d->currentText );
    } else {
        TQObject::customEvent( event );
    }
}
#endif

#include "backgroundchecker.moc"
