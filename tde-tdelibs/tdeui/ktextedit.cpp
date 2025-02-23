/* This file is part of the KDE libraries
   Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "ktextedit.h"

#include <tqapplication.h>
#include <tqclipboard.h>
#include <tqpopupmenu.h>

#include <ksyntaxhighlighter.h>
#include <tdespell.h>
#include <kcursor.h>
#include <tdeglobalsettings.h>
#include <tdestdaccel.h>
#include <kiconloader.h>
#include <tdelocale.h>

class KTextEdit::KTextEditPrivate
{
public:
    KTextEditPrivate()
        : customPalette( false ),
          checkSpellingEnabled( false ),
          highlighter( 0 ),
          spell( 0 )
    {}
    ~KTextEditPrivate() {
        delete highlighter;
        delete spell;
    }

    bool customPalette;
    bool checkSpellingEnabled;
    KDictSpellingHighlighter *highlighter;
    KSpell *spell;
};

KTextEdit::KTextEdit( const TQString& text, const TQString& context,
                      TQWidget *parent, const char *name )
    : TQTextEdit ( text, context, parent, name )
{
    d = new KTextEditPrivate();
    KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::KTextEdit( TQWidget *parent, const char *name )
    : TQTextEdit ( parent, name )
{
    d = new KTextEditPrivate();
    KCursor::setAutoHideCursor( this, true, false );
}

KTextEdit::~KTextEdit()
{
    delete d;
}

void KTextEdit::keyPressEvent( TQKeyEvent *e )
{
    KKey key( e );

    if ( TDEStdAccel::copy().contains( key ) ) {
        copy();
        e->accept();
        return;
    }
    else if ( TDEStdAccel::paste().contains( key ) ) {
        paste();
        e->accept();
        return;
    }
    else if ( TDEStdAccel::cut().contains( key ) ) {
        cut();
        e->accept();
        return;
    }
    else if ( TDEStdAccel::undo().contains( key ) ) {
        undo();
        e->accept();
        return;
    }
    else if ( TDEStdAccel::redo().contains( key ) ) {
        redo();
        e->accept();
        return;
    }
    else if ( TDEStdAccel::deleteWordBack().contains( key ) )
    {
        deleteWordBack();
        e->accept();
        return;
    }
    else if ( TDEStdAccel::deleteWordForward().contains( key ) )
    {
        deleteWordForward();
        e->accept();
        return;
    }
    else if ( TDEStdAccel::backwardWord().contains( key ) )
    {
      CursorAction action = MoveWordBackward;
      int para, index;
      getCursorPosition( &para, & index );
      if (text(para).isRightToLeft())
           action = MoveWordForward;
      moveCursor(action, false );
      e->accept();
      return;
    }
    else if ( TDEStdAccel::forwardWord().contains( key ) )
    {
      CursorAction action = MoveWordForward;
      int para, index;
      getCursorPosition( &para, & index );
      if (text(para).isRightToLeft())
	  action = MoveWordBackward;
      moveCursor( action, false );
      e->accept();
      return;
    }
    else if ( TDEStdAccel::next().contains( key ) )
    {
      moveCursor( MovePgDown, false );
      e->accept();
      return;
    }
    else if ( TDEStdAccel::prior().contains( key ) )
    {
      moveCursor( MovePgUp, false );
      e->accept();
      return;
    }
    else if ( TDEStdAccel::home().contains( key ) )
    {
      moveCursor( MoveHome, false );
      e->accept();
      return;
    }
    else if ( TDEStdAccel::end().contains( key ) )
    {
      moveCursor( MoveEnd, false );
      e->accept();
      return;
    }
    else if ( TDEStdAccel::beginningOfLine().contains( key ) )
    {
      moveCursor( MoveLineStart, false );
      e->accept();
      return;
    }
    else if ( TDEStdAccel::endOfLine().contains( key ) )
    {
      moveCursor(MoveLineEnd, false);
      e->accept();
      return;
    }
    else if ( TDEStdAccel::pasteSelection().contains( key ) )
    {
        TQString text = TQApplication::clipboard()->text( TQClipboard::Selection);
        if ( !text.isEmpty() )
            insert( text );
        e->accept();
        return;
    }

    // ignore Ctrl-Return so that KDialogs can close the dialog
    else if ( e->state() == ControlButton &&
              (e->key() == Key_Return || e->key() == Key_Enter) &&
              topLevelWidget()->inherits( "KDialog" ) )
    {
        e->ignore();
        return;
    }
    
    TQTextEdit::keyPressEvent( e );
}

void KTextEdit::deleteWordBack()
{
    removeSelection();
    moveCursor( MoveWordBackward, true );
    removeSelectedText();
}

void KTextEdit::deleteWordForward()
{
    removeSelection();
    moveCursor( MoveWordForward, true );
    removeSelectedText();
}

void KTextEdit::slotAllowTab()
{
setTabChangesFocus(!tabChangesFocus());
}

TQPopupMenu *KTextEdit::createPopupMenu( const TQPoint &pos )
{
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    TQPopupMenu *menu = TQTextEdit::createPopupMenu( pos );

    if ( isReadOnly() )
      menu->changeItem( menu->idAt(0), SmallIconSet("edit-copy"), menu->text( menu->idAt(0) ) );
    else {
      int id = menu->idAt(0);
      menu->changeItem( id - IdUndo, SmallIconSet("edit-undo"), menu->text( id - IdUndo) );
      menu->changeItem( id - IdRedo, SmallIconSet("edit-redo"), menu->text( id - IdRedo) );
      menu->changeItem( id - IdCut, SmallIconSet("edit-cut"), menu->text( id - IdCut) );
      menu->changeItem( id - IdCopy, SmallIconSet("edit-copy"), menu->text( id - IdCopy) );
      menu->changeItem( id - IdPaste, SmallIconSet("edit-paste"), menu->text( id - IdPaste) );
      menu->changeItem( id - IdClear, SmallIconSet("edit-clear"), menu->text( id - IdClear) );

        menu->insertSeparator();
        id = menu->insertItem( SmallIconSet( "tools-check-spelling" ), i18n( "Check Spelling..." ),
                                   this, TQ_SLOT( checkSpelling() ) );

        if( text().isEmpty() )
            menu->setItemEnabled( id, false );

        id = menu->insertItem( i18n( "Auto Spell Check" ),
                               this, TQ_SLOT( toggleAutoSpellCheck() ) );
        menu->setItemChecked(id, d->checkSpellingEnabled);
	menu->insertSeparator();
	id=menu->insertItem(i18n("Allow Tabulations"),this,TQ_SLOT(slotAllowTab()));
	menu->setItemChecked(id, !tabChangesFocus());
    }

    return menu;
}

TQPopupMenu *KTextEdit::createPopupMenu()
{
    return TQTextEdit::createPopupMenu();
}

void KTextEdit::contentsWheelEvent( TQWheelEvent *e )
{
    if ( TDEGlobalSettings::wheelMouseZooms() )
        TQTextEdit::contentsWheelEvent( e );
    else // thanks, we don't want to zoom, so skip QTextEdit's impl.
        TQScrollView::contentsWheelEvent( e );
}

void KTextEdit::setPalette( const TQPalette& palette )
{
    TQTextEdit::setPalette( palette );
    // unsetPalette() is not virtual and calls setPalette() as well
    // so we can use ownPalette() to find out about unsetting
    d->customPalette = ownPalette();
}

void KTextEdit::toggleAutoSpellCheck()
{
    setCheckSpellingEnabled( !d->checkSpellingEnabled );
}

void KTextEdit::setCheckSpellingEnabled( bool check )
{
    if ( check == d->checkSpellingEnabled )
        return;

    // From the above statment we know know that if we're turning checking
    // on that we need to create a new highlighter and if we're turning it
    // off we should remove the old one.

    d->checkSpellingEnabled = check;
    if ( check )
    {
        if (hasFocus())
            d->highlighter = new KDictSpellingHighlighter( this );
    }
    else
    {
        delete d->highlighter;
        d->highlighter = 0;
    }
}

void KTextEdit::focusInEvent( TQFocusEvent *e )
{
    if ( d->checkSpellingEnabled && !isReadOnly() && !d->highlighter )
        d->highlighter = new KDictSpellingHighlighter( this );

    TQTextEdit::focusInEvent( e );
}

bool KTextEdit::checkSpellingEnabled() const
{
    return d->checkSpellingEnabled;
}

void KTextEdit::setReadOnly(bool readOnly)
{
    if ( !readOnly && hasFocus() && d->checkSpellingEnabled && !d->highlighter )
        d->highlighter = new KDictSpellingHighlighter( this );
	
    if ( readOnly == isReadOnly() )
        return;

    if (readOnly)
    {
	delete d->highlighter;
	d->highlighter = 0;
	    
        bool custom = ownPalette();
        TQPalette p = palette();
        TQColor color = p.color(TQPalette::Disabled, TQColorGroup::Background);
        p.setColor(TQColorGroup::Base, color);
        p.setColor(TQColorGroup::Background, color);
        setPalette(p);
        d->customPalette = custom;
    }
    else
    {
        if ( d->customPalette )
        {
            TQPalette p = palette();
            TQColor color = p.color(TQPalette::Normal, TQColorGroup::Base);
            p.setColor(TQColorGroup::Base, color);
            p.setColor(TQColorGroup::Background, color);
            setPalette( p );
        }
        else
            unsetPalette();
    }

    TQTextEdit::setReadOnly (readOnly);
}

void KTextEdit::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KTextEdit::checkSpelling()
{
    delete d->spell;
    d->spell = new KSpell( this, i18n( "Spell Checking" ),
                          this, TQ_SLOT( slotSpellCheckReady( KSpell *) ), 0, true, true);

    connect( d->spell, TQ_SIGNAL( death() ),
             this, TQ_SLOT( spellCheckerFinished() ) );

    connect( d->spell, TQ_SIGNAL( misspelling( const TQString &, const TQStringList &, unsigned int ) ),
             this, TQ_SLOT( spellCheckerMisspelling( const TQString &, const TQStringList &, unsigned int ) ) );

    connect( d->spell, TQ_SIGNAL( corrected( const TQString &, const TQString &, unsigned int ) ),
             this, TQ_SLOT( spellCheckerCorrected( const TQString &, const TQString &, unsigned int ) ) );
}

void KTextEdit::spellCheckerMisspelling( const TQString &text, const TQStringList &, unsigned int pos )
{
    highLightWord( text.length(), pos );
}

void KTextEdit::spellCheckerCorrected( const TQString &oldWord, const TQString &newWord, unsigned int pos )
{
    unsigned int l = 0;
    unsigned int cnt = 0;
    if ( oldWord != newWord ) {
        posToRowCol( pos, l, cnt );
        setSelection( l, cnt, l, cnt + oldWord.length() );
        removeSelectedText();
        insert( newWord );
    }
}

void KTextEdit::posToRowCol(unsigned int pos, unsigned int &line, unsigned int &col)
{
    for ( line = 0; line < static_cast<uint>( lines() ) && col <= pos; line++ )
        col += paragraphLength( line ) + 1;

    line--;
    col = pos - col + paragraphLength( line ) + 1;
}

void KTextEdit::spellCheckerFinished()
{
    delete d->spell;
    d->spell = 0L;
}

void KTextEdit::slotSpellCheckReady( KSpell *s )
{
    s->check( text() );
    connect( s, TQ_SIGNAL( done( const TQString & ) ), this, TQ_SLOT( slotSpellCheckDone( const TQString & ) ) );
}

void KTextEdit::slotSpellCheckDone( const TQString &s )
{
    if ( s != text() )
        setText( s );
}


void KTextEdit::highLightWord( unsigned int length, unsigned int pos )
{
    unsigned int l = 0;
    unsigned int cnt = 0;
    posToRowCol( pos, l, cnt );
    setSelection( l, cnt, l, cnt + length );
}

#include "ktextedit.moc"
