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

#ifndef _TDESTORAGEDEVICE_H
#define _TDESTORAGEDEVICE_H

#include "tdegenericdevice.h"
#include "tqvariant.h"
#include "tdemacros.h"

struct crypt_device;

// Keep readDiskDeviceSubtypeFromString() in tdehardwaredevices.cpp in sync with this enum
namespace TDEDiskDeviceType {
enum TDEDiskDeviceType : unsigned long long {
	Null          = 0x0000000000000000ULL,
	MediaDevice   = 0x0000000000000001ULL,
	Floppy        = 0x0000000000000002ULL,
	CDROM         = 0x0000000000000004ULL,
	CDR           = 0x0000000000000008ULL,
	CDRW          = 0x0000000000000010ULL,
	CDMO          = 0x0000000000000020ULL,
	CDMRRW        = 0x0000000000000040ULL,
	CDMRRWW       = 0x0000000000000080ULL,
	DVDROM        = 0x0000000000000100ULL,
	DVDRAM        = 0x0000000000000200ULL,
	DVDR          = 0x0000000000000400ULL,
	DVDRW         = 0x0000000000000800ULL,
	DVDRDL        = 0x0000000000001000ULL,
	DVDRWDL       = 0x0000000000002000ULL,
	DVDPLUSR      = 0x0000000000004000ULL,
	DVDPLUSRW     = 0x0000000000008000ULL,
	DVDPLUSRDL    = 0x0000000000010000ULL,
	DVDPLUSRWDL   = 0x0000000000020000ULL,
	BDROM         = 0x0000000000040000ULL,
	BDR           = 0x0000000000080000ULL,
	BDRW          = 0x0000000000100000ULL,
	HDDVDROM      = 0x0000000000200000ULL,
	HDDVDR        = 0x0000000000400000ULL,
	HDDVDRW       = 0x0000000000800000ULL,
	Zip           = 0x0000000001000000ULL,
	Jaz           = 0x0000000002000000ULL,
	Camera        = 0x0000000004000000ULL,
	LUKS          = 0x0000000008000000ULL,
	OtherCrypted  = 0x0000000010000000ULL,
	CDAudio       = 0x0000000020000000ULL,
	CDVideo       = 0x0000000040000000ULL,
	DVDVideo      = 0x0000000080000000ULL,
	BDVideo       = 0x0000000100000000ULL,
	Flash         = 0x0000000200000000ULL,
	USB           = 0x0000000400000000ULL,
	Tape          = 0x0000000800000000ULL,
	HDD           = 0x0000001000000000ULL,
	Optical       = 0x0000002000000000ULL,
	RAM           = 0x0000004000000000ULL,
	Loop          = 0x0000008000000000ULL,
	CompactFlash  = 0x0000010000000000ULL,
	MemoryStick   = 0x0000020000000000ULL,
	SmartMedia    = 0x0000040000000000ULL,
	SDMMC         = 0x0000080000000000ULL,
	UnlockedCrypt = 0x0000100000000000ULL,
	Other         = 0x8000000000000000ULL
};

inline TDEDiskDeviceType operator|(TDEDiskDeviceType a, TDEDiskDeviceType b)
{
	return static_cast<TDEDiskDeviceType>(static_cast<unsigned long long>(a) | static_cast<unsigned long long>(b));
}

inline TDEDiskDeviceType operator&(TDEDiskDeviceType a, TDEDiskDeviceType b)
{
	return static_cast<TDEDiskDeviceType>(static_cast<unsigned long long>(a) & static_cast<unsigned long long>(b));
}

inline TDEDiskDeviceType operator~(TDEDiskDeviceType a)
{
	return static_cast<TDEDiskDeviceType>(~static_cast<unsigned long long>(a));
}
};

