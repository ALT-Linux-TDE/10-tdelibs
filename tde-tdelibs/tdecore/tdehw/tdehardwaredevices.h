/* This file is part of the TDE libraries
   Copyright (C) 2012-2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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
#ifndef _TDEHARDWAREDEVICES_H
#define _TDEHARDWAREDEVICES_H


// TDE includes
#include <tqobject.h>
#include <tqptrlist.h>
#include <tqmap.h>
#include <tqdict.h>
#include <tqstring.h>
#include <tqstringlist.h>

#include "tdelibs_export.h"
#include "kicontheme.h"

#include "tdehwcommontypes.h"

// FIXME delete those headers after complete fixing everything in over modules
#include "tdegenericdevice.h"
#include "kiconloader.h"
extern "C" {
#include <libudev.h>
}
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include "tdestoragedevice.h"
#include "tdecpudevice.h"
#include "tdebatterydevice.h"
#include "tdemainspowerdevice.h"
#include "tdenetworkdevice.h"
#include "tdebacklightdevice.h"
#include "tdemonitordevice.h"
#include "tdesensordevice.h"
#include "tderootsystemdevice.h"
#include "tdeeventdevice.h"
#include "tdeinputdevice.h"
#include "tdecryptographiccarddevice.h"

/**
 * Hardware Device Access and Monitoring Library
 *
 * @author Timothy Pearson
 */
struct udev_device;

class KSimpleDirWatch;

class TDEGenericDevice;
class TDEStorageDevice;
class TDENetworkDevice;
class TDEBacklightDevice;
class TDEMonitorDevice;
class TDESensorDevice;
class TDERootSystemDevice;
class TDEEventDevice;
class TDEInputDevice;
class TQSocketNotifier;

typedef TQPtrList<TDEGenericDevice> TDEGenericHardwareList;
typedef TQMap<TQString, TQString> TDEDeviceIDMap;
typedef TQDict<TDECPUDevice> TDECPUDeviceCache;

class TDECORE_EXPORT TDEHardwareDevices : public TQObject
{
	TQ_OBJECT

	public:	
		/**
		*  Constructor.
		*/
		TDEHardwareDevices();
		
		/**
		* Destructor.
		*/
		~TDEHardwareDevices();

		/**
		*  Query all hardware capabilities on all devices
		*  This does not normally need to be called by an application, as
		*  device detection is handled internally and automatically
		*  
		*  A call to this method immediately invalidates any TDEGenericHardwareList
		*  structures returned by listAllPhysicalDevices()
		*  
		*  @return true if successful
		*/
		bool queryHardwareInformation();

		/**
		*  List all hardware capabilities on all devices
		*  @return TDEGenericHardwareList containing all known hardware devices
		*/
		TDEGenericHardwareList listAllPhysicalDevices();

		/**
		*  List all hardware capabilities on all devices
		*  @param a TDEGenericDeviceType::TDEGenericDeviceType specifying the device class
		*  @return TDEGenericHardwareList containing all known hardware devices
		*/
		TDEGenericHardwareList listByDeviceClass(TDEGenericDeviceType::TDEGenericDeviceType cl);

		/**
		*  Return the device with system path @arg syspath, or 0 if no device exists for that path
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findBySystemPath(TQString syspath);

		/**
		*  Return the device with unique ID @arg uid, or 0 if no device exists for that uid
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findByUniqueID(TQString uid);

		/**
		*  Return the device with device node @arg devnode, or 0 if no device exists at that node
		*  @return TDEGenericDevice
		*/
		TDEGenericDevice* findByDeviceNode(TQString devnode);

		/**
		*  Return the storage device with unique ID @arg uid, or 0 if no device exists for that uid
		*  @return TDEGenericDevice
		*/
		TDEStorageDevice* findDiskByUID(TQString uid);

		/**
		*  Return the CPU device with system path @arg syspath, or 0 if no device exists for that path
		*  @return TDEGenericDevice
		*/
		TDECPUDevice* findCPUBySystemPath(TQString syspath, bool inCache);
			
