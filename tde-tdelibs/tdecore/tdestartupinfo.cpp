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

// kdDebug() can't be turned off in tdeinit
#if 0
#define KSTARTUPINFO_ALL_DEBUG
#warning Extra TDEStartupInfo debug messages enabled.
#endif

#include <tqwidget.h>

#include "config.h"
#ifdef TQ_WS_X11
//#ifdef TQ_WS_X11 // FIXME(E): Re-implement in a less X11 specific way
#include <tqglobal.h>
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "tdestartupinfo.h"

#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <tqtimer.h>
#ifdef TQ_WS_X11
#include <netwm.h>
#endif
#include <kdebug.h>
#include <tdeapplication.h>
#include <signal.h>
#ifdef TQ_WS_X11
#include <twinmodule.h>
#include <kxmessages.h>
#include <twin.h>
#endif

static const char* const NET_STARTUP_MSG = "_NET_STARTUP_INFO";
static const char* const NET_STARTUP_WINDOW = "_NET_STARTUP_ID";
// DESKTOP_STARTUP_ID is used also in tdeinit/wrapper.c ,
// tdesu in both tdelibs and tdebase and who knows where else
static const char* const NET_STARTUP_ENV = "DESKTOP_STARTUP_ID";

static bool auto_app_started_sending = true;

static long get_num( const TQString& item_P );
static unsigned long get_unum( const TQString& item_P );
static TQString get_str( const TQString& item_P );
static TQCString get_cstr( const TQString& item_P );
static TQStringList get_fields( const TQString& txt_P );
static TQString escape_str( const TQString& str_P );

static Atom utf8_string_atom = None;

class TDEStartupInfo::Data
    : public TDEStartupInfoData
    {
    public:
        Data() : TDEStartupInfoData(), age(0) {} // just because it's in a QMap
        Data( const TQString& txt_P )
            : TDEStartupInfoData( txt_P ), age( 0 ) {}
        unsigned int age;
    };

struct TDEStartupInfoPrivate
    {
    public:
        TQMap< TDEStartupInfoId, TDEStartupInfo::Data > startups;
	// contains silenced ASN's only if !AnnounceSilencedChanges
        TQMap< TDEStartupInfoId, TDEStartupInfo::Data > silent_startups;
        // contains ASN's that had change: but no new: yet
        TQMap< TDEStartupInfoId, TDEStartupInfo::Data > uninited_startups;
#ifdef TQ_WS_X11
        KWinModule* wm_module;
        KXMessages msgs;
#endif
	TQTimer* cleanup;
	int flags;
	TDEStartupInfoPrivate( int flags_P )
    	    :
#ifdef TQ_WS_X11
	    msgs( NET_STARTUP_MSG, NULL, false ),
#endif
	      flags( flags_P ) {}
    };

TDEStartupInfo::TDEStartupInfo( int flags_P, TQObject* parent_P, const char* name_P )
    : TQObject( parent_P, name_P ),
        timeout( 60 ), d( NULL )
    {
    init( flags_P );
    }

TDEStartupInfo::TDEStartupInfo( bool clean_on_cantdetect_P, TQObject* parent_P, const char* name_P )
    : TQObject( parent_P, name_P ),
        timeout( 60 ), d( NULL )
    {
    init( clean_on_cantdetect_P ? CleanOnCantDetect : 0 );
    }

void TDEStartupInfo::init( int flags_P )
    {
    // d == NULL means "disabled"
    if( !TDEApplication::kApplication())
        return;
    if( !TDEApplication::kApplication()->getDisplay())
        return;

    d = new TDEStartupInfoPrivate( flags_P );
#ifdef TQ_WS_X11
    if( !( d->flags & DisableKWinModule ))
        {
        d->wm_module = new KWinModule( this );
        connect( d->wm_module, TQ_SIGNAL( windowAdded( WId )), TQ_SLOT( slot_window_added( WId )));
        connect( d->wm_module, TQ_SIGNAL( systemTrayWindowAdded( WId )), TQ_SLOT( slot_window_added( WId )));
        }
    else
        d->wm_module = NULL;
    connect( &d->msgs, TQ_SIGNAL( gotMessage( const TQString& )), TQ_SLOT( got_message( const TQString& )));
#endif
    d->cleanup = new TQTimer( this, "cleanup" );
    connect( d->cleanup, TQ_SIGNAL( timeout()), TQ_SLOT( startups_cleanup()));
    }

TDEStartupInfo::~TDEStartupInfo()
    {
    delete d;
    }

void TDEStartupInfo::got_message( const TQString& msg_P )
    {
// TODO do something with SCREEN= ?
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] got:" << msg_P << endl;
    TQString msg = msg_P.stripWhiteSpace();
    if( msg.startsWith( "new:" )) // must match length below
        got_startup_info( msg.mid( 4 ), false );
    else if( msg.startsWith( "change:" )) // must match length below
        got_startup_info( msg.mid( 7 ), true );
    else if( msg.startsWith( "remove:" )) // must match length below
        got_remove_startup_info( msg.mid( 7 ));
    }

// if the application stops responding for a while, KWinModule may get
// the information about the already mapped window before KXMessages
// actually gets the info about the started application (depends
// on their order in X11 event filter in TDEApplication)
// simply delay info from KWinModule a bit
// SELI???
namespace
{
class DelayedWindowEvent
    : public TQCustomEvent
    {
    public:
	DelayedWindowEvent( WId w_P )
	    : TQCustomEvent( TQEvent::User + 15 ), w( w_P ) {}
	Window w;
    };
}

void TDEStartupInfo::slot_window_added( WId w_P )
    {
    kapp->postEvent( this, new DelayedWindowEvent( w_P ));
    }

void TDEStartupInfo::customEvent( TQCustomEvent* e_P )
    {
    if( e_P->type() == TQEvent::User + 15 )
	window_added( static_cast< DelayedWindowEvent* >( e_P )->w );
    else
	TQObject::customEvent( e_P );
    }

