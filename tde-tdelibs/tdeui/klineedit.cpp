/* This file is part of the KDE libraries

   Copyright (C) 1997 Sven Radej (sven.radej@iname.com)
   Copyright (c) 1999 Patrick Ward <PAT_WARD@HP-USA-om5.om.hp.com>
   Copyright (c) 1999 Preston Brown <pbrown@kde.org>

   Re-designed for KDE 2.x by
   Copyright (c) 2000, 2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000, 2001 Carsten Pfeiffer <pfeiffer@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any later
   version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqclipboard.h>
#include <tqpainter.h>
#include <tqtimer.h>

#include <tdeconfig.h>
#include <tqtooltip.h>
#include <kcursor.h>
#include <tdelocale.h>
#include <tdestdaccel.h>
#include <tdepopupmenu.h>
#include <kdebug.h>
#include <tdecompletionbox.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kiconloader.h>
#include <tdeapplication.h>

#include "klineedit.h"
#include "klineedit.moc"


class KLineEdit::KLineEditPrivate
{
public:
    KLineEditPrivate()
    {
        completionBox = 0L;
        handleURLDrops = true;
        grabReturnKeyEvents = false;

        userSelection = true;
        autoSuggest = false;
        disableRestoreSelection = false;
        enableSqueezedText = false;

        if ( !initialized )
        {
            TDEConfigGroup config( TDEGlobal::config(), "General" );
            backspacePerformsCompletion = config.readBoolEntry( "Backspace performs completion", false );

            initialized = true;
        }

    }

    ~KLineEditPrivate()
    {
// causes a weird crash in KWord at least, so let Qt delete it for us.
//        delete completionBox;
    }

    static bool initialized;
    static bool backspacePerformsCompletion; // Configuration option

    TQColor previousHighlightColor;
    TQColor previousHighlightedTextColor;

    bool userSelection: 1;
    bool autoSuggest : 1;
    bool disableRestoreSelection: 1;
    bool handleURLDrops:1;
    bool grabReturnKeyEvents:1;
    bool enableSqueezedText:1;

    int squeezedEnd;
    int squeezedStart;
    BackgroundMode bgMode;
    TQString squeezedText;
    TDECompletionBox *completionBox;

    TQString clickMessage;
    bool drawClickMsg:1;
};

bool KLineEdit::KLineEditPrivate::backspacePerformsCompletion = false;
bool KLineEdit::KLineEditPrivate::initialized = false;


KLineEdit::KLineEdit( const TQString &string, TQWidget *parent, const char *name )
          :TQLineEdit( string, parent, name )
{
    init();
}

KLineEdit::KLineEdit( TQWidget *parent, const char *name )
          :TQLineEdit( parent, name )
{
    init();
}

KLineEdit::~KLineEdit ()
{
    delete d;
    d = 0;
}

void KLineEdit::init()
{
    d = new KLineEditPrivate;
    possibleTripleClick = false;
    d->bgMode = backgroundMode ();

    // Enable the context menu by default.
    KLineEdit::setContextMenuEnabled( true );
    KCursor::setAutoHideCursor( this, true, true );
    installEventFilter( this );

    TDEGlobalSettings::Completion mode = completionMode();
    d->autoSuggest = (mode == TDEGlobalSettings::CompletionMan ||
                      mode == TDEGlobalSettings::CompletionPopupAuto ||
                      mode == TDEGlobalSettings::CompletionAuto);
    connect( this, TQ_SIGNAL(selectionChanged()), this, TQ_SLOT(slotRestoreSelectionColors()));

    TQPalette p = palette();
    if ( !d->previousHighlightedTextColor.isValid() )
      d->previousHighlightedTextColor=p.color(TQPalette::Normal,TQColorGroup::HighlightedText);
    if ( !d->previousHighlightColor.isValid() )
      d->previousHighlightColor=p.color(TQPalette::Normal,TQColorGroup::Highlight);

    d->drawClickMsg = false;
}

void KLineEdit::setCompletionMode( TDEGlobalSettings::Completion mode )
{
    TDEGlobalSettings::Completion oldMode = completionMode();

    if ( oldMode != mode && (oldMode == TDEGlobalSettings::CompletionPopup ||
         oldMode == TDEGlobalSettings::CompletionPopupAuto ) &&
         d->completionBox && d->completionBox->isVisible() )
      d->completionBox->hide();

    // If the widgets echo mode is not Normal, no completion
    // feature will be enabled even if one is requested.
    if ( echoMode() != TQLineEdit::Normal )
        mode = TDEGlobalSettings::CompletionNone; // Override the request.

    if ( kapp && !kapp->authorize("lineedit_text_completion") )
        mode = TDEGlobalSettings::CompletionNone;

    if ( mode == TDEGlobalSettings::CompletionPopupAuto ||
         mode == TDEGlobalSettings::CompletionAuto ||
         mode == TDEGlobalSettings::CompletionMan )
        d->autoSuggest = true;
    else
        d->autoSuggest = false;

    TDECompletionBase::setCompletionMode( mode );
}

void KLineEdit::setCompletedText( const TQString& t, bool marked )
{
    if ( !d->autoSuggest )
      return;

    TQString txt = text();

    if ( t != txt )
    {
        int start = marked ? txt.length() : t.length();
        validateAndSet( t, cursorPosition(), start, t.length() );
        setUserSelection(false);
    }
    else
      setUserSelection(true);

}

void KLineEdit::setCompletedText( const TQString& text )
{
    TDEGlobalSettings::Completion mode = completionMode();
    bool marked = ( mode == TDEGlobalSettings::CompletionAuto ||
                    mode == TDEGlobalSettings::CompletionMan ||
                    mode == TDEGlobalSettings::CompletionPopup ||
                    mode == TDEGlobalSettings::CompletionPopupAuto );
    setCompletedText( text, marked );
}

void KLineEdit::rotateText( TDECompletionBase::KeyBindingType type )
{
    TDECompletion* comp = compObj();
    if ( comp &&
       (type == TDECompletionBase::PrevCompletionMatch ||
        type == TDECompletionBase::NextCompletionMatch ) )
    {
       TQString input;

       if (type == TDECompletionBase::PrevCompletionMatch)
          comp->previousMatch();
       else
          comp->nextMatch();

       // Skip rotation if previous/next match is null or the same text
       if ( input.isNull() || input == displayText() )
            return;
       setCompletedText( input, hasSelectedText() );
    }
}

void KLineEdit::makeCompletion( const TQString& text )
{
    TDECompletion *comp = compObj();
    TDEGlobalSettings::Completion mode = completionMode();

    if ( !comp || mode == TDEGlobalSettings::CompletionNone )
        return;  // No completion object...

    TQString match = comp->makeCompletion( text );

    if ( mode == TDEGlobalSettings::CompletionPopup ||
         mode == TDEGlobalSettings::CompletionPopupAuto )
    {
        if ( match.isNull() )
        {
            if ( d->completionBox )
            {
                d->completionBox->hide();
                d->completionBox->clear();
            }
        }
        else
            setCompletedItems( comp->allMatches() );
    }
    else // Auto,  ShortAuto (Man) and Shell
    {
        // all other completion modes
        // If no match or the same match, simply return without completing.
        if ( match.isNull() || match == text )
            return;

        if ( mode != TDEGlobalSettings::CompletionShell )
            setUserSelection(false);

        if ( d->autoSuggest )
            setCompletedText( match );
    }
}

void KLineEdit::setReadOnly(bool readOnly)
{
    // Do not do anything if nothing changed...
    if (readOnly == isReadOnly ())
      return;

    TQLineEdit::setReadOnly (readOnly);

    if (readOnly)
    {
        d->bgMode = backgroundMode ();
        setBackgroundMode (TQt::PaletteBackground);
        if (d->enableSqueezedText && d->squeezedText.isEmpty())
        {
            d->squeezedText = text();
            setSqueezedText();
        }
    }
    else
    {
        if (!d->squeezedText.isEmpty())
        {
           setText(d->squeezedText);
           d->squeezedText = TQString::null;
        }
        setBackgroundMode (d->bgMode);
    }
}

void KLineEdit::setSqueezedText( const TQString &text)
{
    setEnableSqueezedText(true);
    setText(text);
}

void KLineEdit::setEnableSqueezedText( bool enable )
{
    d->enableSqueezedText = enable;
}

bool KLineEdit::isSqueezedTextEnabled() const
{
    return d->enableSqueezedText;
}

void KLineEdit::setText( const TQString& text )
{
    d->drawClickMsg = text.isEmpty() && !d->clickMessage.isEmpty();
    update();

    if( d->enableSqueezedText && isReadOnly() )
    {
        d->squeezedText = text;
        setSqueezedText();
        return;
    }

    TQLineEdit::setText( text );
}

void KLineEdit::setSqueezedText()
{
    d->squeezedStart = 0;
    d->squeezedEnd = 0;
    TQString fullText = d->squeezedText;
    TQFontMetrics fm(fontMetrics());
    int labelWidth = size().width() - 2*frameWidth() - 2;
    int textWidth = fm.width(fullText);

    if (textWidth > labelWidth)
    {
          // start with the dots only
          TQString squeezedText = "...";
          int squeezedWidth = fm.width(squeezedText);

          // estimate how many letters we can add to the dots on both sides
          int letters = fullText.length() * (labelWidth - squeezedWidth) / textWidth / 2;
          squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
          squeezedWidth = fm.width(squeezedText);

      if (squeezedWidth < labelWidth)
      {
             // we estimated too short
             // add letters while text < label
          do
          {
                letters++;
                squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
                squeezedWidth = fm.width(squeezedText);
             } while (squeezedWidth < labelWidth);
             letters--;
             squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
      }
      else if (squeezedWidth > labelWidth)
      {
             // we estimated too long
             // remove letters while text > label
          do
          {
               letters--;
                squeezedText = fullText.left(letters) + "..." + fullText.right(letters);
                squeezedWidth = fm.width(squeezedText);
             } while (squeezedWidth > labelWidth);
          }

      if (letters < 5)
      {
             // too few letters added -> we give up squeezing
          TQLineEdit::setText(fullText);
      }
      else
      {
          TQLineEdit::setText(squeezedText);
             d->squeezedStart = letters;
             d->squeezedEnd = fullText.length() - letters;
          }

          TQToolTip::remove( this );
          TQToolTip::add( this, fullText );

    }
    else
    {
      TQLineEdit::setText(fullText);

          TQToolTip::remove( this );
          TQToolTip::hide();
       }

       setCursorPosition(0);
}

void KLineEdit::copy() const
{
    if( !copySqueezedText(true))
        TQLineEdit::copy();
}

bool KLineEdit::copySqueezedText(bool clipboard) const
{
   if (!d->squeezedText.isEmpty() && d->squeezedStart)
   {
      int start, end;
      KLineEdit *that = const_cast<KLineEdit *>(this);
      if (!that->getSelection(&start, &end))
         return false;
      if (start >= d->squeezedStart+3)
         start = start - 3 - d->squeezedStart + d->squeezedEnd;
      else if (start > d->squeezedStart)
         start = d->squeezedStart;
      if (end >= d->squeezedStart+3)
         end = end - 3 - d->squeezedStart + d->squeezedEnd;
      else if (end > d->squeezedStart)
         end = d->squeezedEnd;
      if (start == end)
         return false;
      TQString t = d->squeezedText;
      t = t.mid(start, end - start);
      disconnect( TQApplication::clipboard(), TQ_SIGNAL(selectionChanged()), this, 0);
      TQApplication::clipboard()->setText( t, clipboard ? TQClipboard::Clipboard : TQClipboard::Selection );
      connect( TQApplication::clipboard(), TQ_SIGNAL(selectionChanged()), this,
               TQ_SLOT(clipboardChanged()) );
      return true;
   }
   return false;
}

void KLineEdit::resizeEvent( TQResizeEvent * ev )
{
    if (!d->squeezedText.isEmpty())
        setSqueezedText();

    TQLineEdit::resizeEvent(ev);
}

void KLineEdit::keyPressEvent( TQKeyEvent *e )
{
    KKey key( e );

    if ( TDEStdAccel::copy().contains( key ) )
    {
        copy();
        return;
    }
    else if ( TDEStdAccel::paste().contains( key ) )
    {
        paste();
        return;
    }
    else if ( TDEStdAccel::pasteSelection().contains( key ) )
    {
        TQString text = TQApplication::clipboard()->text( TQClipboard::Selection);
        insert( text );
        deselect();
        return;
    }

    else if ( TDEStdAccel::cut().contains( key ) )
    {
        cut();
        return;
    }
    else if ( TDEStdAccel::undo().contains( key ) )
    {
        undo();
        return;
    }
    else if ( TDEStdAccel::redo().contains( key ) )
    {
        redo();
        return;
    }
    else if ( TDEStdAccel::deleteWordBack().contains( key ) )
    {
        cursorWordBackward(true);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( TDEStdAccel::deleteWordForward().contains( key ) )
    {
        // Workaround for QT bug where
        cursorWordForward(true);
        if ( hasSelectedText() )
            del();

        e->accept();
        return;
    }
    else if ( TDEStdAccel::backwardWord().contains( key ) )
    {
      cursorWordBackward(false);
      e->accept();
      return;
    }
    else if ( TDEStdAccel::forwardWord().contains( key ) )
    {
      cursorWordForward(false);
      e->accept();
      return;
    }
    else if ( TDEStdAccel::beginningOfLine().contains( key ) )
    {
      home(false);
      e->accept();
      return;
    }
    else if ( TDEStdAccel::endOfLine().contains( key ) )
    {
      end(false);
      e->accept();
      return;
    }


    // Filter key-events if EchoMode is normal and
    // completion mode is not set to CompletionNone
    if ( echoMode() == TQLineEdit::Normal &&
         completionMode() != TDEGlobalSettings::CompletionNone )
    {
        KeyBindingMap keys = getKeyBindings();
        TDEGlobalSettings::Completion mode = completionMode();
        bool noModifier = (e->state() == TQt::NoButton ||
                           e->state() == TQt::ShiftButton ||
                           e->state() == TQt::Keypad);

        if ( (mode == TDEGlobalSettings::CompletionAuto ||
              mode == TDEGlobalSettings::CompletionPopupAuto ||
              mode == TDEGlobalSettings::CompletionMan) && noModifier )
        {
            if ( !d->userSelection && hasSelectedText() &&
                 ( e->key() == Key_Right || e->key() == Key_Left ) &&
                 e->state()== TQt::NoButton )
            {
                TQString old_txt = text();
                d->disableRestoreSelection = true;
                int start,end;
                getSelection(&start, &end);

                deselect();
                TQLineEdit::keyPressEvent ( e );
                int cPosition=cursorPosition();
                if (e->key() ==Key_Right && cPosition > start )
                    validateAndSet(old_txt, cPosition, cPosition, old_txt.length());
                else
                    validateAndSet(old_txt, cPosition, start, old_txt.length());

                d->disableRestoreSelection = false;
                return;
            }

            if ( e->key() == Key_Escape )
            {
                if (hasSelectedText() && !d->userSelection )
                {
                    del();
                    setUserSelection(true);
                }

                // Don't swallow the Escape press event for the case
                // of dialogs, which have Escape associated to Cancel
                e->ignore();
                return;
            }

        }

        if ( (mode == TDEGlobalSettings::CompletionAuto ||
              mode == TDEGlobalSettings::CompletionMan) && noModifier )
        {
            TQString keycode = e->text();
            if ( !keycode.isEmpty() && (keycode.unicode()->isPrint() ||
                e->key() == Key_Backspace || e->key() == Key_Delete ) )
            {
                bool hasUserSelection=d->userSelection;
                bool hadSelection=hasSelectedText();

                bool cursorNotAtEnd=false;

                int start,end;
                getSelection(&start, &end);
                int cPos = cursorPosition();

                // When moving the cursor, we want to keep the autocompletion as an
                // autocompletion, so we want to process events at the cursor position
                // as if there was no selection. After processing the key event, we
                // can set the new autocompletion again.
                if ( hadSelection && !hasUserSelection && start>cPos )
                {
                    del();
                    setCursorPosition(cPos);
                    cursorNotAtEnd=true;
                }

                d->disableRestoreSelection = true;
                TQLineEdit::keyPressEvent ( e );
                d->disableRestoreSelection = false;

                TQString txt = text();
                int len = txt.length();
                if ( !hasSelectedText() && len /*&& cursorPosition() == len */)
                {
                    if ( e->key() == Key_Backspace )
                    {
                        if ( hadSelection && !hasUserSelection && !cursorNotAtEnd )
                        {
                            backspace();
                            txt = text();
                            len = txt.length();
                        }

                        if ( !d->backspacePerformsCompletion || !len )
                            d->autoSuggest = false;
                    }

                    if (e->key() == Key_Delete )
                        d->autoSuggest=false;

                    if ( emitSignals() )
                        emit completion( txt );

                    if ( handleSignals() )
                        makeCompletion( txt );

                    if(  (e->key() == Key_Backspace || e->key() == Key_Delete) )
                        d->autoSuggest=true;

                    e->accept();
                }

                return;
            }

        }

        else if (( mode == TDEGlobalSettings::CompletionPopup ||
                   mode == TDEGlobalSettings::CompletionPopupAuto ) &&
                   noModifier && !e->text().isEmpty() )
        {
            TQString old_txt = text();
            bool hasUserSelection=d->userSelection;
            bool hadSelection=hasSelectedText();
            bool cursorNotAtEnd=false;

            int start,end;
            getSelection(&start, &end);
            int cPos = cursorPosition();
            TQString keycode = e->text();

            // When moving the cursor, we want to keep the autocompletion as an
            // autocompletion, so we want to process events at the cursor position
            // as if there was no selection. After processing the key event, we
            // can set the new autocompletion again.
            if (hadSelection && !hasUserSelection && start>cPos &&
               ( (!keycode.isEmpty() && keycode.unicode()->isPrint()) ||
                 e->key() == Key_Backspace || e->key() == Key_Delete ) )
            {
                del();
                setCursorPosition(cPos);
                cursorNotAtEnd=true;
            }

            uint selectedLength=selectedText().length();

            d->disableRestoreSelection = true;
            TQLineEdit::keyPressEvent ( e );
            d->disableRestoreSelection = false;

            if (( selectedLength != selectedText().length() ) && !hasUserSelection )
                slotRestoreSelectionColors(); // and set userSelection to true

            TQString txt = text();
            int len = txt.length();

            if ( txt != old_txt && len/* && ( cursorPosition() == len || force )*/ &&
                 ( (!keycode.isEmpty() && keycode.unicode()->isPrint()) ||
                   e->key() == Key_Backspace || e->key() == Key_Delete) )
            {
                if ( e->key() == Key_Backspace )
                {
                    if ( hadSelection && !hasUserSelection && !cursorNotAtEnd )
                    {
                        backspace();
                        txt = text();
                        len = txt.length();
                    }

                    if ( !d->backspacePerformsCompletion )
                        d->autoSuggest = false;
                }

                if (e->key() == Key_Delete )
                    d->autoSuggest=false;

                if ( d->completionBox )
                  d->completionBox->setCancelledText( txt );
	
                if ( emitSignals() )
                  emit completion( txt ); // emit when requested...

                if ( handleSignals() ) {
                  makeCompletion( txt );  // handle when requested...
                }

                if ( (e->key() == Key_Backspace || e->key() == Key_Delete ) &&
                    mode == TDEGlobalSettings::CompletionPopupAuto )
                  d->autoSuggest=true;

                e->accept();
            }
            else if (!len && d->completionBox && d->completionBox->isVisible())
                d->completionBox->hide();

            return;
        }

        else if ( mode == TDEGlobalSettings::CompletionShell )
        {
            // Handles completion.
            TDEShortcut cut;
            if ( keys[TextCompletion].isNull() )
                cut = TDEStdAccel::shortcut(TDEStdAccel::TextCompletion);
            else
                cut = keys[TextCompletion];

            if ( cut.contains( key ) )
            {
                // Emit completion if the completion mode is CompletionShell
                // and the cursor is at the end of the string.
                TQString txt = text();
                int len = txt.length();
                if ( cursorPosition() == len && len != 0 )
                {
                    if ( emitSignals() )
                        emit completion( txt );
                    if ( handleSignals() )
                        makeCompletion( txt );
                    return;
                }
            }
            else if ( d->completionBox )
                d->completionBox->hide();
        }

        // handle rotation
        if ( mode != TDEGlobalSettings::CompletionNone )
        {
            // Handles previous match
            TDEShortcut cut;
            if ( keys[PrevCompletionMatch].isNull() )
                cut = TDEStdAccel::shortcut(TDEStdAccel::PrevCompletion);
            else
                cut = keys[PrevCompletionMatch];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit textRotation( TDECompletionBase::PrevCompletionMatch );
                if ( handleSignals() )
                    rotateText( TDECompletionBase::PrevCompletionMatch );
                return;
            }

            // Handles next match
            if ( keys[NextCompletionMatch].isNull() )
                cut = TDEStdAccel::shortcut(TDEStdAccel::NextCompletion);
            else
                cut = keys[NextCompletionMatch];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit textRotation( TDECompletionBase::NextCompletionMatch );
                if ( handleSignals() )
                    rotateText( TDECompletionBase::NextCompletionMatch );
                return;
            }
        }

        // substring completion
        if ( compObj() )
        {
            TDEShortcut cut;
            if ( keys[SubstringCompletion].isNull() )
                cut = TDEStdAccel::shortcut(TDEStdAccel::SubstringCompletion);
            else
                cut = keys[SubstringCompletion];

            if ( cut.contains( key ) )
            {
                if ( emitSignals() )
                    emit substringCompletion( text() );
                if ( handleSignals() )
                {
                    setCompletedItems( compObj()->substringCompletion(text()));
                    e->accept();
                }
                return;
            }
        }
    }

    uint selectedLength = selectedText().length();

    // Let TQLineEdit handle any other keys events.
    TQLineEdit::keyPressEvent ( e );

    if ( selectedLength != selectedText().length() )
        slotRestoreSelectionColors(); // and set userSelection to true
}

