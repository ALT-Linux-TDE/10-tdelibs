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

#include "tdegenericdevice.h"

#include <tqpixmap.h>

#include "tdeglobal.h" 
#include "tdelocale.h" 

#include "tdehardwaredevices.h" 

#include "config.h"

TDEGenericDevice::TDEGenericDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TQObject() {
	m_deviceType = dt;
	m_deviceName = dn;

	m_parentDevice = 0;
	m_friendlyName = TQString::null;
	m_blacklistedForUpdate = false;
}

TDEGenericDevice::~TDEGenericDevice() {
}

TDEGenericDeviceType::TDEGenericDeviceType TDEGenericDevice::type() {
	return m_deviceType;
}

TQString TDEGenericDevice::name() {
	return m_deviceName;
}

void TDEGenericDevice::internalSetName(TQString dn) {
	m_deviceName = dn;
}

TQString TDEGenericDevice::vendorName() {
	return m_vendorName;
}

void TDEGenericDevice::internalSetVendorName(TQString vn) {
	m_vendorName = vn;
}

TQString TDEGenericDevice::vendorModel() {
	return m_vendorModel;
}

void TDEGenericDevice::internalSetVendorModel(TQString vm) {
	m_vendorModel = vm;
}

TQString TDEGenericDevice::serialNumber() {
	return m_serialNumber;
}

void TDEGenericDevice::internalSetSerialNumber(TQString sn) {
	m_serialNumber = sn;
}

TQString TDEGenericDevice::systemPath() {
	if (!m_systemPath.endsWith("/")) {
		m_systemPath += "/";
	}
	return m_systemPath;
}

void TDEGenericDevice::internalSetSystemPath(TQString sp) {
	m_systemPath = sp;
}

TQString TDEGenericDevice::deviceNode() {
	return m_deviceNode;
}

void TDEGenericDevice::internalSetDeviceNode(TQString dn) {
	m_deviceNode = dn;
}

TQString TDEGenericDevice::deviceBus() {
	return m_deviceBus;
}

void TDEGenericDevice::internalSetDeviceBus(TQString db) {
	m_deviceBus = db;
}

TQString TDEGenericDevice::uniqueID() {
	m_uniqueID = m_systemPath+m_deviceNode;
	return m_uniqueID;
}

TQString TDEGenericDevice::vendorID() {
	return m_vendorID;
}

void TDEGenericDevice::internalSetVendorID(TQString id) {
	m_vendorID = id;
	m_vendorID.replace("0x", "");
}

TQString TDEGenericDevice::modelID() {
	return m_modelID;
}

void TDEGenericDevice::internalSetModelID(TQString id) {
	m_modelID = id;
	m_modelID.replace("0x", "");
}

TQString TDEGenericDevice::vendorEncoded() {
	return m_vendorenc;
}

void TDEGenericDevice::internalSetVendorEncoded(TQString id) {
	m_vendorenc = id;
}

TQString TDEGenericDevice::modelEncoded() {
	return m_modelenc;
}

void TDEGenericDevice::internalSetModelEncoded(TQString id) {
	m_modelenc = id;
}

TQString TDEGenericDevice::subVendorID() {
	return m_subvendorID;
}

void TDEGenericDevice::internalSetSubVendorID(TQString id) {
	m_subvendorID = id;
	m_subvendorID.replace("0x", "");
}

TQString TDEGenericDevice::PCIClass() {
	return m_pciClass;
}

void TDEGenericDevice::internalSetPCIClass(TQString cl) {
	m_pciClass = cl;
	m_pciClass.replace("0x", "");
}

TQString TDEGenericDevice::moduleAlias() {
	return m_modAlias;
}

void TDEGenericDevice::internalSetModuleAlias(TQString ma) {
	m_modAlias = ma;
}

TQString TDEGenericDevice::deviceDriver() {
	return m_deviceDriver;
}

void TDEGenericDevice::internalSetDeviceDriver(TQString dr) {
	m_deviceDriver = dr;
}

TQString TDEGenericDevice::subsystem() {
	return m_subsystem;
}

void TDEGenericDevice::internalSetSubsystem(TQString ss) {
	m_subsystem = ss;
}

TQString TDEGenericDevice::subModelID() {
	return m_submodelID;
}

void TDEGenericDevice::internalSetSubModelID(TQString id) {
	m_submodelID = id;
	m_submodelID.replace("0x", "");
}

void TDEGenericDevice::internalSetParentDevice(TDEGenericDevice* pd) {
	m_parentDevice = pd;
}

TDEGenericDevice* TDEGenericDevice::parentDevice() {
	return m_parentDevice;
}

TQPixmap TDEGenericDevice::icon(TDEIcon::StdSizes size) {
	return TDEGlobal::hardwareDevices()->getDeviceTypeIconFromType(type(), size);
}

bool TDEGenericDevice::blacklistedForUpdate() {
	return m_blacklistedForUpdate;
}

void TDEGenericDevice::internalSetBlacklistedForUpdate(bool bl) {
	m_blacklistedForUpdate = bl;
}

TQString TDEGenericDevice::friendlyDeviceType() {
	return TDEGlobal::hardwareDevices()->getFriendlyDeviceTypeStringFromType(type());
}

