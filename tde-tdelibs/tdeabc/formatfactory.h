/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef KABC_FORMATFACTORY_H
#define KABC_FORMATFACTORY_H

#include <tqdict.h>
#include <tqstring.h>

#include <tdeconfig.h>
#include <klibloader.h>

#include "formatplugin.h"

namespace TDEABC {

struct FormatInfo
{
  TQString library;
  TQString nameLabel;
  TQString descriptionLabel;
};

/**
 * Class for loading format plugins.
 *
 * Example:
 *
 * \code
 * TDEABC::FormatFactory *factory = TDEABC::FormatFactory::self();
 *
 * TQStringList list = factory->formats();
 * TQStringList::Iterator it;
 * for ( it = list.begin(); it != list.end(); ++it ) {
 *   TDEABC::FormatPlugin *format = factory->format( (*it) );
 *   // do something with format
 * }
 * \endcode
 */
class KABC_EXPORT FormatFactory
{
  public:
    
    /**
      Destructor.
     */
    ~FormatFactory();

    /**
     * Returns the global format factory.
     */
    static FormatFactory *self();

    /**
     * Returns a pointer to a format object or a null pointer
     * if format type doesn't exist.
     *
     * @param type   The type of the format, returned by formats()
     */
    FormatPlugin *format( const TQString &type );

    /**
     * Returns a list of all available format types.
     */
    TQStringList formats();

    /**
     * Returns the info structure for a special type.
     */
    FormatInfo *info( const TQString &type );

  protected:
    FormatFactory();

  private:
    KLibrary *openLibrary( const TQString& libName );

    static FormatFactory *mSelf;

    TQDict<FormatInfo> mFormatList;
};

}
#endif
