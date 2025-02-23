/* This file is part of the KDE libraries
    Copyright (C) 1999 Matthias Ettrich (ettrich@kde.org)

    $Id$

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

#include <stdlib.h>
#include <unistd.h>

#ifdef HAVE_SYSENT_H
#include <sysent.h>
#endif

#include <kuniqueapplication.h>
#include <tqbitmap.h>
#include <tqimage.h>
#include <tqwhatsthis.h>
#include <tqcstring.h>
#include <tqdialog.h>

#include "config.h"
#include "twin.h"
#include "tdeapplication.h"

#include <tdeglobal.h>
#include <kiconloader.h>
#include <kdebug.h>

#include <kdatastream.h>
#include <tdelocale.h>
#include <dcopclient.h>
#include <dcopref.h>
#ifdef TQ_WS_X11
#include <tdestartupinfo.h>
#include <kxerrorhandler.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

#include "netwm.h"

static bool atoms_created = false;
extern Atom tqt_wm_protocols;

static Atom net_wm_context_help;
static Atom kde_wm_change_state;
static Atom kde_wm_window_opacity;
static Atom kde_wm_window_shadow;
static Atom twin_UTF8_STRING;
static Atom net_wm_cm;

static void twin_net_create_atoms() {
    if (!atoms_created){
	const int max = 20;
	Atom* atoms[max];
	const char* names[max];
	Atom atoms_return[max];
	int n = 0;

	atoms[n] = &net_wm_context_help;
	names[n++] = "_NET_WM_CONTEXT_HELP";

	atoms[n] = &kde_wm_change_state;
	names[n++] = "_TDE_WM_CHANGE_STATE";

        atoms[n] = &kde_wm_window_opacity;
        names[n++] = (char*) "_TDE_WM_WINDOW_OPACITY";

        atoms[n] = &kde_wm_window_shadow;
        names[n++] = (char*) "_TDE_WM_WINDOW_SHADOW";

        char net_wm_cm_name[ 100 ];
        sprintf( net_wm_cm_name, "_NET_WM_CM_S%d", DefaultScreen( tqt_xdisplay()));
        atoms[n] = &net_wm_cm;
        names[n++] = net_wm_cm_name;

	// we need a const_cast for the horrible X API
	XInternAtoms( tqt_xdisplay(), const_cast<char**>(names), n, false, atoms_return );
	for (int i = 0; i < n; i++ )
	    *atoms[i] = atoms_return[i];

	atoms_created = True;
    }
}
#endif

/*
  Sends a client message to the ROOT window.
 */
#ifdef TQ_WS_X11
static void sendClientMessageToRoot(Window w, Atom a, long x, long y = 0, long z = 0 ){
  XEvent ev;
  long mask;

  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = y;
  ev.xclient.data.l[2] = z;
  mask = SubstructureRedirectMask;
  XSendEvent(tqt_xdisplay(), tqt_xrootwin(), False, mask, &ev);
}
#endif

/*
  Send a client message to window w
 */
#ifdef TQ_WS_X11
static void sendClientMessage(Window w, Atom a, long x){
  XEvent ev;
  long mask;

  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = w;
  ev.xclient.message_type = a;
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = x;
  ev.xclient.data.l[1] = CurrentTime;
  mask = 0L;
  if (w == tqt_xrootwin())
    mask = SubstructureRedirectMask;        /* magic! */
  XSendEvent(tqt_xdisplay(), w, False, mask, &ev);
}
#endif

bool KWin::compositingActive()
{
#ifdef TQ_WS_X11
    twin_net_create_atoms();
    return XGetSelectionOwner( tqt_xdisplay(), net_wm_cm ) != None;
#else
    return false;
#endif
}

