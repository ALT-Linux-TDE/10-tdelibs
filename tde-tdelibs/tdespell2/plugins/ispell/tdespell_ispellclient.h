/*
 * tdespell_ispellclient.h
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */
#ifndef TDESPELL_ISPELLCLIENT_H
#define TDESPELL_ISPELLCLIENT_H

#include "client.h"
#include <tqobject.h>

#include "ispell_checker.h"

namespace KSpell2 {
    class Dictionary;
}
using KSpell2::Dictionary;

class ISpellClient : public KSpell2::Client
{
    TQ_OBJECT
public:
    ISpellClient( TQObject *parent, const char *name, const TQStringList & /* args */  );
    ~ISpellClient();

    virtual int reliability() const {
        return 10;
    }

    virtual Dictionary* dictionary( const TQString& language );

    virtual TQStringList languages() const;

    virtual TQString name() const {
        return "ISpell";
    }
private:

};

#endif
