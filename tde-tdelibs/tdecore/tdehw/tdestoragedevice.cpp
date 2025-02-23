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

#include "tdestoragedevice.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <linux/cdrom.h>

#include <tqregexp.h>
#include <tqpixmap.h>
#include <tqfile.h>

#include "kdebug.h"
#include "tdelocale.h"
#include "tdeglobal.h"
#include "kiconloader.h"
#include "tdetempfile.h"
#include "kstandarddirs.h"
#include "tdehardwaredevices.h"
#include "disksHelper.h"

#include "config.h"

#if defined(WITH_CRYPTSETUP)
	#ifdef CRYPTSETUP_OLD_API
		#define class cryptsetup_class
		#define CRYPT_SLOT_INACTIVE SLOT_INACTIVE
		#define CRYPT_SLOT_ACTIVE SLOT_ACTIVE
		#define CRYPT_SLOT_ACTIVE_LAST SLOT_ACTIVE_LAST
		#include <libcryptsetup.h>
		#undef class
	#else
		#include <libcryptsetup.h>
	#endif
#endif

TDEStorageDevice::TDEStorageDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn), m_mediaInserted(true), m_cryptDevice(NULL) {
	m_diskType = TDEDiskDeviceType::Null;
	m_diskStatus = TDEDiskDeviceStatus::Null;
}

TDEStorageDevice::~TDEStorageDevice() {
#if defined(WITH_CRYPTSETUP)
	if (m_cryptDevice) {
		crypt_free(m_cryptDevice);
		m_cryptDevice = NULL;
	}
#endif
}

TQString TDEStorageDevice::mappedName() {
	return m_mappedName;
}

void TDEStorageDevice::internalUpdateMappedName() {
	// Get the device mapped name if present
	m_mappedName = TQString::null;
	TQString dmnodename = systemPath();
	dmnodename.append("/dm/name");
	TQFile dmnamefile(dmnodename);
	if (dmnamefile.open(IO_ReadOnly)) {
		TQTextStream stream(&dmnamefile);
		m_mappedName = stream.readLine();
		dmnamefile.close();
	}
	if (!m_mappedName.isEmpty()) {
		m_mappedName.prepend("/dev/mapper/");
	}
}

TDEDiskDeviceType::TDEDiskDeviceType TDEStorageDevice::diskType() {
	return m_diskType;
}

void TDEStorageDevice::internalGetLUKSKeySlotStatus() {
#if defined(WITH_CRYPTSETUP)
	unsigned int i;
	crypt_keyslot_info keyslot_status;
	TDELUKSKeySlotStatus::TDELUKSKeySlotStatus tde_keyslot_status;

	m_cryptKeyslotStatus.clear();
	for (i = 0; i < m_cryptKeySlotCount; i++) {
		keyslot_status = crypt_keyslot_status(m_cryptDevice, i);
		tde_keyslot_status = TDELUKSKeySlotStatus::Invalid;
		if (keyslot_status == CRYPT_SLOT_INACTIVE) {
			tde_keyslot_status = TDELUKSKeySlotStatus::Inactive;
		}
		else if (keyslot_status == CRYPT_SLOT_ACTIVE) {
			tde_keyslot_status = TDELUKSKeySlotStatus::Active;
		}
		else if (keyslot_status == CRYPT_SLOT_ACTIVE_LAST) {
			tde_keyslot_status = TDELUKSKeySlotStatus::Active | TDELUKSKeySlotStatus::Last;
		}
		m_cryptKeyslotStatus.append(tde_keyslot_status);
	}
#endif
}

void TDEStorageDevice::internalInitializeLUKSIfNeeded() {
#if defined(WITH_CRYPTSETUP)
	int ret;

	if (m_diskType & TDEDiskDeviceType::LUKS) {
		if (!m_cryptDevice) {
			TQString node = deviceNode();
			if (node != "") {
				ret = crypt_init(&m_cryptDevice, node.ascii());
				if (ret == 0) {
					ret = crypt_load(m_cryptDevice, NULL, NULL);
					if (ret == 0) {
						int keyslot_count;
#if defined(CRYPTSETUP_OLD_API) || !defined(HAVE_CRYPTSETUP_GET_TYPE)
						kdWarning() << "TDEStorageDevice: The version of libcryptsetup that TDE was compiled against was too old!  Most LUKS features will not function" << endl;
						m_cryptDeviceType = TQString::null;
						keyslot_count = 0;
#else
						m_cryptDeviceType = crypt_get_type(m_cryptDevice);
						keyslot_count = crypt_keyslot_max(m_cryptDeviceType.ascii());
#endif
						if (keyslot_count < 0) {
							m_cryptKeySlotCount = 0;
						}
						else {
							m_cryptKeySlotCount = keyslot_count;
						}
						internalGetLUKSKeySlotStatus();
					}
				}
				else {
					m_cryptDevice = NULL;
				}
			}
		}
	}
	else {
		if (m_cryptDevice) {
			crypt_free(m_cryptDevice);
			m_cryptDevice = NULL;
		}
	}
#endif
}