#ifdef TQ_WS_X11
namespace
{
class ContextWidget : public TQWidget
{
public:
    ContextWidget();
    virtual bool x11Event( XEvent * ev);
};

ContextWidget::ContextWidget()
	: TQWidget(0,0)
    {
	twin_net_create_atoms();
	kapp->installX11EventFilter( this );
	TQWhatsThis::enterWhatsThisMode();
	TQCursor c = *TQApplication::overrideCursor();
	TQWhatsThis::leaveWhatsThisMode();
	XGrabPointer( tqt_xdisplay(), tqt_xrootwin(), true,
		      (uint)( ButtonPressMask | ButtonReleaseMask |
			      PointerMotionMask | EnterWindowMask |
			      LeaveWindowMask ),
		      GrabModeAsync, GrabModeAsync,
		      None, c.handle(), CurrentTime );
	tqApp->enter_loop();
    }


bool ContextWidget::x11Event( XEvent * ev)
    {
	if ( ev->type == ButtonPress && ev->xbutton.button == Button1 ) {
	    XUngrabPointer( tqt_xdisplay(), ev->xbutton.time );
	    Window root;
	    Window child = tqt_xrootwin();
	    int root_x, root_y, lx, ly;
	    uint state;
	    Window w;
	    do {
		w = child;
		XQueryPointer( tqt_xdisplay(), w, &root, &child,
			       &root_x, &root_y, &lx, &ly, &state );
	    } while  ( child != None && child != w );

	    ::sendClientMessage(w, tqt_wm_protocols, net_wm_context_help);
	    XEvent e = *ev;
	    e.xbutton.window = w;
	    e.xbutton.subwindow = w;
	    e.xbutton.x = lx;
	    e.xbutton.y = ly;
	    XSendEvent( tqt_xdisplay(), w, true, ButtonPressMask, &e );
	    tqApp->exit_loop();
	    return true;
	}
	return false;
    }
} // namespace
#endif

void KWin::invokeContextHelp()
{
#ifdef TQ_WS_X11
    ContextWidget w;
#endif
}

void KWin::setSystemTrayWindowFor( WId trayWin, WId forWin )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), trayWin, tqt_xrootwin(), 0 );
    if ( !forWin )
	forWin = tqt_xrootwin();
    info.setKDESystemTrayWinFor( forWin );
    NETRootInfo rootinfo( tqt_xdisplay(), NET::Supported );
    if( !rootinfo.isSupported( NET::WMKDESystemTrayWinFor )) {
        DCOPRef ref( "kded", "kded" );
        if( !ref.send( "loadModule", TQCString( "kdetrayproxy" )))
            kdWarning( 176 ) << "Loading of kdetrayproxy failed." << endl;
    }
#endif
}

void KWin::activateWindow( WId win, long time )
{
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), 0 );
    if( time == 0 )
        time = get_tqt_x_user_time();
    info.setActiveWindow( win, NET::FromApplication, time,
        kapp->activeWindow() ? kapp->activeWindow()->winId() : 0 );
#endif // TQ_WS_X11 ...
    KUniqueApplication::setHandleAutoStarted();
}

void KWin::forceActiveWindow( WId win, long time )
{
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), 0 );
    if( time == 0 )
        time = get_tqt_x_time();
    info.setActiveWindow( win, NET::FromTool, time, 0 );
#endif // TQ_WS_X11
    KUniqueApplication::setHandleAutoStarted();
}

void KWin::setActiveWindow( WId win )
{
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), 0 );
    info.setActiveWindow( win, NET::FromUnknown, 0, 0 );
#endif
    KUniqueApplication::setHandleAutoStarted();
}

void KWin::demandAttention( WId win, bool set )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), 0 );
    info.setState( set ? NET::DemandsAttention : 0, NET::DemandsAttention );
#endif
}

void KWin::setUserTime( WId win, long time )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), 0 );
    info.setUserTime( time );
#endif
}

KWin::WindowInfo KWin::windowInfo( WId win, unsigned long properties, unsigned long properties2 )
{
    return WindowInfo( win, properties, properties2 );
}


WId KWin::transientFor( WId win )
{
#ifdef TQ_WS_X11
    KXErrorHandler handler; // ignore badwindow
    Window transient_for = None;
    if( XGetTransientForHint( tqt_xdisplay(), win, &transient_for ))
        return transient_for;
    // XGetTransientForHint() did sync
    return None;
#else
    return 0L;
#endif
}

void KWin::setMainWindow( TQWidget* subwindow, WId mainwindow )
{
#ifdef TQ_WS_X11
    if( mainwindow != 0 )
    {
        /*
         Grmbl. See TQDialog::show(). That should get fixed in Qt somehow.
        */
        if( tqt_cast< TQDialog* >( subwindow ) != NULL
            && subwindow->parentWidget() == NULL
            && kapp->mainWidget() != NULL )
        {
            kdWarning() << "KWin::setMainWindow(): There either mustn't be kapp->mainWidget(),"
                " or the dialog must have a non-NULL parent, otherwise Qt will reset the change. Bummer." << endl;
        }
        XSetTransientForHint( tqt_xdisplay(), subwindow->winId(), mainwindow );
    }
    else
        XDeleteProperty( tqt_xdisplay(), subwindow->winId(), XA_WM_TRANSIENT_FOR );
#endif
}

WId KWin::groupLeader( WId win )
{
#ifdef TQ_WS_X11
    KXErrorHandler handler; // ignore badwindow
    XWMHints *hints = XGetWMHints( tqt_xdisplay(), win );
    Window window_group = None;
    if ( hints )
    {
        if( hints->flags & WindowGroupHint )
            window_group = hints->window_group;
        XFree( reinterpret_cast< char* >( hints ));
    }
    // XGetWMHints() did sync
    return window_group;
#else
    return 0L;
#endif
}