namespace TDEDiskDeviceStatus {
enum TDEDiskDeviceStatus {
	Null               = 0x00000000,
	Mountable          = 0x00000001,
	Removable          = 0x00000002,
	Inserted           = 0x00000004,
	Blank              = 0x00000008,
	UsedByDevice       = 0x00000010,
	UsesDevice         = 0x00000020,
	ContainsFilesystem = 0x00000040,
	Hotpluggable TDE_DEPRECATED = 0x00000080 , //< @deprecated; use Removable instead
	Hidden             = 0x00000100,
	Other              = 0x80000000
};

inline TDEDiskDeviceStatus operator|(TDEDiskDeviceStatus a, TDEDiskDeviceStatus b)
{
	return static_cast<TDEDiskDeviceStatus>(static_cast<int>(a) | static_cast<int>(b));
}

inline TDEDiskDeviceStatus operator&(TDEDiskDeviceStatus a, TDEDiskDeviceStatus b)
{
	return static_cast<TDEDiskDeviceStatus>(static_cast<int>(a) & static_cast<int>(b));
}

inline TDEDiskDeviceStatus operator~(TDEDiskDeviceStatus a)
{
	return static_cast<TDEDiskDeviceStatus>(~static_cast<int>(a));
}
};

namespace TDELUKSKeySlotStatus {
enum TDELUKSKeySlotStatus {
	Invalid  = 0x00000000,
	Inactive = 0x00000001,
	Active   = 0x00000002,
	Last     = 0x00000004,
	Other    = 0x80000000
};

inline TDELUKSKeySlotStatus operator|(TDELUKSKeySlotStatus a, TDELUKSKeySlotStatus b)
{
	return static_cast<TDELUKSKeySlotStatus>(static_cast<int>(a) | static_cast<int>(b));
}

inline TDELUKSKeySlotStatus operator&(TDELUKSKeySlotStatus a, TDELUKSKeySlotStatus b)
{
	return static_cast<TDELUKSKeySlotStatus>(static_cast<int>(a) & static_cast<int>(b));
}

inline TDELUKSKeySlotStatus operator~(TDELUKSKeySlotStatus a)
{
	return static_cast<TDELUKSKeySlotStatus>(~static_cast<int>(a));
}
};

typedef TQValueList<TDELUKSKeySlotStatus::TDELUKSKeySlotStatus> TDELUKSKeySlotStatusList;

namespace TDELUKSResult {
enum TDELUKSResult {
	Invalid          = 0x00000000,
	Success          = 0x00000001,
	LUKSNotSupported = 0x00000002,
	LUKSNotFound     = 0x00000003,
	InvalidKeyslot   = 0x00000004,
	KeyslotOpFailed  = 0x00000005,
	Other            = 0x80000000
};
};

typedef TQMap<TQString, TQString> TDEStorageMountOptions;

