/****************************************************************************

 $Id$

 Copyright (C) 2001-2003 Lubos Lunak        <l.lunak@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.

****************************************************************************/

#include "kxmessages.h"

#include <tdeapplication.h>

#ifdef TQ_WS_X11 // FIXME(E): Figure out what parts we can/should emulate in QT/E

#include <X11/Xlib.h>
#include <kdebug.h>

// for broadcasting
const long BROADCAST_MASK = PropertyChangeMask;
// CHECKME

KXMessages::KXMessages( const char* accept_broadcast_P, TQWidget* parent_P )
    : TQWidget( parent_P )
    {
    if( accept_broadcast_P != NULL )
        {
        ( void ) kapp->desktop(); //trigger desktop widget creation to select root window events
        kapp->installX11EventFilter( this ); // i.e. PropertyChangeMask
        accept_atom1 = XInternAtom( tqt_xdisplay(), accept_broadcast_P, false );
        accept_atom2 = accept_atom1;
        }
    else
        {
        accept_atom1 = accept_atom2 = None;
        }
    handle = new TQWidget( this );
    }

KXMessages::KXMessages( const char* accept_broadcast_P, TQWidget* parent_P, bool obsolete_P )
    : TQWidget( parent_P )
    {
    if( accept_broadcast_P != NULL )
        {
        ( void ) kapp->desktop(); //trigger desktop widget creation to select root window events
        kapp->installX11EventFilter( this ); // i.e. PropertyChangeMask
        accept_atom2 = XInternAtom( tqt_xdisplay(), accept_broadcast_P, false );
        accept_atom1 = obsolete_P ? accept_atom2
            : XInternAtom( tqt_xdisplay(), TQCString( accept_broadcast_P ) + "_BEGIN", false );
        }
    else
        {
        accept_atom1 = accept_atom2 = None;
        }
    handle = new TQWidget( this );
    }

KXMessages::~KXMessages()    
    {
//    delete d; no private data yet
    }


void KXMessages::broadcastMessage( const char* msg_type_P, const TQString& message_P )
    {
    broadcastMessage( msg_type_P, message_P, -1, true );
    }
    
void KXMessages::broadcastMessage( const char* msg_type_P, const TQString& message_P,
    int screen_P, bool obsolete_P )
    {
    Atom a2 = XInternAtom( tqt_xdisplay(), msg_type_P, false );
    Atom a1 = obsolete_P ? a2 : XInternAtom( tqt_xdisplay(), TQCString( msg_type_P ) + "_BEGIN", false );
    Window root = screen_P == -1 ? tqt_xrootwin() : tqt_xrootwin( screen_P );
    send_message_internal( root, message_P, BROADCAST_MASK, tqt_xdisplay(),
        a1, a2, handle->winId());
    }

void KXMessages::sendMessage( WId w_P, const char* msg_type_P, const TQString& message_P )
    {
    sendMessage( w_P, msg_type_P, message_P, true );
    }
    
void KXMessages::sendMessage( WId w_P, const char* msg_type_P, const TQString& message_P,
    bool obsolete_P )
    {
    Atom a2 = XInternAtom( tqt_xdisplay(), msg_type_P, false );
    Atom a1 = obsolete_P ? a2 : XInternAtom( tqt_xdisplay(), TQCString( msg_type_P ) + "_BEGIN", false );
    send_message_internal( w_P, message_P, 0, tqt_xdisplay(), a1, a2, handle->winId());
    }
    
bool KXMessages::broadcastMessageX( Display* disp, const char* msg_type_P,
    const TQString& message_P )
    {
    return broadcastMessageX( disp, msg_type_P, message_P, -1, true );
    }
    