// this one is deprecated, KWin::WindowInfo should be used instead
KWin::Info KWin::info( WId win )
{
    Info w;
#ifdef TQ_WS_X11
    NETWinInfo inf( tqt_xdisplay(), win, tqt_xrootwin(),
		    NET::WMState |
		    NET::WMStrut |
		    NET::WMWindowType |
		    NET::WMName |
		    NET::WMVisibleName |
		    NET::WMDesktop |
		    NET::WMPid |
		    NET::WMKDEFrameStrut |
		    NET::XAWMState
		    );

    w.win = win;
    w.state = inf.state();
    w.mappingState = inf.mappingState();
    w.strut = inf.strut();
    w.windowType = inf.windowType( -1U );
    if ( inf.name() ) {
	w.name = TQString::fromUtf8( inf.name() );
    } else {
	char* c = 0;
	if ( XFetchName( tqt_xdisplay(), win, &c ) != 0 ) {
	    w.name = TQString::fromLocal8Bit( c );
	    XFree( c );
	}
    }
    if ( inf.visibleName() )
	w.visibleName = TQString::fromUtf8( inf.visibleName() );
    else
	w.visibleName = w.name;

    w.desktop = inf.desktop();
    w.onAllDesktops = inf.desktop() == NETWinInfo::OnAllDesktops;
    w.pid = inf.pid();
    NETRect frame, geom;
    inf.kdeGeometry( frame, geom );
    w.geometry.setRect( geom.pos.x, geom.pos.y, geom.size.width, geom.size.height );
    w.frameGeometry.setRect( frame.pos.x, frame.pos.y, frame.size.width, frame.size.height );
#endif
    return w;
}

TQPixmap KWin::icon( WId win, int width, int height, bool scale )
{
    return icon( win, width, height, scale, NETWM | WMHints | ClassHint | XApp );
}


TQPixmap KWin::icon( WId win, int width, int height, bool scale, int flags )
{
#ifdef TQ_WS_X11
    KXErrorHandler handler; // ignore badwindow
#endif
    TQPixmap result;
#ifdef TQ_WS_X11
    if( flags & NETWM ) {
        NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), NET::WMIcon );
        NETIcon ni = info.icon( width, height );
        if ( ni.data && ni.size.width > 0 && ni.size.height > 0 ) {
    	    TQImage img( (uchar*) ni.data, (int) ni.size.width, (int) ni.size.height, 32, 0, 0, TQImage::IgnoreEndian );
	    img.setAlphaBuffer( true );
	    if ( scale && width > 0 && height > 0 &&img.size() != TQSize( width, height ) && !img.isNull() )
	        img = TQImage(img).smoothScale( width, height );
	    if ( !img.isNull() )
	        result.convertFromImage( img );
	    return result;
        }
    }

    if( flags & WMHints ) {
        Pixmap p = None;
        Pixmap p_mask = None;

        XWMHints *hints = XGetWMHints(tqt_xdisplay(), win );
        if (hints && (hints->flags & IconPixmapHint)){
    	    p = hints->icon_pixmap;
        }
        if (hints && (hints->flags & IconMaskHint)){
	    p_mask = hints->icon_mask;
        }
        if (hints)
	    XFree((char*)hints);

        if (p != None){
	    Window root;
	    int x, y;
	    unsigned int w = 0;
	    unsigned int h = 0;
            unsigned int border_w, depth;
	    XGetGeometry(tqt_xdisplay(), p, &root,
		         &x, &y, &w, &h, &border_w, &depth);
	    if (w > 0 && h > 0){
	        TQPixmap pm(w, h, depth);
	        // Always detach before doing something behind QPixmap's back.
	        pm.detach();
	        XCopyArea(tqt_xdisplay(), p, pm.handle(),
		          tqt_xget_temp_gc(tqt_xscreen(), depth==1),
		          0, 0, w, h, 0, 0);
	        if (p_mask != None){
	    	    TQBitmap bm(w, h);
		    XCopyArea(tqt_xdisplay(), p_mask, bm.handle(),
			      tqt_xget_temp_gc(tqt_xscreen(), true),
			      0, 0, w, h, 0, 0);
		    pm.setMask(bm);
	        }
	        if ( scale && width > 0 && height > 0 && !pm.isNull() &&
		     ( (int) w != width || (int) h != height) ){
		    result.convertFromImage( TQImage(pm.convertToImage()).smoothScale( width, height ) );
	        } else {
		    result = pm;
	        }
	    }
        }
    }

    // Since width can be any arbitrary size, but the icons cannot,
    // take the nearest value for best results (ignoring 22 pixel
    // icons as they don't exist for apps):
    int iconWidth;
    if( width < 24 )
        iconWidth = 16;
    else if( width < 40 )
        iconWidth = 32;
    else
        iconWidth = 48;

    if( flags & ClassHint ) {
        // Try to load the icon from the classhint if the app didn't specify
        // its own:
        if( result.isNull() ) {

	    XClassHint	hint;
	    if( XGetClassHint( tqt_xdisplay(), win, &hint ) ) {
	        TQString className = hint.res_class;

	        TQPixmap pm = TDEGlobal::instance()->iconLoader()->loadIcon( className.lower(), TDEIcon::Small, iconWidth,
								          TDEIcon::DefaultState, 0, true );
	        if( scale && !pm.isNull() )
		    result.convertFromImage( TQImage(pm.convertToImage()).smoothScale( width, height ) );
	        else
		    result = pm;

	        XFree( hint.res_name );
	        XFree( hint.res_class );
	    }
        }
    }

    if( flags & XApp ) {
	// If the icon is still a null pixmap, load the 'xapp' icon
	// as a last resort:
	if ( result.isNull() ) {
	    TQPixmap pm = TDEGlobal::instance()->iconLoader()->loadIcon(  "xapp", TDEIcon::Small, iconWidth,
								       TDEIcon::DefaultState, 0, true );
	    if( scale && !pm.isNull() )
		result.convertFromImage( TQImage(pm.convertToImage()).smoothScale( width, height ) );
	    else
		result = pm;
	}
    }
