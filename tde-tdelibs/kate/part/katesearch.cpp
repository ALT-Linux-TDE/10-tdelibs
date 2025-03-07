/* This file is part of the KDE libraries
   Copyright (C) 2004-2005 Anders Lund <anders@alweb.dk>
   Copyright (C) 2003 Clarence Dang <dang@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001-2004 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "katesearch.h"
#include "katesearch.moc"

#include "kateview.h"
#include "katedocument.h"
#include "katesupercursor.h"
#include "katearbitraryhighlight.h"
#include "kateconfig.h"
#include "katehighlight.h"

#include <tdelocale.h>
#include <kstdaction.h>
#include <tdemessagebox.h>
#include <kstringhandler.h>
#include <kdebug.h>
#include <kfinddialog.h>
#include <kreplacedialog.h>
#include <kpushbutton.h>

#include <tqlayout.h>
#include <tqlabel.h>

//BEGIN KateSearch
TQStringList KateSearch::s_searchList  = TQStringList();
TQStringList KateSearch::s_replaceList = TQStringList();
TQString KateSearch::s_pattern = TQString();
static const bool arbitraryHLExample = false;

KateSearch::KateSearch( KateView* view )
  : TQObject( view, "kate search" )
  , m_view( view )
  , m_doc( view->doc() )
  , replacePrompt( new KateReplacePrompt( view ) )
{
  m_arbitraryHLList = new KateSuperRangeList();
  if (arbitraryHLExample) m_doc->arbitraryHL()->addHighlightToView(m_arbitraryHLList, m_view);

  connect(replacePrompt,TQ_SIGNAL(clicked()),this,TQ_SLOT(replaceSlot()));
}

KateSearch::~KateSearch()
{
  delete m_arbitraryHLList;
}

void KateSearch::createActions( TDEActionCollection* ac )
{
  KStdAction::find( this, TQ_SLOT(find()), ac )->setWhatsThis(
    i18n("Look up the first occurrence of a piece of text or regular expression."));
  KStdAction::findNext( this, TQ_SLOT(slotFindNext()), ac )->setWhatsThis(
    i18n("Look up the next occurrence of the search phrase."));
  KStdAction::findPrev( this, TQ_SLOT(slotFindPrev()), ac, "edit_find_prev" )->setWhatsThis(
    i18n("Look up the previous occurrence of the search phrase."));
  KStdAction::replace( this, TQ_SLOT(replace()), ac )->setWhatsThis(
    i18n("Look up a piece of text or regular expression and replace the result with some given text."));
}

void KateSearch::addToList( TQStringList& list, const TQString& s )
{
  if( list.count() > 0 ) {
    TQStringList::Iterator it = list.find( s );
    if( *it != 0L )
      list.remove( it );
    if( list.count() >= 16 )
      list.remove( list.fromLast() );
  }
  list.prepend( s );
}

void KateSearch::find()
{
  // if multiline selection around, search in it
  long searchf = KateViewConfig::global()->searchFlags();
  if (m_view->hasSelection() && m_view->selStartLine() != m_view->selEndLine())
    searchf |= KFindDialog::SelectedText;

  KFindDialog *findDialog = new KFindDialog (  m_view, "", searchf,
                                               s_searchList, m_view->hasSelection() );

  findDialog->setPattern (getSearchText());


  if( findDialog->exec() == TQDialog::Accepted ) {
    s_searchList =  findDialog->findHistory () ;
    // Do *not* remove the TQString() wrapping, it fixes a nasty crash
    find( TQString(s_searchList.first()), findDialog->options(), true, true );
  }

  delete findDialog;
  m_view->repaintText ();
}

void KateSearch::find( const TQString &pattern, long flags, bool add, bool shownotfound )
{
  KateViewConfig::global()->setSearchFlags( flags );
  if( add )
    addToList( s_searchList, pattern );

   s_pattern = pattern;

  SearchFlags searchFlags;

  searchFlags.caseSensitive = KateViewConfig::global()->searchFlags() & KFindDialog::CaseSensitive;
  searchFlags.wholeWords = KateViewConfig::global()->searchFlags() & KFindDialog::WholeWordsOnly;
  searchFlags.fromBeginning = !(KateViewConfig::global()->searchFlags() & KFindDialog::FromCursor)
      && !(KateViewConfig::global()->searchFlags() & KFindDialog::SelectedText);
  searchFlags.backward = KateViewConfig::global()->searchFlags() & KFindDialog::FindBackwards;
  searchFlags.selected = KateViewConfig::global()->searchFlags() & KFindDialog::SelectedText;
  searchFlags.prompt = false;
  searchFlags.replace = false;
  searchFlags.finished = false;
  searchFlags.regExp = KateViewConfig::global()->searchFlags() & KFindDialog::RegularExpression;
  searchFlags.useBackRefs = KateViewConfig::global()->searchFlags() & KReplaceDialog::BackReference;

  if ( searchFlags.selected )
  {
    s.selBegin = KateTextCursor( m_view->selStartLine(), m_view->selStartCol() );
    s.selEnd   = KateTextCursor( m_view->selEndLine(),   m_view->selEndCol()   );
    s.cursor   = s.flags.backward ? s.selEnd : s.selBegin;
  } else {
    s.cursor = getCursor( searchFlags );
  }

  s.wrappedEnd = s.cursor;
  s.wrapped = false;
  s.showNotFound = shownotfound;

  search( searchFlags );
}

void KateSearch::replace()
{
  if (!doc()->isReadWrite()) return;

  // if multiline selection around, search in it
  long searchf = KateViewConfig::global()->searchFlags();
  if (m_view->hasSelection() && m_view->selStartLine() != m_view->selEndLine())
    searchf |= KFindDialog::SelectedText;

  KReplaceDialog *replaceDialog = new KReplaceDialog (  m_view, "", searchf,
                                               s_searchList, s_replaceList, m_view->hasSelection() );

  replaceDialog->setPattern (getSearchText());

  if( replaceDialog->exec() == TQDialog::Accepted ) {
    long opts = replaceDialog->options();
    m_replacement = replaceDialog->replacement();
    s_searchList = replaceDialog->findHistory () ;
    s_replaceList = replaceDialog->replacementHistory () ;

    // Do *not* remove the TQString() wrapping, it fixes a nasty crash
    replace( TQString(s_searchList.first()), m_replacement, opts );
  }

  delete replaceDialog;
  m_view->update ();
}

void KateSearch::replace( const TQString& pattern, const TQString &replacement, long flags )
{
  if (!doc()->isReadWrite()) return;

  addToList( s_searchList, pattern );
   s_pattern = pattern;
  addToList( s_replaceList, replacement );
  m_replacement = replacement;
  KateViewConfig::global()->setSearchFlags( flags );

  SearchFlags searchFlags;
  searchFlags.caseSensitive = KateViewConfig::global()->searchFlags() & KFindDialog::CaseSensitive;
  searchFlags.wholeWords = KateViewConfig::global()->searchFlags() & KFindDialog::WholeWordsOnly;
  searchFlags.fromBeginning = !(KateViewConfig::global()->searchFlags() & KFindDialog::FromCursor)
      && !(KateViewConfig::global()->searchFlags() & KFindDialog::SelectedText);
  searchFlags.backward = KateViewConfig::global()->searchFlags() & KFindDialog::FindBackwards;
  searchFlags.selected = KateViewConfig::global()->searchFlags() & KFindDialog::SelectedText;
  searchFlags.prompt = KateViewConfig::global()->searchFlags() & KReplaceDialog::PromptOnReplace;
  searchFlags.replace = true;
  searchFlags.finished = false;
  searchFlags.regExp = KateViewConfig::global()->searchFlags() & KFindDialog::RegularExpression;
  searchFlags.useBackRefs = KateViewConfig::global()->searchFlags() & KReplaceDialog::BackReference;
  if ( searchFlags.selected )
  {
    s.selBegin = KateTextCursor( m_view->selStartLine(), m_view->selStartCol() );
    s.selEnd   = KateTextCursor( m_view->selEndLine(), m_view->selEndCol()   );
    s.cursor   = s.flags.backward ? s.selEnd : s.selBegin;
  } else {
    s.cursor = getCursor( searchFlags );
  }

  s.wrappedEnd = s.cursor;
  s.wrapped = false;

  search( searchFlags );
}

void KateSearch::findAgain( bool reverseDirection )
{
  SearchFlags searchFlags;
  searchFlags.caseSensitive = KateViewConfig::global()->searchFlags() & KFindDialog::CaseSensitive;
  searchFlags.wholeWords = KateViewConfig::global()->searchFlags() & KFindDialog::WholeWordsOnly;
  searchFlags.fromBeginning = !(KateViewConfig::global()->searchFlags() & KFindDialog::FromCursor)
                               && !(KateViewConfig::global()->searchFlags() & KFindDialog::SelectedText);
  searchFlags.backward = KateViewConfig::global()->searchFlags() & KFindDialog::FindBackwards;
  searchFlags.selected = KateViewConfig::global()->searchFlags() & KFindDialog::SelectedText;
  searchFlags.prompt = KateViewConfig::global()->searchFlags() & KReplaceDialog::PromptOnReplace;
  searchFlags.replace = false;
  searchFlags.finished = false;
  searchFlags.regExp = KateViewConfig::global()->searchFlags() & KFindDialog::RegularExpression;
  searchFlags.useBackRefs = KateViewConfig::global()->searchFlags() & KReplaceDialog::BackReference;

  if (reverseDirection)
    searchFlags.backward = !searchFlags.backward;

  searchFlags.fromBeginning = false;
  searchFlags.prompt = true; // ### why is the above assignment there?

  s.cursor = getCursor( searchFlags );
  search( searchFlags );
}

void KateSearch::search( SearchFlags flags )
{
  s.flags = flags;

  if( s.flags.fromBeginning ) {
    if( !s.flags.backward ) {
      s.cursor.setPos(0, 0);
    } else {
      s.cursor.setLine(doc()->numLines() - 1);
      s.cursor.setCol(doc()->lineLength( s.cursor.line() ));
    }
  }

  if((!s.flags.backward &&
       s.cursor.col() == 0 &&
       s.cursor.line() == 0 ) ||
     ( s.flags.backward &&
       s.cursor.col() == doc()->lineLength( s.cursor.line() ) &&
       s.cursor.line() == (((int)doc()->numLines()) - 1) ) ) {
    s.flags.finished = true;
  }

  if( s.flags.replace ) {
    replaces = 0;
    if( s.flags.prompt )
      promptReplace();
    else
      replaceAll();
  } else {
    findAgain();
  }
}

void KateSearch::wrapSearch()
{
  if( s.flags.selected )
  {
    KateTextCursor start (s.selBegin);
    KateTextCursor end (s.selEnd);

    // recalc for block sel, to have start with lowest col, end with highest
    if (m_view->blockSelectionMode())
    {
      start.setCol (kMin(s.selBegin.col(), s.selEnd.col()));
      end.setCol (kMax(s.selBegin.col(), s.selEnd.col()));
    }

    s.cursor = s.flags.backward ? end : start;
  }
  else
  {
    if( !s.flags.backward ) {
      s.cursor.setPos(0, 0);
    } else {
      s.cursor.setLine(doc()->numLines() - 1);
      s.cursor.setCol(doc()->lineLength( s.cursor.line() ) );
    }
  }

  // oh, we wrapped around one time allready now !
  // only check that on replace
  s.wrapped = s.flags.replace;

  replaces = 0;
  s.flags.finished = true;
}

void KateSearch::findAgain()
{
  if(  s_pattern.isEmpty() ) {
    find();
    return;
  }

  if ( doSearch(  s_pattern ) ) {
    exposeFound( s.cursor, s.matchedLength );
  } else if( !s.flags.finished ) {
    if( askContinue() ) {
      wrapSearch();
      findAgain();
    } else {
      if (arbitraryHLExample) m_arbitraryHLList->clear();
    }
  } else {
    if (arbitraryHLExample) m_arbitraryHLList->clear();
    if ( s.showNotFound )
      KMessageBox::sorry( view(),
        i18n("Search string '%1' not found!")
             .arg( KStringHandler::csqueeze(  s_pattern ) ),
        i18n("Find"));
  }
}

void KateSearch::replaceAll()
{
  doc()->editStart ();

  while( doSearch(  s_pattern ) )
    replaceOne();

  doc()->editEnd ();

  if( !s.flags.finished ) {
    if( askContinue() ) {
      wrapSearch();
      replaceAll();
    }
  } else {
    KMessageBox::information( view(),
        i18n("%n replacement made.","%n replacements made.",replaces),
        i18n("Replace") );
  }
}

void KateSearch::promptReplace()
{
  if ( doSearch(  s_pattern ) ) {
    exposeFound( s.cursor, s.matchedLength );
    replacePrompt->show();
    replacePrompt->setFocus ();
  } else if( !s.flags.finished && askContinue() ) {
    wrapSearch();
    promptReplace();
  } else {
    if (arbitraryHLExample) m_arbitraryHLList->clear();
    replacePrompt->hide();
    KMessageBox::information( view(),
        i18n("%n replacement made.","%n replacements made.",replaces),
        i18n("Replace") );
  }
}

void KateSearch::replaceOne()
{
  TQString replaceWith = m_replacement;
  if ( s.flags.regExp && s.flags.useBackRefs ) {
    // Replace each "\0"..."\9" with the corresponding capture,
    // "\n" and "\t" with newline and tab,
    // "\\" with "\",
    // and remove the "\" for any other sequence.
    TQRegExp br("\\\\(.)");
    int pos = br.search( replaceWith );
    int ncaps = m_re.numCaptures();
    while ( pos >= 0 ) {
      TQString substitute;
      TQChar argument = TQString(br.cap(1)).at(0);
      if ( argument.isDigit() ) {
        // the second character is a digit, this is a backreference
        int ccap = argument.digitValue();
        if (ccap <= ncaps ) {
          substitute = m_re.cap( ccap );
        } else {
          kdDebug()<<"KateSearch::replaceOne(): you don't have "<<ccap<<" backreferences in regexp '"<<TQString(m_re.pattern())<<"'"<<endl;
          break;
        }
      } else if ( argument == 'n' ) {
        substitute = '\n';
      } else if ( argument == 't' ) {
        substitute = '\t';
      } else {
        // handle a validly escaped backslash, or an invalid escape.
        substitute = argument;
      }
      replaceWith.replace( pos, br.matchedLength(), substitute );
      pos = br.search( replaceWith, pos + substitute.length() );
    }
  }

  doc()->editStart();
  doc()->removeText( s.cursor.line(), s.cursor.col(),
      s.cursor.line(), s.cursor.col() + s.matchedLength );
  doc()->insertText( s.cursor.line(), s.cursor.col(), replaceWith );
  doc()->editEnd(),

  replaces++;

  // if we inserted newlines, we better adjust.
  uint newlines = replaceWith.contains('\n');
  if ( newlines )
  {
    if ( ! s.flags.backward )
    {
      s.cursor.setLine( s.cursor.line() + newlines );
      s.cursor.setCol( replaceWith.length() - replaceWith.findRev('\n') );
    }
    // selection?
    if ( s.flags.selected )
      s.selEnd.setLine( s.selEnd.line() + newlines );
  }


  // adjust selection endcursor if needed
  if( s.flags.selected && s.cursor.line() == s.selEnd.line() )
  {
    s.selEnd.setCol(s.selEnd.col() + replaceWith.length() - s.matchedLength );
  }

  // adjust wrap cursor if needed
  if( s.cursor.line() == s.wrappedEnd.line() && s.cursor.col() <= s.wrappedEnd.col())
  {
    s.wrappedEnd.setCol(s.wrappedEnd.col() + replaceWith.length() - s.matchedLength );
  }

  if( !s.flags.backward ) {
    s.cursor.setCol(s.cursor.col() + replaceWith.length());
  } else if( s.cursor.col() > 0 ) {
    s.cursor.setCol(s.cursor.col() - 1);
  } else {
    s.cursor.setLine(s.cursor.line() - 1);
    if( s.cursor.line() >= 0 ) {
      s.cursor.setCol(doc()->lineLength( s.cursor.line() ));
    }
  }
}

void KateSearch::skipOne()
{
  if( !s.flags.backward ) {
    s.cursor.setCol(s.cursor.col() + s.matchedLength);
  } else if( s.cursor.col() > 0 ) {
    s.cursor.setCol(s.cursor.col() - 1);
  } else {
    s.cursor.setLine(s.cursor.line() - 1);
    if( s.cursor.line() >= 0 ) {
      s.cursor.setCol(doc()->lineLength(s.cursor.line()));
    }
  }
}

void KateSearch::replaceSlot() {
  switch( (Dialog_results)replacePrompt->result() ) {
  case srCancel: replacePrompt->hide();                break;
  case srAll:    replacePrompt->hide(); replaceAll();  break;
  case srYes:    replaceOne(); promptReplace();        break;
  case srLast:   replacePrompt->hide(), replaceOne();  break;
  case srNo:     skipOne();    promptReplace();        break;
  }
}

bool KateSearch::askContinue()
{
  TQString made =
     i18n( "%n replacement made.",
           "%n replacements made.",
           replaces );

  TQString reached = !s.flags.backward ?
     i18n( "End of document reached." ) :
     i18n( "Beginning of document reached." );

  if (KateViewConfig::global()->searchFlags() & KFindDialog::SelectedText)
  {
    reached = !s.flags.backward ?
     i18n( "End of selection reached." ) :
     i18n( "Beginning of selection reached." );
  }

  TQString question = !s.flags.backward ?
     i18n( "Continue from the beginning?" ) :
     i18n( "Continue from the end?" );

  TQString text = s.flags.replace ?
     made + "\n" + reached + "\n" + question :
     reached + "\n" + question;

  return KMessageBox::Yes == KMessageBox::questionYesNo(
     view(), text, s.flags.replace ? i18n("Replace") : i18n("Find"),
     KStdGuiItem::cont(), i18n("&Stop") );
}

TQString KateSearch::getSearchText()
{
  // SelectionOnly: use selection
  // WordOnly: use word under cursor
  // SelectionWord: use selection if available, else use word under cursor
  // WordSelection: use word if available, else use selection
  TQString str;

  int getFrom = view()->config()->textToSearchMode();
  switch (getFrom)
  {
  case KateViewConfig::SelectionOnly: // (Windows)
    //kdDebug() << "getSearchText(): SelectionOnly" << endl;
    if( m_view->hasSelection() )
      str = m_view->selection();
    break;

  case KateViewConfig::SelectionWord: // (classic Kate behavior)
    //kdDebug() << "getSearchText(): SelectionWord" << endl;
    if( m_view->hasSelection() )
      str = m_view->selection();
    else
      str = view()->currentWord();
    break;

  case KateViewConfig::WordOnly: // (weird?)
    //kdDebug() << "getSearchText(): WordOnly" << endl;
    str = view()->currentWord();
    break;

  case KateViewConfig::WordSelection: // (persistent selection lover)
    //kdDebug() << "getSearchText(): WordSelection" << endl;
    str = view()->currentWord();
    if (str.isEmpty() && m_view->hasSelection() )
      str = m_view->selection();
    break;

  default: // (nowhere)
    //kdDebug() << "getSearchText(): Nowhere" << endl;
    break;
  }

  str.replace( TQRegExp("^\\n"), "" );
  str.replace( TQRegExp("\\n.*"), "" );

  return str;
}

KateTextCursor KateSearch::getCursor( SearchFlags flags )
{
  if (flags.backward && !flags.selected && view()->hasSelection())
  {
    // We're heading backwards (and not within a selection),
    // the selection might start before the cursor.
    return kMin( KateTextCursor(view()->selStartLine(), view()->selStartCol()),
                 KateTextCursor(view()->cursorLine(), view()->cursorColumnReal()));
  }
  return KateTextCursor(view()->cursorLine(), view()->cursorColumnReal());
}

bool KateSearch::doSearch( const TQString& text )
{
/*
  rodda: Still Working on this... :)

  bool result = false;

  if (m_searchResults.count()) {
    m_resultIndex++;
    if (m_resultIndex < (int)m_searchResults.count()) {
      s = m_searchResults[m_resultIndex];
      result = true;
    }

  } else {
    int temp = 0;
    do {*/

