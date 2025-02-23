/* This file is part of the KDE libraries
   Copyright (C) 2000 Reginald Stadlbauer <reggie@kde.org>
   Copyright (C) 2000,2003 Charles Samuels <charles@kde.org>
   Copyright (C) 2000 Peter Putzer
   Copyright (C) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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

#include "config.h"

#include <tqdragobject.h>
#include <tqtimer.h>
#include <tqheader.h>
#include <tqcursor.h>
#include <tqtooltip.h>
#include <tqstyle.h>
#include <tqpainter.h>

#include <tdeglobalsettings.h>
#include <tdeconfig.h>
#include <kcursor.h>
#include <tdeapplication.h>
#include <kipc.h>
#include <kdebug.h>

#include "tdelistview.h"
#include "tdelistviewlineedit.h"

class TDEListView::Tooltip : public TQToolTip
{
public:
  Tooltip (TDEListView* parent, TQToolTipGroup* group = 0L);
  virtual ~Tooltip () {}

protected:
  /**
   * Reimplemented from TQToolTip for internal reasons.
   */
  virtual void maybeTip (const TQPoint&);

private:
  TDEListView* mParent;
};

TDEListView::Tooltip::Tooltip (TDEListView* parent, TQToolTipGroup* group)
  : TQToolTip (parent, group),
        mParent (parent)
{
}

void TDEListView::Tooltip::maybeTip (const TQPoint&)
{
  // FIXME
}

class TDEListView::TDEListViewPrivate
{
public:
  TDEListViewPrivate (TDEListView* listview)
    : pCurrentItem (0),
      autoSelectDelay(0),
      useSmallExecuteArea(false),
      dragOverItem(0),
      dragDelay(TDEGlobalSettings::dndEventDelay()),
      editor(new TDEListViewLineEdit(listview)),
      cursorInExecuteArea(false),
      itemsMovable (true),
      selectedBySimpleMove(false),
      selectedUsingMouse(false),
      itemsRenameable (false),
      validDrag (false),
      dragEnabled (false),
      autoOpen (true),
      disableAutoSelection (false),
      dropVisualizer (true),
      dropHighlighter (false),
      createChildren (true),
      pressedOnSelected (false),
      wasShiftEvent (false),
      fullWidth (false),
      sortAscending(true),
      tabRename(true),
      sortColumn(0),
      selectionDirection(0),
      selectionRegion(0),
      tooltipColumn (0),
      selectionMode (Single),
      contextMenuKey (TDEGlobalSettings::contextMenuKey()),
      showContextMenusOnPress (TDEGlobalSettings::showContextMenusOnPress()),
      mDropVisualizerWidth (4),
      paintAbove (0),
      paintCurrent (0),
      paintBelow (0),
      painting (false),
      shadeSortColumn(TDEGlobalSettings::shadeSortColumn()),
      initialFileManagerItem(0)
  {
      renameable.append(0);
      connect(editor, TQ_SIGNAL(done(TQListViewItem*,int)), listview, TQ_SLOT(doneEditing(TQListViewItem*,int)));
      connect(editor, TQ_SIGNAL(renameNext(TQListViewItem*,int)), listview, TQ_SLOT(renameNextProxy(TQListViewItem*,int)));
      connect(editor, TQ_SIGNAL(renamePrev(TQListViewItem*,int)), listview, TQ_SLOT(renamePrevProxy(TQListViewItem*,int)));
  }

  ~TDEListViewPrivate ()
  {
    delete editor;
  }

  TQListViewItem* pCurrentItem;

  TQTimer autoSelect;
  int autoSelectDelay;
  bool useSmallExecuteArea;

  TQTimer dragExpand;
  TQListViewItem* dragOverItem;
  TQPoint dragOverPoint;

  TQPoint startDragPos;
  int dragDelay;

  TDEListViewLineEdit *editor;
  TQValueList<int> renameable;

  bool cursorInExecuteArea:1;
  bool bUseSingle:1;
  bool bChangeCursorOverItem:1;
  bool itemsMovable:1;
  bool selectedBySimpleMove : 1;
  bool selectedUsingMouse:1;
  bool itemsRenameable:1;
  bool validDrag:1;
  bool dragEnabled:1;
  bool autoOpen:1;
  bool disableAutoSelection:1;
  bool dropVisualizer:1;
  bool dropHighlighter:1;
  bool createChildren:1;
  bool pressedOnSelected:1;
  bool wasShiftEvent:1;
  bool fullWidth:1;
  bool sortAscending:1;
  bool tabRename:1;

  int sortColumn;

  //+1 means downwards (y increases, -1 means upwards, 0 means not selected), aleXXX
  int selectionDirection;
  int selectionRegion;
  int tooltipColumn;

  SelectionModeExt selectionMode;
  int contextMenuKey;
  bool showContextMenusOnPress;

  TQRect mOldDropVisualizer;
  int mDropVisualizerWidth;
  TQRect mOldDropHighlighter;
  TQListViewItem *afterItemDrop;
  TQListViewItem *parentItemDrop;

  TQListViewItem *paintAbove;
  TQListViewItem *paintCurrent;
  TQListViewItem *paintBelow;
  bool painting:1;
  bool shadeSortColumn:1;

  TQColor alternateBackground;

  TQListViewItem *initialFileManagerItem;
};


TDEListViewLineEdit::TDEListViewLineEdit(TDEListView *parent)
    : KLineEdit(parent->viewport()), item(0), col(0), p(parent), m_renSett()
{
  setFrame( false );
  hide();
  connect( parent, TQ_SIGNAL( selectionChanged() ), TQ_SLOT( slotSelectionChanged() ));
  connect( parent, TQ_SIGNAL( itemRemoved( TQListViewItem * ) ),
                    TQ_SLOT( slotItemRemoved( TQListViewItem * ) ));
}

TDEListViewLineEdit::~TDEListViewLineEdit()
{
}

TQListViewItem *TDEListViewLineEdit::currentItem() const
{
	return item;
}

void TDEListViewLineEdit::load(TQListViewItem *i, int c)
{
        item=i;
        col=c;

        TQRect rect(p->itemRect(i));
        setText(item->text(c));
        home( true );

        int fieldX = rect.x() - 1;
        int fieldW = p->columnWidth(col) + 2;

        TQHeader* const pHeader = p->header();

        const int pos = pHeader->mapToIndex(col);
        for ( int index = 0; index < pos; ++index )
            fieldX += p->columnWidth( pHeader->mapToSection( index ));

        if ( col == 0 ) {
            int d = i->depth() + (p->rootIsDecorated() ? 1 : 0);
            d *= p->treeStepSize();
            fieldX += d;
            fieldW -= d;
        }

        if ( i->pixmap( col ) ) {// add width of pixmap
            int d = i->pixmap( col )->width();
            fieldX += d;
            fieldW -= d;
        }

        setGeometry(fieldX, rect.y() - 1, fieldW, rect.height() + 2);
        show();
        setFocus();
}

/*	Helper functions to for
 *	tabOrderedRename functionality.
 */

static int nextCol (TDEListView *pl, TQListViewItem *pi, int start, int dir)
{
	if (pi)
	{
		//	Find the next renameable column in the current row
		for (; ((dir == +1) ? (start < pl->columns()) : (start >= 0)); start += dir)
			if (pl->isRenameable(start))
				return start;
	}

	return -1;
}

static TQListViewItem *prevItem (TQListViewItem *pi)
{
	TQListViewItem *pa = pi->itemAbove();

	/*	Does what the TQListViewItem::previousSibling()
	 *	of my dreams would do.
	 */
	if (pa && pa->parent() == pi->parent())
		return pa;

	return 0;
}

static TQListViewItem *lastQChild (TQListViewItem *pi)
{
	if (pi)
	{
		/*	Since there's no TQListViewItem::lastChild().
		 *	This finds the last sibling for the given
		 *	item.
		 */
		for (TQListViewItem *pt = pi->nextSibling(); pt; pt = pt->nextSibling())
			pi = pt;
	}

	return pi;
}

void TDEListViewLineEdit::selectNextCell (TQListViewItem *pitem, int column, bool forward)
{
	const int ncols = p->columns();
	const int dir = forward ? +1 : -1;
	const int restart = forward ? 0 : (ncols - 1);
	TQListViewItem *top = (pitem && pitem->parent())
		? pitem->parent()->firstChild()
		: p->firstChild();
	TQListViewItem *pi = pitem;

	terminate();		//	Save current changes

	do
	{
		/*	Check the rest of the current row for an editable column,
		 *	if that fails, check the entire next/previous row. The
		 *	last case goes back to the first item in the current branch
		 *	or the last item in the current branch depending on the
		 *	direction.
		 */
		if ((column = nextCol(p, pi, column + dir, dir)) != -1 ||
			(column = nextCol(p, (pi = (forward ? pi->nextSibling() : prevItem(pi))), restart, dir)) != -1 ||
			(column = nextCol(p, (pi = (forward ? top : lastQChild(pitem))), restart, dir)) != -1)
		{
			if (pi)
			{
				p->setCurrentItem(pi);		//	Calls terminate
				p->rename(pi, column);

				/*	Some listviews may override rename() to
				 *	prevent certain items from being renamed,
				 *	if this is done, [m_]item will be NULL
				 *	after the rename() call... try again.
				 */
				if (!item)
					continue;

				break;
			}
		}
	}
	while (pi && !item);
}

