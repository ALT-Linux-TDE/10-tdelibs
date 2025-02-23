/*
 *   Copyright (C) 1997  Michael Roth <mroth@wirlweb.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details.
 *
 *   You should have received a copy of the GNU Library General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <tqstyle.h>

#include <kdebug.h>
#include <tdeapplication.h>

#include "kseparator.moc"


KSeparator::KSeparator(TQWidget* parent, const char* name, WFlags f)
   : TQFrame(parent, name, f)
{
   setLineWidth(1);
   setMidLineWidth(0);
   setOrientation( HLine );
}



KSeparator::KSeparator(int orientation, TQWidget* parent, const char* name, WFlags f)
   : TQFrame(parent, name, f)
{
   setLineWidth(1);
   setMidLineWidth(0);
   setOrientation( orientation );
}



void KSeparator::setOrientation(int orientation)
{
   switch(orientation)
   {
      case TQt::Vertical:
      case VLine:
         setFrameStyle( TQFrame::VLine | TQFrame::Sunken );
         setMinimumSize(2, 0);
         break;
      
      default:
         kdWarning() << "KSeparator::setOrientation(): invalid orientation, using default orientation HLine" << endl;
         
      case TQt::Horizontal:
      case HLine:
         setFrameStyle( TQFrame::HLine | TQFrame::Sunken );
         setMinimumSize(0, 2);
         break;
   }
}



int KSeparator::orientation() const
{
   if ( frameStyle() & VLine )
      return VLine;
   
   if ( frameStyle() & HLine )
      return HLine;
   
   return 0;
}

void KSeparator::drawFrame(TQPainter *p)
{
   TQPoint	p1, p2;
   TQRect	r     = frameRect();
   const TQColorGroup & g = colorGroup();

   if ( frameStyle() & HLine ) {
      p1 = TQPoint( r.x(), r.height()/2 );
      p2 = TQPoint( r.x()+r.width(), p1.y() );
   }
   else {
      p1 = TQPoint( r.x()+r.width()/2, 0 );
      p2 = TQPoint( p1.x(), r.height() );
   }

   TQStyleOption opt( lineWidth(), midLineWidth() );
   style().drawPrimitive( TQStyle::PE_Separator, p, TQRect( p1, p2 ), g,
		          TQStyle::Style_Sunken, opt );
}


TQSize KSeparator::sizeHint() const
{
   if ( frameStyle() & VLine )
      return TQSize(2, 0);
   
   if ( frameStyle() & HLine )
      return TQSize(0, 2);
   
   return TQSize(-1, -1);
}

void KSeparator::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