void TDEStartupInfo::window_added( WId w_P )
    {
    TDEStartupInfoId id;
    TDEStartupInfoData data;
    startup_t ret = check_startup_internal( w_P, &id, &data );
    switch( ret )
        {
        case Match:
            kdDebug( 172 ) << "[tdecore-tdestartupinfo] new window match" << endl;
          break;
        case NoMatch:
          break; // nothing
        case CantDetect:
            if( d->flags & CleanOnCantDetect )
                clean_all_noncompliant();
          break;
        }
    }

void TDEStartupInfo::got_startup_info( const TQString& msg_P, bool update_P )
    {
    TDEStartupInfoId id( msg_P );
    if( id.none())
        return;
    TDEStartupInfo::Data data( msg_P );
    new_startup_info_internal( id, data, update_P );
    }

void TDEStartupInfo::new_startup_info_internal( const TDEStartupInfoId& id_P,
    Data& data_P, bool update_P )
    {
    if( d == NULL )
        return;
    if( id_P.none())
        return;
    if( d->startups.contains( id_P ))
        { // already reported, update
        d->startups[ id_P ].update( data_P );
        d->startups[ id_P ].age = 0; // CHECKME
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] updating" << endl;
	if( d->startups[ id_P ].silent() == Data::Yes
	    && !( d->flags & AnnounceSilenceChanges ))
	    {
	    d->silent_startups[ id_P ] = d->startups[ id_P ];
	    d->startups.remove( id_P );
	    emit gotRemoveStartup( id_P, d->silent_startups[ id_P ] );
	    return;
	    }
        emit gotStartupChange( id_P, d->startups[ id_P ] );
        return;
        }
    if( d->silent_startups.contains( id_P ))
        { // already reported, update
        d->silent_startups[ id_P ].update( data_P );
        d->silent_startups[ id_P ].age = 0; // CHECKME
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] updating silenced" << endl;
	if( d->silent_startups[ id_P ].silent() != Data::Yes )
	    {
	    d->startups[ id_P ] = d->silent_startups[ id_P ];
	    d->silent_startups.remove( id_P );
	    emit gotNewStartup( id_P, d->startups[ id_P ] );
	    return;
	    }
        emit gotStartupChange( id_P, d->silent_startups[ id_P ] );
        return;
        }
    if( d->uninited_startups.contains( id_P ))
        {
        d->uninited_startups[ id_P ].update( data_P );
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] updating uninited" << endl;
        if( !update_P ) // uninited finally got new:
            {
            d->startups[ id_P ] = d->uninited_startups[ id_P ];
            d->uninited_startups.remove( id_P );
            emit gotNewStartup( id_P, d->startups[ id_P ] );
            return;
            }
        // no change announce, it's still uninited
        return;
        }
    if( update_P ) // change: without any new: first
        {
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] adding uninited" << endl;
	d->uninited_startups.insert( id_P, data_P );
        }
    else if( data_P.silent() != Data::Yes || d->flags & AnnounceSilenceChanges )
	{
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] adding" << endl;
        d->startups.insert( id_P, data_P );
	emit gotNewStartup( id_P, data_P );
	}
    else // new silenced, and silent shouldn't be announced
	{
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] adding silent" << endl;
	d->silent_startups.insert( id_P, data_P );
	}
    d->cleanup->start( 1000 ); // 1 sec
    }

void TDEStartupInfo::got_remove_startup_info( const TQString& msg_P )
    {
    TDEStartupInfoId id( msg_P );
    TDEStartupInfoData data( msg_P );
    if( data.pids().count() > 0 )
        {
        if( !id.none())
            remove_startup_pids( id, data );
        else
            remove_startup_pids( data );
        return;
        }
    remove_startup_info_internal( id );
    }

void TDEStartupInfo::remove_startup_info_internal( const TDEStartupInfoId& id_P )
    {
    if( d == NULL )
        return;
    if( d->startups.contains( id_P ))
        {
	kdDebug( 172 ) << "[tdecore-tdestartupinfo] removing" << endl;
	emit gotRemoveStartup( id_P, d->startups[ id_P ]);
	d->startups.remove( id_P );
	}
    else if( d->silent_startups.contains( id_P ))
	{
	kdDebug( 172 ) << "[tdecore-tdestartupinfo] removing silent" << endl;
	d->silent_startups.remove( id_P );
	}
    else if( d->uninited_startups.contains( id_P ))
	{
	kdDebug( 172 ) << "[tdecore-tdestartupinfo] removing uninited" << endl;
	d->uninited_startups.remove( id_P );
	}
    return;
    }

void TDEStartupInfo::remove_startup_pids( const TDEStartupInfoData& data_P )
    { // first find the matching info
    if( d == NULL )
        return;
    for( TQMap< TDEStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         ++it )
        {
        if( ( *it ).hostname() != data_P.hostname())
            continue;
        if( !( *it ).is_pid( data_P.pids().first()))
            continue; // not the matching info
        remove_startup_pids( it.key(), data_P );
        break;
        }
    }

void TDEStartupInfo::remove_startup_pids( const TDEStartupInfoId& id_P,
    const TDEStartupInfoData& data_P )
    {
    if( d == NULL )
        return;
    kdFatal( data_P.pids().count() == 0, 172 );
    Data* data = NULL;
    if( d->startups.contains( id_P ))
	data = &d->startups[ id_P ];
    else if( d->silent_startups.contains( id_P ))
	data = &d->silent_startups[ id_P ];
    else if( d->uninited_startups.contains( id_P ))
        data = &d->uninited_startups[ id_P ];
    else
	return;
    for( TQValueList< pid_t >::ConstIterator it2 = data_P.pids().begin();
         it2 != data_P.pids().end();
         ++it2 )
	data->remove_pid( *it2 ); // remove all pids from the info
    if( data->pids().count() == 0 ) // all pids removed -> remove info
    	remove_startup_info_internal( id_P );
    }

bool TDEStartupInfo::sendStartup( const TDEStartupInfoId& id_P, const TDEStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    TQString msg = TQString::fromLatin1( "new: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
    msg = check_required_startup_fields( msg, data_P, tqt_xscreen());
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] sending " << msg << endl;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
    return true;
    }