#if 0
  static int oldLine = -1;
  static int oldCol = -1;
#endif

  uint line = s.cursor.line();
  uint col = s.cursor.col();// + (result ? s.matchedLength : 0);
  bool backward = s.flags.backward;
  bool caseSensitive = s.flags.caseSensitive;
  bool regExp = s.flags.regExp;
  bool wholeWords = s.flags.wholeWords;
  uint foundLine, foundCol, matchLen;
  bool found = false;
  //kdDebug() << "Searching at " << line << ", " << col << endl;
//   kdDebug()<<"KateSearch::doSearch: "<<line<<", "<<col<<", "<<backward<<endl;

  if (backward)
  {
    KateDocCursor docCursor(line, col, doc());

    // If we're at the top of the document, we're not gonna find anything, so bail.
    if (docCursor.line() == 0 && docCursor.col() == 0)
      return false;

    // Move one step backward before searching, if this is a "find again", we don't
    // want to find the same match.
    docCursor.moveBackward(1);
    line = docCursor.line();
    col = docCursor.col();
  }

  do {
      if( regExp ) {
        m_re = TQRegExp( text, caseSensitive );
        found = doc()->searchText( line, col, m_re,
                                  &foundLine, &foundCol,
                                  &matchLen, backward );
      }
      else if ( wholeWords )
      {
        bool maybefound = false;
        do
        {
          maybefound = doc()->searchText( line, col, text,
                                  &foundLine, &foundCol,
                                  &matchLen, caseSensitive, backward );
          if ( maybefound )
          {
            found = (
                      ( foundCol == 0 ||
                        ! doc()->highlight()->isInWord( doc()->textLine( foundLine ).at( foundCol - 1 ) ) ) &&
                      ( foundCol + matchLen == doc()->lineLength( foundLine ) ||
                        ! doc()->highlight()->isInWord( doc()->textLine( foundLine ).at( foundCol + matchLen ) ) )
                    );
            if ( found )
            {
              break;
            }
            else if ( backward && foundCol == 0 ) // we are done on this line and want to avoid endless loops like in #137312
            {
              if ( line == 0 ) // we are completely done...
                break;
              else
                line--;
            }
            else
            {
              line = foundLine;
              col = foundCol + 1;
            }
          }
        } while ( maybefound );
      }
      else {
        found = doc()->searchText( line, col, text,
                                  &foundLine, &foundCol,
                                  &matchLen, caseSensitive, backward );
      }

    if ( found && s.flags.selected )
    {
      KateTextCursor start (s.selBegin);
      KateTextCursor end (s.selEnd);

      // recalc for block sel, to have start with lowest col, end with highest
      if (m_view->blockSelectionMode())
      {
        start.setCol (kMin(s.selBegin.col(), s.selEnd.col()));
        end.setCol (kMax(s.selBegin.col(), s.selEnd.col()));
      }

      if ( !s.flags.backward && KateTextCursor( foundLine, foundCol ) >= end
        ||  s.flags.backward && KateTextCursor( foundLine, foundCol ) < start )
      {
        found = false;
      }
      else if (m_view->blockSelectionMode())
      {
        if ((int)foundCol >= start.col() && (int)foundCol < end.col())
          break;
      }
    }

    line = foundLine;
    col = foundCol+1;
  }
  while (s.flags.selected && m_view->blockSelectionMode() && found);
  // in the case we want to search in selection + blockselection we need to loop

  if( !found ) return false;

  // save the search result
  s.cursor.setPos(foundLine, foundCol);
  s.matchedLength = matchLen;

  // we allready wrapped around one time
  if (s.wrapped)
  {
    if (s.flags.backward)
    {
      if ( (s.cursor.line() < s.wrappedEnd.line())
           || ( (s.cursor.line() == s.wrappedEnd.line()) && ((s.cursor.col()+matchLen) <= uint(s.wrappedEnd.col())) ) )
        return false;
    }
    else
    {
      if ( (s.cursor.line() > s.wrappedEnd.line())
           || ( (s.cursor.line() == s.wrappedEnd.line()) && (s.cursor.col() > s.wrappedEnd.col()) ) )
        return false;
    }
  }

