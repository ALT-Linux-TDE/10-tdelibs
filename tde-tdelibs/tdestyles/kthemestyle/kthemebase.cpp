/*
 $Id$

 This file is part of the KDE libraries
 Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

 KDE3 port (C) 2001-2002 Maksim Orlovich <mo002j@mail.rochester.edu>
 Port version 0.9.7

 Palette setup code is from TDEApplication,
		Copyright (C) 1997 Matthias Kalle Dalheimer (kalle@kde.org)
		Copyright (C) 1998, 1999, 2000 KDE Team

  Includes code portions from the KDE HighColor style.

		KDE3 HighColor Style
		Copyright (C) 2001 Karol Szwed       <gallium@kde.org>
		(C) 2001 Fredrik H�glund   <fredrik@kde.org>

		Drawing routines adapted from the KDE2 HCStyle,
		Copyright (C) 2000 Daniel M. Duley   <mosfet@kde.org>
		(C) 2000 Dirk Mueller      <mueller@kde.org>
		(C) 2001 Martijn Klingens  <klingens@kde.org>



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

#include "kthemebase.h"
#include <kpixmapeffect.h>
#include <tqimage.h>
#include <tqpainter.h>
#include <tqbitmap.h>
#include <stdlib.h>

#include <tqsettings.h>
#include <tqapplication.h>
#include <tqscrollbar.h>

typedef TQMap<TQString, TQString> Prop;

template class TQIntCache<KThemePixmap>
;

/*
Bugs:
Can't delete old slider image when calculating the rotated one for some reason.
*/

//Shamelessly stolen from TDEConfigBase
static TQColor readColorEntry( TQSettings* s, const char *pKey,
                              const TQColor* pDefault )
{
    TQColor aRetColor;
    int nRed = 0, nGreen = 0, nBlue = 0;

    TQString aValue = s->readEntry( pKey );
    if ( !aValue.isEmpty() )
    {
        if ( aValue.at( 0 ) == '#' )
        {
            aRetColor.setNamedColor( aValue );
        }
        else
        {
            bool bOK;
            // find first part (red)
            int nIndex = aValue.find( ',' );
            if ( nIndex == -1 )
            {
                // return a sensible default -- Bernd
                if ( pDefault )
                    aRetColor = *pDefault;
                return aRetColor;
            }

            nRed = aValue.left( nIndex ).toInt( &bOK );

            // find second part (green)
            int nOldIndex = nIndex;
            nIndex = aValue.find( ',', nOldIndex + 1 );

            if ( nIndex == -1 )
            {
                // return a sensible default -- Bernd
                if ( pDefault )
                    aRetColor = *pDefault;
                return aRetColor;
            }
            nGreen = aValue.mid( nOldIndex + 1,
                                 nIndex - nOldIndex - 1 ).toInt( &bOK );

            // find third part (blue)
            nBlue = aValue.right( aValue.length() - nIndex - 1 ).toInt( &bOK );

            aRetColor.setRgb( nRed, nGreen, nBlue );
        }
    }
    else
    {

        if ( pDefault )
            aRetColor = *pDefault;
    }

    return aRetColor;
}


static const char * const widgetEntries[] =
    { // unsunken widgets (see header)
        "PushButton", "ComboBox", "HSBarSlider", "VSBarSlider", "Bevel", "ToolButton",
        "ScrollButton", "HScrollDeco", "VScrollDeco", "ComboDeco", "MenuItem", "Tab",
        "ArrowUp", "ArrowDown", "ArrowLeft", "ArrowRight",
        // sunken widgets
        "PushButtonDown", "ComboBoxDown", "HSBarSliderDown", "VSBarSliderDown",
        "BevelDown", "ToolButtonDown", "ScrollButtonDown", "HScrollDecoDown",
        "VScrollDecoDown", "ComboDecoDown", "MenuItemDown", "TabDown", "SunkenArrowUp",
        "SunkenArrowDown", "SunkenArrowLeft", "SunkenArrowRight",
        // everything else
        "HScrollGroove", "VScrollGroove", "Slider", "SliderGroove", "CheckBoxDown",
        "CheckBox", "CheckBoxTri", "RadioDown", "Radio", "HBarHandle", "VBarHandle",
        "ToolBar", "Splitter", "CheckMark", "MenuBar", "DisableArrowUp",
        "DisableArrowDown", "DisableArrowLeft", "DisableArrowRight", "ProgressBar",
        "ProgressBackground", "MenuBarItem", "Background", "RotSlider", 
        "RotInactiveTab", "RotActiveTab",
    };

#define INHERIT_ITEMS 16


class KThemeBasePrivate
{
public:
    /** Color overrides flags..*/
    bool overrideForeground;
    bool overrideBackground;
    bool overrideSelectForeground;
    bool overrideSelectBackground;
    bool overrideWindowForeground;
    bool overrideWindowBackground;

    /**
    * Colors to override defaults with..
    */
    TQColor overrideForegroundCol;
    TQColor overrideBackgroundCol;
    TQColor overrideSelectForegroundCol;
    TQColor overrideSelectBackgroundCol;
    TQColor overrideWindowForegroundCol;
    TQColor overrideWindowBackgroundCol;

    int contrast;


    TQMap <TQString, TQMap<TQString, TQString> > props;

    TQMap<const TQPixmap*, TQColor> colorCache;

    /*
    A heuristic routine that tries to determine the  avergae color of the image
    Wouldn't work for things like sliders, etc.
    */
    TQColor pixmapAveColor( const TQPixmap* p )
    {
        if ( colorCache.contains( p ) )
            return colorCache[ p ];

        TQImage to_ave = p->convertToImage();
        double h = 0, s = 0, v = 0;
        int count = 0;
        int dh, ds, dv;
        for ( int x = 0; x < p->width(); x++ )
        {
            TQColor pix( to_ave.pixel( x, p->height() / 2 ) );
            pix.hsv( &dh, &ds, &dv );
            h += dh;
            s += ds;
            v += dv;
            count++;
        }

        for ( int y = 0; y < p->height(); y++ )
        {
            TQColor pix( to_ave.pixel( p->width() / 2, y ) );
            pix.hsv( &dh, &ds, &dv );
            h += dh;
            s += ds;
            v += dv;
            count++;
        }
        colorCache[ p ] = TQColor( int( h / count + 0.5 ), int( s / count + 0.5 ), int( v / count + 0.5 ), TQColor::Hsv );
        return colorCache[ p ];
    }
};



// This is used to encode the keys. I used to use masks but I think this
// bitfield is nicer :) I don't know why C++ coders don't use these more..
// (mosfet)
struct kthemeKeyData
{
unsigned int id :
    6;
unsigned int width :
    12;
unsigned int height :
    12;
unsigned int border :
    1;
unsigned int mask :
    1;
};

union kthemeKey{
    kthemeKeyData data;
    unsigned int cacheKey;
};

#define KDE_TQBITMAP_TO_TQPAINTDEVICE(x) static_cast<const TQPixmap*>(static_cast<const TQBitmap*>(x))

