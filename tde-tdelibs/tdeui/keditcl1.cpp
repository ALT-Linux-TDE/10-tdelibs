/* This file is part of the KDE libraries

   Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@math.cornell.edu>
   Copyright (C) 2000 Waldo Bastian <bastian@kde.org>

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

#include <tqdragobject.h>
#include <tqpopupmenu.h>
#include <tqtextstream.h>
#include <tqtimer.h>

#include <tdeapplication.h>
#include <kcursor.h>
#include <kdebug.h>
#include <kcmenumngr.h>
#include <tdefontdialog.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tdestdaccel.h>
#include <kurldrag.h>

#include "keditcl.h"
#include "keditcl.moc"

class KEdit::KEditPrivate
{
public:
    bool overwriteEnabled:1;
    bool posDirty:1;
    bool autoUpdate:1;
};


KEdit::KEdit(TQWidget *_parent, const char *name)
   : TQMultiLineEdit(_parent, name)
{
    d = new KEditPrivate;
    d->overwriteEnabled = false;
    d->posDirty = true;
    d->autoUpdate = true;

    parent = _parent;

    // set some defaults

    line_pos = col_pos = 0;

    srchdialog = NULL;
    replace_dialog= NULL;
    gotodialog = NULL;

    setAcceptDrops(true);
    KCursor::setAutoHideCursor( this, true );

    connect(this, TQ_SIGNAL(cursorPositionChanged(int,int)),
            this, TQ_SLOT(slotCursorPositionChanged()));
}


KEdit::~KEdit()
{
  delete d;
}

void
KEdit::setAutoUpdate(bool b)
{
  d->autoUpdate = b;
}

void
KEdit::insertText(TQTextStream *stream)
{
//   setAutoUpdate(false);
   int line, col;
   getCursorPosition(&line, &col);
   int saveline = line;
   int savecol = col;
   TQString textLine;

   // MS: Patch by Martin Schenk <martin@schenk.com>
   // MS: disable UNDO, or TQMultiLineEdit remembers every textLine !!!
   // memory usage is:
   //   textLine: 2*size rounded up to nearest power of 2 (520Kb -> 1024Kb)
   //   widget:   about (2*size + 60bytes*lines)
   // -> without disabling undo, it often needs almost 8*size
   int oldUndoDepth = undoDepth();
   setUndoDepth( 0 ); // ### -1?

   // MS: read everything at once if file <= 1MB,
   // else read in 5000-line chunks to keep memory usage acceptable.
   TQIODevice *dev=stream->device();
   if (dev && dev->size()>(1024*1024)) {
      while(1) {
        int i;
        textLine="";
        for (i=0; i<5000; i++) {
                TQString line=stream->readLine();
                if (line.isNull()) break;  // EOF
                textLine+=line+'\n';
        }
        insertAt(textLine, line, col);
        line+=i; col=0;
        if (i!=5000) break;
      }
   }
   else {
        textLine = stream->read(); // Read all !
        insertAt( textLine, line, col);
   }
   setUndoDepth( oldUndoDepth );

   setCursorPosition(saveline, savecol);
//   setAutoUpdate(true);

//   repaint();

   setModified(true);
   setFocus();

   // Bernd: Please don't leave debug message like that lying around
   // they cause ENORMOUSE performance hits. Once upon a day
   // kedit used to be really really fast using memmap etc .....
   // oh well ....

   //   TQString str = text();
   //   for (int i = 0; i < (int) str.length(); i++)
   //     printf("KEdit: U+%04X\n", str[i].unicode());

}

void
KEdit::cleanWhiteSpace()
{
   d->autoUpdate = false;
   if (!hasMarkedText())
      selectAll();
   TQString oldText = markedText();
   TQString newText;
   TQStringList lines = TQStringList::split('\n', oldText, true);
   bool addSpace = false;
   bool firstLine = true;
   TQChar lastChar = oldText[oldText.length()-1];
   TQChar firstChar = oldText[0];
   for(TQStringList::Iterator it = lines.begin();
       it != lines.end();)
   {
      TQString line = (*it).simplifyWhiteSpace();
      if (line.isEmpty())
      {
         if (addSpace)
            newText += TQString::fromLatin1("\n\n");
         if (firstLine)
         {
            if (firstChar.isSpace())
               newText += '\n';
            firstLine = false;
         }
         addSpace = false;
      }
      else
      {
         if (addSpace)
            newText += ' ';
         if (firstLine)
         {
            if (firstChar.isSpace())
               newText += ' ';
            firstLine = false;
         }
         newText += line;
         addSpace = true;
      }
      it = lines.remove(it);
   }
   if (addSpace)
   {
      if (lastChar == '\n')
         newText += '\n';
      else if (lastChar.isSpace())
         newText += ' ';
   }

   if (oldText == newText)
   {
      deselect();
      d->autoUpdate = true;
      repaint();
      return;
   }
   if (wordWrap() == NoWrap)
   {
      // If wordwrap is off, we have to do some line-wrapping ourselves now
      // We use another TQMultiLineEdit for this, so that we get nice undo
      // behavior.
      TQMultiLineEdit *we = new TQMultiLineEdit();
      we->setWordWrap(FixedColumnWidth);
      we->setWrapColumnOrWidth(78);
      we->setText(newText);
      newText = TQString::null;
      for(int i = 0; i < we->numLines(); i++)
      {
        TQString line = we->textLine(i);
        if (line.right(1) != "\n")
           line += '\n';
        newText += line;
      }
      delete we;
   }

   insert(newText);
   d->autoUpdate = true;
   repaint();

   setModified(true);
   setFocus();
}


void
KEdit::saveText(TQTextStream *stream)
{
   saveText(stream, false);
}

void
KEdit::saveText(TQTextStream *stream, bool softWrap)
{
   int line_count = numLines()-1;
   if (line_count < 0)
      return;

   if (softWrap || (wordWrap() == NoWrap))
   {
      for(int i = 0; i < line_count; i++)
      {
         (*stream) << textLine(i) << '\n';
      }
      (*stream) << textLine(line_count);
   }
   else
   {
      for(int i = 0; i <= line_count; i++)
      {
         int lines_in_parag = linesOfParagraph(i);
         if (lines_in_parag == 1)
         {
            (*stream) << textLine(i);
         }
         else
         {
            TQString parag_text = textLine(i);
            int pos = 0;
            int first_pos = 0;
            int current_line = 0;
            while(true) {
               while(lineOfChar(i, pos) == current_line) pos++;
               (*stream) << parag_text.mid(first_pos, pos - first_pos - 1) << '\n';
               current_line++;
               first_pos = pos;
               if (current_line+1 == lines_in_parag)
               {
                  // Last line
                  (*stream) << parag_text.mid(pos);
                  break;
               }
            }
         }
         if (i < line_count)
            (*stream) << '\n';
      }
   }
}

int KEdit::currentLine(){

  computePosition();
  return line_pos;

}

int KEdit::currentColumn(){

  computePosition();
  return col_pos;
}

void KEdit::slotCursorPositionChanged()
{
  d->posDirty = true;
  emit CursorPositionChanged();
}

void KEdit::computePosition()
{
  if (!d->posDirty) return;
  d->posDirty = false;

  int line, col;

  getCursorPosition(&line,&col);

  // line is expressed in paragraphs, we now need to convert to lines
  line_pos = 0;
  if (wordWrap() == NoWrap)
  {
     line_pos = line;
  }
  else
  {
     for(int i = 0; i < line; i++)
        line_pos += linesOfParagraph(i);
  }

  int line_offset = lineOfChar(line, col);
  line_pos += line_offset;

  // We now calculate where the current line starts in the paragraph.
  TQString linetext = textLine(line);
  int start_of_line = 0;
  if (line_offset > 0)
  {
     start_of_line = col;
     while(lineOfChar(line, --start_of_line) == line_offset);
     start_of_line++;
  }


  // O.K here is the deal: The function getCursorPositoin returns the character
  // position of the cursor, not the screenposition. I.e,. assume the line
  // consists of ab\tc then the character c will be on the screen on position 8
  // whereas getCursorPosition will return 3 if the cursors is on the character c.
  // Therefore we need to compute the screen position from the character position.
  // That's what all the following trouble is all about:

  int coltemp = col-start_of_line;
  int pos  = 	0;
  int find = 	0;
  int mem  = 	0;
  bool found_one = false;

  // if you understand the following algorithm you are worthy to look at the
  // kedit+ sources -- if not, go away ;-)


  while(find >=0 && find <= coltemp- 1 ){
    find = linetext.find('\t', find+start_of_line, true )-start_of_line;
    if( find >=0 && find <= coltemp - 1 ){
      found_one = true;
      pos = pos + find - mem;
      pos = pos + 8  - pos % 8;
      mem = find;
      find ++;
    }
  }

  pos = pos + coltemp - mem;  // add the number of characters behind the
                              // last tab on the line.

  if (found_one){
    pos = pos - 1;
  }

  col_pos = pos;
}


void KEdit::keyPressEvent ( TQKeyEvent *e)
{
  // ignore Ctrl-Return so that KDialogBase can catch them
  if ( e->key() == Key_Return && e->state() == ControlButton ) {
      e->ignore();
      return;
  }

  KKey key(e);
  int keyQt = key.keyCodeQt();

  if ( keyQt == CTRL+Key_K ){

    int line = 0;
    int col  = 0;
    TQString killstring;

    if(!killing){
      killbufferstring = "";
      killtrue = false;
      lastwasanewline = false;
    }

    if(!atEnd()){

      getCursorPosition(&line,&col);
      killstring = textLine(line);
      killstring = killstring.mid(col,killstring.length());


      if(!killbufferstring.isEmpty() && !killtrue && !lastwasanewline){
        killbufferstring += '\n';
      }

      if( (killstring.length() == 0) && !killtrue){
        killbufferstring += '\n';
        lastwasanewline = true;
      }

      if(killstring.length() > 0){

        killbufferstring += killstring;
        lastwasanewline = false;
        killtrue = true;

      }else{

        lastwasanewline = false;
        killtrue = !killtrue;

      }

    }else{

	if(killbufferstring.isEmpty() && !killtrue && !lastwasanewline){
	  killtrue = true;
	}

    }

    killing = true;

    TQMultiLineEdit::keyPressEvent(e);
    setModified(true);
    return;
  }
  else if ( keyQt == CTRL+Key_Y ){

    int line = 0;
    int col  = 0;

    getCursorPosition(&line,&col);

    TQString tmpstring = killbufferstring;
    if(!killtrue)
      tmpstring += '\n';

    insertAt(tmpstring,line,col);

    killing = false;
    setModified(true);
    return;
  }

  killing = false;

  if ( TDEStdAccel::copy().contains( key ) )
    copy();
  else if ( isReadOnly() )
    TQMultiLineEdit::keyPressEvent( e );
  // If this is an unmodified printable key, send it directly to TQMultiLineEdit.
  else if ( !(key.keyCodeQt() & (CTRL | ALT)) && !e->text().isEmpty() && TQString(e->text()).unicode()->isPrint() )
    TQMultiLineEdit::keyPressEvent( e );
  else if ( TDEStdAccel::paste().contains( key ) ) {
    paste();
    setModified(true);
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::cut().contains( key ) ) {
    cut();
    setModified(true);
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::undo().contains( key ) ) {
    undo();
    setModified(true);
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::redo().contains( key ) ) {
    redo();
    setModified(true);
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::deleteWordBack().contains( key ) ) {
    moveCursor(MoveWordBackward, true);
    if (hasSelectedText())
      del();
    setModified(true);
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::deleteWordForward().contains( key ) ) {
    moveCursor(MoveWordForward, true);
    if (hasSelectedText())
      del();
    setModified(true);
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::backwardWord().contains( key ) ) {
    CursorAction action = MoveWordBackward;
    int para, index;
    getCursorPosition( &para, & index );
    if (text(para).isRightToLeft())
       action = MoveWordForward;
    moveCursor(action, false );
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::forwardWord().contains( key ) ) {
    CursorAction action = MoveWordForward;
    int para, index;
    getCursorPosition( &para, & index );
    if (text(para).isRightToLeft())
      action = MoveWordBackward;
    moveCursor( action, false );
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::next().contains( key ) ) {
    moveCursor( MovePgDown, false );
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::prior().contains( key ) ) {
    moveCursor( MovePgUp, false );
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::home().contains( key ) ) {
    moveCursor( MoveHome, false );
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::end().contains( key ) ) {
    moveCursor( MoveEnd, false );
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::beginningOfLine().contains( key ) ) {
    moveCursor( MoveLineStart, false);
    slotCursorPositionChanged();
  }
  else if ( TDEStdAccel::endOfLine().contains( key ) ) {
    moveCursor( MoveLineEnd, false);
    slotCursorPositionChanged();
  }
  else if ( key == Key_Insert ) {
    if (d->overwriteEnabled)
    {
      this->setOverwriteMode(!this->isOverwriteMode());
      emit toggle_overwrite_signal();
    }
  }
  else
    TQMultiLineEdit::keyPressEvent(e);
}

void KEdit::installRBPopup(TQPopupMenu *p) {
  KContextMenuManager::insert( this, p );
}

void KEdit::selectFont(){

  TQFont font = this->font();
  TDEFontDialog::getFont(font);
  this->setFont(font);

}

void KEdit::doGotoLine() {

   if( !gotodialog )
      gotodialog = new KEdGotoLine( parent, "gotodialog" );

   this->clearFocus();

   gotodialog->exec();
   // this seems to be not necessary
   // gotodialog->setFocus();
   if( gotodialog->result() != KEdGotoLine::Accepted)
      return;
   int target_line = gotodialog->getLineNumber()-1;
   if (wordWrap() == NoWrap)
   {
      setCursorPosition( target_line, 0 );
      setFocus();
      return;
   }

   int max_parag = paragraphs();

   int line = 0;
   int parag = -1;
   int lines_in_parag = 0;
   while ((++parag < max_parag) && (line + lines_in_parag < target_line))
   {
      line += lines_in_parag;
      lines_in_parag = linesOfParagraph(parag);
   }

   int col = 0;
   if (parag >= max_parag)
   {
      target_line = line + lines_in_parag - 1;
      parag = max_parag-1;
   }

   while(1+line+lineOfChar(parag,col) < target_line) col++;
   setCursorPosition( parag, col );
   setFocus();
}


void  KEdit::dragMoveEvent(TQDragMoveEvent* e) {

  if(KURLDrag::canDecode(e))
    e->accept();
  else if(TQTextDrag::canDecode(e))
    TQMultiLineEdit::dragMoveEvent(e);
}

void  KEdit::contentsDragMoveEvent(TQDragMoveEvent* e) {

  if(KURLDrag::canDecode(e))
    e->accept();
  else if(TQTextDrag::canDecode(e))
    TQMultiLineEdit::contentsDragMoveEvent(e);
}

void  KEdit::dragEnterEvent(TQDragEnterEvent* e) {

  kdDebug() << "KEdit::dragEnterEvent()" << endl;
  e->accept(KURLDrag::canDecode(e) || TQTextDrag::canDecode(e));
}

void  KEdit::contentsDragEnterEvent(TQDragEnterEvent* e) {

  kdDebug() << "KEdit::contentsDragEnterEvent()" << endl;
  e->accept(KURLDrag::canDecode(e) || TQTextDrag::canDecode(e));
}


void  KEdit::dropEvent(TQDropEvent* e) {

  kdDebug() << "KEdit::dropEvent()" << endl;

  if(KURLDrag::canDecode(e)) {
   emit gotUrlDrop(e);
  }
  else if(TQTextDrag::canDecode(e))
    TQMultiLineEdit::dropEvent(e);
}

void  KEdit::contentsDropEvent(TQDropEvent* e) {

  kdDebug() << "KEdit::contentsDropEvent()" << endl;

  if(KURLDrag::canDecode(e)) {
   emit gotUrlDrop(e);
  }
  else if(TQTextDrag::canDecode(e))
    TQMultiLineEdit::contentsDropEvent(e);
}

void KEdit::setOverwriteEnabled(bool b)
{
  d->overwriteEnabled = b;
}

// TQWidget::create() turns off mouse-Tracking which would break auto-hiding
void KEdit::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
  TQMultiLineEdit::create( id, initializeWindow, destroyOldWindow );
  KCursor::setAutoHideCursor( this, true );
}

void KEdit::ensureCursorVisible()
{
  if (!d->autoUpdate)
    return;

  TQMultiLineEdit::ensureCursorVisible();
}

void KEdit::setCursor( const TQCursor &c )
{
  if (!d->autoUpdate)
    return;

  TQMultiLineEdit::setCursor(c);
}

void KEdit::viewportPaintEvent( TQPaintEvent*pe )
{
  if (!d->autoUpdate)
    return;

  TQMultiLineEdit::viewportPaintEvent(pe);
}


void KEdGotoLine::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KEdFind::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KEdReplace::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KEdit::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