#endif
    return result;
}

void KWin::setIcons( WId win, const TQPixmap& icon, const TQPixmap& miniIcon )
{
#ifdef TQ_WS_X11
    if ( icon.isNull() )
	return;
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), 0 );
    TQImage img = TQImage(icon.convertToImage()).convertDepth( 32 );
    NETIcon ni;
    ni.size.width = img.size().width();
    ni.size.height = img.size().height();
    ni.data = (unsigned char *) img.bits();
    info.setIcon( ni, true );
    if ( miniIcon.isNull() )
	return;
    img = TQImage(miniIcon.convertToImage()).convertDepth( 32 );
    ni.size.width = img.size().width();
    ni.size.height = img.size().height();
    ni.data = (unsigned char *) img.bits();
    info.setIcon( ni, false );
#endif
}

void KWin::setType( WId win, NET::WindowType windowType )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), 0 );
    info.setWindowType( windowType );
#endif
}

void KWin::setState( WId win, unsigned long state )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), NET::WMState );
    info.setState( state, state );
#endif
}

void KWin::clearState( WId win, unsigned long state )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), NET::WMState );
    info.setState( 0, state );
#endif
}

void KWin::setOpacity( WId win, uint percent )
{
#ifdef TQ_WS_X11
    twin_net_create_atoms();
    if (percent > 99)
        XDeleteProperty (tqt_xdisplay(), win, kde_wm_window_opacity);
    else
    {
        long opacity = long(0xFFFFFFFF/100.0*percent);
        XChangeProperty(tqt_xdisplay(), win, kde_wm_window_opacity, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &opacity, 1L);
    }
#endif
}

void KWin::setShadowSize( WId win, uint percent )
{
#ifdef TQ_WS_X11
    twin_net_create_atoms();
    long shadowSize = long(0xFFFFFFFF/100.0*percent);
    XChangeProperty(tqt_xdisplay(), win, kde_wm_window_shadow, XA_CARDINAL, 32, PropModeReplace, (unsigned char *) &shadowSize, 1L);
#endif
}

void KWin::setOnAllDesktops( WId win, bool b )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), NET::WMDesktop );
    if ( b )
	info.setDesktop( NETWinInfo::OnAllDesktops );
    else if ( info.desktop()  == NETWinInfo::OnAllDesktops ) {
	NETRootInfo rinfo( tqt_xdisplay(), NET::CurrentDesktop );
	info.setDesktop( rinfo.currentDesktop() );
    }
#endif
}

void KWin::setOnDesktop( WId win, int desktop )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), NET::WMDesktop );
    info.setDesktop( desktop );
#endif
}

