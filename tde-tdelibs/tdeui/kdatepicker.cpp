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

#include <tqlayout.h>
#include <tqframe.h>
#include <tqpainter.h>
#include <tqdialog.h>
#include <tqstyle.h>
#include <tqtoolbutton.h>
#include <tqcombobox.h>
#include <tqtooltip.h>
#include <tqfont.h>
#include <tqvalidator.h>
#include <tqpopupmenu.h>
#include <tqtimer.h>

#include "kdatepicker.h"
#include <tdeglobal.h>
#include <tdeapplication.h>
#include <kdialog.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <tdetoolbar.h>
#include <klineedit.h>
#include <kdebug.h>
#include <knotifyclient.h>
#include <kcalendarsystem.h>

#include "kdatetbl.h"
#include "kdatepicker.moc"

// Week numbers are defined by ISO 8601
// See http://www.merlyn.demon.co.uk/weekinfo.htm for details

class KDatePicker::KDatePickerPrivate
{
public:
    KDatePickerPrivate() : closeButton(0L), selectWeek(0L), todayButton(0), navigationLayout(0) {}

    void fillWeeksCombo(const TQDate &date);

    TQToolButton *closeButton;
    TQComboBox *selectWeek;
    TQToolButton *todayButton;
    TQBoxLayout *navigationLayout;
};

void KDatePicker::fillWeeksCombo(const TQDate &date)
{
  // every year can have a different number of weeks
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();

  // it could be that we had 53,1..52 and now 1..53 which is the same number but different
  // so always fill with new values

  d->selectWeek->clear();

  // We show all week numbers for all weeks between first day of year to last day of year
  // This of course can be a list like 53,1,2..52

  TQDate day;
  int year = calendar->year(date);
  calendar->setYMD(day, year, 1, 1);
  int lastMonth = calendar->monthsInYear(day);
  TQDate lastDay, firstDayOfLastMonth;
  calendar->setYMD(firstDayOfLastMonth, year, lastMonth, 1);
  calendar->setYMD(lastDay, year, lastMonth, calendar->daysInMonth(firstDayOfLastMonth));

  for (; day <= lastDay ; day = calendar->addDays(day, 7 /*calendar->daysOfWeek()*/) )
  {
    TQString week = i18n("Week %1").arg(calendar->weekNumber(day, &year));
    if ( year != calendar->year(day) ) week += "*";  // show that this is a week from a different year
    d->selectWeek->insertItem(week);

    // make sure that the week of the lastDay is always inserted: in Chinese calendar
    // system, this is not always the case
    if(day < lastDay && day.daysTo(lastDay) < 7 && calendar->weekNumber(day) != calendar->weekNumber(lastDay))
      day = lastDay.addDays(-7);
  }
}

KDatePicker::KDatePicker(TQWidget *parent, TQDate dt, const char *name)
  : TQFrame(parent,name)
{
  init( dt );
}

KDatePicker::KDatePicker(TQWidget *parent, TQDate dt, const char *name, WFlags f)
  : TQFrame(parent,name, f)
{
  init( dt );
}

KDatePicker::KDatePicker( TQWidget *parent, const char *name )
  : TQFrame(parent,name)
{
  init( TQDate::currentDate() );
}

