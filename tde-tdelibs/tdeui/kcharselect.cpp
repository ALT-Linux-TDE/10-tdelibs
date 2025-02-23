/* This file is part of the KDE libraries

   Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>

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

#include "kcharselect.h"
#include "kcharselect.moc"

#include <tqbrush.h>
#include <tqcolor.h>
#include <tqevent.h>
#include <tqfont.h>
#include <tqfontdatabase.h>
#include <tqhbox.h>
#include <tqkeycode.h>
#include <tqlabel.h>
#include <tqpainter.h>
#include <tqpen.h>
#include <tqregexp.h>
#include <tqstyle.h>
#include <tqtooltip.h>
#include <tqvalidator.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <klineedit.h>
#include <tdelocale.h>

class KCharSelect::KCharSelectPrivate
{
public:
    TQLineEdit *unicodeLine;
};

TQFontDatabase * KCharSelect::fontDataBase = 0;

void KCharSelect::cleanupFontDatabase()
{
    delete fontDataBase;
    fontDataBase = 0;
}

/******************************************************************/
/* Class: KCharSelectTable					  */
/******************************************************************/

//==================================================================
KCharSelectTable::KCharSelectTable( TQWidget *parent, const char *name, const TQString &_font,
				    const TQChar &_chr, int _tableNum )
    : TQGridView( parent, name ), vFont( _font ), vChr( _chr ),
      vTableNum( _tableNum ), vPos( 0, 0 ), focusItem( _chr ), focusPos( 0, 0 ), d(0)
{
    setBackgroundColor( colorGroup().base() );

    setCellWidth( 20 );
    setCellHeight( 25 );

    setNumCols( 32 );
    setNumRows( 8 );

    repaintContents( false );
    
    setToolTips();

    setFocusPolicy( TQWidget::StrongFocus );
    setBackgroundMode( TQWidget::NoBackground );
}

//==================================================================
void KCharSelectTable::setFont( const TQString &_font )
{
    vFont = _font;
    repaintContents( false );

    setToolTips();
}

//==================================================================
void KCharSelectTable::setChar( const TQChar &_chr )
{
    vChr = _chr;
    repaintContents( false );
}

//==================================================================
void KCharSelectTable::setTableNum( int _tableNum )
{
    focusItem = TQChar( _tableNum * 256 );

    vTableNum = _tableNum;
    repaintContents( false );

    setToolTips();
}

//==================================================================
TQSize KCharSelectTable::sizeHint() const
{
    int w = cellWidth();
    int h = cellHeight();

    w *= numCols();
    h *= numRows();

    return TQSize( w, h );
}

//==================================================================
void KCharSelectTable::resizeEvent( TQResizeEvent * e )
{
    const int new_w   = (e->size().width()  - 2*(margin()+frameWidth())) / numCols();
    const int new_h   = (e->size().height() - 2*(margin()+frameWidth())) / numRows();

    if( new_w !=  cellWidth())
        setCellWidth( new_w );
    if( new_h !=  cellHeight())
        setCellHeight( new_h );

    setToolTips();
}

//==================================================================
void KCharSelectTable::paintCell( class TQPainter* p, int row, int col )
{
    const int w = cellWidth();
    const int h = cellHeight();
    const int x2 = w - 1;
    const int y2 = h - 1;

    //if( row == 0 && col == 0 ) {
    //    printf("Repaint %d\n", temp++);
    //    fflush( stdout );
    //    }

    TQFont font = TQFont( vFont );
    font.setPixelSize( int(.7 * h) );

    unsigned short c = vTableNum * 256;
    c += row * numCols();
    c += col;

    if ( c == vChr.unicode() ) {
	p->setBrush( TQBrush( colorGroup().highlight() ) );
	p->setPen( NoPen );
	p->drawRect( 0, 0, w, h );
	p->setPen( colorGroup().highlightedText() );
	vPos = TQPoint( col, row );
    } else {
	TQFontMetrics fm = TQFontMetrics( font );
	if( fm.inFont( c ) )
		p->setBrush( TQBrush( colorGroup().base() ) );
	else
		p->setBrush( TQBrush( colorGroup().button() ) );
	p->setPen( NoPen );
	p->drawRect( 0, 0, w, h );
	p->setPen( colorGroup().text() );
    }

    if ( c == focusItem.unicode() && hasFocus() ) {
	style().drawPrimitive( TQStyle::PE_FocusRect, p, TQRect( 2, 2, w - 4, h - 4 ), 
			       colorGroup() );
	focusPos = TQPoint( col, row );
    }

    p->setFont( font );

    p->drawText( 0, 0, x2, y2, AlignHCenter | AlignVCenter, TQString( TQChar( c ) ) );

    p->setPen( colorGroup().text() );
    p->drawLine( x2, 0, x2, y2 );
    p->drawLine( 0, y2, x2, y2 );

    if ( row == 0 )
	p->drawLine( 0, 0, x2, 0 );
    if ( col == 0 )
	p->drawLine( 0, 0, 0, y2 );
}

