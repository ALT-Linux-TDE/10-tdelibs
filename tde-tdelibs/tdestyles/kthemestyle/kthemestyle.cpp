/*
 $Id$

 This file is part of the KDE libraries
 Copyright (C) 1999 Daniel M. Duley <mosfet@kde.org>

 KDE3 port (C) 2001-2002 Maksim Orlovich <mo002j@mail.rochester.edu>
Port version 0.9.7

 Includes code portions from the dotNET style, and the KDE HighColor style.

 dotNET Style
  Copyright (C) 2001, Chris Lee        <lee@azsites.com>
                    Carsten Pfeiffer <pfeiffer@kde.org>

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

#include "kthemestyle.h"
#include "kthemebase.h"
#include <tqstyleplugin.h>
#include <tqstylefactory.h>
#include <kimageeffect.h>

#include <tqbitmap.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#define INCLUDE_MENUITEM_DEF
#include <tqmenudata.h>
#include <tqpopupmenu.h>
#include <tqpalette.h>
#include <tqtabbar.h>
#include <tqtoolbutton.h>
#include <tdeglobalsettings.h>
#include <kdrawutil.h>
#include <tqdrawutil.h>
#include <tqprogressbar.h>
#include <tqdir.h>
#include <tqapplication.h>
#include <tqmenubar.h>
#include <tqrangecontrol.h>
#include <tqslider.h>
#include <tqtooltip.h>
#include <tqobjectlist.h>
#include <tqradiobutton.h>
#include <tqstatusbar.h>
#include "tdestyledirs.h"

#include <tqimage.h>

#include <limits.h>

#ifdef __GLIBC__
#include <dlfcn.h>
#endif

static const TQCOORD u_arrow[] = { -1, -3, 0, -3, -2, -2, 1, -2, -3, -1, 2, -1, -4, 0, 3, 0, -4, 1, 3, 1};
static const TQCOORD d_arrow[] = { -4, -2, 3, -2, -4, -1, 3, -1, -3, 0, 2, 0, -2, 1, 1, 1, -1, 2, 0, 2};
static const TQCOORD l_arrow[] = { -3, -1, -3, 0, -2, -2, -2, 1, -1, -3, -1, 2, 0, -4, 0, 3, 1, -4, 1, 3};
static const TQCOORD r_arrow[] = { -2, -4, -2, 3, -1, -4, -1, 3, 0, -3, 0, 2, 1, -2, 1, 1, 2, -1, 2, 0};

const TQCOORD win_style_u_arrow[] = { 0, -2, 0, -2, -1, -1, 1, -1, -2, 0, 2, 0, -3, 1, 3, 1 };
const TQCOORD win_style_d_arrow[] = { -3, -2, 3, -2, -2, -1, 2, -1, -1, 0, 1, 0, 0, 1, 0, 1 };
const TQCOORD win_style_l_arrow[] = { 1, -3, 1, -3, 0, -2, 1, -2, -1, -1, 1, -1, -2, 0, 1, 0, -1, 1, 1, 1, 0, 2, 1, 2, 1, 3, 1, 3 };
const TQCOORD win_style_r_arrow[] = { -2, -3, -2, -3, -2, -2, -1, -2, -2, -1, 0, -1, -2, 0, 1, 0, -2, 1, 0, 1, -2, 2, -1, 2, -2, 3, -2, 3 };


#define TQCOORDARRLEN(x) sizeof(x)/(sizeof(TQCOORD)*2)


static const int itemFrame = 2;
static const int itemHMargin = 3;
static const int itemVMargin = 1;
static const int arrowHMargin = 6;
static const int rightBorder = 12;


/*
BUGS:
Sliders flash a bit -- anything else?

TODO:
Nicer disabled buttons.
Sliders are not disabled properly
*/


class KThemeStylePlugin : public TQStylePlugin
{
public:

    KThemeStylePlugin()
    {
#ifdef __GLIBC__
        dlopen("kthemestyle.so",RTLD_LAZY);
        //####### Keep reference count up so tdecore w. fast-malloc doesn't get unloaded
        //####### (Fixes exit crashes with qt-only apps that occur on Linux)
        //####### This should be rethought after 3.0,
        //####### as it relies on the implementation-specific behavior
        //####### of the glibc libdl (finding already loaded libraries based on the
        //####### soname)
#endif
    }

    ~KThemeStylePlugin()
    {}

    TQStringList keys() const
    {
        TQSettings cfg;
        TDEStyleDirs::dirs()->addToSearch( "config", cfg );

        TQStringList keys;
        bool ok;

        keys = cfg.readListEntry( "/kthemestyle/themes", &ok);
        if ( !ok )
            tqWarning( "KThemeStyle cache seems corrupt!\n" ); //Too bad one can't i18n this :-(

        return keys;
    }

    TQStyle* create( const TQString& key )
    {
        TQSettings cfg;
        TDEStyleDirs::dirs()->addToSearch( "config", cfg );

        TQString file = cfg.readEntry( "/kthemestyle/" + key + "/file" );
        if ( !key.isEmpty() )
        {
            TQFileInfo fi( file );
            return new KThemeStyle( fi.dirPath(), fi.fileName() );
        }

        return 0;
    }
};

TDE_EXPORT_PLUGIN( KThemeStylePlugin )


void kDrawWindowsArrow ( TQPainter *p, const TQStyleControlElementData &ceData, const TQStyle::ControlElementFlags elementFlags, const TQStyle* style, TQStyle::PrimitiveElement pe, bool down,
                         int x, int y, int w, int h,
                         const TQColorGroup &cg, bool enabled )
{
    TQPointArray a;
    switch ( pe )
    {
        case TQStyle::PE_ArrowUp:
            a.setPoints( TQCOORDARRLEN( win_style_u_arrow ), win_style_u_arrow );
            break;

        case TQStyle::PE_ArrowDown:
            a.setPoints( TQCOORDARRLEN( win_style_d_arrow ), win_style_d_arrow );
            break;

        case TQStyle::PE_ArrowLeft:
            a.setPoints( TQCOORDARRLEN( win_style_l_arrow ), win_style_l_arrow );
            break;
        default:
            a.setPoints( TQCOORDARRLEN( win_style_r_arrow ), win_style_r_arrow );
    }

    p->save();
    if ( down )
    {
        p->translate( style->pixelMetric( TQStyle::PM_ButtonShiftHorizontal, ceData, elementFlags ),
                      style->pixelMetric( TQStyle::PM_ButtonShiftVertical, ceData, elementFlags ) );
    }

    if ( enabled )
    {
        a.translate( x + w / 2, y + h / 2 );
        p->setPen( cg.buttonText() );
        p->drawLineSegments( a );
    }
    else
    {
        a.translate( x + w / 2 + 1, y + h / 2 + 1 );
        p->setPen( cg.light() );
        p->drawLineSegments( a );
        a.translate( -1, -1 );
        p->setPen( cg.mid() );
        p->drawLineSegments( a );
    }

    p->restore();

}



TQSize KThemeStyle::sizeFromContents( ContentsType contents,
                                     const TQStyleControlElementData &ceData,
                                     ControlElementFlags elementFlags,
                                     const TQSize &contentSize,
                                     const TQStyleOption& opt,
                                     const TQWidget* widget ) const
{
    switch ( contents )
    {
            // PUSHBUTTON SIZE
            // ------------------------------------------------------------------
        case CT_PushButton:
            {
                const TQPushButton * button = ( const TQPushButton* ) widget;
                int w = contentSize.width();
                int h = contentSize.height();
                int bm = pixelMetric( PM_ButtonMargin, ceData, elementFlags, widget );
                int fw = pixelMetric( PM_DefaultFrameWidth, ceData, elementFlags, widget ) * 2;

                w += bm + fw + 6; // ### Add 6 to make way for bold font.
                h += bm + fw;

                // Ensure we stick to standard width and heights.
                if ( button->isDefault() || button->autoDefault() )
                {
                    if ( w < 80 && !button->text().isEmpty() )
                        w = 80;
                }

                if ( h < 22 )
                    h = 22;

                return TQSize( w, h );
            }

            // POPUPMENU ITEM SIZE
            // -----------------------------------------------------------------
        case CT_PopupMenuItem:
            {
                if ( ! widget || opt.isDefault() )
                    return contentSize;

                const TQPopupMenu *popup = ( const TQPopupMenu * ) widget;
                bool checkable = popup->isCheckable();
                TQMenuItem *mi = opt.menuItem();
                int maxpmw = opt.maxIconWidth();
                int w = contentSize.width(), h = contentSize.height();

                if ( mi->custom() )
                {
                    w = mi->custom() ->sizeHint().width();
                    h = mi->custom() ->sizeHint().height();
                    if ( ! mi->custom() ->fullSpan() )
                        h += 2 * itemVMargin + 2 * itemFrame;
                }
                else if ( mi->widget() )
                {}
                else if ( mi->isSeparator() )
                {
                    w = 10; // Arbitrary
                    h = 2;
                }
                else
                {
                    if ( mi->pixmap() )
                        h = TQMAX( h, mi->pixmap() ->height() + 2 * itemFrame );
                    else
                        h = TQMAX( h, popup->fontMetrics().height()
                                  + 2 * itemVMargin + 2 * itemFrame );

                    if ( mi->iconSet() )
                        h = TQMAX( h, mi->iconSet() ->pixmap(
                                      TQIconSet::Small, TQIconSet::Normal ).height() +
                                  2 * itemFrame );
                }

                if ( ! mi->text().isNull() && mi->text().find( '\t' ) >= 0 )
                    w += 12;
                else if ( mi->popup() )
                    w += 2 * arrowHMargin;

                if ( maxpmw )
                    w += maxpmw + 6;
                if ( checkable && maxpmw < 20 )
                    w += 20 - maxpmw;
                if ( checkable || maxpmw > 0 )
                    w += 12;

                w += rightBorder;

                return TQSize( w, h );
            }
            
        default:
            return KThemeBase::sizeFromContents( contents, ceData, elementFlags, contentSize, opt, widget );
    }
}


TQRect KThemeStyle::subRect(SubRect sr, const TQStyleControlElementData &ceData, const ControlElementFlags elementFlags, const TQWidget* widget) const
{
    if (sr == SR_CheckBoxFocusRect)
    {
        const TQCheckBox* cb = static_cast<const TQCheckBox*>(widget);

        //Only checkbox, no label
        if (cb->text().isEmpty() && (cb->pixmap() == 0) )
        {
            TQRect bounding = cb->rect();

            int   cw = pixelMetric(PM_IndicatorWidth, ceData, elementFlags, widget);
            int   ch = pixelMetric(PM_IndicatorHeight, ceData, elementFlags, widget);

            TQRect checkbox(bounding.x() + 2, bounding.y() + 2 + (bounding.height() - ch)/2,  cw - 4, ch - 4);

            return checkbox;
        }
    }
    return TDEStyle::subRect(sr, ceData, elementFlags, widget);
}

