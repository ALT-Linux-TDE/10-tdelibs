/*
    This file is part of the KDE libraries
    Copyright (C) 1997 Tim D. Gilman (tdgilman@best.org)
              (C) 1998-2001 Mirko Boehm (mirko@kde.org)
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

/////////////////// KDateTable widget class //////////////////////
//
// Copyright (C) 1997 Tim D. Gilman
//           (C) 1998-2001 Mirko Boehm
// Written using Qt (http://www.troll.no) for the
// KDE project (http://www.kde.org)
//
// This is a support class for the KDatePicker class.  It just
// draws the calender table without titles, but could theoretically
// be used as a standalone.
//
// When a date is selected by the user, it emits a signal:
//      dateSelected(TQDate)

#include <tdeconfig.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <tdeapplication.h>
#include <tdeaccel.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <knotifyclient.h>
#include <kcalendarsystem.h>
#include <tdeshortcut.h>
#include <tdestdaccel.h>
#include "kdatepicker.h"
#include "kdatetbl.h"
#include "tdepopupmenu.h"
#include <tqdatetime.h>
#include <tqguardedptr.h>
#include <tqstring.h>
#include <tqpen.h>
#include <tqpainter.h>
#include <tqdialog.h>
#include <tqdict.h>
#include <assert.h>


class KDateTable::KDateTablePrivate
{
public:
   KDateTablePrivate()
   {
      popupMenuEnabled=false;
      useCustomColors=false;
   }

   ~KDateTablePrivate()
   {
   }

   bool popupMenuEnabled;
   bool useCustomColors;

   struct DatePaintingMode
   {
     TQColor fgColor;
     TQColor bgColor;
     BackgroundMode bgMode;
   };
   TQDict <DatePaintingMode> customPaintingModes;

};


KDateValidator::KDateValidator(TQWidget* parent, const char* name)
    : TQValidator(parent, name)
{
}

TQValidator::State
KDateValidator::validate(TQString& text, int&) const
{
  TQDate temp;
  // ----- everything is tested in date():
  return date(text, temp);
}

TQValidator::State
KDateValidator::date(const TQString& text, TQDate& d) const
{
  TQDate tmp = TDEGlobal::locale()->readDate(text);
  if (!tmp.isNull())
    {
      d = tmp;
      return Acceptable;
    } else
      return Valid;
}

void
KDateValidator::fixup( TQString& ) const
{

}

KDateTable::KDateTable(TQWidget *parent, TQDate date_, const char* name, WFlags f)
  : TQGridView(parent, name, (f | TQt::WNoAutoErase))
{
  d = new KDateTablePrivate;
  setFontSize(10);
  if(!date_.isValid())
    {
      kdDebug() << "KDateTable ctor: WARNING: Given date is invalid, using current date." << endl;
      date_=TQDate::currentDate();
    }
  setFocusPolicy( TQWidget::StrongFocus );
  setNumRows(7); // 6 weeks max + headline
  setNumCols(7); // 7 days a week
  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);
  viewport()->setEraseColor(TDEGlobalSettings::baseColor());
  setDate(date_); // this initializes firstday, numdays, numDaysPrevMonth

  initAccels();
}

KDateTable::KDateTable(TQWidget *parent, const char* name, WFlags f)
  : TQGridView(parent, name, (f | TQt::WNoAutoErase))
{
  d = new KDateTablePrivate;
  setFontSize(10);
  setFocusPolicy( TQWidget::StrongFocus );
  setNumRows(7); // 6 weeks max + headline
  setNumCols(7); // 7 days a week
  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);
  viewport()->setEraseColor(TDEGlobalSettings::baseColor());
  setDate(TQDate::currentDate()); // this initializes firstday, numdays, numDaysPrevMonth
  initAccels();
}

KDateTable::~KDateTable()
{
  delete d;
}

void KDateTable::initAccels()
{
  TDEAccel* accel = new TDEAccel(this, "date table accel");
  accel->insert(TDEStdAccel::Next, this, TQ_SLOT(nextMonth()));
  accel->insert(TDEStdAccel::Prior, this, TQ_SLOT(previousMonth()));
  accel->insert(TDEStdAccel::Home, this, TQ_SLOT(beginningOfMonth()));
  accel->insert(TDEStdAccel::End, this, TQ_SLOT(endOfMonth()));
  accel->insert(TDEStdAccel::BeginningOfLine, this, TQ_SLOT(beginningOfWeek()));
  accel->insert(TDEStdAccel::EndOfLine, this, TQ_SLOT(endOfWeek()));
  accel->readSettings();
}

int KDateTable::posFromDate( const TQDate &dt )
{
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();
  const int firstWeekDay = TDEGlobal::locale()->weekStartDay();
  int pos = calendar->day( dt );
  int offset = (firstday - firstWeekDay + 7) % 7;
  // make sure at least one day of the previous month is visible.
  // adjust this <1 if more days should be forced visible:
  if ( offset < 1 ) offset += 7;
  return pos + offset;
}

TQDate KDateTable::dateFromPos( int pos )
{
  TQDate pCellDate;
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();
  calendar->setYMD(pCellDate, calendar->year(date), calendar->month(date), 1);

  int firstWeekDay = TDEGlobal::locale()->weekStartDay();
  int offset = (firstday - firstWeekDay + 7) % 7;
  // make sure at least one day of the previous month is visible.
  // adjust this <1 if more days should be forced visible:
  if ( offset < 1 ) offset += 7;
  pCellDate = calendar->addDays( pCellDate, pos - offset );
  return pCellDate;
}

void
KDateTable::paintEmptyArea(TQPainter *paint, int, int, int, int)
{
  // Erase the unused areas on the right and bottom.
  TQRect unusedRight = frameRect();
  unusedRight.setLeft(gridSize().width());

  TQRect unusedBottom = frameRect();
  unusedBottom.setTop(gridSize().height());

  paint->eraseRect(unusedRight);
  paint->eraseRect(unusedBottom);
}

void
KDateTable::paintCell(TQPainter *painter, int row, int col)
{
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();

  TQRect rect;
  TQString text;
  TQPen pen;
  int w=cellWidth();
  int h=cellHeight();
  TQFont font=TDEGlobalSettings::generalFont();
  // -----

  if(row == 0)
    { // we are drawing the headline
      font.setBold(true);
      painter->setFont(font);
      bool normalday = true;
      int firstWeekDay = TDEGlobal::locale()->weekStartDay();
      int daynum = ( col+firstWeekDay < 8 ) ? col+firstWeekDay :
                                              col+firstWeekDay-7;
      if ( daynum == calendar->weekDayOfPray() ||
         ( daynum == 6 && calendar->calendarName() == "gregorian" ) )
          normalday=false;

			TQBrush brushInvertTitle(colorGroup().base());
			TQColor titleColor(isEnabled()?( TDEGlobalSettings::activeTitleColor() ):( TDEGlobalSettings::inactiveTitleColor() ) );
			TQColor textColor(isEnabled()?( TDEGlobalSettings::activeTextColor() ):( TDEGlobalSettings::inactiveTextColor() ) );
      if (!normalday)
        {
          painter->setPen(textColor);
          painter->setBrush(textColor);
          painter->drawRect(0, 0, w, h);
          painter->setPen(titleColor);
        } else {
          painter->setPen(titleColor);
          painter->setBrush(titleColor);
          painter->drawRect(0, 0, w, h);
          painter->setPen(textColor);
        }
      painter->drawText(0, 0, w, h-1, AlignCenter,
                        calendar->weekDayName(daynum, true), -1, &rect);
      painter->setPen(colorGroup().text());
      painter->moveTo(0, h-1);
      painter->lineTo(w-1, h-1);
      // ----- draw the weekday:
    } else {
      bool paintRect=true;
      painter->setFont(font);
      int pos=7*(row-1)+col;

      TQDate pCellDate = dateFromPos( pos );
      // First day of month
      text = calendar->dayString(pCellDate, true);
      if( calendar->month(pCellDate) != calendar->month(date) )
        { // we are either
          // ° painting a day of the previous month or
          // ° painting a day of the following month
          // TODO: don't hardcode gray here! Use a color with less contrast to the background than normal text.
          painter->setPen( colorGroup().mid() );
//          painter->setPen(gray);
        } else { // paint a day of the current month
          if ( d->useCustomColors )
          {
            KDateTablePrivate::DatePaintingMode *mode=d->customPaintingModes[pCellDate.toString()];
            if (mode)
            {
              if (mode->bgMode != NoBgMode)
              {
                TQBrush oldbrush=painter->brush();
                painter->setBrush( mode->bgColor );
                switch(mode->bgMode)
                {
                  case(CircleMode) : painter->drawEllipse(0,0,w,h);break;
                  case(RectangleMode) : painter->drawRect(0,0,w,h);break;
                  case(NoBgMode) : // Should never be here, but just to get one
                                   // less warning when compiling
                  default: break;
                }
                painter->setBrush( oldbrush );
                paintRect=false;
              }
              painter->setPen( mode->fgColor );
            } else
              painter->setPen(colorGroup().text());
          } else //if ( firstWeekDay < 4 ) // <- this doesn' make sense at all!
          painter->setPen(colorGroup().text());
        }

      pen=painter->pen();
      int firstWeekDay=TDEGlobal::locale()->weekStartDay();
      int offset=firstday-firstWeekDay;
      if(offset<1)
        offset+=7;
      int d = calendar->day(date);
           if( (offset+d) == (pos+1))
        {
           // draw the currently selected date
	   if (isEnabled())
	   {
           painter->setPen(colorGroup().highlight());
           painter->setBrush(colorGroup().highlight());
	   }
	   else
	   {
	   painter->setPen(colorGroup().text());
           painter->setBrush(colorGroup().text());
	   }
           pen=TQPen(colorGroup().highlightedText());
        } else {
          painter->setBrush(paletteBackgroundColor());
          painter->setPen(paletteBackgroundColor());
//          painter->setBrush(colorGroup().base());
//          painter->setPen(colorGroup().base());
        }

      if ( pCellDate == TQDate::currentDate() )
      {
         painter->setPen(colorGroup().text());
      }

      if ( paintRect ) painter->drawRect(0, 0, w, h);
      painter->setPen(pen);
      painter->drawText(0, 0, w, h, AlignCenter, text, -1, &rect);
    }
  if(rect.width()>maxCell.width()) maxCell.setWidth(rect.width());
  if(rect.height()>maxCell.height()) maxCell.setHeight(rect.height());
}

void KDateTable::nextMonth()
{
    const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();
  setDate(calendar->addMonths( date, 1 ));
}

void KDateTable::previousMonth()
{
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();
  setDate(calendar->addMonths( date, -1 ));
}

void KDateTable::beginningOfMonth()
{
  setDate(date.addDays(1 - date.day()));
}

void KDateTable::endOfMonth()
{
  setDate(date.addDays(date.daysInMonth() - date.day()));
}

void KDateTable::beginningOfWeek()
{
  setDate(date.addDays(1 - date.dayOfWeek()));
}

void KDateTable::endOfWeek()
{
  setDate(date.addDays(7 - date.dayOfWeek()));
}

void
KDateTable::keyPressEvent( TQKeyEvent *e )
{
    switch( e->key() ) {
    case Key_Up:
            setDate(date.addDays(-7));
        break;
    case Key_Down:
            setDate(date.addDays(7));
        break;
    case Key_Left:
            setDate(date.addDays(-1));
        break;
    case Key_Right:
            setDate(date.addDays(1));
        break;
    case Key_Minus:
        setDate(date.addDays(-1));
	break;
    case Key_Plus:
        setDate(date.addDays(1));
	break;
    case Key_N:
        setDate(TQDate::currentDate());
	break;
    case Key_Return:
    case Key_Enter:
        emit tableClicked();
        break;
    case Key_Control:
    case Key_Alt:
    case Key_Meta:
    case Key_Shift:
      // Don't beep for modifiers
      break;
    default:
      if (!e->state()) { // hm
    KNotifyClient::beep();
}
    }
}

void
KDateTable::viewportResizeEvent(TQResizeEvent * e)
{
  TQGridView::viewportResizeEvent(e);

  setCellWidth(viewport()->width()/7);
  setCellHeight(viewport()->height()/7);
}

void
KDateTable::setFontSize(int size)
{
  int count;
  TQFontMetrics metrics(fontMetrics());
  TQRect rect;
  // ----- store rectangles:
  fontsize=size;
  // ----- find largest day name:
  maxCell.setWidth(0);
  maxCell.setHeight(0);
  for(count=0; count<7; ++count)
    {
      rect=metrics.boundingRect(TDEGlobal::locale()->calendar()
                                ->weekDayName(count+1, true));
      maxCell.setWidth(TQMAX(maxCell.width(), rect.width()));
      maxCell.setHeight(TQMAX(maxCell.height(), rect.height()));
    }
  // ----- compare with a real wide number and add some space:
  rect=metrics.boundingRect(TQString::fromLatin1("88"));
  maxCell.setWidth(TQMAX(maxCell.width()+2, rect.width()));
  maxCell.setHeight(TQMAX(maxCell.height()+4, rect.height()));
}

void
KDateTable::wheelEvent ( TQWheelEvent * e )
{
    setDate(date.addMonths( -(int)(e->delta()/120)) );
    e->accept();
}

void
KDateTable::contentsMousePressEvent(TQMouseEvent *e)
{

  if(e->type()!=TQEvent::MouseButtonPress)
    { // the KDatePicker only reacts on mouse press events:
      return;
    }
  if(!isEnabled())
    {
      KNotifyClient::beep();
      return;
    }

  // -----
  int row, col, pos, temp;
  TQPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=rowAt(mouseCoord.y());
  col=columnAt(mouseCoord.x());
  if(row<1 || col<0)
    { // the user clicked on the frame of the table
      return;
    }

  // Rows and columns are zero indexed.  The (row - 1) below is to avoid counting
  // the row with the days of the week in the calculation.

  // old selected date:
  temp = posFromDate( date );
  // new position and date
  pos = (7 * (row - 1)) + col;
  TQDate clickedDate = dateFromPos( pos );

  // set the new date. If it is in the previous or next month, the month will
  // automatically be changed, no need to do that manually...
  setDate( clickedDate );

  // call updateCell on the old and new selection. If setDate switched to a different
  // month, these cells will be painted twice, but that's no problem.
  updateCell( temp/7+1, temp%7 );
  updateCell( row, col );

  emit tableClicked();

  if (  e->button() == TQt::RightButton && d->popupMenuEnabled )
  {
        TDEPopupMenu *menu = new TDEPopupMenu();
        menu->insertTitle( TDEGlobal::locale()->formatDate(clickedDate) );
        emit aboutToShowContextMenu( menu, clickedDate );
        menu->popup(e->globalPos());
  }
}

bool
KDateTable::setDate(const TQDate& date_)
{
  bool changed=false;
  TQDate temp;
  // -----
  if(!date_.isValid())
    {
      kdDebug() << "KDateTable::setDate: refusing to set invalid date." << endl;
      return false;
    }
  if(date!=date_)
    {
      emit(dateChanged(date, date_));
      date=date_;
      emit(dateChanged(date));
      changed=true;
    }
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();

  calendar->setYMD(temp, calendar->year(date), calendar->month(date), 1);
  //temp.setYMD(date.year(), date.month(), 1);
  //kdDebug() << "firstDayInWeek: " << temp.toString() << endl;
  firstday=temp.dayOfWeek();
  numdays=calendar->daysInMonth(date);

  temp = calendar->addMonths(temp, -1);
  numDaysPrevMonth=calendar->daysInMonth(temp);
  if(changed)
    {
      repaintContents(false);
    }
  return true;
}

const TQDate&
KDateTable::getDate() const
{
  return date;
}

// what are those repaintContents() good for? (pfeiffer)
void KDateTable::focusInEvent( TQFocusEvent *e )
{
//    repaintContents(false);
    TQGridView::focusInEvent( e );
}

void KDateTable::focusOutEvent( TQFocusEvent *e )
{
//    repaintContents(false);
    TQGridView::focusOutEvent( e );
}

TQSize
KDateTable::sizeHint() const
{
  if(maxCell.height()>0 && maxCell.width()>0)
    {
      return TQSize(maxCell.width()*numCols()+2*frameWidth(),
             (maxCell.height()+2)*numRows()+2*frameWidth());
    } else {
      kdDebug() << "KDateTable::sizeHint: obscure failure - " << endl;
      return TQSize(-1, -1);
    }
}

void KDateTable::setPopupMenuEnabled( bool enable )
{
   d->popupMenuEnabled=enable;
}

bool KDateTable::popupMenuEnabled() const
{
   return d->popupMenuEnabled;
}

void KDateTable::setCustomDatePainting(const TQDate &date, const TQColor &fgColor, BackgroundMode bgMode, const TQColor &bgColor)
{
    if (!fgColor.isValid())
    {
        unsetCustomDatePainting( date );
        return;
    }

    KDateTablePrivate::DatePaintingMode *mode=new KDateTablePrivate::DatePaintingMode;
    mode->bgMode=bgMode;
    mode->fgColor=fgColor;
    mode->bgColor=bgColor;

    d->customPaintingModes.replace( date.toString(), mode );
    d->useCustomColors=true;
    update();
}

void KDateTable::unsetCustomDatePainting( const TQDate &date )
{
    d->customPaintingModes.remove( date.toString() );
}

KDateInternalWeekSelector::KDateInternalWeekSelector
(TQWidget* parent, const char* name)
  : TQLineEdit(parent, name),
    val(new TQIntValidator(this)),
    result(0)
{
  TQFont font;
  // -----
  font=TDEGlobalSettings::generalFont();
  setFont(font);
  setFrameStyle(TQFrame::NoFrame);
  setValidator(val);
  connect(this, TQ_SIGNAL(returnPressed()), TQ_SLOT(weekEnteredSlot()));
}

void
KDateInternalWeekSelector::weekEnteredSlot()
{
  bool ok;
  int week;
  // ----- check if this is a valid week:
  week=text().toInt(&ok);
  if(!ok)
    {
      KNotifyClient::beep();
      emit(closeMe(0));
      return;
    }
  result=week;
  emit(closeMe(1));
}

int
KDateInternalWeekSelector::getWeek()
{
  return result;
}

void
KDateInternalWeekSelector::setWeek(int week)
{
  TQString temp;
  // -----
  temp.setNum(week);
  setText(temp);
}

void
KDateInternalWeekSelector::setMaxWeek(int max)
{
  val->setRange(1, max);
}

// ### CFM To avoid binary incompatibility.
//     In future releases, remove this and replace by  a QDate
//     private member, needed in KDateInternalMonthPicker::paintCell
class KDateInternalMonthPicker::KDateInternalMonthPrivate {
public:
        KDateInternalMonthPrivate (int y, int m, int d)
        : year(y), month(m), day(d)
        {}
        int year;
        int month;
        int day;
};

KDateInternalMonthPicker::~KDateInternalMonthPicker() {
   delete d;
}

KDateInternalMonthPicker::KDateInternalMonthPicker
(const TQDate & date, TQWidget* parent, const char* name)
  : TQGridView(parent, name),
    result(0) // invalid
{
  TQRect rect;
  TQFont font;
  // -----
  activeCol = -1;
  activeRow = -1;
  font=TDEGlobalSettings::generalFont();
  setFont(font);
  setHScrollBarMode(AlwaysOff);
  setVScrollBarMode(AlwaysOff);
  setFrameStyle(TQFrame::NoFrame);
  setNumCols(3);
  d = new KDateInternalMonthPrivate(date.year(), date.month(), date.day());
  // For monthsInYear != 12
  setNumRows( (TDEGlobal::locale()->calendar()->monthsInYear(date) + 2) / 3);
  // enable to find drawing failures:
  // setTableFlags(Tbl_clipCellPainting);
  viewport()->setEraseColor(TDEGlobalSettings::baseColor()); // for consistency with the datepicker
  // ----- find the preferred size
  //       (this is slow, possibly, but unfortunately it is needed here):
  TQFontMetrics metrics(font);
  for(int i = 1; ; ++i)
    {
      TQString str = TDEGlobal::locale()->calendar()->monthName(i,
         TDEGlobal::locale()->calendar()->year(date), false);
      if (str.isNull()) break;
      rect=metrics.boundingRect(str);
      if(max.width()<rect.width()) max.setWidth(rect.width());
      if(max.height()<rect.height()) max.setHeight(rect.height());
    }
}

TQSize
KDateInternalMonthPicker::sizeHint() const
{
  return TQSize((max.width()+6)*numCols()+2*frameWidth(),
         (max.height()+6)*numRows()+2*frameWidth());
}

int
KDateInternalMonthPicker::getResult() const
{
  return result;
}

void
KDateInternalMonthPicker::setupPainter(TQPainter *p)
{
  p->setPen(TDEGlobalSettings::textColor());
}

void
KDateInternalMonthPicker::viewportResizeEvent(TQResizeEvent*)
{
  setCellWidth(width() / numCols());
  setCellHeight(height() / numRows());
}

void
KDateInternalMonthPicker::paintCell(TQPainter* painter, int row, int col)
{
  int index;
  TQString text;
  // ----- find the number of the cell:
  index=3*row+col+1;
  text=TDEGlobal::locale()->calendar()->monthName(index,
    TDEGlobal::locale()->calendar()->year(TQDate(d->year, d->month,
    d->day)), false);
  painter->drawText(0, 0, cellWidth(), cellHeight(), AlignCenter, text);
  if ( activeCol == col && activeRow == row )
      painter->drawRect( 0, 0, cellWidth(), cellHeight() );
}

void
KDateInternalMonthPicker::contentsMousePressEvent(TQMouseEvent *e)
{
  if(!isEnabled() || e->button() != TQt::LeftButton)
    {
      KNotifyClient::beep();
      return;
    }
  // -----
  int row, col;
  TQPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=rowAt(mouseCoord.y());
  col=columnAt(mouseCoord.x());

  if(row<0 || col<0)
    { // the user clicked on the frame of the table
      activeCol = -1;
      activeRow = -1;
    } else {
      activeCol = col;
      activeRow = row;
      updateCell( row, col /*, false */ );
  }
}