void KWin::setExtendedStrut( WId win, int left_width, int left_start, int left_end,
    int right_width, int right_start, int right_end, int top_width, int top_start, int top_end,
    int bottom_width, int bottom_start, int bottom_end )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), 0 );
    NETExtendedStrut strut;
    strut.left_width = left_width;
    strut.right_width = right_width;
    strut.top_width = top_width;
    strut.bottom_width = bottom_width;
    strut.left_start = left_start;
    strut.left_end = left_end;
    strut.right_start = right_start;
    strut.right_end = right_end;
    strut.top_start = top_start;
    strut.top_end = top_end;
    strut.bottom_start = bottom_start;
    strut.bottom_end = bottom_end;
    info.setExtendedStrut( strut );
#endif
}

void KWin::setStrut( WId win, int left, int right, int top, int bottom )
{
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(), win, tqt_xrootwin(), 0 );
    NETStrut strut;
    strut.left = left;
    strut.right = right;
    strut.top = top;
    strut.bottom = bottom;
    info.setStrut( strut );
#endif
}

int KWin::currentDesktop()
{
#ifdef TQ_WS_X11
    if (!tqt_xdisplay())
#endif
      return 1;
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), NET::CurrentDesktop );
    return info.currentDesktop();
#endif
}

int KWin::numberOfDesktops()
{
#ifdef TQ_WS_X11
    if (!tqt_xdisplay())
#endif
      return 0;
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), NET::NumberOfDesktops );
    return info.numberOfDesktops();
#endif
}

void KWin::setCurrentDesktop( int desktop )
{
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), NET::CurrentDesktop );
    info.setCurrentDesktop( desktop );
#endif
}

void KWin::setCurrentDesktopViewport( int desktop, TQPoint viewport )
{
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), NET::CurrentDesktop );
    NETPoint netview;
    netview.x = viewport.x();
    netview.y = viewport.y();
    info.setDesktopViewport( desktop, netview );
#endif
}

void KWin::iconifyWindow( WId win, bool animation)
{
#ifdef TQ_WS_X11
    if ( !animation )
    {
        twin_net_create_atoms();
	sendClientMessageToRoot( win, kde_wm_change_state, IconicState, 1 );
    }
    XIconifyWindow( tqt_xdisplay(), win, tqt_xscreen() );
#endif
}


void KWin::deIconifyWindow( WId win, bool animation )
{
#ifdef TQ_WS_X11
    if ( !animation )
    {
        twin_net_create_atoms();
	sendClientMessageToRoot( win, kde_wm_change_state, NormalState, 1 );
    }
    XMapWindow( tqt_xdisplay(), win );
#endif
}

void KWin::raiseWindow( WId win )
{
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), NET::Supported );
    if( info.isSupported( NET::WM2RestackWindow ))
        info.restackRequest( win, None, Above );
    else
        XRaiseWindow( tqt_xdisplay(), win );
#endif
}

void KWin::lowerWindow( WId win )
{
#ifdef TQ_WS_X11
    NETRootInfo info( tqt_xdisplay(), NET::Supported );
    if( info.isSupported( NET::WM2RestackWindow ))
        info.restackRequest( win, None, Below );
    else
        XLowerWindow( tqt_xdisplay(), win );
#endif
}

void KWin::appStarted()
{
#ifdef TQ_WS_X11
    TDEStartupInfo::appStarted();
#endif
}

class KWin::WindowInfoPrivate
{
    public:
	WindowInfoPrivate()
#ifdef TQ_WS_X11
	: info( NULL )
#endif
	{}
#ifdef TQ_WS_X11
	~WindowInfoPrivate() { delete info; }
	NETWinInfo* info;
#endif
	WId win_;
	TQString name_;
        TQString iconic_name_;
	TQRect geometry_;
        TQRect frame_geometry_;
	int ref;
        bool valid;
    private:
	WindowInfoPrivate( const WindowInfoPrivate& );
	void operator=( const WindowInfoPrivate& );
};