void KLineEdit::mouseDoubleClickEvent( TQMouseEvent* e )
{
    if ( e->button() == TQt::LeftButton  )
    {
        possibleTripleClick=true;
        TQTimer::singleShot( TQApplication::doubleClickInterval(),this,
                            TQ_SLOT(tripleClickTimeout()) );
    }
    TQLineEdit::mouseDoubleClickEvent( e );
}

void KLineEdit::mousePressEvent( TQMouseEvent* e )
{
    if ( possibleTripleClick && e->button() == TQt::LeftButton )
    {
        selectAll();
        e->accept();
        return;
    }
    TQLineEdit::mousePressEvent( e );
}

void KLineEdit::mouseReleaseEvent( TQMouseEvent* e )
{
    TQLineEdit::mouseReleaseEvent( e );
    if (TQApplication::clipboard()->supportsSelection() ) {
        if ( e->button() == TQt::LeftButton ) {
            // Fix copying of squeezed text if needed
            copySqueezedText( false );
        }
    }
}

void KLineEdit::tripleClickTimeout()
{
    possibleTripleClick=false;
}

void KLineEdit::contextMenuEvent( TQContextMenuEvent * e )
{
    if ( m_bEnableMenu )
        TQLineEdit::contextMenuEvent( e );
}

TQPopupMenu *KLineEdit::createPopupMenu()
{
    enum { IdUndo, IdRedo, IdSep1, IdCut, IdCopy, IdPaste, IdClear, IdSep2, IdSelectAll };

    TQPopupMenu *popup = TQLineEdit::createPopupMenu();

      int id = popup->idAt(0);
      popup->changeItem( id - IdUndo, SmallIconSet("edit-undo"), popup->text( id - IdUndo) );
      popup->changeItem( id - IdRedo, SmallIconSet("edit-redo"), popup->text( id - IdRedo) );
      popup->changeItem( id - IdCut, SmallIconSet("edit-cut"), popup->text( id - IdCut) );
      popup->changeItem( id - IdCopy, SmallIconSet("edit-copy"), popup->text( id - IdCopy) );
      popup->changeItem( id - IdPaste, SmallIconSet("edit-paste"), popup->text( id - IdPaste) );
      popup->changeItem( id - IdClear, SmallIconSet("edit-clear"), popup->text( id - IdClear) );

    // If a completion object is present and the input
    // widget is not read-only, show the Text Completion
    // menu item.
    if ( compObj() && !isReadOnly() && kapp->authorize("lineedit_text_completion") )
    {
        TQPopupMenu *subMenu = new TQPopupMenu( popup );
        connect( subMenu, TQ_SIGNAL( activated( int ) ),
                 this, TQ_SLOT( completionMenuActivated( int ) ) );

        popup->insertSeparator();
        popup->insertItem( SmallIconSet("completion"), i18n("Text Completion"),
                           subMenu );

        subMenu->insertItem( i18n("None"), NoCompletion );
        subMenu->insertItem( i18n("Manual"), ShellCompletion );
        subMenu->insertItem( i18n("Automatic"), AutoCompletion );
        subMenu->insertItem( i18n("Dropdown List"), PopupCompletion );
        subMenu->insertItem( i18n("Short Automatic"), ShortAutoCompletion );
        subMenu->insertItem( i18n("Dropdown List && Automatic"), PopupAutoCompletion );

        subMenu->setAccel( TDEStdAccel::completion(), ShellCompletion );

        TDEGlobalSettings::Completion mode = completionMode();
        subMenu->setItemChecked( NoCompletion,
                                 mode == TDEGlobalSettings::CompletionNone );
        subMenu->setItemChecked( ShellCompletion,
                                 mode == TDEGlobalSettings::CompletionShell );
        subMenu->setItemChecked( PopupCompletion,
                                 mode == TDEGlobalSettings::CompletionPopup );
        subMenu->setItemChecked( AutoCompletion,
                                 mode == TDEGlobalSettings::CompletionAuto );
        subMenu->setItemChecked( ShortAutoCompletion,
                                 mode == TDEGlobalSettings::CompletionMan );
        subMenu->setItemChecked( PopupAutoCompletion,
                                 mode == TDEGlobalSettings::CompletionPopupAuto );
        if ( mode != TDEGlobalSettings::completionMode() )
        {
            subMenu->insertSeparator();
            subMenu->insertItem( i18n("Default"), Default );
        }
    }

    // ### do we really need this?  Yes, Please do not remove!  This
    // allows applications to extend the popup menu without having to
    // inherit from this class! (DA)
    emit aboutToShowContextMenu( popup );

    return popup;
}

