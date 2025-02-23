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

#ifndef _TDENETWORKBACKEND_NETWORKMANAGER_P_H
#define _TDENETWORKBACKEND_NETWORKMANAGER_P_H

/* TQt headers */
#include <tqvaluelist.h>
#include <tqapplication.h>
#include <tqtimer.h>
#include <tquuid.h>

/* TDE headers */
#include <kdebug.h>
#include <tdelocale.h>

/* TQDbus headers */
#include <tqdbusconnection.h>
#include <tqdbusobjectpath.h>
#include <tqdbusdata.h>
#include <tqdbuserror.h>
#include <tqdbusdatamap.h>
#include <tqdbusvariant.h>
#include <tqdbusdatalist.h>

/* NM headers */
//#include <NetworkManager.h>

/* Proxy headers */
#include "networkmanagerproxy.h"
#include "networkmanagersettings.h"
#include "connectionsettings.h"
#include "deviceproxy.h"
#include "ethernetproxy.h"
#include "infinibandproxy.h"
#include "wifiproxy.h"
#include "wimaxproxy.h"
#include "bluetoothproxy.h"
#include "olpcmeshproxy.h"
#include "activeconnectionproxy.h"
#include "accesspointproxy.h"
#include "vpnpluginproxy.h"

typedef TQT_DBusDataMap<TQString> TQT_DBusTQStringDataMap;
typedef TQValueList<TQT_DBusData> TQT_DBusDataValueList;

typedef TQMap<uint, bool> NMAsyncCallIDMap;
typedef TQMap<uint, TQT_DBusTQStringDataMap> NMAsyncSettingsResponseMap;
typedef TQMap<uint, TQT_DBusObjectPath> NMAddConnectionAsyncResponseMap;
typedef TQMap<uint, TQT_DBusError> NMAddConnectionAsyncErrorResponseMap;

typedef TQValueList<TQT_DBusObjectPath> TQT_DBusObjectPathList;

class TDENetworkConnectionManager_BackendNM;
class TDENetworkConnectionManager_BackendNMPrivate;

class TDENetworkConnectionManager_BackendNM_DBusSignalReceiver : public TQObject
{
	TQ_OBJECT

	public:
		TDENetworkConnectionManager_BackendNM_DBusSignalReceiver(TDENetworkConnectionManager_BackendNMPrivate*);
		~TDENetworkConnectionManager_BackendNM_DBusSignalReceiver();

	public slots:
		void dbusSignal(const TQT_DBusMessage&);

	private:
		TDENetworkConnectionManager_BackendNMPrivate* m_parent;
};

class TDENetworkConnectionManager_BackendNMPrivate : public TQObject
{
	TQ_OBJECT

	public:
		TDENetworkConnectionManager_BackendNMPrivate(TDENetworkConnectionManager_BackendNM*);
		~TDENetworkConnectionManager_BackendNMPrivate();

	public:
		DBus::NetworkManagerProxy* m_networkManagerProxy;
		DBus::SettingsInterface* m_networkManagerSettings;
		DBus::DeviceProxy* m_networkDeviceProxy;
		DBus::WiFiDeviceProxy* m_wiFiDeviceProxy;
		DBus::VPNPluginProxy* m_vpnProxy;
		NMAsyncCallIDMap nmConnectionSettingsAsyncCallWaiting;
		NMAsyncSettingsResponseMap nmConnectionSettingsAsyncSettingsResponse;
		NMAddConnectionAsyncResponseMap nmAddConnectionAsyncResponse;
		NMAddConnectionAsyncErrorResponseMap nmConnectionSettingsAsyncSettingsErrorResponse;
		NMAddConnectionAsyncErrorResponseMap nmConnectionSettingsUpdateAsyncSettingsErrorResponse;
		NMAddConnectionAsyncErrorResponseMap nmAddConnectionAsyncErrorResponse;
		bool nonReentrantCallActive;
		TQString m_dbusDeviceString;
		bool vpn_service_error_notified;
		bool device_autoconnect_error_notified;

	public slots:
		void processConnectionSettingsAsyncReply(int, const TQT_DBusDataMap<TQString>&);
		void processConnectionSettingsUpdateAsyncReply(int);
		void processAddConnectionAsyncReply(int, const TQT_DBusObjectPath&);
		void processConnectionSettingsAsyncError(int, const TQT_DBusError);
		void processConnectionSettingsUpdateAsyncError(int, const TQT_DBusError);
		void processAddConnectionAsyncError(int, const TQT_DBusError);

		void internalProcessGlobalStateChanged(TQ_UINT32 state);
		void internalProcessVPNStateChanged(TQ_UINT32 state);
		void internalProcessVPNLoginBanner(const TQString& banner);
		void internalProcessVPNFailure(TQ_UINT32 reason);
		void internalProcessDeviceStateChanged(TQ_UINT32 newState, TQ_UINT32 oldState, TQ_UINT32 reason);
		void internalProcessWiFiAccessPointAdded(const TQT_DBusObjectPath&);
		void internalProcessWiFiAccessPointRemoved(const TQT_DBusObjectPath&);
		void internalProcessWiFiPropertiesChanged(const TQMap<TQString, TQT_DBusVariant>&);
		void internalProcessAPPropertiesChanged(const TQMap<TQString, TQT_DBusVariant>&);

	private:
		TDENetworkConnectionManager_BackendNM* m_parent;
		TQMap<TQString, DBus::AccessPointProxy*> m_accessPointProxyList;
		TQT_DBusConnection *m_dbusSignalConnection;
		TDENetworkConnectionManager_BackendNM_DBusSignalReceiver *m_dbusSignalReceiver;
		TQ_UINT32 m_prevDeviceState;

		friend class TDENetworkConnectionManager_BackendNM_DBusSignalReceiver;
};

#endif // _TDENETWORKBACKEND_NETWORKMANAGER_P_H