int KThemeStyle::pixelMetric ( PixelMetric metric, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQWidget * widget ) const
{
    switch ( metric )
    {
        case PM_MenuBarFrameWidth:
            return 1;
        case PM_DefaultFrameWidth:
            return ( frameWidth() );

        case PM_ButtonMargin:
            return decoWidth( PushButton ) > decoWidth( PushButtonDown ) ?
                   3 + decoWidth( PushButton ) : 3 + decoWidth( PushButtonDown );

        case PM_ScrollBarExtent:
        case PM_SliderThickness:  //Should this be 16 always?
            return getSBExtent();

        case PM_ButtonDefaultIndicator:
            return 0;

        case PM_ButtonShiftHorizontal:
            return buttonXShift();

        case PM_ButtonShiftVertical:
            return buttonYShift();

        case PM_ExclusiveIndicatorWidth:
            if ( isPixmap( ExIndicatorOn ) )
                return ( uncached( ExIndicatorOn ) ->size().width() );
            else
                return KThemeBase::pixelMetric ( metric, ceData, elementFlags, widget );

        case PM_ExclusiveIndicatorHeight:
            if ( isPixmap( ExIndicatorOn ) )
                return ( uncached( ExIndicatorOn ) ->size().height() );
            else
                return KThemeBase::pixelMetric ( metric, ceData, elementFlags, widget );


        case PM_IndicatorWidth:
            if ( isPixmap( IndicatorOn ) )
                return ( uncached( IndicatorOn ) ->size().width() );
            else
                return KThemeBase::pixelMetric ( metric, ceData, elementFlags, widget );

        case PM_IndicatorHeight:
            if ( isPixmap( IndicatorOn ) )
                return ( uncached( IndicatorOn ) ->size().height() );
            else
                return KThemeBase::pixelMetric ( metric, ceData, elementFlags, widget );

        case PM_SliderLength:
            return ( sliderButtonLength() );

        case PM_SplitterWidth:
            return ( splitWidth() );

        case PM_MenuIndicatorFrameHBorder:
        case PM_MenuIndicatorFrameVBorder:
        case PM_MenuIconIndicatorFrameHBorder:
        case PM_MenuIconIndicatorFrameVBorder:
            return 0;

        default:
            return KThemeBase::pixelMetric ( metric, ceData, elementFlags, widget );
    }
}



KThemeStyle::KThemeStyle( const TQString& configDir, const TQString &configFile )
        : KThemeBase( configDir, configFile ), paletteSaved( false ), polishLock( false ), menuCache( 0 ), vsliderCache( 0 ),
         brushHandle( 0 ), brushHandleSet( false ), kickerMode( false )
{
    mtfstyle = TQStyleFactory::create( "Motif" );
    if ( !mtfstyle )
        mtfstyle = TQStyleFactory::create( *( TQStyleFactory::keys().begin() ) );
}

KThemeStyle::~KThemeStyle()
{
    delete vsliderCache;
    delete menuCache;

}


void KThemeStyle::applicationPolish( const TQStyleControlElementData &ceData, ControlElementFlags, void *ptr )
{
    if (ceData.widgetObjectTypes.contains("TQApplication")) {
        TQApplication *app = reinterpret_cast<TQApplication*>(ptr);
        if (!qstrcmp(app->argv()[0], "kicker")) {
            kickerMode = true;
        }
    }
}


void KThemeStyle::polish( TQPalette &p )
{
    if ( polishLock )
    {
        return ; //Palette polishing disabled ...
    }



    if ( !paletteSaved )
    {
        oldPalette = p;
        paletteSaved = true;
    }

    p = overridePalette( p );

    if ( isPixmap( Background ) )
    {
        TQBrush bgBrush( p.color( TQPalette::Normal,
                                TQColorGroup::Background ),
                                *uncached( Background ) );
        brushHandle = uncached( Background )->handle();
        brushHandleSet = true;
        p.setBrush( TQColorGroup::Background, bgBrush );
    }

}

void KThemeStyle::paletteChanged()
{
    TQPalette p = TQApplication::palette();
    polish( p );
    TQApplication::setPalette( p );
}


void KThemeStyle::applicationUnPolish( const TQStyleControlElementData &ceData, ControlElementFlags, void *ptr )
{
    if (ceData.widgetObjectTypes.contains("TQApplication")) {
        TQApplication *app = reinterpret_cast<TQApplication*>(ptr);
        app->setPalette( oldPalette, true );
    }
}

bool KThemeStyle::objectEventHandler( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void* source, TQEvent *event )
{
    if (ceData.widgetObjectTypes.contains("TQObject")) {
        TQObject* object = reinterpret_cast<TQObject*>(source);

        if( object->inherits("KActiveLabel"))
        {
            if(event->type() == TQEvent::Move || event->type() == TQEvent::Resize ||
                event->type() == TQEvent::Show)
            {
                TQWidget *w = static_cast<TQWidget*>(object);
                TQPoint pos(0, 0);
                pos = w->mapTo(w->topLevelWidget(), pos);
                TQPixmap pix(uncached( Background )->size());
                TQPainter p;
                p.begin(&pix);
                p.drawTiledPixmap(0, 0,
                                uncached( Background )->width(),
                                uncached( Background )->height() ,
                                *uncached( Background ),
                                pos.x(), pos.y());
                p.end();
                TQPalette pal(w->palette());
                TQBrush brush( pal.color( TQPalette::Normal,
                                                        TQColorGroup::Background),
                                    pix );
                pal.setBrush(TQColorGroup::Base, brush);
                w->setPalette(pal);
            }
        }
        if (!qstrcmp(object->name(), "tde toolbar widget") && object->inherits("TQLabel"))
        {
            TQWidget* lb = static_cast<TQWidget*>(object);
            if (lb->backgroundMode() == TQt::PaletteButton)
                lb->setBackgroundMode(TQt::PaletteBackground);
            removeObjectEventHandler(ceData, elementFlags, source, this);
        }
    }

    return TDEStyle::objectEventHandler(ceData, elementFlags, source, event);
}

void KThemeStyle::polish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr )
{
    if (ceData.widgetObjectTypes.contains("TQWidget")) {
        TQWidget *w = reinterpret_cast<TQWidget*>(ptr);

        if (::tqt_cast<TQStatusBar*>(w))
            w->setPaletteBackgroundColor(TQApplication::palette().color(TQPalette::Normal, TQColorGroup::Background));

        if (::tqt_cast<TQLabel*>(w) && !qstrcmp(w->name(), "tde toolbar widget"))
            installObjectEventHandler(ceData, elementFlags, ptr, this);

        if (w->backgroundPixmap() && !w->isTopLevel() && 
            (!kickerMode || 
            (!w->inherits("TaskBar") && !w->inherits("TaskBarContainer") && !w->inherits("TaskbarApplet") && !w->inherits("ContainerArea") && !w->inherits("AppletHandle"))))
        {
            //The brushHandle check verifies that the bg pixmap is actually the brush..
            if (!brushHandleSet || brushHandle == w->backgroundPixmap()->handle())
            {
                w->setBackgroundOrigin( TQWidget::WindowOrigin );
            }
        }

        if (w->inherits("KActiveLabel"))
        {
            if (uncached( Background ))
                installObjectEventHandler(ceData, elementFlags, ptr, this);
        }

        if ( w->inherits( "QTipLabel" ) )
        {
            polishLock = true;

            TQColorGroup clrGroup( TQt::black, TQColor( 255, 255, 220 ),
                                TQColor( 96, 96, 96 ), TQt::black, TQt::black,
                                TQt::black, TQColor( 255, 255, 220 ) );
            TQPalette toolTip ( clrGroup, clrGroup, clrGroup );

            TQToolTip::setPalette( toolTip );
            polishLock = false;
        }

        if ( w->inherits( "KonqIconViewWidget" ) )   //Konqueror background hack/workaround
        {
            w->setPalette( oldPalette );
            return ;
        }

        if ( ::tqt_cast<TQMenuBar*>(w) )
        {
            w->setBackgroundMode( TQWidget::NoBackground );
        }
        else if ( w->inherits( "TDEToolBarSeparator" ) || w->inherits( "QToolBarSeparator" ) )
        {
            w->setBackgroundMode( TQWidget::PaletteBackground );
        }
        else if ( ::tqt_cast<TQPopupMenu*>(w) )
        {
            popupPalette = w->palette();
            if ( isColor( MenuItem ) || isColor( MenuItemDown ) )
            {
                TQPalette newPal( w->palette() );
                if ( isColor( MenuItem ) )
                {
                    newPal.setActive( *colorGroup( newPal.active(), MenuItem ) );
                    newPal.setDisabled( *colorGroup( newPal.active(), MenuItem ) );
                }
                if ( isColor( MenuItemDown ) )
                {
                    newPal.setActive( *colorGroup( newPal.active(), MenuItemDown ) );
                }
                w->setPalette( newPal );
            }

            w->setBackgroundMode( TQWidget::NoBackground );
        }
        else if ( ::tqt_cast<TQCheckBox*>(w) )
        {
            if ( isColor( IndicatorOff ) || isColor( IndicatorOn ) )
            {
                TQPalette newPal( w->palette() );
                if ( isColor( IndicatorOff ) )
                {
                    newPal.setActive( *colorGroup( newPal.active(), IndicatorOff ) );
                    newPal.setDisabled( *colorGroup( newPal.active(), IndicatorOff ) );
                }
                if ( isColor( IndicatorOn ) )
                    newPal.setActive( *colorGroup( newPal.active(), IndicatorOn ) );
                w->setPalette( newPal );
            }
        }
        else if ( ::tqt_cast<TQRadioButton*>(w) )
        {
            if ( isColor( ExIndicatorOff ) || isColor( ExIndicatorOn ) )
            {
                TQPalette newPal( w->palette() );
                if ( isColor( ExIndicatorOff ) )
                {
                    newPal.setActive( *colorGroup( newPal.active(), ExIndicatorOff ) );
                    newPal.setDisabled( *colorGroup( newPal.active(),
                                                    ExIndicatorOff ) );
                }
                if ( isColor( ExIndicatorOn ) )
                    newPal.setActive( *colorGroup( newPal.active(), ExIndicatorOn ) );
                w->setPalette( newPal );
            }
        }
    }

    TDEStyle::polish( ceData, elementFlags, ptr );
}

void KThemeStyle::unPolish( const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, void *ptr )
{
    if (ceData.widgetObjectTypes.contains("TQWidget")) {
        TQWidget *w = reinterpret_cast<TQWidget*>(ptr);

        if (w->backgroundPixmap() && !w->isTopLevel())
        {
            //The brushHandle check verifies that the bg pixmap is actually the brush..
            if (!brushHandleSet || brushHandle ==w->backgroundPixmap()->handle())
            {
                w->setBackgroundOrigin( TQWidget::WidgetOrigin );
            }
        }

        //Toolbar labels should nornally be PaletteButton
        if ( ::tqt_cast<TQLabel*>(w) && !qstrcmp(w->name(), "tde toolbar widget"))
            w->setBackgroundMode( TQWidget::PaletteButton );

        //The same for menu bars, popup menus
        else if ( ::tqt_cast<TQMenuBar*>(w) || ::tqt_cast<TQPopupMenu*>(w) )
            w->setBackgroundMode( TQWidget::PaletteButton );

        //For toolbar internal separators, return to button, too (can't use tqt_cast here since don't have access to the class)
        else if ( w->inherits( "TDEToolBarSeparator" ) || w->inherits( "QToolBarSeparator" ) )
            w->setBackgroundMode( TQWidget::PaletteButton );

        //For scrollbars, we don't do much, since the widget queries the style on the switch

        //Drop some custom palettes. ### this really should check the serial number to be 100% correct.
        if ( ::tqt_cast<TQPopupMenu*>(w) || ::tqt_cast<TQCheckBox*>(w) || ::tqt_cast<TQRadioButton*>(w) || ::tqt_cast<TQStatusBar*>(w) )
         w->unsetPalette();
    }

    TDEStyle::unPolish( ceData, elementFlags, ptr );
}


