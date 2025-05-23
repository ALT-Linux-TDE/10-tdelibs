/* This file is part of the KDE project
   Copyright (C) 2000 Werner Trobin <trobin@kde.org>
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kcommand.h"
#include <tdeaction.h>
#include <tdestdaccel.h>
#include <kstdaction.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdepopupmenu.h>

KCommand::~KCommand()
{
}

KMacroCommand::KMacroCommand( const TQString & name ) : KNamedCommand(name)
{
    m_commands.setAutoDelete(true);
}

void KMacroCommand::addCommand(KCommand *command)
{
    m_commands.append(command);
}

void KMacroCommand::execute()
{
    TQPtrListIterator<KCommand> it(m_commands);
    for ( ; it.current() ; ++it )
        it.current()->execute();
}

void KMacroCommand::unexecute()
{
    TQPtrListIterator<KCommand> it(m_commands);
    it.toLast();
    for ( ; it.current() ; --it )
        it.current()->unexecute();
}


class KCommandHistory::KCommandHistoryPrivate {
public:
    KCommandHistoryPrivate() {
        m_savedAt=-1;
        m_present=0;
    }
    ~KCommandHistoryPrivate() {}
    int m_savedAt;
    KCommand *m_present;
};

////////////

KCommandHistory::KCommandHistory() :
    m_undo(0), m_redo(0), m_undoLimit(50), m_redoLimit(30), m_first(false)
{
    d=new KCommandHistoryPrivate();
    m_commands.setAutoDelete(true);
    clear();
}

KCommandHistory::KCommandHistory(TDEActionCollection * actionCollection, bool withMenus) :
    m_undoLimit(50), m_redoLimit(30), m_first(false)
{
    d=new KCommandHistoryPrivate();
    if (withMenus)
    {
        TDEToolBarPopupAction * undo = new TDEToolBarPopupAction( i18n("&Undo"), "edit-undo",
                                          TDEStdAccel::shortcut(TDEStdAccel::Undo), this, TQ_SLOT( undo() ),
                                          actionCollection, KStdAction::stdName( KStdAction::Undo ) );
        connect( undo->popupMenu(), TQ_SIGNAL( aboutToShow() ), this, TQ_SLOT( slotUndoAboutToShow() ) );
        connect( undo->popupMenu(), TQ_SIGNAL( activated( int ) ), this, TQ_SLOT( slotUndoActivated( int ) ) );
        m_undo = undo;
        m_undoPopup = undo->popupMenu();

        TDEToolBarPopupAction * redo = new TDEToolBarPopupAction( i18n("&Redo"), "edit-redo",
                                          TDEStdAccel::shortcut(TDEStdAccel::Redo), this, TQ_SLOT( redo() ),
                                          actionCollection, KStdAction::stdName( KStdAction::Redo ) );
        connect( redo->popupMenu(), TQ_SIGNAL( aboutToShow() ), this, TQ_SLOT( slotRedoAboutToShow() ) );
        connect( redo->popupMenu(), TQ_SIGNAL( activated( int ) ), this, TQ_SLOT( slotRedoActivated( int ) ) );
        m_redo = redo;
        m_redoPopup = redo->popupMenu();
    }
    else
    {
        m_undo = KStdAction::undo( this, TQ_SLOT( undo() ), actionCollection );
        m_redo = KStdAction::redo( this, TQ_SLOT( redo() ), actionCollection );
        m_undoPopup = 0L;
        m_redoPopup = 0L;
    }
    m_commands.setAutoDelete(true);
    clear();
}

KCommandHistory::~KCommandHistory() {
    delete d;
}

void KCommandHistory::clear() {
    if (m_undo) {
        m_undo->setEnabled(false);
        m_undo->setText(i18n("&Undo"));
    }
    if (m_redo) {
        m_redo->setEnabled(false);
        m_redo->setText(i18n("&Redo"));
    }
    d->m_present = 0L;
    d->m_savedAt=-42;
}

void KCommandHistory::addCommand(KCommand *command, bool execute) {

    if(!command)
        return;

    int index;
    if(d->m_present && (index=m_commands.findRef(d->m_present))!=-1) {
        if (m_first)
            --index;
        m_commands.insert(index+1, command);
        // truncate history
        unsigned int count=m_commands.count();
        for(unsigned int i=index+2; i<count; ++i)
            m_commands.removeLast();
        // check whether we still can reach savedAt
        if(index<d->m_savedAt)
            d->m_savedAt=-1;
        d->m_present=command;
        m_first=false;
        if (m_undo) {
            m_undo->setEnabled(true);
            m_undo->setText(i18n("&Undo: %1").arg(d->m_present->name()));
        }
        if((m_redo) && m_redo->isEnabled()) {
            m_redo->setEnabled(false);
            m_redo->setText(i18n("&Redo"));
        }
        clipCommands();
    }
    else { // either this is the first time we add a Command or something has gone wrong
        kdDebug(230) << "Initializing the Command History" << endl;
        m_commands.clear();
        m_commands.append(command);
        d->m_present=command;
        if (m_undo) {
            m_undo->setEnabled(true);
            m_undo->setText(i18n("&Undo: %1").arg(d->m_present->name()));
        }
        if (m_redo) {
            m_redo->setEnabled(false);
            m_redo->setText(i18n("&Redo"));
        }
        m_first=false;    // Michael B: yes, that *is* correct :-)
    }
    if ( execute )
    {
        command->execute();
        emit commandExecuted();
        emit commandExecuted(command);
    }
}

void KCommandHistory::undo() {

    if (m_first || !d->m_present)
        return;

    d->m_present->unexecute();
    emit commandExecuted();
    emit commandExecuted(d->m_present);
    if (m_redo) {
        m_redo->setEnabled(true);
        m_redo->setText(i18n("&Redo: %1").arg(d->m_present->name()));
    }
    int index;
    if((index=m_commands.findRef(d->m_present))!=-1 && m_commands.prev()) {
        d->m_present=m_commands.current();
        if (m_undo) {
            m_undo->setEnabled(true);
            m_undo->setText(i18n("&Undo: %1").arg(d->m_present->name()));
        }
        --index;
        if(index==d->m_savedAt)
            emit documentRestored();
    }
    else {
        if (m_undo) {
            m_undo->setEnabled(false);
            m_undo->setText(i18n("&Undo"));
        }
        if(d->m_savedAt==-42)
            emit documentRestored();
        m_first=true;
    }
    clipCommands(); // only needed here and in addCommand, NOT in redo
}

void KCommandHistory::redo() {

    int index;
    if(m_first) {
        d->m_present->execute();
        emit commandExecuted();
        emit commandExecuted(d->m_present);
        m_first=false;
        m_commands.first();
        if(!d->m_savedAt)
            emit documentRestored();
    }
    else if((index=m_commands.findRef(d->m_present))!=-1 && m_commands.next()) {
        d->m_present=m_commands.current();
        d->m_present->execute();
        emit commandExecuted();
        emit commandExecuted(d->m_present);
        ++index;
        if(index==d->m_savedAt)
            emit documentRestored();
    }

    if (m_undo) {
        m_undo->setEnabled(true);
        m_undo->setText(i18n("&Undo: %1").arg(d->m_present->name()));
    }

    if(m_commands.next()) {
        if (m_redo) {
            m_redo->setEnabled(true);
            m_redo->setText(i18n("&Redo: %1").arg(m_commands.current()->name()));
        }
    }
    else {
        if((m_redo) && m_redo->isEnabled()) {
            m_redo->setEnabled(false);
            m_redo->setText(i18n("&Redo"));
        }
    }
}

void KCommandHistory::documentSaved() {
    if(d->m_present && !m_first)
        d->m_savedAt=m_commands.findRef(d->m_present);
    else if(!d->m_present && !m_first)
        d->m_savedAt=-42;  // this value signals that the document has
                        // been saved with an empty history.
    else if(m_first)
        d->m_savedAt=-42;
}

void KCommandHistory::setUndoLimit(int limit) {

    if(limit>0 && limit!=m_undoLimit) {
        m_undoLimit=limit;
        clipCommands();
    }
}

void KCommandHistory::setRedoLimit(int limit) {

    if(limit>0 && limit!=m_redoLimit) {
        m_redoLimit=limit;
        clipCommands();
    }
}

void KCommandHistory::clipCommands() {

    int count=m_commands.count();
    if(count<=m_undoLimit && count<=m_redoLimit)
        return;

    int index=m_commands.findRef(d->m_present);
    if(index>=m_undoLimit) {
        for(int i=0; i<=(index-m_undoLimit); ++i) {
            m_commands.removeFirst();
            --d->m_savedAt;
            if(d->m_savedAt==-1)
                d->m_savedAt=-42;
        }
        index=m_commands.findRef(d->m_present); // calculate the new
        count=m_commands.count();            // values (for the redo-branch :)
        // make it easier for us... d->m_savedAt==-1 -> invalid
        if(d->m_savedAt!=-42 && d->m_savedAt<-1)
            d->m_savedAt=-1;
    }
    // adjust the index if it's the first command
    if(m_first)
        index=-1;
    if((index+m_redoLimit+1)<count) {
        if(d->m_savedAt>(index+m_redoLimit))
            d->m_savedAt=-1;
        for(int i=0; i<(count-(index+m_redoLimit+1)); ++i)
            m_commands.removeLast();
    }
}

void KCommandHistory::slotUndoAboutToShow()
{
    m_undoPopup->clear();
    int i = 0;
    if (m_commands.findRef(d->m_present)!=-1)
        while ( m_commands.current() && i<10 ) // TODO make number of items configurable ?
        {
            m_undoPopup->insertItem( i18n("Undo: %1").arg(m_commands.current()->name()), i++ );
            m_commands.prev();
        }
}

void KCommandHistory::slotUndoActivated( int pos )
{
    kdDebug(230) << "KCommandHistory::slotUndoActivated " << pos << endl;
    for ( int i = 0 ; i < pos+1; ++i )
        undo();
}

void KCommandHistory::slotRedoAboutToShow()
{
    m_redoPopup->clear();
    int i = 0;
    if (m_first)
    {
        d->m_present = m_commands.first();
        m_redoPopup->insertItem( i18n("Redo: %1").arg(d->m_present->name()), i++ );
    }
    if (m_commands.findRef(d->m_present)!=-1 && m_commands.next())
        while ( m_commands.current() && i<10 ) // TODO make number of items configurable ?
        {
            m_redoPopup->insertItem( i18n("Redo: %1").arg(m_commands.current()->name()), i++ );
            m_commands.next();
        }
}

void KCommandHistory::slotRedoActivated( int pos )
{
    kdDebug(230) << "KCommandHistory::slotRedoActivated " << pos << endl;
    for ( int i = 0 ; i < pos+1; ++i )
        redo();
}

void KCommandHistory::updateActions()
{
    if ( m_undo && m_redo )
    {
        m_undo->setEnabled( !m_first && ( d->m_present ) );
        m_redo->setEnabled(m_first || (m_commands.findRef(d->m_present)!=-1 && m_commands.next()));
    }
}

void KCommand::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KNamedCommand::virtual_hook( int id, void* data )
{ KCommand::virtual_hook( id, data ); }

void KMacroCommand::virtual_hook( int id, void* data )
{ KNamedCommand::virtual_hook( id, data ); }

void KCommandHistory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcommand.moc"
