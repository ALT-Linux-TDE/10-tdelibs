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

#ifndef _TDEEVENTDEVICE_H
#define _TDEEVENTDEVICE_H

#include "tdegenericdevice.h"

class TDEHardwareDevices;

namespace TDEEventDeviceType {
enum TDEEventDeviceType {
	Unknown,
	ACPILidSwitch,
	ACPISleepButton,
	ACPISuspendButton,
	ACPIPowerButton,
	ACPIOtherInput,
	Other = 0x80000000
};
};

// Keep friendlySwitchList() in tdehardwaredevices.cpp in sync with this enum
namespace TDESwitchType {
enum TDESwitchType {
	Null			= 0x00000000,
	Lid			= 0x00000001,
	TabletMode		= 0x00000002,
	HeadphoneInsert		= 0x00000004,
	RFKill			= 0x00000008,
	Radio			= 0x00000010,
	MicrophoneInsert	= 0x00000020,
	Dock			= 0x00000040,
	LineOutInsert		= 0x00000080,
	JackPhysicalInsert	= 0x00000100,
	VideoOutInsert		= 0x00000200,
	CameraLensCover		= 0x00000400,
	KeypadSlide		= 0x00000800,
	FrontProximity		= 0x00001000,
	RotateLock		= 0x00002000,
	LineInInsert		= 0x00004000,
	PowerButton		= 0x00008000,
	SleepButton		= 0x00010000
};

inline TDESwitchType operator|(TDESwitchType a, TDESwitchType b)
{
	return static_cast<TDESwitchType>(static_cast<int>(a) | static_cast<int>(b));
}

inline TDESwitchType operator&(TDESwitchType a, TDESwitchType b)
{
	return static_cast<TDESwitchType>(static_cast<int>(a) & static_cast<int>(b));
}

inline TDESwitchType operator~(TDESwitchType a)
{
	return static_cast<TDESwitchType>(~static_cast<int>(a));
}
};

class TQSocketNotifier;

class TDECORE_EXPORT TDEEventDevice : public TDEGenericDevice
{
	TQ_OBJECT

	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEEventDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEEventDevice();

		/**
		* @return a TDEEventDeviceType::TDEEventDeviceType with the event device type, if known
		*/
		TDEEventDeviceType::TDEEventDeviceType eventType();

		/**
		* @return a TDESwitchType::TDESwitchType with all switches provided by this device
		*/
		TDESwitchType::TDESwitchType providedSwitches();

		/**
		* @return a TDESwitchType::TDESwitchType with all active switches provided by this device
		*/
		TDESwitchType::TDESwitchType activeSwitches();

		/**
		* @param switches a TDESwitchType::TDESwitchType with any switch flags set
		* @return a TQStringList with friendly names for all set switch flags
		*/
		static TQStringList friendlySwitchList(TDESwitchType::TDESwitchType switches);

	protected:
		/**
		* @param et a TDEEventDeviceType::TDEEventDeviceType with the event device type, if known
		* @internal
		*/
		void internalSetEventType(TDEEventDeviceType::TDEEventDeviceType et);

		/**
		* @internal
		*/
		void internalReadProvidedSwitches();

		/**
		* @param sl a TDESwitchType::TDESwitchType with all switches provided by this device
		* @internal
		*/
		void internalSetProvidedSwitches(TDESwitchType::TDESwitchType sl);

		/**
		* @internal
		*/
		void internalReadActiveSwitches();

		/**
		* @param sl a TDESwitchType::TDESwitchType with all active switches provided by this device
		* @internal
		*/
		void internalSetActiveSwitches(TDESwitchType::TDESwitchType sl);

		/**
		* @param hwmanager the master hardware manager
		* @internal
		*/
		void internalStartMonitoring(TDEHardwareDevices* hwmanager);

	protected slots:
		void eventReceived();
		void processActiveSwitches();
		virtual void connectNotify( const char* signal );

	signals:
		/**
		* @param keycode the code of the key that was pressed/released
		* See include/linux/input.h for a complete list of keycodes
		* @param device a TDEEventDevice* with the device that received the event
		*/
		void keyPressed(unsigned int keycode, TDEEventDevice* device);

		void switchChanged();

	private:
		TDEEventDeviceType::TDEEventDeviceType m_eventType;
		TDESwitchType::TDESwitchType m_providedSwitches;
		TDESwitchType::TDESwitchType m_switchActive;

		int m_fd;
		bool m_monitorActive;
		TQTimer* m_watchTimer;
		TQSocketNotifier* m_eventNotifier;

	friend class TDEHardwareDevices;
};

#endif // _TDEEVENTDEVICE_H