void TDEStorageDevice::cryptSetOperationsUnlockPassword(TQByteArray password) {
#if defined(WITH_CRYPTSETUP)
	crypt_memory_lock(NULL, 1);
	m_cryptDevicePassword = password;
#endif
}

void TDEStorageDevice::cryptClearOperationsUnlockPassword() {
	m_cryptDevicePassword.fill(0);
	m_cryptDevicePassword.resize(0);
#if defined(WITH_CRYPTSETUP)
	crypt_memory_lock(NULL, 0);
#endif
}

bool TDEStorageDevice::cryptOperationsUnlockPasswordSet() {
	if (m_cryptDevicePassword.size() > 0) {
		return true;
	}
	else {
		return false;
	}
}

TDELUKSResult::TDELUKSResult TDEStorageDevice::cryptCheckKey(unsigned int keyslot) {
#if defined(WITH_CRYPTSETUP)
	int ret;

	if (m_cryptDevice) {
		if (keyslot < m_cryptKeySlotCount) {
			ret = crypt_activate_by_passphrase(m_cryptDevice, NULL, keyslot, m_cryptDevicePassword.data(), m_cryptDevicePassword.size(), 0);
			if (ret < 0) {
				return TDELUKSResult::KeyslotOpFailed;
			}
			else {
				return TDELUKSResult::Success;
			}
		}
		else {
			return TDELUKSResult::InvalidKeyslot;
		}
	}
	else {
		return TDELUKSResult::LUKSNotFound;
	}
#else
	return TDELUKSResult::LUKSNotSupported;
#endif
}

TDELUKSResult::TDELUKSResult TDEStorageDevice::cryptAddKey(unsigned int keyslot, TQByteArray password) {
#if defined(WITH_CRYPTSETUP)
	int ret;

	if (m_cryptDevice) {
		if (keyslot < m_cryptKeySlotCount) {
			ret = crypt_keyslot_add_by_passphrase(m_cryptDevice, keyslot, m_cryptDevicePassword.data(), m_cryptDevicePassword.size(), password.data(), password.size());
			if (ret < 0) {
				return TDELUKSResult::KeyslotOpFailed;
			}
			else {
				internalGetLUKSKeySlotStatus();
				return TDELUKSResult::Success;
			}
		}
		else {
			return TDELUKSResult::InvalidKeyslot;
		}
	}
	else {
		return TDELUKSResult::LUKSNotFound;
	}
#else
	return TDELUKSResult::LUKSNotSupported;
#endif
}

TDELUKSResult::TDELUKSResult TDEStorageDevice::cryptDelKey(unsigned int keyslot) {
#if defined(WITH_CRYPTSETUP)
	int ret;

	if (m_cryptDevice) {
		if (keyslot < m_cryptKeySlotCount) {
			ret = crypt_keyslot_destroy(m_cryptDevice, keyslot);
			if (ret < 0) {
				return TDELUKSResult::KeyslotOpFailed;
			}
			else {
				internalGetLUKSKeySlotStatus();
				return TDELUKSResult::Success;
			}
		}
		else {
			return TDELUKSResult::InvalidKeyslot;
		}
	}
	else {
		return TDELUKSResult::LUKSNotFound;
	}
#else
	return TDELUKSResult::LUKSNotSupported;
#endif
}

unsigned int TDEStorageDevice::cryptKeySlotCount() {
	return m_cryptKeySlotCount;
}

TDELUKSKeySlotStatusList TDEStorageDevice::cryptKeySlotStatus() {
	return m_cryptKeyslotStatus;
}

TQString TDEStorageDevice::cryptKeySlotFriendlyName(TDELUKSKeySlotStatus::TDELUKSKeySlotStatus status) {
	if (status & TDELUKSKeySlotStatus::Inactive) {
		return i18n("Inactive");
	}
	else if (status & TDELUKSKeySlotStatus::Active) {
		return i18n("Active");
	}
	else {
		return i18n("Unknown");
	}
}

void TDEStorageDevice::internalSetDeviceNode(TQString dn) {
	TDEGenericDevice::internalSetDeviceNode(dn);
	internalInitializeLUKSIfNeeded();
}