void KThemeBase::generateBorderPix( int i )
{
    // separate pixmap into separate components
    if ( pbPixmaps[ i ] )
    {
        // evidently I have to do masks manually...
        const TQBitmap * srcMask = pbPixmaps[ i ] ->mask();
        TQBitmap destMask( pbWidth[ i ], pbWidth[ i ] );
        TQPixmap tmp( pbWidth[ i ], pbWidth[ i ] );

        bitBlt( &tmp, 0, 0, pbPixmaps[ i ], 0, 0, pbWidth[ i ], pbWidth[ i ],
                TQt::CopyROP, false );
        if ( srcMask )
        {
            bitBlt( &destMask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0, pbWidth[ i ], pbWidth[ i ],
                    TQt::CopyROP, false );
            tmp.setMask( destMask );
        }
        pbPixmaps[ i ] ->setBorder( KThemePixmap::TopLeft, tmp );

        bitBlt( &tmp, 0, 0, pbPixmaps[ i ], pbPixmaps[ i ] ->width() - pbWidth[ i ], 0,
                pbWidth[ i ], pbWidth[ i ], TQt::CopyROP, false );
        if ( srcMask )
        {
            bitBlt( &destMask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), pbPixmaps[ i ] ->width() - pbWidth[ i ],
                    0, pbWidth[ i ], pbWidth[ i ], TQt::CopyROP, false );
            tmp.setMask( destMask );
        }
        pbPixmaps[ i ] ->setBorder( KThemePixmap::TopRight, tmp );

        bitBlt( &tmp, 0, 0, pbPixmaps[ i ], 0, pbPixmaps[ i ] ->height() - pbWidth[ i ],
                pbWidth[ i ], pbWidth[ i ], TQt::CopyROP, false );
        if ( srcMask )
        {
            bitBlt( &destMask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, pbPixmaps[ i ] ->height() - pbWidth[ i ],
                    pbWidth[ i ], pbWidth[ i ], TQt::CopyROP, false );
            tmp.setMask( destMask );
        }
        pbPixmaps[ i ] ->setBorder( KThemePixmap::BottomLeft, tmp );

        bitBlt( &tmp, 0, 0, pbPixmaps[ i ], pbPixmaps[ i ] ->width() - pbWidth[ i ],
                pbPixmaps[ i ] ->height() - pbWidth[ i ], pbWidth[ i ], pbWidth[ i ],
                TQt::CopyROP, false );
        if ( srcMask )
        {
            bitBlt( &destMask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), pbPixmaps[ i ] ->width() - pbWidth[ i ],
                    pbPixmaps[ i ] ->height() - pbWidth[ i ], pbWidth[ i ], pbWidth[ i ],
                    TQt::CopyROP, false );
            tmp.setMask( destMask );
        }
        pbPixmaps[ i ] ->setBorder( KThemePixmap::BottomRight, tmp );

        tmp.resize( pbPixmaps[ i ] ->width() - pbWidth[ i ] * 2, pbWidth[ i ] );
        destMask.resize( pbPixmaps[ i ] ->width() - pbWidth[ i ] * 2, pbWidth[ i ] );
        bitBlt( &tmp, 0, 0, pbPixmaps[ i ], pbWidth[ i ], 0,
                pbPixmaps[ i ] ->width() - pbWidth[ i ] * 2, pbWidth[ i ], TQt::CopyROP, false );
        if ( srcMask )
        {
            bitBlt( &destMask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), pbWidth[ i ], 0,
                    pbPixmaps[ i ] ->width() - pbWidth[ i ] * 2, pbWidth[ i ],
                    TQt::CopyROP, false );
            tmp.setMask( destMask );
        }
        pbPixmaps[ i ] ->setBorder( KThemePixmap::Top, tmp );

        bitBlt( &tmp, 0, 0, pbPixmaps[ i ], pbWidth[ i ],
                pbPixmaps[ i ] ->height() - pbWidth[ i ],
                pbPixmaps[ i ] ->width() - pbWidth[ i ] * 2, pbWidth[ i ], TQt::CopyROP, false );
        if ( srcMask )
        {
            bitBlt( &destMask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), pbWidth[ i ],
                    pbPixmaps[ i ] ->height() - pbWidth[ i ],
                    pbPixmaps[ i ] ->width() - pbWidth[ i ] * 2, pbWidth[ i ], TQt::CopyROP, false );
            tmp.setMask( destMask );
        }
        pbPixmaps[ i ] ->setBorder( KThemePixmap::Bottom, tmp );

        tmp.resize( pbWidth[ i ], pbPixmaps[ i ] ->height() - pbWidth[ i ] * 2 );
        destMask.resize( pbWidth[ i ], pbPixmaps[ i ] ->height() - pbWidth[ i ] * 2 );
        bitBlt( &tmp, 0, 0, pbPixmaps[ i ], 0, pbWidth[ i ], pbWidth[ i ],
                pbPixmaps[ i ] ->height() - pbWidth[ i ] * 2, TQt::CopyROP, false );
        if ( srcMask )
        {
            bitBlt( &destMask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, pbWidth[ i ], pbWidth[ i ],
                    pbPixmaps[ i ] ->height() - pbWidth[ i ] * 2, TQt::CopyROP, false );
            tmp.setMask( destMask );
        }

        pbPixmaps[ i ] ->setBorder( KThemePixmap::Left, tmp );

        bitBlt( &tmp, 0, 0, pbPixmaps[ i ], pbPixmaps[ i ] ->width() - pbWidth[ i ],
                pbWidth[ i ], pbWidth[ i ], pbPixmaps[ i ] ->height() - pbWidth[ i ] * 2,
                TQt::CopyROP, false );
        if ( srcMask )
        {
            bitBlt( &destMask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), pbPixmaps[ i ] ->width() - pbWidth[ i ],
                    pbWidth[ i ], pbWidth[ i ], pbPixmaps[ i ] ->height() - pbWidth[ i ] * 2,
                    TQt::CopyROP, false );
            tmp.setMask( destMask );
        }
        pbPixmaps[ i ] ->setBorder( KThemePixmap::Right, tmp );
    }
    else
        tqWarning( "KThemeBase: Tried making border from empty pixmap\n" );
}


void KThemeBase::copyWidgetConfig( int sourceID, int destID, TQString *pixnames,
                                   TQString *brdnames )
{
    scaleHints[ destID ] = scaleHints[ sourceID ];
    gradients[ destID ] = gradients[ sourceID ];
    blends[ destID ] = blends[ sourceID ];
    bContrasts[ destID ] = bContrasts[ sourceID ];
    borders[ destID ] = borders[ sourceID ];
    highlights[ destID ] = highlights[ sourceID ];

    if ( grLowColors[ sourceID ] )
        grLowColors[ destID ] = new TQColor( *grLowColors[ sourceID ] );
    else
        grLowColors[ destID ] = NULL;

    if ( grHighColors[ sourceID ] )
        grHighColors[ destID ] = new TQColor( *grHighColors[ sourceID ] );
    else
        grHighColors[ destID ] = NULL;

    if ( colors[ sourceID ] )
        colors[ destID ] = new TQColorGroup( *colors[ sourceID ] );
    else
        colors[ destID ] = NULL;

    // pixmap
    pixnames[ destID ] = pixnames[ sourceID ];
    duplicate[ destID ] = false;
    pixmaps[ destID ] = NULL;
    images[ destID ] = NULL;
    if ( !pixnames[ destID ].isEmpty() )
    {
        if ( scaleHints[ sourceID ] == TileScale && blends[ sourceID ] == 0.0 )
        {
            pixmaps[ destID ] = pixmaps[ sourceID ];
            duplicate[ destID ] = true;
        }
        if ( !duplicate[ destID ] )
        {
            pixmaps[ destID ] = loadPixmap( pixnames[ destID ] );
            if ( scaleHints[ destID ] == TileScale && blends[ destID ] == 0.0 )
                images[ destID ] = NULL;
            else
                images[ destID ] = loadImage( pixnames[ destID ] );
        }
    }

    // border pixmap
    pbDuplicate[ destID ] = false;
    pbPixmaps[ destID ] = NULL;
    pbWidth[ destID ] = pbWidth[ sourceID ];
    brdnames[ destID ] = brdnames[ sourceID ];
    if ( !brdnames[ destID ].isEmpty() )
    {
        pbPixmaps[ destID ] = pbPixmaps[ sourceID ];
        pbDuplicate[ destID ] = true;
    }

    if ( sourceID == ActiveTab && destID == InactiveTab )
        aTabLine = iTabLine;
    else if ( sourceID == InactiveTab && destID == ActiveTab )
        iTabLine = aTabLine;
}