bool KXMessages::broadcastMessageX( Display* disp, const char* msg_type_P,
    const TQString& message_P, int screen_P, bool obsolete_P )
    {
    if( disp == NULL )
        return false;
    Atom a2 = XInternAtom( disp, msg_type_P, false );
    Atom a1 = obsolete_P ? a2 : XInternAtom( disp, TQCString( msg_type_P ) + "_BEGIN", false );
    Window root = screen_P == -1 ? DefaultRootWindow( disp ) : RootWindow( disp, screen_P );
    Window win = XCreateSimpleWindow( disp, root, 0, 0, 1, 1,
        0, BlackPixel( disp, screen_P == -1 ? DefaultScreen( disp ) : screen_P ),
        BlackPixel( disp, screen_P == -1 ? DefaultScreen( disp ) : screen_P ));
    send_message_internal( root, message_P, BROADCAST_MASK, disp,
        a1, a2, win );
    XDestroyWindow( disp, win );
    return true;
    }

bool KXMessages::sendMessageX( Display* disp, WId w_P, const char* msg_type_P,
    const TQString& message_P )
    {
    return sendMessageX( disp, w_P, msg_type_P, message_P, true );
    }
    
bool KXMessages::sendMessageX( Display* disp, WId w_P, const char* msg_type_P,
    const TQString& message_P, bool obsolete_P )
    {
    if( disp == NULL )
        return false;
    Atom a2 = XInternAtom( disp, msg_type_P, false );
    Atom a1 = obsolete_P ? a2 : XInternAtom( disp, TQCString( msg_type_P ) + "_BEGIN", false );
    Window win = XCreateSimpleWindow( disp, DefaultRootWindow( disp ), 0, 0, 1, 1,
        0, BlackPixelOfScreen( DefaultScreenOfDisplay( disp )),
        BlackPixelOfScreen( DefaultScreenOfDisplay( disp )));
    send_message_internal( w_P, message_P, 0, disp, a1, a2, win );
    XDestroyWindow( disp, win );
    return true;
    }
    
void KXMessages::send_message_internal( WId w_P, const TQString& msg_P, long mask_P,
    Display* disp, Atom atom1_P, Atom atom2_P, Window handle_P )
    {
    unsigned int pos = 0;
    TQCString msg = msg_P.utf8();
    unsigned int len = strlen( msg );
    XEvent e;
    e.xclient.type = ClientMessage;
    e.xclient.message_type = atom1_P; // leading message
    e.xclient.display = disp;
    e.xclient.window = handle_P;
    e.xclient.format = 8;
    do
        {
        unsigned int i;
        for( i = 0;
             i < 20 && i + pos <= len;
             ++i )
            e.xclient.data.b[ i ] = msg[ i + pos ];
        XSendEvent( disp, w_P, false, mask_P, &e );
        e.xclient.message_type = atom2_P; // following messages
        pos += i;
        } while( pos <= len );
    XFlush( disp );
    }

bool KXMessages::x11Event( XEvent* ev_P )
    {
    if( ev_P->type != ClientMessage || ev_P->xclient.format != 8 )
        return TQWidget::x11Event( ev_P );
    if( ev_P->xclient.message_type != accept_atom1 && ev_P->xclient.message_type != accept_atom2 )
        return TQWidget::x11Event( ev_P );
    char buf[ 21 ]; // can't be longer
    int i;
    for( i = 0;
         i < 20 && ev_P->xclient.data.b[ i ] != '\0';
         ++i )
        buf[ i ] = ev_P->xclient.data.b[ i ];
    buf[ i ] = '\0';
    if( incoming_messages.contains( ev_P->xclient.window ))
        {
        if( ev_P->xclient.message_type == accept_atom1 && accept_atom1 != accept_atom2 )
            // two different messages on the same window at the same time shouldn't happen anyway
            incoming_messages[ ev_P->xclient.window ] = TQCString();
        incoming_messages[ ev_P->xclient.window ] += buf;
        }
    else
        {
        if( ev_P->xclient.message_type == accept_atom2 && accept_atom1 != accept_atom2 )
            return false; // middle of message, but we don't have the beginning
        incoming_messages[ ev_P->xclient.window ] = buf;
        }
    if( i < 20 ) // last message fragment
        {
        emit gotMessage( TQString::fromUtf8( incoming_messages[ ev_P->xclient.window ] ));
        incoming_messages.remove( ev_P->xclient.window );
        }
    return false; // lets other KXMessages instances get the event too
    }

#include "kxmessages.moc"
#endif