#ifdef KeyPress
#undef KeyPress
#endif

bool TDEListViewLineEdit::event(TQEvent *pe)
{
	if (pe->type() == TQEvent::KeyPress)
	{
		TQKeyEvent *k = (TQKeyEvent*)pe;
    KKey kk(k);
    if (m_renSett.m_useRenameSignals &&
        (m_renSett.m_SCNext.contains(kk) || m_renSett.m_SCPrev.contains(kk)))
    {
      keyPressEvent(k);
      return true;
    }
	  else if ((k->key() == TQt::Key_Backtab || k->key() == TQt::Key_Tab) &&
			       p->tabOrderedRenaming() && p->itemsRenameable() &&
			       !(k->state() & ControlButton || k->state() & AltButton))
		{
			selectNextCell(item, col, (k->key() == Key_Tab && !(k->state() & ShiftButton)));
			return true;
	  }
	}
	return KLineEdit::event(pe);
}

void TDEListViewLineEdit::keyPressEvent(TQKeyEvent *e)
{
  KKey kk(e);
  if (m_renSett.m_useRenameSignals &&
      (m_renSett.m_SCNext.contains(kk) || m_renSett.m_SCPrev.contains(kk)))
  {
    TQListViewItem *i=item;
    int c=col;
    terminate(true);
    KLineEdit::keyPressEvent(e);
    if (m_renSett.m_SCNext.contains(kk))
    {
      emit renameNext(i,c);
    }
    else
    {
      emit renamePrev(i,c);
    }
  }
  else if (e->key() == TQt::Key_Return || e->key() == TQt::Key_Enter)
		terminate(true);
	else if(e->key() == TQt::Key_Escape)
		terminate(false);
  else if (e->key() == TQt::Key_Down || e->key() == TQt::Key_Up)
  {
		terminate(true);
    KLineEdit::keyPressEvent(e);
  }
  else
		KLineEdit::keyPressEvent(e);
}

void TDEListViewLineEdit::terminate()
{
    terminate(true);
}

void TDEListViewLineEdit::terminate(bool commit)
{
    if ( item )
    {
        //kdDebug() << "TDEListViewLineEdit::terminate " << commit << endl;
        if (commit)
            item->setText(col, text());
        int c=col;
        TQListViewItem *i=item;
        col=0;
        item=0;
        p->setFocus();// will call focusOutEvent, that's why we set item=0 before
        hide();
        if (commit)
            emit done(i,c);
    }
}

void TDEListViewLineEdit::focusOutEvent(TQFocusEvent *ev)
{
    TQFocusEvent * focusEv = static_cast<TQFocusEvent*>(ev);
    // Don't let a RMB close the editor
    if (focusEv->reason() != TQFocusEvent::Popup && focusEv->reason() != TQFocusEvent::ActiveWindow)
        terminate(true);
    else
        KLineEdit::focusOutEvent(ev);
}

void TDEListViewLineEdit::paintEvent( TQPaintEvent *e )
{
    KLineEdit::paintEvent( e );

    if ( !frame() ) {
        TQPainter p( this );
        p.setClipRegion( e->region() );
        p.drawRect( rect() );
    }
}

// selection changed -> terminate. As our "item" can be already deleted,
// we can't call terminate(false), because that would emit done() with
// a dangling pointer to "item".
void TDEListViewLineEdit::slotSelectionChanged()
{
    item = 0;
    col = 0;
    hide();
}

// if the current item was removed -> terminate.  Can't call terminate(false)
// due to same reason as slotSelectionChanged().
void TDEListViewLineEdit::slotItemRemoved(TQListViewItem *i)
{
    if (currentItem() != i)
        return;

    item = 0;
    col = 0;
    hide();
}


TDEListView::TDEListView( TQWidget *parent, const char *name )
  : TQListView(parent, name), d(new TDEListViewPrivate(this))
{
  setDragAutoScroll(true);

  connect( this, TQ_SIGNAL( onViewport() ),
                   this, TQ_SLOT( slotOnViewport() ) );
  connect( this, TQ_SIGNAL( onItem( TQListViewItem * ) ),
                   this, TQ_SLOT( slotOnItem( TQListViewItem * ) ) );

  connect (this, TQ_SIGNAL(contentsMoving(int,int)),
                   this, TQ_SLOT(cleanDropVisualizer()));
  connect (this, TQ_SIGNAL(contentsMoving(int,int)),
                   this, TQ_SLOT(cleanItemHighlighter()));

  slotSettingsChanged(TDEApplication::SETTINGS_MOUSE);
  if (kapp)
  {
    connect( kapp, TQ_SIGNAL( settingsChanged(int) ), TQ_SLOT( slotSettingsChanged(int) ) );
    kapp->addKipcEventMask( KIPC::SettingsChanged );
  }

  connect(&d->autoSelect, TQ_SIGNAL( timeout() ),
                  this, TQ_SLOT( slotAutoSelect() ) );
  connect(&d->dragExpand, TQ_SIGNAL( timeout() ),
                  this, TQ_SLOT( slotDragExpand() ) );

  // context menu handling
  if (d->showContextMenusOnPress)
        {
          connect (this, TQ_SIGNAL (rightButtonPressed (TQListViewItem*, const TQPoint&, int)),
                           this, TQ_SLOT (emitContextMenu (TQListViewItem*, const TQPoint&, int)));
        }
  else
        {
          connect (this, TQ_SIGNAL (rightButtonClicked (TQListViewItem*, const TQPoint&, int)),
                           this, TQ_SLOT (emitContextMenu (TQListViewItem*, const TQPoint&, int)));
        }

  connect (this, TQ_SIGNAL (menuShortCutPressed (TDEListView*, TQListViewItem*)),
                   this, TQ_SLOT (emitContextMenu (TDEListView*, TQListViewItem*)));
  d->alternateBackground = TDEGlobalSettings::alternateBackgroundColor();
}

TDEListView::~TDEListView()
{
  delete d;
}

bool TDEListView::isExecuteArea( const TQPoint& point )
{
  TQListViewItem* item = itemAt( point );
  if ( item ) {
    return isExecuteArea( point.x(), item );
  }

  return false;
}

bool TDEListView::isExecuteArea( int x )
{
  return isExecuteArea( x, 0 );
}

bool TDEListView::isExecuteArea( int x, TQListViewItem* item )
{
	if ( allColumnsShowFocus() ) {
		return true;
	}
	else {
		int offset = 0;
		int width = columnWidth( 0 );
		
		TQHeader* const thisHeader = header();
		const int pos = thisHeader->mapToIndex( 0 );
		
		for ( int index = 0; index < pos; ++index ) {
			offset += columnWidth( thisHeader->mapToSection( index ) );
		}
		
		x += contentsX(); // in case of a horizontal scrollbar

		// If a small execute area was requested, trim area to the size of the item text/icon
		// Otherwise just use the column width, as at least one entire column is highlighted on row selection!
		if ( item && d->useSmallExecuteArea ) {
			width = treeStepSize()*( item->depth() + ( rootIsDecorated() ? 1 : 0 ) );
			width += itemMargin();
			int ca = AlignHorizontal_Mask & columnAlignment( 0 );
			if ( ca == AlignLeft || ca == AlignAuto ) {
				width += item->width( fontMetrics(), this, 0 );
				if ( width > columnWidth( 0 ) ) {
					width = columnWidth( 0 );
				}
			}
		}
		if ( item ) {
			if (!allColumnsShowFocus()) {
				offset += treeStepSize()*( item->depth() + ( rootIsDecorated() ? 1 : 0 ) );
			}
		}
		
		return ( x > offset && x < ( offset + width ) );
	}
}

void TDEListView::slotOnItem( TQListViewItem *item )
{
  TQPoint vp = viewport()->mapFromGlobal( TQCursor::pos() );
  if ( item && isExecuteArea( vp.x() ) && (d->autoSelectDelay > -1) && d->bUseSingle ) {
    d->autoSelect.start( d->autoSelectDelay, true );
    d->pCurrentItem = item;
  }
}

void TDEListView::slotOnViewport()
{
  if ( d->bChangeCursorOverItem )
    viewport()->unsetCursor();

  d->autoSelect.stop();
  d->pCurrentItem = 0L;
}

void TDEListView::slotSettingsChanged(int category)
{
  switch (category)
  {
  case TDEApplication::SETTINGS_MOUSE:
    d->dragDelay =  TDEGlobalSettings::dndEventDelay();
    d->bUseSingle = TDEGlobalSettings::singleClick();

    disconnect(this, TQ_SIGNAL (mouseButtonClicked (int, TQListViewItem*, const TQPoint &, int)),
               this, TQ_SLOT (slotMouseButtonClicked (int, TQListViewItem*, const TQPoint &, int)));

    if( d->bUseSingle )
      connect (this, TQ_SIGNAL (mouseButtonClicked (int, TQListViewItem*, const TQPoint &, int)),
               this, TQ_SLOT (slotMouseButtonClicked( int, TQListViewItem*, const TQPoint &, int)));

    d->bChangeCursorOverItem = TDEGlobalSettings::changeCursorOverIcon();
    if ( !d->disableAutoSelection )
      d->autoSelectDelay = TDEGlobalSettings::autoSelectDelay();

    if( !d->bUseSingle || !d->bChangeCursorOverItem )
       viewport()->unsetCursor();

    break;

  case TDEApplication::SETTINGS_POPUPMENU:
    d->contextMenuKey = TDEGlobalSettings::contextMenuKey ();
    d->showContextMenusOnPress = TDEGlobalSettings::showContextMenusOnPress ();

    if (d->showContextMenusOnPress)
    {
      disconnect (0L, 0L, this, TQ_SLOT (emitContextMenu (TQListViewItem*, const TQPoint&, int)));

      connect(this, TQ_SIGNAL (rightButtonPressed (TQListViewItem*, const TQPoint&, int)),
              this, TQ_SLOT (emitContextMenu (TQListViewItem*, const TQPoint&, int)));
    }
    else
    {
      disconnect (0L, 0L, this, TQ_SLOT (emitContextMenu (TQListViewItem*, const TQPoint&, int)));

      connect(this, TQ_SIGNAL (rightButtonClicked (TQListViewItem*, const TQPoint&, int)),
              this, TQ_SLOT (emitContextMenu (TQListViewItem*, const TQPoint&, int)));
    }
    break;

  default:
    break;
  }
}

