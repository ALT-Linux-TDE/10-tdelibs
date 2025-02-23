/*  This file is part of the KDE project
    Copyright (C) 2003 Matthias Kretz <kretz@kde.org>

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

#include "ksettings/dispatcher.h"

#include <tqsignal.h>

#include <kstaticdeleter.h>
#include <kdebug.h>
#include <tdeconfig.h>
#include <assert.h>

namespace KSettings
{

//class Dispatcher::DispatcherPrivate
//{
//};

static KStaticDeleter<Dispatcher> ksd_kpd;

Dispatcher * Dispatcher::m_self = 0;

Dispatcher * Dispatcher::self()
{
    kdDebug( 701 ) << k_funcinfo << endl;
    if( m_self == 0 )
        ksd_kpd.setObject( m_self, new Dispatcher() );
    return m_self;
}

Dispatcher::Dispatcher( TQObject * parent, const char * name )
    : TQObject( parent, name )
    //, d( 0 )
{
    kdDebug( 701 ) << k_funcinfo << endl;
}

Dispatcher::~Dispatcher()
{
    kdDebug( 701 ) << k_funcinfo << endl;
    //delete d;
}

void Dispatcher::registerInstance( TDEInstance * instance, TQObject * recv, const char * slot )
{
    assert( instance != 0 );
    // keep the TDEInstance around and call
    // instance->config()->reparseConfiguration when the app should reparse
    TQCString instanceName = instance->instanceName();
    kdDebug( 701 ) << k_funcinfo << instanceName << endl;
    m_instanceName[ recv ] = instanceName;
    TQSignal * sig;
    if( m_instanceInfo.contains( instanceName ) )
    {
        sig = m_instanceInfo[ instanceName ].signal;
    }
    else
    {
        sig = new TQSignal( this, "signal dispatcher" );
        m_instanceInfo[ instanceName ].signal = sig;
        m_instanceInfo[ instanceName ].instance = instance;
    }
    sig->connect( recv, slot );

    ++m_instanceInfo[ instanceName ].count;
    connect( recv, TQ_SIGNAL( destroyed( TQObject * ) ), this, TQ_SLOT( unregisterInstance( TQObject * ) ) );
}

TDEConfig * Dispatcher::configForInstanceName( const TQCString & instanceName )
{
    kdDebug( 701 ) << k_funcinfo << endl;
    if( m_instanceInfo.contains( instanceName ) )
    {
        TDEInstance * inst = m_instanceInfo[ instanceName ].instance;
        if( inst )
            return inst->config();
    }
    //if( fallback )
        //return new KSimpleConfig( instanceName );
    return 0;
}

TQStrList Dispatcher::instanceNames() const
{
    kdDebug( 701 ) << k_funcinfo << endl;
    TQStrList names;
    for( TQMap<TQCString, InstanceInfo>::ConstIterator it = m_instanceInfo.begin(); it != m_instanceInfo.end(); ++it )
        if( ( *it ).count > 0 )
            names.append( it.key() );
    return names;
}

void Dispatcher::reparseConfiguration( const TQCString & instanceName )
{
    kdDebug( 701 ) << k_funcinfo << instanceName << endl;
    // check if the instanceName is valid:
    if( ! m_instanceInfo.contains( instanceName ) )
        return;
    // first we reparse the config of the instance so that the TDEConfig object
    // will be up to date
    m_instanceInfo[ instanceName ].instance->config()->reparseConfiguration();
    TQSignal * sig = m_instanceInfo[ instanceName ].signal;
    if( sig )
    {
        kdDebug( 701 ) << "emit signal to instance" << endl;
        sig->activate();
    }
}

void Dispatcher::syncConfiguration()
{
    for( TQMap<TQCString, InstanceInfo>::ConstIterator it = m_instanceInfo.begin(); it != m_instanceInfo.end(); ++it )
    {
        ( *it ).instance->config()->sync();
    }
}

void Dispatcher::unregisterInstance( TQObject * obj )
{
    kdDebug( 701 ) << k_funcinfo << endl;
    TQCString name = m_instanceName[ obj ];
    m_instanceName.remove( obj ); //obj will be destroyed when we return, so we better remove this entry
    --m_instanceInfo[ name ].count;
    if( m_instanceInfo[ name ].count == 0 )
    {
        delete m_instanceInfo[ name ].signal;
        m_instanceInfo.remove( name );
    }
}

//X TDEInstance * Dispatcher::instanceForName( const TQCString & instanceName )
//X {
//X     return m_instanceInfo[ instanceName ].instance;
//X }

} //namespace

#include "dispatcher.moc"