void
KDateInternalMonthPicker::contentsMouseMoveEvent(TQMouseEvent *e)
{
  if (e->state() & TQt::LeftButton)
    {
      int row, col;
      TQPoint mouseCoord;
      // -----
      mouseCoord = e->pos();
      row=rowAt(mouseCoord.y());
      col=columnAt(mouseCoord.x());
      int tmpRow = -1, tmpCol = -1;
      if(row<0 || col<0)
        { // the user clicked on the frame of the table
          if ( activeCol > -1 )
            {
              tmpRow = activeRow;
              tmpCol = activeCol;
            }
          activeCol = -1;
          activeRow = -1;
        } else {
          bool differentCell = (activeRow != row || activeCol != col);
          if ( activeCol > -1 && differentCell)
            {
              tmpRow = activeRow;
              tmpCol = activeCol;
            }
          if ( differentCell)
            {
              activeRow = row;
              activeCol = col;
              updateCell( row, col /*, false */ ); // mark the new active cell
            }
        }
      if ( tmpRow > -1 ) // repaint the former active cell
          updateCell( tmpRow, tmpCol /*, true */ );
    }
}

void
KDateInternalMonthPicker::contentsMouseReleaseEvent(TQMouseEvent *e)
{
  if(!isEnabled())
    {
      return;
    }
  // -----
  int row, col, pos;
  TQPoint mouseCoord;
  // -----
  mouseCoord = e->pos();
  row=rowAt(mouseCoord.y());
  col=columnAt(mouseCoord.x());
  if(row<0 || col<0)
    { // the user clicked on the frame of the table
      emit(closeMe(0));
      return;
    }

  pos=3*row+col+1;
  result=pos;
  emit(closeMe(1));
}