void TDEListView::slotAutoSelect()
{
  // check that the item still exists
  if( itemIndex( d->pCurrentItem ) == -1 )
    return;

  if (!isActiveWindow())
        {
          d->autoSelect.stop();
          return;
        }

  //Give this widget the keyboard focus.
  if( !hasFocus() )
    setFocus();

  ButtonState keybstate = TDEApplication::keyboardMouseState();

  TQListViewItem* previousItem = currentItem();
  setCurrentItem( d->pCurrentItem );

  if( d->pCurrentItem ) {
    //Shift pressed?
    if( (keybstate & TQt::ShiftButton) ) {
      bool block = signalsBlocked();
      blockSignals( true );

      //No Ctrl? Then clear before!
      if( !(keybstate & TQt::ControlButton) )
                clearSelection();

      bool select = !d->pCurrentItem->isSelected();
      bool update = viewport()->isUpdatesEnabled();
      viewport()->setUpdatesEnabled( false );

      bool down = previousItem->itemPos() < d->pCurrentItem->itemPos();
      TQListViewItemIterator lit( down ? previousItem : d->pCurrentItem );
      for ( ; lit.current(); ++lit ) {
                if ( down && lit.current() == d->pCurrentItem ) {
                  d->pCurrentItem->setSelected( select );
                  break;
                }
                if ( !down && lit.current() == previousItem ) {
                  previousItem->setSelected( select );
                  break;
                }
                lit.current()->setSelected( select );
      }

      blockSignals( block );
      viewport()->setUpdatesEnabled( update );
      triggerUpdate();

      emit selectionChanged();

      if( selectionMode() == TQListView::Single )
                emit selectionChanged( d->pCurrentItem );
    }
    else if( (keybstate & TDEApplication::ControlModifier) )
      setSelected( d->pCurrentItem, !d->pCurrentItem->isSelected() );
    else {
      bool block = signalsBlocked();
      blockSignals( true );

      if( !d->pCurrentItem->isSelected() )
                clearSelection();

      blockSignals( block );

      setSelected( d->pCurrentItem, true );
    }
  }
  else
    kdDebug() << "TDEListView::slotAutoSelect: That�s not supposed to happen!!!!" << endl;
}

void TDEListView::slotHeaderChanged()
{

  const int colCount = columns();
  if (d->fullWidth && colCount)
  {
    int w = 0;
    const int lastColumn = colCount - 1;
    for (int i = 0; i < lastColumn; ++i) w += columnWidth(i);
    setColumnWidth( lastColumn, viewport()->width() - w - 1 );
  }
}

void TDEListView::emitExecute( TQListViewItem *item, const TQPoint &pos, int c )
{
    if( isExecuteArea( viewport()->mapFromGlobal(pos) ) ) {
	d->validDrag=false;

        // Double click mode ?
        if ( !d->bUseSingle )
        {
            viewport()->unsetCursor();
            emit executed( item );
            emit executed( item, pos, c );
        }
        else
        {
            ButtonState keybstate = TDEApplication::keyboardMouseState();

            d->autoSelect.stop();

            //Don't emit executed if in SC mode and Shift or Ctrl are pressed
            if( !( ((keybstate & TQt::ShiftButton) || (keybstate & TQt::ControlButton)) ) ) {
                viewport()->unsetCursor();
                emit executed( item );
                emit executed( item, pos, c );
            }
        }
    }
}

void TDEListView::focusInEvent( TQFocusEvent *fe )
{
 //   kdDebug()<<"TDEListView::focusInEvent()"<<endl;
  TQListView::focusInEvent( fe );
  if ((d->selectedBySimpleMove)
      && (d->selectionMode == FileManager)
      && (fe->reason()!=TQFocusEvent::Popup)
      && (fe->reason()!=TQFocusEvent::ActiveWindow)
      && (currentItem()))
  {
      currentItem()->setSelected(true);
      currentItem()->repaint();
      emit selectionChanged();
  };
}

void TDEListView::focusOutEvent( TQFocusEvent *fe )
{
  cleanDropVisualizer();
  cleanItemHighlighter();

  d->autoSelect.stop();

  if ((d->selectedBySimpleMove)
      && (d->selectionMode == FileManager)
      && (fe->reason()!=TQFocusEvent::Popup)
      && (fe->reason()!=TQFocusEvent::ActiveWindow)
      && (currentItem())
      && (!d->editor->isVisible()))
  {
      currentItem()->setSelected(false);
      currentItem()->repaint();
      emit selectionChanged();
  };

  TQListView::focusOutEvent( fe );
}

void TDEListView::leaveEvent( TQEvent *e )
{
  d->autoSelect.stop();

  TQListView::leaveEvent( e );
}

bool TDEListView::event( TQEvent *e )
{
  if (e->type() == TQEvent::ApplicationPaletteChange)
    d->alternateBackground=TDEGlobalSettings::alternateBackgroundColor();

  return TQListView::event(e);
}

