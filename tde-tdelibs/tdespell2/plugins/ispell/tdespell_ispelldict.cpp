/*
 * tdespell_aspelldict.cpp
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
#include "tdespell_ispelldict.h"

#include <kdebug.h>

#include "ispell_checker.h"

using namespace KSpell2;

ISpellDict::ISpellDict( const TQString& lang )
    : Dictionary( lang )
{
    m_checker = new ISpellChecker();

    if ( !m_checker->requestDictionary( lang.latin1() ) ) {
        kdError()<<"Language \""<< lang << "\" doesn't exist for Ispell"<<endl;
    }
}

ISpellDict::~ISpellDict()
{
}

bool ISpellDict::check( const TQString& word )
{
    return m_checker->checkWord( word );
}

TQStringList ISpellDict::suggest( const TQString& word )
{
    return m_checker->suggestWord( word );
}

bool ISpellDict::checkAndSuggest( const TQString& word,
                                  TQStringList& suggestions )
{
    bool c = check( word );
    if ( c )
        suggestions = suggest( word );
    return c;
}

bool ISpellDict::storeReplacement( const TQString& ,
                                   const TQString& )
{
    return false;
}

bool ISpellDict::addToPersonal( const TQString& )
{
    return false;
}

bool ISpellDict::addToSession( const TQString& )
{
    return false;
}