void TDEStorageDevice::internalSetDiskType(TDEDiskDeviceType::TDEDiskDeviceType dt) {
	m_diskType = dt;
	internalInitializeLUKSIfNeeded();
}

bool TDEStorageDevice::isDiskOfType(TDEDiskDeviceType::TDEDiskDeviceType tf) {
	return ((m_diskType&tf)!=TDEDiskDeviceType::Null);
}

TDEDiskDeviceStatus::TDEDiskDeviceStatus TDEStorageDevice::diskStatus() {
	return m_diskStatus;
}

void TDEStorageDevice::internalSetDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus st) {
	m_diskStatus = st;
}

bool TDEStorageDevice::checkDiskStatus(TDEDiskDeviceStatus::TDEDiskDeviceStatus sf) {
	return ((m_diskStatus&sf)!=(TDEDiskDeviceStatus::TDEDiskDeviceStatus)0);
}

bool TDEStorageDevice::lockDriveMedia(bool lock) {
	int fd = open(deviceNode().ascii(), O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		return false;
	}
	if (ioctl(fd, CDROM_LOCKDOOR, (lock)?1:0) != 0) {
		close(fd);
		return false;
	}
	else {
		close(fd);
		return true;
	}
}

TQStringVariantMap TDEStorageDevice::ejectDrive() {
	TQStringVariantMap result;
	TQStringVariantMap ejectResult;

	// If the device is mounted, try unmounting it first
	if (!mountPath().isEmpty()) {
		unmountDevice();
	}

#ifdef WITH_UDISKS2
	if (!(TDEGlobal::dirs()->findExe("udisksctl").isEmpty())) {
		ejectResult = udisks2EjectDrive(this);
		if (ejectResult["result"].toBool()) {
			result["result"] = true;
			return result;
		}
		else {
			result["errStr"] = ejectResult["errStr"];
			result["result"] = false;
			return result;
		}
	}
#endif
#ifdef WITH_UDISKS
	if (!(TDEGlobal::dirs()->findExe("udisks").isEmpty())) {
		ejectResult = udisksEjectDrive(this);
		if (ejectResult["result"].toBool()) {
			result["result"] = true;
			return result;
		}
		else {
			result["errStr"] = ejectResult["errStr"];
			result["result"] = false;
			return result;
		}
	}
#endif

	if (!(TDEGlobal::dirs()->findExe("eject").isEmpty())) {
		TQString command = TQString("eject -v '%1' 2>&1").arg(deviceNode());

		FILE *exepipe = popen(command.ascii(), "r");
		if (exepipe) {
			TQString eject_output;
			TQTextStream ts(exepipe, IO_ReadOnly);
			eject_output = ts.read();
			int retcode = pclose(exepipe);
			if (retcode == 0) {
				result["result"] = true;
				return result;
			}
			else {
				result["errStr"] = eject_output;
				result["retCode"] = retcode;
			}
		}
	}

	result["result"] = false;
	return result;
}

bool TDEStorageDevice::ejectDriveMedia() {
	int fd = open(deviceNode().ascii(), O_RDWR | O_NONBLOCK);
	if (fd < 0) {
		return false;
	}
	if (ioctl(fd, CDROMEJECT) != 0) {
		close(fd);
		return false;
	}
	else {
		close(fd);
		return true;
	}
}

TQString TDEStorageDevice::diskLabel() {
	return m_diskName;
}

void TDEStorageDevice::internalSetDiskLabel(TQString dn) {
	m_diskName = dn;
}

bool TDEStorageDevice::mediaInserted() {
	return m_mediaInserted;
}

void TDEStorageDevice::internalSetMediaInserted(bool inserted) {
	m_mediaInserted = inserted;
}

TQString TDEStorageDevice::fileSystemName() {
	return m_fileSystemName;
}

void TDEStorageDevice::internalSetFileSystemName(TQString fn) {
	m_fileSystemName = fn;
}

TQString TDEStorageDevice::fileSystemUsage() {
	return m_fileSystemUsage;
}

void TDEStorageDevice::internalSetFileSystemUsage(TQString fu) {
	m_fileSystemUsage = fu;
}

TQString TDEStorageDevice::diskUUID() {
	return m_diskUUID;
}

void TDEStorageDevice::internalSetDiskUUID(TQString id) {
	m_diskUUID = id;
}

TQStringList TDEStorageDevice::holdingDevices() {
	return m_holdingDevices;
}

void TDEStorageDevice::internalSetHoldingDevices(TQStringList hd) {
	m_holdingDevices = hd;
}

