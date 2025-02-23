/* This file is part of the KDE libraries
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>
   Copyright (C) 2002 John Firebaugh <jfirebaugh@kde.org>
   Copyright (C) 2001 by Victor Röder <Victor_Roeder@GMX.de>
   Copyright (C) 2002 by Roberto Raggi <roberto@kdevelop.org>

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

/******** Partly based on the ArgHintWidget of Qt3 by Trolltech AS *********/
/* Trolltech doesn't mind, if we license that piece of code as LGPL, because there isn't much
 * left from the desigener code */

#include "katecodecompletion.h"
#include "katecodecompletion.moc"

#include "katedocument.h"
#include "kateview.h"
#include "katerenderer.h"
#include "kateconfig.h"
#include "katefont.h"

#include <kdebug.h>

#include <tqwhatsthis.h>
#include <tqvbox.h>
#include <tqlistbox.h>
#include <tqtimer.h>
#include <tqtooltip.h>
#include <tqapplication.h>
#include <tqsizegrip.h>
#include <tqfontmetrics.h>
#include <tqlayout.h>
#include <tqregexp.h>

/**
 * This class is used as the codecompletion listbox. It can be resized according to its contents,
 *  therfor the needed size is provided by sizeHint();
 *@short Listbox showing codecompletion
 *@author Jonas B. Jacobi <j.jacobi@gmx.de>
 */
class KateCCListBox : public TQListBox
{
  public:
    /**
      @short Create a new CCListBox
    */
    KateCCListBox (TQWidget* parent = 0, const char* name = 0, WFlags f = 0):TQListBox(parent, name, f)
    {
    }

    TQSize sizeHint()  const
    {
        int count = this->count();
        int height = 20;
        int tmpwidth = 8;
        //FIXME the height is for some reasons at least 3 items heigh, even if there is only one item in the list
        if (count > 0)
            if(count < 11)
                height =  count * itemHeight(0);
            else  {
                height = 10 * itemHeight(0);
                tmpwidth += verticalScrollBar()->width();
            }

        int maxcount = 0, tmpcount = 0;
        for (int i = 0; i < count; ++i)
            if ( (tmpcount = fontMetrics().width(text(i)) ) > maxcount)
                    maxcount = tmpcount;

        if (maxcount > TQApplication::desktop()->width()){
            tmpwidth = TQApplication::desktop()->width() - 5;
            height += horizontalScrollBar()->height();
        } else
            tmpwidth += maxcount;
        return TQSize(tmpwidth,height);

    }
};

class KateCompletionItem : public TQListBoxText
{
  public:
    KateCompletionItem( TQListBox* lb, KTextEditor::CompletionEntry entry )
      : TQListBoxText( lb )
      , m_entry( entry )
    {
      if( entry.postfix == "()" ) { // should be configurable
        setText( entry.prefix + " " + entry.text + entry.postfix );
      } else {
        setText( entry.prefix + " " + entry.text + " " + entry.postfix);
      }
    }

    KTextEditor::CompletionEntry m_entry;
};


KateCodeCompletion::KateCodeCompletion( KateView* view )
  : TQObject( view, "Kate Code Completion" )
  , m_view( view )
  , m_commentLabel( 0 )
{
  m_completionPopup = new TQVBox( 0, 0, (WFlags)WType_Popup );
  m_completionPopup->setFrameStyle( TQFrame::Box | TQFrame::Plain );
  m_completionPopup->setLineWidth( 1 );

  m_completionListBox = new KateCCListBox( m_completionPopup );
  m_completionListBox->setFrameStyle( TQFrame::NoFrame );
  //m_completionListBox->setCornerWidget( new TQSizeGrip( m_completionListBox) );
  m_completionListBox->setFocusProxy( m_view->m_viewInternal );

  m_completionListBox->installEventFilter( this );

  m_completionPopup->resize(m_completionListBox->sizeHint() + TQSize(2,2));
  m_completionPopup->installEventFilter( this );
  m_completionPopup->setFocusProxy( m_view->m_viewInternal );

  m_pArgHint = new KateArgHint( m_view );
  connect( m_pArgHint, TQ_SIGNAL(argHintHidden()),
           this, TQ_SIGNAL(argHintHidden()) );

  connect( m_view, TQ_SIGNAL(cursorPositionChanged()),
           this, TQ_SLOT(slotCursorPosChanged()) );
}

KateCodeCompletion::~KateCodeCompletion()
{
  delete m_completionPopup;
}

bool KateCodeCompletion::codeCompletionVisible () {
  return m_completionPopup->isVisible();
}

