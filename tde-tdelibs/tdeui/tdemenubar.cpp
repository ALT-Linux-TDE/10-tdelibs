/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998, 1999, 2000  Sven Radej (radej@kde.org)
    Copyright (C) 1997, 1998, 1999, 2000 Matthias Ettrich (ettrich@kde.org)
    Copyright (C) 1999, 2000 Daniel "Mosfet" Duley (mosfet@kde.org)

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


#ifndef INCLUDE_MENUITEM_DEF
#define INCLUDE_MENUITEM_DEF
#endif

#include "config.h"
#include <tqevent.h>
#include <tqobjectlist.h>
#include <tqaccel.h>
#include <tqpainter.h>
#include <tqstyle.h>
#include <tqtimer.h>

#include <tdeconfig.h>
#include <tdeglobalsettings.h>
#include <tdemenubar.h>
#include <tdeapplication.h>
#include <tdeglobal.h>
#include <kdebug.h>
#include <kmanagerselection.h>

#ifdef TQ_WS_X11
#include <twin.h> 
#include <twinmodule.h> 
#include <qxembed.h> 

#include <X11/Xlib.h> 
#include <X11/Xutil.h> 
#include <X11/Xatom.h> 

#ifndef None
#define None 0L
#endif
#endif

/*

 Toplevel menubar (not for the fallback size handling done by itself):
 - should not alter position or set strut
 - every toplevel must have at most one matching topmenu
 - embedder won't allow shrinking below a certain size
 - must have WM_TRANSIENT_FOR pointing the its mainwindow
     - the exception is desktop's menubar, which can be transient for root window
       because of using root window as the desktop window
 - Fitts' Law

*/

class KMenuBar::KMenuBarPrivate
{
public:
    KMenuBarPrivate()
	:   forcedTopLevel( false ),
	    topLevel( false ),
	    wasTopLevel( false ),
#ifdef TQ_WS_X11
	    selection( NULL ),
#endif
            min_size( 0, 0 )
	{
	}
    ~KMenuBarPrivate()
        {
#ifdef TQ_WS_X11
        delete selection;
#endif
        }
    bool forcedTopLevel;
    bool topLevel;
    bool wasTopLevel; // when TLW is fullscreen, remember state
    int frameStyle; // only valid in toplevel mode
    int lineWidth;  // dtto
    int margin;     // dtto
    bool fallback_mode; // dtto
#ifdef TQ_WS_X11
    TDESelectionWatcher* selection;
#endif
    TQTimer selection_timer;
    TQSize min_size;
    static Atom makeSelectionAtom();
};

#ifdef TQ_WS_X11
static Atom selection_atom = None;
static Atom msg_type_atom = None;

static
void initAtoms()
{
    char nm[ 100 ];
    sprintf( nm, "_KDE_TOPMENU_OWNER_S%d", DefaultScreen( tqt_xdisplay()));
    char nm2[] = "_KDE_TOPMENU_MINSIZE";
    char* names[ 2 ] = { nm, nm2 };
    Atom atoms[ 2 ];
    XInternAtoms( tqt_xdisplay(), names, 2, False, atoms );
    selection_atom = atoms[ 0 ];
    msg_type_atom = atoms[ 1 ];
}
#endif

Atom KMenuBar::KMenuBarPrivate::makeSelectionAtom()
{
#ifdef TQ_WS_X11
    if( selection_atom == None )
	initAtoms();
    return selection_atom;
#else
    return 0;
#endif
}

KMenuBar::KMenuBar(TQWidget *parent, const char *name)
  : TQMenuBar(parent, name)
{
#ifdef TQ_WS_X11
    QXEmbed::initialize();
#endif
    d = new KMenuBarPrivate;
    connect( &d->selection_timer, TQ_SIGNAL( timeout()),
        this, TQ_SLOT( selectionTimeout()));

    connect( tqApp->desktop(), TQ_SIGNAL( resized( int )), TQ_SLOT( updateFallbackSize()));

    if ( kapp )
        // toolbarAppearanceChanged(int) is sent when changing macstyle
        connect( kapp, TQ_SIGNAL(toolbarAppearanceChanged(int)),
            this, TQ_SLOT(slotReadConfig()));

    slotReadConfig();
}

KMenuBar::~KMenuBar()
{
  delete d;
}

void KMenuBar::setTopLevelMenu(bool top_level)
{
  d->forcedTopLevel = top_level;
  setTopLevelMenuInternal( top_level );
}

