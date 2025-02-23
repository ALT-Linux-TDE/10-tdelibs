/*  This file is part of tdepim.
    Copyright (C) 2005,2007 Will Stephenson <wstephenson@kde.org>

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

#ifndef KDED_NETWORKSTATUSIFACE_H
#define KDED_NETWORKSTATUSIFACE_H

#include <dcopobject.h>
#include <tqstringlist.h>

#include "networkstatuscommon.h"

class NetworkStatusIface : virtual public DCOPObject
{
K_DCOP
k_dcop:
	// Client interface
	virtual TQStringList networks() = 0;
	virtual int status() = 0;
	// Service interface
	virtual void setNetworkStatus( const TQString & networkName, int status ) = 0;
	virtual void registerNetwork( NetworkStatus::Properties properties ) = 0;
	virtual void unregisterNetwork( const TQString & networkName ) = 0 ;
k_dcop_signals:
	/**
	 * A status change occurred affecting the overall connectivity
	 * @param status The new status
	 */
	virtual void statusChange( int status ) = 0;
};
#endif