//==================================================================
void KCharSelectTable::mouseMoveEvent( TQMouseEvent *e )
{
    const int row = rowAt( e->y() );
    const int col = columnAt( e->x() );
    if ( row >= 0 && row < numRows() && col >= 0 && col < numCols() ) {
	const TQPoint oldPos = vPos;

	vPos.setX( col );
	vPos.setY( row );

	vChr = TQChar( vTableNum * 256 + numCols() * vPos.y() + vPos.x() );

	const TQPoint oldFocus = focusPos;

	focusPos = vPos;
	focusItem = vChr;

	repaintCell( oldFocus.y(), oldFocus.x(), true );
	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( vPos.y(), vPos.x(), true );

	emit highlighted( vChr );
	emit highlighted();

	emit focusItemChanged( focusItem );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::keyPressEvent( TQKeyEvent *e )
{
    switch ( e->key() ) {
    case Key_Left:
	gotoLeft();
	break;
    case Key_Right:
	gotoRight();
	break;
    case Key_Up:
	gotoUp();
	break;
    case Key_Down:
	gotoDown();
	break;
    case Key_Next:
	emit tableDown();
	break;
    case Key_Prior:
	emit tableUp();
	break;
    case Key_Space:
	emit activated( ' ' );
	emit activated();
	emit highlighted( ' ' );
	emit highlighted();
        break;
    case Key_Enter: case Key_Return: {
	const TQPoint oldPos = vPos;

	vPos = focusPos;
	vChr = focusItem;

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( vPos.y(), vPos.x(), true );

	emit activated( vChr );
	emit activated();
	emit highlighted( vChr );
	emit highlighted();
    } break;
    }
}

//==================================================================
void KCharSelectTable::gotoLeft()
{
    if ( focusPos.x() > 0 ) {
	const TQPoint oldPos = focusPos;

	focusPos.setX( focusPos.x() - 1 );

	focusItem = TQChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoRight()
{
    if ( focusPos.x() < numCols()-1 ) {
	const TQPoint oldPos = focusPos;

	focusPos.setX( focusPos.x() + 1 );

	focusItem = TQChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoUp()
{
    if ( focusPos.y() > 0 ) {
	const TQPoint oldPos = focusPos;

	focusPos.setY( focusPos.y() - 1 );

	focusItem = TQChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::gotoDown()
{
    if ( focusPos.y() < numRows()-1 ) {
	const TQPoint oldPos = focusPos;

	focusPos.setY( focusPos.y() + 1 );

	focusItem = TQChar( vTableNum * 256 + numCols() * focusPos.y() + focusPos.x() );

	repaintCell( oldPos.y(), oldPos.x(), true );
	repaintCell( focusPos.y(), focusPos.x(), true );

	emit focusItemChanged( vChr );
	emit focusItemChanged();
    }
}

//==================================================================
void KCharSelectTable::setToolTips()
{
    const int rowCount = numRows();
    const int colCount = numCols();
    for( int i=0 ; i< rowCount; ++i )
    {
	for( int j=0; j< colCount; ++j )
	{
	    const TQRect r( cellWidth()*j, cellHeight()*i, cellWidth(), cellHeight() );
	    TQToolTip::remove(this,r);
	    const ushort uni = vTableNum * 256 + numCols()*i + j;
	    TQString s;
	    s.sprintf( "%04X", uint( uni ) );
	    TQString character; // Character (which sometimes need to be escaped)
            switch ( uni )
	    {
                case 0x3c: character = "&lt;"; break;
                case 0x3e: character = "&gt;"; break;
                case 0x26: character = "&amp;"; break;
                case 0x27: character = "&apos;"; break;
                case 0x22: character = "&quot;"; break;
                default: character = TQChar( uni ); break;
	    }
	    TQToolTip::add(this, r, i18n( "Character","<qt><font size=\"+4\" face=\"%1\">%2</font><br>Unicode code point: U+%3<br>(In decimal: %4)<br>(Character: %5)</qt>" ).arg( vFont ).arg( character ).arg( s ).arg( uni ).arg( character ) );
	}
    }
}

/******************************************************************/
/* Class: KCharSelect						  */
/******************************************************************/

//==================================================================
KCharSelect::KCharSelect( TQWidget *parent, const char *name, const TQString &_font, const TQChar &_chr, int _tableNum )
  : TQVBox( parent, name ), d(new KCharSelectPrivate)
{
    setSpacing( KDialog::spacingHint() );
    TQHBox* const bar = new TQHBox( this );
    bar->setSpacing( KDialog::spacingHint() );

    TQLabel* const lFont = new TQLabel( i18n( "Font:" ), bar );
    lFont->resize( lFont->sizeHint() );
    lFont->setAlignment( TQt::AlignRight | TQt::AlignVCenter );
    lFont->setMaximumWidth( lFont->sizeHint().width() );

    fontCombo = new TQComboBox( true, bar );
    fillFontCombo();
    fontCombo->resize( fontCombo->sizeHint() );

    connect( fontCombo, TQ_SIGNAL( activated( const TQString & ) ), this, TQ_SLOT( fontSelected( const TQString & ) ) );

    TQLabel* const lTable = new TQLabel( i18n( "Table:" ), bar );
    lTable->resize( lTable->sizeHint() );
    lTable->setAlignment( TQt::AlignRight | TQt::AlignVCenter );
    lTable->setMaximumWidth( lTable->sizeHint().width() );

    tableSpinBox = new TQSpinBox( 0, 255, 1, bar );
    tableSpinBox->resize( tableSpinBox->sizeHint() );

    connect( tableSpinBox, TQ_SIGNAL( valueChanged( int ) ), this, TQ_SLOT( tableChanged( int ) ) );

    TQLabel* const lUnicode = new TQLabel( i18n( "&Unicode code point:" ), bar );
    lUnicode->resize( lUnicode->sizeHint() );
    lUnicode->setAlignment( TQt::AlignRight | TQt::AlignVCenter );
    lUnicode->setMaximumWidth( lUnicode->sizeHint().width() );

    const TQRegExp rx( "[a-fA-F0-9]{1,4}" );
    TQValidator* const validator = new TQRegExpValidator( rx, this );

    d->unicodeLine = new KLineEdit( bar );
    d->unicodeLine->setValidator(validator);
    lUnicode->setBuddy(d->unicodeLine);
    d->unicodeLine->resize( d->unicodeLine->sizeHint() );
    slotUpdateUnicode(_chr);

    connect( d->unicodeLine, TQ_SIGNAL( returnPressed() ), this, TQ_SLOT( slotUnicodeEntered() ) );

    charTable = new KCharSelectTable( this, name, _font.isEmpty() ? TQString(TQVBox::font().family()) : _font, _chr, _tableNum );
    const TQSize sz( charTable->contentsWidth()  +  4 ,
                    charTable->contentsHeight() +  4 );
    charTable->resize( sz );
    //charTable->setMaximumSize( sz );
    charTable->setMinimumSize( sz );
    charTable->setHScrollBarMode( TQScrollView::AlwaysOff );
    charTable->setVScrollBarMode( TQScrollView::AlwaysOff );

    setFont( _font.isEmpty() ? TQString(TQVBox::font().family()) : _font );
    setTableNum( _tableNum );

    connect( charTable, TQ_SIGNAL( highlighted( const TQChar & ) ), this, TQ_SLOT( slotUpdateUnicode( const TQChar & ) ) );
    connect( charTable, TQ_SIGNAL( highlighted( const TQChar & ) ), this, TQ_SLOT( charHighlighted( const TQChar & ) ) );
    connect( charTable, TQ_SIGNAL( highlighted() ), this, TQ_SLOT( charHighlighted() ) );
    connect( charTable, TQ_SIGNAL( activated( const TQChar & ) ), this, TQ_SLOT( charActivated( const TQChar & ) ) );
    connect( charTable, TQ_SIGNAL( activated() ), this, TQ_SLOT( charActivated() ) );
    connect( charTable, TQ_SIGNAL( focusItemChanged( const TQChar & ) ),
	     this, TQ_SLOT( charFocusItemChanged( const TQChar & ) ) );
    connect( charTable, TQ_SIGNAL( focusItemChanged() ), this, TQ_SLOT( charFocusItemChanged() ) );
    connect( charTable, TQ_SIGNAL( tableUp() ), this, TQ_SLOT( charTableUp() ) );
    connect( charTable, TQ_SIGNAL( tableDown() ), this, TQ_SLOT( charTableDown() ) );

    connect( charTable, TQ_SIGNAL(doubleClicked()),this,TQ_SLOT(slotDoubleClicked()));

    setFocusPolicy( TQWidget::StrongFocus );
    setFocusProxy( charTable );
}

KCharSelect::~KCharSelect()
{
    delete d;
}

//==================================================================
TQSize KCharSelect::sizeHint() const
{
    return TQVBox::sizeHint();
}

//==================================================================
void KCharSelect::setFont( const TQString &_font )
{
    const TQValueList<TQString>::Iterator it = fontList.find( _font );
    if ( it != fontList.end() ) {
	TQValueList<TQString>::Iterator it2 = fontList.begin();
	int pos = 0;
	for ( ; it != it2; ++it2, ++pos);
	fontCombo->setCurrentItem( pos );
	charTable->setFont( _font );
    }
    else
	kdWarning() << "Can't find Font: " << _font << endl;
}

//==================================================================
void KCharSelect::setChar( const TQChar &_chr )
{
    charTable->setChar( _chr );
    slotUpdateUnicode( _chr );
}

//==================================================================
void KCharSelect::setTableNum( int _tableNum )
{
    tableSpinBox->setValue( _tableNum );
    charTable->setTableNum( _tableNum );
}

//==================================================================
void KCharSelect::fillFontCombo()
{
    if ( !fontDataBase ) {
	fontDataBase = new TQFontDatabase();
	tqAddPostRoutine( cleanupFontDatabase );
    }
    fontList=fontDataBase->families();
    fontCombo->insertStringList( fontList );
}

//==================================================================
void KCharSelect::fontSelected( const TQString &_font )
{
    charTable->setFont( _font );
    emit fontChanged( _font );
}

//==================================================================
void KCharSelect::tableChanged( int _value )
{
    charTable->setTableNum( _value );
}

//==================================================================
void KCharSelect::slotUnicodeEntered( )
{
    const TQString s = d->unicodeLine->text();
    if (s.isEmpty())
        return;
    
    bool ok;
    const int uc = s.toInt(&ok, 16);
    if (!ok)
        return;
    
    const int table = uc / 256;
    charTable->setTableNum( table );
    tableSpinBox->setValue(table);
    const TQChar ch(uc);
    charTable->setChar( ch );
    charActivated( ch );
}

void KCharSelect::slotUpdateUnicode( const TQChar &c )
{
    const int uc = c.unicode();
    TQString s;
    s.sprintf("%04X", uc);
    d->unicodeLine->setText(s);
}

void KCharSelectTable::virtual_hook( int, void*)
{ /*BASE::virtual_hook( id, data );*/ }

void KCharSelect::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

