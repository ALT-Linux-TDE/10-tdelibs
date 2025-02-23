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
// KColorDialog::getColor() so that it contains a parent argument. This
// improves centering capability.
//
// layout management added Oct 1997 by Mario Weilguni
// <mweilguni@sime.com>
//

#include <stdio.h>
#include <stdlib.h>

#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqdrawutil.h>
#include <tqevent.h>
#include <tqfile.h>
#include <tqimage.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqvalidator.h>
#include <tqpainter.h>
#include <tqpushbutton.h>
#include <tqspinbox.h>
#include <tqtimer.h>

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <tdelistbox.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kseparator.h>
#include <kpalette.h>
#include <kimageeffect.h>

#include "kcolordialog.h"
#include "kcolordrag.h"
#include "kstaticdeleter.h"
#include <config.h>
#include <kdebug.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef TQ_WS_X11
#include <X11/Xlib.h> 

// defined in qapplication_x11.cpp
typedef int (*QX11EventFilter) (XEvent*);
extern QX11EventFilter tqt_set_x11_event_filter (QX11EventFilter filter);
#endif

struct ColorPaletteNameType
{
    const char* m_fileName;
    const char* m_displayName;
};

const ColorPaletteNameType colorPaletteName[]=
{
    { "Recent_Colors", I18N_NOOP2( "palette name", "* Recent Colors *" ) },
    { "Custom_Colors", I18N_NOOP2( "palette name", "* Custom Colors *" ) },
    { "40.colors",     I18N_NOOP2( "palette name", "Forty Colors" ) },
    { "Rainbow.colors",I18N_NOOP2( "palette name", "Rainbow Colors" ) },
    { "Royal.colors",  I18N_NOOP2( "palette name", "Royal Colors" ) },
    { "Web.colors",    I18N_NOOP2( "palette name", "Web Colors" ) },
    { 0, 0 } // end of data
};

const int recentColorIndex = 0;
const int customColorIndex = 1;

class KColorSpinBox : public TQSpinBox
{
public:
  KColorSpinBox(int minValue, int maxValue, int step, TQWidget* parent)
   : TQSpinBox(minValue, maxValue, step, parent, "kcolorspinbox")
  { }

  // Override Qt's braindead auto-selection.
  virtual void valueChange()
  {
      updateDisplay();
      emit valueChanged( value() );
      emit valueChanged( currentValueText() );
  }

};


#define STANDARD_PAL_SIZE 17

KColor::KColor()
: TQColor()
{
  r = 0; g = 0; b = 0; h = 0; s = 0; v = 0;
}

KColor::KColor( const KColor &col)
: TQColor( col )
{
  h = col.h; s = col.s; v = col.v;
  r = col.r; g = col.g; b = col.b;
}

KColor::KColor( const TQColor &col)
: TQColor( col )
{
  TQColor::getRgb(&r, &g, &b);
  TQColor::getHsv(&h, &s, &v);
}

bool KColor::operator==(const KColor& col) const
{
  return (h == col.h) && (s == col.s) && (v == col.v) &&
         (r == col.r) && (g == col.g) && (b == col.b);
}

KColor& KColor::operator=(const KColor& col)
{
  *(TQColor *)this = col;
  h = col.h; s = col.s; v = col.v;
  r = col.r; g = col.g; b = col.b;
  return *this;
}

void
KColor::setHsv(int _h, int _s, int _v)
{
  h = _h; s = _s; v = _v;
  TQColor::setHsv(h, s, v);
  TQColor::rgb(&r, &g, &b);
}

void
KColor::setRgb(int _r, int _g, int _b)
{
  r = _r; g = _g; b = _b;
  TQColor::setRgb(r, g, b);
  TQColor::hsv(&h, &s, &v);
}

void
KColor::rgb(int *_r, int *_g, int *_b) const
{
  *_r = r; *_g = g; *_b = b;
}

void
KColor::hsv(int *_h, int *_s, int *_v) const
{
  *_h = h; *_s = s; *_v = v;
}


static TQColor *standardPalette = 0;
static KStaticDeleter<TQColor> spd;

