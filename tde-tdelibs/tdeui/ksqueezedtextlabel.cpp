/* This file is part of the KDE libraries
   Copyright (C) 2000 Ronny Standtke <Ronny.Standtke@gmx.de>

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

#include "ksqueezedtextlabel.h"
#include "kstringhandler.h"
#include <tqtooltip.h>

KSqueezedTextLabel::KSqueezedTextLabel( const TQString &text , TQWidget *parent, const char *name )
 : TQLabel ( parent, name ) {
  setSizePolicy(TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ));
  fullText = text;
  squeezeTextToLabel();
}

KSqueezedTextLabel::KSqueezedTextLabel( TQWidget *parent, const char *name )
 : TQLabel ( parent, name ) {
  setSizePolicy(TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ));
}

void KSqueezedTextLabel::resizeEvent( TQResizeEvent * ) {
  squeezeTextToLabel();
}

TQSize KSqueezedTextLabel::minimumSizeHint() const
{
  TQSize sh = TQLabel::minimumSizeHint();
  sh.setWidth(-1);
  return sh;
}

TQSize KSqueezedTextLabel::sizeHint() const
{
  return TQSize(contentsRect().width(), TQLabel::sizeHint().height());
}

void KSqueezedTextLabel::setText( const TQString &text ) {
  fullText = text;
  squeezeTextToLabel();
}

void KSqueezedTextLabel::squeezeTextToLabel() {
  TQFontMetrics fm(fontMetrics());
  int labelWidth = size().width();
  int textWidth = fm.width(fullText);
  if (textWidth > labelWidth) {
    TQString squeezedText = KStringHandler::cPixelSqueeze(fullText, fm, labelWidth);
	TQLabel::setText(squeezedText);

    TQToolTip::remove( this );
    TQToolTip::add( this, fullText );

  } else {
    TQLabel::setText(fullText);

    TQToolTip::remove( this );
    TQToolTip::hide();

  }
}

void KSqueezedTextLabel::setAlignment( int alignment )
{
  // save fullText and restore it
  TQString tmpFull(fullText);
  TQLabel::setAlignment(alignment);
  fullText = tmpFull;
}

void KSqueezedTextLabel::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "ksqueezedtextlabel.moc"