void KThemeBase::readConfig( TQt::GUIStyle /*style*/ )
{
#define PREBLEND_ITEMS 12
    static const WidgetType preBlend[] =
        {
            Slider, IndicatorOn, IndicatorOff,
            ExIndicatorOn, ExIndicatorOff, HScrollDeco, VScrollDeco, HScrollDecoDown,
            VScrollDecoDown, ComboDeco, ComboDecoDown, CheckMark
        };

    int i;
    TQString tmpStr;
    TQString pixnames[ WIDGETS ]; // used for duplicate check
    TQString brdnames[ WIDGETS ];
    bool loaded[ WIDGETS ]; // used for preloading for CopyWidget

    TQSettings config;
    if (configDirName.isEmpty() || configDirName == ".")
    {
    	TDEStyleDirs::dirs()->addToSearch( "themerc", config );
    }
    else config.insertSearchPath( TQSettings::Unix, configDirName );

    applyConfigFile( config );

    d->contrast = config.readNumEntry( configFileName + "KDE/contrast", 7 );



    for ( i = 0; i < INHERIT_ITEMS; ++i )
        applyResourceGroup( &config, i );
    for ( ; i < INHERIT_ITEMS*2; ++i )
    {
        if ( config.entryList( configFileName + widgetEntries[ i ] ).size() )
            applyResourceGroup( &config, i );
#ifndef TQ_WS_QWS //FIXME

        else
        {
            Prop& copyProp = d->props[ widgetEntries[ i ] ];
            copyProp[ "CopyWidget" ] = TQString( widgetEntries[ i - INHERIT_ITEMS ] );
        }
#endif

    }
    for ( ; i < WIDGETS; ++i )
        applyResourceGroup( &config, i );
    applyMiscResourceGroup( &config );

    // initialize defaults that may not be read
    for ( i = 0; i < WIDGETS; ++i )
        loaded[ i ] = false;
    btnXShift = btnYShift = focus3DOffset = 0;
    aTabLine = iTabLine = true;
    roundedButton = roundedCombo = roundedSlider = focus3D = false;
    splitterWidth = 10;

    //Handle the rotated background separately..
    d->props[ widgetEntries[ RotSliderGroove ] ] = d->props[ widgetEntries[ SliderGroove ] ];
    d->props[ widgetEntries[ RotInactiveTab ] ]  = d->props[ widgetEntries[ InactiveTab ] ];
    d->props[ widgetEntries[ RotActiveTab ] ]    = d->props[ widgetEntries[ ActiveTab ] ];

    // misc items
    readMiscResourceGroup();


    for ( i = 0; i < WIDGETS; ++i )
        readResourceGroup( i, pixnames, brdnames, loaded );

    if ( pixmaps[ RotSliderGroove ] )
    {
        TQWMatrix r270; //TODO: 90 if reverse?
        r270.rotate( 270 );
        KThemePixmap* bf = new KThemePixmap( pixmaps[ RotSliderGroove ], pixmaps[ RotSliderGroove ] ->xForm( r270 ) ); //
        pixmaps[ RotSliderGroove ] = bf;
        if ( images[ RotSliderGroove ] )
        {
            delete images[ RotSliderGroove ];
            images[ RotSliderGroove ] = new TQImage( bf->convertToImage() );
        }
    }

    if ( pixmaps[ RotActiveTab ] )
    {
        TQWMatrix r180; 
        r180.rotate( 180 );
        KThemePixmap* bf = new KThemePixmap( pixmaps[ RotActiveTab ], pixmaps[ RotActiveTab ] ->xForm( r180 ) );
        
        pixmaps[ RotActiveTab ] = bf;
        if ( images[ RotActiveTab ] )
        {
            delete images[ RotActiveTab ];
            images[ RotActiveTab ] = new TQImage( bf->convertToImage() );
        }
    }
    
    if ( pixmaps[ RotInactiveTab ] )
    {
        TQWMatrix r180; 
        r180.rotate( 180 );
        KThemePixmap* bf = new KThemePixmap( pixmaps[ RotInactiveTab ], pixmaps[ RotInactiveTab ] ->xForm( r180 ) );
        
        pixmaps[ RotInactiveTab ] = bf;
        if ( images[ RotInactiveTab ] )
        {
            delete images[ RotInactiveTab ];
            images[ RotInactiveTab ] = new TQImage( bf->convertToImage() );
        }
    }

    // Handle preblend items
    for ( i = 0; i < PREBLEND_ITEMS; ++i )
    {
        if ( pixmaps[ preBlend[ i ] ] != NULL && blends[ preBlend[ i ] ] != 0.0 )
            blend( preBlend[ i ] );
    }

    d->props.clear();
}

KThemeBase::KThemeBase( const TQString& dir, const TQString & configFile )
        : TDEStyle( FilledFrameWorkaround ), configFileName( configFile )
{
    d = new KThemeBasePrivate;
    if ( configFileName.isEmpty() )
        configFileName = "tdestylerc";


    configDirName = dir;
    //Strip of rc from the configFileName
    if ( configFileName.endsWith( "rc" ) )
    {
        configFileName.truncate( configFileName.length() - 2 ); //Get rid of rc..
    }
    //else SCREAM!!


    configFileName = "/" + configFileName + "/";

    readConfig( TQt::WindowsStyle );
    cache = new KThemeCache( cacheSize );

    switch ( scrollBarLayout() )
    {
        case SBBottomLeft:
            setScrollBarType( NextStyleScrollBar );
            break;
        case SBBottomRight:
            setScrollBarType( PlatinumStyleScrollBar );
            break;
        case SBOpposite:
            break;
            //Do nothing, this type already set..
    }
    ;
}

void KThemeBase::applyConfigFile( TQSettings& config )
{
    TQStringList keys = config.entryList( configFileName );

    if ( keys.contains( "foreground" ) )
    {
        d->overrideForeground = true;
        d->overrideForegroundCol = readColorEntry( &config, ( configFileName + "foreground" ).latin1(), 0 );
    }
    else
        d->overrideForeground = false;

    if ( keys.contains( "background" ) )
    {
        d->overrideBackground = true;
        d->overrideBackgroundCol = readColorEntry( &config, ( configFileName + "background" ).latin1(), 0 );
    }
    else
        d->overrideBackground = false;



    if ( keys.contains( "selectForeground" ) )
    {
        d->overrideSelectForeground = true;
        d->overrideSelectForegroundCol = readColorEntry( &config, ( configFileName + "selectForeground" ).latin1(), 0 );
    }
    else
        d->overrideSelectForeground = false;

    if ( keys.contains( "selectBackground" ) )
    {
        d->overrideSelectBackground = true;
        d->overrideSelectBackgroundCol = readColorEntry( &config, ( configFileName + "selectBackground" ).latin1(), 0 );
    }
    else
        d->overrideSelectBackground = false;

    if ( keys.contains( "windowBackground" ) )
    {
        d->overrideWindowBackground = true;
        d->overrideWindowBackgroundCol = readColorEntry( &config, ( configFileName + "windowBackground" ).latin1(), 0 );
    }
    else
        d->overrideWindowBackground = false;


    if ( keys.contains( "windowForeground" ) )
    {
        d->overrideWindowForeground = true;
        d->overrideWindowForegroundCol = readColorEntry( &config, ( configFileName + "windowForeground" ).latin1(), 0 );
    }
    else
        d->overrideWindowForeground = false;


#ifndef TQ_WS_QWS //FIXME

    for ( int input = 0; input < WIDGETS; ++input )
    {
        d->props.erase( widgetEntries[ input ] );
    }
    d->props.erase( "Misc" );
#endif
}

KThemeBase::~KThemeBase()
{
    int i;
    for ( i = 0; i < WIDGETS; ++i )
    {
        if ( !duplicate[ i ] )
        {
            if ( images[ i ] )
                delete images[ i ];
            if ( pixmaps[ i ] )
                delete pixmaps[ i ];
        }
        if ( !pbDuplicate[ i ] && pbPixmaps[ i ] )
            delete pbPixmaps[ i ];
        if ( colors[ i ] )
            delete( colors[ i ] );
        if ( grLowColors[ i ] )
            delete( grLowColors[ i ] );
        if ( grHighColors[ i ] )
            delete( grHighColors[ i ] );
    }
    TDEStyleDirs::release();
    delete cache;
    delete d;
}

TQImage* KThemeBase::loadImage( const TQString &name )
{
    TQImage * image = new TQImage;
    TQString path = TDEStyleDirs::dirs()->findResource( "themepixmap",name );
    image->load( path );
    if ( !image->isNull() )
        return ( image );
    tqWarning( "KThemeBase: Unable to load image %s\n", name.latin1() );
    delete image;
    return ( NULL );
}

KThemePixmap* KThemeBase::loadPixmap( const TQString &name )
{
    KThemePixmap * pixmap = new KThemePixmap( false );
    TQString path = TDEStyleDirs::dirs()->findResource( "themepixmap", name );
    pixmap->load( path );
    if ( !pixmap->isNull() )
        return pixmap;
    tqWarning( "KThemeBase: Unable to load pixmap %s\n", name.latin1() );
    delete pixmap;
    return ( NULL );
}