void KateCodeCompletion::showCompletionBox(
    TQValueList<KTextEditor::CompletionEntry> complList, int offset, bool casesensitive )
{
  kdDebug(13035) << "showCompletionBox " << endl;

  if ( codeCompletionVisible() ) return;

  m_caseSensitive = casesensitive;
  m_complList = complList;
  m_offset = offset;
  m_view->cursorPositionReal( &m_lineCursor, &m_colCursor );
  m_colCursor -= offset;

  updateBox( true );
}

bool KateCodeCompletion::eventFilter( TQObject *o, TQEvent *e )
{
  if ( o != m_completionPopup &&
       o != m_completionListBox &&
       o != m_completionListBox->viewport() )
    return false;

   if( e->type() == TQEvent::Hide )
   { 
     //don't use abortCompletion() as aborting here again will send abort signal
     //even on successfull completion we will emit completionAborted() twice...
     m_completionPopup->hide();
     delete m_commentLabel;
     m_commentLabel = 0;
     return false;
   }


   if ( e->type() == TQEvent::MouseButtonDblClick  ) {
    doComplete();
    return false;
   }

   if ( e->type() == TQEvent::MouseButtonPress ) {
    TQTimer::singleShot(0, this, TQ_SLOT(showComment()));
    return false;
   }

  return false;
}

void KateCodeCompletion::handleKey (TQKeyEvent *e)
{
  // close completion if you move out of range
  if ((e->key() == Key_Up) && (m_completionListBox->currentItem() == 0))
  {
    abortCompletion();
    m_view->setFocus();
    return;
  }

  // keyboard movement
  if( (e->key() == Key_Up)    || (e->key() == Key_Down ) ||
        (e->key() == Key_Home ) || (e->key() == Key_End)   ||
        (e->key() == Key_Prior) || (e->key() == Key_Next ))
  {
    TQTimer::singleShot(0,this,TQ_SLOT(showComment()));
    TQApplication::sendEvent( m_completionListBox, (TQEvent*)e );
    return;
  }

  // update the box
  updateBox();
}

void KateCodeCompletion::doComplete()
{
  KateCompletionItem* item = static_cast<KateCompletionItem*>(
     m_completionListBox->item(m_completionListBox->currentItem()));

  if( item == 0 )
    return;

  TQString text = item->m_entry.text;
  TQString currentLine = m_view->currentTextLine();
  int len = m_view->cursorColumnReal() - m_colCursor;
  TQString currentComplText = currentLine.mid(m_colCursor,len);
  TQString add = text.mid(currentComplText.length());
  if( item->m_entry.postfix == "()" )
    add += "(";

  emit filterInsertString(&(item->m_entry),&add);
  m_view->insertText(add);

  complete( item->m_entry );
  m_view->setFocus();
}

void KateCodeCompletion::abortCompletion()
{
  m_completionPopup->hide();
  delete m_commentLabel;
  m_commentLabel = 0;
  emit completionAborted();
}

void KateCodeCompletion::complete( KTextEditor::CompletionEntry entry )
{
  m_completionPopup->hide();
  delete m_commentLabel;
  m_commentLabel = 0;
  emit completionDone( entry );
  emit completionDone();
}