//   kdDebug() << "Found at " << s.cursor.line() << ", " << s.cursor.col() << endl;


  //m_searchResults.append(s);

  if (arbitraryHLExample)  {
    KateArbitraryHighlightRange* hl = new KateArbitraryHighlightRange(new KateSuperCursor(m_doc, true, s.cursor), new KateSuperCursor(m_doc, true, s.cursor.line(), s.cursor.col() + s.matchedLength), this);
    hl->setBold();
    hl->setTextColor(TQt::white);
    hl->setBGColor(TQt::black);
    // destroy the highlight upon change
    connect(hl, TQ_SIGNAL(contentsChanged()), hl, TQ_SIGNAL(eliminated()));
    m_arbitraryHLList->append(hl);
  }

  return true;

    /* rodda: more of my search highlighting work

    } while (++temp < 100);

    if (result) {
      s = m_searchResults.first();
      m_resultIndex = 0;
    }
  }

  return result;*/
}

void KateSearch::exposeFound( KateTextCursor &cursor, int slen )
{
  view()->setCursorPositionInternal ( cursor.line(), cursor.col() + slen, 1 );
  view()->setSelection( cursor.line(), cursor.col(), cursor.line(), cursor.col() + slen );
  view()->syncSelectionCache();
}
//END KateSearch

