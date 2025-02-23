/*
    This file is part of libtdeabc.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef VCARDLINE_H
#define VCARDLINE_H

#include <tqstringlist.h>
#include <tqvaluelist.h>
#include <tqvariant.h>
#include <tqmap.h>
#include <tqstring.h>

namespace TDEABC {

class VCardLine
{
  public:
    typedef TQValueList<VCardLine> List;
    typedef TQMap<TQString, TQStringList> ParamMap;

    VCardLine();
    VCardLine( const TQString &identifier );
    VCardLine( const TQString &identifier, const TQVariant &value );
    VCardLine( const VCardLine& );

    ~VCardLine();

    VCardLine& operator=( const VCardLine& );

    /**
     * Sets the identifier of this line e.g. UID, FN, CLASS
     */
    void setIdentifier( const TQString& identifier );

    /**
     * Returns the identifier of this line.
     */
    TQString identifier() const;

    /**
     * Sets the value of of this line.
     */
    void setValue( const TQVariant& value );

    /**
     * Returns the value of this line.
     */
    TQVariant value() const;

    /**
     * Sets the group the line belongs to.
     */
    void setGroup( const TQString& group );

    /**
     * Returns the group the line belongs to.
     */
    TQString group() const;

    /**
     * Returns whether the line belongs to a group.
     */
    bool hasGroup() const;

    /**
     * Returns all parameters.
     */
    TQStringList parameterList() const;

    /**
     * Add a new parameter to the line.
     */
    void addParameter( const TQString& param, const TQString& value );

    /**
     * Returns the values of a special parameter.
     * You can get a list of all parameters with paramList().
     */
    TQStringList parameters( const TQString& param ) const;

    /**
     * Returns only the first value of a special parameter.
     * You can get a list of all parameters with paramList().
     */
    TQString parameter( const TQString& param ) const;

  private:
    ParamMap mParamMap;
    TQString mIdentifier;
    TQVariant mValue;

    class VCardLinePrivate;
    VCardLinePrivate *d;
};

}

#endif
