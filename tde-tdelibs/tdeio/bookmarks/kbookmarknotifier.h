/* This file is part of the KDE libraries
   Copyright (C) 2001, 2003 Alexander Kellett <lypanov@kde.org>

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

#ifndef __kbookmarknotifier_h__
#define __kbookmarknotifier_h__

#include <dcopobject.h>

/**
 * DCOP interface for a bookmark notifier (an object which emits signals
 * upon changes to the bookmarks)
 */
class TDEIO_EXPORT KBookmarkNotifier : virtual public DCOPObject
{
  K_DCOP

public:
  KBookmarkNotifier(TQCString objId = "KBookmarkNotifier") : DCOPObject(objId) {}

k_dcop_signals:
  void addedBookmark( TQString filename, TQString url, TQString text, TQString address, TQString icon );
  void createdNewFolder( TQString filename, TQString text, TQString address );
  void updatedAccessMetadata( TQString filename, TQString url );
};

#endif