//BEGIN KateReplacePrompt
// this dialog is not modal
KateReplacePrompt::KateReplacePrompt ( TQWidget *parent )
  : KDialogBase ( parent, 0L, false, i18n( "Replace Confirmation" ),
                  User3 | User2 | User1 | Close | Ok , Ok, true,
                  i18n("Replace &All"), i18n("Re&place && Close"), i18n("&Replace") )
{
  setButtonOK( i18n("&Find Next") );
  TQWidget *page = new TQWidget(this);
  setMainWidget(page);

  TQBoxLayout *topLayout = new TQVBoxLayout( page, 0, spacingHint() );
  TQLabel *label = new TQLabel(i18n("Found an occurrence of your search term. What do you want to do?"),page);
  topLayout->addWidget(label );
}

void KateReplacePrompt::slotOk ()
{ // Search Next
  done(KateSearch::srNo);
  actionButton(Ok)->setFocus();
}

void KateReplacePrompt::slotClose ()
{ // Close
  done(KateSearch::srCancel);
  actionButton(Close)->setFocus();
}

void KateReplacePrompt::slotUser1 ()
{ // Replace All
  done(KateSearch::srAll);
  actionButton(User1)->setFocus();
}

void KateReplacePrompt::slotUser2 ()
{ // Replace & Close
  done(KateSearch::srLast);
  actionButton(User2)->setFocus();
}

