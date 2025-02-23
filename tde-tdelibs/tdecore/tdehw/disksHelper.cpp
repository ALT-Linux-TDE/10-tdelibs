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

#if defined(WITH_UDISKS) || defined(WITH_UDISKS2)
	#include <tqdbusdata.h>
	#include <tqdbusmessage.h>
	#include <tqdbusproxy.h>
	#include <tqdbusvariant.h>
	#include <tqdbusconnection.h>
	#include <tqdbuserror.h>
	#include <tqdbusdatamap.h>
	#include <tqdbusobjectpath.h>
	#include "tqdbusdatalist.h"
	#include "tqstring.h"

	#include "tdelocale.h"
	#include "tdestoragedevice.h"
	#include "disksHelper.h"
#endif


#ifdef WITH_UDISKS
//-------------------------------
//  UDisks
//-------------------------------
TQStringVariantMap udisksEjectDrive(TDEStorageDevice *sdevice) {
	TQStringVariantMap result;
	result["result"] = false;

	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = sdevice->deviceNode();
		blockDeviceString.replace("/dev/", "");
		blockDeviceString.replace("-", "_2d");
		blockDeviceString = "/org/freedesktop/UDisks/devices/" + blockDeviceString;

		// Eject the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks", blockDeviceString, "org.freedesktop.UDisks.Device", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			TQT_DBusDataList options;
			params << TQT_DBusData::fromList(options);
			TQT_DBusMessage reply = driveControl.sendWithReply("DriveEject", params, &error);
			if (error.isValid()) {
				// Error!
				result["errStr"] = error.name() + ": " + error.message();
				return result;
			}
			else {
				// Eject was successful. Check if the media can be powered off and do so in case
				TQT_DBusProxy driveInformation("org.freedesktop.UDisks", blockDeviceString,
						"org.freedesktop.DBus.Properties", dbusConn);
				params.clear();
				params << TQT_DBusData::fromString("org.freedesktop.UDisks.Drive") << TQT_DBusData::fromString("DriveCanDetach");
				TQT_DBusMessage reply = driveInformation.sendWithReply("Get", params, &error);
				if (error.isValid()) {
					// Error!
					result["errStr"] = error.name() + ": " + error.message();
					return result;
				}

				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					bool canPowerOff = reply[0].toVariant().value.toBool();
					if (!canPowerOff) {
						// This drive does not support power off. Just return since the eject operation has finished.
						result["result"] = true;
						return result;
					}

					// Power off the drive!
					params.clear();
					TQT_DBusDataMap<TQString> options(TQT_DBusData::Variant);
					params << TQT_DBusData::fromStringKeyMap(options);
					TQT_DBusMessage reply = driveControl.sendWithReply("DriveDetach", params, &error);
					if (error.isValid()) {
						// Error!
						result["errStr"] = error.name() + ": " + error.message();
						return result;
					}
					else {
						result["result"] = true;
						return result;
					}
				}
			}
		}
	}
	return result;
}

TQStringVariantMap udisksMountDrive(const TQString &deviceNode, const TQString &fileSystemType, TQStringList mountOptions) {
	TQStringVariantMap result;
	result["result"] = false;
	result["retcode"] = -2;

	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString.replace("-", "_2d");
		blockDeviceString = "/org/freedesktop/UDisks/devices/" + blockDeviceString;

		// Mount the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks", blockDeviceString, "org.freedesktop.UDisks.Device", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			params << TQT_DBusData::fromString(fileSystemType);
			params << TQT_DBusData::fromList(TQT_DBusDataList(mountOptions));
			TQT_DBusMessage reply = driveControl.sendWithReply("FilesystemMount", params, &error);
			if (!error.isValid()) {
				// Success
				result["retcode"] = 0;
				result["result"] = true;
				return result;
			}
			else {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					return result;  // Service not installed or unavailable
				}
				else {
					result["errStr"] = error.name() + ": " + error.message();
					result["retcode"] = -1;
					return result;
				}
			}
		}
	}
	return result;
}

TQStringVariantMap udisksUnmountDrive(const TQString &deviceNode, TQStringList unmountOptions) {
	TQStringVariantMap result;
	result["result"] = false;
	result["retcode"] = -2;

	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString.replace("-", "_2d");
		blockDeviceString = "/org/freedesktop/UDisks/devices/" + blockDeviceString;

		// Mount the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks", blockDeviceString, "org.freedesktop.UDisks.Device", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			params << TQT_DBusData::fromList(TQT_DBusDataList(unmountOptions));
			TQT_DBusMessage reply = driveControl.sendWithReply("FilesystemUnmount", params, &error);
			if (!error.isValid()) {
				// Success
				result["retcode"] = 0;
				result["result"] = true;
				return result;
			}
			else {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					return result;  // Service not installed or unavailable
				}
				else {
					result["errStr"] = error.name() + ": " + error.message();
					result["retcode"] = -1;
					return result;
				}
			}
		}
	}
	return result;
}
#endif


