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

#ifndef _TDEGENERICDEVICE_H
#define _TDEGENERICDEVICE_H

#include <tqobject.h>
#include <tqstring.h>

#include "tdelibs_export.h" 
#include "kicontheme.h" 

#include "tdehwcommontypes.h" 

class TDECORE_EXPORT TDEGenericDevice : public TQObject
{
	TQ_OBJECT

	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDEGenericDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDEGenericDevice();

		/**
		* @return a TDEGenericDeviceType::TDEGenericDeviceType specifying the device type
		*/
		TDEGenericDeviceType::TDEGenericDeviceType type();

		/**
		* @return a TQString with the device name, if any
		*/
		TQString name();

		/**
		* @return a TQString with the vendor name, if any
		*/
		TQString vendorName();

		/**
		* @return a TQString with the vendor model, if any
		*/
		TQString vendorModel();

		/**
		* @return a TQString with the serial number, if any
		*/
		TQString serialNumber();

		/**
		* @return a TQString with a friendly name
		*
		* While TDE tries very hard to generate and return a friendly name for this device,
		* sometimes the best it will be able to do is "Unknown Device [xxxx:yyyy]"
		*/
		virtual TQString friendlyName();

		/**
		* @return a TQString with a detailed friendly name
		*
		* In most cases this will return exactly the same string as friendlyName(), however
		* for certain devices (e.g. disks) this method will provide additional information
		*/
		virtual TQString detailedFriendlyName();

		/**
		* @return a TQString with the device bus name, if any
		*/
		TQString deviceBus();

		/**
		* @return a TQString with the system path, if any
		*
		* This method is non-portable, so be careful!
		*/
		TQString systemPath();

		/**
		* @return a TQString with the system device node, if any
		*
		* This method is non-portable, so be careful!
		*/
		TQString deviceNode();

		/**
		* @return true if this device has been blacklisted for update actions
		*/
		bool blacklistedForUpdate();

		/**
		* @return a TQString containing a unique identifier for this device
		*/
		TQString uniqueID();

		/**
		* @return a TQString with the vendor ID, if any
		*/
		TQString vendorID();

		/**
		* @return a TQString with the model ID, if any
		*/
		TQString modelID();

		/**
		* @return a TQString with the encoded vendor, if any
		*/
		TQString vendorEncoded();

		/**
		* @return a TQString with the encoded model, if any
		*/
		TQString modelEncoded();

		/**
		* @return a TQString with the subvendor ID, if any
		*/
		TQString subVendorID();

		/**
		* @return a TQString with the submodel ID, if any
		*/
		TQString subModelID();

		/**
		* @return a TQString with the PCI device class, if any
		*/
		TQString PCIClass();

		/**
		* @return a TQString with the module alias string, if any
		*/
		TQString moduleAlias();

		/**
		* @return a TQString with the device driver, if any
		*/
		TQString deviceDriver();

		/**
		* @return a TQString with the subsystem type, if any
		*/
		TQString subsystem();

		/**
		* @return a TDEGenericDevice* with the parent device, if any
		*/
		TDEGenericDevice* parentDevice();

		/**
		*  @return a TQString containing the friendly type name
		*/
		virtual TQString friendlyDeviceType();

		/**
		*  @return a TQString containing the device bus ID, if any
		*/
		TQString busID();

		/**
		*  Get an icon for this device
		*  @param size a TDEIcon::StdSizes structure specifying the desired icon size
		*  @return a TQPixmap containing the icon for the specified type
		*/
		virtual TQPixmap icon(TDEIcon::StdSizes size);

	protected:
		/**
		* @param a TQString with the device name, if any
		* @internal
		*/
		void internalSetName(TQString dn);

		/**
		* @param a TQString with the vendor name, if any
		* @internal
		*/
		void internalSetVendorName(TQString vn);

		/**
		* @param a TQString with the vendor model, if any
		* @internal
		*/
		void internalSetVendorModel(TQString vm);

		/**
		* @param a TQString with the serial number, if any
		* @internal
		*/
		void internalSetSerialNumber(TQString sn);

		/**
		* @param a TQString with the device bus name, if any
		* @internal
		*/
		void internalSetDeviceBus(TQString db);

		/**
		* @param a TQString with the system path, if any
		* @internal
		*
		* This method is non-portable, so be careful!
		*/
		void internalSetSystemPath(TQString sp);

		/**
		* @param a TQString with the system device node, if any
		* @internal
		*
		* This method is non-portable, so be careful!
		*/
		void internalSetDeviceNode(TQString dn);

		/**
		* @param bl true if this device has been blacklisted for update actions
		* @internal
		*/
		void internalSetBlacklistedForUpdate(bool bl);

		/**
		* @param a TQString with the vendor ID, if any
		* @internal
		*/
		void internalSetVendorID(TQString id);

		/**
		* @param a TQString with the model ID, if any
		* @internal
		*/
		void internalSetModelID(TQString id);

		/**
		* @param a TQString with the encoded vendor, if any
		* @internal
		*/
		void internalSetVendorEncoded(TQString id);

		/**
		* @param a TQString with the encoded model, if any
		* @internal
		*/
		void internalSetModelEncoded(TQString id);

		/**
		* @param a TQString with the subvendor ID, if any
		* @internal
		*/
		void internalSetSubVendorID(TQString id);

		/**
		* @param a TQString with the submodel ID, if any
		* @internal
		*/
		void internalSetSubModelID(TQString id);

		/**
		* @param a TQString with the PCI device class, if any
		* @internal
		*/
		void internalSetPCIClass(TQString cl);

		/**
		* @param a TQString with the module alias string, if any
		* @internal
		*/
		void internalSetModuleAlias(TQString ma);

		/**
		* @param a TQString with the device driver, if any
		* @internal
		*/
		void internalSetDeviceDriver(TQString dr);

		/**
		* @param a TQString with the subsystem type, if any
		* @internal
		*/
		void internalSetSubsystem(TQString ss);

		/**
		* @param a TDEGenericDevice* with the parent device, if any
		* @internal
		*/
		void internalSetParentDevice(TDEGenericDevice* pd);

	private:
		TDEGenericDeviceType::TDEGenericDeviceType m_deviceType;
		TQString m_deviceName;
		TQString m_systemPath;
		TQString m_deviceNode;
		TQString m_vendorName;
		TQString m_vendorModel;
		TQString m_serialNumber;
		TQString m_deviceBus;
		TQString m_uniqueID;
		TQString m_vendorID;
		TQString m_modelID;
		TQString m_vendorenc;
		TQString m_modelenc;
		TQString m_subvendorID;
		TQString m_submodelID;
		TQString m_pciClass;
		TQString m_modAlias;
		TQString m_deviceDriver;
		TQString m_subsystem;
		TQString m_friendlyName;
		bool m_blacklistedForUpdate;
		TDEGenericDevice* m_parentDevice;

		// Internal use only!
		TQStringList m_externalSubtype;
		TQString m_externalRulesFile;
		TQString m_udevtype;
		TQString m_udevdevicetypestring;
		TQString udevdevicetypestring_alt;

	friend class TDEHardwareDevices;
};

#endif // _TDEGENERICDEVICE_H
