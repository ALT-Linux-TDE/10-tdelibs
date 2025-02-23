/* This file is part of the TDE libraries
   Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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
#ifndef _TDENETWORKBACKEND_NETWORKMANAGER_H
#define _TDENETWORKBACKEND_NETWORKMANAGER_H

#include "tdenetworkconnections.h"

//====================================================================================================
// General NetworkManager DBUS service paths
//====================================================================================================
#define	NM_DBUS_PATH				"/org/freedesktop/NetworkManager"
#define NM_DBUS_PATH_SETTINGS			"/org/freedesktop/NetworkManager/Settings"
#define NM_DBUS_PATH_SETTINGS_CONNECTION	"/org/freedesktop/NetworkManager/Settings/Connection"
#define NM_VPN_DBUS_PLUGIN_PATH			"/org/freedesktop/NetworkManager/VPN/Plugin"

#define	NM_DBUS_SERVICE				"org.freedesktop.NetworkManager"
#define NM_DBUS_ACTIVE_CONNECTION_SERVICE	"org.freedesktop.NetworkManager.Connection.Active"
#define NM_DBUS_DEVICE_SERVICE			"org.freedesktop.NetworkManager.Device"
#define NM_DBUS_WIRED_DEVICE_SERVICE		"org.freedesktop.NetworkManager.Device.Wired"
#define NM_DBUS_WIRELESS_DEVICE_SERVICE		"org.freedesktop.NetworkManager.Device.Wireless"
#define NM_DBUS_SETTINGS_SERVICE		"org.freedesktop.NetworkManager.Settings"
#define NM_DBUS_SETTINGS_CONNECTION_SERVICE	"org.freedesktop.NetworkManager.Settings.Connection"
#define NM_VPN_DBUS_PLUGIN_SERVICE		"org.freedesktop.NetworkManager.VPN.Plugin"
#define NM_VPN_DBUS_CONNECTION_SERVICE		"org.freedesktop.NetworkManager.VPN.Connection"
//====================================================================================================

//====================================================================================================
// These defines MUST be kept in sync with their respective introspection XML files
//====================================================================================================
#define NM_DEVICE_TYPE_UNKNOWN		0
#define NM_DEVICE_TYPE_ETHERNET		1
#define NM_DEVICE_TYPE_WIFI		2
#define NM_DEVICE_TYPE_UNUSED1		3
#define NM_DEVICE_TYPE_UNUSED2		4
#define NM_DEVICE_TYPE_BT		5
#define NM_DEVICE_TYPE_OLPC_MESH	6
#define NM_DEVICE_TYPE_WIMAX		7
#define NM_DEVICE_TYPE_MODEM		8
#define NM_DEVICE_TYPE_INFINIBAND	9
#define NM_DEVICE_TYPE_BOND		10
#define NM_DEVICE_TYPE_VLAN		11
#define NM_DEVICE_TYPE_ADSL		12
//====================================================================================================
#define NM_STATE_UNKNOWN		0
#define NM_STATE_ASLEEP			10
#define NM_STATE_DISCONNECTED		20
#define NM_STATE_DISCONNECTING		30
#define NM_STATE_CONNECTING		40
#define NM_STATE_CONNECTED_LOCAL	50
#define NM_STATE_CONNECTED_SITE		60
#define NM_STATE_CONNECTED_GLOBAL	70
//====================================================================================================
#define NM_DEVICE_STATE_UNKNOWN		0
#define NM_DEVICE_STATE_UNMANAGED	10
#define NM_DEVICE_STATE_UNAVAILABLE	20
#define NM_DEVICE_STATE_DISCONNECTED	30
#define NM_DEVICE_STATE_PREPARE		40
#define NM_DEVICE_STATE_CONFIG		50
#define NM_DEVICE_STATE_NEED_AUTH	60
#define NM_DEVICE_STATE_IP_CONFIG	70
#define NM_DEVICE_STATE_IP_CHECK	80
#define NM_DEVICE_STATE_SECONDARIES	90
#define NM_DEVICE_STATE_ACTIVATED	100
#define NM_DEVICE_STATE_DEACTIVATING	110
#define NM_DEVICE_STATE_FAILED		120
//====================================================================================================
#define NM_VPN_STATE_UNKNOWN		0
#define NM_VPN_STATE_PREPARE		1
#define NM_VPN_STATE_NEED_AUTH		2
#define NM_VPN_STATE_CONNECT		3
#define NM_VPN_STATE_IP_CONFIG_GET	4
#define NM_VPN_STATE_ACTIVATED		5
#define NM_VPN_STATE_FAILED		6
#define NM_VPN_STATE_DISCONNECTED	7
//====================================================================================================
#define NM_DEVICE_CAP_NONE		0
#define NM_DEVICE_CAP_NM_SUPPORTED	1
#define NM_DEVICE_CAP_CARRIER_DETECT	2
//====================================================================================================
#define NM_EAP_FAST_PROVISIONING_DISABLED	0
#define NM_EAP_FAST_PROVISIONING_UNAUTHONLY	1
#define NM_EAP_FAST_PROVISIONING_AUTHONLY	2
#define NM_EAP_FAST_PROVISIONING_BOTH		3
//====================================================================================================
#define NM_PASSWORD_SECRET_NONE		0
#define NM_PASSWORD_SECRET_AGENTOWNED	1
#define NM_PASSWORD_SECRET_NOTSAVED	2
#define NM_PASSWORD_SECRET_NOTREQUIRED	4
//====================================================================================================
#define NM_ACCESS_POINT_CAP_NONE	0x0
#define NM_ACCESS_POINT_CAP_PRIVACY	0x1
//====================================================================================================
#define NM_ACCESS_POINT_SEC_NONE		0x0
#define NM_ACCESS_POINT_SEC_PAIR_WEP40		0x1
#define NM_ACCESS_POINT_SEC_PAIR_WEP104		0x2
#define NM_ACCESS_POINT_SEC_PAIR_TKIP		0x4
#define NM_ACCESS_POINT_SEC_PAIR_CCMP		0x8
#define NM_ACCESS_POINT_SEC_GROUP_WEP40		0x10
#define NM_ACCESS_POINT_SEC_GROUP_WEP104	0x20
#define NM_ACCESS_POINT_SEC_GROUP_TKIP		0x40
#define NM_ACCESS_POINT_SEC_GROUP_CCMP		0x80
#define NM_ACCESS_POINT_SEC_KEY_MGMT_PSK	0x100
#define NM_ACCESS_POINT_SEC_KEY_MGMT_802_1X	0x200
//====================================================================================================
#define NM_WEP_TYPE_HEXADECIMAL		1
#define NM_WEP_TYPE_PASSPHRASE		2
//====================================================================================================
#define NM_VLAN_REORDER_PACKET_HEADERS	0x01
#define NM_VLAN_USE_GVRP		0x02
#define NM_VLAN_LOOSE_BINDING		0x04
//====================================================================================================
#define NM_GSM_3G_ALL			-1
#define NM_GSM_3G_ONLY			0
#define NM_GSM_GPRS_EDGE_ONLY		1
#define NM_GSM_PREFER_3G		2
#define NM_GSM_PREFER_2G		3
//====================================================================================================
#define NM_802_11_MODE_UNKNOWN		0
#define NM_802_11_MODE_ADHOC		1
#define NM_802_11_MODE_INFRASTRUCTURE	2
//====================================================================================================
#define NM_802_11_DEVICE_CAP_NONE		0x0
#define NM_802_11_DEVICE_CAP_CIPHER_WEP40	0x1
#define NM_802_11_DEVICE_CAP_CIPHER_WEP104	0x2
#define NM_802_11_DEVICE_CAP_CIPHER_TKIP	0x4
#define NM_802_11_DEVICE_CAP_CIPHER_CCMP	0x8
#define NM_802_11_DEVICE_CAP_WPA		0x10
#define NM_802_11_DEVICE_CAP_RSN		0x20
//====================================================================================================
#define NM_PLUGIN_SERVICE_DIR	"/etc/NetworkManager/VPN"
//====================================================================================================

//====================================================================================================
// Device state change reason codes
// Taken from NetworkManager.h
//====================================================================================================
#define NM_DEVICE_STATE_REASON_NONE				0
#define NM_DEVICE_STATE_REASON_UNKNOWN				1
#define NM_DEVICE_STATE_REASON_NOW_MANAGED			2
#define NM_DEVICE_STATE_REASON_NOW_UNMANAGED			3
#define NM_DEVICE_STATE_REASON_CONFIG_FAILED			4
#define NM_DEVICE_STATE_REASON_IP_CONFIG_UNAVAILABLE		5
#define NM_DEVICE_STATE_REASON_IP_CONFIG_EXPIRED		6
#define NM_DEVICE_STATE_REASON_NO_SECRETS			7
#define NM_DEVICE_STATE_REASON_SUPPLICANT_DISCONNECT		8
#define NM_DEVICE_STATE_REASON_SUPPLICANT_CONFIG_FAILED		9
#define NM_DEVICE_STATE_REASON_SUPPLICANT_FAILED		10
#define NM_DEVICE_STATE_REASON_SUPPLICANT_TIMEOUT		11
#define NM_DEVICE_STATE_REASON_PPP_START_FAILED			12
#define NM_DEVICE_STATE_REASON_PPP_DISCONNECT			13
#define NM_DEVICE_STATE_REASON_PPP_FAILED			14
#define NM_DEVICE_STATE_REASON_DHCP_START_FAILED		15
#define NM_DEVICE_STATE_REASON_DHCP_ERROR			16
#define NM_DEVICE_STATE_REASON_DHCP_FAILED			17
#define NM_DEVICE_STATE_REASON_SHARED_START_FAILED		18
#define NM_DEVICE_STATE_REASON_SHARED_FAILED			19
#define NM_DEVICE_STATE_REASON_AUTOIP_START_FAILED		20
#define NM_DEVICE_STATE_REASON_AUTOIP_ERROR			21
#define NM_DEVICE_STATE_REASON_AUTOIP_FAILED			22
#define NM_DEVICE_STATE_REASON_MODEM_BUSY			23
#define NM_DEVICE_STATE_REASON_MODEM_NO_DIAL_TONE		24
#define NM_DEVICE_STATE_REASON_MODEM_NO_CARRIER			25
#define NM_DEVICE_STATE_REASON_MODEM_DIAL_TIMEOUT		26
#define NM_DEVICE_STATE_REASON_MODEM_DIAL_FAILED		27
#define NM_DEVICE_STATE_REASON_MODEM_INIT_FAILED		28
#define NM_DEVICE_STATE_REASON_GSM_APN_FAILED			29
#define NM_DEVICE_STATE_REASON_GSM_REGISTRATION_NOT_SEARCHING	30
#define NM_DEVICE_STATE_REASON_GSM_REGISTRATION_DENIED		31
#define NM_DEVICE_STATE_REASON_GSM_REGISTRATION_TIMEOUT		32
#define NM_DEVICE_STATE_REASON_GSM_REGISTRATION_FAILED		33
#define NM_DEVICE_STATE_REASON_GSM_PIN_CHECK_FAILED		34
#define NM_DEVICE_STATE_REASON_FIRMWARE_MISSING			35
#define NM_DEVICE_STATE_REASON_REMOVED				36
#define NM_DEVICE_STATE_REASON_SLEEPING				37
#define NM_DEVICE_STATE_REASON_CONNECTION_REMOVED		38
#define NM_DEVICE_STATE_REASON_USER_REQUESTED			39
#define NM_DEVICE_STATE_REASON_CARRIER				40
#define NM_DEVICE_STATE_REASON_CONNECTION_ASSUMED		41
#define NM_DEVICE_STATE_REASON_SUPPLICANT_AVAILABLE		42
#define NM_DEVICE_STATE_REASON_MODEM_NOT_FOUND			43
#define NM_DEVICE_STATE_REASON_BT_FAILED			44
#define NM_DEVICE_STATE_REASON_GSM_SIM_NOT_INSERTED		45
#define NM_DEVICE_STATE_REASON_GSM_SIM_PIN_REQUIRED		46
#define NM_DEVICE_STATE_REASON_GSM_SIM_PUK_REQUIRED		47
#define NM_DEVICE_STATE_REASON_GSM_SIM_WRONG			48
#define NM_DEVICE_STATE_REASON_INFINIBAND_MODE			49
#define NM_DEVICE_STATE_REASON_DEPENDENCY_FAILED		50
#define NM_DEVICE_STATE_REASON_BR2684_FAILED			51
#define NM_DEVICE_STATE_REASON_MODEM_MANAGER_UNAVAILABLE	52
#define NM_DEVICE_STATE_REASON_SSID_NOT_FOUND			53
#define NM_DEVICE_STATE_REASON_SECONDARY_CONNECTION_FAILED	54
//====================================================================================================

class TQT_DBusObjectPath;
class TDENetworkConnectionManager_BackendNMPrivate;

class TDECORE_EXPORT TDENetworkConnectionManager_BackendNM : public TDENetworkConnectionManager
{
	TQ_OBJECT

	public:
		TDENetworkConnectionManager_BackendNM(TDENetworkDevice* networkDevice);
		~TDENetworkConnectionManager_BackendNM();

		virtual TQString backendName();
		virtual TDENetworkDeviceType::TDENetworkDeviceType deviceType();
		virtual TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags backendStatus();
		virtual TDENetworkDeviceInformation deviceInformation();
		virtual TDENetworkDeviceInformation deviceStatus();

		virtual void loadConnectionInformation();
		virtual void loadConnectionAllowedValues(TDENetworkConnection* connection);
		virtual bool loadConnectionSecrets(TQString uuid);
		virtual bool saveConnection(TDENetworkConnection* connection);
		virtual bool deleteConnection(TQString uuid);
		virtual bool verifyConnectionSettings(TDENetworkConnection* connection, TDENetworkConnectionErrorFlags::TDENetworkConnectionErrorFlags* type=NULL, TDENetworkErrorStringMap* reason=NULL);

		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus initiateConnection(TQString uuid);
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus checkConnectionStatus(TQString uuid);
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus deactivateConnection(TQString uuid);
		virtual TQStringList validSettings();

		virtual TDENetworkHWNeighborList* siteSurvey();
		virtual TQStringList connectionPhysicalDeviceUUIDs(TQString uuid);
		virtual TDENetworkVPNTypeList availableVPNTypes();

		virtual bool networkingEnabled();
		virtual bool wiFiHardwareEnabled();

		virtual bool enableNetworking(bool enable);
		virtual bool enableWiFi(bool enable);
		virtual bool wiFiEnabled();

		virtual TQStringList defaultNetworkDevices();

	private:
		TDENetworkDeviceType::TDENetworkDeviceType nmDeviceTypeToTDEDeviceType(TQ_UINT32 nmType);
		TQString deviceInterfaceString(TQString deviceNode);
		bool loadConnectionSecretsForGroup(TQString uuid, TQString group);
		TDENetworkWiFiAPInfo* getAccessPointDetails(TQString dbusPath);
		TDENetworkConnectionType::TDENetworkConnectionType connectionType(TQString dbusPath);
		TQT_DBusObjectPath getActiveConnectionPath(TQString uuid);

	private:
		TDENetworkConnectionManager_BackendNMPrivate* d;
		friend class TDENetworkConnectionManager_BackendNMPrivate;
};

#endif // _TDENETWORKBACKEND_NETWORKMANAGER_H
