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

#ifndef _TDEMONITORDEVICE_H
#define _TDEMONITORDEVICE_H

#include "tdegenericdevice.h"
#include "tdehwcommontypes.h"

typedef TQPair<unsigned int, unsigned int> TDEResolutionPair;
typedef TQValueList< TDEResolutionPair > TDEResolutionList;

class TDECORE_EXPORT TDEMonitorDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEMonitorDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEMonitorDevice();

		/**
		* @return true if a monitor is connected, false if not
		*/
		bool connected();

		/**
		* @return true if this port is enabled, false if not
		*/
		bool enabled();

		/**
		* @return a TQByteArray containing this monitor's EDID information
		*/
		TQByteArray edid();

		/**
		* @return a TDEResolutionList containing this monitor's supported resolutions
		*/
		TDEResolutionList resolutions();

		/**
		* @return a TQString containing the display port type
		*/
		TQString portType();

		/**
		* @return a TDEDisplayPowerLevel::TDEDisplayPowerLevel with the current power level
		*/
		TDEDisplayPowerLevel::TDEDisplayPowerLevel powerLevel();

	protected:
		/**
		* @param true if a monitor is connected, false if not
		* @internal
		*/
		void internalSetConnected(bool cn);

		/**
		* @param true if this port is enabled, false if not
		* @internal
		*/
		void internalSetEnabled(bool en);

		/**
		* @param ed a TQByteArray containing this monitor's EDID information
		* @internal
		*/
		void internalSetEdid(TQByteArray ed);

		/**
		* @param rs a TDEResolutionList containing this monitor's supported resolutions
		* @internal
		*/
		void internalSetResolutions(TDEResolutionList rs);

		/**
		* @param pt a TQString containing the display port type
		* @internal
		*/
		void internalSetPortType(TQString pt);

		/**
		* @param pl a TDEDisplayPowerLevel::TDEDisplayPowerLevel with the current power level
		* @internal
		*/
		void internalSetPowerLevel(TDEDisplayPowerLevel::TDEDisplayPowerLevel pl);

	private:
		bool m_connected;
		bool m_enabled;
		TQByteArray m_edid;
		TDEResolutionList m_resolutions;
		TQString m_portType;
		TDEDisplayPowerLevel::TDEDisplayPowerLevel m_powerLevel;

	friend class TDEHardwareDevices;
};


#endif // _TDEMONITORDEVICE_H