void KLineEdit::completionMenuActivated( int id )
{
    TDEGlobalSettings::Completion oldMode = completionMode();

    switch ( id )
    {
        case Default:
           setCompletionMode( TDEGlobalSettings::completionMode() );
           break;
        case NoCompletion:
           setCompletionMode( TDEGlobalSettings::CompletionNone );
           break;
        case AutoCompletion:
            setCompletionMode( TDEGlobalSettings::CompletionAuto );
            break;
        case ShortAutoCompletion:
            setCompletionMode( TDEGlobalSettings::CompletionMan );
            break;
        case ShellCompletion:
            setCompletionMode( TDEGlobalSettings::CompletionShell );
            break;
        case PopupCompletion:
            setCompletionMode( TDEGlobalSettings::CompletionPopup );
            break;
        case PopupAutoCompletion:
            setCompletionMode( TDEGlobalSettings::CompletionPopupAuto );
            break;
        default:
            return;
    }

    if ( oldMode != completionMode() )
    {
        if ( (oldMode == TDEGlobalSettings::CompletionPopup ||
              oldMode == TDEGlobalSettings::CompletionPopupAuto ) &&
              d->completionBox && d->completionBox->isVisible() )
            d->completionBox->hide();
        emit completionModeChanged( completionMode() );
    }
}