void TDEListView::contentsMousePressEvent( TQMouseEvent *e )
{
  if( (selectionModeExt() == Extended) && (e->state() & ShiftButton) && !(e->state() & ControlButton) )
  {
    bool block = signalsBlocked();
    blockSignals( true );

    clearSelection();

    blockSignals( block );
  }
  else if ((selectionModeExt()==FileManager) && (d->selectedBySimpleMove))
  {
     d->selectedBySimpleMove=false;
     d->selectedUsingMouse=true;
     if (currentItem())
     {
        currentItem()->setSelected(false);
        currentItem()->repaint();
//        emit selectionChanged();
     }
  }

  TQPoint p( contentsToViewport( e->pos() ) );
  TQListViewItem *at = itemAt (p);

  // true if the root decoration of the item "at" was clicked (i.e. the +/- sign)
  bool rootDecoClicked = at
           && ( p.x() <= header()->cellPos( header()->mapToActual( 0 ) ) +
                treeStepSize() * ( at->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() )
           && ( p.x() >= header()->cellPos( header()->mapToActual( 0 ) ) );

  if (e->button() == TQt::LeftButton && !rootDecoClicked)
  {
    //Start a drag
    d->startDragPos = e->pos();

    if (at)
    {
      d->validDrag = true;
      d->pressedOnSelected = at->isSelected();
    }
  }

  TQListView::contentsMousePressEvent( e );
}

void TDEListView::contentsMouseMoveEvent( TQMouseEvent *e )
{
  if (!dragEnabled() || d->startDragPos.isNull() || !d->validDrag)
      TQListView::contentsMouseMoveEvent (e);

  TQPoint vp = contentsToViewport(e->pos());
  TQListViewItem *item = itemAt( vp );

  //do we process cursor changes at all?
  if ( item && d->bChangeCursorOverItem && d->bUseSingle )
    {
      //Cursor moved on a new item or in/out the execute area
      if( (item != d->pCurrentItem) ||
          (isExecuteArea(vp) != d->cursorInExecuteArea) )
        {
          d->cursorInExecuteArea = isExecuteArea(vp);

          if( d->cursorInExecuteArea ) //cursor moved in execute area
            viewport()->setCursor( KCursor::handCursor() );
          else //cursor moved out of execute area
            viewport()->unsetCursor();
        }
    }

  bool dragOn = dragEnabled();
  TQPoint newPos = e->pos();
  if (dragOn && d->validDrag &&
      (newPos.x() > d->startDragPos.x()+d->dragDelay ||
       newPos.x() < d->startDragPos.x()-d->dragDelay ||
       newPos.y() > d->startDragPos.y()+d->dragDelay ||
       newPos.y() < d->startDragPos.y()-d->dragDelay))
    //(d->startDragPos - e->pos()).manhattanLength() > TQApplication::startDragDistance())
    {
      TQListView::contentsMouseReleaseEvent( 0 );
      startDrag();
      d->startDragPos = TQPoint();
      d->validDrag = false;
    }
}

void TDEListView::contentsMouseReleaseEvent( TQMouseEvent *e )
{
  if (e->button() == TQt::LeftButton)
  {
    // If the row was already selected, maybe we want to start an in-place editing
    if ( d->pressedOnSelected && itemsRenameable() )
    {
      TQPoint p( contentsToViewport( e->pos() ) );
      TQListViewItem *at = itemAt (p);
      if ( at )
      {
        // true if the root decoration of the item "at" was clicked (i.e. the +/- sign)
        bool rootDecoClicked =
                  ( p.x() <= header()->cellPos( header()->mapToActual( 0 ) ) +
                    treeStepSize() * ( at->depth() + ( rootIsDecorated() ? 1 : 0) ) + itemMargin() )
               && ( p.x() >= header()->cellPos( header()->mapToActual( 0 ) ) );

        if (!rootDecoClicked)
        {
          int col = header()->mapToLogical( header()->cellAt( p.x() ) );
          if ( d->renameable.contains(col) )
            rename(at, col);
        }
      }
    }

    d->pressedOnSelected = false;
    d->validDrag = false;
    d->startDragPos = TQPoint();
  }
  TQListView::contentsMouseReleaseEvent( e );
}

void TDEListView::contentsMouseDoubleClickEvent ( TQMouseEvent *e )
{
  // We don't want to call the parent method because it does setOpen,
  // whereas we don't do it in single click mode... (David)
  //TQListView::contentsMouseDoubleClickEvent( e );
  if ( !e || e->button() != TQt::LeftButton ) {
    return;
  }

  TQPoint vp = contentsToViewport(e->pos());
  TQListViewItem *item = itemAt( vp );
  emit TQListView::doubleClicked( item ); // we do it now

  int col = item ? header()->mapToLogical( header()->cellAt( vp.x() ) ) : -1;

  if( item ) {
    emit doubleClicked( item, e->globalPos(), col );

    if( (e->button() == TQt::LeftButton) && !d->bUseSingle ) {
      emitExecute( item, e->globalPos(), col );
    }
  }
}

void TDEListView::slotMouseButtonClicked( int btn, TQListViewItem *item, const TQPoint &pos, int c )
{
  if( (btn == TQt::LeftButton) && item ) {
    emitExecute(item, pos, c);
  }
}

void TDEListView::contentsDropEvent(TQDropEvent* e)
{
  cleanDropVisualizer();
  cleanItemHighlighter();
  d->dragExpand.stop();

  if (acceptDrag (e))
  {
    e->acceptAction();
    TQListViewItem *afterme;
    TQListViewItem *parent;

    findDrop(e->pos(), parent, afterme);

    if (e->source() == viewport() && itemsMovable())
        movableDropEvent(parent, afterme);
    else
    {
        emit dropped(e, afterme);
        emit dropped(this, e, afterme);
        emit dropped(e, parent, afterme);
        emit dropped(this, e, parent, afterme);
    }
  }
}

void TDEListView::movableDropEvent (TQListViewItem* parent, TQListViewItem* afterme)
{
  TQPtrList<TQListViewItem> items, afterFirsts, afterNows;
  TQListViewItem *current=currentItem();
  bool hasMoved=false;
  for (TQListViewItem *i = firstChild(), *iNext=0; i; i = iNext)
  {
    iNext=i->itemBelow();
    if (!i->isSelected())
      continue;

    // don't drop an item after itself, or else
    // it moves to the top of the list
    if (i==afterme)
      continue;

    i->setSelected(false);

    TQListViewItem *afterFirst = i->itemAbove();

        if (!hasMoved)
        {
                emit aboutToMove();
                hasMoved=true;
        }

    moveItem(i, parent, afterme);

    // ###### This should include the new parent !!! -> KDE 3.0
    // If you need this right now, have a look at keditbookmarks.
    emit moved(i, afterFirst, afterme);

    items.append (i);
    afterFirsts.append (afterFirst);
    afterNows.append (afterme);

    afterme = i;
  }
  clearSelection();
  for (TQListViewItem *i=items.first(); i; i=items.next() )
    i->setSelected(true);
  if (current)
    setCurrentItem(current);

  emit moved(items,afterFirsts,afterNows);

  if (firstChild())
    emit moved();
}

void TDEListView::contentsDragMoveEvent(TQDragMoveEvent *event)
{
  if (acceptDrag(event))
  {
    event->acceptAction();
    //Clean up the view

    findDrop(event->pos(), d->parentItemDrop, d->afterItemDrop);
    TQPoint vp = contentsToViewport( event->pos() );
    TQListViewItem *item = isExecuteArea( vp ) ? itemAt( vp ) : 0L;

    if ( item != d->dragOverItem )
    {
      d->dragExpand.stop();
      d->dragOverItem = item;
      d->dragOverPoint = vp;
      if ( d->dragOverItem && d->dragOverItem->isExpandable() && !d->dragOverItem->isOpen() )
        d->dragExpand.start( TQApplication::startDragTime(), true );
    }
    if (dropVisualizer())
    {
      TQRect tmpRect = drawDropVisualizer(0, d->parentItemDrop, d->afterItemDrop);
      if (tmpRect != d->mOldDropVisualizer)
      {
        cleanDropVisualizer();
        d->mOldDropVisualizer=tmpRect;
        viewport()->repaint(tmpRect);
      }
    }
    if (dropHighlighter())
    {
      TQRect tmpRect = drawItemHighlighter(0, itemAt( vp ));
      if (tmpRect != d->mOldDropHighlighter)
      {
        cleanItemHighlighter();
        d->mOldDropHighlighter=tmpRect;
        viewport()->repaint(tmpRect);
      }
    }
  }
  else
      event->ignore();
}

void TDEListView::slotDragExpand()
{
  if ( itemAt( d->dragOverPoint ) == d->dragOverItem )
    d->dragOverItem->setOpen( true );
}

void TDEListView::contentsDragLeaveEvent (TQDragLeaveEvent*)
{
  d->dragExpand.stop();
  cleanDropVisualizer();
  cleanItemHighlighter();
}

void TDEListView::cleanDropVisualizer()
{
  if (d->mOldDropVisualizer.isValid())
  {
    TQRect rect=d->mOldDropVisualizer;
    d->mOldDropVisualizer = TQRect();
    viewport()->repaint(rect, true);
  }
}

int TDEListView::depthToPixels( int depth )
{
    return treeStepSize() * ( depth + (rootIsDecorated() ? 1 : 0) ) + itemMargin();
}

void TDEListView::findDrop(const TQPoint &pos, TQListViewItem *&parent, TQListViewItem *&after)
{
	TQPoint p (contentsToViewport(pos));

	// Get the position to put it in
	TQListViewItem *atpos = itemAt(p);

	TQListViewItem *above;
	if (!atpos) // put it at the end
		above = lastItem();
	else
	{
		// Get the closest item before us ('atpos' or the one above, if any)
		if (p.y() - itemRect(atpos).topLeft().y() < (atpos->height()/2))
			above = atpos->itemAbove();
		else
			above = atpos;
	}

	if (above)
	{
		// if above has children, I might need to drop it as the first item there

		if (above->firstChild() && above->isOpen())
		{
			parent = above;
			after = 0;
			return;
		}

      // Now, we know we want to go after "above". But as a child or as a sibling ?
      // We have to ask the "above" item if it accepts children.
      if (above->isExpandable())
      {
          // The mouse is sufficiently on the right ? - doesn't matter if 'above' has visible children
          if (p.x() >= depthToPixels( above->depth() + 1 ) ||
              (above->isOpen() && above->childCount() > 0) )
          {
              parent = above;
              after = 0L;
              return;
          }
      }

      // Ok, there's one more level of complexity. We may want to become a new
      // sibling, but of an upper-level group, rather than the "above" item
      TQListViewItem * betterAbove = above->parent();
      TQListViewItem * last = above;
      while ( betterAbove )
      {
          // We are allowed to become a sibling of "betterAbove" only if we are
          // after its last child
          if ( !last->nextSibling() )
          {
              if (p.x() < depthToPixels ( betterAbove->depth() + 1 ))
                  above = betterAbove; // store this one, but don't stop yet, there may be a better one
              else
                  break; // not enough on the left, so stop
              last = betterAbove;
              betterAbove = betterAbove->parent(); // up one level
          } else
              break; // we're among the child of betterAbove, not after the last one
      }
  }
  // set as sibling
  after = above;
  parent = after ? after->parent() : 0L ;
}

TQListViewItem* TDEListView::lastChild () const
{
  TQListViewItem* lastchild = firstChild();

  if (lastchild)
        for (; lastchild->nextSibling(); lastchild = lastchild->nextSibling());

  return lastchild;
}

TQListViewItem *TDEListView::lastItem() const
{
  TQListViewItem* last = lastChild();

  for (TQListViewItemIterator it (last); it.current(); ++it)
    last = it.current();

  return last;
}

KLineEdit *TDEListView::renameLineEdit() const
{
  return d->editor;
}

void TDEListView::startDrag()
{
  TQDragObject *drag = dragObject();

  if (!drag)
        return;

  if (drag->drag() && drag->target() != viewport())
    emit moved();
}

TQDragObject *TDEListView::dragObject()
{
  if (!currentItem())
        return 0;


  return new TQStoredDrag("application/x-qlistviewitem", viewport());
}

void TDEListView::setItemsMovable(bool b)
{
  d->itemsMovable=b;
}

bool TDEListView::itemsMovable() const
{
  return d->itemsMovable;
}

void TDEListView::setItemsRenameable(bool b)
{
  d->itemsRenameable=b;
}

bool TDEListView::itemsRenameable() const
{
  return d->itemsRenameable;
}


void TDEListView::setDragEnabled(bool b)
{
  d->dragEnabled=b;
}

bool TDEListView::dragEnabled() const
{
  return d->dragEnabled;
}

void TDEListView::setAutoOpen(bool b)
{
  d->autoOpen=b;
}

bool TDEListView::autoOpen() const
{
  return d->autoOpen;
}

bool TDEListView::dropVisualizer() const
{
  return d->dropVisualizer;
}

void TDEListView::setDropVisualizer(bool b)
{
  d->dropVisualizer=b;
}

TQPtrList<TQListViewItem> TDEListView::selectedItems() const
{
  return selectedItems(true);
}

TQPtrList<TQListViewItem> TDEListView::selectedItems(bool includeHiddenItems) const
{
  TQPtrList<TQListViewItem> list;

  // Using selectionMode() instead of selectionModeExt() since for the cases that
  // we're interested in selectionMode() should work for either variety of the
  // setSelectionMode().

  switch(selectionMode())
  {
  case NoSelection:
      break;
  case Single:
      if(selectedItem() && (includeHiddenItems || selectedItem()->isVisible()))
          list.append(selectedItem());
      break;
  default:
  {
      int flags = TQListViewItemIterator::Selected;
      if (!includeHiddenItems)
      {
        flags |= TQListViewItemIterator::Visible;
      }

      TQListViewItemIterator it(const_cast<TDEListView *>(this), flags);

      for(; it.current(); ++it)
          list.append(it.current());

      break;
  }
  }

  return list;
}


void TDEListView::moveItem(TQListViewItem *item, TQListViewItem *parent, TQListViewItem *after)
{
  // sanity check - don't move a item into its own child structure
  TQListViewItem *i = parent;
  while(i)
    {
      if(i == item)
        return;
      i = i->parent();
    }

  if (after)
  {
      item->moveItem(after);
      return;
  }

  // Basically reimplementing the TQListViewItem(TQListViewItem*, TQListViewItem*) constructor
  // in here, without ever deleting the item.
  if (item->parent())
        item->parent()->takeItem(item);
  else
        takeItem(item);

  if (parent)
        parent->insertItem(item);
  else
        insertItem(item);
}

void TDEListView::contentsDragEnterEvent(TQDragEnterEvent *event)
{
  if (acceptDrag (event))
    event->accept();
}

void TDEListView::setDropVisualizerWidth (int w)
{
  d->mDropVisualizerWidth = w > 0 ? w : 1;
}

TQRect TDEListView::drawDropVisualizer(TQPainter *p, TQListViewItem *parent,
                                    TQListViewItem *after)
{
    TQRect insertmarker;

    if (!after && !parent)
        insertmarker = TQRect (0, 0, viewport()->width(), d->mDropVisualizerWidth/2);
    else
    {
        int level = 0;
        if (after)
        {
            TQListViewItem* it = 0L;
            if (after->isOpen())
            {
                // Look for the last child (recursively)
                it = after->firstChild();
                if (it)
                    while (it->nextSibling() || it->firstChild())
                        if ( it->nextSibling() )
                            it = it->nextSibling();
                        else
                            it = it->firstChild();
            }

            insertmarker = itemRect (it ? it : after);
            level = after->depth();
        }
        else if (parent)
        {
            insertmarker = itemRect (parent);
            level = parent->depth() + 1;
        }
        insertmarker.setLeft( treeStepSize() * ( level + (rootIsDecorated() ? 1 : 0) ) + itemMargin() );
        insertmarker.setRight (viewport()->width());
        insertmarker.setTop (insertmarker.bottom() - d->mDropVisualizerWidth/2 + 1);
        insertmarker.setBottom (insertmarker.bottom() + d->mDropVisualizerWidth/2);
    }

    // This is not used anymore, at least by TDEListView itself (see viewportPaintEvent)
    // Remove for KDE 4.0.
    if (p)
        p->fillRect(insertmarker, Dense4Pattern);

    return insertmarker;
}

TQRect TDEListView::drawItemHighlighter(TQPainter *painter, TQListViewItem *item)
{
  TQRect r;

  if (item)
  {
    r = itemRect(item);
    r.setLeft(r.left()+(item->depth()+(rootIsDecorated() ? 1 : 0))*treeStepSize());
    if (painter)
      style().drawPrimitive(TQStyle::PE_FocusRect, painter, r, colorGroup(),
                            TQStyle::Style_FocusAtBorder, colorGroup().highlight());
  }

  return r;
}

void TDEListView::cleanItemHighlighter ()
{
  if (d->mOldDropHighlighter.isValid())
  {
    TQRect rect=d->mOldDropHighlighter;
    d->mOldDropHighlighter = TQRect();
    viewport()->repaint(rect, true);
  }
}

void TDEListView::rename(TQListViewItem *item, int c)
{
  if (d->renameable.contains(c))
  {
    ensureItemVisible(item);
    d->editor->load(item,c);
  }
}

bool TDEListView::isRenameable (int col) const
{
  return d->renameable.contains(col);
}

void TDEListView::setRenameable (int col, bool renameable)
{
  if (col>=header()->count()) return;

  d->renameable.remove(col);
  if (renameable)
    d->renameable+=col;
}

void TDEListView::doneEditing(TQListViewItem *item, int row)
{
  emit itemRenamed(item, item->text(row), row);
  emit itemRenamed(item);
}

void TDEListView::renameNextProxy(TQListViewItem *item, int col)
{
  emit renameNext(item, col);
}

void TDEListView::renamePrevProxy(TQListViewItem *item, int col)
{
  emit renamePrev(item, col);
}


bool TDEListView::acceptDrag(TQDropEvent* e) const
{
  return acceptDrops() && itemsMovable() && (e->source()==viewport());
}

void TDEListView::setCreateChildren(bool b)
{
        d->createChildren=b;
}

bool TDEListView::createChildren() const
{
        return d->createChildren;
}


int TDEListView::tooltipColumn() const
{
        return d->tooltipColumn;
}

void TDEListView::setTooltipColumn(int column)
{
        d->tooltipColumn=column;
}

void TDEListView::setDropHighlighter(bool b)
{
        d->dropHighlighter=b;
}

bool TDEListView::dropHighlighter() const
{
        return d->dropHighlighter;
}

bool TDEListView::showTooltip(TQListViewItem *item, const TQPoint &, int column) const
{
        return ((column==tooltipColumn()) && !tooltip(item, column).isEmpty());
}

TQString TDEListView::tooltip(TQListViewItem *item, int column) const
{
        return item->text(column);
}

void TDEListView::setTabOrderedRenaming(bool b)
{
	d->tabRename = b;
}

bool TDEListView::tabOrderedRenaming() const
{
	return d->tabRename;
}

void TDEListView::keyPressEvent (TQKeyEvent* e)
{
  //don't we need a contextMenuModifier too ? (aleXXX)
  if (e->key() == d->contextMenuKey)
        {
          emit menuShortCutPressed (this, currentItem());
          return;
        }

  if (d->selectionMode != FileManager)
        TQListView::keyPressEvent (e);
  else
        fileManagerKeyPressEvent (e);
}

void TDEListView::activateAutomaticSelection()
{
   d->selectedBySimpleMove=true;
   d->selectedUsingMouse=false;
   if (currentItem())
   {
      currentItem()->setSelected(true);
      currentItem()->repaint();
      emit selectionChanged();
   };
}

void TDEListView::deactivateAutomaticSelection()
{
   d->selectedBySimpleMove=false;
}

bool TDEListView::automaticSelection() const
{
   return d->selectedBySimpleMove;
}

void TDEListView::resetKeyboardSelectionOperation()
{
	d->wasShiftEvent = false;
	d->selectionDirection = 0;
}

void TDEListView::setActiveMultiSelectItem(TQListViewItem *item) {
	TQListViewItem* origItem = currentItem();
	if (!d->initialFileManagerItem) {
		d->initialFileManagerItem = origItem;
	}
	setCurrentItem(item);
}

void TDEListView::fileManagerKeyPressEvent (TQKeyEvent* e)
{
	//don't care whether it's on the keypad or not
	int e_state=(e->state() & ~Keypad);

	// Handle non-control keypresses
	if ((e->key()!=Key_Shift) && (e->key()!=Key_Control)
		&& (e->key()!=Key_Meta) && (e->key()!=Key_Alt)) {
			if ((e_state==ShiftButton) && (!d->wasShiftEvent) && (!d->selectedBySimpleMove)) {
				selectAll(false);
				d->selectionRegion = 0;
				d->initialFileManagerItem = NULL;
			}
			d->selectionDirection=0;
			d->wasShiftEvent = (e_state == ShiftButton);
	}

	//d->wasShiftEvent = (e_state == ShiftButton);

	TQListViewItem* item = currentItem();
	if (!item) {
		return;
	}
	
	TQListViewItem* repaintItem1 = item;
	TQListViewItem* repaintItem2 = 0L;
	TQListViewItem* visItem = 0L;
	
	TQListViewItem* nextItem = 0L;
	int items = 0;

	bool shiftOrCtrl((e_state==ControlButton) || (e_state==ShiftButton));
	int selectedItems(0);
	for (TQListViewItem *tmpItem=firstChild(); tmpItem; tmpItem=tmpItem->nextSibling()) {
		if (tmpItem->isSelected()) selectedItems++;
	}

	if (((!selectedItems) || ((selectedItems==1) && (d->selectedUsingMouse)))
			&& (e_state==TQt::NoButton)
			&& ((e->key()==Key_Down)
			|| (e->key()==Key_Up)
			|| (e->key()==Key_Next)
			|| (e->key()==Key_Prior)
			|| (e->key()==Key_Home)
			|| (e->key()==Key_End))) {
		d->selectedBySimpleMove=true;
		d->selectedUsingMouse=false;
	}
	else if (selectedItems>1) {
		d->selectedBySimpleMove=false;
	}

	bool emitSelectionChanged(false);

	switch (e->key()) {
		case Key_Escape:
			selectAll(false);
			emitSelectionChanged=true;
			break;
	
		case Key_Space:
			//toggle selection of current item
			if (d->selectedBySimpleMove) {
				d->selectedBySimpleMove=false;
			}
			item->setSelected(!item->isSelected());
			emitSelectionChanged=true;
			break;
	
		case Key_Insert:
			//toggle selection of current item and move to the next item
			if (d->selectedBySimpleMove) {
				d->selectedBySimpleMove=false;
				if (!item->isSelected()) item->setSelected(true);
			}
			else {
				item->setSelected(!item->isSelected());
			}
			
			nextItem=item->itemBelow();
			
			if (nextItem) {
				repaintItem2=nextItem;
				visItem=nextItem;
				setCurrentItem(nextItem);
			}
			d->selectionDirection=1;
			emitSelectionChanged=true;
			break;

		case Key_Down:
			nextItem=item->itemBelow();
			if (shiftOrCtrl) {
				d->selectionDirection=1;
				d->selectedBySimpleMove=false;
				if (!d->initialFileManagerItem) {
					d->initialFileManagerItem = item;
					item->setSelected(true);
					if (nextItem) {
						nextItem->setSelected(true);
					}
					emitSelectionChanged=true;
					d->selectionRegion=1;
				}
				else {
					if (item == d->initialFileManagerItem) {
						item->setSelected(true);
						if (nextItem) {
							nextItem->setSelected(true);
						}
						emitSelectionChanged=true;
						d->selectionRegion=1;
					}
					else {
						if (d->selectionRegion == 1) {
							if (nextItem) {
								nextItem->setSelected(true);
							}
							emitSelectionChanged=true;
						}
						else if (d->selectionRegion == -1) {
							item->setSelected(false);
							emitSelectionChanged=true;
						}
					}
				}
			}
			else if ((d->selectedBySimpleMove) && (nextItem)) {
				item->setSelected(false);
				emitSelectionChanged=true;
			}
			
			if (nextItem) {
				if (d->selectedBySimpleMove) {
					nextItem->setSelected(true);
				}
				repaintItem2=nextItem;
				visItem=nextItem;
				setCurrentItem(nextItem);
			}
			break;
	
		case Key_Up:
			nextItem=item->itemAbove();
			if (shiftOrCtrl) {
				d->selectionDirection=-1;
				d->selectedBySimpleMove=false;
				if (!d->initialFileManagerItem) {
					d->initialFileManagerItem = item;
					item->setSelected(true);
					if (nextItem) {
						nextItem->setSelected(true);
					}
					emitSelectionChanged=true;
					d->selectionRegion=-1;
				}
				else {
					if (item == d->initialFileManagerItem) {
						item->setSelected(true);
						if (nextItem) {
							nextItem->setSelected(true);
						}
						emitSelectionChanged=true;
						d->selectionRegion=-1;
					}
					else {
						if (d->selectionRegion == -1) {
							if (nextItem) {
								nextItem->setSelected(true);
							}
							emitSelectionChanged=true;
						}
						else if (d->selectionRegion == 1) {
							item->setSelected(false);
							emitSelectionChanged=true;
						}
					}
				}
			}
			else if ((d->selectedBySimpleMove) && (nextItem)) {
				item->setSelected(false);
				emitSelectionChanged=true;
			}
			
			if (nextItem) {
				if (d->selectedBySimpleMove) {
					nextItem->setSelected(true);
				}
				repaintItem2=nextItem;
				visItem=nextItem;
				setCurrentItem(nextItem);
			}
			break;

		case Key_End:
			// move to the last item and toggle selection of all items in-between
			nextItem=item;
			if (d->selectedBySimpleMove) {
				item->setSelected(false);
			}
			if (shiftOrCtrl) {
				d->selectedBySimpleMove=false;
			}
			
			while (nextItem) {
				if (shiftOrCtrl) {
					if (!d->initialFileManagerItem) {
						d->initialFileManagerItem = nextItem;
						nextItem->setSelected(true);
						emitSelectionChanged=true;
						d->selectionRegion=1;
					}
					else {
						if (nextItem == d->initialFileManagerItem) {
							nextItem->setSelected(true);
							emitSelectionChanged=true;
							d->selectionRegion=1;
						}
						else {
							if (d->selectionRegion == 1) {
								nextItem->setSelected(true);
								emitSelectionChanged=true;
							}
							else if (d->selectionRegion == -1) {
								nextItem->setSelected(false);
								emitSelectionChanged=true;
							}
						}
					}
				}
				if (!nextItem->itemBelow()) {
					if (d->selectedBySimpleMove) {
						nextItem->setSelected(true);
					}
					repaintItem2=nextItem;
					visItem=nextItem;
					setCurrentItem(nextItem);
				}
				nextItem=nextItem->itemBelow();
			}
			emitSelectionChanged=true;
			break;

		case Key_Home:
			// move to the first item and toggle selection of all items in-between
			nextItem=item;
			if (d->selectedBySimpleMove) {
				item->setSelected(false);
			}
			if (shiftOrCtrl) {
				d->selectedBySimpleMove=false;
			}
			
			while (nextItem) {
				if (shiftOrCtrl) {
					if (!d->initialFileManagerItem) {
						d->initialFileManagerItem = nextItem;
						nextItem->setSelected(true);
						emitSelectionChanged=true;
						d->selectionRegion=-1;
					}
					else {
						if (nextItem == d->initialFileManagerItem) {
							nextItem->setSelected(true);
							emitSelectionChanged=true;
							d->selectionRegion=-1;
						}
						else {
							if (d->selectionRegion == -1) {
								nextItem->setSelected(true);
								emitSelectionChanged=true;
							}
							else if (d->selectionRegion == 1) {
								nextItem->setSelected(false);
								emitSelectionChanged=true;
							}
						}
					}
				}
				if (!nextItem->itemAbove()) {
					if (d->selectedBySimpleMove) {
						nextItem->setSelected(true);
					}
					repaintItem2=nextItem;
					visItem=nextItem;
					setCurrentItem(nextItem);
				}
				nextItem=nextItem->itemAbove();
			}
			emitSelectionChanged=true;
			break;
			
		case Key_Next:
			items=visibleHeight()/item->height();
			nextItem=item;
			if (d->selectedBySimpleMove) {
				item->setSelected(false);
			}
			if (shiftOrCtrl) {
				d->selectedBySimpleMove=false;
				d->selectionDirection=1;
			}
			
			for (int i=0; i<items; i++) {
				if (shiftOrCtrl) {
					if (!d->initialFileManagerItem) {
						d->initialFileManagerItem = nextItem;
						nextItem->setSelected(true);
						emitSelectionChanged=true;
						d->selectionRegion=1;
					}
					else {
						if (nextItem == d->initialFileManagerItem) {
							nextItem->setSelected(true);
							emitSelectionChanged=true;
							d->selectionRegion=1;
						}
						else {
							if (d->selectionRegion == 1) {
								nextItem->setSelected(true);
								emitSelectionChanged=true;
							}
							else if (d->selectionRegion == -1) {
								if (i==items-1) {
									nextItem->setSelected(true);
								}
								else {
									nextItem->setSelected(false);
								}
								emitSelectionChanged=true;
							}
						}
					}
				}
				// last item
				if ((i==items-1) || (!nextItem->itemBelow())) {
					if (d->selectedBySimpleMove) {
						nextItem->setSelected(true);
					}
					ensureItemVisible(nextItem);
					setCurrentItem(nextItem);
					update();
					if ((shiftOrCtrl) || (d->selectedBySimpleMove)) {
						emit selectionChanged();
					}
					return;
				}
				nextItem=nextItem->itemBelow();
			}
			break;

		case Key_Prior:
			items=visibleHeight()/item->height();
			nextItem=item;
			if (d->selectedBySimpleMove) {
				item->setSelected(false);
			}
			if (shiftOrCtrl) {
				d->selectionDirection=-1;
				d->selectedBySimpleMove=false;
			}
			
			for (int i=0; i<items; i++) {
				if (shiftOrCtrl) {
					if (!d->initialFileManagerItem) {
						d->initialFileManagerItem = nextItem;
						nextItem->setSelected(true);
						emitSelectionChanged=true;
						d->selectionRegion=-1;
					}
					else {
						if (nextItem == d->initialFileManagerItem) {
							nextItem->setSelected(true);
							emitSelectionChanged=true;
							d->selectionRegion=-1;
						}
						else {
							if (d->selectionRegion == -1) {
								nextItem->setSelected(true);
								emitSelectionChanged=true;
							}
							else if (d->selectionRegion == 1) {
								if (i==items-1) {
									nextItem->setSelected(true);
								}
								else {
									nextItem->setSelected(false);
								}
								emitSelectionChanged=true;
							}
						}
					}
				}
				// last item
				if ((i==items-1) || (!nextItem->itemAbove())) {
					if (d->selectedBySimpleMove) {
						nextItem->setSelected(true);
					}
					ensureItemVisible(nextItem);
					setCurrentItem(nextItem);
					update();
					if ((shiftOrCtrl) || (d->selectedBySimpleMove)) {
						emit selectionChanged();
					}
					return;
				}
				nextItem=nextItem->itemAbove();
			}
			break;
	
		case Key_Minus:
			if ( item->isOpen() ) {
				setOpen( item, false );
			}
			break;
		case Key_Plus:
			if (  !item->isOpen() && (item->isExpandable() || item->childCount()) ) {
				setOpen( item, true );
			}
			break;
		default:
			bool realKey = ((e->key()!=Key_Shift) && (e->key()!=Key_Control)
					&& (e->key()!=Key_Meta) && (e->key()!=Key_Alt));
		
			bool selectCurrentItem = (d->selectedBySimpleMove) && (item->isSelected());
			if (realKey && selectCurrentItem) {
				item->setSelected(false);
			}
			//this is mainly for the "goto filename beginning with pressed char" feature (aleXXX)
			TQListView::SelectionMode oldSelectionMode = selectionMode();
			setSelectionMode (TQListView::Multi);
			TQListView::keyPressEvent (e);
			setSelectionMode (oldSelectionMode);
			if (realKey && selectCurrentItem) {
				currentItem()->setSelected(true);
				emitSelectionChanged=true;
			}
			repaintItem2=currentItem();
			if (realKey) {
				visItem=currentItem();
			}
			break;
	}

	if (visItem) {
		ensureItemVisible(visItem);
	}

	TQRect ir;
	if (repaintItem1) {
		ir = ir.unite( itemRect(repaintItem1) );
	}
	if (repaintItem2) {
		ir = ir.unite( itemRect(repaintItem2) );
	}

	if ( !ir.isEmpty() ) {
		// rectangle to be repainted
		if ( ir.x() < 0 ) {
			ir.moveBy( -ir.x(), 0 );
		}
		viewport()->repaint( ir, false );
	}
	/*if (repaintItem1) {
		repaintItem1->repaint();
	}
	if (repaintItem2) {
		repaintItem2->repaint();
	}*/

	update();
	if (emitSelectionChanged) {
		emit selectionChanged();
	}
}

void TDEListView::setSelectionModeExt (SelectionModeExt mode)
{
    d->selectionMode = mode;

    switch (mode)
    {
    case Single:
    case Multi:
    case Extended:
    case NoSelection:
        setSelectionMode (static_cast<TQListView::SelectionMode>(static_cast<int>(mode)));
        break;

    case FileManager:
        setSelectionMode (TQListView::Extended);
        break;

    default:
        kdWarning () << "Warning: illegal selection mode " << int(mode) << " set!" << endl;
        break;
    }
}

TDEListView::SelectionModeExt TDEListView::selectionModeExt () const
{
  return d->selectionMode;
}

int TDEListView::itemIndex( const TQListViewItem *item ) const
{
    if ( !item )
        return -1;

    if ( item == firstChild() )
        return 0;
    else {
        TQListViewItemIterator it(firstChild());
        uint j = 0;
        for (; it.current() && it.current() != item; ++it, ++j );

        if( !it.current() )
          return -1;

        return j;
    }
}

TQListViewItem* TDEListView::itemAtIndex(int index)
{
   if (index<0)
      return 0;

   int j(0);
   for (TQListViewItemIterator it=firstChild(); it.current(); ++it)
   {
      if (j==index)
         return it.current();
      ++j;
   };
   return 0;
}


void TDEListView::emitContextMenu (TDEListView*, TQListViewItem* i)
{
  TQPoint p;

  if (i)
        p = viewport()->mapToGlobal(itemRect(i).center());
  else
        p = mapToGlobal(rect().center());

  emit contextMenu (this, i, p);
}

void TDEListView::emitContextMenu (TQListViewItem* i, const TQPoint& p, int)
{
  emit contextMenu (this, i, p);
}

void TDEListView::setAcceptDrops (bool val)
{
  TQListView::setAcceptDrops (val);
  viewport()->setAcceptDrops (val);
}

int TDEListView::dropVisualizerWidth () const
{
        return d->mDropVisualizerWidth;
}


void TDEListView::viewportPaintEvent(TQPaintEvent *e)
{
  d->paintAbove = 0;
  d->paintCurrent = 0;
  d->paintBelow = 0;
  d->painting = true;

  TQListView::viewportPaintEvent(e);

  if (d->mOldDropVisualizer.isValid() && e->rect().intersects(d->mOldDropVisualizer))
    {
      TQPainter painter(viewport());

      // This is where we actually draw the drop-visualizer
      painter.fillRect(d->mOldDropVisualizer, Dense4Pattern);
    }
  if (d->mOldDropHighlighter.isValid() && e->rect().intersects(d->mOldDropHighlighter))
    {
      TQPainter painter(viewport());

      // This is where we actually draw the drop-highlighter
      style().drawPrimitive(TQStyle::PE_FocusRect, &painter, d->mOldDropHighlighter, colorGroup(),
                            TQStyle::Style_FocusAtBorder);
    }
  d->painting = false;
}

void TDEListView::setFullWidth()
{
  setFullWidth(true);
}

void TDEListView::setFullWidth(bool fullWidth)
{
  d->fullWidth = fullWidth;
  header()->setStretchEnabled(fullWidth, columns()-1);
}

bool TDEListView::fullWidth() const
{
  return d->fullWidth;
}

int TDEListView::addColumn(const TQString& label, int width)
{
  int result = TQListView::addColumn(label, width);
  if (d->fullWidth) {
    header()->setStretchEnabled(false, columns()-2);
    header()->setStretchEnabled(true, columns()-1);
  }
  return result;
}

int TDEListView::addColumn(const TQIconSet& iconset, const TQString& label, int width)
{
  int result = TQListView::addColumn(iconset, label, width);
  if (d->fullWidth) {
    header()->setStretchEnabled(false, columns()-2);
    header()->setStretchEnabled(true, columns()-1);
  }
  return result;
}

void TDEListView::removeColumn(int index)
{
  TQListView::removeColumn(index);
  if (d->fullWidth && index == columns()) header()->setStretchEnabled(true, columns()-1);
}

void TDEListView::viewportResizeEvent(TQResizeEvent* e)
{
  TQListView::viewportResizeEvent(e);
}

const TQColor &TDEListView::alternateBackground() const
{
  return d->alternateBackground;
}

void TDEListView::setAlternateBackground(const TQColor &c)
{
  d->alternateBackground = c;
  repaint();
}

void TDEListView::setShadeSortColumn(bool shadeSortColumn)
{
  d->shadeSortColumn = shadeSortColumn;
  repaint();
}

bool TDEListView::shadeSortColumn() const
{
  return d->shadeSortColumn;
}

void TDEListView::saveLayout(TDEConfig *config, const TQString &group) const
{
  TDEConfigGroupSaver saver(config, group);
  TQStringList widths, order;

  const int colCount = columns();
  TQHeader* const thisHeader = header();
  for (int i = 0; i < colCount; ++i)
  {
    widths << TQString::number(columnWidth(i));
    order << TQString::number(thisHeader->mapToIndex(i));
  }
  config->writeEntry("ColumnWidths", widths);
  config->writeEntry("ColumnOrder", order);
  config->writeEntry("SortColumn", d->sortColumn);
  config->writeEntry("SortAscending", d->sortAscending);
}

void TDEListView::restoreLayout(TDEConfig *config, const TQString &group)
{
  TDEConfigGroupSaver saver(config, group);
  TQStringList cols = config->readListEntry("ColumnWidths");
  int i = 0;
  { // scope the iterators
    TQStringList::ConstIterator it = cols.constBegin();
    const TQStringList::ConstIterator itEnd = cols.constEnd();
    for (; it != itEnd; ++it)
      setColumnWidth(i++, (*it).toInt());
  }

  // move sections in the correct sequence: from lowest to highest index position
  // otherwise we move a section from an index, which modifies
  // all index numbers to the right of the moved one
  cols = config->readListEntry("ColumnOrder");
  const int colCount = columns();
  for (i = 0; i < colCount; ++i)   // final index positions from lowest to highest
  {
    TQStringList::ConstIterator it = cols.constBegin();
    const TQStringList::ConstIterator itEnd = cols.constEnd();

    int section = 0;
    for (; (it != itEnd) && ((*it).toInt() != i); ++it, ++section) ;

    if ( it != itEnd ) {
      // found the section to move to position i
      header()->moveSection(section, i);
    }
  }

  if (config->hasKey("SortColumn"))
    setSorting(config->readNumEntry("SortColumn"), config->readBoolEntry("SortAscending", true));
}

void TDEListView::setSorting(int column, bool ascending)
{
  TQListViewItem *selected = 0;

  if (selectionMode() == TQListView::Single) {
    selected = selectedItem();
    if (selected && !selected->isVisible())
      selected = 0;
  }
  else if (selectionMode() != TQListView::NoSelection) {
    TQListViewItem *item = firstChild();
    while (item && !selected) {
      if (item->isSelected() && item->isVisible())
	selected = item;
      item = item->itemBelow();
    }
  }

  d->sortColumn = column;
  d->sortAscending = ascending;
  TQListView::setSorting(column, ascending);

  if (selected)
    ensureItemVisible(selected);

  TQListViewItem* item = firstChild();
  while ( item ) {
    TDEListViewItem *kItem = dynamic_cast<TDEListViewItem*>(item);
    if (kItem) kItem->m_known = false;
    item = item->itemBelow();
  }
}

int TDEListView::columnSorted(void) const
{
  return d->sortColumn;
}

bool TDEListView::ascendingSort(void) const
{
  return d->sortAscending;
}

void TDEListView::takeItem(TQListViewItem *item)
{
  if(item && item == d->editor->currentItem())
    d->editor->terminate();

  TQListView::takeItem(item);
}

void TDEListView::disableAutoSelection()
{
  if ( d->disableAutoSelection )
    return;

  d->disableAutoSelection = true;
  d->autoSelect.stop();
  d->autoSelectDelay = -1;
}

void TDEListView::resetAutoSelection()
{
  if ( !d->disableAutoSelection )
    return;

  d->disableAutoSelection = false;
  d->autoSelectDelay = TDEGlobalSettings::autoSelectDelay();
}

void TDEListView::doubleClicked( TQListViewItem *item, const TQPoint &pos, int c )
{
  emit TQListView::doubleClicked( item, pos, c );
}

TDEListViewItem::TDEListViewItem(TQListView *parent)
  : TQListViewItem(parent)
{
  init();
}

TDEListViewItem::TDEListViewItem(TQListViewItem *parent)
  : TQListViewItem(parent)
{
  init();
}

TDEListViewItem::TDEListViewItem(TQListView *parent, TQListViewItem *after)
  : TQListViewItem(parent, after)
{
  init();
}

TDEListViewItem::TDEListViewItem(TQListViewItem *parent, TQListViewItem *after)
  : TQListViewItem(parent, after)
{
  init();
}

TDEListViewItem::TDEListViewItem(TQListView *parent,
    TQString label1, TQString label2, TQString label3, TQString label4,
    TQString label5, TQString label6, TQString label7, TQString label8)
  : TQListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8)
{
  init();
}