void KThemeStyle::drawBaseButton( TQPainter *p, int x, int y, int w, int h,
                                  const TQColorGroup &g, bool sunken, bool
                                  rounded, WidgetType type ) const
{
    int offset = borderPixmap( type ) ? 0 : decoWidth( type ) ; //##### This is wrong, but the code relies on it..
    TQPen oldPen = p->pen();

    // handle reverse bevel here since it uses decowidth differently
    if ( gradientHint( type ) == GrReverseBevel )
    {
        int i;
        bitBlt( p->device(), x, y, scalePixmap( w, h, type ), 0, 0, w, h,
                TQt::CopyROP, true );
        p->setPen( g.text() );
        for ( i = 0; i < borderWidth( type ); ++i, ++x, ++y, w -= 2, h -= 2 )
            p->drawRect( x, y, w, h );
    }
    // same with KDE style borders
    else if ( !borderPixmap( type ) && shade() == KDE )
    {
        kDrawBeButton( p, x, y, w, h, g, sunken );
        if ( isPixmap( type ) )
            p->drawTiledPixmap( x + 4, y + 4, w - 6, h - 6,
                                *scalePixmap( w - 6, h - 6,
                                              type ) );
        else
            p->fillRect( x + 4, y + 4, w - 6, h - offset * 6,
                         g.brush( TQColorGroup::Button ) );

    }
    else
    {
        if ( ( w - offset * 2 ) > 0 && ( h - offset * 2 ) > 0 )
        {
            if ( isPixmap( type ) )
                if ( rounded )
                    p->drawTiledPixmap( x, y, w, h, *scalePixmap( w, h, type ) );
                else
                    p->drawTiledPixmap( x + offset, y + offset, w - offset * 2,
                                        h - offset * 2,
                                        *scalePixmap( w - offset * 2, h - offset * 2,
                                                      type ) );
            else if ( 1 )  //##### TODO - Get this optimization working... !borderPixmap( type ) || (( w - decoWidth(type) * 2 ) > 0 && ( h - decoWidth(type) * 2 ) > 0) )
                //Sometimes border covers the whole thing - in that case, avoid drawing the base.
            {
                p->fillRect( x + offset, y + offset, w - offset * 2, h - offset * 2,
                             g.brush( TQColorGroup::Button ) );
            }
        }
        if ( borderPixmap( type ) )
        {
            bitBlt( p->device(), x, y, scaleBorder( w, h, type ), 0, 0, w, h,
                    TQt::CopyROP, false );
        }
        else
            drawShade( p, x, y, w, h, g, sunken, rounded,
                       highlightWidth( type ), borderWidth( type ), shade() );
    }
    p->setPen( oldPen );
}

void KThemeStyle::drawPrimitive ( PrimitiveElement pe, TQPainter * p, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQRect & r, const TQColorGroup & g_base,
                                  SFlags flags, const TQStyleOption & opt ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );

    bool sunken = ( flags & Style_Sunken );
    bool enabled = ( flags & Style_Enabled );
    bool down = ( flags & Style_Down );
    bool on = flags & Style_On;
    bool active = flags & Style_Active;
    TQColorGroup g = g_base;

    switch ( pe )
    {
        case PE_ArrowUp:
        case PE_ArrowDown:
        case PE_ArrowRight:
        case PE_ArrowLeft:
            {
                TQRect r( x, y, w, h );
                if ( r.width() > 12 )
                {
                    r.setRect( r.x() + ( r.width() - 12 ) / 2, r.y(), 12, r.height() );
                }
                if ( r.height() > 12 )
                {
                    r.setRect( r.x(), r.y() + ( r.height() - 12 ) / 2, r.width(), 12 );
                }
                r.rect( &x, &y, &w, &h );
                // Handles pixmapped arrows. A little inefficient because you can specify
                // some as pixmaps and some as default types.
                WidgetType widget;
                switch ( pe )
                {
                    case PE_ArrowUp:
                    widget = enabled ? down ? SunkenArrowUp : ArrowUp : DisArrowUp;
                        break;
                    case PE_ArrowDown:
                    widget = enabled ? down ? SunkenArrowDown : ArrowDown : DisArrowDown;
                        break;
                    case PE_ArrowLeft:
                    widget = enabled ? down ? SunkenArrowLeft : ArrowLeft : DisArrowLeft;
                        break;
                    case PE_ArrowRight:
                    default:
                    widget = enabled ? down ? SunkenArrowRight : ArrowRight : DisArrowRight;
                        break;
                }
                if ( isPixmap( widget ) )
                {
                    bitBlt( p->device(), x + ( w - uncached( widget ) ->width() ) / 2,
                            y + ( h - uncached( widget ) ->height() ) / 2,
                            uncached( widget ) );

                    return ;
                }
                const TQColorGroup *cg = colorGroup( g, widget );
                // Standard arrow types
                if ( arrowType() == MotifArrow )
                {
                    mtfstyle->drawPrimitive( pe, p, ceData, elementFlags, r, g, flags, opt );

                    handled = true;
                }
                else if ( arrowType() == SmallArrow )
                {
                    // #### FIXME: This should be like the Platinum style - uses HighColor look for now
                    TQPointArray a;

                    switch ( pe )
                    {
                        case PE_ArrowUp:
                            a.setPoints( TQCOORDARRLEN( u_arrow ), u_arrow );
                            break;

                        case PE_ArrowDown:
                            a.setPoints( TQCOORDARRLEN( d_arrow ), d_arrow );
                            break;

                        case PE_ArrowLeft:
                            a.setPoints( TQCOORDARRLEN( l_arrow ), l_arrow );
                            break;

                        default:
                            a.setPoints( TQCOORDARRLEN( r_arrow ), r_arrow );
                    }

                    p->save();

                    if ( flags & Style_Down )
                        p->translate( pixelMetric( PM_ButtonShiftHorizontal, ceData, elementFlags ),
                                      pixelMetric( PM_ButtonShiftVertical, ceData, elementFlags ) );

                    if ( flags & Style_Enabled )
                    {
                        a.translate( r.x() + r.width() / 2, r.y() + r.height() / 2 );
                        p->setPen( cg->buttonText() );
                        p->drawLineSegments( a );
                    }
                    else
                    {
                        a.translate( r.x() + r.width() / 2 + 1, r.y() + r.height() / 2 + 1 );
                        p->setPen( cg->mid() );
                        p->drawLineSegments( a );
                    }
                    p->restore();
                }
                else
                {
                    TQPointArray a;
                    int x2 = x + w - 1, y2 = y + h - 1;
                    switch ( pe )
                    {
                        case PE_ArrowUp:
                            a.setPoints( 4, x, y2, x2, y2, x + w / 2, y, x, y2 );
                            break;
                        case PE_ArrowDown:
                            a.setPoints( 4, x, y, x2, y, x + w / 2, y2, x, y );
                            break;
                        case PE_ArrowLeft:
                            a.setPoints( 4, x2, y, x2, y2, x, y + h / 2, x2, y );
                            break;
                        default:
                            a.setPoints( 4, x, y, x, y2, x2, y + h / 2, x, y );
                            break;
                    }
                    TQBrush oldBrush = p->brush();
                    TQPen oldPen = p->pen();
                    p->setBrush( cg->brush( TQColorGroup::Shadow ) );
                    p->setPen( cg->shadow() );
                    p->drawPolygon( a );
                    p->setBrush( oldBrush );
                    p->setPen( oldPen );
                    handled = true;
                }
                break;

            }
        case PE_HeaderSectionMenu:
        case PE_HeaderSection:
            {
                sunken = false; //Never mind this one
            }
        case PE_ButtonBevel:
            {
                WidgetType type = ( sunken || on || down ) ? BevelDown : Bevel;
                drawBaseButton( p, x, y, w, h, *colorGroup( g, type ), ( sunken || on || down ), false, type );
                handled = true;
                break;
            }
        case PE_ButtonCommand:
            {
                drawBaseButton( p, x, y, w, h, g, ( sunken || on || down ), roundButton(), ( sunken || on || down ) ?
                                PushButtonDown : PushButton );
                handled = true;
                break;
            }
        case PE_PanelDockWindow:
            {
                drawBaseButton( p, x, y, w, h, *colorGroup( g, ToolBar ), false, false,
                                ToolBar );
                handled = true;
                break;
            }
        case PE_CheckMark:
            {
                if ( isPixmap( CheckMark ) )
                {
                    if ( flags & Style_Enabled || flags & Style_On )
                        bitBlt( p->device(), x + ( w - uncached( CheckMark ) ->width() ) / 2,
                                y + ( h - uncached( CheckMark ) ->height() ) / 2,
                                uncached( CheckMark ) );
                    handled = true;
                }
                else //Small hack to ensure the checkmark gets painter proper color..
                {
                    g.setColor( TQColorGroup::Text, g.buttonText() );
                }
                break;
            }
        case PE_ExclusiveIndicator:
            {
                if ( isPixmap( ( flags & Style_On || flags & Style_Down ) ? ExIndicatorOn : ExIndicatorOff ) )
                {
                    p->drawPixmap( x, y, *uncached( ( flags & Style_On || flags & Style_Down ) ? ExIndicatorOn :
                                                         ExIndicatorOff ) );
                    handled = true;
                }

                break;
            }
        case PE_ExclusiveIndicatorMask:
            {
                if ( isPixmap( ( flags & Style_On || flags & Style_Down ) ? ExIndicatorOn : ExIndicatorOff ) )
                {
                    const TQBitmap * mask = uncached( ( flags & Style_On || flags & Style_Down ) ? ExIndicatorOn : ExIndicatorOff ) ->mask();
                    if ( mask )
                    {
                        p->setPen( TQt::color1 );
                        p->drawPixmap( x, y, *mask );
                    }
                    else
                        p->fillRect( x, y, w, h, TQBrush( color1, SolidPattern ) );
                    handled = true;
                }
                break;
            }

        case PE_IndicatorMask:
            {
                if ( isPixmap( ( flags & Style_On ) ? IndicatorOn : IndicatorOff ) )
                {
                    const TQBitmap * mask = uncached( ( flags & Style_On ) ? IndicatorOn :
                                                     IndicatorOff ) ->mask();
                    if ( mask )
                    {
                        p->setPen( TQt::color1 );
                        p->drawPixmap( x, y, *mask );
                    }
                    else
                        p->fillRect( x, y, w, h, TQBrush( color1, SolidPattern ) );
                    handled = true;
                }
                break;
            }
        case PE_Indicator:
            {
                if ( isPixmap( ( flags & Style_On || flags & Style_Down ) ?
                               IndicatorOn : IndicatorOff ) )
                {
                    p->drawPixmap( x, y, *uncached( ( flags & Style_On || flags & Style_Down ) ?
                                                         IndicatorOn : IndicatorOff ) );
                    handled = true;
                }
                break;
            }
        case PE_Splitter:
            {
                drawBaseButton( p, x, y, w, h, *colorGroup( g, Splitter ), false, false,
                                Splitter );
                handled = true;
                break;
            }
        case PE_FocusRect:
            {                
                if ( is3DFocus() )
                {
                    p->setPen( g.dark() );
                    int i = focusOffset();
                    p->drawLine( r.x() + i, r.y() + 1 + i, r.x() + i, r.bottom() - 1 - i );
                    p->drawLine( r.x() + 1 + i, r.y() + i, r.right() - 1 - i, r.y() + i );
                    p->setPen( g.light() );
                    p->drawLine( r.right() - i, r.y() + 1 + i, r.right() - i, r.bottom() - 1 - i );
                    p->drawLine( r.x() + 1 + i, r.bottom() - i, r.right() - 1 - i, r.bottom() - i );
                    handled = true;
                }
                else
                {
                    handled = true;
                    p->drawWinFocusRect(r);
                }
                break;
            }
        case PE_PanelMenuBar:
            {
                TQPixmap* cache = makeMenuBarCache(w, h);
                p->drawPixmap( x, y, *cache);
                handled = true;
                break;
            }
        case PE_ScrollBarAddPage:
        case PE_ScrollBarSubPage:
            {
                WidgetType widget = ( flags & Style_Horizontal ) ? HScrollGroove : VScrollGroove;

                if ( !isPixmap( widget ) )
                {
                    p->fillRect( r, colorGroup( g, widget ) ->brush( TQColorGroup::Background ) );
                    // Do the borders and frame
                    drawShade( p, r.x(), r.y(), r.width(),
                               r.height(), *colorGroup( g, widget ), true, false,
                               highlightWidth( widget ), borderWidth( widget ), shade() );
                }
                else
                {
                    // If the groove is pixmapped we make a full-sized image (it gets
                    // cached) then bitBlt it to the appropriate rect.
                    p->drawTiledPixmap( r.x(), r.y(), r.width(), r.height(),
                                        *scalePixmap( r.width(), r.height(),
                                                      widget ) );
                    drawShade( p, r.x(), r.y(), r.width(),
                               r.height(), *colorGroup( g, widget ), true, false,
                               highlightWidth( widget ), borderWidth( widget ), shade() );
                }

                handled = true;
                break;
            }
        case PE_ScrollBarAddLine:
            {
                bool horizontal = ( flags & Style_Horizontal );
                drawBaseButton( p, r.x(), r.y(), r.width(), r.height(),
                                *colorGroup( g, down ? ScrollButtonDown : ScrollButton ),
                                down, false, down ? ScrollButtonDown : ScrollButton );

                drawPrimitive( ( horizontal ) ? PE_ArrowRight : PE_ArrowDown, p, ceData, elementFlags ,
                               TQRect( r.x() + 3, r.y() + 3, r.width() - 6, r.height() - 6 ),
                               *colorGroup( g, down ? ScrollButtonDown : ScrollButton ),
                               flags );

                handled = true;
                break;
            }
        case PE_ScrollBarSubLine:
            {
                bool horizontal = ( flags & Style_Horizontal );
                drawBaseButton( p, r.x(), r.y(), r.width(), r.height(),
                                *colorGroup( g, down ? ScrollButtonDown : ScrollButton ),
                                down, false, down ? ScrollButtonDown : ScrollButton );

                drawPrimitive( ( horizontal ) ? PE_ArrowLeft : PE_ArrowUp, p, ceData, elementFlags ,
                               TQRect( r.x() + 3, r.y() + 3, r.width() - 6, r.height() - 6 ),
                               *colorGroup( g, down ? ScrollButtonDown : ScrollButton ),
                               flags );
                handled = true;
                break;
            }
        case PE_ScrollBarSlider:
            {
                bool active = ( flags & Style_Active ) || ( flags & Style_Down ); //activeControl == TQStyle::AddLine;
                bool horizontal = ( flags & Style_Horizontal );
                int offsetH = horizontal ? 0: decoWidth(VScrollGroove) ;
                int offsetV = horizontal ? decoWidth(HScrollGroove):0;

                WidgetType widget = horizontal ?
                                active ? HScrollBarSliderDown : HScrollBarSlider :
                                    active ? VScrollBarSliderDown : VScrollBarSlider;
                drawBaseButton( p, r.x()+offsetH, r.y()+offsetV, r.width()-2*offsetH,
                                r.height()-2*offsetV, *colorGroup( g, widget ), active, false,
                                widget );

                int spaceW = horizontal ? r.width() - decoWidth( widget ) - 4 :
                             r.width();
                int spaceH = horizontal ? r.height() :
                             r.height() - decoWidth( widget ) - 4;

                widget = active ? horizontal ? HScrollDecoDown : VScrollDecoDown :
                         horizontal ? HScrollDeco : VScrollDeco;
                if ( isPixmap( widget ) )
                {
                    if ( spaceW >= uncached( widget ) ->width() &&
                            spaceH >= uncached( widget ) ->height() )
                    {
                        bitBlt( p->device(),
                                r.x() + ( r.width() - uncached( widget ) ->width() ) / 2,
                                r.y() + ( r.height() - uncached( widget ) ->height() ) / 2,
                                uncached( widget ) );
                    }
                }
                handled = true;
                break;

            }

        case PE_MenuItemIndicatorFrame:
            {
                // Draw nothing
                break;
            }
        case PE_MenuItemIndicatorIconFrame:
            {
                int x, y, w, h;
                r.rect( &x, &y, &w, &h );
                int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, opt, NULL, NULL);
                bool reverse = TQApplication::reverseLayout();

                int cx = reverse ? x + w - checkcol : x;
                TQRect cr = visualRect( TQRect( x, y, checkcol, h ), r );
                const TQColorGroup& cg_ours = *colorGroup( g_base, active ? MenuItemDown : MenuItem );
                drawBaseButton( p, cr.x(), cr.y(), cr.width(), cr.height(), *colorGroup( cg_ours, BevelDown ), true, false, BevelDown );
                break;
            }
        case PE_MenuItemIndicatorCheck:
            {
                int x, y, w, h;
                r.rect( &x, &y, &w, &h );
                int checkcol = styleHint(SH_MenuIndicatorColumnWidth, ceData, elementFlags, opt, NULL, NULL);
                bool reverse = TQApplication::reverseLayout();

                int cx = reverse ? x + w - checkcol : x;
                TQRect cr = visualRect( TQRect( x, y, checkcol, h ), r );
                const TQColorGroup& cg_ours = *colorGroup( g_base, active ? MenuItemDown : MenuItem );

                SFlags cflags = Style_Default;
                cflags |= active ? Style_Enabled : Style_On;

                drawPrimitive( PE_CheckMark, p, ceData, elementFlags, TQRect( x + itemFrame, y + itemFrame, checkcol - itemFrame * 2, h - itemFrame * 2 ), cg_ours, cflags );
                break;
            }
        default:
            handled = false;
    }

    if ( !handled )
        KThemeBase::drawPrimitive ( pe, p, ceData, elementFlags, r, g,
                                    flags, opt );
}



