/**
 * tdespell_hspellclient.cpp
 *
 * Copyright (C)  2003  Zack Rusin <zack@kde.org>
 * Copyright (C)  2005  Mashrab Kuvatov <kmashrab@uni-bremen.de>
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
#include "tdespell_hspellclient.h"

#include "tdespell_hspelldict.h"

#include <kgenericfactory.h>
#include <kdebug.h>

typedef KGenericFactory<HSpellClient> HSpellClientFactory;
K_EXPORT_COMPONENT_FACTORY( tdespell_hspell, HSpellClientFactory( "tdespell_hspell" )  )

using namespace KSpell2;

HSpellClient::HSpellClient( TQObject *parent, const char *name, const TQStringList& /* args */  )
    : Client( parent, name )
{
}

HSpellClient::~HSpellClient()
{
}

Dictionary* HSpellClient::dictionary( const TQString& language )
{
    HSpellDict *ad = new HSpellDict( language );
    return ad;
}

TQStringList HSpellClient::languages() const
{
    TQStringList langs;
    langs.append( "he" );

    return langs;
}

#include "tdespell_hspellclient.moc"