TDEListViewItem::TDEListViewItem(TQListViewItem *parent,
    TQString label1, TQString label2, TQString label3, TQString label4,
    TQString label5, TQString label6, TQString label7, TQString label8)
  : TQListViewItem(parent, label1, label2, label3, label4, label5, label6, label7, label8)
{
  init();
}

TDEListViewItem::TDEListViewItem(TQListView *parent, TQListViewItem *after,
    TQString label1, TQString label2, TQString label3, TQString label4,
    TQString label5, TQString label6, TQString label7, TQString label8)
  : TQListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8)
{
  init();
}

TDEListViewItem::TDEListViewItem(TQListViewItem *parent, TQListViewItem *after,
    TQString label1, TQString label2, TQString label3, TQString label4,
    TQString label5, TQString label6, TQString label7, TQString label8)
  : TQListViewItem(parent, after, label1, label2, label3, label4, label5, label6, label7, label8)
{
  init();
}

TDEListViewItem::~TDEListViewItem()
{
  if(listView())
    emit static_cast<TDEListView *>(listView())->itemRemoved(this);
}

void TDEListViewItem::init()
{
  m_odd = m_known = false;
  TDEListView *lv = static_cast<TDEListView *>(listView());
  setDragEnabled( dragEnabled() || lv->dragEnabled() );
  emit lv->itemAdded(this);
}

