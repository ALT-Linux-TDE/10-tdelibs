/*
 * tdespell_aspellclient.cpp
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
#include "tdespell_aspellclient.h"

#include "tdespell_aspelldict.h"

#include <kgenericfactory.h>
#include <kdebug.h>

typedef KGenericFactory<ASpellClient> ASpellClientFactory;
K_EXPORT_COMPONENT_FACTORY( tdespell_aspell, ASpellClientFactory( "tdespell_aspell" )  )

using namespace KSpell2;

ASpellClient::ASpellClient( TQObject *parent, const char *name, const TQStringList& /* args */  )
    : Client( parent, name )
{
    m_config = new_aspell_config();
}

ASpellClient::~ASpellClient()
{
    delete_aspell_config( m_config );
}

Dictionary* ASpellClient::dictionary( const TQString& language )
{
    ASpellDict *ad = new ASpellDict( language );
    return ad;
}

TQStringList ASpellClient::languages() const
{
    AspellDictInfoList *l = get_aspell_dict_info_list( m_config );
    AspellDictInfoEnumeration *el = aspell_dict_info_list_elements( l );

    TQStringList langs;
    const AspellDictInfo *di = 0;
    while ( ( di = aspell_dict_info_enumeration_next( el ) ) ) {
        langs.append( di->name );
    }

    delete_aspell_dict_info_enumeration( el );

    return langs;
}

#include "tdespell_aspellclient.moc"
