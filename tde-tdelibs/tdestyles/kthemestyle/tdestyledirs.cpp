/*
 $Id$

 Simple helper routines for style's use of TDEStandardDirs with TQSettings, etc.

 This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.


 This file is part of the KDE libraries
*/

#include <tqfile.h>
#include <kstandarddirs.h>
#include "tdestyledirs.h"

TDEStyleDirs* TDEStyleDirs::instance = 0;

TDEStyleDirs::TDEStyleDirs()
{
    addResourceType( "themepixmap", TDEStandardDirs::kde_default( "data" ) + "tdestyle/pixmaps/" );
    addResourceType( "themerc", TDEStandardDirs::kde_default( "data" ) + "tdestyle/themes/" );
}

TDEStyleDirs::~TDEStyleDirs()
{
}

void TDEStyleDirs::addToSearch( const char* type, TQSettings& s ) const
{
    const TQStringList & dirs = resourceDirs(type);
    for ( int c = dirs.size()-1; c >= 0 ; c-- )
    {
        s.insertSearchPath( TQSettings::Unix, dirs[ c ]);
    }
}