TQPixmap* KThemeStyle::makeMenuBarCache(int w, int h) const
{
    if (menuCache)
    {
        if (menuCache->width() != w || menuCache->height() != h )
        {
            delete menuCache;
        }
        else
            return menuCache;
    }

    const TQColorGroup *g = colorGroup( TQApplication::palette().active(), MenuBar);

    menuCache = new TQPixmap ( w, h );
    TQPainter p(menuCache);
    drawBaseButton( &p, 0, 0, w, h, *g, false, false, MenuBar );
    p.end();
    return menuCache;
}


void KThemeStyle::drawControl( ControlElement element,
                               TQPainter *p,
                               const TQStyleControlElementData &ceData,
                               ControlElementFlags elementFlags,
                               const TQRect &r,
                               const TQColorGroup &cg,
                               SFlags how ,
                               const TQStyleOption& opt,
                               const TQWidget *widget ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );


    switch ( element )
    {
        case CE_PushButton:
            {
                const TQPushButton * btn = ( const TQPushButton* ) widget;
                bool sunken = btn->isOn() || btn->isDown();
                int diw = pixelMetric( PM_ButtonDefaultIndicator, ceData, elementFlags, btn );
                drawBaseButton( p, diw, diw, w - 2 * diw, h - 2 * diw,
                                *colorGroup( btn->colorGroup(), sunken ? PushButtonDown :
                                             PushButton ), sunken, roundButton(),
                                sunken ? PushButtonDown : PushButton );
                // TODO if diw, draw fancy default button indicator
                handled = true;
                break;
            }
        case CE_PushButtonLabel:
            {
                const TQPushButton* button = ( const TQPushButton* ) widget;
                bool active = button->isOn() || button->isDown();
                int x, y, w, h;
                r.rect( &x, &y, &w, &h );

                // Shift button contents if pushed.
                if ( active )
                {
                    x += pixelMetric( PM_ButtonShiftHorizontal, ceData, elementFlags, widget );
                    y += pixelMetric( PM_ButtonShiftVertical, ceData, elementFlags, widget );
                    how |= Style_Sunken;
                }

                // Does the button have a popup menu?
                if ( button->isMenuButton() )
                {
                    int dx = pixelMetric( PM_MenuButtonIndicator, ceData, elementFlags, widget );
                    drawPrimitive( PE_ArrowDown, p, ceData, elementFlags, TQRect( x + w - dx - 2, y + 2, dx, h - 4 ),
                                   cg, how, opt );
                    w -= dx;
                }

                // Draw the icon if there is one
                if ( button->iconSet() && !button->iconSet() ->isNull() )
                {
                    TQIconSet::Mode mode = TQIconSet::Disabled;
                    TQIconSet::State state = TQIconSet::Off;

                    if ( button->isEnabled() )
                        mode = button->hasFocus() ? TQIconSet::Active : TQIconSet::Normal;
                    if ( button->isToggleButton() && button->isOn() )
                        state = TQIconSet::On;

                    TQPixmap pixmap = button->iconSet() ->pixmap( TQIconSet::Small, mode, state );

                    // Center the iconset if there's no text or pixmap
                    if (button->text().isEmpty() && !button->pixmap())
                        p->drawPixmap( x + (w - pixmap.width())  / 2,
                                       y + (h - pixmap.height()) / 2, pixmap );
                    else
                        p->drawPixmap( x + 4, y + (h - pixmap.height()) / 2, pixmap );

                    int pw = pixmap.width();
                    x += pw + 4;
                    w -= pw + 4;
                }

                // Make the label indicate if the button is a default button or not
                if ( active || button->isDefault() && button->isEnabled() )
                {
                    // Draw "fake" bold text  - this enables the font metrics to remain
                    // the same as computed in TQPushButton::sizeHint(), but gives
                    // a reasonable bold effect.
                    int i;

                    // Text shadow
                    for ( i = 0; i < 2; i++ )
                        drawItem( p, TQRect( x + i + 1, y + 1, w, h ), AlignCenter | ShowPrefix,
                                  button->colorGroup(), button->isEnabled(), NULL,
                                  button->text(), -1,
                                  active ? &button->colorGroup().dark() : &button->colorGroup().mid() );

                    // Normal Text
                    for ( i = 0; i < 2; i++ )
                        drawItem( p, TQRect( x + i, y, w, h ), AlignCenter | ShowPrefix,
                                  button->colorGroup(), true, i == 0 ? button->pixmap() : NULL,
                                  button->text(), -1,
                                  active ? &button->colorGroup().light() : &button->colorGroup().buttonText() );
                }
                else
                {
                    if ( button->isEnabled() )
                    {
                        drawItem( p, TQRect( x, y, w, h ), AlignCenter | ShowPrefix, button->colorGroup(),
                                  true, button->pixmap(), button->text(), -1,
                                  active ? &button->colorGroup().light() : &button->colorGroup().buttonText() );
                    }
                    else
                    {
                        //TODO: Handle reversed
                        drawItem( p, TQRect( x + 1, y + 1, w, h ), AlignCenter | ShowPrefix, button->colorGroup(),
                                  true, button->pixmap(), button->text(), -1,
                                  &button->colorGroup().light() );

                        drawItem( p, TQRect( x, y, w, h ), AlignCenter | ShowPrefix, button->colorGroup(),
                                  true, button->pixmap(), button->text(), -1,
                                  &button->colorGroup().buttonText() );
                    }
                }

                // Draw a focus rect if the button has focus
                if ( how & Style_HasFocus )
                    drawPrimitive( PE_FocusRect, p, ceData, elementFlags,
                                   TQStyle::visualRect( subRect( SR_PushButtonFocusRect, ceData, elementFlags, widget ), ceData, elementFlags ),
                                   cg, how );
                handled = true;
                break;
            }
            
        case CE_MenuBarEmptyArea:
            {
                //Expand to cover entire region
                drawPrimitive(PE_PanelMenuBar, p, ceData, elementFlags, 
                             TQRect(0,0,r.width()+r.x()*2, r.height()+r.y()*2),
                             cg, Style_Default);
                handled = true;
                break;
            }

        case CE_TabBarTab:
            {
                TQTabBar::Shape tbs = ceData.tabBarData.shape;
                bool selected = how & Style_Selected;
                WidgetType widget = selected ? ActiveTab : InactiveTab;
                const TQColorGroup *cg = colorGroup( ceData.colorGroup, widget );
                int i;
                int x2 = x + w - 1, y2 = y + h - 1;
                int bWidth = borderWidth( widget );
                int hWidth = highlightWidth( widget );
                handled = true;
                if ( tbs == TQTabBar::RoundedAbove || tbs == TQTabBar::TriangularAbove )
                {
                    if ( !selected )
                    {
                        p->fillRect( x, y, x2 - x + 1, 2,
                                     ceData.palette.active().brush( TQColorGroup::Background ) );
                        y += 2;
                    }
                    p->setPen( cg->text() );
                    i = 0;
                    if ( i < bWidth )
                    {
                        p->drawLine( x, y + 1, x, y2 );
                        p->drawLine( x2, y + 1, x2, y2 );
                        p->drawLine( x + 1, y, x2 - 1, y );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y2, x2, y2 );
                            --y2;
                        }
                        ++i, ++x, ++y, --x2;
                    }
                    for ( ; i < bWidth; ++i, ++x, ++y, --x2 )
                    {
                        p->drawLine( x, y, x, y2 );
                        p->drawLine( x2, y, x2, y2 );
                        p->drawLine( x, y, x2, y );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y2, x2, y2 );
                            --y2;
                        }
                    }
                    i = 0;
                    if ( i < hWidth && bWidth == 0 )
                    {
                        p->setPen( cg->light() );
                        p->drawLine( x, y + 1, x, y2 );
                        p->drawLine( x + 1, y, x2 - 1, y );
                        p->setPen( cg->dark() );
                        p->drawLine( x2, y + 1, x2, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y2, x2, y2 );
                            --y2;
                        }
                        ++i, ++x, ++y, --x2;
                    }
                    for ( ; i < hWidth; ++i, ++x, ++y, --x2 )
                    {
                        p->setPen( cg->light() );
                        p->drawLine( x, y, x, y2 );
                        p->drawLine( x, y, x2, y );
                        p->setPen( cg->dark() );
                        p->drawLine( x2, y + 1, x2, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y2, x2, y2 );
                            --y2;
                        }
                    }
                    if ( isPixmap( widget ) )
                        p->drawTiledPixmap( x, y, x2 - x + 1, y2 - y + 1,
                                            *scalePixmap( x2 - x + 1, y2 - y + 1, widget ) );
                    else
                        p->fillRect( x, y, x2 - x + 1, y2 - y + 1, cg->background() );
                }
                else if ( ceData.tabBarData.shape == TQTabBar::RoundedBelow ||
                        ceData.tabBarData.shape == TQTabBar::TriangularBelow )
                {
                    if ( widget == ActiveTab )
                        widget = RotActiveTab;
                    else
                        widget = RotInactiveTab;
                        
                    if ( !selected )
                    {
                        p->fillRect( x, y2 - 2, x2 - x + 1, 2,
                                     ceData.palette.active().brush( TQColorGroup::Background ) );
                        y2 -= 2;
                    }
                    p->setPen( cg->text() );
                    i = 0;
                    if ( i < bWidth )
                    {
                        p->drawLine( x, y, x, y2 - 1 );
                        p->drawLine( x2, y, x2, y2 - 1 );
                        p->drawLine( x + 1, y2, x2 - 1, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y, x2, y );
                            ++y;
                        }
                    }
                    for ( ; i < bWidth; ++i, ++x, --x2, --y2 )
                    {
                        p->drawLine( x, y, x, y2 );
                        p->drawLine( x2, y, x2, y2 );
                        p->drawLine( x, y2, x2, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y, x2, y );
                            ++y;
                        }
                    }
                    i = 0;
                    if ( i < hWidth && bWidth == 0 )
                    {
                        p->setPen( cg->dark() );
                        p->drawLine( x + 1, y2, x2 - 1, y2 );
                        p->drawLine( x2, y, x2, y2 - 1 );
                        p->setPen( cg->light() );
                        p->drawLine( x, y, x, y2 - 1 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y, x2, y );
                            ++y;
                        }
                        ++i, ++x, --x2, --y2;
                    }
                    for ( ; i < hWidth; ++i, ++x, --x2, --y2 )
                    {
                        p->setPen( cg->dark() );
                        p->drawLine( x, y2, x2, y2 );
                        p->drawLine( x2, y, x2, y2 );
                        p->setPen( cg->light() );
                        p->drawLine( x, y, x, y2 );
                        if ( selected ? activeTabLine() : inactiveTabLine() )
                        {
                            p->drawLine( x, y, x2, y );
                            ++y;
                        }
                    }
                    if ( isPixmap( widget ) )
                        p->drawTiledPixmap( x, y, x2 - x + 1, y2 - y + 1,
                                            *scalePixmap( x2 - x + 1, y2 - y + 1, widget ) );
                    else
                        p->fillRect( x, y, x2 - x + 1, y2 - y + 1, cg->background() );
                }
                break;
            }
        case CE_MenuBarItem:
            {

                r.rect( &x, &y, &w, &h );
                TQMenuItem *mi = opt.menuItem();
                TQMenuBar *mb = ( TQMenuBar* ) widget;
                TQRect pr = mb->rect();
                bool active = how & Style_Active;
                //bool focused = how & Style_HasFocus;
                const TQColorGroup *g = colorGroup( cg, active ? MenuBarItem : MenuBar );
                TQColor btext = g->buttonText();

                TQPixmap* cache = makeMenuBarCache(pr.width(), pr.height());

                TQPixmap buf( w, pr.height() );

                bitBlt(&buf, 0, 0, cache, x, y, w, pr.height());
                TQPainter p2( &buf );

                if ( active )
                {
                    drawBaseButton( &p2, 0, 0, w, h, *g, false, false, MenuBarItem );
                }
                
                p2.end();
                p->drawPixmap( x, y, buf, 0, 0, w, h );
                                
                drawItem( p, TQRect(x,y,w,h), AlignCenter | AlignVCenter | ShowPrefix | DontClip | SingleLine,
                          *g, mi->isEnabled(), mi->pixmap(), mi->text(),
                          -1, &btext );
                handled = true;
                break;
            }
        case CE_PopupMenuItem:
            {
                bool separator = false;
                int x, y, w, h;
                r.rect( &x, &y, &w, &h );

                TQMenuItem *mi = opt.menuItem();
                if ( mi )
                {
                    separator = mi->isSeparator();
                }

                int tab = opt.tabWidth();
                int checkcol = opt.maxIconWidth();
                bool enabled = (mi? mi->isEnabled():true);
                bool checkable = (elementFlags & CEF_IsCheckable);
                bool active = how & Style_Active;
                bool etchtext = styleHint( SH_EtchDisabledText, ceData, elementFlags, TQStyleOption::Default, 0, 0 );
                bool reverse = TQApplication::reverseLayout();

                const TQColorGroup& cg_ours = *colorGroup( cg, active ? MenuItemDown : MenuItem );
                //TQColor btext = cg_ours.buttonText();


                if ( checkable )
                    checkcol = TQMAX( checkcol, 20 );

                // Are we a menu item separator?
                if ( separator )
                {
                    p->setPen( cg_ours.dark() );
                    p->drawLine( x, y, x + w, y );
                    p->setPen( cg_ours.light() );
                    p->drawLine( x, y + 1, x + w, y + 1 );
                    break;
                }

                // Draw the menu item background
                if ( active )
                    drawBaseButton( p, x, y, w, h, cg_ours, true, false, MenuItemDown );
                else
                {
                    drawShade( p, x, y, w, h, *colorGroup( cg_ours, MenuItem ), false, false,
                               highlightWidth( MenuItem ), borderWidth( MenuItem ),
                               shade() );
                    int dw = decoWidth( MenuItem );
                    if ( !isPixmap( MenuItem ) )
                    {
                        p->fillRect(
                            x + dw, y + dw, w - dw * 2, h - dw * 2,
                            cg_ours.brush( TQColorGroup::Background ) );
                        //cg.brush( TQColorGroup::Background ));
                        //colorGroup( cg_ours, MenuItem ) ->brush( TQColorGroup::Background ) );
                    }
                    else
                    {
                        // process inactive item pixmaps as one large item
                        p->drawTiledPixmap( x + dw, y + dw, w - dw * 2, h - dw * 2, *scalePixmap
                                            ( w, p->window().height(), MenuItem ),
                                            x, y );
                    }
                }

                if (!mi)
                    break;

                // Do we have an icon?
                if ( mi->iconSet() )
                {
                    TQIconSet::Mode mode;
                    TQRect cr = visualRect( TQRect( x, y, checkcol, h ), r );

                    // Select the correct icon from the iconset
                    if ( active )
                        mode = enabled ? TQIconSet::Active : TQIconSet::Disabled;
                    else
                        mode = enabled ? TQIconSet::Normal : TQIconSet::Disabled;

                    // Do we have an icon and are checked at the same time?
                    // Then draw a "pressed" background behind the icon
                    if ( checkable && mi->isChecked() )  //!active && -- ??
                        drawPrimitive(PE_MenuItemIndicatorIconFrame, p, ceData, elementFlags, r, cg, how, opt);

                    // Draw the icon
                    TQPixmap pixmap = mi->iconSet() ->pixmap( TQIconSet::Small, mode );
                    int pixw = pixmap.width();
                    int pixh = pixmap.height();
                    TQRect pmr( 0, 0, pixw, pixh );
                    pmr.moveCenter( cr.center() );
                    p->setPen( cg_ours.highlightedText() );
                    p->drawPixmap( pmr.topLeft(), pixmap );
                }

                // Are we checked? (This time without an icon)
                else if ( checkable && mi->isChecked() )
                {
                    // We only have to draw the background if the menu item is inactive -
                    // if it's active the "pressed" background is already drawn
                    //if ( ! active )
                    // qDrawShadePanel( p, cx, y, checkcol, h, cg_ours, true, 1,
                    //     &cg_ours.brush(TQColorGroup::Midlight) );

                    // Draw the checkmark
                    drawPrimitive(PE_MenuItemIndicatorCheck, p, ceData, elementFlags, r, cg, how, opt);
                }

                // Time to draw the menu item label...
                int xm = itemFrame + checkcol + itemHMargin; // X position margin

                int xp = reverse ?  // X position
                         x + tab + rightBorder + itemHMargin + itemFrame - 1 :
                         x + xm;

                int offset = reverse ? -1 : 1; // Shadow offset for etched text

                // Label width (minus the width of the accelerator portion)
                int tw = w - xm - tab - arrowHMargin - itemHMargin * 3 - itemFrame + 1;

                // Set the color for enabled and disabled text
                // (used for both active and inactive menu items)
                p->setPen( enabled ? cg_ours.buttonText() : cg_ours.mid() );

                // This color will be used instead of the above if the menu item
                // is active and disabled at the same time. (etched text)
                TQColor discol = cg_ours.mid();

                // Does the menu item draw it's own label?
                if ( mi->custom() )
                {
                    int m = itemVMargin;
                    // Save the painter state in case the custom
                    // paint method changes it in some way
                    p->save();

                    // Draw etched text if we're inactive and the menu item is disabled
                    if ( etchtext && !enabled && !active )
                    {
                        p->setPen( cg_ours.light() );
                        mi->custom() ->paint( p, cg_ours, active, enabled, xp + offset, y + m + 1, tw, h - 2 * m );
                        p->setPen( discol );
                    }
                    mi->custom() ->paint( p, cg_ours, active, enabled, xp, y + m, tw, h - 2 * m );
                    p->restore();
                }
                else
                {
                    // The menu item doesn't draw it's own label
                    TQString s = mi->text();

                    // Does the menu item have a text label?
                    if ( !s.isNull() )
                    {
                        int t = s.find( '\t' );
                        int m = itemVMargin;
                        int text_flags = AlignVCenter | ShowPrefix | DontClip | SingleLine;
                        text_flags |= reverse ? AlignRight : AlignLeft;

                        // Does the menu item have a tabstop? (for the accelerator text)
                        if ( t >= 0 )
                        {
                            int tabx = reverse ? x + rightBorder + itemHMargin + itemFrame :
                                       x + w - tab - rightBorder - itemHMargin - itemFrame;


                            // Draw the right part of the label (accelerator text)
                            if ( etchtext && !enabled && !active )
                            {
                                // Draw etched text if we're inactive and the menu item is disabled
                                p->setPen( cg_ours.light() );
                                p->drawText( tabx + offset, y + m + 1, tab, h - 2 * m, text_flags, s.mid( t + 1 ) );
                                p->setPen( discol );
                            }
                            p->drawText( tabx, y + m, tab, h - 2 * m, text_flags, s.mid( t + 1 ) );
                            s = s.left( t );
                        }


                        // Draw the left part of the label (or the whole label
                        // if there's no accelerator)
                        if ( etchtext && !enabled && !active )
                        {
                            // Etched text again for inactive disabled menu items...
                            p->setPen( cg_ours.light() );
                            p->drawText( xp + offset, y + m + 1, tw, h - 2 * m, text_flags, s, t );
                            p->setPen( discol );
                        }

                        p->drawText( xp, y + m, tw, h - 2 * m, text_flags, s, t );

                    }

                    // The menu item doesn't have a text label
                    // Check if it has a pixmap instead
                    else if ( mi->pixmap() )
                    {
                        TQPixmap * pixmap = mi->pixmap();

                        // Draw the pixmap
                        if ( pixmap->depth() == 1 )
                            p->setBackgroundMode( TQt::OpaqueMode );

                        int diffw = ( ( w - pixmap->width() ) / 2 )
                                    + ( ( w - pixmap->width() ) % 2 );
                        p->drawPixmap( x + diffw, y + itemFrame, *pixmap );

                        if ( pixmap->depth() == 1 )
                            p->setBackgroundMode( TQt::TransparentMode );
                    }
                }

                // Does the menu item have a submenu?
                if ( mi->popup() )
                {
                    PrimitiveElement arrow = reverse ? PE_ArrowLeft : PE_ArrowRight;
                    int dim = 10 -  itemFrame; //We're not very useful to inherit off, so just hardcode..
                    TQRect vr = visualRect( TQRect( x + w - arrowHMargin - itemFrame - dim,
                                                  y + h / 2 - dim / 2, dim, dim ), r );

                    // Draw an arrow at the far end of the menu item
                    if ( active )
                    {
                        if ( enabled )
                            discol = cg_ours.buttonText();

                        TQColorGroup g2( discol, cg_ours.highlight(), white, white,
                                        enabled ? white : discol, discol, white );

                        drawPrimitive( arrow, p, ceData, elementFlags, vr, g2, Style_Enabled | Style_Down );
                    }
                    else
                        drawPrimitive( arrow, p, ceData, elementFlags, vr, cg_ours,
                                       enabled ? Style_Enabled : Style_Default );
                }
                handled = true;
                break;
            }
        case CE_ProgressBarGroove:
            {
                TQBrush bg;
                const TQColorGroup * cg2 = colorGroup( cg, ProgressBg );
                qDrawWinPanel( p, r, *cg2, true );
                bg.setColor( cg2->color( TQColorGroup::Background ) );
                if ( isPixmap( ProgressBg ) )
                    bg.setPixmap( *uncached( ProgressBg ) );
                p->fillRect( x + 2, y + 2, w - 4, h - 4, bg );

                handled = true;
                break;
            }
        case CE_ProgressBarContents:
            {
                const TQProgressBar* pb = (const TQProgressBar*)widget;
                TQRect cr = subRect(SR_ProgressBarContents, ceData, elementFlags, widget);
                double progress = pb->progress();
                bool reverse = TQApplication::reverseLayout();
                int steps = pb->totalSteps();
                
                int pstep = 0;

                if (!cr.isValid())
                        return;

                // Draw progress bar
                if (progress > 0 || steps == 0)
                {
                        double pg = (steps == 0) ? 0.1 : progress / steps;
                        int width = TQMIN(cr.width(), (int)(pg * cr.width()));
                        if (steps == 0)
                        { //Busy indicator

                                if (width < 1) width = 1; //A busy indicator with width 0 is kind of useless

                                int remWidth = cr.width() - width; //Never disappear completely
                                if (remWidth <= 0) remWidth = 1; //Do something non-crashy when too small...                                       

                                pstep =  int(progress) % ( 2 *  remWidth ); 

                                if ( pstep > remWidth )
                                {
                                        //Bounce about.. We're remWidth + some delta, we want to be remWidth - delta...                                           
                                        // - ( (remWidth + some delta) - 2* remWidth )  = - (some deleta - remWidth) = remWidth - some delta..
                                        pstep = - (pstep - 2 * remWidth );                                                                                      
                                }
                        }
                                                                           
                        if ( !reverse )
                                drawBaseButton( p, x + pstep, y, width, h, *colorGroup( cg, ProgressBar ), false, false, ProgressBar );
                        else
                        {
                                //TODO:Optimize
                                TQPixmap buf( width, h );
                                TQPainter p2( &buf );
                                drawBaseButton( &p2, 0, 0, width, h, *colorGroup( cg, ProgressBar ), false, false, ProgressBar );
                                p2.end();
                                TQPixmap mirroredPix = TQPixmap( TQImage(buf.convertToImage()).mirror( true, false ) );
                                bitBlt( p->device(), x + w - width - pstep, y, &mirroredPix );
                        }
                }

                handled = true;
                break;
            }
        default:
            handled = false;
    };

    if ( !handled )
        KThemeBase::drawControl( element,
                                 p, ceData, elementFlags, r, cg, how, opt, widget );
}