void KateCodeCompletion::updateBox( bool )
{
  if( m_colCursor > m_view->cursorColumnReal() ) {
    // the cursor is too far left
    kdDebug(13035) << "Aborting Codecompletion after sendEvent" << endl;
    kdDebug(13035) << m_view->cursorColumnReal() << endl;
    abortCompletion();
    m_view->setFocus();
    return;
  }

  m_completionListBox->clear();

  TQString currentLine = m_view->currentTextLine();
  int len = m_view->cursorColumnReal() - m_colCursor;
  TQString currentComplText = currentLine.mid(m_colCursor,len);
/* No-one really badly wants those, or?
  kdDebug(13035) << "Column: " << m_colCursor << endl;
  kdDebug(13035) << "Line: " << currentLine << endl;
  kdDebug(13035) << "CurrentColumn: " << m_view->cursorColumnReal() << endl;
  kdDebug(13035) << "Len: " << len << endl;
  kdDebug(13035) << "Text: '" << currentComplText << "'" << endl;
  kdDebug(13035) << "Count: " << m_complList.count() << endl;
*/
  TQValueList<KTextEditor::CompletionEntry>::Iterator it;
  if( m_caseSensitive ) {
    for( it = m_complList.begin(); it != m_complList.end(); ++it ) {
      if( (*it).text.startsWith(currentComplText) ) {
        new KateCompletionItem(m_completionListBox,*it);
      }
    }
  } else {
    currentComplText = currentComplText.upper();
    for( it = m_complList.begin(); it != m_complList.end(); ++it ) {
      if( (*it).text.upper().startsWith(currentComplText) ) {
        new KateCompletionItem(m_completionListBox,*it);
      }
    }
  }

  if( m_completionListBox->count() == 0 ||
      ( m_completionListBox->count() == 1 && // abort if we equaled the last item
        currentComplText == m_completionListBox->text(0).stripWhiteSpace() ) ) {
    abortCompletion();
    m_view->setFocus();
    return;
  }

    kdDebug(13035)<<"KateCodeCompletion::updateBox: Resizing widget"<<endl;
        m_completionPopup->resize(m_completionListBox->sizeHint() + TQSize(2,2));
    TQPoint p = m_view->mapToGlobal( m_view->cursorCoordinates() );
        int x = p.x();
        int y = p.y() ;
        if ( y + m_completionPopup->height() + m_view->renderer()->config()->fontMetrics( )->height() > TQApplication::desktop()->height() )
                y -= (m_completionPopup->height() );
        else
                y += m_view->renderer()->config()->fontMetrics( )->height();

        if (x + m_completionPopup->width() > TQApplication::desktop()->width())
                x = TQApplication::desktop()->width() - m_completionPopup->width();

        m_completionPopup->move( TQPoint(x,y) );

  m_completionListBox->setCurrentItem( 0 );
  m_completionListBox->setSelected( 0, true );
  m_completionListBox->setFocus();
  m_completionPopup->show();

  TQTimer::singleShot(0,this,TQ_SLOT(showComment()));
}

void KateCodeCompletion::showArgHint ( TQStringList functionList, const TQString& strWrapping, const TQString& strDelimiter )
{
  unsigned int line, col;
  m_view->cursorPositionReal( &line, &col );
  m_pArgHint->reset( line, col );
  m_pArgHint->setArgMarkInfos( strWrapping, strDelimiter );

  int nNum = 0;
  TQStringList::Iterator end(functionList.end());
  for( TQStringList::Iterator it = functionList.begin(); it != end; ++it )
  {
    kdDebug(13035) << "Insert function text: " << *it << endl;

    m_pArgHint->addFunction( nNum, ( *it ) );

    nNum++;
  }

  m_pArgHint->move(m_view->mapToGlobal(m_view->cursorCoordinates() + TQPoint(0,m_view->renderer()->config()->fontMetrics( )->height())) );
  m_pArgHint->show();
}

void KateCodeCompletion::slotCursorPosChanged()
{
  m_pArgHint->cursorPositionChanged ( m_view, m_view->cursorLine(), m_view->cursorColumnReal() );
}

void KateCodeCompletion::showComment()
{
  if (!m_completionPopup->isVisible())
    return;

  KateCompletionItem* item = static_cast<KateCompletionItem*>(m_completionListBox->item(m_completionListBox->currentItem()));

  if( !item )
    return;

  if( item->m_entry.comment.isEmpty() )
    return;

  delete m_commentLabel;
  m_commentLabel = new KateCodeCompletionCommentLabel( 0, item->m_entry.comment );
  m_commentLabel->setFont(TQToolTip::font());
  m_commentLabel->setPalette(TQToolTip::palette());

  TQPoint rightPoint = m_completionPopup->mapToGlobal(TQPoint(m_completionPopup->width(),0));
  TQPoint leftPoint = m_completionPopup->mapToGlobal(TQPoint(0,0));
  TQRect screen = TQApplication::desktop()->screenGeometry ( m_commentLabel );
  TQPoint finalPoint;
  if (rightPoint.x()+m_commentLabel->width() > screen.x() + screen.width())
    finalPoint.setX(leftPoint.x()-m_commentLabel->width());
  else
    finalPoint.setX(rightPoint.x());

  m_completionListBox->ensureCurrentVisible();

  finalPoint.setY(
    m_completionListBox->viewport()->mapToGlobal(m_completionListBox->itemRect(
      m_completionListBox->item(m_completionListBox->currentItem())).topLeft()).y());

  m_commentLabel->move(finalPoint);
  m_commentLabel->show();
}