void KateReplacePrompt::slotUser3 ()
{ // Replace
  done(KateSearch::srYes);
  actionButton(User3)->setFocus();
}

void KateReplacePrompt::done (int result)
{
  setResult(result);

  emit clicked();
}
//END KateReplacePrompt

//BEGIN SearchCommand
bool SearchCommand::exec(class Kate::View *view, const TQString &cmd, TQString &msg)
{
  TQString flags, pattern, replacement;
  if ( cmd.startsWith( "find" ) )
  {

    static TQRegExp re_find("find(?::([bcersw]*))?\\s+(.+)");
    if ( re_find.search( cmd ) < 0 )
    {
      msg = i18n("Usage: find[:[bcersw]] PATTERN");
      return false;
    }
    flags = re_find.cap( 1 );
    pattern = re_find.cap( 2 );
  }

  else if ( cmd.startsWith( "ifind" ) )
  {
    static TQRegExp re_ifind("ifind(?::([bcrs]*))?\\s+(.*)");
    if ( re_ifind.search( cmd ) < 0 )
    {
      msg = i18n("Usage: ifind[:[bcrs]] PATTERN");
      return false;
    }
    ifindClear();
    return true;
  }

  else if ( cmd.startsWith( "replace" ) )
  {
    // Try if the pattern and replacement is quoted, using a quote character ["']
    static TQRegExp re_rep("replace(?::([bceprsw]*))?\\s+([\"'])((?:[^\\\\\\\\2]|\\\\.)*)\\2\\s+\\2((?:[^\\\\\\\\2]|\\\\.)*)\\2\\s*$");
    // Or one quoted argument
    TQRegExp re_rep1("replace(?::([bceprsw]*))?\\s+([\"'])((?:[^\\\\\\\\2]|\\\\.)*)\\2\\s*$");
    // Else, it's just one or two (space separated) words
    TQRegExp re_rep2("replace(?::([bceprsw]*))?\\s+(\\S+)(.*)");
#define unbackslash(s) p=0;\
while ( (p = pattern.find( '\\' + delim, p )) > -1 )\
{\
  if ( !p || pattern[p-1] != '\\' )\
    pattern.remove( p, 1 );\
  p++;\
}

    if ( re_rep.search( cmd ) >= 0 )
    {
      flags = re_rep.cap(1);
      pattern = re_rep.cap( 3 );
      replacement = re_rep.cap( 4 );

      int p(0);
      // unbackslash backslashed delimiter strings
      // in pattern ..
      TQString delim = re_rep.cap( 2 );
      unbackslash(pattern);
      // .. and in replacement
      unbackslash(replacement);
    }
    else if ( re_rep1.search( cmd ) >= 0 )
    {
      flags = re_rep1.cap(1);
      pattern = re_rep1.cap( 3 );

      int p(0);
      TQString delim = re_rep1.cap( 2 );
      unbackslash(pattern);
    }
    else if ( re_rep2.search( cmd ) >= 0 )
    {
      flags = re_rep2.cap( 1 );
      pattern = re_rep2.cap( 2 );
      replacement = TQString(re_rep2.cap( 3 )).stripWhiteSpace();
    }
    else
    {
      msg = i18n("Usage: replace[:[bceprsw]] PATTERN [REPLACEMENT]");
      return false;
    }
    kdDebug()<<"replace '"<<pattern<<"' with '"<<replacement<<"'"<<endl;
#undef unbackslash
  }

  long f = 0;
  if ( flags.contains( 'b' ) ) f |= KFindDialog::FindBackwards;
  if ( flags.contains( 'c' ) ) f |= KFindDialog::FromCursor;
  if ( flags.contains( 'e' ) ) f |= KFindDialog::SelectedText;
  if ( flags.contains( 'r' ) ) f |= KFindDialog::RegularExpression;
  if ( flags.contains( 'p' ) ) f |= KReplaceDialog::PromptOnReplace;
  if ( flags.contains( 's' ) ) f |= KFindDialog::CaseSensitive;
  if ( flags.contains( 'w' ) ) f |= KFindDialog::WholeWordsOnly;

  if ( cmd.startsWith( "find" ) )
  {
    ((KateView*)view)->find( pattern, f );
    return true;
  }
  else if ( cmd.startsWith( "replace" ) )
  {
    f |= KReplaceDialog::BackReference; // mandatory here?
    ((KateView*)view)->replace( pattern, replacement, f );
    return true;
  }

  return false;
}