void KDatePicker::init( const TQDate &dt )
{
  d = new KDatePickerPrivate();

  TQBoxLayout * topLayout = new TQVBoxLayout(this);

  d->navigationLayout = new TQHBoxLayout(topLayout);
  d->navigationLayout->addStretch();
  yearBackward = new TQToolButton(this);
  yearBackward->setAutoRaise(true);
  d->navigationLayout->addWidget(yearBackward);
  monthBackward = new TQToolButton(this);
  monthBackward ->setAutoRaise(true);
  d->navigationLayout->addWidget(monthBackward);
  d->navigationLayout->addSpacing(KDialog::spacingHint());

  selectMonth = new TQToolButton(this);
  selectMonth ->setAutoRaise(true);
  d->navigationLayout->addWidget(selectMonth);
  selectYear = new TQToolButton(this);
  selectYear->setToggleButton(true);
  selectYear->setAutoRaise(true);
  d->navigationLayout->addWidget(selectYear);
  d->navigationLayout->addSpacing(KDialog::spacingHint());

  monthForward = new TQToolButton(this);
  monthForward ->setAutoRaise(true);
  d->navigationLayout->addWidget(monthForward);
  yearForward = new TQToolButton(this);
  yearForward ->setAutoRaise(true);
  d->navigationLayout->addWidget(yearForward);
  d->navigationLayout->addStretch();

  line = new KLineEdit(this);
  val = new KDateValidator(this);
  table = new KDateTable(this);
  fontsize = TDEGlobalSettings::generalFont().pointSize();
  if (fontsize == -1)
     fontsize = TQFontInfo(TDEGlobalSettings::generalFont()).pointSize();

  fontsize++; // Make a little bigger

  d->selectWeek = new TQComboBox(false, this);  // read only week selection
  d->todayButton = new TQToolButton(this);
  d->todayButton->setIconSet(SmallIconSet("today"));

  TQToolTip::add(yearForward, i18n("Next year"));
  TQToolTip::add(yearBackward, i18n("Previous year"));
  TQToolTip::add(monthForward, i18n("Next month"));
  TQToolTip::add(monthBackward, i18n("Previous month"));
  TQToolTip::add(d->selectWeek, i18n("Select a week"));
  TQToolTip::add(selectMonth, i18n("Select a month"));
  TQToolTip::add(selectYear, i18n("Select a year"));
  TQToolTip::add(d->todayButton, i18n("Select the current day"));

  // -----
  setFontSize(fontsize);
  line->setValidator(val);
  line->installEventFilter( this );
  if (  TQApplication::reverseLayout() )
  {
      yearForward->setIconSet(BarIconSet(TQString::fromLatin1("2leftarrow")));
      yearBackward->setIconSet(BarIconSet(TQString::fromLatin1("2rightarrow")));
      monthForward->setIconSet(BarIconSet(TQString::fromLatin1("1leftarrow")));
      monthBackward->setIconSet(BarIconSet(TQString::fromLatin1("1rightarrow")));
  }
  else
  {
      yearForward->setIconSet(BarIconSet(TQString::fromLatin1("2rightarrow")));
      yearBackward->setIconSet(BarIconSet(TQString::fromLatin1("2leftarrow")));
      monthForward->setIconSet(BarIconSet(TQString::fromLatin1("1rightarrow")));
      monthBackward->setIconSet(BarIconSet(TQString::fromLatin1("1leftarrow")));
  }
  connect(table, TQ_SIGNAL(dateChanged(TQDate)), TQ_SLOT(dateChangedSlot(TQDate)));
  connect(table, TQ_SIGNAL(tableClicked()), TQ_SLOT(tableClickedSlot()));
  connect(monthForward, TQ_SIGNAL(clicked()), TQ_SLOT(monthForwardClicked()));
  connect(monthBackward, TQ_SIGNAL(clicked()), TQ_SLOT(monthBackwardClicked()));
  connect(yearForward, TQ_SIGNAL(clicked()), TQ_SLOT(yearForwardClicked()));
  connect(yearBackward, TQ_SIGNAL(clicked()), TQ_SLOT(yearBackwardClicked()));
  connect(d->selectWeek, TQ_SIGNAL(activated(int)), TQ_SLOT(weekSelected(int)));
  connect(d->todayButton, TQ_SIGNAL(clicked()), TQ_SLOT(todayButtonClicked()));
  connect(selectMonth, TQ_SIGNAL(clicked()), TQ_SLOT(selectMonthClicked()));
  connect(selectYear, TQ_SIGNAL(toggled(bool)), TQ_SLOT(selectYearClicked()));
  connect(line, TQ_SIGNAL(returnPressed()), TQ_SLOT(lineEnterPressed()));
  table->setFocus();


  topLayout->addWidget(table);

  TQBoxLayout * bottomLayout = new TQHBoxLayout(topLayout);
  bottomLayout->addWidget(d->todayButton);
  bottomLayout->addWidget(line);
  bottomLayout->addWidget(d->selectWeek);

  table->setDate(dt);
  dateChangedSlot(dt);  // needed because table emits changed only when newDate != oldDate
}

KDatePicker::~KDatePicker()
{
  delete d;
}

bool
KDatePicker::eventFilter(TQObject *o, TQEvent *e )
{
   if ( e->type() == TQEvent::KeyPress ) {
      TQKeyEvent *k = (TQKeyEvent *)e;

      if ( (k->key() == TQt::Key_Prior) ||
           (k->key() == TQt::Key_Next)  ||
           (k->key() == TQt::Key_Up)    ||
           (k->key() == TQt::Key_Down) )
       {
          TQApplication::sendEvent( table, e );
          table->setFocus();
          return true; // eat event
       }
   }
   return TQFrame::eventFilter( o, e );
}