void KLineEdit::drawContents( TQPainter *p )
{
    TQLineEdit::drawContents( p );

    if ( d->drawClickMsg && !hasFocus() ) {
        TQPen tmp = p->pen();
        p->setPen( palette().color( TQPalette::Disabled, TQColorGroup::Text ) );
        TQRect cr = contentsRect();

        // Add two pixel margin on the left side
        cr.rLeft() += 3;
        p->drawText( cr, AlignAuto | AlignVCenter, d->clickMessage );
        p->setPen( tmp );
    }
}

void KLineEdit::dropEvent(TQDropEvent *e)
{
    d->drawClickMsg = false;
    KURL::List urlList;
    if( d->handleURLDrops && KURLDrag::decode( e, urlList ) )
    {
        TQString dropText = text();
        KURL::List::ConstIterator it;
        for( it = urlList.begin() ; it != urlList.end() ; ++it )
        {
            if(!dropText.isEmpty())
                dropText+=' ';

            dropText += (*it).prettyURL();
        }

        validateAndSet( dropText, dropText.length(), 0, 0);

        e->accept();
    }
    else
        TQLineEdit::dropEvent(e);
}

bool KLineEdit::eventFilter( TQObject* o, TQEvent* ev )
{
    if( o == this )
    {
        KCursor::autoHideEventFilter( this, ev );
        if ( ev->type() == TQEvent::AccelOverride )
        {
            TQKeyEvent *e = static_cast<TQKeyEvent*>( ev );
            if (overrideAccel (e))
            {
                e->accept();
                return true;
            }
        }
        else if( ev->type() == TQEvent::KeyPress )
        {
            TQKeyEvent *e = static_cast<TQKeyEvent*>( ev );

            if( e->key() == TQt::Key_Return || e->key() == TQt::Key_Enter )
            {
                bool trap = d->completionBox && d->completionBox->isVisible();

                bool stopEvent = trap || (d->grabReturnKeyEvents &&
                                          (e->state() == TQt::NoButton ||
                                           e->state() == TQt::Keypad));

                // Qt will emit returnPressed() itself if we return false
                if ( stopEvent )
                {
                  emit TQLineEdit::returnPressed();
                  e->accept ();
                }

                emit returnPressed( displayText() );

                if ( trap )
                {
                    d->completionBox->hide();
                    deselect();
                    setCursorPosition(text().length());
                }

                // Eat the event if the user asked for it, or if a completionbox was visible
                return stopEvent;
            }
        }
    }
    return TQLineEdit::eventFilter( o, ev );
}