TQString TDEGenericDevice::busID() {
	TQString busid = m_systemPath;
	busid = busid.remove(0, busid.findRev("/")+1);
	busid = busid.remove(0, busid.find(":")+1);
	return busid;
}

TQString TDEGenericDevice::friendlyName() {
	if (m_friendlyName.isNull()) {
		if (type() == TDEGenericDeviceType::RootSystem) {
			m_friendlyName = "Linux System";
		}
		else if (type() == TDEGenericDeviceType::Root) {
			TQString friendlyDriverName = systemPath();
			friendlyDriverName.truncate(friendlyDriverName.length()-1);
			friendlyDriverName.remove(0, friendlyDriverName.findRev("/")+1);
			m_friendlyName = friendlyDriverName;
		}
		else if (m_modAlias.lower().startsWith("pci")) {
			m_friendlyName = TDEGlobal::hardwareDevices()->findPCIDeviceName(m_vendorID, m_modelID, m_subvendorID, m_submodelID);
		}
		else if (m_modAlias.lower().startsWith("usb")) {
			m_friendlyName = TDEGlobal::hardwareDevices()->findUSBDeviceName(m_vendorID, m_modelID, m_subvendorID, m_submodelID);
		}
		else {
			TQString acpigentype = systemPath();
			acpigentype.truncate(acpigentype.length()-1);
			acpigentype.remove(0, acpigentype.findRev("/")+1);
			TQString pnpgentype = acpigentype;
			pnpgentype.truncate(pnpgentype.find(":"));
			if (pnpgentype.startsWith("PNP")) {
				m_friendlyName = TDEGlobal::hardwareDevices()->findPNPDeviceName(pnpgentype);
			}
			else if (acpigentype.startsWith("device:")) {
				acpigentype.remove(0, acpigentype.findRev(":")+1);
				acpigentype.prepend("0x");
				m_friendlyName = i18n("ACPI Node %1").arg(acpigentype.toUInt(0,0));
			}
		}
	}

	if (m_friendlyName.isNull()) {
		// Could not identify based on model/vendor codes
		// Try to construct something from the model/vendor strings if they are available
		if (!m_vendorName.isNull() && !m_vendorModel.isNull()) {
			m_friendlyName = m_vendorName + " " + m_vendorModel;
		}
	}

	if (m_friendlyName.isNull()) {
		// Could not identify based on model/vendor
		// Guess by type
		if (type() == TDEGenericDeviceType::CPU) {
			m_friendlyName = name();
		}
		else if (type() == TDEGenericDeviceType::Event) {
			if (m_systemPath.contains("PNP0C0D")) {
				m_friendlyName = i18n("ACPI Lid Switch");
			}
			else if (m_systemPath.contains("PNP0C0E") || m_systemPath.contains("/LNXSLPBN")) {
				m_friendlyName = i18n("ACPI Sleep Button");
			}
			else if (m_systemPath.contains("PNP0C0C") || m_systemPath.contains("/LNXPWRBN")) {
				m_friendlyName = i18n("ACPI Power Button");
			}
			else if (m_parentDevice) {
				// Use parent node name
				return m_parentDevice->friendlyName();
			}
			else {
				m_friendlyName = i18n("Generic Event Device");
			}
		}
		else if (type() == TDEGenericDeviceType::Input) {
			if (m_systemPath.contains("PNP0C0D")) {
				m_friendlyName = i18n("ACPI Lid Switch");
			}
			else if (m_systemPath.contains("PNP0C0E") || m_systemPath.contains("/LNXSLPBN")) {
				m_friendlyName = i18n("ACPI Sleep Button");
			}
			else if (m_systemPath.contains("PNP0C0C") || m_systemPath.contains("/LNXPWRBN")) {
				m_friendlyName = i18n("ACPI Power Button");
			}
			else if (m_parentDevice) {
				// Use parent node name
				return m_parentDevice->friendlyName();
			}
			else {
				m_friendlyName = i18n("Generic Input Device");
			}
		}
		// Guess by driver
		else if (!m_deviceDriver.isNull()) {
			TQString friendlyDriverName = m_deviceDriver.lower();
			friendlyDriverName[0] = friendlyDriverName[0].upper();
			m_friendlyName = i18n("Generic %1 Device").arg(friendlyDriverName);
		}
		else if (m_systemPath.lower().startsWith("/sys/devices/virtual")) {
			TQString friendlyDriverName = systemPath();
			friendlyDriverName.truncate(friendlyDriverName.length()-1);
			friendlyDriverName.remove(0, friendlyDriverName.findRev("/")+1);
			if (!friendlyDriverName.isNull()) {
				m_friendlyName = i18n("Virtual Device %1").arg(friendlyDriverName);
			}
			else {
				m_friendlyName = i18n("Unknown Virtual Device");
			}
		}
		else {
			// I really have no idea what this peripheral is; say so!
			m_friendlyName = i18n("Unknown Device") + " " + name();
		}
	}

	return m_friendlyName;
}

TQString TDEGenericDevice::detailedFriendlyName() {
	return friendlyName();
}

#include "tdegenericdevice.moc"