// KWin::info() should be updated too if something has to be changed here
KWin::WindowInfo::WindowInfo( WId win, unsigned long properties, unsigned long properties2 )
{
#ifdef TQ_WS_X11
    KXErrorHandler handler;
    d = new WindowInfoPrivate;
    d->ref = 1;
    if( properties == 0 )
	properties = NET::WMState |
		     NET::WMStrut |
		     NET::WMWindowType |
		     NET::WMName |
		     NET::WMVisibleName |
                     NET::WMIconName |
                     NET::WMVisibleIconName |
		     NET::WMDesktop |
		     NET::WMPid |
		     NET::WMKDEFrameStrut |
		     NET::XAWMState |
                     NET::WMGeometry;
    if( properties & NET::WMVisibleIconName )
	properties |= NET::WMIconName | NET::WMVisibleName; // force, in case it will be used as a fallback
    if( properties & NET::WMVisibleName )
	properties |= NET::WMName; // force, in case it will be used as a fallback
    if( properties2 & NET::WM2ExtendedStrut )
        properties |= NET::WMStrut; // will be used as fallback
    properties |= NET::XAWMState; // force to get error detection for valid()
    unsigned long props[ 2 ] = { properties, properties2 };
    d->info = new NETWinInfo( tqt_xdisplay(), win, tqt_xrootwin(), props, 2 );
    d->win_ = win;
    if( properties & NET::WMName ) {
        if( d->info->name() && d->info->name()[ 0 ] != '\0' )
	    d->name_ = TQString::fromUtf8( d->info->name() );
        else
            d->name_ = readNameProperty( win, XA_WM_NAME );
    }
    if( properties & NET::WMIconName ) {
        if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
            d->iconic_name_ = TQString::fromUtf8( d->info->iconName());
        else
            d->iconic_name_ = readNameProperty( win, XA_WM_ICON_NAME );
    }
    if( properties & ( NET::WMGeometry | NET::WMKDEFrameStrut )) {
        NETRect frame, geom;
        d->info->kdeGeometry( frame, geom );
        d->geometry_.setRect( geom.pos.x, geom.pos.y, geom.size.width, geom.size.height );
        d->frame_geometry_.setRect( frame.pos.x, frame.pos.y, frame.size.width, frame.size.height );
    }
    d->valid = !handler.error( false ); // no sync - NETWinInfo did roundtrips
#endif
}

// this one is only to make TQValueList<> or similar happy
KWin::WindowInfo::WindowInfo()
    : d( NULL )
{
}

KWin::WindowInfo::~WindowInfo()
{
    if( d != NULL ) {
	if( --d->ref == 0 ) {
	    delete d;
	}
    }
}

KWin::WindowInfo::WindowInfo( const WindowInfo& wininfo )
    : d( wininfo.d )
{
    if( d != NULL )
	++d->ref;
}

KWin::WindowInfo& KWin::WindowInfo::operator=( const WindowInfo& wininfo )
{
    if( d != wininfo.d ) {
	if( d != NULL )
	    if( --d->ref == 0 )
		delete d;
	d = wininfo.d;
	if( d != NULL )
	    ++d->ref;
    }
    return *this;
}

bool KWin::WindowInfo::valid( bool withdrawn_is_valid ) const
{
    if( !d->valid )
        return false;
    if( !withdrawn_is_valid && mappingState() == NET::Withdrawn )
        return false;
    return true;
}

WId KWin::WindowInfo::win() const
{
    return d->win_;
}

unsigned long KWin::WindowInfo::state() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMState ) == 0, 176 )
        << "Pass NET::WMState to KWin::windowInfo()" << endl;
    return d->info->state();
#else
    return 0;
#endif
}

NET::MappingState KWin::WindowInfo::mappingState() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::XAWMState ) == 0, 176 )
        << "Pass NET::XAWMState to KWin::windowInfo()" << endl;
    return d->info->mappingState();
#else
    return NET::Visible;
#endif
}

NETExtendedStrut KWin::WindowInfo::extendedStrut() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ExtendedStrut ) == 0, 176 )
        << "Pass NET::WM2ExtendedStrut to second argument of KWin::windowInfo()" << endl;
    NETExtendedStrut ext = d->info->extendedStrut();
    NETStrut str = d->info->strut();
    if( ext.left_width == 0 && ext.right_width == 0 && ext.top_width == 0 && ext.bottom_width == 0
        && ( str.left != 0 || str.right != 0 || str.top != 0 || str.bottom != 0 )) {
        // build extended from simple
        if( str.left != 0 ) {
            ext.left_width = str.left;
            ext.left_start = 0;
            ext.left_end = XDisplayHeight( tqt_xdisplay(), DefaultScreen( tqt_xdisplay()));
        }
        if( str.right != 0 ) {
            ext.right_width = str.right;
            ext.right_start = 0;
            ext.right_end = XDisplayHeight( tqt_xdisplay(), DefaultScreen( tqt_xdisplay()));
        }
        if( str.top != 0 ) {
            ext.top_width = str.top;
            ext.top_start = 0;
            ext.top_end = XDisplayWidth( tqt_xdisplay(), DefaultScreen( tqt_xdisplay()));
        }
        if( str.bottom != 0 ) {
            ext.bottom_width = str.bottom;
            ext.bottom_start = 0;
            ext.bottom_end = XDisplayWidth( tqt_xdisplay(), DefaultScreen( tqt_xdisplay()));
        }
    }
    return ext;
#else
    NETExtendedStrut n;
    return n;
#endif
}

NETStrut KWin::WindowInfo::strut() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMStrut ) == 0, 176 )
        << "Pass NET::WMStrut to KWin::windowInfo()" << endl;
    return d->info->strut();
