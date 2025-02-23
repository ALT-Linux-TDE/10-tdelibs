/* This file is part of the KDE libraries
   Copyright (C) 1997 David Sweet <dsweet@kde.org>

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
*/

#include "tdespell.h"
#include <tdeapplication.h>
#include <kdebug.h>

#include <tqstring.h>
#include <tqlabel.h>
#include <tqtextcodec.h>

int main(int argc, char **argv)
{
    TDEApplication *app = new TDEApplication(argc, argv, "KSpellTest");

    // test ASCII
    //TQString text( "I have noo idee of how to wride englisch or englisch" );

    // test Latin-3 (need dictionary "esperanto" and "Latin3" in config)
    //TQCString ctext( "sed jen: e¶oþanø' æiu¼aýde; e¶uþunøo øiun fendredon - kaj dimanøon...");
    //TQTextCodec *codec = TQTextCodec::codecForName("ISO 8859-3");

    // test UTF-8 (need dictionary "esperanto" and "UTF-8" in config)  
   TQCString ctext( "sed jen: e\304\245oÅanÄo Ä‰iuÄµaÅ­de e\304\245uÅunÄo Äiun fendredon kaj dimanÄon");
   TQTextCodec *codec = TQTextCodec::codecForName("UTF-8");
    
    TQString text = codec->toUnicode(ctext.data());

    KSpell::modalCheck( text );

    kdDebug() << "Returned " << text << endl;

    return 0;

}