void TDEListViewItem::insertItem(TQListViewItem *item)
{
  TQListViewItem::insertItem(item);
  if(listView())
    emit static_cast<TDEListView *>(listView())->itemAdded(item);
}

void TDEListViewItem::takeItem(TQListViewItem *item)
{
  TQListViewItem::takeItem(item);
  if(listView())
    emit static_cast<TDEListView *>(listView())->itemRemoved(item);
}

const TQColor &TDEListViewItem::backgroundColor()
{
  if (isAlternate())
    return static_cast< TDEListView* >(listView())->alternateBackground();
  return listView()->viewport()->colorGroup().base();
}

TQColor TDEListViewItem::backgroundColor(int column)
{
  TDEListView* view = static_cast< TDEListView* >(listView());
  TQColor color = isAlternate() ?
                 view->alternateBackground() :
                 view->viewport()->colorGroup().base();

  // calculate a different color if the current column is sorted (only if more than 1 column)
  if ( (view->columns() > 1) && view->shadeSortColumn() && (column == view->columnSorted()) )
  {
    if ( color == TQt::black )
      color = TQColor(55, 55, 55);  // dark gray
    else
    {
      int h,s,v;
      color.hsv(&h, &s, &v);
      if ( v > 175 )
        color = color.dark(104);
      else
        color = color.light(120);
    }
  }

  return color;
}