void KLineEdit::setURLDropsEnabled(bool enable)
{
    d->handleURLDrops=enable;
}

bool KLineEdit::isURLDropsEnabled() const
{
    return d->handleURLDrops;
}

void KLineEdit::setTrapReturnKey( bool grab )
{
    d->grabReturnKeyEvents = grab;
}

bool KLineEdit::trapReturnKey() const
{
    return d->grabReturnKeyEvents;
}

void KLineEdit::setURL( const KURL& url )
{
    setText( url.prettyURL() );
}

void KLineEdit::setCompletionBox( TDECompletionBox *box )
{
    if ( d->completionBox )
        return;

    d->completionBox = box;
    if ( handleSignals() )
    {
        connect( d->completionBox, TQ_SIGNAL(highlighted( const TQString& )),
                 TQ_SLOT(setTextWorkaround( const TQString& )) );
        connect( d->completionBox, TQ_SIGNAL(userCancelled( const TQString& )),
                 TQ_SLOT(userCancelled( const TQString& )) );
        connect( d->completionBox, TQ_SIGNAL( activated( const TQString& )),
                 TQ_SIGNAL(completionBoxActivated( const TQString& )) );
    }
}

void KLineEdit::userCancelled(const TQString & cancelText)
{
    if ( completionMode() != TDEGlobalSettings::CompletionPopupAuto )
    {
      // TODO: this sets modified==false. But maybe it was true before...
      setText(cancelText);
    }
    else if (hasSelectedText() )
    {
      if (d->userSelection)
        deselect();
      else
      {
        d->autoSuggest=false;
        int start,end;
        getSelection(&start, &end);
        TQString s=text().remove(start, end-start+1);
        validateAndSet(s,start,s.length(),s.length());
        d->autoSuggest=true;
      }
    }
}