KThemePixmap* KThemeBase::scale( int w, int h, WidgetType widget ) const
{
    if ( scaleHints[ widget ] == FullScale )
    {
        if ( !pixmaps[ widget ] || pixmaps[ widget ] ->width() != w ||
                pixmaps[ widget ] ->height() != h )
        {
            KThemePixmap * cachePix = cache->pixmap( w, h, widget );
            if ( cachePix )
            {
                cachePix = new KThemePixmap( *cachePix );
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::FullScale,
                                   widget );
                else
                    tqWarning( "We would have inserted a null pixmap!\n" );
                pixmaps[ widget ] = cachePix;
            }
            else
            {
                cache->insert( pixmaps[ widget ], KThemeCache::FullScale, widget );
                TQImage tmpImg = images[ widget ] ->smoothScale( w, h );
                pixmaps[ widget ] = new KThemePixmap;
                pixmaps[ widget ] ->convertFromImage( tmpImg );
                if ( blends[ widget ] != 0.0 )
                    blend( widget );
            }
        }
    }
    else if ( scaleHints[ widget ] == HorizontalScale )
    {
        if ( pixmaps[ widget ] ->width() != w )
        {
            KThemePixmap * cachePix = cache->horizontalPixmap( w, widget );
            if ( cachePix )
            {
                cachePix = new KThemePixmap( *cachePix );
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::HorizontalScale, widget );
                else
                    tqWarning( "We would have inserted a null pixmap!\n" );
                pixmaps[ widget ] = cachePix;
            }
            else
            {
                cache->insert( pixmaps[ widget ], KThemeCache::HorizontalScale, widget );
                TQImage tmpImg = images[ widget ] ->
                                smoothScale( w, images[ widget ] ->height() );
                pixmaps[ widget ] = new KThemePixmap;
                pixmaps[ widget ] ->convertFromImage( tmpImg );
                if ( blends[ widget ] != 0.0 )
                    blend( widget );
            }
        }
    }
    else if ( scaleHints[ widget ] == VerticalScale )
    {
        if ( pixmaps[ widget ] ->height() != h )
        {
            KThemePixmap * cachePix = cache->verticalPixmap( w, widget );
            if ( cachePix )
            {
                cachePix = new KThemePixmap( *cachePix );
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::VerticalScale, widget );
                else
                    tqWarning( "We would have inserted a null pixmap!\n" );
                pixmaps[ widget ] = cachePix;
            }
            else
            {
                cache->insert( pixmaps[ widget ], KThemeCache::VerticalScale, widget );
                TQImage tmpImg =
                    images[ widget ] ->smoothScale( images[ widget ] ->width(), h );
                pixmaps[ widget ] = new KThemePixmap;
                pixmaps[ widget ] ->convertFromImage( tmpImg );
                if ( blends[ widget ] != 0.0 )
                    blend( widget );
            }
        }
    }
    // If blended tile here so the blend is scaled properly
    else if ( scaleHints[ widget ] == TileScale && blends[ widget ] != 0.0 )
    {
        if ( !pixmaps[ widget ] || pixmaps[ widget ] ->width() != w ||
                pixmaps[ widget ] ->height() != h )
        {
            KThemePixmap * cachePix = cache->pixmap( w, h, widget );
            if ( cachePix )
            {
                cachePix = new KThemePixmap( *cachePix );
                cache->insert( pixmaps[ widget ], KThemeCache::FullScale, widget );
                pixmaps[ widget ] = cachePix;
            }
            else
            {
                cache->insert( pixmaps[ widget ], KThemeCache::FullScale, widget );
                TQPixmap tile;
                tile.convertFromImage( *images[ widget ] );
                pixmaps[ widget ] = new KThemePixmap;
                pixmaps[ widget ] ->resize( w, h );
                TQPainter p( pixmaps[ widget ] );
                p.drawTiledPixmap( 0, 0, w, h, tile );
                if ( blends[ widget ] != 0.0 )
                    blend( widget );
            }
        }
    }
    return ( pixmaps[ widget ] );
}

KThemePixmap* KThemeBase::scaleBorder( int w, int h, WidgetType widget ) const
{
    KThemePixmap * pixmap = NULL;
    if ( !pbPixmaps[ widget ] && !pbWidth[ widget ] )
        return ( NULL );
    pixmap = cache->pixmap( w, h, widget, true );
    if ( pixmap )
    {
        pixmap = new KThemePixmap( *pixmap );
    }
    else
    {
        pixmap = new KThemePixmap();
        pixmap->resize( w, h );
        TQBitmap mask;
        mask.resize( w, h );
        mask.fill( color0 );
        TQPainter mPainter;
        mPainter.begin( &mask );

        TQPixmap *tmp = borderPixmap( widget ) ->border( KThemePixmap::TopLeft );
        const TQBitmap *srcMask = tmp->mask();
        int bdWidth = tmp->width();

        bitBlt( pixmap, 0, 0, tmp, 0, 0, bdWidth, bdWidth,
                TQt::CopyROP, false );
        if ( srcMask )
            bitBlt( &mask, 0, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0, bdWidth, bdWidth,
                    TQt::CopyROP, false );
        else
            mPainter.fillRect( 0, 0, bdWidth, bdWidth, color1 );


        tmp = borderPixmap( widget ) ->border( KThemePixmap::TopRight );
        srcMask = tmp->mask();
        bitBlt( pixmap, w - bdWidth, 0, tmp, 0, 0, bdWidth,
                bdWidth, TQt::CopyROP, false );
        if ( srcMask )
            bitBlt( &mask, w - bdWidth, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0, bdWidth,
                    bdWidth, TQt::CopyROP, false );
        else
            mPainter.fillRect( w - bdWidth, 0, bdWidth, bdWidth, color1 );

        tmp = borderPixmap( widget ) ->border( KThemePixmap::BottomLeft );
        srcMask = tmp->mask();
        bitBlt( pixmap, 0, h - bdWidth, tmp, 0, 0, bdWidth,
                bdWidth, TQt::CopyROP, false );
        if ( srcMask )
            bitBlt( &mask, 0, h - bdWidth, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0, bdWidth,
                    bdWidth, TQt::CopyROP, false );
        else
            mPainter.fillRect( 0, h - bdWidth, bdWidth, bdWidth, color1 );

        tmp = borderPixmap( widget ) ->border( KThemePixmap::BottomRight );
        srcMask = tmp->mask();
        bitBlt( pixmap, w - bdWidth, h - bdWidth, tmp, 0, 0,
                bdWidth, bdWidth, TQt::CopyROP, false );
        if ( srcMask )
            bitBlt( &mask, w - bdWidth, h - bdWidth, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0,
                    bdWidth, bdWidth, TQt::CopyROP, false );
        else
            mPainter.fillRect( w - bdWidth, h - bdWidth, bdWidth, bdWidth, color1 );

        TQPainter p;
        p.begin( pixmap );
        if ( w - bdWidth * 2 > 0 )
        {
            tmp = borderPixmap( widget ) ->border( KThemePixmap::Top );
            srcMask = tmp->mask();
            p.drawTiledPixmap( bdWidth, 0, w - bdWidth * 2, bdWidth, *tmp );
            if ( srcMask )
                bitBlt( &mask, bdWidth, 0, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0,
                        w - bdWidth * 2, bdWidth, TQt::CopyROP, false );
            else
                mPainter.fillRect( bdWidth, 0, w - bdWidth * 2, bdWidth, color1 );

            tmp = borderPixmap( widget ) ->border( KThemePixmap::Bottom );
            srcMask = tmp->mask();
            p.drawTiledPixmap( bdWidth, h - bdWidth, w - bdWidth * 2, bdWidth,
                               *tmp );
            if ( srcMask )
                bitBlt( &mask, bdWidth, h - bdWidth, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0,
                        w - bdWidth * 2, bdWidth, TQt::CopyROP, false );
            else
                mPainter.fillRect( bdWidth, h - bdWidth, w - bdWidth * 2, bdWidth,
                                   color1 );
        }
        if ( h - bdWidth * 2 > 0 )
        {
            tmp = borderPixmap( widget ) ->border( KThemePixmap::Left );
            srcMask = tmp->mask();
            p.drawTiledPixmap( 0, bdWidth, bdWidth, h - bdWidth * 2, *tmp );
            if ( srcMask )
                bitBlt( &mask, 0, bdWidth, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0,
                        bdWidth, h - bdWidth * 2, TQt::CopyROP, false );
            else
                mPainter.fillRect( 0, bdWidth, bdWidth, h - bdWidth * 2, color1 );

            tmp = borderPixmap( widget ) ->border( KThemePixmap::Right );
            srcMask = tmp->mask();
            p.drawTiledPixmap( w - bdWidth, bdWidth, bdWidth, h - bdWidth * 2,
                               *tmp );
            if ( srcMask )
                bitBlt( &mask, w - bdWidth, bdWidth, KDE_TQBITMAP_TO_TQPAINTDEVICE(srcMask), 0, 0,
                        bdWidth, h - bdWidth * 2, TQt::CopyROP, false );
            else
                mPainter.fillRect( w - bdWidth, bdWidth, bdWidth, h - bdWidth * 2, color1 );
        }
        p.end();
        mPainter.end();
        pixmap->setMask( mask );
        cache->insert( pixmap, KThemeCache::FullScale, widget, true );
        if ( !pixmap->mask() )
            tqWarning( "No mask for border pixmap!\n" );
    }
    return ( pixmap );
}