bool TDEListViewItem::isAlternate()
{
  TDEListView* const lv = static_cast<TDEListView *>(listView());
  if (lv && lv->alternateBackground().isValid())
  {
    TDEListViewItem *above;

    TDEListView::TDEListViewPrivate* const lvD = lv->d;

    // Ok, there's some weirdness here that requires explanation as this is a
    // speed hack.  itemAbove() is a O(n) operation (though this isn't
    // immediately clear) so we want to call it as infrequently as possible --
    // especially in the case of painting a cell.
    //
    // So, in the case that we *are* painting a cell:  (1) we're assuming that
    // said painting is happening top to bottem -- this assumption is present
    // elsewhere in the implementation of this class, (2) itemBelow() is fast --
    // roughly constant time.
    //
    // Given these assumptions we can do a mixture of caching and telling the
    // next item that the when that item is the current item that the now
    // current item will be the item above it.
    //
    // Ideally this will make checking to see if the item above the current item
    // is the alternate color a constant time operation rather than 0(n).

    if (lvD->painting) {
      if (lvD->paintCurrent != this)
      {
        lvD->paintAbove = lvD->paintBelow == this ? lvD->paintCurrent : itemAbove();
        lvD->paintCurrent = this;
        lvD->paintBelow = itemBelow();
      }

      above = dynamic_cast<TDEListViewItem *>(lvD->paintAbove);
    }
    else
    {
      above = dynamic_cast<TDEListViewItem *>(itemAbove());
    }

    m_known = above ? above->m_known : true;
    if (m_known)
    {
       m_odd = above ? !above->m_odd : false;
    }
    else
    {
       TDEListViewItem *item;
       bool previous = true;
       if (parent())
       {
          item = dynamic_cast<TDEListViewItem *>(parent());
          if (item)
             previous = item->m_odd;
          item = dynamic_cast<TDEListViewItem *>(parent()->firstChild());
       }
       else
       {
          item = dynamic_cast<TDEListViewItem *>(lv->firstChild());
       }

       while(item)
       {
          previous = !previous;
          item->m_odd = previous;
          item->m_known = true;
          item = dynamic_cast<TDEListViewItem *>(item->nextSibling());
       }
    }
    return m_odd;
  }
  return false;
}

