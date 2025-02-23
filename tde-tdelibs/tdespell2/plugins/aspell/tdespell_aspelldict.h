/**
 * tdespell_aspelldict.h
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
#ifndef TDESPELL_ASPELLDICT_H
#define TDESPELL_ASPELLDICT_H

#include "dictionary.h"

#include "aspell.h"

class ASpellDict : public KSpell2::Dictionary
{
public:
    ASpellDict( const TQString& lang );
    ~ASpellDict();
    virtual bool check( const TQString& word );

    virtual TQStringList suggest( const TQString& word );

    virtual bool checkAndSuggest( const TQString& word,
                                  TQStringList& suggestions ) ;

    virtual bool storeReplacement( const TQString& bad,
                                   const TQString& good );

    virtual bool addToPersonal( const TQString& word );
    virtual bool addToSession( const TQString& word );
private:
    AspellConfig  *m_config;
    AspellSpeller *m_speller;
};

#endif