KThemePixmap* KThemeBase::blend( WidgetType widget ) const
{
    KPixmapEffect::GradientType g;
    switch ( gradients[ widget ] )
    {
        case GrHorizontal:
            g = KPixmapEffect::HorizontalGradient;
            break;
        case GrVertical:
            g = KPixmapEffect::VerticalGradient;
            break;
        case GrPyramid:
            g = KPixmapEffect::PyramidGradient;
            break;
        case GrRectangle:
            g = KPixmapEffect::RectangleGradient;
            break;
        case GrElliptic:
            g = KPixmapEffect::EllipticGradient;
            break;
        default:
            g = KPixmapEffect::DiagonalGradient;
            break;
    }
    KPixmapEffect::blend( *pixmaps[ widget ], blends[ widget ], *grLowColors[ widget ],
                          g, false );
    return ( pixmaps[ widget ] );
}

KThemePixmap* KThemeBase::gradient( int w, int h, WidgetType widget ) const
{
    if ( gradients[ widget ] == GrVertical )
    {
        if ( !pixmaps[ widget ] || pixmaps[ widget ] ->height() != h )
        {
            KThemePixmap * cachePix = cache->verticalPixmap( h, widget );
            if ( cachePix )
            {
                cachePix = new KThemePixmap( *cachePix );
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::VerticalScale,
                                   widget );
                pixmaps[ widget ] = cachePix;
            }
            else
            {
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::VerticalScale,
                                   widget );
                pixmaps[ widget ] = new KThemePixmap;
                pixmaps[ widget ] ->resize( w, h );
                KPixmapEffect::gradient( *pixmaps[ widget ], *grHighColors[ widget ],
                                         *grLowColors[ widget ],
                                         KPixmapEffect::VerticalGradient );
            }
        }
    }
    else if ( gradients[ widget ] == GrHorizontal )
    {
        if ( !pixmaps[ widget ] || pixmaps[ widget ] ->width() != w )
        {
            KThemePixmap * cachePix = cache->horizontalPixmap( w, widget );
            if ( cachePix )
            {
                cachePix = new KThemePixmap( *cachePix );
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ],
                                   KThemeCache::HorizontalScale, widget );
                pixmaps[ widget ] = cachePix;
            }
            else
            {
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ],
                                   KThemeCache::HorizontalScale, widget );
                pixmaps[ widget ] = new KThemePixmap;
                pixmaps[ widget ] ->resize( w, h );
                KPixmapEffect::gradient( *pixmaps[ widget ], *grHighColors[ widget ],
                                         *grLowColors[ widget ],
                                         KPixmapEffect::HorizontalGradient );
            }
        }
    }
    else if ( gradients[ widget ] == GrReverseBevel )
    {
        if ( !pixmaps[ widget ] || pixmaps[ widget ] ->width() != w ||
                pixmaps[ widget ] ->height() != h )
        {
            KThemePixmap * cachePix = cache->pixmap( w, h, widget );
            if ( cachePix )
            {
                cachePix = new KThemePixmap( *cachePix );
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::FullScale,
                                   widget );
                pixmaps[ widget ] = cachePix;
            }
            else
            {
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::FullScale,
                                   widget );
                pixmaps[ widget ] = new KThemePixmap;
                pixmaps[ widget ] ->resize( w, h );

                KPixmap s;
                int offset = decoWidth( widget );
                s.resize( w - offset * 2, h - offset * 2 );
                TQColor lc( *grLowColors[ widget ] );
                TQColor hc( *grHighColors[ widget ] );
                if ( bevelContrast( widget ) )
                {
                    int bc = bevelContrast( widget );
                    // want single increments, not factors like light()/dark()
                    lc.setRgb( lc.red() - bc, lc.green() - bc, lc.blue() - bc );
                    hc.setRgb( hc.red() + bc, hc.green() + bc, hc.blue() + bc );
                }
                KPixmapEffect::gradient( *pixmaps[ widget ],
                                         lc, hc,
                                         KPixmapEffect::DiagonalGradient );
                KPixmapEffect::gradient( s, *grHighColors[ widget ],
                                         *grLowColors[ widget ],
                                         KPixmapEffect::DiagonalGradient );
                bitBlt( pixmaps[ widget ], offset, offset, &s, 0, 0, w - offset * 2,
                        h - offset * 2, TQt::CopyROP );
            }
        }
    }
    else
    {
        KPixmapEffect::GradientType g;
        switch ( gradients[ widget ] )
        {
            case GrPyramid:
                g = KPixmapEffect::PyramidGradient;
                break;
            case GrRectangle:
                g = KPixmapEffect::RectangleGradient;
                break;
            case GrElliptic:
                g = KPixmapEffect::EllipticGradient;
                break;
            default:
                g = KPixmapEffect::DiagonalGradient;
                break;
        }
        if ( !pixmaps[ widget ] || pixmaps[ widget ] ->width() != w ||
                pixmaps[ widget ] ->height() != h )
        {
            KThemePixmap * cachePix = cache->pixmap( w, h, widget );
            if ( cachePix )
            {
                cachePix = new KThemePixmap( *cachePix );
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::FullScale,
                                   widget );
                pixmaps[ widget ] = cachePix;
            }
            else
            {
                if ( pixmaps[ widget ] )
                    cache->insert( pixmaps[ widget ], KThemeCache::FullScale,
                                   widget );
                pixmaps[ widget ] = new KThemePixmap;
                pixmaps[ widget ] ->resize( w, h );
                KPixmapEffect::gradient( *pixmaps[ widget ], *grHighColors[ widget ],
                                         *grLowColors[ widget ], g );
            }
        }
    }
    return ( pixmaps[ widget ] );
}

KThemePixmap* KThemeBase::scalePixmap( int w, int h, WidgetType widget ) const
{

    if ( gradients[ widget ] && blends[ widget ] == 0.0 )
        return ( gradient( w, h, widget ) );

    return ( scale( w, h, widget ) );
}

TQColorGroup* KThemeBase::makeColorGroup( const TQColor &fg, const TQColor &bg,
        TQt::GUIStyle )
{
    if ( shading == Motif )
    {
        int highlightVal, lowlightVal;
        highlightVal = 100 + ( 2 * d->contrast + 4 ) * 16 / 10;
        lowlightVal = 100 + ( ( 2 * d->contrast + 4 ) * 10 );
        return ( new TQColorGroup( fg, bg, bg.light( highlightVal ),
                                  bg.dark( lowlightVal ), bg.dark( 120 ),
                                  fg, TQApplication::palette().active().base() ) );
    }
    else
        return ( new TQColorGroup( fg, bg, bg.light( 150 ), bg.dark(),
                                  bg.dark( 120 ), fg,
                                  TQApplication::palette().active().base() ) );
}