bool SearchCommand::help(class Kate::View *, const TQString &cmd, TQString &msg)
{
  if ( cmd == "find" )
    msg = i18n("<p>Usage: <code>find[:bcersw] PATTERN</code></p>");

  else if ( cmd == "ifind" )
    msg = i18n("<p>Usage: <code>ifind:[:bcrs] PATTERN</code>"
        "<br>ifind does incremental or 'as-you-type' search</p>");

  else
    msg = i18n("<p>Usage: <code>replace[:bceprsw] PATTERN REPLACEMENT</code></p>");

  msg += i18n(
      "<h4><caption>Options</h4><p>"
      "<b>b</b> - Search backward"
      "<br><b>c</b> - Search from cursor"
      "<br><b>r</b> - Pattern is a regular expression"
      "<br><b>s</b> - Case sensitive search"
             );

  if ( cmd == "find" )
    msg += i18n(
        "<br><b>e</b> - Search in selected text only"
        "<br><b>w</b> - Search whole words only"
               );

  if ( cmd == "replace" )
    msg += i18n(
        "<br><b>p</b> - Prompt for replace</p>"
        "<p>If REPLACEMENT is not present, an empty string is used.</p>"
        "<p>If you want to have whitespace in your PATTERN, you need to "
        "quote both PATTERN and REPLACEMENT with either single or double "
        "quotes. To have the quote characters in the strings, prepend them "
        "with a backslash.");

  msg += "</p>";
  return true;
}