#else
    NETStrut n;
    return n;
#endif
}

NET::WindowType KWin::WindowInfo::windowType( int supported_types ) const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMWindowType ) == 0, 176 )
        << "Pass NET::WMWindowType to KWin::windowInfo()" << endl;
    return d->info->windowType( supported_types );
#else
    return 0;
#endif
}

TQString KWin::WindowInfo::visibleNameWithState() const
{
    TQString s = visibleName();
    if ( isMinimized() ) {
	s.prepend('(');
	s.append(')');
    }
    return s;
}

TQString KWin::Info::visibleNameWithState() const
{
    TQString s = visibleName;
    if ( isMinimized() ) {
	s.prepend('(');
	s.append(')');
    }
    return s;
}

TQString KWin::WindowInfo::visibleName() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMVisibleName ) == 0, 176 )
        << "Pass NET::WMVisibleName to KWin::windowInfo()" << endl;
    return d->info->visibleName() && d->info->visibleName()[ 0 ] != '\0'
        ? TQString::fromUtf8(d->info->visibleName()) : name();
#else
    return TQString("name");
#endif
}

TQString KWin::WindowInfo::name() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMName ) == 0, 176 )
        << "Pass NET::WMName to KWin::windowInfo()" << endl;
    return d->name_;
#else
    return TQString();
#endif
}

TQString KWin::WindowInfo::visibleIconNameWithState() const
{
    TQString s = visibleIconName();
    if ( isMinimized() ) {
	s.prepend('(');
	s.append(')');
    }
    return s;
}

TQString KWin::WindowInfo::visibleIconName() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMVisibleIconName ) == 0, 176 )
        << "Pass NET::WMVisibleIconName to KWin::windowInfo()" << endl;
    if( d->info->visibleIconName() && d->info->visibleIconName()[ 0 ] != '\0' )
        return TQString::fromUtf8( d->info->visibleIconName());
    if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
        return TQString::fromUtf8( d->info->iconName());
    if( !d->iconic_name_.isEmpty())
        return d->iconic_name_;
#endif
    return visibleName();
}

TQString KWin::WindowInfo::iconName() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMIconName ) == 0, 176 )
        << "Pass NET::WMIconName to KWin::windowInfo()" << endl;
    if( d->info->iconName() && d->info->iconName()[ 0 ] != '\0' )
        return TQString::fromUtf8( d->info->iconName());
    if( !d->iconic_name_.isEmpty())
        return d->iconic_name_;
#endif
    return name();
}

bool KWin::WindowInfo::isOnCurrentDesktop() const
{
#ifdef TQ_WS_X11
    return isOnDesktop( KWin::currentDesktop());
#else
    return false;
#endif
}

bool KWin::WindowInfo::isOnDesktop( int desktop ) const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWin::windowInfo()" << endl;
    return d->info->desktop() == desktop || d->info->desktop() == NET::OnAllDesktops;
#else
    return false;
#endif
}

bool KWin::WindowInfo::onAllDesktops() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWin::windowInfo()" << endl;
    return d->info->desktop() == NET::OnAllDesktops;
#else
    return false;
#endif
}

int KWin::WindowInfo::desktop() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMDesktop ) == 0, 176 )
        << "Pass NET::WMDesktop to KWin::windowInfo()" << endl;
    return d->info->desktop();
#else
    return 1;
#endif
}

TQRect KWin::WindowInfo::geometry() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMGeometry ) == 0, 176 )
        << "Pass NET::WMGeometry to KWin::windowInfo()" << endl;
    return d->geometry_;
#else
    return TQRect( 100, 100, 200, 200 );
#endif
}

TQRect KWin::WindowInfo::frameGeometry() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS ] & NET::WMKDEFrameStrut ) == 0, 176 )
        << "Pass NET::WMKDEFrameStrut to KWin::windowInfo()" << endl;
    return d->frame_geometry_;
#else
    return TQRect();
#endif
}

WId KWin::WindowInfo::transientFor() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2TransientFor ) == 0, 176 )
        << "Pass NET::WM2TransientFor to KWin::windowInfo()" << endl;
    return d->info->transientFor();
#else
    return 0;
#endif
}

WId KWin::WindowInfo::groupLeader() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2GroupLeader ) == 0, 176 )
        << "Pass NET::WM2GroupLeader to KWin::windowInfo()" << endl;
    return d->info->groupLeader();
#else
    return 0;
#endif
}

TQCString KWin::WindowInfo::windowClassClass() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
        << "Pass NET::WM2WindowClass to KWin::windowInfo()" << endl;
    return d->info->windowClassClass();
#else
    return 0;