KDateInternalYearSelector::KDateInternalYearSelector
(TQWidget* parent, const char* name)
  : TQLineEdit(parent, name),
    val(new TQIntValidator(this)),
    result(0)
{
  TQFont font;
  // -----
  font=TDEGlobalSettings::generalFont();
  setFont(font);
  setFrameStyle(TQFrame::NoFrame);
  // we have to respect the limits of TQDate here, I fear:
  val->setRange(0, 8000);
  setValidator(val);
  connect(this, TQ_SIGNAL(returnPressed()), TQ_SLOT(yearEnteredSlot()));
}

void
KDateInternalYearSelector::yearEnteredSlot()
{
  bool ok;
  int year;
  TQDate date;
  // ----- check if this is a valid year:
  year=text().toInt(&ok);
  if(!ok)
    {
      KNotifyClient::beep();
      emit(closeMe(0));
      return;
    }
  //date.setYMD(year, 1, 1);
  TDEGlobal::locale()->calendar()->setYMD(date, year, 1, 1);
  if(!date.isValid())
    {
      KNotifyClient::beep();
      emit(closeMe(0));
      return;
    }
  result=year;
  emit(closeMe(1));
}

int
KDateInternalYearSelector::getYear()
{
  return result;
}

void
KDateInternalYearSelector::setYear(int year)
{
  TQString temp;
  // -----
  temp.setNum(year);
  setText(temp);
}

