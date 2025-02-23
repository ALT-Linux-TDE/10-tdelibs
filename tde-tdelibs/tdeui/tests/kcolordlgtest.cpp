/* This file is part of the KDE libraries
    Copyright (C) 1997 Martin Jones (mjones@kde.org)

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

#include <tdeapplication.h>
#include "kcolordialog.h"
#include <tdeconfig.h>
#include <tdelocale.h>

int main( int argc, char *argv[] )
{
	TQColor color;

	TDELocale::setMainCatalogue("tdelibs");
	TQApplication::setColorMode( TQApplication::CustomColors );
	TDEApplication a( argc, argv, "KColorDialogTest" );
        TDEConfig aConfig;
        aConfig.setGroup( "KColorDialog-test" );
    
	color = aConfig.readColorEntry( "Chosen" );
	int nRet = KColorDialog::getColor( color, TQt::red /*testing default color*/ );
	aConfig.writeEntry( "Chosen", color );
	
	return nRet;
}

