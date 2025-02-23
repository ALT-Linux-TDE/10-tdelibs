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

#include "katespell.h"
#include "katespell.moc"

#include "kateview.h"

#include <tdeaction.h>
#include <kstdaction.h>
#include <tdespell.h>
#include <ksconfig.h>
#include <kdebug.h>
#include <tdemessagebox.h>

KateSpell::KateSpell( KateView* view )
  : TQObject( view )
  , m_view (view)
  , m_tdespell (0)
{
}

KateSpell::~KateSpell()
{
  // tdespell stuff
  if( m_tdespell )
  {
    m_tdespell->setAutoDelete(true);
    m_tdespell->cleanUp(); // need a way to wait for this to complete
    delete m_tdespell;
  }
}

void KateSpell::createActions( TDEActionCollection* ac )
{
   KStdAction::spelling( this, TQ_SLOT(spellcheck()), ac );
   TDEAction *a = new TDEAction( i18n("Spelling (from cursor)..."), "tools-check-spelling", 0, this, TQ_SLOT(spellcheckFromCursor()), ac, "tools_spelling_from_cursor" );
   a->setWhatsThis(i18n("Check the document's spelling from the cursor and forward"));

   m_spellcheckSelection = new TDEAction( i18n("Spellcheck Selection..."), "tools-check-spelling", 0, this, TQ_SLOT(spellcheckSelection()), ac, "tools_spelling_selection" );
   m_spellcheckSelection->setWhatsThis(i18n("Check spelling of the selected text"));
}

void KateSpell::updateActions ()
{
  m_spellcheckSelection->setEnabled (m_view->hasSelection ());
}

void KateSpell::spellcheckFromCursor()
{
  spellcheck( KateTextCursor(m_view->cursorLine(), m_view->cursorColumnReal()) );
}

void KateSpell::spellcheckSelection()
{
  KateTextCursor from( m_view->selStartLine(), m_view->selStartCol() );
  KateTextCursor to( m_view->selEndLine(), m_view->selEndCol() );
  spellcheck( from, to );
}

void KateSpell::spellcheck()
{
  spellcheck( KateTextCursor( 0, 0 ) );
}

void KateSpell::spellcheck( const KateTextCursor &from, const KateTextCursor &to )
{
  m_spellStart = from;
  m_spellEnd = to;

  if ( to.line() == 0 && to.col() == 0 )
  {
    int lln = m_view->doc()->lastLine();
    m_spellEnd.setLine( lln );
    m_spellEnd.setCol( m_view->doc()->lineLength( lln ) );
  }

  m_spellPosCursor = from;
  m_spellLastPos = 0;

  TQString mt = m_view->doc()->mimeType()/*->name()*/;

  KSpell::SpellerType type = KSpell::Text;
  if ( mt == "text/x-tex" || mt == "text/x-latex" )
    type = KSpell::TeX;
  else if ( mt == "text/html" || mt == "text/xml" || mt == "text/docbook" || mt == "application/x-php")
    type = KSpell::HTML;

  KSpellConfig *ksc = new KSpellConfig;
  TQStringList ksEncodings;
  ksEncodings << "US-ASCII" << "ISO 8859-1" << "ISO 8859-2" << "ISO 8859-3"
      << "ISO 8859-4" << "ISO 8859-5" << "ISO 8859-7" << "ISO 8859-8"
      << "ISO 8859-9" << "ISO 8859-13" << "ISO 8859-15" << "UTF-8"
      << "KOI8-R" << "KOI8-U" << "CP1251" << "CP1255";

  int enc = ksEncodings.findIndex( m_view->doc()->encoding() );
  if ( enc > -1 )
  {
    ksc->setEncoding( enc );
    kdDebug(13020)<<"KateSpell::spellCheck(): using encoding: "<<enc<<" ("<<ksEncodings[enc]<<") and KSpell::Type "<<type<<" (for '"<<mt<<"')"<<endl;
  }
  else
    kdDebug(13020)<<"KateSpell::spellCheck(): using encoding: "<<enc<<" and KSpell::Type "<<type<<" (for '"<<mt<<"')"<<endl;

  m_tdespell = new KSpell( m_view, i18n("Spellcheck"),
                         this, TQ_SLOT(ready(KSpell *)), ksc, true, true, type );

  connect( m_tdespell, TQ_SIGNAL(death()),
           this, TQ_SLOT(spellCleanDone()) );

  connect( m_tdespell, TQ_SIGNAL(misspelling(const TQString&, const TQStringList&, unsigned int)),
           this, TQ_SLOT(misspelling(const TQString&, const TQStringList&, unsigned int)) );
  connect( m_tdespell, TQ_SIGNAL(corrected(const TQString&, const TQString&, unsigned int)),
           this, TQ_SLOT(corrected(const TQString&, const TQString&, unsigned int)) );
  connect( m_tdespell, TQ_SIGNAL(done(const TQString&)),
           this, TQ_SLOT(spellResult(const TQString&)) );
}

void KateSpell::ready(KSpell *)
{
  m_tdespell->setProgressResolution( 1 );

  m_tdespell->check( m_view->doc()->text( m_spellStart.line(), m_spellStart.col(), m_spellEnd.line(), m_spellEnd.col() ) );

  kdDebug (13020) << "SPELLING READY STATUS: " << m_tdespell->status () << endl;
}

void KateSpell::locatePosition( uint pos, uint& line, uint& col )
{
  uint remains;

  while ( m_spellLastPos < pos )
  {
    remains = pos - m_spellLastPos;
    uint l = m_view->doc()->lineLength( m_spellPosCursor.line() ) - m_spellPosCursor.col();
    if ( l > remains )
    {
      m_spellPosCursor.setCol( m_spellPosCursor.col() + remains );
      m_spellLastPos = pos;
    }
    else
    {
      m_spellPosCursor.setLine( m_spellPosCursor.line() + 1 );
      m_spellPosCursor.setCol(0);
      m_spellLastPos += l + 1;
    }
  }

  line = m_spellPosCursor.line();
  col = m_spellPosCursor.col();
}

void KateSpell::misspelling( const TQString& origword, const TQStringList&, unsigned int pos )
{
  uint line, col;

  locatePosition( pos, line, col );

  m_view->setCursorPositionInternal (line, col, 1);
  m_view->setSelection( line, col, line, col + origword.length() );
}

void KateSpell::corrected( const TQString& originalword, const TQString& newword, unsigned int pos )
{
  uint line, col;

  locatePosition( pos, line, col );

  m_view->doc()->removeText( line, col, line, col + originalword.length() );
  m_view->doc()->insertText( line, col, newword );
}

void KateSpell::spellResult( const TQString& )
{
  m_view->clearSelection();
  m_tdespell->cleanUp();
}

void KateSpell::spellCleanDone()
{
  KSpell::spellStatus status = m_tdespell->status();

  if( status == KSpell::Error ) {
    KMessageBox::sorry( 0,
      i18n("The spelling program could not be started. "
           "Please make sure you have set the correct spelling program "
           "and that it is properly configured and in your PATH."));
  } else if( status == KSpell::Crashed ) {
    KMessageBox::sorry( 0,
      i18n("The spelling program seems to have crashed."));
  }

  delete m_tdespell;
  m_tdespell = 0;

  kdDebug (13020) << "SPELLING END" << endl;
}
//END
