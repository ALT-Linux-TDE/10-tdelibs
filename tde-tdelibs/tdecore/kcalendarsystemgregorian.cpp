/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002-2003 Hans Petter Bieker <bieker@kde.org>

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

// Derived gregorian kde calendar class
// Just a schema.

#include <tqdatetime.h>
#include <tqstring.h>

#include <tdelocale.h>
#include <kdebug.h>

#include "kcalendarsystemgregorian.h"

KCalendarSystemGregorian::KCalendarSystemGregorian(const TDELocale * locale)
  : KCalendarSystem(locale)
{
}

KCalendarSystemGregorian::~KCalendarSystemGregorian()
{
}

int KCalendarSystemGregorian::year(const TQDate& date) const
{
  return date.year();
}

int KCalendarSystemGregorian::monthsInYear( const TQDate & date ) const
{
  Q_UNUSED( date )

  return 12;
}

int KCalendarSystemGregorian::weeksInYear(int year) const
{
  TQDate temp;
  temp.setYMD(year, 12, 31);

  // If the last day of the year is in the first week, we have to check the
  // week before
  if ( temp.weekNumber() == 1 )
    temp = temp.addDays(-7);

  return temp.weekNumber();
}

int KCalendarSystemGregorian::weekNumber(const TQDate& date,
                                         int * yearNum) const
{
  return date.weekNumber(yearNum);
}

TQString KCalendarSystemGregorian::monthName(const TQDate& date,
                                            bool shortName) const
{
  return monthName(month(date), year(date), shortName);
}

TQString KCalendarSystemGregorian::monthNamePossessive(const TQDate& date, bool shortName) const
{
  return monthNamePossessive(month(date), year(date), shortName);
}

TQString KCalendarSystemGregorian::monthName(int month, int year, bool shortName) const
{
  Q_UNUSED(year);

  if ( shortName )
    switch ( month )
      {
      case 1:
        return locale()->translate("January", "Jan");
      case 2:
        return locale()->translate("February", "Feb");
      case 3:
        return locale()->translate("March", "Mar");
      case 4:
        return locale()->translate("April", "Apr");
      case 5:
        return locale()->translate("May short", "May");
      case 6:
        return locale()->translate("June", "Jun");
      case 7:
        return locale()->translate("July", "Jul");
      case 8:
        return locale()->translate("August", "Aug");
      case 9:
        return locale()->translate("September", "Sep");
      case 10:
        return locale()->translate("October", "Oct");
      case 11:
        return locale()->translate("November", "Nov");
      case 12:
        return locale()->translate("December", "Dec");
      }
  else
    switch ( month )
      {
      case 1:
        return locale()->translate("January");
      case 2:
        return locale()->translate("February");
      case 3:
        return locale()->translate("March");
      case 4:
        return locale()->translate("April");
      case 5:
        return locale()->translate("May long", "May");
      case 6:
        return locale()->translate("June");
      case 7:
        return locale()->translate("July");
      case 8:
        return locale()->translate("August");
      case 9:
        return locale()->translate("September");
      case 10:
        return locale()->translate("October");
      case 11:
        return locale()->translate("November");
      case 12:
        return locale()->translate("December");
      }

  return TQString::null;
}

TQString KCalendarSystemGregorian::monthNamePossessive(int month, int year,
                                                      bool shortName) const
{
  Q_UNUSED(year);

  if ( shortName )
    switch ( month )
      {
      case 1:
        return locale()->translate("of January", "of Jan");
      case 2:
        return locale()->translate("of February", "of Feb");
      case 3:
        return locale()->translate("of March", "of Mar");
      case 4:
        return locale()->translate("of April", "of Apr");
      case 5:
        return locale()->translate("of May short", "of May");
      case 6:
        return locale()->translate("of June", "of Jun");
      case 7:
        return locale()->translate("of July", "of Jul");
      case 8:
        return locale()->translate("of August", "of Aug");
      case 9:
        return locale()->translate("of September", "of Sep");
      case 10:
        return locale()->translate("of October", "of Oct");
      case 11:
       return locale()->translate("of November", "of Nov");
      case 12:
        return locale()->translate("of December", "of Dec");
      }
  else
    switch ( month )
      {
      case 1:
        return locale()->translate("of January");
      case 2:
        return locale()->translate("of February");
      case 3:
        return locale()->translate("of March");
      case 4:
        return locale()->translate("of April");
      case 5:
        return locale()->translate("of May long", "of May");
      case 6:
        return locale()->translate("of June");
      case 7:
        return locale()->translate("of July");
      case 8:
        return locale()->translate("of August");
      case 9:
        return locale()->translate("of September");
      case 10:
        return locale()->translate("of October");
      case 11:
        return locale()->translate("of November");
      case 12:
        return locale()->translate("of December");
      }

  return TQString::null;
}

bool KCalendarSystemGregorian::setYMD(TQDate & date, int y, int m, int d) const
{
  // We don't want Qt to add 1900 to them
  if ( y >= 0 && y <= 99 )
    return false;

  // TQDate supports gregorian internally
  return date.setYMD(y, m, d);
}

TQDate KCalendarSystemGregorian::addYears(const TQDate & date, int nyears) const
{
  return date.addYears(nyears);
}

TQDate KCalendarSystemGregorian::addMonths(const TQDate & date, int nmonths) const
{
  return date.addMonths(nmonths);
}

TQDate KCalendarSystemGregorian::addDays(const TQDate & date, int ndays) const
{
  return date.addDays(ndays);
}

TQString KCalendarSystemGregorian::weekDayName(int col, bool shortName) const
{
  // ### Should this really be different to each calendar system? Or are we
  //     only going to support weeks with 7 days?

  return KCalendarSystem::weekDayName(col, shortName);
}

TQString KCalendarSystemGregorian::weekDayName(const TQDate& date, bool shortName) const
{
  return weekDayName(dayOfWeek(date), shortName);
}


int KCalendarSystemGregorian::dayOfWeek(const TQDate& date) const
{
  return date.dayOfWeek();
}

int KCalendarSystemGregorian::dayOfYear(const TQDate & date) const
{
  return date.dayOfYear();
}

int KCalendarSystemGregorian::daysInMonth(const TQDate& date) const
{
  return date.daysInMonth();
}

int KCalendarSystemGregorian::minValidYear() const
{
  return 1753; // TQDate limit
}

int KCalendarSystemGregorian::maxValidYear() const
{
  return 8000; // TQDate limit
}

int KCalendarSystemGregorian::day(const TQDate& date) const
{
  return date.day();
}

int KCalendarSystemGregorian::month(const TQDate& date) const
{
  return date.month();
}

int KCalendarSystemGregorian::daysInYear(const TQDate& date) const
{
  return date.daysInYear();
}

int KCalendarSystemGregorian::weekDayOfPray() const
{
  return 7; // sunday
}

TQString KCalendarSystemGregorian::calendarName() const
{
  return TQString::fromLatin1("gregorian");
}

bool KCalendarSystemGregorian::isLunar() const
{
  return false;
}

bool KCalendarSystemGregorian::isLunisolar() const
{
  return false;
}

bool KCalendarSystemGregorian::isSolar() const
{
  return true;
}

int KCalendarSystemGregorian::yearStringToInteger(const TQString & sNum, int & iLength) const
{
  int iYear;
  iYear = KCalendarSystem::yearStringToInteger(sNum, iLength);
  
  // Qt treats a year in the range 0-100 as 1900-1999.
  // It is nicer for the user if we treat 0-68 as 2000-2068
  if (iYear < 69)
    iYear += 2000;
  else if (iYear < 100)
    iYear += 1900;

  return iYear;
}