bool KLineEdit::overrideAccel (const TQKeyEvent* e)
{
    TDEShortcut scKey;

    KKey key( e );
    KeyBindingMap keys = getKeyBindings();

    if (keys[TextCompletion].isNull())
        scKey = TDEStdAccel::shortcut(TDEStdAccel::TextCompletion);
    else
        scKey = keys[TextCompletion];

    if (scKey.contains( key ))
        return true;

    if (keys[NextCompletionMatch].isNull())
        scKey = TDEStdAccel::shortcut(TDEStdAccel::NextCompletion);
    else
        scKey = keys[NextCompletionMatch];

    if (scKey.contains( key ))
        return true;

    if (keys[PrevCompletionMatch].isNull())
        scKey = TDEStdAccel::shortcut(TDEStdAccel::PrevCompletion);
    else
        scKey = keys[PrevCompletionMatch];

    if (scKey.contains( key ))
        return true;

    // Override all the text manupilation accelerators...
    if ( TDEStdAccel::copy().contains( key ) )
        return true;
    else if ( TDEStdAccel::paste().contains( key ) )
        return true;
    else if ( TDEStdAccel::cut().contains( key ) )
        return true;
    else if ( TDEStdAccel::undo().contains( key ) )
        return true;
    else if ( TDEStdAccel::redo().contains( key ) )
        return true;
    else if (TDEStdAccel::deleteWordBack().contains( key ))
        return true;
    else if (TDEStdAccel::deleteWordForward().contains( key ))
        return true;
    else if (TDEStdAccel::forwardWord().contains( key ))
        return true;
    else if (TDEStdAccel::backwardWord().contains( key ))
        return true;
    else if (TDEStdAccel::beginningOfLine().contains( key ))
        return true;
    else if (TDEStdAccel::endOfLine().contains( key ))
        return true;

    if (d->completionBox && d->completionBox->isVisible ())
    {
        int key = e->key();
        ButtonState state = e->state();
        if ((key == Key_Backtab || key == Key_Tab) &&
            (state == TQt::NoButton || (state & TQt::ShiftButton)))
        {
            return true;
        }
    }


    return false;
}