#endif
}

TQCString KWin::WindowInfo::windowClassName() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowClass ) == 0, 176 )
        << "Pass NET::WM2WindowClass to KWin::windowInfo()" << endl;
    return d->info->windowClassName();
#else
    return 0;
#endif
}

TQCString KWin::WindowInfo::windowRole() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2WindowRole ) == 0, 176 )
        << "Pass NET::WM2WindowRole to KWin::windowInfo()" << endl;
    return d->info->windowRole();
#else
    return 0;
#endif
}

TQCString KWin::WindowInfo::clientMachine() const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2ClientMachine ) == 0, 176 )
        << "Pass NET::WM2ClientMachine to KWin::windowInfo()" << endl;
    return d->info->clientMachine();
#else
    return 0;
#endif
}

bool KWin::WindowInfo::actionSupported( NET::Action action ) const
{
#ifdef TQ_WS_X11
    kdWarning(( d->info->passedProperties()[ NETWinInfo::PROTOCOLS2 ] & NET::WM2AllowedActions ) == 0, 176 )
        << "Pass NET::WM2AllowedActions to KWin::windowInfo()" << endl;
    if( allowedActionsSupported())
        return d->info->allowedActions() & action;
    else
#endif
        return true; // no idea if it's supported or not -> pretend it is
}

// see NETWM spec section 7.6
bool KWin::WindowInfo::isMinimized() const
{
#ifdef TQ_WS_X11
    if( mappingState() != NET::Iconic )
        return false;
    // NETWM 1.2 compliant WM - uses NET::Hidden for minimized windows
    if(( state() & NET::Hidden ) != 0
	&& ( state() & NET::Shaded ) == 0 ) // shaded may have NET::Hidden too
        return true;
    // older WMs use WithdrawnState for other virtual desktops
    // and IconicState only for minimized
    return icccmCompliantMappingState() ? false : true;
#else
    return false;
#endif
}

bool KWin::Info::isMinimized() const
{
#ifdef TQ_WS_X11
    if( mappingState != NET::Iconic )
        return false;
    // NETWM 1.2 compliant WM - uses NET::Hidden for minimized windows
    if(( state & NET::Hidden ) != 0
	&& ( state & NET::Shaded ) == 0 ) // shaded may have NET::Hidden too
        return true;
    // older WMs use WithdrawnState for other virtual desktops
    // and IconicState only for minimized
    return icccmCompliantMappingState() ? false : true;
#else
    return false;
#endif
}

bool KWin::Info::isIconified() const
{
    return isMinimized();
}

bool KWin::icccmCompliantMappingState()
{
#ifdef TQ_WS_X11
    static enum { noidea, yes, no } wm_is_1_2_compliant = noidea;
    if( wm_is_1_2_compliant == noidea ) {
        NETRootInfo info( tqt_xdisplay(), NET::Supported );
        wm_is_1_2_compliant = info.isSupported( NET::Hidden ) ? yes : no;
    }
    return wm_is_1_2_compliant == yes;
#else
    return false;
#endif
}

bool KWin::allowedActionsSupported()
{
#ifdef TQ_WS_X11
    static enum { noidea, yes, no } wm_supports_allowed_actions = noidea;
    if( wm_supports_allowed_actions == noidea ) {
        NETRootInfo info( tqt_xdisplay(), NET::Supported );
        wm_supports_allowed_actions = info.isSupported( NET::WM2AllowedActions ) ? yes : no;
    }
    return wm_supports_allowed_actions == yes;
#else
    return false;
#endif
}

TQString KWin::readNameProperty( WId win, unsigned long atom )
{
#ifdef TQ_WS_X11
    XTextProperty tp;
    char **text = NULL;
    int count;
#endif
    TQString result;
#ifdef TQ_WS_X11
    if ( XGetTextProperty( tqt_xdisplay(), win, &tp, atom ) != 0 && tp.value != NULL ) 
    {
        if (!twin_UTF8_STRING)
          twin_UTF8_STRING = XInternAtom( tqt_xdisplay(), "UTF8_STRING", False);

        if ( tp.encoding == twin_UTF8_STRING ) {
            result = TQString::fromUtf8 ( (const char*) tp.value );
        }
        else if ( XmbTextPropertyToTextList( tqt_xdisplay(), &tp, &text, &count) == Success &&
                  text != NULL && count > 0 ) {
            result = TQString::fromLocal8Bit( text[0] );
        } else if ( tp.encoding == XA_STRING )
            result = TQString::fromLocal8Bit( (const char*) tp.value );
        if( text != NULL )
            XFreeStringList( text );
        XFree( tp.value );
    }
#endif
    return result;
}

//#endif