bool TDEStartupInfo::sendStartupX( Display* disp_P, const TDEStartupInfoId& id_P,
    const TDEStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    TQString msg = TQString::fromLatin1( "new: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
    msg = check_required_startup_fields( msg, data_P, DefaultScreen( disp_P ));
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
    }

TQString TDEStartupInfo::check_required_startup_fields( const TQString& msg, const TDEStartupInfoData& data_P,
    int screen )
    {
    TQString ret = msg;
    if( data_P.name().isEmpty())
        {
//        kdWarning( 172 ) << "[tdecore-tdestartupinfo] NAME not specified in initial startup message" << endl;
        TQString name = data_P.bin();
        if( name.isEmpty())
            name = "UNKNOWN";
        ret += TQString( " NAME=\"%1\"" ).arg( escape_str( name ));
        }
    if( data_P.screen() == -1 ) // add automatically if needed
        ret += TQString( " SCREEN=%1" ).arg( screen );
    return ret;
    }

bool TDEStartupInfo::sendChange( const TDEStartupInfoId& id_P, const TDEStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    TQString msg = TQString::fromLatin1( "change: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] sending " << msg << endl;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
    return true;
    }

bool TDEStartupInfo::sendChangeX( Display* disp_P, const TDEStartupInfoId& id_P,
    const TDEStartupInfoData& data_P )
    {
    if( id_P.none())
        return false;
    TQString msg = TQString::fromLatin1( "change: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
    }

bool TDEStartupInfo::sendFinish( const TDEStartupInfoId& id_P )
    {
    if( id_P.none())
        return false;
    KXMessages msgs;
    TQString msg = TQString::fromLatin1( "remove: %1" ).arg( id_P.to_text());
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] sending " << msg << endl;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
    return true;
    }

bool TDEStartupInfo::sendFinishX( Display* disp_P, const TDEStartupInfoId& id_P )
    {
    if( id_P.none())
        return false;
    TQString msg = TQString::fromLatin1( "remove: %1" ).arg( id_P.to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
    }

bool TDEStartupInfo::sendFinish( const TDEStartupInfoId& id_P, const TDEStartupInfoData& data_P )
    {
//    if( id_P.none()) // id may be none, the pids and hostname matter then
//        return false;
    KXMessages msgs;
    TQString msg = TQString::fromLatin1( "remove: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] sending " << msg << endl;
    msgs.broadcastMessage( NET_STARTUP_MSG, msg, -1, false );
    return true;
    }

bool TDEStartupInfo::sendFinishX( Display* disp_P, const TDEStartupInfoId& id_P,
    const TDEStartupInfoData& data_P )
    {
//    if( id_P.none()) // id may be none, the pids and hostname matter then
//        return false;
    TQString msg = TQString::fromLatin1( "remove: %1 %2" )
        .arg( id_P.to_text()).arg( data_P.to_text());
#ifdef KSTARTUPINFO_ALL_DEBUG
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] sending " << msg << endl;
#endif
    return KXMessages::broadcastMessageX( disp_P, NET_STARTUP_MSG, msg, -1, false );
    }

void TDEStartupInfo::appStarted()
    {
    if( kapp != NULL )  // TDEApplication constructor unsets the env. variable
        appStarted( kapp->startupId());
    else
        appStarted( TDEStartupInfo::currentStartupIdEnv().id());
    }

void TDEStartupInfo::appStarted( const TQCString& startup_id )
    {
    TDEStartupInfoId id;
    id.initId( startup_id );
    if( id.none())
        return;
    if( kapp != NULL )
        TDEStartupInfo::sendFinish( id );
    else if( getenv( "DISPLAY" ) != NULL ) // don't rely on tqt_xdisplay()
        {
#ifdef TQ_WS_X11
        Display* disp = XOpenDisplay( NULL );
        if( disp != NULL )
            {
            TDEStartupInfo::sendFinishX( disp, id );
            XCloseDisplay( disp );
            }
#endif
        }
    }

void TDEStartupInfo::disableAutoAppStartedSending( bool disable )
    {
    auto_app_started_sending = !disable;
    }

void TDEStartupInfo::silenceStartup( bool silence )
    {
    TDEStartupInfoId id;
    id.initId( kapp->startupId());
    if( id.none())
        return;
    TDEStartupInfoData data;
    data.setSilent( silence ? TDEStartupInfoData::Yes : TDEStartupInfoData::No );
    sendChange( id, data );
    }

void TDEStartupInfo::handleAutoAppStartedSending()
    {
    if( auto_app_started_sending )
        appStarted();
    }

void TDEStartupInfo::setNewStartupId( TQWidget* window, const TQCString& startup_id )
    {
    bool activate = true;
    kapp->setStartupId( startup_id );
    if( window != NULL )
        {
        if( !startup_id.isEmpty() && startup_id != "0" )
            {
            NETRootInfo i( tqt_xdisplay(), NET::Supported );
            if( i.isSupported( NET::WM2StartupId ))
                {
                TDEStartupInfo::setWindowStartupId( window->winId(), startup_id );
                activate = false; // WM will take care of it
                }
            }
        if( activate )
            {
            KWin::setOnDesktop( window->winId(), KWin::currentDesktop());
        // This is not very nice, but there's no way how to get any
        // usable timestamp without ASN, so force activating the window.
        // And even with ASN, it's not possible to get the timestamp here,
        // so if the WM doesn't have support for ASN, it can't be used either.
            KWin::forceActiveWindow( window->winId());
            }
        }
    TDEStartupInfo::handleAutoAppStartedSending();
    }

TDEStartupInfo::startup_t TDEStartupInfo::checkStartup( WId w_P, TDEStartupInfoId& id_O,
    TDEStartupInfoData& data_O )
    {
    return check_startup_internal( w_P, &id_O, &data_O );
    }

TDEStartupInfo::startup_t TDEStartupInfo::checkStartup( WId w_P, TDEStartupInfoId& id_O )
    {
    return check_startup_internal( w_P, &id_O, NULL );
    }

TDEStartupInfo::startup_t TDEStartupInfo::checkStartup( WId w_P, TDEStartupInfoData& data_O )
    {
    return check_startup_internal( w_P, NULL, &data_O );
    }

TDEStartupInfo::startup_t TDEStartupInfo::checkStartup( WId w_P )
    {
    return check_startup_internal( w_P, NULL, NULL );
    }

TDEStartupInfo::startup_t TDEStartupInfo::check_startup_internal( WId w_P, TDEStartupInfoId* id_O,
    TDEStartupInfoData* data_O )
    {
    if( d == NULL )
        return NoMatch;
    if( d->startups.count() == 0 )
        return NoMatch; // no startups
    // Strategy:
    //
    // Is this a compliant app ?
    //  - Yes - test for match
    //  - No - Is this a NET_WM compliant app ?
    //           - Yes - test for pid match
    //           - No - test for WM_CLASS match
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] check_startup" << endl;
    TQCString id = windowStartupId( w_P );
    if( !id.isNull())
        {
        if( id.isEmpty() || id == "0" ) // means ignore this window
            {
            kdDebug( 172 ) << "[tdecore-tdestartupinfo] ignore" << endl;
            return NoMatch;
            }
        return find_id( id, id_O, data_O ) ? Match : NoMatch;
        }
#ifdef TQ_WS_X11
    NETWinInfo info( tqt_xdisplay(),  w_P, tqt_xrootwin(),
        NET::WMWindowType | NET::WMPid | NET::WMState );
    pid_t pid = info.pid();
    if( pid > 0 )
        {
        TQCString hostname = get_window_hostname( w_P );
        if( !hostname.isEmpty()
            && find_pid( pid, hostname, id_O, data_O ))
            return Match;
        // try XClass matching , this PID stuff sucks :(
        }
    XClassHint hint;
    if( XGetClassHint( tqt_xdisplay(), w_P, &hint ) != 0 )
        { // We managed to read the class hint
        TQCString res_name = hint.res_name;
        TQCString res_class = hint.res_class;
        XFree( hint.res_name );
        XFree( hint.res_class );
        if( find_wclass( res_name, res_class, id_O, data_O ))
            return Match;
        }
    // ignore NET::Tool and other special window types, if they can't be matched
    NET::WindowType type = info.windowType( NET::NormalMask | NET::DesktopMask
        | NET::DockMask | NET::ToolbarMask | NET::MenuMask | NET::DialogMask
        | NET::OverrideMask | NET::TopMenuMask | NET::UtilityMask | NET::SplashMask );
    if( type != NET::Normal
        && type != NET::Override
        && type != NET::Unknown
        && type != NET::Dialog
        && type != NET::Utility )
//        && type != NET::Dock ) why did I put this here?
	return NoMatch;
    // lets see if this is a transient
    Window transient_for;
    if( XGetTransientForHint( tqt_xdisplay(), static_cast< Window >( w_P ), &transient_for )
        && static_cast< WId >( transient_for ) != tqt_xrootwin()
        && transient_for != None )
	return NoMatch;
#endif
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] check_startup:cantdetect" << endl;
    return CantDetect;
    }

bool TDEStartupInfo::find_id( const TQCString& id_P, TDEStartupInfoId* id_O,
    TDEStartupInfoData* data_O )
    {
    if( d == NULL )
        return false;
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] find_id:" << id_P << endl;
    TDEStartupInfoId id;
    id.initId( id_P );
    if( d->startups.contains( id ))
        {
        if( id_O != NULL )
            *id_O = id;
        if( data_O != NULL )
            *data_O = d->startups[ id ];
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] check_startup_id:match" << endl;
        return true;
        }
    return false;
    }