KateArgHint::KateArgHint( KateView* parent, const char* name )
    : TQFrame( parent, name, (WFlags)WType_Popup )
{
    setBackgroundColor( black );
    setPaletteForegroundColor( TQt::black );

    labelDict.setAutoDelete( true );
    layout = new TQVBoxLayout( this, 1, 2 );
    layout->setAutoAdd( true );
    editorView = parent;

    m_markCurrentFunction = true;

    setFocusPolicy( TQWidget::StrongFocus );
    setFocusProxy( parent );

    reset( -1, -1 );
}

KateArgHint::~KateArgHint()
{
}

void KateArgHint::setArgMarkInfos( const TQString& wrapping, const TQString& delimiter )
{
    m_wrapping = wrapping;
    m_delimiter = delimiter;
    m_markCurrentFunction = true;
}

void KateArgHint::reset( int line, int col )
{
    m_functionMap.clear();
    m_currentFunction = -1;
    labelDict.clear();

    m_currentLine = line;
    m_currentCol = col - 1;
}

void KateArgHint::slotDone(bool completed)
{
    hide();

    m_currentLine = m_currentCol = -1;

    emit argHintHidden();
    if (completed)
        emit argHintCompleted();
    else
        emit argHintAborted();
}

void KateArgHint::cursorPositionChanged( KateView* view, int line, int col )
{
    if( m_currentCol == -1 || m_currentLine == -1 ){
        slotDone(false);
        return;
    }

    int nCountDelimiter = 0;
    int count = 0;

    TQString currentTextLine = view->doc()->textLine( line );
    TQString text = currentTextLine.mid( m_currentCol, col - m_currentCol );
    TQRegExp strconst_rx( "\"[^\"]*\"" );
    TQRegExp chrconst_rx( "'[^']*'" );

    text = text
        .replace( strconst_rx, "\"\"" )
        .replace( chrconst_rx, "''" );

    int index = 0;
    while( index < (int)text.length() ){
        if( text[index] == m_wrapping[0] ){
            ++count;
        } else if( text[index] == m_wrapping[1] ){
            --count;
        } else if( count > 0 && text[index] == m_delimiter[0] ){
            ++nCountDelimiter;
        }
        ++index;
    }

    if( (m_currentLine > 0 && m_currentLine != line) || (m_currentLine < col) || (count == 0) ){
        slotDone(count == 0);
        return;
    }

    // setCurArg ( nCountDelimiter + 1 );

}

void KateArgHint::addFunction( int id, const TQString& prot )
{
    m_functionMap[ id ] = prot;
    TQLabel* label = new TQLabel( prot.stripWhiteSpace().simplifyWhiteSpace(), this );
    label->setBackgroundColor( TQColor(255, 255, 238) );
    label->show();
    labelDict.insert( id, label );

    if( m_currentFunction < 0 )
        setCurrentFunction( id );
}

void KateArgHint::setCurrentFunction( int currentFunction )
{
    if( m_currentFunction != currentFunction ){

        if( currentFunction < 0 )
            currentFunction = (int)m_functionMap.size() - 1;

        if( currentFunction > (int)m_functionMap.size()-1 )
            currentFunction = 0;

        if( m_markCurrentFunction && m_currentFunction >= 0 ){
            TQLabel* label = labelDict[ m_currentFunction ];
            label->setFont( font() );
        }

        m_currentFunction = currentFunction;

        if( m_markCurrentFunction ){
            TQLabel* label = labelDict[ currentFunction ];
            TQFont fnt( font() );
            fnt.setBold( true );
            label->setFont( fnt );
        }

        adjustSize();
    }
}

void KateArgHint::show()
{
    TQFrame::show();
    adjustSize();
}

bool KateArgHint::eventFilter( TQObject*, TQEvent* e )
{
    if( isVisible() && e->type() == TQEvent::KeyPress ){
        TQKeyEvent* ke = static_cast<TQKeyEvent*>( e );
        if( (ke->state() & ControlButton) && ke->key() == Key_Left ){
            setCurrentFunction( currentFunction() - 1 );
            ke->accept();
            return true;
        } else if( ke->key() == Key_Escape ){
            slotDone(false);
            return false;
        } else if( (ke->state() & ControlButton) && ke->key() == Key_Right ){
            setCurrentFunction( currentFunction() + 1 );
            ke->accept();
            return true;
        }
    }

    return false;
}

void KateArgHint::adjustSize( )
{
    TQRect screen = TQApplication::desktop()->screenGeometry( pos() );

    TQFrame::adjustSize();
    if( width() > screen.width() )
        resize( screen.width(), height() );

    if( x() + width() > screen.x() + screen.width() )
        move( screen.x() + screen.width() - width(), y() );
}