void
KDatePicker::resizeEvent(TQResizeEvent* e)
{
  TQWidget::resizeEvent(e);
}

void
KDatePicker::dateChangedSlot(TQDate date)
{
    kdDebug(298) << "KDatePicker::dateChangedSlot: date changed (" << date.year() << "/" << date.month() << "/" << date.day() << ")." << endl;

    const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();

    line->setText(TDEGlobal::locale()->formatDate(date, true));
    selectMonth->setText(calendar->monthName(date, false));
    fillWeeksCombo(date);

    // calculate the item num in the week combo box; normalize selected day so as if 1.1. is the first day of the week
    TQDate firstDay;
    calendar->setYMD(firstDay, calendar->year(date), 1, 1);
    d->selectWeek->setCurrentItem((calendar->dayOfYear(date) + calendar->dayOfWeek(firstDay) - 2) / 7/*calendar->daysInWeek()*/);
    selectYear->setText(calendar->yearString(date, false));

    emit(dateChanged(date));
}

void
KDatePicker::tableClickedSlot()
{
  kdDebug(298) << "KDatePicker::tableClickedSlot: table clicked." << endl;
  emit(dateSelected(table->getDate()));
  emit(tableClicked());
}

const TQDate&
KDatePicker::getDate() const
{
  return table->getDate();
}

const TQDate &
KDatePicker::date() const
{
    return table->getDate();
}

bool
KDatePicker::setDate(const TQDate& date)
{
    if(date.isValid())
    {
        table->setDate(date);  // this also emits dateChanged() which then calls our dateChangedSlot()
        return true;
    }
    else
    {
        kdDebug(298) << "KDatePicker::setDate: refusing to set invalid date." << endl;
        return false;
    }
}

void
KDatePicker::monthForwardClicked()
{
    TQDate temp;
    temp = TDEGlobal::locale()->calendar()->addMonths( table->getDate(), 1 );

    setDate( temp );
}

void
KDatePicker::monthBackwardClicked()
{
    TQDate temp;
    temp = TDEGlobal::locale()->calendar()->addMonths( table->getDate(), -1 );

    setDate( temp );
}

void
KDatePicker::yearForwardClicked()
{
    TQDate temp;
    temp = TDEGlobal::locale()->calendar()->addYears( table->getDate(), 1 );

    setDate( temp );
}

void
KDatePicker::yearBackwardClicked()
{
    TQDate temp;
    temp = TDEGlobal::locale()->calendar()->addYears( table->getDate(), -1 );

    setDate( temp );
}

void KDatePicker::selectWeekClicked() {}  // ### in 3.2 obsolete; kept for binary compatibility

void
KDatePicker::weekSelected(int week)
{
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();

  TQDate date = table->getDate();
  int year = calendar->year(date);

  calendar->setYMD(date, year, 1, 1);  // first day of selected year

  // calculate the first day in the selected week (day 1 is first day of week)
  date = calendar->addDays(date, week * 7/*calendar->daysOfWeek()*/ -calendar->dayOfWeek(date) + 1);

  setDate(date);
}

void
KDatePicker::selectMonthClicked()
{
  // every year can have different month names (in some calendar systems)
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();
  TQDate date = table->getDate();
  int i, month, months = calendar->monthsInYear(date);

  TQPopupMenu popup(selectMonth);

  for (i = 1; i <= months; i++)
    popup.insertItem(calendar->monthName(i, calendar->year(date)), i);

  popup.setActiveItem(calendar->month(date) - 1);

  if ( (month = popup.exec(selectMonth->mapToGlobal(TQPoint(0, 0)), calendar->month(date) - 1)) == -1 ) return;  // canceled

  int day = calendar->day(date);
  // ----- construct a valid date in this month:
  calendar->setYMD(date, calendar->year(date), month, 1);
  date = date.addDays(TQMIN(day, calendar->daysInMonth(date)) - 1);
  // ----- set this month
  setDate(date);
}