void KThemeStyle::drawControlMask( ControlElement element,
                                   TQPainter *p,
                                   const TQStyleControlElementData &ceData,
                                   ControlElementFlags elementFlags,
                                   const TQRect &r,
                                   const TQStyleOption& opt,
                                   const TQWidget *widget ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );

    switch ( element )
    {
        case CE_PushButton:
            {
                //Is this correct?
                drawBaseMask( p, x, y, w, h, roundButton() );
                handled = true;
                break;
            }
        default:
            handled = false;
    };

    if ( !handled )
        KThemeBase::drawControlMask( element,
                                     p, ceData, elementFlags, r, opt, widget );

}


void KThemeStyle::drawTDEStylePrimitive( TDEStylePrimitive kpe,
                                       TQPainter* p,
                                       const TQStyleControlElementData &ceData,
                                       ControlElementFlags elementFlags,
                                       const TQRect &r,
                                       const TQColorGroup &cg,
                                       SFlags flags,
                                       const TQStyleOption& opt,
                                       const TQWidget* widget ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    switch ( kpe )
    {
        case KPE_SliderGroove:
            {
                if ( !roundSlider() )
                {
                    const TQSlider * slider = ( const TQSlider* ) widget;
                    bool horizontal = slider->orientation() == TQt::Horizontal;
                    if ( horizontal )
                    {
                        drawBaseButton( p, x, y, w, h, *colorGroup( cg, SliderGroove ), true,
                                        false, SliderGroove );
                    }
                    else
                    {
                        drawBaseButton( p, x, y, w, h, *colorGroup( cg, RotSliderGroove ), true,
                                        false, RotSliderGroove );
                    }
                }
                else
                {
                    //This code is from HighColorDefault..
                    const TQSlider* slider = ( const TQSlider* ) widget;
                    bool horizontal = slider->orientation() == TQt::Horizontal;
                    int gcenter = ( horizontal ? r.height() : r.width() ) / 2;

                    TQRect gr;
                    if ( horizontal )
                        gr = TQRect( r.x(), r.y() + gcenter - 3, r.width(), 7 );
                    else
                        gr = TQRect( r.x() + gcenter - 3, r.y(), 7, r.height() );

                    int x, y, w, h;
                    gr.rect( &x, &y, &w, &h );
                    int x2 = x + w - 1;
                    int y2 = y + h - 1;

                    // Draw the slider groove.
                    p->setPen( cg.dark() );
                    p->drawLine( x + 2, y, x2 - 2, y );
                    p->drawLine( x, y + 2, x, y2 - 2 );
                    p->fillRect( x + 2, y + 2, w - 4, h - 4,
                                 slider->isEnabled() ? cg.dark() : cg.mid() );
                    p->setPen( cg.shadow() );
                    p->drawRect( x + 1, y + 1, w - 2, h - 2 );
                    p->setPen( cg.light() );
                    p->drawPoint( x + 1, y2 - 1 );
                    p->drawPoint( x2 - 1, y2 - 1 );
                    p->drawLine( x2, y + 2, x2, y2 - 2 );
                    p->drawLine( x + 2, y2, x2 - 2, y2 );
                }
                handled = true;
                break;
            }
        case KPE_SliderHandle:
            {
                if ( isPixmap( Slider ) )
                {
                    const TQSlider * slider = ( const TQSlider* ) widget;
                    bool horizontal = slider->orientation() == TQt::Horizontal;
                    if ( horizontal )
                    {
                        bitBlt( p->device(), x, y + ( h - uncached( Slider ) ->height() ) / 2,
                                uncached( Slider ) );
                    }
                    else
                    {
                        if ( !vsliderCache )
                        {
                            TQWMatrix r270;
                            r270.rotate( 270 );
                            vsliderCache = new TQPixmap( uncached( Slider ) ->xForm( r270 ) );
                            if ( uncached( Slider ) ->mask() )
                                vsliderCache->setMask( uncached( Slider ) ->mask() ->xForm( r270 ) );
                        }
                        bitBlt( p->device(), x + ( w - vsliderCache->width() ) / 2, y,
                                vsliderCache );
                    }
                }
                else
                {
                    //This code again from HighColor..
                    //...except sans the gradient..
                    const TQSlider* slider = ( const TQSlider* ) widget;
                    bool horizontal = slider->orientation() == TQt::Horizontal;
                    int x, y, w, h;
                    r.rect( &x, &y, &w, &h );
                    int x2 = x + w - 1;
                    int y2 = y + h - 1;

                    p->setPen( cg.mid() );
                    p->drawLine( x + 1, y, x2 - 1, y );
                    p->drawLine( x, y + 1, x, y2 - 1 );
                    p->setPen( cg.shadow() );
                    p->drawLine( x + 1, y2, x2 - 1, y2 );
                    p->drawLine( x2, y + 1, x2, y2 - 1 );

                    p->setPen( cg.light() );
                    p->drawLine( x + 1, y + 1, x2 - 1, y + 1 );
                    p->drawLine( x + 1, y + 1, x + 1, y2 - 1 );
                    p->setPen( cg.dark() );
                    p->drawLine( x + 2, y2 - 1, x2 - 1, y2 - 1 );
                    p->drawLine( x2 - 1, y + 2, x2 - 1, y2 - 1 );
                    p->setPen( cg.midlight() );
                    p->drawLine( x + 2, y + 2, x2 - 2, y + 2 );
                    p->drawLine( x + 2, y + 2, x + 2, y2 - 2 );
                    p->setPen( cg.mid() );
                    p->drawLine( x + 3, y2 - 2, x2 - 2, y2 - 2 );
                    p->drawLine( x2 - 2, y + 3, x2 - 2, y2 - 2 );
                    p->fillRect( TQRect( x + 3, y + 3, w - 6, h - 6 ),
                                 cg.button() );

                    // Paint riffles
                    if ( horizontal )
                    {
                        p->setPen( cg.light() );
                        p->drawLine( x + 5, y + 4, x + 5, y2 - 4 );
                        p->drawLine( x + 8, y + 4, x + 8, y2 - 4 );
                        p->drawLine( x + 11, y + 4, x + 11, y2 - 4 );
                        p->setPen( slider->isEnabled() ? cg.shadow() : cg.mid() );
                        p->drawLine( x + 6, y + 4, x + 6, y2 - 4 );
                        p->drawLine( x + 9, y + 4, x + 9, y2 - 4 );
                        p->drawLine( x + 12, y + 4, x + 12, y2 - 4 );
                    }
                    else
                    {
                        p->setPen( cg.light() );
                        p->drawLine( x + 4, y + 5, x2 - 4, y + 5 );
                        p->drawLine( x + 4, y + 8, x2 - 4, y + 8 );
                        p->drawLine( x + 4, y + 11, x2 - 4, y + 11 );
                        p->setPen( slider->isEnabled() ? cg.shadow() : cg.mid() );
                        p->drawLine( x + 4, y + 6, x2 - 4, y + 6 );
                        p->drawLine( x + 4, y + 9, x2 - 4, y + 9 );
                        p->drawLine( x + 4, y + 12, x2 - 4, y + 12 );
                    }
                }
                handled = true;
                break;
            }
            //case KPE_DockWindowHandle:
        case KPE_ToolBarHandle:
        case KPE_GeneralHandle:
            {
                if ( w > h )
                    drawBaseButton( p, x, y, w, h, *colorGroup( cg, HBarHandle ), false, false,
                                    HBarHandle );
                else
                    drawBaseButton( p, x, y, w, h, *colorGroup( cg, VBarHandle ), false, false,
                                    VBarHandle );

                handled = true;
                break;
            }
        default:
            handled = false;

    }

    if ( !handled )
    {
        KThemeBase::drawTDEStylePrimitive( kpe, p, ceData, elementFlags,
                                         r, cg, flags, opt, widget );
    }

}