static void createStandardPalette()
{
    if ( standardPalette )
	return;

    spd.setObject(standardPalette, new TQColor [STANDARD_PAL_SIZE], true/*array*/);

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


KHSSelector::KHSSelector( TQWidget *parent, const char *name )
	: KXYSelector( parent, name )
{
	setRange( 0, 0, 359, 255 );
}

void KHSSelector::updateContents()
{
	drawPalette(&pixmap);
}

void KHSSelector::resizeEvent( TQResizeEvent * )
{
	updateContents();
}

void KHSSelector::drawContents( TQPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KHSSelector::drawPalette( TQPixmap *pixmap )
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	TQImage image( xSize, ySize, 32 );
	TQColor col;
	int h, s;
	uint *p;

	for ( s = ySize-1; s >= 0; s-- )
	{
		p = (uint *) image.scanLine( ySize - s - 1 );
		for( h = 0; h < xSize; h++ )
		{
			col.setHsv( 359*h/(xSize-1), 255*s/((ySize == 1) ? 1 : ySize-1), 192 );
			*p = col.rgb();
			p++;
		}
	}

	if ( TQColor::numBitPlanes() <= 8 )
	{
		createStandardPalette();
		KImageEffect::dither( image, standardPalette, STANDARD_PAL_SIZE );
	}
	pixmap->convertFromImage( image );
}


//-----------------------------------------------------------------------------

KValueSelector::KValueSelector( TQWidget *parent, const char *name )
	: TDESelector( TQt::Vertical, parent, name ), _hue(0), _sat(0)
{
	setRange( 0, 255 );
	pixmap.setOptimization( TQPixmap::BestOptim );
}

KValueSelector::KValueSelector(Orientation o, TQWidget *parent, const char *name
 )
	: TDESelector( o, parent, name), _hue(0), _sat(0)
{
	setRange( 0, 255 );
	pixmap.setOptimization( TQPixmap::BestOptim );
}

void KValueSelector::updateContents()
{
	drawPalette(&pixmap);
}

void KValueSelector::resizeEvent( TQResizeEvent * )
{
	updateContents();
}

void KValueSelector::drawContents( TQPainter *painter )
{
	painter->drawPixmap( contentsRect().x(), contentsRect().y(), pixmap );
}

void KValueSelector::drawPalette( TQPixmap *pixmap )
{
	int xSize = contentsRect().width(), ySize = contentsRect().height();
	TQImage image( xSize, ySize, 32 );
	TQColor col;
	uint *p;
	TQRgb rgb;

	if ( orientation() == TQt::Horizontal )
	{
		for ( int v = 0; v < ySize; v++ )
		{
			p = (uint *) image.scanLine( ySize - v - 1 );

			for( int x = 0; x < xSize; x++ )
			{
				col.setHsv( _hue, _sat, 255*x/((xSize == 1) ? 1 : xSize-1) );
				rgb = col.rgb();
				*p++ = rgb;
			}
		}
	}

	if( orientation() == TQt::Vertical )
	{
		for ( int v = 0; v < ySize; v++ )
		{
			p = (uint *) image.scanLine( ySize - v - 1 );
			col.setHsv( _hue, _sat, 255*v/((ySize == 1) ? 1 : ySize-1) );
			rgb = col.rgb();
			for ( int i = 0; i < xSize; i++ )
				*p++ = rgb;
		}
	}

	if ( TQColor::numBitPlanes() <= 8 )
	{
		createStandardPalette();
		KImageEffect::dither( image, standardPalette, STANDARD_PAL_SIZE );
	}
	pixmap->convertFromImage( image );
}

//-----------------------------------------------------------------------------

KColorCells::KColorCells( TQWidget *parent, int rows, int cols )
	: TQGridView( parent )
{
	shade = true;
	setNumRows( rows );
	setNumCols( cols );
	colors = new TQColor [ rows * cols ];

	for ( int i = 0; i < rows * cols; i++ )
		colors[i] = TQColor();

	selected = 0;
        inMouse = false;

	// Drag'n'Drop
	setAcceptDrops( true);

	setHScrollBarMode( AlwaysOff );
	setVScrollBarMode( AlwaysOff );
	viewport()->setBackgroundMode( PaletteBackground );
	setBackgroundMode( PaletteBackground );
}

KColorCells::~KColorCells()
{
	delete [] colors;
}

void KColorCells::setColor( int colNum, const TQColor &col )
{
	colors[colNum] = col;
	updateCell( colNum/numCols(), colNum%numCols() );
}

void KColorCells::paintCell( TQPainter *painter, int row, int col )
{
	TQBrush brush;
        int w = 1;

	if (shade)
        {
		qDrawShadePanel( painter, 1, 1, cellWidth()-2,
		    cellHeight()-2, colorGroup(), true, 1, &brush );
		w = 2;
        }
        TQColor color = colors[ row * numCols() + col ];
        if (!color.isValid())
	{
		if (!shade) return;
		color = backgroundColor();
	}

	painter->setPen( color );
	painter->setBrush( TQBrush( color ) );
	painter->drawRect( w, w, cellWidth()-w*2, cellHeight()-w*2 );

	if ( row * numCols() + col == selected )
		painter->drawWinFocusRect( w, w, cellWidth()-w*2, cellHeight()-w*2 );
}

void KColorCells::resizeEvent( TQResizeEvent * )
{
	setCellWidth( width() / numCols() );
	setCellHeight( height() / numRows() );
}

void KColorCells::mousePressEvent( TQMouseEvent *e )
{
    inMouse = true;
    mPos = e->pos();
}

int KColorCells::posToCell(const TQPoint &pos, bool ignoreBorders)
{
   int row = pos.y() / cellHeight();
   int col = pos.x() / cellWidth();
   int cell = row * numCols() + col;

   if (!ignoreBorders)
   {
      int border = 2;
      int x = pos.x() - col * cellWidth();
      int y = pos.y() - row * cellHeight();
      if ( (x < border) || (x > cellWidth()-border) ||
           (y < border) || (y > cellHeight()-border))
         return -1;
   }
   return cell;
}

void KColorCells::mouseMoveEvent( TQMouseEvent *e )
{
    if( !(e->state() & TQt::LeftButton)) return;

    if(inMouse) {
        int delay = TDEGlobalSettings::dndEventDelay();
        if(e->x() > mPos.x()+delay || e->x() < mPos.x()-delay ||
           e->y() > mPos.y()+delay || e->y() < mPos.y()-delay){
            // Drag color object
            int cell = posToCell(mPos);
            if ((cell != -1) && colors[cell].isValid())
            {
               KColorDrag *d = new KColorDrag( colors[cell], this);
               d->dragCopy();
            }
        }
    }
}

void KColorCells::dragEnterEvent( TQDragEnterEvent *event)
{
     event->accept( acceptDrags && KColorDrag::canDecode( event));
}

void KColorCells::dropEvent( TQDropEvent *event)
{
     TQColor c;
     if( KColorDrag::decode( event, c)) {
          int cell = posToCell(event->pos(), true);
	  setColor(cell,c);
     }
}

void KColorCells::mouseReleaseEvent( TQMouseEvent *e )
{
	int cell = posToCell(mPos);
        int currentCell = posToCell(e->pos());

        // If we release the mouse in another cell and we don't have
        // a drag we should ignore this event.
        if (currentCell != cell)
           cell = -1;

	if ( (cell != -1) && (selected != cell) )
	{
		int prevSel = selected;
		selected = cell;
		updateCell( prevSel/numCols(), prevSel%numCols() );
		updateCell( cell/numCols(), cell%numCols() );
        }

        inMouse = false;
        if (cell != -1)
	    emit colorSelected( cell );
}

void KColorCells::mouseDoubleClickEvent( TQMouseEvent * /*e*/ )
{
  int cell = posToCell(mPos);

  if (cell != -1)
    emit colorDoubleClicked( cell );
}


//-----------------------------------------------------------------------------

KColorPatch::KColorPatch( TQWidget *parent ) : TQFrame( parent )
{
	setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
	colContext = 0;
	setAcceptDrops( true);
}

KColorPatch::~KColorPatch()
{
  if ( colContext )
    TQColor::destroyAllocContext( colContext );
}

void KColorPatch::setColor( const TQColor &col )
{
	if ( colContext )
		TQColor::destroyAllocContext( colContext );
	colContext = TQColor::enterAllocContext();
	color.setRgb( col.rgb() );
	color.alloc();
	TQColor::leaveAllocContext();

	TQPainter painter;

	painter.begin( this );
	drawContents( &painter );
	painter.end();
}

void KColorPatch::drawContents( TQPainter *painter )
{
	painter->setPen( color );
	painter->setBrush( TQBrush( color ) );
	painter->drawRect( contentsRect() );
}

void KColorPatch::mouseMoveEvent( TQMouseEvent *e )
{
        // Drag color object
        if( !(e->state() & TQt::LeftButton)) return;
	KColorDrag *d = new KColorDrag( color, this);
	d->dragCopy();
}

void KColorPatch::dragEnterEvent( TQDragEnterEvent *event)
{
     event->accept( KColorDrag::canDecode( event));
}

void KColorPatch::dropEvent( TQDropEvent *event)
{
     TQColor c;
     if( KColorDrag::decode( event, c)) {
	  setColor( c);
	  emit colorChanged( c);
     }
}

class KPaletteTable::KPaletteTablePrivate
{
public:
    TQMap<TQString,TQColor> m_namedColorMap;
};

KPaletteTable::KPaletteTable( TQWidget *parent, int minWidth, int cols)
    : TQWidget( parent ), cells(0), mPalette(0), mMinWidth(minWidth), mCols(cols)
{
  d = new KPaletteTablePrivate;
  
  i18n_namedColors  = i18n("Named Colors");

  TQStringList diskPaletteList = KPalette::getPaletteList();
  TQStringList paletteList;

  // We must replace the untranslated file names by translate names (of course only for KDE's standard palettes)
  for ( int i = 0; colorPaletteName[i].m_fileName; ++i )
  {
      diskPaletteList.remove( colorPaletteName[i].m_fileName );
      paletteList.append( i18n( "palette name", colorPaletteName[i].m_displayName ) );
  }
  paletteList += diskPaletteList;
  paletteList.append( i18n_namedColors );

  TQVBoxLayout *layout = new TQVBoxLayout( this );

  combo = new TQComboBox( false, this );
  combo->insertStringList( paletteList );
  layout->addWidget(combo);

  sv = new TQScrollView( this );
  TQSize cellSize = TQSize( mMinWidth, 120);
  sv->setHScrollBarMode( TQScrollView::AlwaysOff);
  sv->setVScrollBarMode( TQScrollView::AlwaysOn);
  TQSize minSize = TQSize(sv->verticalScrollBar()->width(), 0);
  minSize += TQSize(sv->frameWidth(), 0);
  minSize += TQSize(cellSize);
  sv->setFixedSize(minSize);
  layout->addWidget(sv);

  mNamedColorList = new TDEListBox( this, "namedColorList", 0 );
  mNamedColorList->setFixedSize(minSize);
  mNamedColorList->hide();
  layout->addWidget(mNamedColorList);
  connect( mNamedColorList, TQ_SIGNAL(highlighted( const TQString & )),
	   this, TQ_SLOT( slotColorTextSelected( const TQString & )) );

  setFixedSize( sizeHint());
  connect( combo, TQ_SIGNAL(activated(const TQString &)),
	this, TQ_SLOT(slotSetPalette( const TQString &)));
}

KPaletteTable::~KPaletteTable()
{
   delete mPalette;
   delete d;
}

TQString
KPaletteTable::palette() const
{
  return combo->currentText();
}


static const char * const *namedColorFilePath( void )
{
  //
  // 2000-02-05 Espen Sand.
  // Add missing filepaths here. Make sure the last entry is 0!
  //
  static const char * const path[] =
  {
#ifdef X11_RGBFILE
    X11_RGBFILE,
#endif
    0
  };
  return path;
}




void
KPaletteTable::readNamedColor( void )
{
  if( mNamedColorList->count() != 0 )
  {
    return; // Strings already present
  }

  TDEGlobal::locale()->insertCatalogue("tdelibs_colors");

  //
  // Code somewhat inspired by KPalette.
  //

  const char * const *path = namedColorFilePath();
  for( int i=0; path[i]; ++i )
  {
    TQFile paletteFile( path[i] );
    if( !paletteFile.open( IO_ReadOnly ) )
    {
      continue;
    }

    TQString line;
    TQStringList list;
    while( paletteFile.readLine( line, 100 ) != -1 )
    {
      int red, green, blue;
      int pos = 0;

      if( sscanf(line.ascii(), "%d %d %d%n", &red, &green, &blue, &pos ) == 3 )
      {
	//
	// Remove duplicates. Every name with a space and every name
	// that start with "gray".
	//
	TQString name = line.mid(pos).stripWhiteSpace();
	if( name.isNull() || name.find(' ') != -1 ||
	    name.find( "gray" ) != -1 ||  name.find( "grey" ) != -1 )
	{
	  continue;
	}

        const TQColor color ( red, green, blue );
        if ( color.isValid() )
        {
            const TQString colorName( i18n("color", name.latin1() ) );
            list.append( colorName );
            d->m_namedColorMap[ colorName ] = color;
        }
      }
    }

    list.sort();
    mNamedColorList->insertStringList( list );
    break;
  }

  if( mNamedColorList->count() == 0 )
  {
    //
    // Give the error dialog box a chance to center above the
    // widget (or dialog). If we had displayed it now we could get a
    // situation where the (modal) error dialog box pops up first
    // preventing the real dialog to become visible until the
    // error dialog box is removed (== bad UI).
    //
    TQTimer::singleShot( 10, this, TQ_SLOT(slotShowNamedColorReadError()) );
  }
}


void
KPaletteTable::slotShowNamedColorReadError( void )
{
  if( mNamedColorList->count() == 0 )
  {
    TQString msg = i18n(""
      "Unable to read X11 RGB color strings. The following "
      "file location(s) were examined:\n");

    const char * const *path = namedColorFilePath();
    for( int i=0; path[i]; ++i )
    {
      msg += path[i];
      msg += "\n";
    }
    KMessageBox::sorry( this, msg );
  }
}


//
// 2000-02-12 Espen Sand
// Set the color in two steps. The setPalette() slot will not emit a signal
// with the current color setting. The reason is that setPalette() is used
// by the color selector dialog on startup. In the color selector dialog
// we normally want to display a startup color which we specify
// when the dialog is started. The slotSetPalette() slot below will
// set the palette and then use the information to emit a signal with the
// new color setting. It is only used by the combobox widget.
//
void
KPaletteTable::slotSetPalette( const TQString &_paletteName )
{
  setPalette( _paletteName );
  if( mNamedColorList->isVisible() )
  {
    int item = mNamedColorList->currentItem();
    mNamedColorList->setCurrentItem( item < 0 ? 0 : item );
    slotColorTextSelected( mNamedColorList->currentText() );
  }
  else
  {
    slotColorCellSelected(0); // FIXME: We need to save the current value!!
  }
}


void
KPaletteTable::setPalette( const TQString &_paletteName )
{
  TQString paletteName( _paletteName);
  if (paletteName.isEmpty())
     paletteName = i18n_recentColors;

  if (combo->currentText() != paletteName)
  {
     bool found = false;
     for(int i = 0; i < combo->count(); i++)
     {
        if (combo->text(i) == paletteName)
        {
           combo->setCurrentItem(i);
           found = true;
           break;
        }
     }
     if (!found)
     {
        combo->insertItem(paletteName);
        combo->setCurrentItem(combo->count()-1);
     }
  }

  // We must again find the file name of the palette from the eventual translation
  for ( int i = 0; colorPaletteName[i].m_fileName; ++i )
  {
      if ( paletteName == i18n( "palette name", colorPaletteName[i].m_displayName ) )
      {
          paletteName = colorPaletteName[i].m_fileName;
          break;
      }
  }


  //
  // 2000-02-12 Espen Sand
  // The palette mode "i18n_namedColors" does not use the KPalette class.
  // In fact, 'mPalette' and 'cells' are 0 when in this mode. The reason
  // for this is maninly that KPalette reads from and writes to files using
  // "locate()". The colors used in "i18n_namedColors" mode comes from the
  // X11 diretory and is not writable. I don't think this fit in KPalette.
  //
  if( !mPalette || mPalette->name() != paletteName )
  {
    if( paletteName == i18n_namedColors )
    {
      sv->hide();
      mNamedColorList->show();
      readNamedColor();

      delete cells; cells = 0;
      delete mPalette; mPalette = 0;
    }
    else
    {
      mNamedColorList->hide();
      sv->show();

      delete cells;
      delete mPalette;
      mPalette = new KPalette(paletteName);
      int rows = (mPalette->nrColors()+mCols-1) / mCols;
      if (rows < 1) rows = 1;
      cells = new KColorCells( sv->viewport(), rows, mCols);
      cells->setShading(false);
      cells->setAcceptDrags(false);
      TQSize cellSize = TQSize( mMinWidth, mMinWidth * rows / mCols);
      cells->setFixedSize( cellSize );
      for( int i = 0; i < mPalette->nrColors(); i++)
      {
        cells->setColor( i, mPalette->color(i) );
      }
      connect( cells, TQ_SIGNAL( colorSelected( int ) ),
	       TQ_SLOT( slotColorCellSelected( int ) ) );
      connect( cells, TQ_SIGNAL( colorDoubleClicked( int ) ),
	       TQ_SLOT( slotColorCellDoubleClicked( int ) ) );
      sv->addChild( cells );
      cells->show();
      sv->updateScrollBars();
    }
  }
}



void
KPaletteTable::slotColorCellSelected( int col )
{
  if (!mPalette || (col >= mPalette->nrColors()))
     return;
  emit colorSelected( mPalette->color(col), mPalette->colorName(col) );
}

void
KPaletteTable::slotColorCellDoubleClicked( int col )
{
  if (!mPalette || (col >= mPalette->nrColors()))
     return;
  emit colorDoubleClicked( mPalette->color(col), mPalette->colorName(col) );
}


void
KPaletteTable::slotColorTextSelected( const TQString &colorText )
{
  emit colorSelected( d->m_namedColorMap[ colorText ], colorText );
}


void
KPaletteTable::addToCustomColors( const TQColor &color)
{
  setPalette(i18n( "palette name", colorPaletteName[ customColorIndex ].m_displayName ));
  mPalette->addColor( color );
  mPalette->save();
  delete mPalette;
  mPalette = 0;
  setPalette(i18n( "palette name", colorPaletteName[ customColorIndex ].m_displayName ));
}

void
KPaletteTable::addToRecentColors( const TQColor &color)
{
  //
  // 2000-02-12 Espen Sand.
  // The 'mPalette' is always 0 when current mode is i18n_namedColors
  //
  bool recentIsSelected = false;
  if ( mPalette && mPalette->name() == colorPaletteName[ recentColorIndex ].m_fileName )
  {
     delete mPalette;
     mPalette = 0;
     recentIsSelected = true;
  }
  KPalette *recentPal = new KPalette( colorPaletteName[ recentColorIndex ].m_fileName );
  if (recentPal->findColor(color) == -1)
  {
     recentPal->addColor( color );
     recentPal->save();
  }
  delete recentPal;
  if (recentIsSelected)
      setPalette( i18n( "palette name", colorPaletteName[ recentColorIndex ].m_displayName ) );
}

class KColorDialog::KColorDialogPrivate {
public:
    KPaletteTable *table;
    TQString originalPalette;
    bool bRecursion;
    bool bEditRgb;
    bool bEditHsv;
    bool bEditHtml;
    bool bColorPicking;
    TQLabel *colorName;
    KLineEdit *htmlName;
    KColorSpinBox *hedit;
    KColorSpinBox *sedit;
    KColorSpinBox *vedit;
    KColorSpinBox *redit;
    KColorSpinBox *gedit;
    KColorSpinBox *bedit;
    KColorPatch *patch;
    KHSSelector *hsSelector;
    KPalette *palette;
    KValueSelector *valuePal;
    TQVBoxLayout* l_right;
    TQGridLayout* tl_layout;
    TQCheckBox *cbDefaultColor;
    KColor defaultColor;
    KColor selColor;
#ifdef TQ_WS_X11
    QX11EventFilter oldfilter;
#endif
};


KColorDialog::KColorDialog( TQWidget *parent, const char *name, bool modal )
  :KDialogBase( parent, name, modal, i18n("Select Color"),
		modal ? Ok|Cancel : Close,
		Ok, true )
{
  d = new KColorDialogPrivate;
  d->bRecursion = true;
  d->bColorPicking = false;
#ifdef TQ_WS_X11
  d->oldfilter = 0;
#endif
  d->cbDefaultColor = 0L;
  connect( this, TQ_SIGNAL(okClicked(void)),this,TQ_SLOT(slotWriteSettings(void)));
  connect( this, TQ_SIGNAL(closeClicked(void)),this,TQ_SLOT(slotWriteSettings(void)));

  TQLabel *label;

  //
  // Create the top level page and its layout
  //
  TQWidget *page = new TQWidget( this );
  setMainWidget( page );

  TQGridLayout *tl_layout = new TQGridLayout( page, 3, 3, 0, spacingHint() );
  d->tl_layout = tl_layout;
  tl_layout->addColSpacing( 1, spacingHint() * 2 );

  //
  // the more complicated part: the left side
  // add a V-box
  //
  TQVBoxLayout *l_left = new TQVBoxLayout();
  tl_layout->addLayout(l_left, 0, 0);

  //
  // add a H-Box for the XY-Selector and a grid for the
  // entry fields
  //
  TQHBoxLayout *l_ltop = new TQHBoxLayout();
  l_left->addLayout(l_ltop);

  // a little space between
  l_left->addSpacing(10);

  TQGridLayout *l_lbot = new TQGridLayout(3, 6);
  l_left->addLayout(l_lbot);

  //
  // the palette and value selector go into the H-box
  //
  d->hsSelector = new KHSSelector( page );
  d->hsSelector->setMinimumSize(140, 70);
  l_ltop->addWidget(d->hsSelector, 8);
  connect( d->hsSelector, TQ_SIGNAL( valueChanged( int, int ) ),
	   TQ_SLOT( slotHSChanged( int, int ) ) );

  d->valuePal = new KValueSelector( page );
  d->valuePal->setMinimumSize(26, 70);
  l_ltop->addWidget(d->valuePal, 1);
  connect( d->valuePal, TQ_SIGNAL( valueChanged( int ) ),
	   TQ_SLOT( slotVChanged( int ) ) );


  //
  // add the HSV fields
  //
  label = new TQLabel( i18n("H:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 0, 2);
  d->hedit = new KColorSpinBox( 0, 359, 1, page );
  d->hedit->setValidator( new TQIntValidator( d->hedit ) );
  l_lbot->addWidget(d->hedit, 0, 3);
  connect( d->hedit, TQ_SIGNAL( valueChanged(int) ),
  	TQ_SLOT( slotHSVChanged() ) );

  label = new TQLabel( i18n("S:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 1, 2);
  d->sedit = new KColorSpinBox( 0, 255, 1, page );
  d->sedit->setValidator( new TQIntValidator( d->sedit ) );
  l_lbot->addWidget(d->sedit, 1, 3);
  connect( d->sedit, TQ_SIGNAL( valueChanged(int) ),
  	TQ_SLOT( slotHSVChanged() ) );

  label = new TQLabel( i18n("V:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 2, 2);
  d->vedit = new KColorSpinBox( 0, 255, 1, page );
  d->vedit->setValidator( new TQIntValidator( d->vedit ) );
  l_lbot->addWidget(d->vedit, 2, 3);
  connect( d->vedit, TQ_SIGNAL( valueChanged(int) ),
  	TQ_SLOT( slotHSVChanged() ) );

  //
  // add the RGB fields
  //
  label = new TQLabel( i18n("R:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 0, 4);
  d->redit = new KColorSpinBox( 0, 255, 1, page );
  d->redit->setValidator( new TQIntValidator( d->redit ) );
  l_lbot->addWidget(d->redit, 0, 5);
  connect( d->redit, TQ_SIGNAL( valueChanged(int) ),
  	TQ_SLOT( slotRGBChanged() ) );

  label = new TQLabel( i18n("G:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget( label, 1, 4);
  d->gedit = new KColorSpinBox( 0, 255,1, page );
  d->gedit->setValidator( new TQIntValidator( d->gedit ) );
  l_lbot->addWidget(d->gedit, 1, 5);
  connect( d->gedit, TQ_SIGNAL( valueChanged(int) ),
  	TQ_SLOT( slotRGBChanged() ) );

  label = new TQLabel( i18n("B:"), page );
  label->setAlignment(AlignRight | AlignVCenter);
  l_lbot->addWidget(label, 2, 4);
  d->bedit = new KColorSpinBox( 0, 255, 1, page );
  d->bedit->setValidator( new TQIntValidator( d->bedit ) );
  l_lbot->addWidget(d->bedit, 2, 5);
  connect( d->bedit, TQ_SIGNAL( valueChanged(int) ),
  	TQ_SLOT( slotRGBChanged() ) );

  //
  // the entry fields should be wide enough to hold 8888888
  //
  int w = d->hedit->fontMetrics().width("8888888");
  d->hedit->setFixedWidth(w);
  d->sedit->setFixedWidth(w);
  d->vedit->setFixedWidth(w);

  d->redit->setFixedWidth(w);
  d->gedit->setFixedWidth(w);
  d->bedit->setFixedWidth(w);

  //
  // add a layout for the right side
  //
  d->l_right = new TQVBoxLayout;
  tl_layout->addLayout(d->l_right, 0, 2);

  //
  // Add the palette table
  //
  d->table = new KPaletteTable( page );
  d->l_right->addWidget(d->table, 10);

  connect( d->table, TQ_SIGNAL( colorSelected( const TQColor &, const TQString & ) ),
	   TQ_SLOT( slotColorSelected( const TQColor &, const TQString & ) ) );

  connect(
    d->table,
    TQ_SIGNAL( colorDoubleClicked( const TQColor &, const TQString & ) ),
    TQ_SLOT( slotColorDoubleClicked( const TQColor &, const TQString & ) )
  );
  // Store the default value for saving time.
  d->originalPalette = d->table->palette();

  //
  // a little space between
  //
  d->l_right->addSpacing(10);

  TQHBoxLayout *l_hbox = new TQHBoxLayout( d->l_right );

  //
  // The add to custom colors button
  //
  TQPushButton *button = new TQPushButton( page );
  button->setText(i18n("&Add to Custom Colors"));
  l_hbox->addWidget(button, 0, AlignLeft);
  connect( button, TQ_SIGNAL( clicked()), TQ_SLOT( slotAddToCustomColors()));

  //
  // The color picker button
  //
  button = new TQPushButton( page );
  button->setPixmap( BarIcon("colorpicker"));
  l_hbox->addWidget(button, 0, AlignHCenter );
  connect( button, TQ_SIGNAL( clicked()), TQ_SLOT( slotColorPicker()));

  //
  // a little space between
  //
  d->l_right->addSpacing(10);

  //
  // and now the entry fields and the patch (=colored box)
  //
  TQGridLayout *l_grid = new TQGridLayout( d->l_right, 2, 3);

  l_grid->setColStretch(2, 1);

  label = new TQLabel( page );
  label->setText(i18n("Name:"));
  l_grid->addWidget(label, 0, 1, TQt::AlignLeft);

  d->colorName = new TQLabel( page );
  l_grid->addWidget(d->colorName, 0, 2, TQt::AlignLeft);

  label = new TQLabel( page );
  label->setText(i18n("HTML:"));
  l_grid->addWidget(label, 1, 1, TQt::AlignLeft);

  d->htmlName = new KLineEdit( page );
  d->htmlName->setMaxLength( 13 ); // Qt's TQColor allows 12 hexa-digits
  d->htmlName->setText("#FFFFFF"); // But HTML uses only 6, so do not worry about the size
  w = d->htmlName->fontMetrics().width(TQString::fromLatin1("#DDDDDDD"));
  d->htmlName->setFixedWidth(w);
  l_grid->addWidget(d->htmlName, 1, 2, TQt::AlignLeft);

  connect( d->htmlName, TQ_SIGNAL( textChanged(const TQString &) ),
      TQ_SLOT( slotHtmlChanged() ) );

  d->patch = new KColorPatch( page );
  d->patch->setFixedSize(48, 48);
  l_grid->addMultiCellWidget(d->patch, 0, 1, 0, 0, TQt::AlignHCenter | TQt::AlignVCenter);
  connect( d->patch, TQ_SIGNAL( colorChanged( const TQColor&)),
	   TQ_SLOT( setColor( const TQColor&)));

  tl_layout->activate();
  page->setMinimumSize( page->sizeHint() );

  readSettings();
  d->bRecursion = false;
  d->bEditHsv = false;
  d->bEditRgb = false;
  d->bEditHtml = false;

  disableResize();
  KColor col;
  col.setHsv( 0, 0, 255 );
  _setColor( col );

  d->htmlName->installEventFilter(this);
  d->hsSelector->installEventFilter(this);
  d->hsSelector->setAcceptDrops(true);
}

KColorDialog::~KColorDialog()
{
#ifdef TQ_WS_X11
    if (d->bColorPicking)
        tqt_set_x11_event_filter(d->oldfilter);
#endif
    delete d;
}

bool
KColorDialog::eventFilter( TQObject *obj, TQEvent *ev )
{
    if ((obj == d->htmlName) || (obj == d->hsSelector))
    switch(ev->type())
    {
      case TQEvent::DragEnter:
      case TQEvent::DragMove:
      case TQEvent::DragLeave:
      case TQEvent::Drop:
      case TQEvent::DragResponse:
            tqApp->sendEvent(d->patch, ev);
            return true;
      default:
            break;
    }
    return KDialogBase::eventFilter(obj, ev);
}

void
KColorDialog::setDefaultColor( const TQColor& col )
{
    if ( !d->cbDefaultColor )
    {
        //
        // a little space between
        //
        d->l_right->addSpacing(10);

        //
        // and the "default color" checkbox, under all items on the right side
        //
        d->cbDefaultColor = new TQCheckBox( i18n( "Default color" ), mainWidget() );
        d->cbDefaultColor->setChecked(true);

        d->l_right->addWidget( d->cbDefaultColor );

        mainWidget()->setMaximumSize( TQWIDGETSIZE_MAX, TQWIDGETSIZE_MAX ); // cancel setFixedSize()
        d->tl_layout->activate();
        mainWidget()->setMinimumSize( mainWidget()->sizeHint() );
        disableResize();

        connect( d->cbDefaultColor, TQ_SIGNAL( clicked() ), TQ_SLOT( slotDefaultColorClicked() ) );
    }

    d->defaultColor = col;

    slotDefaultColorClicked();
}

TQColor KColorDialog::defaultColor() const
{
    return d->defaultColor;
}

void KColorDialog::slotDefaultColorClicked()
{
    if ( d->cbDefaultColor->isChecked() )
    {
        d->selColor = d->defaultColor;
        showColor( d->selColor, i18n( "-default-" ) );
    } else
    {
        showColor( d->selColor, TQString::null );
    }
}

void
KColorDialog::readSettings()
{
  TDEConfigGroup group( TDEGlobal::config(), "Colors" );

  TQString palette = group.readEntry("CurrentPalette");
  d->table->setPalette(palette);
}

void
KColorDialog::slotWriteSettings()
{
  TDEConfigGroup group( TDEGlobal::config(), "Colors" );

  TQString palette = d->table->palette();
  if (!group.hasDefault("CurrentPalette") &&
      (d->table->palette() == d->originalPalette))
  {
     group.revertToDefault("CurrentPalette");
  }
  else
  {
     group.writeEntry("CurrentPalette", d->table->palette());
  }
}

TQColor
KColorDialog::color() const
{
  if ( d->cbDefaultColor && d->cbDefaultColor->isChecked() )
     return TQColor();
  if ( d->selColor.isValid() )
    d->table->addToRecentColors( d->selColor );
  return d->selColor;
}

void KColorDialog::setColor( const TQColor &col )
{
  _setColor( col );
}

//
// static function to display dialog and return color
//
int KColorDialog::getColor( TQColor &theColor, TQWidget *parent )
{
  KColorDialog dlg( parent, "Color Selector", true );
  if ( theColor.isValid() )
    dlg.setColor( theColor );
  int result = dlg.exec();

  if ( result == Accepted )
  {
    theColor = dlg.color();
  }

  return result;
}

//
// static function to display dialog and return color
//
int KColorDialog::getColor( TQColor &theColor, const TQColor& defaultCol, TQWidget *parent )
{
  KColorDialog dlg( parent, "Color Selector", true );
  dlg.setDefaultColor( defaultCol );
  dlg.setColor( theColor );
  int result = dlg.exec();

  if ( result == Accepted )
    theColor = dlg.color();

  return result;
}

void KColorDialog::slotRGBChanged( void )
{
  if (d->bRecursion) return;
  int red = d->redit->value();
  int grn = d->gedit->value();
  int blu = d->bedit->value();

  if ( red > 255 || red < 0 ) return;
  if ( grn > 255 || grn < 0 ) return;
  if ( blu > 255 || blu < 0 ) return;

  KColor col;
  col.setRgb( red, grn, blu );
  d->bEditRgb = true;
  _setColor( col );
  d->bEditRgb = false;
}

void KColorDialog::slotHtmlChanged( void )
{
  if (d->bRecursion || d->htmlName->text().isEmpty()) return;

  TQString strColor( d->htmlName->text() );

  // Assume that a user does not want to type the # all the time
  if ( strColor[0] != '#' )
  {
    bool signalsblocked = d->htmlName->signalsBlocked();
    d->htmlName->blockSignals(true);
    strColor.prepend("#");
    d->htmlName->setText(strColor);
    d->htmlName->blockSignals(signalsblocked);
  }

  const TQColor color( strColor );

  if ( color.isValid() )
  {
    KColor col( color );
    d->bEditHtml = true;
    _setColor( col );
    d->bEditHtml = false;
  }
}

void KColorDialog::slotHSVChanged( void )
{
  if (d->bRecursion) return;
  int hue = d->hedit->value();
  int sat = d->sedit->value();
  int val = d->vedit->value();

  if ( hue > 359 || hue < 0 ) return;
  if ( sat > 255 || sat < 0 ) return;
  if ( val > 255 || val < 0 ) return;

  KColor col;
  col.setHsv( hue, sat, val );
  d->bEditHsv = true;
  _setColor( col );
  d->bEditHsv = false;
}

void KColorDialog::slotHSChanged( int h, int s )
{
  int _h, _s, v;
  d->selColor.hsv(&_h, &_s, &v);
  if (v < 0)
     v = 0;
  KColor col;
  col.setHsv( h, s, v );
  _setColor( col );
}

void KColorDialog::slotVChanged( int v )
{
  int h, s, _v;
  d->selColor.hsv(&h, &s, &_v);
  KColor col;
  col.setHsv( h, s, v );
  _setColor( col );
}

void KColorDialog::slotColorSelected( const TQColor &color )
{
  _setColor( color );
}

void KColorDialog::slotAddToCustomColors( )
{
  d->table->addToCustomColors( d->selColor );
}

void KColorDialog::slotColorSelected( const TQColor &color, const TQString &name )
{
  _setColor( color, name);
}

void KColorDialog::slotColorDoubleClicked
(
  const TQColor  & color,
  const TQString & name
)
{
  _setColor(color, name);
  accept();
}

void KColorDialog::_setColor(const KColor &color, const TQString &name)
{
  if (color.isValid())
  {
     if (d->cbDefaultColor && d->cbDefaultColor->isChecked())
        d->cbDefaultColor->setChecked(false);
     d->selColor = color;
  }
  else
  {
     if (d->cbDefaultColor && d->cbDefaultColor->isChecked())
        d->cbDefaultColor->setChecked(true);
     d->selColor = d->defaultColor;
  }

  showColor( d->selColor, name );

  emit colorSelected( d->selColor );
}

// show but don't set into selColor, nor emit colorSelected
void KColorDialog::showColor( const KColor &color, const TQString &name )
{
  d->bRecursion = true;

  if (name.isEmpty())
     d->colorName->setText( i18n("-unnamed-"));
  else
     d->colorName->setText( name );

  d->patch->setColor( color );

  setRgbEdit( color );
  setHsvEdit( color );
  setHtmlEdit( color );

  int h, s, v;
  color.hsv( &h, &s, &v );
  d->hsSelector->setValues( h, s );
  d->valuePal->blockSignals(true);
  d->valuePal->setHue( h );
  d->valuePal->setSaturation( s );
  d->valuePal->setValue( v );
  d->valuePal->updateContents();
  d->valuePal->blockSignals(false);
  d->valuePal->repaint( false );
  d->bRecursion = false;
}


static TQWidget *kde_color_dlg_widget = 0;

#ifdef TQ_WS_X11
static int kde_color_dlg_handler(XEvent *event)
{
    if (event->type == ButtonRelease)
    {
        TQMouseEvent e( TQEvent::MouseButtonRelease, TQPoint(),
                       TQPoint(event->xmotion.x_root, event->xmotion.y_root) , 0, 0 );
        TQApplication::sendEvent( kde_color_dlg_widget, &e );
        return true;
    }
    return false;
}
#endif
void
KColorDialog::slotColorPicker()
{
    d->bColorPicking = true;
#ifdef TQ_WS_X11
    d->oldfilter = tqt_set_x11_event_filter(kde_color_dlg_handler);
#endif
    kde_color_dlg_widget = this;
    grabMouse( TQt::crossCursor );
    grabKeyboard();
}

void
KColorDialog::mouseReleaseEvent( TQMouseEvent *e )
{
  if (d->bColorPicking)
  {
     d->bColorPicking = false;
#ifdef TQ_WS_X11
     tqt_set_x11_event_filter(d->oldfilter);
     d->oldfilter = 0;
#endif
     releaseMouse();
     releaseKeyboard();
     _setColor( grabColor( e->globalPos() ) );
     return;
  }
  KDialogBase::mouseReleaseEvent( e );
}

TQColor
KColorDialog::grabColor(const TQPoint &p)
{
    TQWidget *desktop = TQApplication::desktop();
    TQPixmap pm = TQPixmap::grabWindow( desktop->winId(), p.x(), p.y(), 1, 1);
    TQImage i = pm.convertToImage();
    return i.pixel(0,0);
}

void
KColorDialog::keyPressEvent( TQKeyEvent *e )
{
  if (d->bColorPicking)
  {
     if (e->key() == Key_Escape)
     {
        d->bColorPicking = false;
#ifdef TQ_WS_X11
        tqt_set_x11_event_filter(d->oldfilter);
        d->oldfilter = 0;
#endif
        releaseMouse();
        releaseKeyboard();
     }
     e->accept();
     return;
  }
  KDialogBase::keyPressEvent( e );
}

void KColorDialog::setRgbEdit( const KColor &col )
{
  if (d->bEditRgb) return;
  int r, g, b;
  col.rgb( &r, &g, &b );

  d->redit->setValue( r );
  d->gedit->setValue( g );
  d->bedit->setValue( b );
}

void KColorDialog::setHtmlEdit( const KColor &col )
{
  if (d->bEditHtml) return;
  int r, g, b;
  col.rgb( &r, &g, &b );
  TQString num;

  num.sprintf("#%02X%02X%02X", r,g,b);
  d->htmlName->setText( num );
}


void KColorDialog::setHsvEdit( const KColor &col )
{
  if (d->bEditHsv) return;
  int h, s, v;
  col.hsv( &h, &s, &v );

  d->hedit->setValue( h );
  d->sedit->setValue( s );
  d->vedit->setValue( v );
}

void KHSSelector::virtual_hook( int id, void* data )
{ KXYSelector::virtual_hook( id, data ); }

void KValueSelector::virtual_hook( int id, void* data )
{ TDESelector::virtual_hook( id, data ); }

void KPaletteTable::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KColorCells::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KColorPatch::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KColorDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }


#include "kcolordialog.moc"
//#endif