void KLineEdit::setCompletedItems( const TQStringList& items )
{
    setCompletedItems( items, true );
}

void KLineEdit::setCompletedItems( const TQStringList& items, bool autoSuggest )
{
    TQString txt;
    if ( d->completionBox && d->completionBox->isVisible() ) {
        // The popup is visible already - do the matching on the initial string,
        // not on the currently selected one.
        txt = completionBox()->cancelledText();
    } else {
        txt = text();
    }

    if ( !items.isEmpty() &&
         !(items.count() == 1 && txt == items.first()) )
    {
        // create completion box if non-existent
        completionBox();

        if ( d->completionBox->isVisible() )
        {
            bool wasSelected = d->completionBox->isSelected( d->completionBox->currentItem() );
            const TQString currentSelection = d->completionBox->currentText();
            d->completionBox->setItems( items );
            TQListBoxItem* item = d->completionBox->findItem( currentSelection, TQt::ExactMatch );
            // If no item is selected, that means the listbox hasn't been manipulated by the user yet,
            // because it's not possible otherwise to have no selected item. In such case make
            // always the first item current and unselected, so that the current item doesn't jump.
            if( !item || !wasSelected )
            {
                wasSelected = false;
                item = d->completionBox->item( 0 );
            }
            if ( item )
            {
                d->completionBox->blockSignals( true );
                d->completionBox->setCurrentItem( item );
                d->completionBox->setSelected( item, wasSelected );
                d->completionBox->blockSignals( false );
            }
        }
        else // completion box not visible yet -> show it
        {
            if ( !txt.isEmpty() )
                d->completionBox->setCancelledText( txt );
            d->completionBox->setItems( items );
            d->completionBox->popup();
        }

        if ( d->autoSuggest && autoSuggest )
        {
            int index = items.first().find( txt );
            TQString newText = items.first().mid( index );
            setUserSelection(false);
            setCompletedText(newText,true);
        }
    }
    else
    {
        if ( d->completionBox && d->completionBox->isVisible() )
            d->completionBox->hide();
    }
}