void KThemeStyle::drawComplexControl ( ComplexControl control, TQPainter * p, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags,
                                       const TQRect & r, const TQColorGroup & g, SFlags how ,
                                       SCFlags controls, SCFlags active,
                                       const TQStyleOption & opt, const TQWidget * widget ) const
{
    bool handled = false;
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    bool down = how & Style_Down;
    bool on = how & Style_On;

    // bool enabled = ( how & Style_Enabled );

    switch ( control )
    {
        case CC_ToolButton:
            {
                const TQToolButton * toolbutton = ( const TQToolButton * ) widget;
                TQRect button, menu;
                button = querySubControlMetrics( control, ceData, elementFlags, SC_ToolButton, opt, widget );
                menu = querySubControlMetrics( control, ceData, elementFlags, SC_ToolButtonMenu, opt, widget );


                if ( controls & SC_ToolButton )
                {
                    WidgetType widget = ( down || on ) ? ToolButtonDown : ToolButton;

                    drawBaseButton( p, button.x(), button.y(), button.width(), button.height(), *colorGroup( g, widget ), down || on, false,
                                    widget );

                    // int m = decoWidth( widget );
                }

                if ( controls & SC_ToolButtonMenu )
                {
                    drawPrimitive( PE_ArrowDown, p, ceData, elementFlags, menu, g, how );
                    /*                if ( enabled )
                         kDrawWindowsArrow(p, ceData, elementFlags, this, PE_ArrowDown, false, menu.x(), menu.y(), menu.width(), menu.height(),
                                                     g, true );
                                    else
                         kDrawWindowsArrow(p, ceData, elementFlags, this, PE_ArrowDown, false, menu.x(), menu.y(), menu.width(), menu.height(),
                                                     g, false );*/
                }

                if ( toolbutton->hasFocus() && !toolbutton->focusProxy() )
                {
                    TQRect fr = toolbutton->rect();
                    fr.addCoords( 3, 3, -3, -3 );
                    drawPrimitive( PE_FocusRect, p, ceData, elementFlags, fr, g );
                }

                handled = true;
                break;
            }

        case CC_ComboBox:
            {
                if ( controls & SC_ComboBoxFrame )
                {
                    //TODO: Anyway of detecting when the popup is there -- would look nicer if sunken then too..
                    bool sunken = ( active == SC_ComboBoxArrow );
                    //No frame, edit box and button for now?
                    WidgetType widget = sunken ? ComboBoxDown : ComboBox;
                    drawBaseButton( p, x, y, w, h, *colorGroup( g, widget ), sunken,
                                    roundComboBox(), widget );

                    controls ^= SC_ComboBoxFrame;
                }

                if ( controls & SC_ComboBoxArrow )
                {
                    bool sunken = ( active == SC_ComboBoxArrow );
                    TQRect ar = TQStyle::visualRect(
                                   querySubControlMetrics( CC_ComboBox, ceData, elementFlags, SC_ComboBoxArrow, TQStyleOption::Default, widget ),
                                   ceData, elementFlags );
                    ar.rect( &x, &y, &w, &h );
                    WidgetType widget = sunken ? ComboBoxDown : ComboBox;

                    if ( !sunken && isPixmap( ComboDeco ) )
                        bitBlt( p->device(),
                                x + ( w - uncached( ComboDeco ) ->width() - decoWidth( ComboBox ) / 2 ),
                                y + ( h - uncached( ComboDeco ) ->height() ) / 2,
                                uncached( ComboDeco ) );
                    else if ( sunken && isPixmap( ComboDecoDown ) )
                        bitBlt( p->device(),
                                x + ( w - uncached( ComboDecoDown ) ->width() - decoWidth( ComboBoxDown ) ) / 2,
                                y + ( h - uncached( ComboDecoDown ) ->height() ) / 2,
                                uncached( ComboDecoDown ) );
                    else
                    {

                        mtfstyle->drawPrimitive( PE_ArrowDown, p, ceData, elementFlags, TQRect( x, y, w, h ), *colorGroup( g, widget ), sunken ? ( how | Style_Sunken ) : how, opt );
                        qDrawShadeRect( p, x, y, w, h, *colorGroup( g, widget ) ); //w-14, y+7+(h-15), 10, 3,
                    }
                    controls ^= SC_ComboBoxArrow;
                }
                break;
            }
        case CC_ScrollBar:
            {
                const TQScrollBar *sb = ( const TQScrollBar* ) widget;
                bool maxedOut = ( sb->minValue() == sb->maxValue() );
                bool horizontal = ( sb->orientation() == TQt::Horizontal );
                SFlags sflags = ( ( horizontal ? Style_Horizontal : Style_Default ) |
                                  ( maxedOut ? Style_Default : Style_Enabled ) );

                //Here, we don't do add page, subpage, etc.,
                TQRect addline, subline, subline2, groove, slider;
                subline = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarSubLine, opt, widget );
                addline = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarAddLine, opt, widget );
                groove = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarGroove, opt, widget );

                slider = querySubControlMetrics( control, ceData, elementFlags, SC_ScrollBarSlider, opt, widget );
                subline2 = addline;

                TQPixmap buf( sb->width(), sb->height() );
                TQPainter p2( &buf );

                if ( groove.isValid() )
                {
                    p2.fillRect( groove, TQColor( 255, 0, 0 ) );
                    drawPrimitive( PE_ScrollBarSubPage, &p2, ceData, elementFlags, groove, g,
                                   sflags | ( ( active == SC_ScrollBarSubPage ) ?
                                              Style_Down : Style_Default ) );
                }


                // Draw the up/left button set
                if ( subline.isValid() )
                {
                    drawPrimitive( PE_ScrollBarSubLine, &p2, ceData, elementFlags, subline, g,
                                   sflags | ( active == SC_ScrollBarSubLine ?
                                              Style_Down : Style_Default ) );
                }

                if ( addline.isValid() )
                    drawPrimitive( PE_ScrollBarAddLine, &p2, ceData, elementFlags, addline, g,
                                   sflags | ( ( active == SC_ScrollBarAddLine ) ?
                                              Style_Down : Style_Default ) );

                if ( slider.isValid() )
                { //(controls & SC_ScrollBarSlider) &&
                    drawPrimitive( PE_ScrollBarSlider, &p2, ceData, elementFlags, slider, g,
                                   sflags | ( ( active == SC_ScrollBarSlider ) ?
                                              Style_Down : Style_Default ) );
                    // Draw focus rect
                    if ( sb->hasFocus() )
                    {
                        TQRect fr( slider.x() + 2, slider.y() + 2,
                                  slider.width() - 5, slider.height() - 5 );
                        drawPrimitive( PE_FocusRect, &p2, ceData, elementFlags, fr, g, Style_Default );
                    }
                    p2.end();
                    bitBlt( p->device(), x, y, &buf );
                    handled = true;

                }
                break;
            }
        default:
            handled = false;
    }

    if ( !handled )
    {
        KThemeBase::drawComplexControl ( control, p, ceData, elementFlags,
                                         r, g, how ,
                                         controls, active,
                                         opt, widget );
    }

}