TQStringList TDEStorageDevice::slaveDevices() {
	return m_slaveDevices;
}

void TDEStorageDevice::internalSetSlaveDevices(TQStringList sd) {
	m_slaveDevices = sd;
}

TQString decodeHexEncoding(TQString str) {
	TQRegExp hexEncRegExp("\\\\x[0-9A-Fa-f]{1,2}");
	hexEncRegExp.setMinimal(false);
	hexEncRegExp.setCaseSensitive(true);
	int s = -1;

	while((s = hexEncRegExp.search(str, s+1))>=0){
		str.replace(s, hexEncRegExp.cap(0).length(), TQChar((char)strtol(hexEncRegExp.cap(0).mid(2).ascii(), NULL, 16)));
	}

	return str;
}

TQString TDEStorageDevice::friendlyName() {
	// Return the actual storage device name
	TQString devicevendorid = vendorEncoded();
	TQString devicemodelid = modelEncoded();

	devicevendorid = decodeHexEncoding(devicevendorid);
	devicemodelid = decodeHexEncoding(devicemodelid);

	devicevendorid = devicevendorid.stripWhiteSpace();
	devicemodelid = devicemodelid.stripWhiteSpace();
	devicevendorid = devicevendorid.simplifyWhiteSpace();
	devicemodelid = devicemodelid.simplifyWhiteSpace();

	TQString devicename = devicevendorid + " " + devicemodelid;

	devicename = devicename.stripWhiteSpace();
	devicename = devicename.simplifyWhiteSpace();

	if (devicename != "") {
		return devicename;
	}

	if (isDiskOfType(TDEDiskDeviceType::Camera)) {
		return TDEGenericDevice::friendlyName();
	}

	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		return friendlyDeviceType();
	}

	TQString label = diskLabel();
	if (label.isNull()) {
		if (deviceSize() > 0) {
			if (checkDiskStatus(TDEDiskDeviceStatus::Removable)) {
				label = i18n("%1 Removable Device").arg(deviceFriendlySize());
			}
			else {
				label = i18n("%1 Fixed Storage Device").arg(deviceFriendlySize());
			}
		}
	}

	if (!label.isNull()) {
		return label;
	}

	return friendlyDeviceType();
}

TQString TDEStorageDevice::detailedFriendlyName() {
	return TQString("%1 [%2]").arg(friendlyName()).arg(deviceNode());
}