TQStringList SearchCommand::cmds()
{
  TQStringList l;
  l << "find" << "replace" << "ifind";
  return l;
}

bool SearchCommand::wantsToProcessText( const TQString &cmdname )
{
  return  cmdname == "ifind";
}

void SearchCommand::processText( Kate::View *view, const TQString &cmd )
{
  static TQRegExp re_ifind("ifind(?::([bcrs]*))?\\s(.*)");
  if ( re_ifind.search( cmd ) > -1 )
  {
    TQString flags = re_ifind.cap( 1 );
    TQString pattern = re_ifind.cap( 2 );


    // if there is no setup, or the text length is 0, set up the properties
    if ( ! m_ifindFlags || pattern.isEmpty() )
      ifindInit( flags );
    // if there is no fromCursor, add it if this is not the first character
    else if ( ! ( m_ifindFlags & KFindDialog::FromCursor ) && ! pattern.isEmpty() )
      m_ifindFlags |= KFindDialog::FromCursor;

    // search..
    if ( ! pattern.isEmpty() )
    {
      KateView *v = (KateView*)view;

      // If it *looks like* we are continuing, place the cursor
      // at the beginning of the selection, so that the search continues.
      // ### check more carefully, like is  the cursor currently at the end
      // of the selection.
      if ( pattern.startsWith( v->selection() ) &&
           v->selection().length() + 1 == pattern.length() )
        v->setCursorPositionInternal( v->selStartLine(), v->selStartCol() );

      v->find( pattern, m_ifindFlags, false );
    }
  }
}

void SearchCommand::ifindInit( const TQString &flags )
{
  long f = 0;
  if ( flags.contains( 'b' ) ) f |= KFindDialog::FindBackwards;
  if ( flags.contains( 'c' ) ) f |= KFindDialog::FromCursor;
  if ( flags.contains( 'r' ) ) f |= KFindDialog::RegularExpression;
  if ( flags.contains( 's' ) ) f |= KFindDialog::CaseSensitive;
  m_ifindFlags = f;
}

void SearchCommand::ifindClear()
{
  m_ifindFlags = 0;
}
//END SearchCommand