void KMenuBar::setTopLevelMenuInternal(bool top_level)
{
  if (d->forcedTopLevel)
    top_level = true;

  d->wasTopLevel = top_level;
  if( parentWidget()
      && parentWidget()->topLevelWidget()->isFullScreen())
    top_level = false;

  if ( isTopLevelMenu() == top_level )
    return;
  d->topLevel = top_level;
  if ( isTopLevelMenu() )
  {
#ifdef TQ_WS_X11
      d->selection = new TDESelectionWatcher( KMenuBarPrivate::makeSelectionAtom(),
          DefaultScreen( tqt_xdisplay()));
      connect( d->selection, TQ_SIGNAL( newOwner( Window )),
          this, TQ_SLOT( updateFallbackSize()));
      connect( d->selection, TQ_SIGNAL( lostOwner()),
          this, TQ_SLOT( updateFallbackSize()));
#endif
      d->frameStyle = frameStyle();
      d->lineWidth = lineWidth();
      d->margin = margin();
      d->fallback_mode = false;
      bool wasShown = !isHidden();
      reparent( parentWidget(), (WFlags)(WType_TopLevel | WStyle_Tool | WStyle_Customize | WStyle_NoBorder), TQPoint(0,0), false );
#ifdef TQ_WS_X11
      KWin::setType( winId(), NET::TopMenu );
      if( parentWidget())
          XSetTransientForHint( tqt_xdisplay(), winId(), parentWidget()->topLevelWidget()->winId());
#endif
      TQMenuBar::setFrameStyle( NoFrame );
      TQMenuBar::setLineWidth( 0 );
      TQMenuBar::setMargin( 0 );
      updateFallbackSize();
      d->min_size = TQSize( 0, 0 );
      if( parentWidget() && !parentWidget()->isTopLevel())
          setShown( parentWidget()->isVisible());
      else if ( wasShown )
          show();
  } else
  {
#ifdef TQ_WS_X11
      delete d->selection;
      d->selection = NULL;
#endif
      setBackgroundMode( PaletteButton );
      setFrameStyle( d->frameStyle );
      setLineWidth( d->lineWidth );
      setMargin( d->margin );
      setMinimumSize( 0, 0 );
      setMaximumSize( TQWIDGETSIZE_MAX, TQWIDGETSIZE_MAX );
      updateMenuBarSize();
      if ( parentWidget() )
          reparent( parentWidget(), TQPoint(0,0), !isHidden());
  }
}

bool KMenuBar::isTopLevelMenu() const
{
  return d->topLevel;
}

// KDE4 remove
void KMenuBar::show()
{
    TQMenuBar::show();
}

void KMenuBar::slotReadConfig()
{
  TDEConfig *config = TDEGlobal::config();
  TDEConfigGroupSaver saver( config, "KDE" );
  setTopLevelMenuInternal( config->readBoolEntry( "macStyle", false ) );
}

bool KMenuBar::eventFilter(TQObject *obj, TQEvent *ev)
{
    if ( d->topLevel )
    {
	if ( parentWidget() && obj == parentWidget()->topLevelWidget()  )
        {
	    if( ev->type() == TQEvent::Resize )
		return false; // ignore resizing of parent, TQMenuBar would try to adjust size
	    if ( ev->type() == TQEvent::Accel || ev->type() == TQEvent::AccelAvailable )
            {
		if ( TQApplication::sendEvent( topLevelWidget(), ev ) )
		    return true;
	    }
            if(ev->type() == TQEvent::ShowFullScreen )
                // will update the state properly
                setTopLevelMenuInternal( d->topLevel );
        }
        if( parentWidget() && obj == parentWidget() && ev->type() == TQEvent::Reparent )
            {
#ifdef TQ_WS_X11
            XSetTransientForHint( tqt_xdisplay(), winId(), parentWidget()->topLevelWidget()->winId());
#else
            //TODO: WIN32?
#endif
            setShown( parentWidget()->isTopLevel() || parentWidget()->isVisible());
            }
        if( parentWidget() && !parentWidget()->isTopLevel() && obj == parentWidget())
        { // if the parent is not toplevel, KMenuBar needs to match its visibility status
            if( ev->type() == TQEvent::Show )
                {
#ifdef TQ_WS_X11
                XSetTransientForHint( tqt_xdisplay(), winId(), parentWidget()->topLevelWidget()->winId());
#else
                //TODO: WIN32?
#endif
                show();
                }
            if( ev->type() == TQEvent::Hide )
                hide();
	}
    }
    else
    {
        if( parentWidget() && obj == parentWidget()->topLevelWidget())
        {
            if( ev->type() == TQEvent::WindowStateChange
                && !parentWidget()->topLevelWidget()->isFullScreen() )
                setTopLevelMenuInternal( d->wasTopLevel );
        }
    }
    return TQMenuBar::eventFilter( obj, ev );
}