bool TDEStartupInfo::find_pid( pid_t pid_P, const TQCString& hostname_P,
    TDEStartupInfoId* id_O, TDEStartupInfoData* data_O )
    {
    if( d == NULL )
        return false;
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] find_pid:" << pid_P << endl;
    for( TQMap< TDEStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         ++it )
        {
        if( ( *it ).is_pid( pid_P ) && ( *it ).hostname() == hostname_P )
            { // Found it !
            if( id_O != NULL )
                *id_O = it.key();
            if( data_O != NULL )
                *data_O = *it;
            // non-compliant, remove on first match
            remove_startup_info_internal( it.key());
            kdDebug( 172 ) << "[tdecore-tdestartupinfo] check_startup_pid:match" << endl;
            return true;
            }
        }
    return false;
    }

bool TDEStartupInfo::find_wclass( TQCString res_name, TQCString res_class,
    TDEStartupInfoId* id_O, TDEStartupInfoData* data_O )
    {
    if( d == NULL )
        return false;
    res_name = res_name.lower();
    res_class = res_class.lower();
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] find_wclass:" << res_name << ":" << res_class << endl;
    for( TQMap< TDEStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         ++it )
        {
        const TQCString wmclass = ( *it ).findWMClass();
        if( wmclass.lower() == res_name || wmclass.lower() == res_class )
            { // Found it !
            if( id_O != NULL )
                *id_O = it.key();
            if( data_O != NULL )
                *data_O = *it;
            // non-compliant, remove on first match
            remove_startup_info_internal( it.key());
            kdDebug( 172 ) << "[tdecore-tdestartupinfo] check_startup_wclass:match" << endl;
            return true;
            }
        }
    return false;
    }

#ifdef TQ_WS_X11
static Atom net_startup_atom = None;

static TQCString read_startup_id_property( WId w_P )
    {
    TQCString ret;
    unsigned char *name_ret;
    Atom type_ret;
    int format_ret;
    unsigned long nitems_ret = 0, after_ret = 0;
    if( XGetWindowProperty( tqt_xdisplay(), w_P, net_startup_atom, 0l, 4096,
            False, utf8_string_atom, &type_ret, &format_ret, &nitems_ret, &after_ret, &name_ret )
	    == Success )
        {
	if( type_ret == utf8_string_atom && format_ret == 8 && name_ret != NULL )
  	    ret = reinterpret_cast< char* >( name_ret );
        if ( name_ret != NULL )
            XFree( name_ret );
        }
    return ret;
    }

#endif

