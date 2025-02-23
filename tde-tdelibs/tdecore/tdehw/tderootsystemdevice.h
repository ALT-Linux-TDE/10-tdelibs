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

#ifndef _TDEROOTSYSTEMDEVICE_H
#define _TDEROOTSYSTEMDEVICE_H

#include "tdegenericdevice.h"
#include "tdehwcommontypes.h"

namespace TDESystemFormFactor {
enum TDESystemFormFactor {
	Unclassified,
	Desktop,
	Laptop,
	Server,
	Other = 0x80000000
};
};

namespace TDESystemPowerState {
enum TDESystemPowerState {
	Active,
	Standby,
	Freeze,
	Suspend,
	Hibernate,
	PowerOff,
	Reboot,
	HybridSuspend,
	Disk   // Used temporarily to detect hibernation and hybrid suspend capability
};
};

namespace TDESystemHibernationMethod {
enum TDESystemHibernationMethod {
	Unsupported,
	Platform,
	Shutdown,
	Reboot,
	TestProc,
	Test,
	Suspend
};
};

typedef TQValueList<TDESystemPowerState::TDESystemPowerState> TDESystemPowerStateList;
typedef TQValueList<TDESystemHibernationMethod::TDESystemHibernationMethod> TDESystemHibernationMethodList;

class TDECORE_EXPORT TDERootSystemDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDERootSystemDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);

		/**
		* Destructor.
		*/
		~TDERootSystemDevice();

		/**
		* @return a TDESystemFormFactor::TDESystemFormFactor with the system's form factor
		*/
		TDESystemFormFactor::TDESystemFormFactor formFactor();

		/**
		* @return a TDESystemPowerStateList with all available power states
		*/
		TDESystemPowerStateList powerStates();

		/**
		* @return a TDESystemHibernationMethodList with all available hibernation methods
		*/
		TDESystemHibernationMethodList hibernationMethods();

		/**
		* @return a TDESystemHibernationMethod::TDESystemHibernationMethod with the current hibernation method
		*/
		TDESystemHibernationMethod::TDESystemHibernationMethod hibernationMethod();

		/**
		* @return an unsigned long with the number of bytes required to hibernate
		*/
		unsigned long diskSpaceNeededForHibernation();

		/**
		* @return true if permissions allow the hibernation method to be set, false if not
		*/
		bool canSetHibernationMethod();

		/**
		* @return true if hardware and permissions allow the system to enter standby, false if not
		*/
		bool canStandby();

		/**
		* @return true if hardware and permissions allow the system to enter freeze state, false if not
		*/
		bool canFreeze();

		/**
		* @return true if hardware and permissions allow the system to be suspended, false if not
		*/
		bool canSuspend();

		/**
		* @return true if hardware and permissions allow the system to be hibernated, false if not
		*/
		bool canHibernate();

		/**
		* @return true if hardware and permissions allow the system to be hybrid suspended, false if not
		*/
		bool canHybridSuspend();

		/**
		* @return true if permissions allow the system to be powered down, false if not
		*/
		bool canPowerOff();

		/**
		* @return true if permissions allow the system to be rebooted, false if not
		*/
		bool canReboot();

		/**
		* @param hm a TDESystemHibernationMethod::TDESystemHibernationMethod with the desired hibernation method
		*/
		void setHibernationMethod(TDESystemHibernationMethod::TDESystemHibernationMethod hm);

		/**
		* @param ps a TDESystemPowerState::TDESystemPowerState with the desired power state
		* @return true if power state was set
		*/
		bool setPowerState(TDESystemPowerState::TDESystemPowerState ps);

	protected:
		/**
		* @param ff a TDESystemFormFactor::TDESystemFormFactor with the system's form factor
		* @internal
		*/
		void internalSetFormFactor(TDESystemFormFactor::TDESystemFormFactor ff);

		/**
		* @param ps a TDESystemPowerStateList with all available power states
		* @internal
		*/
		void internalSetPowerStates(TDESystemPowerStateList ps);

		/**
		* @param hm a TDESystemHibernationMethodList with all available hibernation methods
		* @internal
		*/
		void internalSetHibernationMethods(TDESystemHibernationMethodList hm);

		/**
		* @param hm a TDESystemHibernationMethod::TDESystemHibernationMethod with the current hibernation method
		* @internal
		*/
		void internalSetHibernationMethod(TDESystemHibernationMethod::TDESystemHibernationMethod hm);

		/**
		* @param sz an unsigned long with the number of bytes required to hibernate
		* @internal
		*/
		void internalSetDiskSpaceNeededForHibernation(unsigned long sz);

	private:
		TDESystemFormFactor::TDESystemFormFactor m_formFactor;
		TDESystemPowerStateList m_powerStates;
		TDESystemHibernationMethodList m_hibernationMethods;
		TDESystemHibernationMethod::TDESystemHibernationMethod m_hibernationMethod;
		unsigned long m_hibernationSpace;

	friend class TDEHardwareDevices;
};

#endif // _TDEROOTSYSTEMDEVICE_H