// KDE4 remove
void KMenuBar::showEvent( TQShowEvent *e )
{
    TQMenuBar::showEvent(e);
}

void KMenuBar::updateFallbackSize()
{
    if( !d->topLevel )
	return;
#ifdef TQ_WS_X11
    if( d->selection->owner() != None )
#endif
    { // somebody is managing us, don't mess anything, undo changes
      // done in fallback mode if needed
        d->selection_timer.stop();
        if( d->fallback_mode )
        {
            d->fallback_mode = false;
            KWin::setStrut( winId(), 0, 0, 0, 0 );
            setMinimumSize( 0, 0 );
            setMaximumSize( TQWIDGETSIZE_MAX, TQWIDGETSIZE_MAX );
            updateMenuBarSize();
        }
	return;
    }
    if( d->selection_timer.isActive())
	return;
    d->selection_timer.start( 100, true );
}

void KMenuBar::selectionTimeout()
{ // nobody is managing us, handle resizing
    if ( d->topLevel )
    {
        d->fallback_mode = true; // KMenuBar is handling its position itself
        TDEConfigGroup xineramaConfig(TDEGlobal::config(),"Xinerama");
        int screen = xineramaConfig.readNumEntry("MenubarScreen",
            TQApplication::desktop()->screenNumber(TQPoint(0,0)) );
        TQRect area;
        if (kapp->desktop()->numScreens() < 2)
            area = kapp->desktop()->geometry();
        else
            area = kapp->desktop()->screenGeometry(screen);
        int margin = 0;
	move(area.left() - margin, area.top() - margin); 
        setFixedSize(area.width() + 2* margin , heightForWidth( area.width() + 2 * margin ) );
#ifdef TQ_WS_X11
        int strut_height = height() - margin;
        if( strut_height < 0 )
            strut_height = 0;
        KWin::setStrut( winId(), 0, 0, strut_height, 0 );
#endif
    }
}

int KMenuBar::block_resize = 0;

void KMenuBar::resizeEvent( TQResizeEvent *e )
{
    if( e->spontaneous() && d->topLevel && !d->fallback_mode )
        {
        ++block_resize; // do not respond with configure request to ConfigureNotify event
        TQMenuBar::resizeEvent(e); // to avoid possible infinite loop
        --block_resize;
        }
    else
        TQMenuBar::resizeEvent(e);
}

void KMenuBar::setGeometry( const TQRect& r )
{
    setGeometry( r.x(), r.y(), r.width(), r.height() );
}

void KMenuBar::setGeometry( int x, int y, int w, int h )
{
    if( block_resize > 0 )
    {
	move( x, y );
	return;
    }
    checkSize( w, h );
    if( geometry() != TQRect( x, y, w, h ))
        TQMenuBar::setGeometry( x, y, w, h );
}

void KMenuBar::resize( int w, int h )
{
    if( block_resize > 0 )
	return;
    checkSize( w, h );
    if( size() != TQSize( w, h ))
        TQMenuBar::resize( w, h );
//    kdDebug() << "RS:" << w << ":" << h << ":" << width() << ":" << height() << ":" << minimumWidth() << ":" << minimumHeight() << endl;
}

void KMenuBar::checkSize( int& w, int& h )
{
    if( !d->topLevel || d->fallback_mode )
	return;
    TQSize s = sizeHint();
    w = s.width();
    h = s.height();
    // This is not done as setMinimumSize(), because that would set the minimum
    // size in WM_NORMAL_HINTS, and KWin would not allow changing to smaller size
    // anymore
    w = KMAX( w, d->min_size.width());
    h = KMAX( h, d->min_size.height());
}

// QMenuBar's sizeHint() gives wrong size (insufficient width), which causes wrapping in the kicker applet
TQSize KMenuBar::sizeHint() const
{
    if( !d->topLevel || block_resize > 0 )
        return TQMenuBar::sizeHint();
    // Since TQMenuBar::sizeHint() may indirectly call resize(),
    // avoid infinite recursion.
    ++block_resize;
    // find the minimum useful height, and enlarge the width until the menu fits in that height (one row)
    int h = heightForWidth( 1000000 );
    int w = TQMenuBar::sizeHint().width();
    // optimization - don't call heightForWidth() too many times
    while( heightForWidth( w + 12 ) > h )
        w += 12;
    while( heightForWidth( w + 4 ) > h )
        w += 4;
    while( heightForWidth( w ) > h )
        ++w;
    --block_resize;
    return TQSize( w, h );
}

