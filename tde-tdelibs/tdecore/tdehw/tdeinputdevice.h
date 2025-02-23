/* This file is part of the TDE libraries
   Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>
             (C) 2013 Golubev Alexander <fatzer2@gmail.com>

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

#ifndef _TDEINPUTDEVICE_H
#define _TDEINPUTDEVICE_H

#include "tdegenericdevice.h"

namespace TDEInputDeviceType {
enum TDEInputDeviceType {
	Unknown,
	ACPILidSwitch,
	ACPISleepButton,
	ACPISuspendButton,
	ACPIPowerButton,
	ACPIOtherInput,
	Other = 0x80000000
};
};

class TDECORE_EXPORT TDEInputDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEInputDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEInputDevice();

		/**
		* @return a TDEInputDeviceType::TDEInputDeviceType with the input device type, if known
		*/
		TDEInputDeviceType::TDEInputDeviceType inputType();

	protected:
		/**
		* @param it a TDEInputDeviceType::TDEInputDeviceType with the input device type, if known
		* @internal
		*/
		void internalSetInputType(TDEInputDeviceType::TDEInputDeviceType it);

	private:
		TDEInputDeviceType::TDEInputDeviceType m_inputType;

	friend class TDEHardwareDevices;
};

#endif // _TDEINPUTDEVICE_H
