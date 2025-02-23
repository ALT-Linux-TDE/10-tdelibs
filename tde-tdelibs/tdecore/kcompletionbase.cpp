/* This file is part of the KDE libraries

   Copyright (c) 2000 Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqobject.h>

#include <kcompletion.h>

TDECompletionBase::TDECompletionBase()
{
    m_delegate = 0L;
    // Assign the default completion type to use.
    m_iCompletionMode = TDEGlobalSettings::completionMode();

    // Initialize all key-bindings to 0 by default so that
    // the event filter will use the global settings.
    useGlobalKeyBindings();

    // By default we initialize everything to false.
    // All the variables would be setup properly when
    // the appropriate member functions are called.
    setup( false, false, false );
}

TDECompletionBase::~TDECompletionBase()
{
    if( m_bAutoDelCompObj && m_pCompObj )
    {
        delete m_pCompObj;
    }
}

void TDECompletionBase::setDelegate( TDECompletionBase *delegate )
{
    m_delegate = delegate;

    if ( m_delegate ) {
        m_delegate->m_bAutoDelCompObj = m_bAutoDelCompObj;
        m_delegate->m_bHandleSignals  = m_bHandleSignals;
        m_delegate->m_bEmitSignals    = m_bEmitSignals;
        m_delegate->m_iCompletionMode = m_iCompletionMode;
        m_delegate->m_keyMap          = m_keyMap;
    }
}

TDECompletion* TDECompletionBase::completionObject( bool hsig )
{
    if ( m_delegate )
        return m_delegate->completionObject( hsig );
    
    if ( !m_pCompObj )
    {
        setCompletionObject( new TDECompletion(), hsig );
	m_bAutoDelCompObj = true;
    }
    return m_pCompObj;
}

void TDECompletionBase::setCompletionObject( TDECompletion* compObj, bool hsig )
{
    if ( m_delegate ) {
        m_delegate->setCompletionObject( compObj, hsig );
        return;
    }
    
    if ( m_bAutoDelCompObj && compObj != m_pCompObj )
        delete m_pCompObj;

    m_pCompObj = compObj;

    // We emit rotation and completion signals
    // if completion object is not NULL.
    setup( false, hsig, !m_pCompObj.isNull() );
}

// BC: Inline this function and possibly rename it to setHandleEvents??? (DA)
void TDECompletionBase::setHandleSignals( bool handle )
{
    if ( m_delegate )
        m_delegate->setHandleSignals( handle );
    else
        m_bHandleSignals = handle;
}

void TDECompletionBase::setCompletionMode( TDEGlobalSettings::Completion mode )
{
    if ( m_delegate ) {
        m_delegate->setCompletionMode( mode );
        return;
    }
    
    m_iCompletionMode = mode;
    // Always sync up TDECompletion mode with ours as long as we
    // are performing completions.
    if( m_pCompObj && m_iCompletionMode != TDEGlobalSettings::CompletionNone )
        m_pCompObj->setCompletionMode( m_iCompletionMode );
}

bool TDECompletionBase::setKeyBinding( KeyBindingType item, const TDEShortcut& cut )
{
    if ( m_delegate )
        return m_delegate->setKeyBinding( item, cut );


    if( !cut.isNull() )
    {
        for( KeyBindingMap::Iterator it = m_keyMap.begin(); it != m_keyMap.end(); ++it )
            if( it.data() == cut )  return false;
    }
    m_keyMap.replace( item, cut );
    return true;
}

void TDECompletionBase::useGlobalKeyBindings()
{
    if ( m_delegate ) {
        m_delegate->useGlobalKeyBindings();
        return;
    }
    
    m_keyMap.clear();
    m_keyMap.insert( TextCompletion, 0 );
    m_keyMap.insert( PrevCompletionMatch, 0 );
    m_keyMap.insert( NextCompletionMatch, 0 );
    m_keyMap.insert( SubstringCompletion, 0 );
}

void TDECompletionBase::setup( bool autodel, bool hsig, bool esig )
{
    if ( m_delegate ) {
        m_delegate->setup( autodel, hsig, esig );
        return;
    }
    
    m_bAutoDelCompObj = autodel;
    m_bHandleSignals = hsig;
    m_bEmitSignals = esig;
}