#ifdef WITH_UDISKS2
//-------------------------------
//  UDisks2
//-------------------------------
TQStringVariantMap udisks2EjectDrive(TDEStorageDevice *sdevice) {
	TQStringVariantMap result;
	result["result"] = false;

	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = sdevice->deviceNode();
		blockDeviceString.replace("/dev/", "");
		blockDeviceString.replace("-", "_2d");
		blockDeviceString = "/org/freedesktop/UDisks2/block_devices/" + blockDeviceString;
		TQT_DBusProxy hardwareControl("org.freedesktop.UDisks2", blockDeviceString, "org.freedesktop.DBus.Properties", dbusConn);
		if (hardwareControl.canSend()) {
			// get associated udisks2 drive path
			TQT_DBusError error;
			TQValueList<TQT_DBusData> params;
			params << TQT_DBusData::fromString("org.freedesktop.UDisks2.Block") << TQT_DBusData::fromString("Drive");
			TQT_DBusMessage reply = hardwareControl.sendWithReply("Get", params, &error);
			if (error.isValid()) {
				// Error!
				result["errStr"] = error.name() + ": " + error.message();
				return result;
			}

			if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
				TQT_DBusObjectPath driveObjectPath = reply[0].toVariant().value.toObjectPath();
				if (!driveObjectPath.isValid()) {
					return result;
				}
				error = TQT_DBusError();
				TQT_DBusProxy driveInformation("org.freedesktop.UDisks2", driveObjectPath,
								"org.freedesktop.DBus.Properties", dbusConn);
				// can eject?
				params.clear();
				params << TQT_DBusData::fromString("org.freedesktop.UDisks2.Drive") << TQT_DBusData::fromString("Ejectable");
				TQT_DBusMessage reply = driveInformation.sendWithReply("Get", params, &error);
				if (error.isValid()) {
					// Error!
					result["errStr"] = error.name() + ": " + error.message();
					return result;
				}

				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					bool ejectable = reply[0].toVariant().value.toBool();
					if (!ejectable) {
						result["errStr"] = i18n("Media not ejectable");
						return result;
					}

					// Eject the drive!
					TQT_DBusProxy driveControl("org.freedesktop.UDisks2", driveObjectPath, "org.freedesktop.UDisks2.Drive", dbusConn);
					params.clear();
					TQT_DBusDataMap<TQString> options(TQT_DBusData::Variant);
					params << TQT_DBusData::fromStringKeyMap(options);
					TQT_DBusMessage reply = driveControl.sendWithReply("Eject", params, &error);
					if (error.isValid()) {
						// Error!
						result["errStr"] = error.name() + ": " + error.message();
						return result;
					}
					else {
						// Eject was successful. Check if the media can be powered off and do so in case
						params.clear();
						params << TQT_DBusData::fromString("org.freedesktop.UDisks2.Drive") << TQT_DBusData::fromString("CanPowerOff");
						TQT_DBusMessage reply = driveInformation.sendWithReply("Get", params, &error);
						if (error.isValid()) {
							// Error!
							result["errStr"] = error.name() + ": " + error.message();
							return result;
						}

						if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
							bool canPowerOff = reply[0].toVariant().value.toBool();
							if (!canPowerOff) {
								// This drive does not support power off. Just return since the eject operation has finished.
								result["result"] = true;
								return result;
							}

							// Power off the drive!
							params.clear();
							TQT_DBusDataMap<TQString> options(TQT_DBusData::Variant);
							params << TQT_DBusData::fromStringKeyMap(options);
							TQT_DBusMessage reply = driveControl.sendWithReply("PowerOff", params, &error);
							if (error.isValid()) {
								// Error!
								result["errStr"] = error.name() + ": " + error.message();
								return result;
							}
							else {
								result["result"] = true;
								return result;
							}
						}
					}
				}
			}
		}
	}
	return result;
}