void KThemeBase::applyMiscResourceGroup( TQSettings *config )
{
#ifndef TQ_WS_QWS //FIXME
    d->props.erase( "Misc" ); // clear the old property

    TQString base = configFileName + "Misc/";

    Prop& prop = d->props[ "Misc" ];
    TQString tmpStr;

    tmpStr = config->readEntry( base + "SButtonPosition" );
    if ( tmpStr == "BottomLeft" )
        prop[ "SButtonPosition" ] = TQString::number( ( int ) SBBottomLeft );
    else if ( tmpStr == "BottomRight" )
        prop[ "SButtonPosition" ] = TQString::number( ( int ) SBBottomRight );
    else
    {
        if ( tmpStr != "Opposite" && !tmpStr.isEmpty() )
            tqWarning( "KThemeBase: Unrecognized sb button option %s, using Opposite.\n", tmpStr.latin1() );
        ;
        prop[ "SButtonPosition" ] = TQString::number( ( int ) SBOpposite );
    }
    tmpStr = config->readEntry( base + "ArrowType" );
    if ( tmpStr == "Small" )
        prop[ "ArrowType" ] = TQString::number( ( int ) SmallArrow );
    else if ( tmpStr == "3D" )
        prop[ "ArrowType" ] = TQString::number( ( int ) MotifArrow );
    else
    {
        if ( tmpStr != "Normal" && !tmpStr.isEmpty() )
            tqWarning( "KThemeBase: Unrecognized arrow option %s, using Normal.\n", tmpStr.latin1() );
        prop[ "ArrowType" ] = TQString::number( ( int ) LargeArrow );
    }
    tmpStr = config->readEntry( base + "ShadeStyle" );
    if ( tmpStr == "Motif" )
        prop[ "ShadeStyle" ] = TQString::number( ( int ) Motif );
    else if ( tmpStr == "Next" )
        prop[ "ShadeStyle" ] = TQString::number( ( int ) Next );
    else if ( tmpStr == "KDE" )
        prop[ "ShadeStyle" ] = TQString::number( ( int ) KDE );
    else
        prop[ "ShadeStyle" ] = TQString::number( ( int ) Windows );

    prop[ "FrameWidth" ] = TQString::number( config->readNumEntry( base + "FrameWidth", 2 ) );
    prop[ "Cache" ] = TQString::number( config->readNumEntry( base + "Cache", 1024 ) );
    prop[ "ScrollBarExtent" ] = TQString::number( config->readNumEntry( base + "ScrollBarExtent", 16 ) );
#endif
}

static int readNumEntry( Prop& prop, TQString setting, int def )
{
    bool ok;
    TQString s_val = prop[ setting ];
    int val = s_val.toInt( &ok );
    if ( ok )
        return val;
    return def;
}

static TQColor readColorEntry( Prop& prop, TQString setting, const TQColor& def )
{
    TQString s_val = prop[ setting ];
    if ( !s_val.isEmpty() )
    {
        TQColor c( s_val );
        return c;
    }
    return def;
}

void KThemeBase::readMiscResourceGroup()
{
#ifndef TQ_WS_QWS //FIXME
    Prop & prop = d->props[ "Misc" ];

    sbPlacement = ( SButton ) readNumEntry( prop, "SButtonPosition",
                                            ( int ) SBOpposite );
    arrowStyle = ( ArrowStyle ) readNumEntry( prop, "ArrowType",
                 ( int ) LargeArrow );
    shading = ( ShadeStyle ) readNumEntry( prop, "ShadeStyle", ( int ) Windows );
    defaultFrame = readNumEntry( prop, "FrameWidth", 2 );
    cacheSize = readNumEntry( prop, "Cache", 1024 );
    sbExtent = readNumEntry( prop, "ScrollBarExtent", 16 );
#endif
}


void KThemeBase::applyResourceGroup( TQSettings *config, int i )
{
#ifndef TQ_WS_QWS //FIXME
    TQString tmpStr;
    int tmpVal;

    // clear the old property
    d->props.erase( widgetEntries[ i ] );

    TQString base = configFileName + widgetEntries[ i ] + "/";

    Prop& prop = d->props[ widgetEntries[ i ] ];

    tmpStr = config->readEntry( base + "CopyWidget", "" );
    prop[ "CopyWidget" ] = tmpStr;
    if ( !tmpStr.isEmpty() )
    {
        return ;
    }

    tmpStr = config->readEntry( base + "Scale" );
    if ( tmpStr == "Full" )
        tmpVal = ( int ) FullScale;
    else if ( tmpStr == "Horizontal" )
        tmpVal = ( int ) HorizontalScale;
    else if ( tmpStr == "Vertical" )
        tmpVal = ( int ) VerticalScale;
    else
    {
        if ( tmpStr != "Tile" && !tmpStr.isEmpty() )
            tqWarning( "KThemeBase: Unrecognized scale option %s, using Tile.\n", tmpStr.latin1() );
        tmpVal = ( int ) TileScale;
    }
    prop[ "ScaleHint" ] = TQString::number( tmpVal );

    // Gradient type
    tmpStr = config->readEntry( base + "Gradient" );
    if ( tmpStr == "Diagonal" )
        tmpVal = ( int ) GrDiagonal;
    else if ( tmpStr == "Horizontal" )
        tmpVal = ( int ) GrHorizontal;
    else if ( tmpStr == "Vertical" )
        tmpVal = ( int ) GrVertical;
    else if ( tmpStr == "Pyramid" )
        tmpVal = ( int ) GrPyramid;
    else if ( tmpStr == "Rectangle" )
        tmpVal = ( int ) GrRectangle;
    else if ( tmpStr == "Elliptic" )
        tmpVal = ( int ) GrElliptic;
    else if ( tmpStr == "ReverseBevel" )
        tmpVal = ( int ) GrReverseBevel;
    else
    {
        if ( tmpStr != "None" && !tmpStr.isEmpty() )
            tqWarning( "KThemeBase: Unrecognized gradient option %s, using None.\n", tmpStr.latin1() );
        tmpVal = ( int ) GrNone;
    }
    prop[ "Gradient" ] = TQString::number( tmpVal );

    // Blend intensity
    tmpStr.setNum( config->readDoubleEntry( base + "BlendIntensity", 0.0 ) );
    prop[ "Blend" ] = tmpStr;

    // Bevel contrast
    prop[ "BContrast" ] = TQString::number( config->readNumEntry( base + "BevelContrast", 0 ) );

    // Border width
    prop[ "Border" ] = TQString::number( config->readNumEntry( base + "Border", 1 ) );

    // Highlight width
    prop[ "Highlight" ] = TQString::number( config->readNumEntry( base + "Highlight", 1 ) );

    TQStringList keys = config->entryList( base );

    // Gradient low color or blend background
    if ( keys.contains( "GradientLow" ) )
        prop[ "GrLow" ] = readColorEntry( config, TQString( base + "GradientLow" ).latin1(),
                                          &TQApplication::palette().active().background() ).name();

    // Gradient high color
    if ( keys.contains( "GradientHigh" ) )
        prop[ "GrHigh" ] = readColorEntry( config, TQString( base + "GradientHigh" ).latin1(),
                                           &TQApplication::palette().active().foreground() ).name();

    // Extended color attributes
    if ( keys.contains( "Foreground" ) || keys.contains( "Background" ) )
    {
        TQColor fg, bg;
        if ( keys.contains( "Background" ) )
            bg = readColorEntry( config, TQString( base + "Background" ).latin1(), &bg );
        if ( keys.contains( "Foreground" ) )
            fg = readColorEntry( config, TQString( base + "Foreground" ).latin1(), &fg );
        prop[ "Foreground" ] = fg.name();
        prop[ "Background" ] = bg.name();

    }
    else
        colors[ i ] = NULL;

    // Pixmap
    tmpStr = config->readEntry( base + "Pixmap", "" );
    if ( !tmpStr.isEmpty() )
        prop[ "Pixmap" ] = tmpStr;
    // Pixmap border
    tmpStr = config->readEntry( base + "PixmapBorder", "" );
    if ( !tmpStr.isEmpty() )
    {
        prop[ "PixmapBorder" ] = tmpStr;
        prop[ "PixmapBWidth" ] = TQString::number(
                                     config->readNumEntry( base + "PixmapBWidth", 0 ) );
    }

    // Various widget specific settings. This was more efficient when bunched
    // together in the misc group, but this makes an easier to read config.
    if ( i == SliderGroove )
        prop[ "SmallGroove" ] = TQString::number(
                                    config->readBoolEntry( base + "SmallGroove", false ) );
    else if ( i == ActiveTab || i == InactiveTab )
        prop[ "BottomLine" ] = TQString::number(
                                   config->readBoolEntry( base + "BottomLine", true ) );
    else if ( i == Splitter )
        prop[ "Width" ] = TQString::number( config->readNumEntry( base + "Width", 10 ) );
    else if ( i == ComboBox || i == ComboBoxDown )
    {
        if ( keys.contains( "Round" ) )
            prop[ "Round" ] = TQString::number( config->readBoolEntry( base + "Round", false ) );
        else
            prop[ "Round" ] = "5000"; // invalid, used w/multiple groups

    }
    else if ( i == PushButton || i == PushButtonDown )
    {
        if ( keys.contains( "XShift" ) )
            prop[ "XShift" ] = TQString::number( config->readNumEntry( base + "XShift", 0 ) );
        else
            prop[ "XShift" ] = "5000";
        if ( keys.contains( "YShift" ) )
            prop[ "YShift" ] = TQString::number( config->readNumEntry( base + "YShift", 0 ) );
        else
            prop[ "YShift" ] = "5000";
        if ( keys.contains( "3DFocusRect" ) )
            prop[ "3DFRect" ] = TQString::number( config->
                                                 readBoolEntry(  base + "3DFocusRect", false ) );
        else
            prop[ "3DFRect" ] = "5000";
        if ( keys.contains( "3DFocusOffset" ) )
            prop[ "3DFOffset" ] = TQString::number( config->
                                                   readBoolEntry(  base + "3DFocusOffset", 0 ) );
        else
            prop[ "3DFOffset" ] = "5000";
        if ( keys.contains( "Round" ) )
            prop[ "Round" ] = TQString::number( config->readBoolEntry( base + "Round", false ) );
        else
            prop[ "Round" ] = "5000";
    }
#endif
}