#ifdef TQ_WS_X11
bool KMenuBar::x11Event( XEvent* ev )
{
    if( ev->type == ClientMessage && ev->xclient.message_type == msg_type_atom
        && ev->xclient.window == winId())
    {
        // TQMenuBar is trying really hard to keep the size it deems right.
        // Forcing minimum size and blocking resizing to match parent size
        // in checkResizingToParent() seem to be the only way to make
        // KMenuBar keep the size it wants
	d->min_size = TQSize( ev->xclient.data.l[ 1 ], ev->xclient.data.l[ 2 ] );
//        kdDebug() << "MINSIZE:" << d->min_size << endl;
        updateMenuBarSize();
	return true;
    }
    return TQMenuBar::x11Event( ev );
}
#endif

void KMenuBar::updateMenuBarSize()
    {
    menuContentsChanged(); // trigger invalidating calculated size
    resize( sizeHint());   // and resize to preferred size
    }

void KMenuBar::setFrameStyle( int style )
{
    if( d->topLevel )
	d->frameStyle = style;
    else
	TQMenuBar::setFrameStyle( style );
}

void KMenuBar::setLineWidth( int width )
{
    if( d->topLevel )
	d->lineWidth = width;
    else
	TQMenuBar::setLineWidth( width );
}

void KMenuBar::setMargin( int margin )
{
    if( d->topLevel )
	d->margin = margin;
    else
	TQMenuBar::setMargin( margin );
}

void KMenuBar::closeEvent( TQCloseEvent* e )
{
    if( d->topLevel )
        e->ignore(); // mainly for the fallback mode 
    else
        TQMenuBar::closeEvent( e );
}

void KMenuBar::drawContents( TQPainter* p )
{
    // Closes the BR77113
    // We need to overload this method to paint only the menu items
    // This way when the KMenuBar is embedded in the menu applet it
    // integrates correctly.
    //
    // Background mode and origin are set so late because of styles
    // using the polish() method to modify these settings.
    //
    // Of course this hack can safely be removed when real transparency
    // will be available

    if( !d->topLevel )
    {
        TQMenuBar::drawContents(p);
    }
    else
    {
        bool up_enabled = isUpdatesEnabled();
        BackgroundMode bg_mode = backgroundMode();
        BackgroundOrigin bg_origin = backgroundOrigin();
        
        setUpdatesEnabled(false);
        setBackgroundMode(X11ParentRelative);
        setBackgroundOrigin(WindowOrigin);

	p->eraseRect( rect() );
	erase();
        
        TQColorGroup g = colorGroup();
        bool e;

        for ( int i=0; i<(int)count(); i++ )
        {
            TQMenuItem *mi = findItem( idAt( i ) );

            if ( !mi->text().isNull() || mi->pixmap() )
            {
                TQRect r = itemRect(i);
                if(r.isEmpty() || !mi->isVisible())
                    continue;

                e = mi->isEnabledAndVisible();
                if ( e )
                    g = isEnabled() ? ( isActiveWindow() ? palette().active() :
                                        palette().inactive() ) : palette().disabled();
                else
                    g = palette().disabled();

                bool item_active = ( actItem ==  i );

                p->setClipRect(r);

                if( item_active )
                {
                    TQStyle::SFlags flags = TQStyle::Style_Default;
                    if (isEnabled() && e)
                        flags |= TQStyle::Style_Enabled;
                    if ( item_active )
                        flags |= TQStyle::Style_Active;
                    if ( item_active && actItemDown )
                        flags |= TQStyle::Style_Down;
                    flags |= TQStyle::Style_HasFocus;

                    style().drawControl(TQStyle::CE_MenuBarItem, p, this,
                                        r, g, flags, TQStyleOption(mi));
                }
                else
                {
                    style().drawItem(p, r, AlignCenter | AlignVCenter | ShowPrefix,
                                     g, e, mi->pixmap(), mi->text());
                }
            }
        }

        setBackgroundOrigin(bg_origin);
        setBackgroundMode(bg_mode);
        setUpdatesEnabled(up_enabled);
    }
}

void KMenuBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdemenubar.moc"
