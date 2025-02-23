/*
 *  Copyright (C) 2003 David Faure   <faure@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation;
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 */

#include <tdeapplication.h>
#include <kdebug.h>
#include <tqwidget.h>
#include "kwordwrap.h"

int main(int argc, char *argv[])
{
	TDEApplication app(argc, argv, "KWordWrapTest");
	
	TQFont font( "helvetica", 12 ); // let's hope we all have the same...
	TQFontMetrics fm( font );
	TQRect r( 0, 0, 100, -1 );
	TQString str = "test wadabada [/foo/bar/waba]";
	KWordWrap* ww = KWordWrap::formatText( fm, r, 0, str );
	kdDebug() << str << " => " << ww->truncatedString() << endl;
	delete ww;

	str = "</p></p></p></p>";
	for ( ; r.width() > 0 ; r.setWidth( r.width()-10 ) )
	{
	    ww = KWordWrap::formatText( fm, r, 0, str );
	    kdDebug() << str << " => " << ww->truncatedString() << endl;
	    delete ww;
	}
}
