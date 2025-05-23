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

#ifndef KDE_NETWORKSTATUS_INDICATOR_H
#define KDE_NETWORKSTATUS_INDICATOR_H

#include <tqhbox.h>
#include <tdemacros.h>
#include <networkstatuscommon.h>

class StatusBarNetworkStatusIndicator : public TQHBox
{
TQ_OBJECT
public:
  StatusBarNetworkStatusIndicator( TQWidget * parent, const char * name );
  virtual ~StatusBarNetworkStatusIndicator();
  void init();
protected slots:
  void networkStatusChanged( const TQString & host, NetworkStatus::EnumStatus status );
  void networkStatusChanged( NetworkStatus::EnumStatus status );
};

#endif

