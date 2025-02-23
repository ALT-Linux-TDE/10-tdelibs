/*  This file is part of the KDE project
    Copyright (C) 2007 Will Stephenson <wstephenson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library.  If not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this library
    with any edition of TQt, and distribute the resulting executable,
    without including the source code for TQt in the source distribution.
*/

#include <tqlabel.h>
#include <tqtooltip.h>
#include <kiconloader.h>
#include <tdelocale.h>

#include "connectionmanager.h"

#include "networkstatusindicator.h"

StatusBarNetworkStatusIndicator::StatusBarNetworkStatusIndicator(
    TQWidget * parent, const char * name ) : TQHBox( parent, name )/*, d( new StatusBarNetworkStatusIndicatorPrivate )*/
{
  setMargin( 2 );
  setSpacing( 1 );
  TQLabel * label = new TQLabel( this, "offlinemodelabel" );
  label->setPixmap( SmallIcon("connect_no") );
  TQToolTip::add( label, i18n( "The desktop is offline" ) );

  connect( ConnectionManager::self(), TQ_SIGNAL( statusChanged( const TQString &, NetworkStatus::EnumStatus ) ),
      TQ_SLOT( networkStatusChanged( const TQString &, NetworkStatus::EnumStatus) ) );

}

void StatusBarNetworkStatusIndicator::init()
{
  networkStatusChanged( ConnectionManager::self()->status(TQString("")));
}

StatusBarNetworkStatusIndicator::~StatusBarNetworkStatusIndicator()
{
}

void StatusBarNetworkStatusIndicator::networkStatusChanged( const TQString & host, NetworkStatus::EnumStatus status )
{
  networkStatusChanged(status);
}

void StatusBarNetworkStatusIndicator::networkStatusChanged( NetworkStatus::EnumStatus status )
{
  if ( status == NetworkStatus::Online || status == NetworkStatus::NoNetworks ) {
    hide();
  } else {
    show();
  }
}

#include "networkstatusindicator.moc"