void KThemeStyle::drawBaseMask( TQPainter *p, int x, int y, int w, int h,
                                bool round ) const
{
    // round edge fills
    static const TQCOORD btm_left_fill[] =
        {
            0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 0, 1, 1, 1, 2, 1, 3, 1, 4, 1,
            1, 2, 2, 2, 3, 2, 4, 2, 2, 3, 3, 3, 4, 3, 3, 4, 4, 4
        };

    static const TQCOORD btm_right_fill[] =
        {
            0, 0, 1, 0, 2, 0, 3, 0, 4, 0, 0, 1, 1, 1, 2, 1, 3, 1, 4,
            1, 0, 2, 1, 2, 2, 2, 3, 2, 0, 3, 1, 3, 2, 3, 0, 4, 1, 4
        };

    static const TQCOORD top_left_fill[] =
        {
            3, 0, 4, 0, 2, 1, 3, 1, 4, 1, 1, 2, 2, 2, 3, 2, 4, 2, 0, 3,
            1, 3, 2, 3, 3, 3, 4, 3, 0, 4, 1, 4, 2, 4, 3, 4, 4, 4
        };

    static const TQCOORD top_right_fill[] =
        {
            0, 0, 1, 0, 0, 1, 1, 1, 2, 1, 0, 2, 1, 2, 2, 2, 3, 2, 0,
            3, 1, 3, 2, 3, 3, 3, 4, 3, 0, 4, 1, 4, 2, 4, 3, 4, 4, 4
        };

    TQBrush fillBrush( color1, SolidPattern );
    p->setPen( color1 );
    if ( round && w > 19 && h > 19 )
    {
        int x2 = x + w - 1;
        int y2 = y + h - 1;
        TQPointArray a( TQCOORDARRLEN( top_left_fill ), top_left_fill );
        a.translate( 1, 1 );
        p->drawPoints( a );
        a.setPoints( TQCOORDARRLEN( btm_left_fill ), btm_left_fill );
        a.translate( 1, h - 6 );
        p->drawPoints( a );
        a.setPoints( TQCOORDARRLEN( top_right_fill ), top_right_fill );
        a.translate( w - 6, 1 );
        p->drawPoints( a );
        a.setPoints( TQCOORDARRLEN( btm_right_fill ), btm_right_fill );
        a.translate( w - 6, h - 6 );
        p->drawPoints( a );

        p->fillRect( x + 6, y, w - 12, h, fillBrush );
        p->fillRect( x, y + 6, x + 6, h - 12, fillBrush );
        p->fillRect( x2 - 6, y + 6, x2, h - 12, fillBrush );
        p->drawLine( x + 6, y, x2 - 6, y );
        p->drawLine( x + 6, y2, x2 - 6, y2 );
        p->drawLine( x, y + 6, x, y2 - 6 );
        p->drawLine( x2, y + 6, x2, y2 - 6 );

    }
    else
        p->fillRect( x, y, w, h, fillBrush );
}