TQStringVariantMap udisks2MountDrive(const TQString &deviceNode, const TQString &fileSystemType, const TQString &mountOptions) {
	TQStringVariantMap result;
	result["result"] = false;
	result["retcode"] = -2;

	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString.replace("-", "_2d");
		blockDeviceString = "/org/freedesktop/UDisks2/block_devices/" + blockDeviceString;

		// Mount the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks2", blockDeviceString, "org.freedesktop.UDisks2.Filesystem", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			TQMap<TQString, TQT_DBusData> optionsMap;
			if (fileSystemType != "") {
				optionsMap["fstype"] = (TQT_DBusData::fromString(fileSystemType)).getAsVariantData();
			}
			optionsMap["options"] = (TQT_DBusData::fromString(mountOptions)).getAsVariantData();
			params << TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(optionsMap));
			TQT_DBusMessage reply = driveControl.sendWithReply("Mount", params, &error);
			if (!error.isValid()) {
				// Success
				result["retcode"] = 0;
				result["result"] = true;
				return result;
			}
			else {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					return result;  // Service not installed or unavailable
				}
				else {
					result["errStr"] = error.name() + ": " + error.message();
					result["retcode"] = -1;
					return result;
				}
			}
		}
	}
	return result;
}

TQStringVariantMap udisks2UnmountDrive(const TQString &deviceNode, const TQString &unmountOptions) {
	TQStringVariantMap result;
	result["result"] = false;
	result["retcode"] = -2;

	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString.replace("-", "_2d");
		blockDeviceString = "/org/freedesktop/UDisks2/block_devices/" + blockDeviceString;

		// Mount the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks2", blockDeviceString, "org.freedesktop.UDisks2.Filesystem", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			TQMap<TQString, TQT_DBusData> optionsMap;
			optionsMap["options"] = (TQT_DBusData::fromString(unmountOptions)).getAsVariantData();
			params << TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(optionsMap));
			TQT_DBusMessage reply = driveControl.sendWithReply("Unmount", params, &error);
			if (!error.isValid()) {
				// Success
				result["retcode"] = 0;
				result["result"] = true;
				return result;
			}
			else {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					return result;  // Service not installed or unavailable
				}
				else {
					result["errStr"] = error.name() + ": " + error.message();
					result["retcode"] = -1;
					return result;
				}
			}
		}
	}
	return result;
}

TQStringVariantMap udisks2UnlockDrive(const TQString &deviceNode, const TQString &passphrase) {
	TQStringVariantMap result;
	result["result"] = false;
	result["retcode"] = -2;

	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString.replace("-", "_2d");
		blockDeviceString = "/org/freedesktop/UDisks2/block_devices/" + blockDeviceString;

		// Unlock/decrypt the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks2", blockDeviceString, "org.freedesktop.UDisks2.Encrypted", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			params << TQT_DBusData::fromString(passphrase);
			TQMap<TQString, TQT_DBusVariant> optionsMap;
			params << TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(optionsMap));
			TQT_DBusMessage reply = driveControl.sendWithReply("Unlock", params, &error);
			if (!error.isValid()) {
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					TQT_DBusObjectPath deviceObjectPath = reply[0].toObjectPath();
					if (deviceObjectPath.isValid()) {
						// Success
						result["unlockedDevice"] = deviceObjectPath;
						result["retcode"] = 0;
						result["result"] = true;
						return result;
					}
				}
				result["errStr"] = i18n("Unknown error during unlocking operation.");
				result["retcode"] = -1;
				return result;
			}
			else {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					return result;  // Service not installed or unavailable
				}
				else {
					result["errStr"] = error.name() + ": " + error.message();
					result["retcode"] = -1;
					return result;
				}
			}
		}
	}
	return result;
}

TQStringVariantMap udisks2LockDrive(const TQString &deviceNode) {
	TQStringVariantMap result;
	result["result"] = false;
	result["retcode"] = -2;

	TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
	if (dbusConn.isConnected()) {
		TQString blockDeviceString = deviceNode;
		blockDeviceString.replace("/dev/", "");
		blockDeviceString.replace("-", "_2d");
		blockDeviceString = "/org/freedesktop/UDisks2/block_devices/" + blockDeviceString;

		// Lock/encrypt the drive!
		TQT_DBusError error;
		TQT_DBusProxy driveControl("org.freedesktop.UDisks2", blockDeviceString, "org.freedesktop.UDisks2.Encrypted", dbusConn);
		if (driveControl.canSend()) {
			TQValueList<TQT_DBusData> params;
			TQMap<TQString, TQT_DBusVariant> optionsMap;
			params << TQT_DBusData::fromStringKeyMap(TQT_DBusDataMap<TQString>(optionsMap));
			TQT_DBusMessage reply = driveControl.sendWithReply("Lock", params, &error);
			if (!error.isValid()) {
				// Success
				result["retcode"] = 0;
				result["result"] = true;
				return result;
			}
			else {
				// Error!
				if (error.name() == "org.freedesktop.DBus.Error.ServiceUnknown") {
					return result;  // Service not installed or unavailable
				}
				else {
					result["errStr"] = error.name() + ": " + error.message();
					result["retcode"] = -1;
					return result;
				}
			}
		}
	}
	return result;
}
#endif
