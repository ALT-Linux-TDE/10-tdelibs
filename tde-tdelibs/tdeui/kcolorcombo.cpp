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
//-----------------------------------------------------------------------------
// KDE color selection dialog.
//
// 1999-09-27 Espen Sand <espensa@online.no>
// KColorDialog is now subclassed from KDialogBase. I have also extended
// KColorDialog::getColor() so that in contains a parent argument. This
// improves centering capability.
//
// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>
//


#include <stdio.h>
#include <stdlib.h>

#include <tqdrawutil.h>
#include <tqevent.h>
#include <tqfile.h>
#include <tqimage.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqvalidator.h>
#include <tqpainter.h>
#include <tqpushbutton.h>
#include <tqtimer.h>

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kiconloader.h>
#include <tdelistbox.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kseparator.h>
#include <kpalette.h>
#include <kimageeffect.h>

//#include "kcolordialog.h"
//#include "kcolordrag.h"
#include "kcolorcombo.h"

// This is repeated from the KColorDlg, but I didn't
// want to make it public BL.
// We define it out when compiling with --enable-final in which case
// we use the version defined in KColorDlg

#ifndef KDE_USE_FINAL
#define STANDARD_PAL_SIZE 17

static TQColor *standardPalette = 0;

static void createStandardPalette()
{
    if ( standardPalette )
	return;

    standardPalette = new TQColor [STANDARD_PAL_SIZE];

    int i = 0;

    standardPalette[i++] = TQt::red;
    standardPalette[i++] = TQt::green;
    standardPalette[i++] = TQt::blue;
    standardPalette[i++] = TQt::cyan;
    standardPalette[i++] = TQt::magenta;
    standardPalette[i++] = TQt::yellow;
    standardPalette[i++] = TQt::darkRed;
    standardPalette[i++] = TQt::darkGreen;
    standardPalette[i++] = TQt::darkBlue;
    standardPalette[i++] = TQt::darkCyan;
    standardPalette[i++] = TQt::darkMagenta;
    standardPalette[i++] = TQt::darkYellow;
    standardPalette[i++] = TQt::white;
    standardPalette[i++] = TQt::lightGray;
    standardPalette[i++] = TQt::gray;
    standardPalette[i++] = TQt::darkGray;
    standardPalette[i++] = TQt::black;
}
#endif

class KColorCombo::KColorComboPrivate
{
	protected:
	friend class KColorCombo;
	KColorComboPrivate(){}
	~KColorComboPrivate(){}
	bool showEmptyList;
};

KColorCombo::KColorCombo( TQWidget *parent, const char *name )
	: TQComboBox( parent, name )
{
	d=new KColorComboPrivate();
	d->showEmptyList=false;

	customColor.setRgb( 255, 255, 255 );
	internalcolor.setRgb( 255, 255, 255 );

	createStandardPalette();

	addColors();

	connect( this, TQ_SIGNAL( activated(int) ), TQ_SLOT( slotActivated(int) ) );
	connect( this, TQ_SIGNAL( highlighted(int) ), TQ_SLOT( slotHighlighted(int) ) );
}


KColorCombo::~KColorCombo()
{
	delete d;
}
/**
   Sets the current color
 */
void KColorCombo::setColor( const TQColor &col )
{
	internalcolor = col;
	d->showEmptyList=false;
	addColors();
}


/**
   Returns the currently selected color
 */
TQColor KColorCombo::color() const {
  return internalcolor;
}

void KColorCombo::resizeEvent( TQResizeEvent *re )
{
	TQComboBox::resizeEvent( re );

	addColors();
}

/**
   Show an empty list, till the next color is set with setColor
 */
void KColorCombo::showEmptyList()
{
	d->showEmptyList=true;
	addColors();
}

void KColorCombo::slotActivated( int index )
{
	if ( index == 0 )
	{
	    if ( KColorDialog::getColor( customColor, this ) == TQDialog::Accepted )
		{
			TQPainter painter;
			TQPen pen;
			TQRect rect( 0, 0, width(), TQFontMetrics(painter.font()).height()+4);
			TQPixmap pixmap( rect.width(), rect.height() );

			if ( tqGray( customColor.rgb() ) < 128 )
				pen.setColor( white );
			else
				pen.setColor( black );

			painter.begin( &pixmap );
			TQBrush brush( customColor );
			painter.fillRect( rect, brush );
			painter.setPen( pen );
			painter.drawText( 2, TQFontMetrics(painter.font()).ascent()+2, i18n("Custom...") );
			painter.end();

			changeItem( pixmap, 0 );
			pixmap.detach();
		}

		internalcolor = customColor;
	}
	else
		internalcolor = standardPalette[ index - 1 ];

	emit activated( internalcolor );
}

void KColorCombo::slotHighlighted( int index )
{
	if ( index == 0 )
		internalcolor = customColor;
	else
		internalcolor = standardPalette[ index - 1 ];

	emit highlighted( internalcolor );
}

void KColorCombo::addColors()
{
	TQPainter painter;
	TQPen pen;
	TQRect rect( 0, 0, width(), TQFontMetrics(painter.font()).height()+4 );
	TQPixmap pixmap( rect.width(), rect.height() );
	int i;

	clear();
	if (d->showEmptyList) return;

	createStandardPalette();

	for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
		if ( standardPalette[i] == internalcolor ) break;

	if ( i == STANDARD_PAL_SIZE )
		customColor = internalcolor;

	if ( tqGray( customColor.rgb() ) < 128 )
		pen.setColor( white );
	else
		pen.setColor( black );

	painter.begin( &pixmap );
	TQBrush brush( customColor );
	painter.fillRect( rect, brush );
	painter.setPen( pen );
	painter.drawText( 2, TQFontMetrics(painter.font()).ascent()+2, i18n("Custom...") );
	painter.end();

	insertItem( pixmap );
	pixmap.detach();

	for ( i = 0; i < STANDARD_PAL_SIZE; i++ )
	{
		painter.begin( &pixmap );
		TQBrush brush( standardPalette[i] );
		painter.fillRect( rect, brush );
		painter.end();

		insertItem( pixmap );
		pixmap.detach();

		if ( standardPalette[i] == internalcolor )
			setCurrentItem( i + 1 );
	}
}

void KColorCombo::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcolorcombo.moc"