		/**
		*  Look up the device in the system PCI database
		*  @param vendorid a TQString containing the vendor ID in hexadecimal
		*  @param modelid a TQString containing the model ID in hexadecimal
		*  @param subvendorid a TQString containing the subvendor ID in hexadecimal
		*  @param submodelid a TQString containing the submodel ID in hexadecimal
		*  @return a TQString containing the device name, if found
		*/
		TQString findPCIDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid);

		/**
		*  Look up the device in the system USB database
		*  @param vendorid a TQString containing the vendor ID in hexadecimal
		*  @param modelid a TQString containing the model ID in hexadecimal
		*  @param subvendorid a TQString containing the subvendor ID in hexadecimal
		*  @param submodelid a TQString containing the submodel ID in hexadecimal
		*  @return a TQString containing the device name, if found
		*/
		TQString findUSBDeviceName(TQString vendorid, TQString modelid, TQString subvendorid, TQString submodelid);

		/**
		*  Look up the device in the system PNP database
		*  @param pnpid a TQString containing the PNP ID
		*  @return a TQString containing the device name, if found
		*/
		TQString findPNPDeviceName(TQString pnpid);

		/**
		*  Look up the monitor manufacturer in the system display database
		*  @param pnpid a TQString containing the display manufacturer ID
		*  @return a TQString containing the manufacturer name, if found
		*/
		TQString findMonitorManufacturerName(TQString dpyid);

		/**
		*  Get a friendly string describing a device type
		*  @param query a TDEGenericDeviceType::TDEGenericDeviceType specifying a device type
		*  @return a TQString containing the friendly type name
		*/
		TQString getFriendlyDeviceTypeStringFromType(TDEGenericDeviceType::TDEGenericDeviceType query);

		/**
		*  Get an icon for a device type
		*  @param query a TDEGenericDeviceType::TDEGenericDeviceType specifying a device type
		*  @param size a TDEIcon::StdSizes structure specifying the desired icon size
		*  @return a TQPixmap containing the icon for the specified type
		*/
		TQPixmap getDeviceTypeIconFromType(TDEGenericDeviceType::TDEGenericDeviceType query, TDEIcon::StdSizes size);

		/**
		*  Convenience function to obtain the root system device
		*  @return a pointer to a TDERootSystemDevice object
		*/
		TDERootSystemDevice* rootSystemDevice();

		/**
		*  Rescan a hardware device to look for changes
		*  WARNING: This method can be very expensive.  Use with caution!
		*  The computational expense can be reduced somewhat if the device tree structure
		*  has not changed by calling this method with regenerateDeviceTree = false.
		*  @param hwdevice TDEGenericDevice* with the device to rescan
		*  @param dev udev_device* with the udev device to rescan
		*  @param regenerateDeviceTree true to update parent/child links in device tree
		*/
		void rescanDeviceInformation(TDEGenericDevice* hwdevice, udev_device* dev = NULL, bool regenerateDeviceTree = true);

		/**
		*  Enable or disable automatic state updates of triggerless hardware devices
		*  such as CPUs and network cards.  When enabled, your application will use
		*  additional CPU resources to continually poll triggerless hardware devices.
		*  Automatic updates are disabled by default.
		*  @param enable a bool specifiying whether or not automatic updates should be enabled
		*/
		void setTriggerlessHardwareUpdatesEnabled(bool enable);

		/**
		*  Enable or disable automatic state updates of battery hardware devices.
		*  When enabled, your application will use
		*  additional CPU resources to continually poll triggerless hardware devices.
		*  Automatic updates are disabled by default.
		*  @param enable a bool specifiying whether or not automatic updates should be enabled
		*/
		void setBatteryUpdatesEnabled(bool enable);

		/**
		*  Convert a byte count to human readable form
		*  @param bytes a double containing the number of bytes
		*  @return a TQString containing the human readable byte count
		*/
		static TQString bytesToFriendlySizeString(double bytes);

	signals:
		void hardwareAdded(TDEGenericDevice*);
		void hardwareRemoved(TDEGenericDevice*);
		void hardwareUpdated(TDEGenericDevice*);

		/**
		* @param keycode the code of the key that was pressed/released
		* See include/linux/input.h for a complete list of keycodes
		* @param device a TDEEventDevice* with the device that received the event
		*/
		void eventDeviceKeyPressed(unsigned int keycode, TDEEventDevice* device);

	private slots:
		void processHotPluggedHardware();
		void processModifiedMounts();
		void processModifiedCPUs();
		void processBatteryDevices();
		void processStatelessDevices();
		void processEventDeviceKeyPressed(unsigned int keycode, TDEEventDevice* edevice);

	private:
		void updateBlacklists(TDEGenericDevice* hwdevice, udev_device* dev);

	private:
		TDEGenericDevice *classifyUnknownDevice(udev_device* dev, TDEGenericDevice* existingdevice=0, bool force_full_classification=true);
		TDEGenericDevice *classifyUnknownDeviceByExternalRules(udev_device* dev, TDEGenericDevice* existingdevice=0, bool classifySubDevices=false);
		void updateExistingDeviceInformation(TDEGenericDevice *device, udev_device *dev=NULL);

		void updateParentDeviceInformation();
		void updateParentDeviceInformation(TDEGenericDevice* hwdevice);

		void addCoreSystemDevices();

		/**
		* Get the binary monitor EDID for the specified sysfs path
		* @return a TQByteArray containing the EDID
		*/
		TQByteArray getEDID(TQString path);
		
		/**
		* Get the monitor EDID name for the specified sysfs path
		* @return a TQPair<TQString,TQString> containing the monitor vendor and model, if available
		*/
		TQPair<TQString,TQString> getEDIDMonitorName(TQString path);

		struct udev *m_udevStruct;
		struct udev_monitor *m_udevMonitorStruct;
		TDEGenericHardwareList m_deviceList;
		int m_procMountsFd;
		KSimpleDirWatch* m_cpuWatch;
		TQTimer* m_cpuWatchTimer;
		TQTimer* m_batteryWatchTimer;
		TQTimer* m_deviceWatchTimer;

		TQSocketNotifier* m_devScanNotifier;
		TQSocketNotifier* m_mountScanNotifier;

		TQMap<TQString, bool> m_mountTable;
		TQStringList m_cpuInfo;

		TDEDeviceIDMap* pci_id_map;
		TDEDeviceIDMap* usb_id_map;
		TDEDeviceIDMap* pnp_id_map;
		TDEDeviceIDMap* dpy_id_map;
		
		TDECPUDeviceCache m_cpuByPathCache;

	friend class TDEGenericDevice;
	friend class TDEStorageDevice;
	friend class TDECPUDevice;
};

#endif // _TDEHARDWAREDEVICES_H
