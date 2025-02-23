/*
   Copyright (c) 2002 Carlos Moro <cfmoro@correo.uniovi.es>
   Copyright (c) 2002 Hans Petter Bieker <bieker@kde.org>

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

#ifndef KCALENDARSYSTEMFACTORY_H
#define KCALENDARSYSTEMFACTORY_H

#include <tqstring.h>
#include <tqstringlist.h>
#include "tdelibs_export.h"

class KCalendarSystem;
class TDELocale;

/**
 * Factory class for calendar types
 * @author Carlos Moro <cfmoro@correo.uniovi.es>
 * @since 3.2
 */
class TDECORE_EXPORT KCalendarSystemFactory
{
public:
  KCalendarSystemFactory ();
  ~KCalendarSystemFactory ();

  /**
   * Gets specific calendar type number of days in previous month for a
   * given date
   *
   * @param calType string identification of the specific calendar type
   * to be constructed
   * @param locale Locale used for translations. Use the global locale when
   * 0 is specified.
   * @return a KCalendarSystem object
   */
  static KCalendarSystem *create (const TQString & calType = TQString::fromLatin1("gregorian"),
                                  const TDELocale * locale = 0);

  /**
   * Gets list of names of supported calendar systems
   *
   * @return A TQStringList object
   */
  static TQStringList calendarSystems();

private:
};

#endif