TDECompletionBox * KLineEdit::completionBox( bool create )
{
    if ( create && !d->completionBox ) {
        setCompletionBox( new TDECompletionBox( this, "completion box" ) );
        d->completionBox->setFont(font());
    }

    return d->completionBox;
}

void KLineEdit::setCompletionObject( TDECompletion* comp, bool hsig )
{
    TDECompletion *oldComp = compObj();
    if ( oldComp && handleSignals() )
        disconnect( oldComp, TQ_SIGNAL( matches( const TQStringList& )),
                    this, TQ_SLOT( setCompletedItems( const TQStringList& )));

    if ( comp && hsig )
      connect( comp, TQ_SIGNAL( matches( const TQStringList& )),
               this, TQ_SLOT( setCompletedItems( const TQStringList& )));

    TDECompletionBase::setCompletionObject( comp, hsig );
}

// TQWidget::create() turns off mouse-Tracking which would break auto-hiding
void KLineEdit::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
    TQLineEdit::create( id, initializeWindow, destroyOldWindow );
    KCursor::setAutoHideCursor( this, true, true );
}

void KLineEdit::setUserSelection(bool userSelection)
{
    TQPalette p = palette();

    if (userSelection)
    {
        p.setColor(TQColorGroup::Highlight, d->previousHighlightColor);
        p.setColor(TQColorGroup::HighlightedText, d->previousHighlightedTextColor);
    }
    else
    {
        TQColor color=p.color(TQPalette::Disabled, TQColorGroup::Text);
        p.setColor(TQColorGroup::HighlightedText, color);
        color=p.color(TQPalette::Active, TQColorGroup::Base);
        p.setColor(TQColorGroup::Highlight, color);
    }

    d->userSelection=userSelection;
    setPalette(p);
}

void KLineEdit::slotRestoreSelectionColors()
{
    if (d->disableRestoreSelection)
      return;

    setUserSelection(true);
}

void KLineEdit::clear()
{
    setText( TQString::null );
}

void KLineEdit::setTextWorkaround( const TQString& text )
{
    setText( text );
    end( false ); // force cursor at end
}

TQString KLineEdit::originalText() const
{
    if ( d->enableSqueezedText && isReadOnly() )
        return d->squeezedText;

    return text();
}

void KLineEdit::focusInEvent( TQFocusEvent* ev)
{
    if ( d->drawClickMsg ) {
        d->drawClickMsg = false;
        update();
    }

    // Don't selectAll() in TQLineEdit::focusInEvent if selection exists
    if ( ev->reason() == TQFocusEvent::Tab && inputMask().isNull() && hasSelectedText() )
        return;
    
    TQLineEdit::focusInEvent(ev);
}

void KLineEdit::focusOutEvent( TQFocusEvent* ev)
{
    if ( text().isEmpty() && !d->clickMessage.isEmpty() ) {
        d->drawClickMsg = true;
        update();
    }
    TQLineEdit::focusOutEvent( ev );
}

bool KLineEdit::autoSuggest() const
{
    return d->autoSuggest;
}

void KLineEdit::setClickMessage( const TQString &msg )
{
    d->clickMessage = msg;
    update();
}

TQString KLineEdit::clickMessage() const
{
    return d->clickMessage;
}


void KLineEdit::virtual_hook( int id, void* data )
{ TDECompletionBase::virtual_hook( id, data ); }
