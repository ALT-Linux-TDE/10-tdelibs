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

#ifndef _TDEBACKLIGHTDEVICE_H
#define _TDEBACKLIGHTDEVICE_H

#include "tdegenericdevice.h"
#include "tdehwcommontypes.h"

class TDECORE_EXPORT TDEBacklightDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEBacklightDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEBacklightDevice();

		/**
		* @return a TDEDisplayPowerLevel::TDEDisplayPowerLevel with the current power level
		*/
		TDEDisplayPowerLevel::TDEDisplayPowerLevel powerLevel();

		/**
		* @return an integer with the number of discrete control steps available
		*/
		int brightnessSteps();

		/**
		* @return a double with the current brightness percentage
		*/
		double brightnessPercent();

		/**
		* @return true if permissions allow brightness can be set, false if not
		*/
		bool canSetBrightness();

		/**
		* @return an int with the current raw brightness
		*/
		int rawBrightness();

		/**
		* @param br an integer with the new raw brightness value
		*/
		void setRawBrightness(int br);

	protected:
		/**
		* @param pl a TDEDisplayPowerLevel::TDEDisplayPowerLevel with the current power level
		* @internal
		*/
		void internalSetPowerLevel(TDEDisplayPowerLevel::TDEDisplayPowerLevel pl);

		/**
		* @param br an integer with the maximum raw brightness value
		* @internal
		*/
		void internalSetMaximumRawBrightness(int br);

		/**
		* @param br an integer with the current raw brightness value
		* @internal
		*/
		void internalSetCurrentRawBrightness(int br);

	private:
		TDEDisplayPowerLevel::TDEDisplayPowerLevel m_powerLevel;
		int m_currentBrightness;
		int m_maximumBrightness;

	friend class TDEHardwareDevices;
};

#endif // _TDEBACKLIGHTDEVICE_H
