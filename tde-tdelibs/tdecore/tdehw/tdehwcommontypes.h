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

#ifndef _TDEHWCOMMON_H
#define _TDEHWCOMMON_H

// Keep readGenericDeviceTypeFromString(), getFriendlyDeviceTypeStringFromType(), and getDeviceTypeIconFromType() in tdehardwaredevices.cpp in sync with this enum
namespace TDEGenericDeviceType {
enum TDEGenericDeviceType {
	Root,
	RootSystem,
	CPU,
	GPU,
	RAM,
	Bus,
	I2C,
	MDIO,
	Mainboard,
	Disk,
	SCSI,
	StorageController,
	Mouse,
	Keyboard,
	HID,
	Modem,
	Monitor,
	Network,
	NonvolatileMemory,
	Printer,
	Scanner,
	Sound,
	VideoCapture,
	IEEE1394,
	PCMCIA,
	Camera,
	TextIO,
	Serial,
	Parallel,
	Peripheral,
	Backlight,
	Battery,
	PowerSupply,
	Dock,
	ThermalSensor,
	ThermalControl,
	BlueTooth,
	Bridge,
	Hub,
	Platform,
	Cryptography,
	CryptographicCard,
	BiometricSecurity,
	TestAndMeasurement,
	Timekeeping,
	Event,
	Input,
	PNP,
	OtherACPI,
	OtherUSB,
	OtherMultimedia,
	OtherPeripheral,
	OtherSensor,
	OtherVirtual,
	Other,
	Last = Other
};
};

namespace TDEDisplayPowerLevel {
enum TDEDisplayPowerLevel {
	On,
	Standby,
	Suspend,
	Off
};
};

#endif // _TDEHWCOMMON_H