TQCString TDEStartupInfo::windowStartupId( WId w_P )
    {
#ifdef TQ_WS_X11
    if( net_startup_atom == None )
        net_startup_atom = XInternAtom( tqt_xdisplay(), NET_STARTUP_WINDOW, False );
    if( utf8_string_atom == None )
        utf8_string_atom = XInternAtom( tqt_xdisplay(), "UTF8_STRING", False );
    TQCString ret = read_startup_id_property( w_P );
    if( ret.isEmpty())
        { // retry with window group leader, as the spec says
        XWMHints* hints = XGetWMHints( tqt_xdisplay(), w_P );
        if( hints && ( hints->flags & WindowGroupHint ) != 0 )
            ret = read_startup_id_property( hints->window_group );
        if( hints )
            XFree( hints );
        }
    return ret;
#else
    return TQCString();
#endif
    }

void TDEStartupInfo::setWindowStartupId( WId w_P, const TQCString& id_P )
    {
#ifdef TQ_WS_X11
    if( id_P.isNull())
        return;
    if( net_startup_atom == None )
        net_startup_atom = XInternAtom( tqt_xdisplay(), NET_STARTUP_WINDOW, False );
    if( utf8_string_atom == None )
        utf8_string_atom = XInternAtom( tqt_xdisplay(), "UTF8_STRING", False );
    XChangeProperty( tqt_xdisplay(), w_P, net_startup_atom, utf8_string_atom, 8,
        PropModeReplace, reinterpret_cast< unsigned char* >( const_cast<TQCString&>(id_P).data()), id_P.length());
#endif
    }

TQCString TDEStartupInfo::get_window_hostname( WId w_P )
    {
#ifdef TQ_WS_X11
    XTextProperty tp;
    char** hh;
    int cnt;
    if( XGetWMClientMachine( tqt_xdisplay(), w_P, &tp ) != 0
        && XTextPropertyToStringList( &tp, &hh, &cnt ) != 0 )
        {
        if( cnt == 1 )
            {
            TQCString hostname = hh[ 0 ];
            XFreeStringList( hh );
            return hostname;
            }
        XFreeStringList( hh );
        }
#endif
    // no hostname
    return TQCString();
    }

void TDEStartupInfo::setTimeout( unsigned int secs_P )
    {
    timeout = secs_P;
 // schedule removing entries that are older than the new timeout
    TQTimer::singleShot( 0, this, TQ_SLOT( startups_cleanup_no_age()));
    }

void TDEStartupInfo::startups_cleanup_no_age()
    {
    startups_cleanup_internal( false );
    }

void TDEStartupInfo::startups_cleanup()
    {
    if( d == NULL )
        return;
    if( d->startups.count() == 0 && d->silent_startups.count() == 0
        && d->uninited_startups.count() == 0 )
        {
        d->cleanup->stop();
        return;
        }
    startups_cleanup_internal( true );
    }

