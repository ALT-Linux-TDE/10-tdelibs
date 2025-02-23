/*
 *   copyright            : (C) 2001-2002 by Richard Moore
 *   License              : This file is released under the terms of the LGPL, version 2.
 *   email                : rich@kde.org
 */

#include <tqobjectlist.h>
#include <tqpixmap.h>
#include <tqtimer.h>
#include <tqtooltip.h>
#include <ksystemtray.h>
#include <twin.h>

#include "twindowinfo.h"
#include "twindowinfo.moc"

static const int UNSPECIFIED_TIMEOUT = -1;
static const int DEFAULT_MESSAGE_TIMEOUT = 3000;

KWindowInfo::KWindowInfo( TQWidget *parent, const char *name )
    : TQObject( parent, name ), win( parent ), autoDel( false )
{
}

KWindowInfo::~KWindowInfo()
{
}

void KWindowInfo::showMessage( TQWidget *window, const TQString &text, int timeout )
{
    KWindowInfo *info = new KWindowInfo( window );
    info->autoDel = true;
    info->message( text, timeout );
    if ( timeout == 0 )
	delete info;
}

void KWindowInfo::showMessage( TQWidget *window, const TQString &text, const TQPixmap &pix, int timeout )
{
    KWindowInfo *info = new KWindowInfo( window );
    info->autoDel = true;
    info->message( text, pix, timeout );
}

void KWindowInfo::message( const TQString &text )
{
    message( text, TQPixmap(), UNSPECIFIED_TIMEOUT );
}

void KWindowInfo::message( const TQString &text, const TQPixmap &pix )
{
    message( text, pix, UNSPECIFIED_TIMEOUT );
}

void KWindowInfo::message( const TQString &text, int timeout )
{
    message( text, TQPixmap(), timeout );
}

void KWindowInfo::message( const TQString &text, const TQPixmap &pix, int timeout )
{
    if ( timeout != 0 )
	save();

    display( text, pix );

    if ( timeout < 0 )
	timeout = DEFAULT_MESSAGE_TIMEOUT;
    if ( timeout != 0 )
	TQTimer::singleShot( timeout, this, TQ_SLOT( restore() ) );
}

void KWindowInfo::permanent( const TQString &text )
{
#ifdef TQ_WS_X11
    oldMiniIcon = KWin::icon( win->winId(), 16, 16, true );
    oldIcon = KWin::icon( win->winId(), 34, 34, false );
    if ( oldIcon.isNull() )
	oldIcon = KWin::icon( win->winId(), 32, 32, true );
#endif

    permanent( text, oldIcon );
}

void KWindowInfo::permanent( const TQString &text, const TQPixmap &pix )
{
    if ( !oldText.isNull() ) {
	TQObjectList *l = queryList( "TQTimer" );
	TQObjectListIt it( *l );
	TQObject *obj;

	while ( (obj = it.current()) != 0 ) {
	    ++it;
	    delete obj;
	}
	delete l;
    }

    oldText = TQString::null;
    display( text, pix );
}

void KWindowInfo::display( const TQString &text, const TQPixmap &pix )
{
    TQPixmap icon;
    if ( pix.isNull() )
	icon.load( "bell.png" );
    else
	icon = pix;

    if ( win->inherits( "KSystemTray" ) ) {
	KSystemTray *tray = static_cast<KSystemTray *>( win );
	tray->setPixmap( icon );
	TQToolTip::add( tray, text );
	return;
    }

    win->setCaption( text );
    win->setIcon( icon );
#ifdef TQ_WS_X11
    KWin::setIcons( win->winId(), icon, icon );
#endif
}

void KWindowInfo::save()
{
    if ( !oldText.isNull() )
	return;

    if ( win->inherits( "KSystemTray" ) ) {
	KSystemTray *tray = static_cast<KSystemTray *>( win );
	oldIcon = *(tray->pixmap());
	oldText = TQToolTip::textFor( tray );
	return;
    }

    oldText = win->caption();
#ifdef TQ_WS_X11
    oldMiniIcon = KWin::icon( win->winId(), 16, 16, true );
    oldIcon = KWin::icon( win->winId(), 34, 34, false );
    if ( oldIcon.isNull() )
	oldIcon = KWin::icon( win->winId(), 32, 32, true );
#endif

    if ( oldIcon.isNull() ) {
	const TQPixmap *px = win->icon();
	if ( px )
	    oldIcon = *px;
	else
	    oldIcon.resize( 0, 0 );
    }
}

void KWindowInfo::restore()
{
    if ( win->inherits( "KSystemTray" ) ) {
	KSystemTray *tray = static_cast<KSystemTray *>( win );
	tray->setPixmap( oldIcon );
	TQToolTip::add( tray, oldText );
	oldText = TQString::null;
	return;
    }

    win->setIcon( oldIcon );
#ifdef TQ_WS_X11
    KWin::setIcons( win->winId(), oldIcon, oldMiniIcon );
#endif
    win->setCaption( oldText );
    oldText = TQString::null;

    if ( autoDel )
	delete this;
}