void KThemeBase::readResourceGroup( int i, TQString *pixnames, TQString *brdnames,
                                    bool *loadArray )
{
#ifndef TQ_WS_QWS //FIXME
    if ( loadArray[ i ] == true )
    {
        return ; // already been preloaded.
    }

    int tmpVal;
    Prop prop = d->props[ widgetEntries[ i ] ];
    TQString tmpStr;

    tmpStr = prop[ "CopyWidget" ];
    if ( !tmpStr.isEmpty() )
    { // Duplicate another widget's config
        int sIndex;
        loadArray[ i ] = true;
        for ( sIndex = 0; sIndex < WIDGETS; ++sIndex )
        {
            if ( tmpStr == widgetEntries[ sIndex ] )
            {
                if ( !loadArray[ sIndex ] )   // hasn't been loaded yet
                    readResourceGroup( sIndex, pixnames, brdnames,
                                       loadArray );
                break;
            }
        }
        if ( loadArray[ sIndex ] )
        {
            copyWidgetConfig( sIndex, i, pixnames, brdnames );
        }
        else
            tqWarning( "KThemeBase: Unable to identify source widget for %s\n", widgetEntries[ i ] );
        return ;
    }
    // special inheritance for disabled arrows (these are tri-state unlike
    // the rest of what we handle).
    for ( tmpVal = DisArrowUp; tmpVal <= DisArrowRight; ++tmpVal )
    {
        if ( tmpVal == i )
        {
            tmpStr = prop[ "Pixmap" ];
            if ( tmpStr.isEmpty() )
            {
                copyWidgetConfig( ArrowUp + ( tmpVal - DisArrowUp ), i, pixnames,
                                  brdnames );
                return ;
            }
        }
    }

    // Scale hint
    scaleHints[ i ] = ( ScaleHint ) readNumEntry( prop, "ScaleHint", ( int ) TileScale );
    gradients[ i ] = ( Gradient ) readNumEntry( prop, "Gradient", ( int ) GrNone );

    // Blend intensity
    tmpStr = prop[ "Blend" ];
    if ( tmpStr.isEmpty() )
        tmpStr = TQString::fromLatin1( "0.0" );
    blends[ i ] = tmpStr.toFloat();

    // Bevel contrast
    bContrasts[ i ] = readNumEntry( prop, "BContrast", 0 );

    // Border width
    borders[ i ] = readNumEntry( prop, "Border", 1 );

    // Highlight width
    highlights[ i ] = readNumEntry( prop, "Highlight", 1 );

    // Gradient low color or blend background
    if ( gradients[ i ] != GrNone || blends[ i ] != 0.0 )
        grLowColors[ i ] =
            new TQColor( readColorEntry( prop, "GrLow",
                                        TQApplication::palette().active().
                                        background() ) );
    else
        grLowColors[ i ] = NULL;

    // Gradient high color
    if ( gradients[ i ] != GrNone )
        grHighColors[ i ] =
            new TQColor( readColorEntry( prop, "GrHigh",
                                        TQApplication::palette().active().
                                        background() ) );
    else
        grHighColors[ i ] = NULL;

    // Extended color attributes
    TQColor fg, bg;
    fg = readColorEntry( prop, "Foreground", fg );
    bg = readColorEntry( prop, "Background", bg );
    if ( fg.isValid() || bg.isValid() )
    {
        if ( !fg.isValid() )
            fg = TQApplication::palette().active().foreground();
        if ( !bg.isValid() )
            bg = TQApplication::palette().active().background();
        colors[ i ] = makeColorGroup( fg, bg, TQt::WindowsStyle );
    }
    else
        colors[ i ] = NULL;

    // Pixmap
    int existing;
    tmpStr = prop[ "Pixmap" ];
    pixnames[ i ] = tmpStr;
    duplicate[ i ] = false;
    pixmaps[ i ] = NULL;
    images[ i ] = NULL;
    // Scan for duplicate pixmaps(two identical pixmaps, tile scale, no blend,
    // no pixmapped border)
    if ( !tmpStr.isEmpty() )
    {
        for ( existing = 0; existing < i; ++existing )
        {
            if ( tmpStr == pixnames[ existing ] && scaleHints[ i ] == TileScale &&
                    scaleHints[ existing ] == TileScale && blends[ existing ] == 0.0 &&
                    blends[ i ] == 0.0 )
            {
                pixmaps[ i ] = pixmaps[ existing ];
                duplicate[ i ] = true;
                break;
            }
        }
    }
    // load
    if ( !duplicate[ i ] && !tmpStr.isEmpty() )
    {
        pixmaps[ i ] = loadPixmap( tmpStr );
        // load and save images for scaled/blended widgets for speed.
        if ( scaleHints[ i ] == TileScale && blends[ i ] == 0.0 )
            images[ i ] = NULL;
        else
            images[ i ] = loadImage( tmpStr );
    }

    // Pixmap border
    tmpStr = prop[ "PixmapBorder" ];
    brdnames[ i ] = tmpStr;
    pbDuplicate[ i ] = false;
    pbPixmaps[ i ] = NULL;
    pbWidth[ i ] = 0;
    if ( !tmpStr.isEmpty() )
    {
        pbWidth[ i ] = readNumEntry( prop, "PixmapBWidth", 0 );
        if ( pbWidth[ i ] == 0 )
        {
            tqWarning( "KThemeBase: No border width specified for pixmapped border widget %s\n",
                      widgetEntries[ i ] );
            tqWarning( "KThemeBase: Using default of 2.\n" );
            pbWidth[ i ] = 2;
        }
        // duplicate check
        for ( existing = 0; existing < i; ++existing )
        {
            if ( tmpStr == brdnames[ existing ] )
            {
                pbPixmaps[ i ] = pbPixmaps[ existing ];
                pbDuplicate[ i ] = true;
                break;
            }
        }
    }
    // load
    if ( !pbDuplicate[ i ] && !tmpStr.isEmpty() )
        pbPixmaps[ i ] = loadPixmap( tmpStr );

    if ( pbPixmaps[ i ] && !pbDuplicate[ i ] )
        generateBorderPix( i );

    // Various widget specific settings. This was more efficient when bunched
    // together in the misc group, but this makes an easier to read config.
    if ( i == SliderGroove )
        roundedSlider = readNumEntry( prop, "SmallGroove", false );
    else if ( i == ActiveTab )
        aTabLine = readNumEntry( prop, "BottomLine", true );
    else if ( i == InactiveTab )
        iTabLine = readNumEntry( prop, "BottomLine", true );
    else if ( i == Splitter )
        splitterWidth = readNumEntry( prop, "Width", 10 );
    else if ( i == ComboBox || i == ComboBoxDown )
    {
        tmpVal = readNumEntry( prop, "Round", 5000 );
        if ( tmpVal != 5000 )
            roundedCombo = tmpVal;
    }
    else if ( i == PushButton || i == PushButtonDown )
    {
        tmpVal = readNumEntry( prop, "XShift", 0 );
        if ( tmpVal != 5000 )
            btnXShift = tmpVal;
        tmpVal = readNumEntry( prop, "YShift", 0 );
        if ( tmpVal != 5000 )
            btnYShift = tmpVal;
        tmpVal = readNumEntry( prop, "3DFRect", false );
        if ( tmpVal != 5000 )
            focus3D = tmpVal;
        tmpVal = readNumEntry( prop, "3DFOffset", 0 );
        if ( tmpVal != 5000 )
            focus3DOffset = tmpVal;
        tmpVal = readNumEntry( prop, "Round", false );
        if ( tmpVal != 5000 )
            roundedButton = tmpVal;
    }
    loadArray[ i ] = true;
#endif
}


