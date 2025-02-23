/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Richard Moore <rich@kde.org>
 *               2000 Wynn Wilkes <wynnw@caldera.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kjavaappletwidget.h"
#include "kjavaappletcontext.h"

#include <tdelocale.h>
#include <kdebug.h>
#include <tdeparts/browserextension.h>



class KJavaAppletPrivate
{
public:
   bool    reallyExists;
   TQString className;
   TQString appName;
   TQString baseURL;
   TQString codeBase;
   TQString archives;
   TQSize   size;
   TQString windowName;
   KJavaApplet::AppletState state;
   bool    failed;

   KJavaAppletWidget* UIwidget;
};


KJavaApplet::KJavaApplet( KJavaAppletWidget* _parent,
                          KJavaAppletContext* _context )
    : params()
{
    d = new KJavaAppletPrivate;

    d->UIwidget = _parent;
    d->state = UNKNOWN;
    d->failed = false;

    if( _context )
        setAppletContext( _context );

    d->reallyExists = false;
}

KJavaApplet::~KJavaApplet()
{
    if ( d->reallyExists )
        context->destroy( this );

    delete d;
}

bool KJavaApplet::isCreated()
{
    return d->reallyExists;
}

void KJavaApplet::setAppletContext( KJavaAppletContext* _context )
{
    context = _context;
    context->registerApplet( this );
}

void KJavaApplet::setAppletClass( const TQString& _className )
{
    d->className = _className;
}

TQString& KJavaApplet::appletClass()
{
    return d->className;
}

TQString& KJavaApplet::parameter( const TQString& name )
{
    return params[ name ];
}

void KJavaApplet::setParameter( const TQString& name, const TQString& value )
{
    params.insert( name, value );
}

TQMap<TQString,TQString>& KJavaApplet::getParams()
{
    return params;
}

void KJavaApplet::setBaseURL( const TQString& baseURL )
{
    d->baseURL = baseURL;
}

TQString& KJavaApplet::baseURL()
{
    return d->baseURL;
}

void KJavaApplet::setCodeBase( const TQString& codeBase )
{
    d->codeBase = codeBase;
}

TQString& KJavaApplet::codeBase()
{
    return d->codeBase;
}

void KJavaApplet::setSize( TQSize size )
{
    d->size = size;
}

TQSize KJavaApplet::size()
{
    return d->size;
}

void KJavaApplet::setArchives( const TQString& _archives )
{
    d->archives = _archives;
}

TQString& KJavaApplet::archives()
{
    return d->archives;
}

void KJavaApplet::resizeAppletWidget( int width, int height )
{
    kdDebug(6100) << "KJavaApplet, id = " << id << ", ::resizeAppletWidget to " << width << ", " << height << endl;

    TQStringList sl;
    sl.push_back( TQString::number( 0 ) ); // applet itself has id 0
    sl.push_back( TQString( "eval" ) );    // evaluate next script
    sl.push_back( TQString::number( KParts::LiveConnectExtension::TypeString ) );
    sl.push_back( TQString( "this.setAttribute('WIDTH',%1);this.setAttribute('HEIGHT',%2)" ).arg( width ).arg( height ) );
    jsData( sl );
}

void KJavaApplet::setAppletName( const TQString& name )
{
    d->appName = name;
}

void KJavaApplet::setWindowName( const TQString& title )
{
    d->windowName = title;
}

TQString& KJavaApplet::getWindowName()
{
    return d->windowName;
}

TQString& KJavaApplet::appletName()
{
    return d->appName;
}

void KJavaApplet::create( )
{
    if (  !context->create( this ) )
        setFailed();
    d->reallyExists = true;
}

void KJavaApplet::init()
{
    context->init( this );
}

void KJavaApplet::start()
{
    context->start( this );
}

void KJavaApplet::stop()
{
    context->stop( this );
}

int KJavaApplet::appletId()
{
    return id;
}

void KJavaApplet::setAppletId( int _id )
{
    id = _id;
}

void KJavaApplet::stateChange( const int newStateInt ) {
    AppletState newState = (AppletState)newStateInt;
    bool ok = false;
    if (d->failed) {
        return;
    }
    switch ( newState ) {
        case CLASS_LOADED:
            ok = (d->state == UNKNOWN);
            break;
        case INSTANCIATED:
            if (ok) {
                showStatus(i18n("Initializing Applet \"%1\"...").arg(appletName()));
            }
            ok = (d->state == CLASS_LOADED);
            break;
        case INITIALIZED:
            ok = (d->state == INSTANCIATED);
            if (ok) { 
                showStatus(i18n("Starting Applet \"%1\"...").arg(appletName()));
                start();
            }
            break;
        case STARTED:
            ok = (d->state == INITIALIZED || d->state == STOPPED);
            if (ok) {    
                showStatus(i18n("Applet \"%1\" started").arg(appletName()));
            }
            break;
        case STOPPED:
            ok = (d->state == INITIALIZED || d->state == STARTED);
            if (ok) {    
                showStatus(i18n("Applet \"%1\" stopped").arg(appletName()));
            }
            break;
        case DESTROYED:
            ok = true;
            break;
        default:
            break;
    }
    if (ok) {
        d->state = newState;
    } else {
        kdError(6100) << "KJavaApplet::stateChange : don't want to switch from state "
            << d->state << " to " << newState << endl;
    } 
}

void KJavaApplet::showStatus(const TQString &msg) {
    TQStringList args;
    args << msg;
    context->processCmd("showstatus", args); 
}

void KJavaApplet::setFailed() {
    d->failed = true;
}

bool KJavaApplet::isAlive() const {
   return (
        !d->failed 
        && d->state >= INSTANCIATED
        && d->state < STOPPED
   ); 
}

KJavaApplet::AppletState KJavaApplet::state() const {
    return d->state;
}

bool KJavaApplet::failed() const {
    return d->failed;
}

#include "kjavaapplet.moc"