void
KDatePicker::selectYearClicked()
{
  const KCalendarSystem * calendar = TDEGlobal::locale()->calendar();

  if (selectYear->state() == TQButton::Off)
  {
    return;
  }

  int year;
  TDEPopupFrame* popup = new TDEPopupFrame(this);
  KDateInternalYearSelector* picker = new KDateInternalYearSelector(popup);
  // -----
  picker->resize(picker->sizeHint());
  picker->setYear( table->getDate().year() );
  picker->selectAll();
  popup->setMainWidget(picker);
  connect(picker, TQ_SIGNAL(closeMe(int)), popup, TQ_SLOT(close(int)));
  picker->setFocus();
  if(popup->exec(selectYear->mapToGlobal(TQPoint(0, selectMonth->height()))))
    {
      TQDate date;
      int day;
      // -----
      year=picker->getYear();
      date=table->getDate();
      day=calendar->day(date);
      // ----- construct a valid date in this month:
      //date.setYMD(year, date.month(), 1);
      //date.setYMD(year, date.month(), TQMIN(day, date.daysInMonth()));
      calendar->setYMD(date, year, calendar->month(date),
                       TQMIN(day, calendar->daysInMonth(date)));
      // ----- set this month
      setDate(date);
    } else {
      KNotifyClient::beep();
    }

  delete popup;
  TQTimer::singleShot(0, this, TQ_SLOT(ensureSelectYearIsUp()));
}

void
KDatePicker::ensureSelectYearIsUp()
{
  if (!selectYear->isDown())
  {
    selectYear->setOn( false );
  }
}

void
KDatePicker::setEnabled(bool enable)
{
  TQWidget *widgets[]= {
    yearForward, yearBackward, monthForward, monthBackward,
    selectMonth, selectYear,
    line, table, d->selectWeek, d->todayButton };
  const int Size=sizeof(widgets)/sizeof(widgets[0]);
  int count;
  // -----
  for(count=0; count<Size; ++count)
    {
      widgets[count]->setEnabled(enable);
    }
}

void
KDatePicker::lineEnterPressed()
{
  TQDate temp;
  // -----
  if(val->date(line->text(), temp)==TQValidator::Acceptable)
    {
        kdDebug(298) << "KDatePicker::lineEnterPressed: valid date entered." << endl;
        emit(dateEntered(temp));
        setDate(temp);
    } else {
      KNotifyClient::beep();
      kdDebug(298) << "KDatePicker::lineEnterPressed: invalid date entered." << endl;
    }
}

void
KDatePicker::todayButtonClicked()
{
  setDate(TQDate::currentDate());
}

TQSize
KDatePicker::sizeHint() const
{
  return TQWidget::sizeHint();
}

void
KDatePicker::setFontSize(int s)
{
  TQWidget *buttons[]= {
    // yearBackward,
    // monthBackward,
    selectMonth,
    selectYear,
    // monthForward,
    // yearForward
  };
  const int NoOfButtons=sizeof(buttons)/sizeof(buttons[0]);
  int count;
  TQFont font;
  TQRect r;
  // -----
  fontsize=s;
  for(count=0; count<NoOfButtons; ++count)
    {
      font=buttons[count]->font();
      font.setPointSize(s);
      buttons[count]->setFont(font);
    }
  TQFontMetrics metrics(selectMonth->fontMetrics());

  for (int i = 1; ; ++i)
    {
      TQString str = TDEGlobal::locale()->calendar()->monthName(i,
         TDEGlobal::locale()->calendar()->year(table->getDate()), false);
      if (str.isNull()) break;
      r=metrics.boundingRect(str);
      maxMonthRect.setWidth(TQMAX(r.width(), maxMonthRect.width()));
      maxMonthRect.setHeight(TQMAX(r.height(),  maxMonthRect.height()));
    }

  TQSize metricBound = style().sizeFromContents(TQStyle::CT_ToolButton,
                                               selectMonth,
                                               maxMonthRect);
  selectMonth->setMinimumSize(metricBound);

  table->setFontSize(s);
}

void
KDatePicker::setCloseButton( bool enable )
{
    if ( enable == (d->closeButton != 0L) )
        return;

    if ( enable ) {
        d->closeButton = new TQToolButton( this );
        d->closeButton->setAutoRaise(true);
        d->navigationLayout->addSpacing(KDialog::spacingHint());
        d->navigationLayout->addWidget(d->closeButton);
        TQToolTip::add(d->closeButton, i18n("Close"));
        d->closeButton->setPixmap( SmallIcon("remove") );
        connect( d->closeButton, TQ_SIGNAL( clicked() ),
                 topLevelWidget(), TQ_SLOT( close() ) );
    }
    else {
        delete d->closeButton;
        d->closeButton = 0L;
    }

    updateGeometry();
}

bool KDatePicker::hasCloseButton() const
{
    return (d->closeButton);
}

void KDatePicker::virtual_hook( int /*id*/, void* /*data*/ )
{ /*BASE::virtual_hook( id, data );*/ }