int KThemeStyle::styleHint( StyleHint sh, const TQStyleControlElementData &ceData, ControlElementFlags elementFlags, const TQStyleOption &opt, TQStyleHintReturn *shr, const TQWidget *w ) const
{
    switch ( sh )
    {
        case SH_EtchDisabledText:
        case SH_Slider_SnapToValue:
        case SH_PrintDialog_RightAlignButtons:
        case SH_FontDialog_SelectAssociatedText:
        case SH_PopupMenu_AllowActiveAndDisabled:
        case SH_MenuBar_AltKeyNavigation:
        case SH_MenuBar_MouseTracking:
        case SH_PopupMenu_MouseTracking:
        case SH_ComboBox_ListMouseTracking:
            return 1;

        case SH_GUIStyle:
            return WindowsStyle;

	case SH_ScrollBar_BackgroundMode:
	    return NoBackground;

    case SH_MenuIndicatorColumnWidth:
        {
            int checkcol = opt.maxIconWidth();
            bool checkable = (elementFlags & CEF_IsCheckable);

            if ( checkable )
                    checkcol = TQMAX( checkcol, 20 );

            return checkcol;
            break;
        }

        default:
            return KThemeBase::styleHint( sh, ceData, elementFlags, opt, shr, w );
    };
}



/* This is where we draw the borders and highlights. The new round button
 * code is a pain in the arse. We don't want to be calculating arcs so
 * use a whole lotta QPointArray's ;-) The code is made a lot more complex
 * because you can have variable width border and highlights...
 * I may want to cache this if round buttons are used, but am concerned
 * about excessive cache misses. This is a memory/speed tradeoff that I
 * have to test.
 */
void KThemeStyle::drawShade( TQPainter *p, int x, int y, int w, int h,
                             const TQColorGroup &g, bool sunken, bool rounded,
                             int hWidth, int bWidth, ShadeStyle style ) const
{
    int i, sc, bc, x2, y2;
    TQPen highPen, lowPen;

    if ( style == Motif )
    {
        highPen.setColor( sunken ? g.dark() : g.light() );
        lowPen.setColor( sunken ? g.light() : g.dark() );
    }
    else
    {
        highPen.setColor( sunken ? g.shadow() : g.light() );
        lowPen.setColor( sunken ? g.light() : g.shadow() );
    }

    // Advanced round buttons
    if ( rounded && w > 19 && h > 19 )
    {
        x2 = x + w - 1, y2 = y + h - 1;
        TQPointArray bPntArray, hPntArray, lPntArray;
        TQPointArray bLineArray, hLineArray, lLineArray;
        // borders
        for ( i = 0, bc = 0; i < bWidth; ++i )
        {
            bPntArray.putPoints( bc, 24, x + 4, y + 1, x + 5, y + 1, x + 3, y + 2, x + 2, y + 3,
                                 x + 1, y + 4, x + 1, y + 5, x + 1, y2 - 5, x + 1, y2 - 4, x + 2, y2 - 3,
                                 x2 - 5, y + 1, x2 - 4, y + 1, x2 - 3, y + 2, x2 - 5, y2 - 1,
                                 x2 - 4, y2 - 1, x2 - 3, y2 - 2, x2 - 2, y2 - 3, x2 - 1, y2 - 5,
                                 x2 - 1, y2 - 4, x + 3, y2 - 2, x + 4, y2 - 1, x + 5, y2 - 1,
                                 x2 - 2, y + 3, x2 - 1, y + 4, x2 - 1, y + 5 );
            bc += 24;
            // ellispe edges don't match exactly, so fill in blanks
            if ( i < bWidth - 1 || hWidth != 0 )
            {
                bPntArray.putPoints( bc, 20, x + 6, y + 1, x + 4, y + 2, x + 3, y + 3,
                                     x + 2, y + 4, x + 1, y + 6, x2 - 6, y + 1, x2 - 4, y + 2,
                                     x2 - 3, y + 3, x + 2, y2 - 4, x + 1, y2 - 6, x2 - 6, y2 - 1,
                                     x2 - 4, y2 - 2, x2 - 3, y2 - 3, x2 - 2, y2 - 4, x2 - 1, y2 - 6,
                                     x + 6, y2 - 1, x + 4, y2 - 2, x + 3, y2 - 3, x2 - 1, y + 6,
                                     x2 - 2, y + 4 );
                bc += 20;
            }
            bLineArray.putPoints( i * 8, 8, x + 6, y, x2 - 6, y, x, y + 6, x, y2 - 6,
                                  x + 6, y2, x2 - 6, y2, x2, y + 6, x2, y2 - 6 );
            ++x, ++y;
            --x2, --y2;
        }
        // highlights
        for ( i = 0, sc = 0; i < hWidth; ++i )
        {
            hPntArray.putPoints( sc, 12, x + 4, y + 1, x + 5, y + 1,   // top left
                                 x + 3, y + 2, x + 2, y + 3, x + 1, y + 4, x + 1, y + 5,
                                 x + 1, y2 - 5, x + 1, y2 - 4, x + 2, y2 - 3,   // half corners
                                 x2 - 5, y + 1, x2 - 4, y + 1, x2 - 3, y + 2 );
            lPntArray.putPoints( sc, 12, x2 - 5, y2 - 1, x2 - 4, y2 - 1,   // btm right
                                 x2 - 3, y2 - 2, x2 - 2, y2 - 3, x2 - 1, y2 - 5, x2 - 1, y2 - 4,
                                 x + 3, y2 - 2, x + 4, y2 - 1, x + 5, y2 - 1,   //half corners
                                 x2 - 2, y + 3, x2 - 1, y + 4, x2 - 1, y + 5 );
            sc += 12;
            if ( i < hWidth - 1 )
            {
                hPntArray.putPoints( sc, 10, x + 6, y + 1, x + 4, y + 2,   // top left
                                     x + 3, y + 3, x + 2, y + 4, x + 1, y + 6,
                                     x2 - 6, y + 1, x2 - 4, y + 2,   // half corners
                                     x2 - 3, y + 3, x + 2, y2 - 4, x + 1, y2 - 6 );
                lPntArray.putPoints( sc, 10, x2 - 6, y2 - 1, x2 - 4, y2 - 2,   // btm right
                                     x2 - 3, y2 - 3, x2 - 2, y2 - 4, x2 - 1, y2 - 6,
                                     x + 6, y2 - 1, x + 4, y2 - 2,   // half corners
                                     x + 3, y2 - 3, x2 - 1, y + 6, x2 - 2, y + 4 );
                sc += 10;
            }
            hLineArray.putPoints( i * 4, 4, x + 6, y, x2 - 6, y, x, y + 6, x, y2 - 6 );
            lLineArray.putPoints( i * 4, 4, x + 6, y2, x2 - 6, y2, x2, y + 6, x2, y2 - 6 );
            ++x, ++y;
            --x2, --y2;
        }
        p->setPen( TQt::black );
        p->drawPoints( bPntArray );
        p->drawLineSegments( bLineArray );
        p->setPen( highPen );
        p->drawPoints( hPntArray );
        p->drawLineSegments( hLineArray );
        p->setPen( lowPen );
        p->drawPoints( lPntArray );
        p->drawLineSegments( lLineArray );
    }
    // Rectangular buttons
    else
    {
        TQPointArray highShade( hWidth * 4 );
        TQPointArray lowShade( hWidth * 4 );

        p->setPen( g.shadow() );
        for ( i = 0; i < bWidth && w > 2 && h > 2; ++i, ++x, ++y, w -= 2, h -= 2 )
            p->drawRect( x, y , w, h );

        if ( !hWidth )
            return ;

        x2 = x + w - 1, y2 = y + h - 1;
        for ( i = 0; i < hWidth; ++i, ++x, ++y, --x2, --y2 )
        {
            highShade.putPoints( i * 4, 4, x, y, x2, y, x, y, x, y2 );
            lowShade.putPoints( i * 4, 4, x, y2, x2, y2, x2, y, x2, y2 );
        }
        if ( style == Windows && hWidth > 1 )
        {
            p->setPen( highPen );
            p->drawLineSegments( highShade, 0, 2 );
            p->setPen( lowPen );
            p->drawLineSegments( lowShade, 0, 2 );

            p->setPen( ( sunken ) ? g.dark() : g.mid() );
            p->drawLineSegments( highShade, 4 );
            p->setPen( ( sunken ) ? g.mid() : g.dark() );
            p->drawLineSegments( lowShade, 4 );
        }
        else
        {
            p->setPen( ( sunken ) ? g.dark() : g.light() );
            p->drawLineSegments( highShade );
            p->setPen( ( sunken ) ? g.light() : g.dark() );
            p->drawLineSegments( lowShade );
        }
    }
}




int KThemeStyle::popupMenuItemHeight( bool /*checkable*/, TQMenuItem *mi,
                                      const TQFontMetrics &fm )
{
    int h2, h = 0;
    int offset = TQMAX( decoWidth( MenuItemDown ), decoWidth( MenuItem ) ) + 4;

    if ( mi->isSeparator() )
        return ( 2 );
    if ( mi->isChecked() )
        h = isPixmap( CheckMark ) ? uncached( CheckMark ) ->height() + offset :
            offset + 16;
    if ( mi->pixmap() )
    {
        h2 = mi->pixmap() ->height() + offset;
        h = h2 > h ? h2 : h;
    }
    if ( mi->iconSet() )
    {
        h2 = mi->iconSet() ->
             pixmap( TQIconSet::Small, TQIconSet::Normal ).height() + offset;
        h = h2 > h ? h2 : h;
    }
    h2 = fm.height() + offset;
    h = h2 > h ? h2 : h;
    return ( h );
}

#include "kthemestyle.moc"
