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
#include "tdespell_ispellclient.h"

#include "tdespell_ispelldict.h"
#include "ispell_checker.h"

#include <kgenericfactory.h>
#include <kdebug.h>

typedef KGenericFactory<ISpellClient> ISpellClientFactory;
K_EXPORT_COMPONENT_FACTORY( tdespell_ispell, ISpellClientFactory( "tdespell_ispell" )  )

using namespace KSpell2;

ISpellClient::ISpellClient( TQObject *parent, const char *name, const TQStringList& /* args */  )
    : Client( parent, name )
{
}

ISpellClient::~ISpellClient()
{
}

Dictionary* ISpellClient::dictionary( const TQString& language )
{
    ISpellDict *ad = new ISpellDict( language );
    return ad;
}

TQStringList ISpellClient::languages() const
{
    return ISpellChecker::allDics();
}

#include "tdespell_ispellclient.moc"
