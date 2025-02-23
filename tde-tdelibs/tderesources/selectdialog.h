/*
    This file is part of libtderesources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef TDERESOURCES_SELECTDIALOG_H
#define TDERESOURCES_SELECTDIALOG_H

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqmap.h>

#include <kdialog.h>

class TDEListBox;

namespace KRES {

class Resource;

/**
 * Dialog for selecting a resource.
 *
 * Example:
 *
 * \code
 *
 * TQPtrList<Resource> list = ... // can be retrived from KRES::Manager (e.g. TDEABC::AddressBook)
 *
 * TDEABC::Resource *res = TDEABC::SelectDialog::getResource( list, parentWdg );
 * if ( !res ) {
 *   // no resource selected
 * } else {
 *   // do something with resource
 * }
 * \endcode
 */
class TDERESOURCES_EXPORT SelectDialog : KDialog
{
  public:
    /**
     * Constructor.
     * @param list   The list of available resources
     * @param parent The parent widget
     * @param name   The name of the dialog
     */
    SelectDialog( TQPtrList<Resource> list, TQWidget *parent = 0,
        const char *name = 0);

    /**
     * Returns selected resource.
     */
    Resource *resource();

    /**
     * Opens a dialog showing the available resources and returns the resource the
     * user has selected. Returns 0, if the dialog was canceled.
     */
    static Resource *getResource( TQPtrList<Resource> list, TQWidget *parent = 0 );

  private:
    TDEListBox *mResourceId;

    TQMap<int, Resource*> mResourceMap;
};

}

#endif