TQString TDEStorageDevice::friendlyDeviceType() {
	TQString ret = i18n("Hard Disk Drive");

	// Keep this in sync with TDEStorageDevice::icon(TDEIcon::StdSizes size) below
	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		ret = i18n("Floppy Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Optical)) {
		ret = i18n("Optical Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::CDROM)) {
		ret = i18n("CDROM Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::CDRW)) {
		ret = i18n("CDRW Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDROM)) {
		ret = i18n("DVD Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRW)) {
		ret = i18n("DVDRW Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRAM)) {
		ret = i18n("DVDRAM Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Zip)) {
		ret = i18n("Zip Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Tape)) {
		ret = i18n("Tape Drive");
	}
	if (isDiskOfType(TDEDiskDeviceType::Camera)) {
		ret = i18n("Digital Camera");
	}

	if (isDiskOfType(TDEDiskDeviceType::HDD)) {
		ret = i18n("Hard Disk Drive");
		if (checkDiskStatus(TDEDiskDeviceStatus::Removable)) {
			ret = i18n("Removable Storage");
		}
		if (isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
			ret = i18n("Compact Flash");
		}
		if (isDiskOfType(TDEDiskDeviceType::MemoryStick)) {
			ret = i18n("Memory Stick");
		}
		if (isDiskOfType(TDEDiskDeviceType::SmartMedia)) {
			ret = i18n("Smart Media");
		}
		if (isDiskOfType(TDEDiskDeviceType::SDMMC)) {
			ret = i18n("Secure Digital");
		}
	}

	if (isDiskOfType(TDEDiskDeviceType::RAM)) {
		ret = i18n("Random Access Memory");
	}
	if (isDiskOfType(TDEDiskDeviceType::Loop)) {
		ret = i18n("Loop Device");
	}

	return ret;
}

TQPixmap TDEStorageDevice::icon(TDEIcon::StdSizes size) {
	TQString mountString;
	if (mountPath() != TQString::null) {
		mountString = "-mounted";
	}
	else {
		mountString = "-unmounted";
	}

	TQPixmap ret = DesktopIcon("drive-harddisk" + mountString, size);

	if (isDiskOfType(TDEDiskDeviceType::Floppy)) {
		ret = DesktopIcon("media-floppy-3_5" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Optical)) {
		ret = DesktopIcon("media-optical-cdrom" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::CDROM)) {
		ret = DesktopIcon("media-optical-cdrom" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::CDRW)) {
		ret = DesktopIcon("cd-rw" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDROM)) {
		ret = DesktopIcon("media-optical-dvd" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRW)) {
		ret = DesktopIcon("media-optical-dvd" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::DVDRAM)) {
		ret = DesktopIcon("media-optical-dvd" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Zip)) {
		ret = DesktopIcon("media-floppy-zip" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Tape)) {
		ret = DesktopIcon("media-tape" + mountString, size);
	}
	if (isDiskOfType(TDEDiskDeviceType::Camera)) {
		ret = DesktopIcon("camera" + mountString, size);
	}

	if (isDiskOfType(TDEDiskDeviceType::HDD)) {
		ret = DesktopIcon("drive-harddisk" + mountString, size);
		if (checkDiskStatus(TDEDiskDeviceStatus::Removable)) {
			ret = DesktopIcon("media-flash-usb" + mountString, size);
		}
		if (isDiskOfType(TDEDiskDeviceType::CompactFlash)) {
			ret = DesktopIcon("media-flash-compact_flash" + mountString, size);
		}
		if (isDiskOfType(TDEDiskDeviceType::MemoryStick)) {
			ret = DesktopIcon("media-flash-memory_stick" + mountString, size);
		}
		if (isDiskOfType(TDEDiskDeviceType::SmartMedia)) {
			ret = DesktopIcon("media-flash-smart_media" + mountString, size);
		}
		if (isDiskOfType(TDEDiskDeviceType::SDMMC)) {
			ret = DesktopIcon("media-flash-sd_mmc" + mountString, size);
		}
	}

	if (isDiskOfType(TDEDiskDeviceType::RAM)) {
		ret = DesktopIcon("memory", size);  // FIXME there is only a single icon available for this device
	}
	if (isDiskOfType(TDEDiskDeviceType::Loop)) {
		ret = DesktopIcon("blockdevice", size);  // FIXME there is only a single icon available for this device
	}

	return ret;
}

unsigned long long TDEStorageDevice::deviceSize() {
	TQString bsnodename = systemPath();
	// While at first glance it would seem that checking /queue/physical_block_size would be needed to get an accurate device size, in reality Linux
	// appears to only ever report the device size in 512 byte units.  This does not appear to be documented anywhere!
	TQString blocksize = "512";

	TQString dsnodename = systemPath();
	dsnodename.append("/size");
	TQFile dsfile( dsnodename );
	TQString devicesize;
	if ( dsfile.open( IO_ReadOnly ) ) {
		TQTextStream stream( &dsfile );
		devicesize = stream.readLine();
		dsfile.close();
	}

	return ((unsigned long long)blocksize.toULong()*(unsigned long long)devicesize.toULong());
}

TQString TDEStorageDevice::deviceFriendlySize() {
	return TDEHardwareDevices::bytesToFriendlySizeString(deviceSize());
}

TQString TDEStorageDevice::mountPath()
{
	return m_mountPath;
}

void TDEStorageDevice::internalUpdateMountPath()
{
	// See if this device node is mounted
	// This requires parsing /proc/mounts, looking for deviceNode()

	// The Device Mapper throws a monkey wrench into this
	// It likes to advertise mounts as /dev/mapper/<something>,
	// where <something> is listed in <system path>/dm/name

	// Assumed all device information (mainly holders/slaves) is accurate
	// prior to the call

	m_mountPath = TQString::null;

	TQStringList lines;
	TQFile file( "/proc/mounts" );
	if ( file.open( IO_ReadOnly ) ) {
		TQTextStream stream( &file );
		TQString line;
		while ( !stream.atEnd() ) {
			line = stream.readLine();
			TQStringList mountInfo = TQStringList::split(" ", line, true);
			TQString testNode = *mountInfo.at(0);
			// Check for match
			if ((testNode == deviceNode()) || (testNode == mappedName()) || (testNode == ("/dev/disk/by-uuid/" + diskUUID()))) {
				m_mountPath = *mountInfo.at(1);
				m_mountPath.replace("\\040", " ");
				file.close();
				return;
			}
			lines += line;
		}
		file.close();
	}
}

TQStringVariantMap TDEStorageDevice::mountDevice(TQString mediaName, TDEStorageMountOptions mountOptions) {
	TQStringVariantMap result;

	// Check if device is already mounted
	TQString mountpath = mountPath();
	if (!mountpath.isEmpty()) {
		result["mountPath"] = mountpath;
		result["result"] = true;
		return result;
	}

	TQString devNode = deviceNode();
	devNode.replace("'", "'\\''");
	mediaName.replace("'", "'\\''");
	TQString command = TQString::null;

#if defined(WITH_UDISKS2) || defined(WITH_UDISKS) || defined(WITH_UDEVIL)
	// Prepare filesystem options for mount
	TQStringList udisksOptions;
	if (mountOptions["ro"] == "true") {
		udisksOptions.append("ro");
	}

	if (mountOptions["atime"] != "true") {
		udisksOptions.append("noatime");
	}

	if (mountOptions["sync"] == "true") {
		udisksOptions.append("sync");
	}

	if ((mountOptions["filesystem"] == "fat") || (mountOptions["filesystem"] == "vfat") ||
	    (mountOptions["filesystem"] == "msdos") || (mountOptions["filesystem"] == "umsdos")) {
		if (mountOptions.contains("shortname")) {
			udisksOptions.append(TQString("shortname=%1").arg(mountOptions["shortname"]));
		}
	}

	if ((mountOptions["filesystem"] == "jfs")) {
		if (mountOptions["utf8"] == "true") {
			// udisks/udisks2 for now does not support option iocharset= for jfs
			// udisksOptions.append("iocharset=utf8");
		}
	}

	if ((mountOptions["filesystem"] == "ntfs-3g")) {
		if (mountOptions.contains("locale")) {
			udisksOptions.append(TQString("locale=%1").arg(mountOptions["locale"]));
		}
	}

	if ((mountOptions["filesystem"] == "ext3") || (mountOptions["filesystem"] == "ext4")) {
		if (mountOptions.contains("journaling")) {
			// udisks/udisks2 for now does not support option data= for ext3/ext4
			// udisksOptions.append(TQString("data=%1").arg(mountOptions["journaling"]));
		}
	}

	TQString optionString;
	for (TQStringList::Iterator it = udisksOptions.begin(); it != udisksOptions.end(); ++it) {
		optionString.append(",");
		optionString.append(*it);
	}

	if (!optionString.isEmpty()) {
		optionString.remove(0, 1);
	}

	TQString fileSystemType;
	if (mountOptions.contains("filesystem") && !mountOptions["filesystem"].isEmpty()) {
		fileSystemType = mountOptions["filesystem"];
	}

	TQStringVariantMap mountResult;
#endif

#if defined(WITH_UDISKS2)
	// Try to use UDISKS v2 via DBUS, if available
	mountResult = udisks2MountDrive(devNode, fileSystemType, optionString);
	if (mountResult["result"].toBool()) {
		// Update internal mount data
		TDEGlobal::hardwareDevices()->processModifiedMounts();
		result["mountPath"] = mountPath();
		result["result"] = true;
		return result;
	}
	else if (mountResult["retcode"].toInt() == -1) {
		// Update internal mount data
		TDEGlobal::hardwareDevices()->processModifiedMounts();
		result["errStr"] = mountResult["errStr"];
		result["result"] = false;
		return result;
	}
#endif

#if defined(WITH_UDISKS)
	// The UDISKS v2 DBUS service was either not available or was unusable
	// Try to use UDISKS v1 via DBUS, if available
	mountResult = udisksMountDrive(devNode, fileSystemType, udisksOptions);
	if (mountResult["result"].toBool()) {
		// Update internal mount data
		TDEGlobal::hardwareDevices()->processModifiedMounts();
		result["mountPath"] = mountPath();
		result["result"] = true;
		return result;
	}
	else if (mountResult["retcode"].toInt() == -1) {
		// Update internal mount data
		TDEGlobal::hardwareDevices()->processModifiedMounts();
		result["errStr"] = mountResult["errStr"];
		result["result"] = false;
		return result;
	}
#endif

#if defined(WITH_UDEVIL)
	// The UDISKS v1 DBUS service was either not available or was unusable
	// Use 'udevil' command, if available
	if (!TDEGlobal::dirs()->findExe("udevil").isEmpty()) {
		if (mountOptions.contains("filesystem") && !mountOptions["filesystem"].isEmpty()) {
			fileSystemType = TQString("-t %1").arg(mountOptions["filesystem"]);
		}
		TQString mountpoint;
		if (mountOptions.contains("mountpoint") && !mountOptions["mountpoint"].isEmpty() &&
			  (mountOptions["mountpoint"] != "/media/")) {
			mountpoint = mountOptions["mountpoint"];
			mountpoint.replace("'", "'\\''");
		}
		else {
			mountpoint = TQString("/media/%1").arg(mediaName);
		}
		command = TQString("udevil mount %1 -o '%2' '%3' '%4' 2>&1")
		          .arg(fileSystemType).arg(optionString).arg(devNode).arg(mountpoint);
	}
#endif

	// If no other method was found, use 'pmount' command if available
	if(command.isEmpty()) {
		if (!TDEGlobal::dirs()->findExe("pmount").isEmpty()) {
			TQString optionString;
			if (mountOptions["ro"] == "true") {
				optionString.append(" -r");
			}

			if (mountOptions["atime"] != "true") {
				optionString.append(" -A");
			}

			if (mountOptions["utf8"] == "true") {
				optionString.append(" -c utf8");
			}

			if (mountOptions["sync"] == "true") {
				optionString.append(" -s");
			}

			if (mountOptions.contains("filesystem") && !mountOptions["filesystem"].isEmpty()) {
				optionString.append(TQString(" -t %1").arg(mountOptions["filesystem"]));
			}

			if (mountOptions.contains("locale")) {
				optionString.append(TQString(" -c %1").arg(mountOptions["locale"]));
			}

			TQString mountpoint;
			if (mountOptions.contains("mountpoint") && !mountOptions["mountpoint"].isEmpty() &&
				  (mountOptions["mountpoint"] != "/media/")) {
				mountpoint = mountOptions["mountpoint"];
				mountpoint.replace("'", "'\\''");
			}
			else {
				mountpoint = mediaName;
			}

			// %1 (option string) without quotes, otherwise pmount fails
			command = TQString("pmount %1 '%2' '%3' 2>&1")
			          .arg(optionString).arg(devNode).arg(mountpoint);
		}
	}

	if(command.isEmpty()) {
		result["errStr"] = i18n("No supported mounting methods were detected on your system");
		result["result"] = false;
		return result;
	}

	FILE *exepipe = popen(command.local8Bit(), "r");
	if (exepipe) {
		TQTextStream* ts = new TQTextStream(exepipe, IO_ReadOnly);
		TQString mount_output = ts->read();
		delete ts;
		int retcode = pclose(exepipe);
		result["errStr"] = mount_output;
		result["retCode"] = retcode;
	}

	// Update internal mount data
	TDEGlobal::hardwareDevices()->processModifiedMounts();
	result["mountPath"] = mountPath();
	result["result"] = !mountPath().isEmpty();
	return result;
}

TQStringVariantMap TDEStorageDevice::unmountDevice() {
	TQStringVariantMap result;

	// Check if device is already unmounted
	TQString mountpath = mountPath();
	if (mountpath.isEmpty()) {
		result["result"] = true;
		return result;
	}

	mountpath.replace("'", "'\\''");
	TQString devNode = deviceNode();
	TQString command = TQString::null;
	TQStringVariantMap unmountResult;

#if defined(WITH_UDISKS2)
	// Try to use UDISKS v2 via DBUS, if available
	unmountResult = udisks2UnmountDrive(devNode, TQString::null);
	if (unmountResult["result"].toBool()) {
		// Update internal mount data
		TDEGlobal::hardwareDevices()->processModifiedMounts();
		result["result"] = true;
		return result;
	}
	else if (unmountResult["retcode"].toInt() == -1) {
		// Update internal mount data
		TDEGlobal::hardwareDevices()->processModifiedMounts();
		result["errStr"] = unmountResult["errStr"];
		result["result"] = false;
		return result;
	}
#endif

#if defined(WITH_UDISKS)
	// The UDISKS v2 DBUS service was either not available or was unusable
	// Try to use UDISKS v1 via DBUS, if available
	unmountResult = udisksUnmountDrive(devNode, TQStringList());
	if (unmountResult["result"].toBool()) {
		// Update internal mount data
		TDEGlobal::hardwareDevices()->processModifiedMounts();
		result["result"] = true;
		return result;
	}
	else if (unmountResult["retcode"].toInt() == -1) {
		// Update internal mount data
		TDEGlobal::hardwareDevices()->processModifiedMounts();
		result["errStr"] = unmountResult["errStr"];
		result["result"] = false;
		return result;
	}
#endif

#if defined(WITH_UDEVIL)
	// The UDISKS v1 DBUS service was either not available or was unusable
	// Use 'udevil' command, if available
	if (!TDEGlobal::dirs()->findExe("udevil").isEmpty()) {
		command = TQString("udevil umount '%1' 2>&1").arg(mountpath);
	}
#endif

	// If no other method was found, use 'pmount' command if available
	if(command.isEmpty() && !TDEGlobal::dirs()->findExe("pumount").isEmpty()) {
		command = TQString("pumount '%1' 2>&1").arg(mountpath);
	}

	if(command.isEmpty()) {
		result["errStr"] = i18n("No supported unmounting methods were detected on your system");
		result["result"] = false;
		return result;
	}

	FILE *exepipe = popen(command.local8Bit(), "r");
	if (exepipe) {
		TQTextStream* ts = new TQTextStream(exepipe, IO_ReadOnly);
		TQString umount_output = ts->read();
		delete ts;
		int retcode = pclose(exepipe);
		if (retcode == 0) {
			// Update internal mount data
			TDEGlobal::hardwareDevices()->processModifiedMounts();
			result["result"] = true;
			return result;
		}
		else {
			result["errStr"] = umount_output;
			result["retCode"] = retcode;
		}
	}

	// Update internal mount data
	TDEGlobal::hardwareDevices()->processModifiedMounts();
	result["result"] = false;
	return result;
}

TQStringVariantMap TDEStorageDevice::unlockDevice(const TQString &passphrase)
{
	TQStringVariantMap result;

	TQString devNode = deviceNode();
	devNode.replace("'", "'\\''");

  TQStringVariantMap unlockResult;

#if defined(WITH_UDISKS2)
	// Try to use UDISKS v2 via DBUS, if available
	unlockResult = udisks2UnlockDrive(devNode, passphrase);
	if (unlockResult["result"].toBool()) {
		result["unlockedDevice"] = unlockResult["unlockedDevice"];
		result["result"] = true;
		return result;
	}
	else if (unlockResult["retcode"].toInt() == -1) {
		result["errStr"] = unlockResult["errStr"];
		result["result"] = false;
		return result;
	}
#endif

	// If no other method was found, use 'pmount' command if available
	if (!TDEGlobal::dirs()->findExe("pmount").isEmpty()) {
		// Create dummy password file
		KTempFile passwordFile(TQString::null, "tmp", 0600);
		passwordFile.setAutoDelete(true);
		TQFile *pwFile = passwordFile.file();
		if (!pwFile) {
			result["errStr"] = i18n("Cannot create temporary password file");
			result["result"] = false;
			return result;
		}
		pwFile->writeBlock(passphrase.local8Bit(), passphrase.length());
		pwFile->flush();
		TQString passFileName = passwordFile.name();
		passFileName.replace("'", "'\\''");

		TQString command = TQString("pmount -p '%1' '%2'").arg(passFileName).arg(devNode);
		FILE *exepipe = popen(command.local8Bit(), "r");
		if (exepipe) {
			TQTextStream* ts = new TQTextStream(exepipe, IO_ReadOnly);
			TQString unlock_output = ts->read();
			delete ts;
			int retcode = pclose(exepipe);
			if (retcode == 0) {
				result["result"] = true;
			}
			else {
				result["errStr"] = unlock_output;
				result["retCode"] = retcode;
				result["result"] = false;
			}
			return result;
		}
	}

	// No supported methods found for unlocking the device
	result["errStr"] = i18n("No supported unlocking methods were detected on your system.");
	result["result"] = false;
	return result;
}

TQStringVariantMap TDEStorageDevice::lockDevice()
{
	TQStringVariantMap result;

	TQString devNode = deviceNode();
	devNode.replace("'", "'\\''");

  TQStringVariantMap lockResult;

#if defined(WITH_UDISKS2)
	// Try to use UDISKS v2 via DBUS, if available
	lockResult = udisks2LockDrive(devNode);
	if (lockResult["result"].toBool()) {
		result["result"] = true;
		return result;
	}
	else if (lockResult["retcode"].toInt() == -1) {
		result["errStr"] = lockResult["errStr"];
		result["result"] = false;
		return result;
	}
#endif

	// If no other method was found, use 'pumount' command if available
	if (!TDEGlobal::dirs()->findExe("pumount").isEmpty()) {
		TQString command = TQString("pumount '%1'").arg(devNode);
		FILE *exepipe = popen(command.local8Bit(), "r");
		if (exepipe) {
			TQTextStream* ts = new TQTextStream(exepipe, IO_ReadOnly);
			TQString lock_output = ts->read();
			delete ts;
			int retcode = pclose(exepipe);
			if (retcode == 0) {
				result["result"] = true;
			}
			else {
				result["errStr"] = lock_output;
				result["retCode"] = retcode;
				result["result"] = false;
			}
			return result;
		}
	}

	// No supported methods found for locking the device
	result["errStr"] = i18n("No supported locking methods were detected on your system.");
	result["result"] = false;
	return result;
}

#include "tdestoragedevice.moc"