class TDECORE_EXPORT TDEStorageDevice : public TDEGenericDevice
{
	public:
		/**
		 *  Constructor.
		 *  @param Device type
		 */
		TDEStorageDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);

		/**
		 * Destructor.
		 */
		~TDEStorageDevice();

		/**
		 * @return a TQString with the disk or partition label, if any
		 */
		TQString diskLabel();

		/**
		 * @return a TQString with the disk UUID, if any
		 */
		TQString diskUUID();

		/**
		* @return a TQString with the device mapped name, if any
		*/
		TQString mappedName();

		/**
		 * @return an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		 */
		TDEDiskDeviceType::TDEDiskDeviceType diskType();

		/**
		 * @return an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		 */
		TDEDiskDeviceStatus::TDEDiskDeviceStatus diskStatus();

		/**
		 * @return true if media inserted, false if no media available
		 */
		bool mediaInserted();

		/**
		 * @return a TQString with the filesystem name, if any
		 */
		TQString fileSystemName();

		/**
		 * @return a TQString with the filesystem usage string, if any
		 */
		TQString fileSystemUsage();

		/**
		 * @return a TQStringList containing system paths to all devices with a lock on this device, if any
		 */
		TQStringList holdingDevices();

		/**
		 * @return a TQStringList containing system paths to all devices locked by this device, if any
		 */
		TQStringList slaveDevices();

		/**
		 * Mounts the device if not encrypted
		 *
		 * @param a TQString containing a requested mount name under /media, if desired
		 * @param a TDEStorageMountOptions containing any mount options for pmount, if desired
		 *
		 * @return a TQStringVariantMap object containing information about the operation outcome
		 */
		TQStringVariantMap mountDevice(TQString mediaName = TQString::null,
		    TDEStorageMountOptions mountOptions = TDEStorageMountOptions());

		/**
		 * Unmounts the device
		 *
		 * @return a TQStringVariantMap object containing information about the operation outcome
		 */
		TQStringVariantMap unmountDevice();

		/**
		 * Unlock/decrypt an encrypted storage device
		 *
		 * @param a TQString containing the passphrase
		 *
		 * @return a TQStringVariantMap object containing information about the operation outcome
		 */
		TQStringVariantMap unlockDevice(const TQString &passphrase);

		/**
		 * Lock/encrypt an encrypted storage device
		 *
		 * @return a TQStringVariantMap object containing information about the operation outcome
		 */
		TQStringVariantMap lockDevice();

		/**
		 * @return a TQString with the mount path, if mounted
		 */
		TQString mountPath();

		/**
		 * @return an unsigned long with the device size in bytes
		 */
		unsigned long long deviceSize();

		/**
		 * @return a TQString with the device size in human readable form
		 */
		TQString deviceFriendlySize();

		/**
		 * Get an icon for this device
		 * @param size a TDEIcon::StdSizes structure specifying the desired icon size
		 * @return a TQPixmap containing the icon for the specified type
		 *
		 * This method overrides TDEGenericDevice::icon(TDEIcon::StdSizes size)
		 */
		TQPixmap icon(TDEIcon::StdSizes size);

		/**
		 * @return a TQString with a friendly name
		 *
		 * This method overrides TDEGenericDevice::friendlyName()
		 */
		TQString friendlyName();

		/**
		 * @return a TQString with a detailed friendly name
		 *
		 * This method overrides TDEGenericDevice::detailedFriendlyName()
		 */
		TQString detailedFriendlyName();

		/**
		 *  @return a TQString containing the friendly type name
		 *
		 * This method overrides TDEGenericDevice::friendlyDeviceType()
		 */
		TQString friendlyDeviceType();

		/**
		 * @param an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		 */
		bool isDiskOfType(TDEDiskDeviceType::TDEDiskDeviceType tf);

		/**
		 * @param an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		 */
		bool checkDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus sf);

		/**
		 * @param true to engage media lock, false to disable it
		 * @return true on success, false on failure
		 *
		 * This method currently works on CD-ROM drives and similar devices
		 */
		bool lockDriveMedia(bool lock);

		/**
		 * @return true on success, false on failure
		 *
		 * This method currently works on CD-ROM drives and similar devices
		 */
		bool ejectDriveMedia();

		/**
		 * @return true on success, false on failure
		 *
		 * This method currently works on all removable storage devices
		 */
		TQStringVariantMap ejectDrive();

		/**
		 * Set the unlock password to use in subsequent LUKS operations
		 * @see cryptClearOperationsUnlockPassword
		 *
		 * @param password LUKS unlock password for any keyslot
		 */
		void cryptSetOperationsUnlockPassword(TQByteArray password);

		/**
		 * Erases the unlock password from application memory cache
		 * @see cryptSetOperationsUnlockPassword
		 */
		void cryptClearOperationsUnlockPassword();

		/**
		 * @return true if unlock password is in the application memory cache
		 * @see cryptSetOperationsUnlockPassword
		 * @see cryptClearOperationsUnlockPassword
		 */
		bool cryptOperationsUnlockPasswordSet();

		/**
		 * Checks the preloaded unlock password against the specified keyslot
		 *
		 * @param keyslot Keyslot number
		 * @return TDELUKSResult::TDELUKSResult containing the status code returned
		 * from the operation, or TDELUKSResult::LUKSNotSupported if LUKS support unavailable
		 * @return TDELUKSResult::Success on success
		 *
		 * @see cryptSetOperationsUnlockPassword
		 */
		TDELUKSResult::TDELUKSResult cryptCheckKey(unsigned int keyslot);

		/**
		 * Adds a new key to the specified keyslot, overwriting the existing key if present
		 *
		 * @param keyslot New keyslot number
		 * @param password New keyslot password
		 * @return TDELUKSResult::TDELUKSResult containing the status code returned
		 * from the operation, or TDELUKSResult::LUKSNotSupported if LUKS support unavailable
		 * @return TDELUKSResult::Success on success
		 *
		 * @see cryptSetOperationsUnlockPassword
		 */
		TDELUKSResult::TDELUKSResult cryptAddKey(unsigned int keyslot, TQByteArray password);

		/**
		 * Permanently deletes the associated key from a specific keyslot
		 *
		 * @param keyslot Keyslot number
		 * @return TDELUKSResult::TDELUKSResult containing the status code returned
		 * from the operation, or TDELUKSResult::LUKSNotSupported if LUKS support unavailable
		 * @return TDELUKSResult::Success on success
		 */
		TDELUKSResult::TDELUKSResult cryptDelKey(unsigned int keyslot);

		/**
		 * @return the maximum number of key slots available
		 */
		unsigned int cryptKeySlotCount();

		/**
		 * @return TDELUKSKeySlotStatusList with the status of all keyslots
		 */
		TDELUKSKeySlotStatusList cryptKeySlotStatus();

		/**
		 * @param status Keyslot status
		 * @return a TQString with the friendly name of the given slot status
		 */
		TQString cryptKeySlotFriendlyName(TDELUKSKeySlotStatus::TDELUKSKeySlotStatus status);

	protected:
		/**
		 * @param a TQString with the system device node, if any
		 * @internal
		 *
		 * This method is non-portable, so be careful!
		 */
		void internalSetDeviceNode(TQString dn);

		/**
		 * @param a TQString with the disk or partition label, if any
		 * @internal
		 */
		void internalSetDiskLabel(TQString dn);

		/**
		 * @param a TQString with the disk UUID, if any
		 * @internal
		 */
		void internalSetDiskUUID(TQString id);

		/**
		 * @param an OR-ed combination of TDEDiskDeviceType::TDEDiskDeviceType type flags
		 * @internal
		 */
		void internalSetDiskType(TDEDiskDeviceType::TDEDiskDeviceType tf);

		/**
		 * @param an OR-ed combination of TDEDiskDeviceStatus::TDEDiskDeviceStatus type flags
		 * @internal
		 */
		void internalSetDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus st);

		/**
		 * @param a bool with the media status
		 * @internal
		 */
		void internalSetMediaInserted(bool inserted);

		/**
		 * @param a TQString with the filesystem name, if any
		 * @internal
		 */
		void internalSetFileSystemName(TQString fn);

		/**
		 * @param a TQString with the filesystem usage string, if any
		 * @internal
		 */
		void internalSetFileSystemUsage(TQString fu);

		/**
		 * @param a TQStringList containing system paths to all devices with a lock on this device, if any
		 * @internal
		 */
		void internalSetHoldingDevices(TQStringList hd);

		/**
		 * @param a TQStringList containing system paths to all devices locked by this device, if any
		 * @internal
		 */
		void internalSetSlaveDevices(TQStringList sd);

		/**
		 * @internal
		 */
		void internalInitializeLUKSIfNeeded();

		/**
		 * @internal
		 */
		void internalGetLUKSKeySlotStatus();

		/**
		* Find and store the alternative mapped name of a mapped device
		*/
		void internalUpdateMappedName();

		/**
		* Find and store the mount point of the device, if any
		*/
		void internalUpdateMountPath();

	private:
		TQString m_mappedName;  // Alternative name for a mapped device
		TDEDiskDeviceType::TDEDiskDeviceType m_diskType;
		TDEDiskDeviceStatus::TDEDiskDeviceStatus m_diskStatus;
		TQString m_diskName;
		TQString m_diskUUID;
		TQString m_fileSystemName;
		TQString m_fileSystemUsage;
		bool m_mediaInserted;
		TQString m_mountPath;
		TQStringList m_holdingDevices;
		TQStringList m_slaveDevices;
		struct crypt_device* m_cryptDevice;
		TQByteArray m_cryptDevicePassword;
		TQString m_cryptDeviceType;
		unsigned int m_cryptKeySlotCount;
		TDELUKSKeySlotStatusList m_cryptKeyslotStatus;

	friend class TDEHardwareDevices;
};

#endif // _TDESTORAGEDEVICE_H