void TDEStartupInfo::startups_cleanup_internal( bool age_P )
    {
    if( d == NULL )
        return;
    for( TQMap< TDEStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         )
        {
        if( age_P )
            ( *it ).age++;
	unsigned int tout = timeout;
	if( ( *it ).silent() == Data::Yes ) // TODO
	    tout *= 20;
        if( ( *it ).age >= tout )
            {
            const TDEStartupInfoId& key = it.key();
            ++it;
            kdDebug( 172 ) << "[tdecore-tdestartupinfo] startups entry timeout:" << key.id() << endl;
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    for( TQMap< TDEStartupInfoId, Data >::Iterator it = d->silent_startups.begin();
         it != d->silent_startups.end();
         )
        {
        if( age_P )
            ( *it ).age++;
	unsigned int tout = timeout;
	if( ( *it ).silent() == Data::Yes ) // TODO
	    tout *= 20;
        if( ( *it ).age >= tout )
            {
            const TDEStartupInfoId& key = it.key();
            ++it;
            kdDebug( 172 ) << "[tdecore-tdestartupinfo] silent entry timeout:" << key.id() << endl;
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    for( TQMap< TDEStartupInfoId, Data >::Iterator it = d->uninited_startups.begin();
         it != d->uninited_startups.end();
         )
        {
        if( age_P )
            ( *it ).age++;
	unsigned int tout = timeout;
	if( ( *it ).silent() == Data::Yes ) // TODO
	    tout *= 20;
        if( ( *it ).age >= tout )
            {
            const TDEStartupInfoId& key = it.key();
            ++it;
            kdDebug( 172 ) << "[tdecore-tdestartupinfo] uninited entry timeout:" << key.id() << endl;
            remove_startup_info_internal( key );
            }
        else
            ++it;
        }
    }

void TDEStartupInfo::clean_all_noncompliant()
    {
    if( d == NULL )
        return;
    for( TQMap< TDEStartupInfoId, Data >::Iterator it = d->startups.begin();
         it != d->startups.end();
         )
        {
        if( ( *it ).WMClass() != "0" )
            {
            ++it;
            continue;
            }
        const TDEStartupInfoId& key = it.key();
        ++it;
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] entry cleaning:" << key.id() << endl;
        remove_startup_info_internal( key );
        }
    }

TQCString TDEStartupInfo::createNewStartupId()
    {
    // Assign a unique id, use hostname+time+pid, that should be 200% unique.
    // Also append the user timestamp (for focus stealing prevention).
    struct timeval tm;
    gettimeofday( &tm, NULL );
    char hostname[ 256 ];
    hostname[ 0 ] = '\0';
    if (!gethostname( hostname, 255 ))
	hostname[sizeof(hostname)-1] = '\0';
    TQCString id = TQString(TQString( "%1;%2;%3;%4_TIME%5" ).arg( hostname ).arg( tm.tv_sec )
        .arg( tm.tv_usec ).arg( getpid()).arg( get_tqt_x_user_time() )).utf8();
    kdDebug( 172 ) << "[tdecore-tdestartupinfo] creating: " << id << ":" << tqAppName() << endl;
    return id;
    }


struct TDEStartupInfoIdPrivate
    {
    TDEStartupInfoIdPrivate() : id( "" ) {}
    TQCString id; // id
    };

const TQCString& TDEStartupInfoId::id() const
    {
    return d->id;
    }


TQString TDEStartupInfoId::to_text() const
    {
    return TQString::fromLatin1( " ID=\"%1\" " ).arg( escape_str( id()));
    }

TDEStartupInfoId::TDEStartupInfoId( const TQString& txt_P )
    {
    d = new TDEStartupInfoIdPrivate;
    TQStringList items = get_fields( txt_P );
    const TQString id_str = TQString::fromLatin1( "ID=" );
    for( TQStringList::Iterator it = items.begin();
         it != items.end();
         ++it )
        {
        if( ( *it ).startsWith( id_str ))
            d->id = get_cstr( *it );
        }
    }

void TDEStartupInfoId::initId( const TQCString& id_P )
    {
    if( !id_P.isEmpty())
        {
        d->id = id_P;
#ifdef KSTARTUPINFO_ALL_DEBUG
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] using: " << d->id << endl;
#endif
        return;
        }
    const char* startup_env = getenv( NET_STARTUP_ENV );
    if( startup_env != NULL && *startup_env != '\0' )
        { // already has id
        d->id = startup_env;
#ifdef KSTARTUPINFO_ALL_DEBUG
        kdDebug( 172 ) << "[tdecore-tdestartupinfo] reusing: " << d->id << endl;
#endif
        return;
        }
    d->id = TDEStartupInfo::createNewStartupId();
    }

bool TDEStartupInfoId::setupStartupEnv() const
    {
    if( id().isEmpty())
        {
        unsetenv( NET_STARTUP_ENV );
        return false;
        }
    return setenv( NET_STARTUP_ENV, id(), true ) == 0;
    }

TDEStartupInfoId TDEStartupInfo::currentStartupIdEnv()
    {
    const char* startup_env = getenv( NET_STARTUP_ENV );
    TDEStartupInfoId id;
    if( startup_env != NULL && *startup_env != '\0' )
        id.d->id = startup_env;
    else
        id.d->id = "0";
    return id;
    }

void TDEStartupInfo::resetStartupEnv()
    {
    unsetenv( NET_STARTUP_ENV );
    }

TDEStartupInfoId::TDEStartupInfoId()
    {
    d = new TDEStartupInfoIdPrivate;
    }

TDEStartupInfoId::~TDEStartupInfoId()
    {
    delete d;
    }

TDEStartupInfoId::TDEStartupInfoId( const TDEStartupInfoId& id_P )
    {
    d = new TDEStartupInfoIdPrivate( *id_P.d );
    }

TDEStartupInfoId& TDEStartupInfoId::operator=( const TDEStartupInfoId& id_P )
    {
    if( &id_P == this )
        return *this;
    delete d;
    d = new TDEStartupInfoIdPrivate( *id_P.d );
    return *this;
    }

bool TDEStartupInfoId::operator==( const TDEStartupInfoId& id_P ) const
    {
    return id() == id_P.id();
    }

bool TDEStartupInfoId::operator!=( const TDEStartupInfoId& id_P ) const
    {
    return !(*this == id_P );
    }

// needed for QMap
bool TDEStartupInfoId::operator<( const TDEStartupInfoId& id_P ) const
    {
    return id() < id_P.id();
    }

bool TDEStartupInfoId::none() const
    {
    return d->id.isEmpty() || d->id == "0";
    }

unsigned long TDEStartupInfoId::timestamp() const
    {
    if( none())
        return 0;
    int pos = d->id.findRev( "_TIME" );
    if( pos >= 0 )
        {
        bool ok;
        unsigned long time = d->id.mid( pos + 5 ).toULong( &ok );
        if( !ok && d->id[ pos + 5 ] == '-' ) // try if it's as a negative signed number perhaps
            time = d->id.mid( pos + 5 ).toLong( &ok );
        if( ok )
            return time;
        }
    // libstartup-notification style :
    // snprintf (s, len, "%s/%s/%lu/%d-%d-%s",
    //   canonicalized_launcher, canonicalized_launchee, (unsigned long) timestamp,
    //  (int) getpid (), (int) sequence_number, hostbuf);
    int pos1 = d->id.findRev( '/' );
    if( pos1 > 0 )
        {
        int pos2 = d->id.findRev( '/', pos1 - 1 );
        if( pos2 >= 0 )
            {
            bool ok;
            unsigned long time = d->id.mid( pos2 + 1, pos1 - pos2 - 1 ).toULong( &ok );
            if( !ok && d->id[ pos2 + 1 ] == '-' ) // try if it's as a negative signed number perhaps
                time = d->id.mid( pos2 + 1, pos1 - pos2 - 1 ).toLong( &ok );
            if( ok )
                return time;
            }
        }
    // bah ... old TDEStartupInfo or a problem
    return 0;
    }

struct TDEStartupInfoDataPrivate
    {
    TDEStartupInfoDataPrivate() : desktop( 0 ), wmclass( "" ), hostname( "" ),
	silent( TDEStartupInfoData::Unknown ), timestamp( -1U ), screen( -1 ), xinerama( -1 ), launched_by( 0 ) {}
    TQString bin;
    TQString name;
    TQString description;
    TQString icon;
    int desktop;
    TQValueList< pid_t > pids;
    TQCString wmclass;
    TQCString hostname;
    TDEStartupInfoData::TriState silent;
    unsigned long timestamp;
    int screen;
    int xinerama;
    WId launched_by;
    };

TQString TDEStartupInfoData::to_text() const
    {
    TQString ret = "";
    if( !d->bin.isEmpty())
        ret += TQString::fromLatin1( " BIN=\"%1\"" ).arg( escape_str( d->bin ));
    if( !d->name.isEmpty())
        ret += TQString::fromLatin1( " NAME=\"%1\"" ).arg( escape_str( d->name ));
    if( !d->description.isEmpty())
        ret += TQString::fromLatin1( " DESCRIPTION=\"%1\"" ).arg( escape_str( d->description ));
    if( !d->icon.isEmpty())
        ret += TQString::fromLatin1( " ICON=%1" ).arg( d->icon );
    if( d->desktop != 0 )
        ret += TQString::fromLatin1( " DESKTOP=%1" )
            .arg( d->desktop == NET::OnAllDesktops ? NET::OnAllDesktops : d->desktop - 1 ); // spec counts from 0
    if( !d->wmclass.isEmpty())
        ret += TQString::fromLatin1( " WMCLASS=\"%1\"" ).arg( TQString(d->wmclass) );
    if( !d->hostname.isEmpty())
        ret += TQString::fromLatin1( " HOSTNAME=%1" ).arg( TQString(d->hostname) );
    for( TQValueList< pid_t >::ConstIterator it = d->pids.begin();
         it != d->pids.end();
         ++it )
        ret += TQString::fromLatin1( " PID=%1" ).arg( *it );
    if( d->silent != Unknown )
	ret += TQString::fromLatin1( " SILENT=%1" ).arg( d->silent == Yes ? 1 : 0 );
    if( d->timestamp != -1U )
        ret += TQString::fromLatin1( " TIMESTAMP=%1" ).arg( d->timestamp );
    if( d->screen != -1 )
        ret += TQString::fromLatin1( " SCREEN=%1" ).arg( d->screen );
    if( d->xinerama != -1 )
        ret += TQString::fromLatin1( " XINERAMA=%1" ).arg( d->xinerama );
    if( d->launched_by != 0 )
        ret += TQString::fromLatin1( " LAUNCHED_BY=%1" ).arg( d->launched_by );
    return ret;
    }

TDEStartupInfoData::TDEStartupInfoData( const TQString& txt_P )
    {
    d = new TDEStartupInfoDataPrivate;
    TQStringList items = get_fields( txt_P );
    const TQString bin_str = TQString::fromLatin1( "BIN=" );
    const TQString name_str = TQString::fromLatin1( "NAME=" );
    const TQString description_str = TQString::fromLatin1( "DESCRIPTION=" );
    const TQString icon_str = TQString::fromLatin1( "ICON=" );
    const TQString desktop_str = TQString::fromLatin1( "DESKTOP=" );
    const TQString wmclass_str = TQString::fromLatin1( "WMCLASS=" );
    const TQString hostname_str = TQString::fromLatin1( "HOSTNAME=" ); // SELI nonstd
    const TQString pid_str = TQString::fromLatin1( "PID=" );  // SELI nonstd
    const TQString silent_str = TQString::fromLatin1( "SILENT=" );
    const TQString timestamp_str = TQString::fromLatin1( "TIMESTAMP=" );
    const TQString screen_str = TQString::fromLatin1( "SCREEN=" );
    const TQString xinerama_str = TQString::fromLatin1( "XINERAMA=" );
    const TQString launched_by_str = TQString::fromLatin1( "LAUNCHED_BY=" );
    for( TQStringList::Iterator it = items.begin();
         it != items.end();
         ++it )
        {
        if( ( *it ).startsWith( bin_str ))
            d->bin = get_str( *it );
        else if( ( *it ).startsWith( name_str ))
            d->name = get_str( *it );
        else if( ( *it ).startsWith( description_str ))
            d->description = get_str( *it );
        else if( ( *it ).startsWith( icon_str ))
            d->icon = get_str( *it );
        else if( ( *it ).startsWith( desktop_str ))
            {
            d->desktop = get_num( *it );
            if( d->desktop != NET::OnAllDesktops )
                ++d->desktop; // spec counts from 0
            }
        else if( ( *it ).startsWith( wmclass_str ))
            d->wmclass = get_cstr( *it );
        else if( ( *it ).startsWith( hostname_str ))
            d->hostname = get_cstr( *it );
        else if( ( *it ).startsWith( pid_str ))
            addPid( get_num( *it ));
        else if( ( *it ).startsWith( silent_str ))
            d->silent = get_num( *it ) != 0 ? Yes : No;
        else if( ( *it ).startsWith( timestamp_str ))
            d->timestamp = get_unum( *it );
        else if( ( *it ).startsWith( screen_str ))
            d->screen = get_num( *it );
        else if( ( *it ).startsWith( xinerama_str ))
            d->xinerama = get_num( *it );
        else if( ( *it ).startsWith( launched_by_str ))
            d->launched_by = get_num( *it );
        }
    }

TDEStartupInfoData::TDEStartupInfoData( const TDEStartupInfoData& data )
{
    d = new TDEStartupInfoDataPrivate( *data.d );
}

TDEStartupInfoData& TDEStartupInfoData::operator=( const TDEStartupInfoData& data )
{
    if( &data == this )
        return *this;
    delete d;
    d = new TDEStartupInfoDataPrivate( *data.d );
    return *this;
}

void TDEStartupInfoData::update( const TDEStartupInfoData& data_P )
    {
    if( !data_P.bin().isEmpty())
        d->bin = data_P.bin();
    if( !data_P.name().isEmpty() && name().isEmpty()) // don't overwrite
        d->name = data_P.name();
    if( !data_P.description().isEmpty() && description().isEmpty()) // don't overwrite
        d->description = data_P.description();
    if( !data_P.icon().isEmpty() && icon().isEmpty()) // don't overwrite
        d->icon = data_P.icon();
    if( data_P.desktop() != 0 && desktop() == 0 ) // don't overwrite
        d->desktop = data_P.desktop();
    if( !data_P.d->wmclass.isEmpty())
        d->wmclass = data_P.d->wmclass;
    if( !data_P.d->hostname.isEmpty())
        d->hostname = data_P.d->hostname;
    for( TQValueList< pid_t >::ConstIterator it = data_P.d->pids.begin();
         it != data_P.d->pids.end();
         ++it )
        addPid( *it );
    if( data_P.silent() != Unknown )
	d->silent = data_P.silent();
    if( data_P.timestamp() != -1U && timestamp() == -1U ) // don't overwrite
        d->timestamp = data_P.timestamp();
    if( data_P.screen() != -1 )
        d->screen = data_P.screen();
    if( data_P.xinerama() != -1 && xinerama() != -1 ) // don't overwrite
        d->xinerama = data_P.xinerama();
    if( data_P.launchedBy() != 0 && launchedBy() != 0 ) // don't overwrite
        d->launched_by = data_P.launchedBy();
    }

TDEStartupInfoData::TDEStartupInfoData()
{
    d = new TDEStartupInfoDataPrivate;
}

TDEStartupInfoData::~TDEStartupInfoData()
{
    delete d;
}

void TDEStartupInfoData::setBin( const TQString& bin_P )
    {
    d->bin = bin_P;
    }

const TQString& TDEStartupInfoData::bin() const
    {
    return d->bin;
    }

void TDEStartupInfoData::setName( const TQString& name_P )
    {
    d->name = name_P;
    }

const TQString& TDEStartupInfoData::name() const
    {
    return d->name;
    }

const TQString& TDEStartupInfoData::findName() const
    {
    if( !name().isEmpty())
        return name();
    return bin();
    }

void TDEStartupInfoData::setDescription( const TQString& desc_P )
    {
    d->description = desc_P;
    }

const TQString& TDEStartupInfoData::description() const
    {
    return d->description;
    }

const TQString& TDEStartupInfoData::findDescription() const
    {
    if( !description().isEmpty())
        return description();
    return name();
    }

void TDEStartupInfoData::setIcon( const TQString& icon_P )
    {
    d->icon = icon_P;
    }

const TQString& TDEStartupInfoData::findIcon() const
    {
    if( !icon().isEmpty())
        return icon();
    return bin();
    }

const TQString& TDEStartupInfoData::icon() const
    {
    return d->icon;
    }

void TDEStartupInfoData::setDesktop( int desktop_P )
    {
    d->desktop = desktop_P;
    }

int TDEStartupInfoData::desktop() const
    {
    return d->desktop;
    }

void TDEStartupInfoData::setWMClass( const TQCString& wmclass_P )
    {
    d->wmclass = wmclass_P;
    }

const TQCString TDEStartupInfoData::findWMClass() const
    {
    if( !WMClass().isEmpty() && WMClass() != "0" )
        return WMClass();
    return bin().utf8();
    }

const TQCString& TDEStartupInfoData::WMClass() const
    {
    return d->wmclass;
    }

void TDEStartupInfoData::setHostname( const TQCString& hostname_P )
    {
    if( !hostname_P.isNull())
        d->hostname = hostname_P;
    else
        {
        char tmp[ 256 ];
        tmp[ 0 ] = '\0';
        if (!gethostname( tmp, 255 ))
	    tmp[sizeof(tmp)-1] = '\0';
        d->hostname = tmp;
        }
    }

const TQCString& TDEStartupInfoData::hostname() const
    {
    return d->hostname;
    }

void TDEStartupInfoData::addPid( pid_t pid_P )
    {
    if( !d->pids.contains( pid_P ))
        d->pids.append( pid_P );
    }

void TDEStartupInfoData::remove_pid( pid_t pid_P )
    {
    d->pids.remove( pid_P );
    }

const TQValueList< pid_t >& TDEStartupInfoData::pids() const
    {
    return d->pids;
    }

bool TDEStartupInfoData::is_pid( pid_t pid_P ) const
    {
    return d->pids.contains( pid_P );
    }

void TDEStartupInfoData::setSilent( TriState state_P )
    {
    d->silent = state_P;
    }

TDEStartupInfoData::TriState TDEStartupInfoData::silent() const
    {
    return d->silent;
    }

void TDEStartupInfoData::setTimestamp( unsigned long time )
    {
    d->timestamp = time;
    }

unsigned long TDEStartupInfoData::timestamp() const
    {
    return d->timestamp;
    }

void TDEStartupInfoData::setScreen( int screen )
    {
    d->screen = screen;
    }

int TDEStartupInfoData::screen() const
    {
    return d->screen;
    }

void TDEStartupInfoData::setXinerama( int xinerama )
    {
    d->xinerama = xinerama;
    }

int TDEStartupInfoData::xinerama() const
    {
    return d->xinerama;
    }

void TDEStartupInfoData::setLaunchedBy( WId window )
    {
    d->launched_by = window;
    }

WId TDEStartupInfoData::launchedBy() const
    {
    return d->launched_by;
    }

static
long get_num( const TQString& item_P )
    {
    unsigned int pos = item_P.find( '=' );
    return item_P.mid( pos + 1 ).toLong();
    }

static
unsigned long get_unum( const TQString& item_P )
    {
    unsigned int pos = item_P.find( '=' );
    return item_P.mid( pos + 1 ).toULong();
    }

static
TQString get_str( const TQString& item_P )
    {
    unsigned int pos = item_P.find( '=' );
    if( item_P.length() > pos + 2 && item_P[ pos + 1 ] == (TQChar)'\"' )
        {
        int pos2 = item_P.left( pos + 2 ).find( '\"' );
        if( pos2 < 0 )
            return TQString::null;                      // 01234
        return item_P.mid( pos + 2, pos2 - 2 - pos );  // A="C"
        }
    return item_P.mid( pos + 1 );
    }

static
TQCString get_cstr( const TQString& item_P )
    {
    return get_str( item_P ).utf8();
    }

static
TQStringList get_fields( const TQString& txt_P )
    {
    TQString txt = txt_P.simplifyWhiteSpace();
    TQStringList ret;
    TQString item = "";
    bool in = false;
    bool escape = false;
    for( unsigned int pos = 0;
         pos < txt.length();
         ++pos )
        {
        if( escape )
            {
            item += txt[ pos ];
            escape = false;
            }
        else if( txt[ pos ] == '\\' )
            escape = true;
        else if( txt[ pos ] == '\"' )
            in = !in;
        else if( txt[ pos ] == ' ' && !in )
            {
            ret.append( item );
            item = "";
            }
        else
            item += txt[ pos ];
        }
    ret.append( item );
    return ret;
    }

static TQString escape_str( const TQString& str_P )
    {
    TQString ret = "";
    for( unsigned int pos = 0;
	 pos < str_P.length();
	 ++pos )
	{
	if( str_P[ pos ] == (TQChar)'\\'
	    || str_P[ pos ] == (TQChar)'"' )
	    ret += '\\';
	ret += str_P[ pos ];
	}
    return ret;
    }

#include "tdestartupinfo.moc"
#endif
