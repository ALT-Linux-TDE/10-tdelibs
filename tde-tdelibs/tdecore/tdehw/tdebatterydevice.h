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

#ifndef _TDEBATTERYDEVICE_H
#define _TDEBATTERYDEVICE_H

#include "tdegenericdevice.h"

namespace TDEBatteryStatus {
enum TDEBatteryStatus {
	Charging,
	Discharging,
	Full,
	Unknown = 0x80000000
};
};

class TDECORE_EXPORT TDEBatteryDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEBatteryDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEBatteryDevice();

		/**
		* @return a double with the current battery voltage, if available
		*/
		double voltage();

		/**
		* @return a double with the minimum battery voltage, if available
		*/
		double minimumVoltage();

		/**
		* @return a double with the maximum battery voltage, if available
		*/
		double maximumVoltage();

		/**
		* @return a double with the designed maximum battery voltage, if available
		*/
		double maximumDesignVoltage();

		/**
		* @return a double with the current battery energy in watt-hours, if available
		*/
		double energy();

		/**
		* @return a double with the current battery alarm energy in watt-hours, if available
		*/
		double alarmEnergy();

		/**
		* @return a double with the maximum battery energy in watt-hours, if available
		*/
		double maximumEnergy();

		/**
		* @return a double with the designed maximum battery energy in watt-hours, if available
		*/
		double maximumDesignEnergy();

		/**
		* @return a double with the current battery discharge rate in watt-hours, if available
		*/
		double dischargeRate();

		/**
		* @return a double with the current battery discharge time remaining in seconds, if available
		*/
		double timeRemaining();

		/**
		* @return a TQString with the battery technology, if available
		*/
		TQString technology();

		/**
		* @return a TDEBatteryStatus::TDEBatteryStatus with the current battery status
		*/
		TDEBatteryStatus::TDEBatteryStatus status();

		/**
		* @return true if the battery is installed
		*/
		bool installed();

		/**
		* @return a double with the current battery charge in percent, if available
		*/
		double chargePercent();

	protected:
		/**
		* @param a double with the current battery voltage, if available
		* @internal
		*/
		void internalSetVoltage(double vt);

		/**
		* @param a double with the minimum battery voltage, if available
		* @internal
		*/
		void internalSetMinimumVoltage(double vt);

		/**
		* @param a double with the maximum battery voltage, if available
		* @internal
		*/
		void internalSetMaximumVoltage(double vt);

		/**
		* @param a double with the designed maximum battery voltage, if available
		* @internal
		*/
		void internalSetMaximumDesignVoltage(double vt);

		/**
		* @param a double with the current battery energy in watt-hours, if available
		* @internal
		*/
		void internalSetEnergy(double vt);

		/**
		* @param a double with the current battery alarm energy in watt-hours, if available
		* @internal
		*/
		void internalSetAlarmEnergy(double vt);

		/**
		* @param a double with the maximum battery energy in watt-hours, if available
		* @internal
		*/
		void internalSetMaximumEnergy(double vt);

		/**
		* @param a double with the designed maximum battery energy in watt-hours, if available
		* @internal
		*/
		void internalSetMaximumDesignEnergy(double vt);

		/**
		* @param a double with the current battery discharge rate in volt-hours, if available
		* @internal
		*/
		void internalSetDischargeRate(double vt);

		/**
		* @param a double with the current battery discharge time remaining in seconds, if available
		* @internal
		*/
		void internalSetTimeRemaining(double tr);

		/**
		* @param a TQString with the battery technology, if available
		* @internal
		*/
		void internalSetTechnology(TQString tc);

		/**
		* @param a TQString with the battery status, if available
		* @internal
		*/
		void internalSetStatus(TQString tc);

		/**
		* @param true if the battery is installed
		* @internal
		*/
		void internalSetInstalled(bool tc);

	private:
		double m_currentVoltage;
		double m_minimumVoltage;
		double m_maximumVoltage;
		double m_maximumDesignVoltage;
		double m_alarmEnergy;
		double m_currentEnergy;
		double m_maximumEnergy;
		double m_maximumDesignEnergy;
		double m_dischargeRate;
		double m_timeRemaining;
		TQString m_technology;
		TDEBatteryStatus::TDEBatteryStatus m_status;
		bool m_installed;

	friend class TDEHardwareDevices;
};

#endif // _TDEBATTERYDEVICE_H
