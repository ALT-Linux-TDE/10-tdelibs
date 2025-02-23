/* This file is part of the KDE project
   Copyright (C) 2001 Ian Reinhart Geiser <geiseri@yahoo.com>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KAPPDCOP_INTERFACE_H
#define KAPPDCOP_INTERFACE_H

#include <dcopobject.h>
#include <tqstringlist.h>
#include <tqcstring.h>
#include <dcopref.h>
#include "tdelibs_export.h"

class TDEApplication;

/**
This is the main interface to the TDEApplication.  This will provide a consistant
dcop interface to all KDE applications that use it.
@short DCOP interface to TDEApplication.
@author Ian Reinhart Geiser <geiseri@yahoo.com>
*/
class TDECORE_EXPORT KAppDCOPInterface : virtual public DCOPObject
{
K_DCOP

public:
	/**
	Construct a new interface object.
	@param theKapp - The parent TDEApplication object
	    that will provide us with the functional interface.
	*/
	KAppDCOPInterface( TDEApplication * theKapp );
	/**
	Destructor
	Cleans up the dcop connection.
	**/
	~KAppDCOPInterface();
k_dcop:
	/**
	Disable current applications session management
	**/
	void disableSessionManagement();
	
	TQCString startupId();
	TQCString caption();
	
	void quit();

	void reparseConfiguration();

	void updateUserTimestamp( ulong time );
	/**
	Send a fake keypress to all TDEApplication instances
	For internal use in connecting insecure function keys to
	KDE applications while the X keyboard is locked.
	**/
	void sendFakeKey( unsigned int keyCode);

private:
	TDEApplication *m_TDEApplication;
};

#endif