void TDEListViewItem::paintCell(TQPainter *p, const TQColorGroup &cg, int column, int width, int alignment)
{
  TQColorGroup _cg = cg;
  TQListView* lv = listView();
  const TQPixmap *pm = lv->viewport()->backgroundPixmap();

  if (pm && !pm->isNull())
  {
    _cg.setBrush(TQColorGroup::Base, TQBrush(backgroundColor(column), *pm));
    TQPoint o = p->brushOrigin();
    p->setBrushOrigin( o.x()-lv->contentsX(), o.y()-lv->contentsY() );
  }
  else
  {
    _cg.setColor((lv->viewport()->backgroundMode() == TQt::FixedColor) ?
                 TQColorGroup::Background : TQColorGroup::Base,
                 backgroundColor(column));
  }
  TQListViewItem::paintCell(p, _cg, column, width, alignment);
}

/*!
    If \a select is TRUE, all the items get selected; otherwise all
    the items get unselected. This only works in the selection modes \c
    Multi and \c Extended. In \c Single and \c NoSelection mode the
    selection of the current item is just set to \a select.
*/

void TDEListView::selectAll( bool select )
{
    if ( ((SelectionModeExt)selectionMode() == Multi) || ((SelectionModeExt)selectionMode() == Extended) ) {
	bool b = signalsBlocked();
	blockSignals( TRUE );
	bool anything = FALSE;
	TQListViewItemIterator it( this );
	while ( it.current() ) {
	    TQListViewItem *i = it.current();
	    if ( select == TRUE ) {
		if ( (bool)i->isVisible() == TRUE ) {
		    i->setSelected( TRUE );
		    anything = TRUE;
		}
		if ( (bool)i->isVisible() == FALSE ) {
		    i->setSelected( FALSE );
		    anything = TRUE;
		}
	    }
	    else {
		if ( (bool)i->isSelected() != select ) {
		    i->setSelected( select );
		    anything = TRUE;
		}
	    }
	    ++it;
	}
	blockSignals( b );
	if ( anything ) {
	    emit selectionChanged();
// 	    d->useDoubleBuffer = TRUE;
	    triggerUpdate();
	}
    } else if ( currentItem() ) {
	TQListViewItem * i = currentItem();
	setSelected( i, select );
    }
}

void TDEListView::setUseSmallExecuteArea(bool enable)
{
    d->useSmallExecuteArea = enable;
}

bool TDEListView::useSmallExecuteArea() const
{
    return d->useSmallExecuteArea;
}

void TDEListView::setRenameSettings(const TDEListViewRenameSettings &renSett)
{
  d->editor->setRenameSettings(renSett);
}

void TDEListView::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdelistview.moc"
#include "tdelistviewlineedit.moc"