TQPalette KThemeBase::overridePalette( const TQPalette& pal )
{

    //Read current settings for colors..
    TQColor background = pal.active().background();
    TQColor foreground = pal.active().foreground();
    TQColor button = background; //CHECKME
    TQColor highlight = pal.active().highlight();
    TQColor highlightedText = pal.active().highlightedText(); //CHECKME
    TQColor base = pal.active().base(); //config->readColorEntry( "windowBackground", &white );
    TQColor baseText = pal.active().text(); //CHECKME

    //See whether there are any immediate overrides.
    if ( d->overrideBackground )
        background = d->overrideBackgroundCol;

    if ( d->overrideForeground )
        foreground = d->overrideForegroundCol;

    if ( d->overrideSelectBackground )
        highlight = d->overrideSelectBackgroundCol;
    if ( d->overrideSelectForeground )
        highlightedText = d->overrideSelectForegroundCol;

    if ( d->overrideWindowBackground )
        base = d->overrideWindowBackgroundCol;
    if ( d->overrideWindowForeground )
        baseText = d->overrideWindowForegroundCol;

    //Now, try to get the button color from the pixmap
    if ( uncached( Bevel ) )
        button = d->pixmapAveColor( uncached( Bevel ) );

    TQColor buttonText = foreground;

    int highlightVal, lowlightVal;
    highlightVal = 100 + ( 2 * d->contrast + 4 ) * 16 / 10;
    lowlightVal = 100 + ( 2 * d->contrast + 4 ) * 10;


    if ( isPixmap( Background ) || isColor( Background ) )
    {
        if ( isColor( Background ) )
        {
            background = colorGroup( pal.active(), Background )
                         ->background();
        }
        if ( isPixmap( Background ) )
        {
            background = d->pixmapAveColor( uncached( Background ) );
        }


        TQColorGroup pre( foreground, button, background.light( highlightVal ),
                            background.dark( lowlightVal ), background.dark( 120 ),
                            baseText, buttonText /*CHECKME: BrightText*/, base, background );

        buttonText = colorGroup( pre, PushButton ) ->foreground();
    }

    TQColor disfg = foreground;
    int h, s, v;
    disfg.hsv( &h, &s, &v );
    if ( v > 128 )
        // dark bg, light fg - need a darker disabled fg
        disfg = disfg.dark( lowlightVal );
    else if ( disfg != black )
        // light bg, dark fg - need a lighter disabled fg - but only if !black
        disfg = disfg.light( highlightVal );
    else
        // black fg - use darkgray disabled fg
        disfg = TQt::darkGray;


    TQColorGroup disabledgrp( disfg, background,   //TODO:Convert this to the new ctor.
                             background.light( highlightVal ),
                             background.dark( lowlightVal ),
                             background.dark( 120 ),
                             background.dark( 120 ), base );


    TQColorGroup colgrp( foreground, button, background.light( highlightVal ),
                        background.dark( lowlightVal ), background.dark( 120 ),
                        baseText, buttonText /*CHECKME: BrightText*/, base, background );



    colgrp.setColor( TQColorGroup::Highlight, highlight );
    colgrp.setColor( TQColorGroup::HighlightedText, highlightedText );
    colgrp.setColor( TQColorGroup::ButtonText, buttonText );
    colgrp.setColor( TQColorGroup::Midlight, button.light( 110 ) );


    disabledgrp.setColor( TQColorGroup::Base, base );
    disabledgrp.setColor( TQColorGroup::Button, button );
    disabledgrp.setColor( TQColorGroup::ButtonText, buttonText );
    disabledgrp.setColor( TQColorGroup::Midlight, button.light( 110 ) );

    TQPalette newPal( colgrp, disabledgrp, colgrp );

    return newPal;

}

KThemePixmap::KThemePixmap( bool timer )
        : KPixmap()
{
    if ( timer )
    {
        t = new TQTime;
        t->start();
    }
    else
        t = NULL;
    int i;
    for ( i = 0; i < 8; ++i )
        b[ i ] = NULL;
}

KThemePixmap::KThemePixmap( const KThemePixmap &p )
        : KPixmap( p )
{
    if ( p.t )
    {
        t = new TQTime;
        t->start();
    }
    else
        t = NULL;
    int i;
    for ( i = 0; i < 8; ++i )
        if ( p.b[ i ] )
            b[ i ] = new TQPixmap( *p.b[ i ] );
        else
            b[ i ] = NULL;
}

KThemePixmap::KThemePixmap( const KThemePixmap &p, const TQPixmap &p2 )
        : KPixmap( p2 )
{
    if ( p.t )
    {
        t = new TQTime;
        t->start();
    }
    else
        t = NULL;
    int i;
    for ( i = 0; i < 8; ++i )
        if ( p.b[ i ] )
            b[ i ] = new TQPixmap( *p.b[ i ] );
        else
            b[ i ] = NULL;
}



KThemePixmap::~KThemePixmap()
{
    if ( t )
        delete t;
    int i;
    for ( i = 0; i < 8; ++i )
        if ( b[ i ] )
            delete b[ i ];
}

KThemeCache::KThemeCache( int maxSize, TQObject *parent, const char *name )
        : TQObject( parent, name )
{
    cache.setMaxCost( maxSize * 1024 );
    cache.setAutoDelete( true );
    flushTimer.start( 300000 ); // 5 minutes
    connect( &flushTimer, TQ_SIGNAL( timeout() ), TQ_SLOT( flushTimeout() ) );
}

void KThemeCache::flushTimeout()
{
    TQIntCacheIterator<KThemePixmap> it( cache );
    while ( it.current() )
    {
        if ( it.current() ->isOld() )
            cache.remove( it.currentKey() );
        else
            ++it;
    }
}

KThemePixmap* KThemeCache::pixmap( int w, int h, int widgetID, bool border,
                                   bool mask )
{

    kthemeKey key;
    key.cacheKey = 0; // shut up, gcc
    key.data.id = widgetID;
    key.data.width = w;
    key.data.height = h;
    key.data.border = border;
    key.data.mask = mask;

    KThemePixmap *pix = cache.find( ( unsigned long ) key.cacheKey );
    if ( pix )
        pix->updateAccessed();
    return ( pix );
}

KThemePixmap* KThemeCache::horizontalPixmap( int w, int widgetID )
{
    kthemeKey key;
    key.cacheKey = 0; // shut up, gcc
    key.data.id = widgetID;
    key.data.width = w;
    key.data.height = 0;
    key.data.border = false;
    key.data.mask = false;
    KThemePixmap *pix = cache.find( ( unsigned long ) key.cacheKey );
    if ( pix )
        pix->updateAccessed();
    return ( pix );
}

KThemePixmap* KThemeCache::verticalPixmap( int h, int widgetID )
{
    kthemeKey key;
    key.cacheKey = 0; // shut up, gcc
    key.data.id = widgetID;
    key.data.width = 0;
    key.data.height = h;
    key.data.border = false;
    key.data.mask = false;
    KThemePixmap *pix = cache.find( ( unsigned long ) key.cacheKey );
    if ( pix )
        pix->updateAccessed();
    return ( pix );
}

bool KThemeCache::insert( KThemePixmap *pixmap, ScaleHint scale, int widgetID,
                          bool border, bool mask )
{
    kthemeKey key;
    key.cacheKey = 0; // shut up, gcc
    key.data.id = widgetID;
    key.data.width = ( scale == FullScale || scale == HorizontalScale ) ?
                     pixmap->width() : 0;
    key.data.height = ( scale == FullScale || scale == VerticalScale ) ?
                      pixmap->height() : 0;
    key.data.border = border;
    key.data.mask = mask;

    if ( cache.find( ( unsigned long ) key.cacheKey, true ) != NULL )
    {
        return ( true ); // a pixmap of this scale is already in there
    }
    return ( cache.insert( ( unsigned long ) key.cacheKey, pixmap,
                           pixmap->width() * pixmap->height() * pixmap->depth() / 8 ) );
}




#include "kthemebase.moc"