class TDEPopupFrame::TDEPopupFramePrivate
{
    public:
        TDEPopupFramePrivate() : exec(false) {}

        bool exec;
};

TDEPopupFrame::TDEPopupFrame(TQWidget* parent, const char*  name)
  : TQFrame(parent, name, (WFlags)WType_Popup),
    result(0), // rejected
    main(0),
    d(new TDEPopupFramePrivate)
{
  setFrameStyle(TQFrame::Box|TQFrame::Raised);
  setMidLineWidth(2);
}

TDEPopupFrame::~TDEPopupFrame()
{
    delete d;
}

void
TDEPopupFrame::keyPressEvent(TQKeyEvent* e)
{
  if(e->key()==Key_Escape)
    {
      result=0; // rejected
      d->exec = false;
      tqApp->exit_loop();
    }
}

void
TDEPopupFrame::close(int r)
{
  result=r;
  d->exec = false;
  tqApp->exit_loop();
}

void
TDEPopupFrame::hide()
{
    TQFrame::hide();
    if (d->exec)
    {
        d->exec = false;
        tqApp->exit_loop();
    }
}

void
TDEPopupFrame::setMainWidget(TQWidget* m)
{
  main=m;
  if(main)
    {
      resize(main->width()+2*frameWidth(), main->height()+2*frameWidth());
    }
}

void
TDEPopupFrame::resizeEvent(TQResizeEvent*)
{
  if(main)
    {
      main->setGeometry(frameWidth(), frameWidth(),
          width()-2*frameWidth(), height()-2*frameWidth());
    }
}

void
TDEPopupFrame::popup(const TQPoint &pos)
{
  // Make sure the whole popup is visible.
  TQRect d = TDEGlobalSettings::desktopGeometry(pos);

  int x = pos.x();
  int y = pos.y();
  int w = width();
  int h = height();
  if (x+w > d.x()+d.width())
    x = d.width() - w;
  if (y+h > d.y()+d.height())
    y = d.height() - h;
  if (x < d.x())
    x = 0;
  if (y < d.y())
    y = 0;

  // Pop the thingy up.
  move(x, y);
  show();
}

int
TDEPopupFrame::exec(TQPoint pos)
{
  popup(pos);
  repaint();
  d->exec = true;
  const TQGuardedPtr<TQObject> that = this;
  tqApp->enter_loop();
  if ( !that )
    return TQDialog::Rejected;
  hide();
  return result;
}

int
TDEPopupFrame::exec(int x, int y)
{
  return exec(TQPoint(x, y));
}

void TDEPopupFrame::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KDateTable::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kdatetbl.moc"
