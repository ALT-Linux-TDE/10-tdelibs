/*
    $Id$

    Requires the Qt widget libraries, available at no cost at 
    http://www.troll.no
       
    Copyright (C) 1996 Bernd Johannes Wuebben   
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
  
*/


#include <tdeapplication.h>
#include "tdefontdialog.h"
#include <tdeconfig.h>


 int main( int argc, char **argv )
{
  TDEApplication app( argc, argv, "TDEFontDialogTest" );

  TDEConfig aConfig;
  aConfig.setGroup( "TDEFontDialog-test" );

  app.setFont(TQFont("Helvetica",12));

  //  TQFont font = TQFont("Times",18,TQFont::Bold);

  TQFont font = aConfig.readFontEntry( "Chosen" );
  int nRet = TDEFontDialog::getFont(font);
  int flags;
  nRet = TDEFontDialog::getFontDiff(font, flags);
  aConfig.writeEntry( "Chosen", font, true );

  aConfig.sync();
  return nRet;
}
