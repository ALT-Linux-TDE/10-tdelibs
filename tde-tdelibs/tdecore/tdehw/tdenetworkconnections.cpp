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

#include "tdehardwaredevices.h"
#include "tdenetworkconnections.h"

#include <tqtimer.h>

#include <tdelocale.h>

#include "tdenetworkdevice.h"
// #define DEBUG_SIGNAL_QUEUE 1

#include "config.h"

#ifdef WITH_NETWORK_MANAGER_BACKEND
#include "network-manager.h"
#endif // WITH_NETWORK_MANAGER_BACKEND

#define SET_BIT(x, y) (x |= 1 << y)
#define TEST_BIT(x, y) ((x & (1 << y)) >> y)


/*================================================================================================*/
/* TDENetworkSearchDomain                                                                         */
/*================================================================================================*/

extern "C" {
	TDE_EXPORT TDEGlobalNetworkManager* create_tdeGlobalNetworkManager()
	{
		return new TDEGlobalNetworkManager();
	}
}

TDENetworkSearchDomain::TDENetworkSearchDomain() {
	m_isIPV6 = false;
}

TDENetworkSearchDomain::TDENetworkSearchDomain(TQString domain, bool ipv6) {
	m_isIPV6 = ipv6;
	m_domain = domain;
}

TDENetworkSearchDomain::~TDENetworkSearchDomain() {
	//
}

TQString TDENetworkSearchDomain::searchDomain() {
	return m_domain;
}

void TDENetworkSearchDomain::setSearchDomain(TQString domain, bool ipv6) {
	m_isIPV6 = ipv6;
	m_domain = domain;
}

bool TDENetworkSearchDomain::isIPv4() {
	return !m_isIPV6;
}

bool TDENetworkSearchDomain::isIPv6() {
	return m_isIPV6;
}

/*================================================================================================*/
/* TDENetMask                                                                                     */
/*================================================================================================*/

TDENetMask::TDENetMask() {
	m_ipv4NetMask = 0;
	m_isIPV6 = false;
}

TDENetMask::TDENetMask(TQ_UINT32 netmask) {
	m_ipv4NetMask = netmask;
	m_isIPV6 = false;
}

TDENetMask::TDENetMask(TQ_UINT8* netmask) {
	m_ipv6NetMask = TQHostAddress(netmask);
	m_isIPV6 = true;
}

TDENetMask::~TDENetMask() {
	//
}

void TDENetMask::fromCIDRMask(unsigned char mask, bool ipv6) {
	unsigned int i;
	unsigned int j;
	unsigned int internalMask = mask;
	if (!ipv6) {
		m_ipv4NetMask = 0;
		for (i=32; i>(32-internalMask); ) {
			i--;
			SET_BIT(m_ipv4NetMask, i);
		}
		m_isIPV6 = false;
	}
	else {
		Q_IPV6ADDR maskarray;
		j=0;
		unsigned int byteno=0;
		memset(maskarray.c, 0, 16);
		for (i=128; i>(128-internalMask); ) {
			i--;
			SET_BIT(maskarray.c[byteno], (i-((15-byteno)*8)));
			j++;
			if (j>7) {
				j=0;
				byteno++;
			}
		}
		m_ipv6NetMask = TQHostAddress(maskarray);
		m_isIPV6 = true;
	}
}

unsigned char TDENetMask::toCIDRMask() {
	unsigned int i;
	unsigned int j;
	if (!m_isIPV6) {
		for (i=0; i<32; i++) {
			if (TEST_BIT(m_ipv4NetMask, i)) {
				break;
			}
		}
		return 32-i;
	}
	else {
		Q_IPV6ADDR mask = m_ipv6NetMask.toIPv6Address();
		bool found = false;
		for (j=0; j<16; ++j) {
			for (i=0; i<8; i++) {
				if (!TEST_BIT(mask.c[j], i)) {
					found = true;
					break;
				}
			}
			if (found) break;
		}
		return ((j*8)+i);
	}
}

void TDENetMask::fromString(TQString mask) {
	if (mask.contains(".")) {
		m_isIPV6 = false;
		m_ipv4NetMask = 0;
		TQStringList pieces = TQStringList::split(".", mask);
		TQ_UINT8 chunk;
		chunk = pieces[0].toUShort();
		m_ipv4NetMask = m_ipv4NetMask | (chunk << 24);
		chunk = pieces[1].toUShort();
		m_ipv4NetMask = m_ipv4NetMask | (chunk << 16);
		chunk = pieces[2].toUShort();
		m_ipv4NetMask = m_ipv4NetMask | (chunk << 8);
		chunk = pieces[3].toUShort();
		m_ipv4NetMask = m_ipv4NetMask | (chunk << 0);
	}
	else if (mask.contains(":")) {
		m_isIPV6 = true;
		m_ipv6NetMask.setAddress(mask);
	}
}

TQString TDENetMask::toString() {
	if (!m_isIPV6) {
		return TQString("%1.%2.%3.%4").arg((m_ipv4NetMask & 0xff000000) >> 24).arg((m_ipv4NetMask & 0x00ff0000) >> 16).arg((m_ipv4NetMask & 0x0000ff00) >> 8).arg((m_ipv4NetMask & 0x000000ff) >> 0);
	}
	else {
		return m_ipv6NetMask.toString();
	}
}

bool TDENetMask::isIPv4() {
	return !m_isIPV6;
}

bool TDENetMask::isIPv6() {
	return m_isIPV6;
}

/*================================================================================================*/
/* TDEMACAddress                                                                      */
/*================================================================================================*/

TDEMACAddress::TDEMACAddress() {
	m_macAddress.clear();
	m_isValid = false;
}

TDEMACAddress::TDEMACAddress(TDENetworkByteList address) {
	m_macAddress = address;
	m_isValid = true;
}

TDEMACAddress::~TDEMACAddress() {
	//
}

TDENetworkByteList TDEMACAddress::address() {
	return m_macAddress;
}

void TDEMACAddress::setAddress(TDENetworkByteList address) {
	m_macAddress = address;
	m_isValid = true;
}

bool TDEMACAddress::isValid() {
	if (!m_isValid) {
		return false;
	}

	if (m_macAddress.count() > 0) {
		return true;
	}
	else {
		return false;
	}
}

void TDEMACAddress::fromString(TQString address) {
	TQStringList pieces = TQStringList::split(":", address);
	m_macAddress.clear();
	for (TQStringList::Iterator it = pieces.begin(); it != pieces.end(); ++it) {
		m_macAddress.append((*it).toUShort(0, 16));
	}
	m_isValid = true;
}

TQString TDEMACAddress::toString() {
	TQString ret;
	TDENetworkByteList::iterator it;
	for (it = m_macAddress.begin(); it != m_macAddress.end(); ++it) {
		if (ret != "") {
			ret.append(":");
		}
		ret.append(TQString().sprintf("%02x", *it));
	}
	return ret.lower();
}

bool operator==(const TDEMACAddress &a1, const TDEMACAddress &a2) {
	if (a1.m_macAddress.count() != a2.m_macAddress.count()) {
		return false;
	}
	else {
		unsigned int i;
		for (i=0; i<a1.m_macAddress.count(); i++) {
			if (a1.m_macAddress[i] != a2.m_macAddress[i]) {
				return false;
			}
		}
		return true;
	}
}

bool operator<(const TDEMACAddress &a1, const TDEMACAddress &a2) {
	if (a1.m_macAddress.count() < a2.m_macAddress.count()) {
		return true;
	}
	else {
		unsigned int i;
		for (i=0; i<a1.m_macAddress.count(); i++) {
			if (a1.m_macAddress[i] < a2.m_macAddress[i]) {
				return true;
			}
			if (a1.m_macAddress[i] > a2.m_macAddress[i]) {
				return false;
			}
		}
		return false;
	}
}

/*================================================================================================*/
/* TDENetworkSingleIPConfiguration                                                                */
/*================================================================================================*/

TDENetworkSingleIPConfiguration::TDENetworkSingleIPConfiguration() {
	valid = false;
}

TDENetworkSingleIPConfiguration::~TDENetworkSingleIPConfiguration() {
	//
}

bool TDENetworkSingleIPConfiguration::isIPv4() {
	return ipAddress.isIPv4Address() & valid;
}

bool TDENetworkSingleIPConfiguration::isIPv6() {
	return ipAddress.isIPv6Address() & valid;
}

/*================================================================================================*/
/* TDENetworkSingleRouteConfiguration                                                                */
/*================================================================================================*/

TDENetworkSingleRouteConfiguration::TDENetworkSingleRouteConfiguration() {
	valid = false;
}

TDENetworkSingleRouteConfiguration::~TDENetworkSingleRouteConfiguration() {
	//
}

bool TDENetworkSingleRouteConfiguration::isIPv4() {
	return ipAddress.isIPv4Address() & valid;
}

bool TDENetworkSingleRouteConfiguration::isIPv6() {
	return ipAddress.isIPv6Address() & valid;
}

/*================================================================================================*/
/* TDENetworkIEEE8021xConfiguration                                                               */
/*================================================================================================*/

TDENetworkIEEE8021xConfiguration::TDENetworkIEEE8021xConfiguration() {
	valid = false;
	allowedValid = false;
	secretsValid = false;
	type = TDENetworkIEEE8021xType::None;
	fastProvisioningFlags = TDENetworkIEEE8021xFastFlags::None;
	phase2NonEAPAuthMethod = TDENetworkIEEE8021xType::None;
	phase2EAPAuthMethod = TDENetworkIEEE8021xType::None;
	passwordFlags = TDENetworkPasswordHandlingFlags::None;
	binaryPasswordFlags = TDENetworkPasswordHandlingFlags::None;
	privateKeyPasswordFlags = TDENetworkPasswordHandlingFlags::None;
	phase2PrivateKeyPasswordFlags = TDENetworkPasswordHandlingFlags::None;
	forceSystemCaCertificates = false;
}

TDENetworkIEEE8021xConfiguration::~TDENetworkIEEE8021xConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkPPPConfiguration                                                                     */
/*================================================================================================*/

TDENetworkPPPConfiguration::TDENetworkPPPConfiguration() {
	valid = false;
	requireServerAuthentication = false;
	flags = TDENetworkPPPFlags::None;
	baudRate = 115200;
	mru = 0;
	mtu = 0;
	lcpEchoPingInterval = 0;
	lcpEchoFailureThreshold = 0;
}

TDENetworkPPPConfiguration::~TDENetworkPPPConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkPPPOEConfiguration                                                                   */
/*================================================================================================*/

TDENetworkPPPOEConfiguration::TDENetworkPPPOEConfiguration() {
	valid = false;
	secretsValid = false;
	passwordFlags = TDENetworkPasswordHandlingFlags::None;
}

TDENetworkPPPOEConfiguration::~TDENetworkPPPOEConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkSerialConfiguration                                                                  */
/*================================================================================================*/

TDENetworkSerialConfiguration::TDENetworkSerialConfiguration() {
	valid = false;
	baudRate = 115200;
	byteWidth = 8;
	parity = TDENetworkParity::None;
	stopBits = 1;
	txDelay = 0;
}

TDENetworkSerialConfiguration::~TDENetworkSerialConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkCDMAConfiguration                                                                    */
/*================================================================================================*/

TDENetworkCDMAConfiguration::TDENetworkCDMAConfiguration() {
	valid = false;
	secretsValid = false;
	passwordFlags = TDENetworkPasswordHandlingFlags::None;
}

TDENetworkCDMAConfiguration::~TDENetworkCDMAConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkGSMConfiguration                                                                     */
/*================================================================================================*/

TDENetworkGSMConfiguration::TDENetworkGSMConfiguration() {
	valid = false;
	secretsValid = false;
	passwordFlags = TDENetworkPasswordHandlingFlags::None;
	networkType = TDEGSMNetworkType::Any;
	pinFlags = TDENetworkPasswordHandlingFlags::None;
	allowRoaming = false;
}

TDENetworkGSMConfiguration::~TDENetworkGSMConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkIPConfiguration                                                                      */
/*================================================================================================*/

TDENetworkIPConfiguration::TDENetworkIPConfiguration() {
	valid = false;
	connectionFlags =	TDENetworkIPConfigurationFlags::IPV4DHCPIP			| \
				TDENetworkIPConfigurationFlags::IPV4DHCPDNS			| \
				TDENetworkIPConfigurationFlags::IPV4DHCPRoutes			| \
				TDENetworkIPConfigurationFlags::IPV4MayUseAsDefaultRoute	| \
				TDENetworkIPConfigurationFlags::IPV6DHCPIP			| \
				TDENetworkIPConfigurationFlags::IPV6DHCPDNS			| \
				TDENetworkIPConfigurationFlags::IPV6DHCPRoutes			| \
				TDENetworkIPConfigurationFlags::IPV6MayUseAsDefaultRoute;
}

TDENetworkIPConfiguration::~TDENetworkIPConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkWiFiSecurityConfiguration                                                            */
/*================================================================================================*/

TDENetworkWiFiSecurityConfiguration::TDENetworkWiFiSecurityConfiguration() {
	valid = false;
	secretsValid = false;
	keyType = TDENetworkWiFiKeyType::Other;
	authType = TDENetworkWiFiAuthType::Other;
	wpaVersion = TDENetworkWiFiWPAVersionFlags::Any;
	cipher = TDENetworkWiFiConnectionCipher::None;
	wepKeyFlags = TDENetworkPasswordHandlingFlags::None;
	wepKeyIndex = 0;
	wepKeyType = TDENetworkWepKeyType::Hexadecimal;
	pskFlags = TDENetworkPasswordHandlingFlags::None;
	leapPasswordFlags = TDENetworkPasswordHandlingFlags::None;
}

TDENetworkWiFiSecurityConfiguration::~TDENetworkWiFiSecurityConfiguration() {
	//
}

/*================================================================================================*/
/* TDENetworkWiFiDeviceInfo                                                                      */
/*================================================================================================*/

TDENetworkWiFiDeviceInfo::TDENetworkWiFiDeviceInfo() {
	valid = false;
}

TDENetworkWiFiDeviceInfo::~TDENetworkWiFiDeviceInfo() {
	//
}

/*================================================================================================*/
/* TDENetworkDeviceInformation                                                                      */
/*================================================================================================*/

TDENetworkDeviceInformation::TDENetworkDeviceInformation() {
	valid = false;
	statusFlags = TDENetworkConnectionStatus::Invalid;
	capabilityFlags = TDENetworkDeviceCapabilityFlags::None;
}

TDENetworkDeviceInformation::~TDENetworkDeviceInformation() {
	//
}

/*================================================================================================*/
/* TDENetworkHWNeighbor                                                                      */
/*================================================================================================*/

TDENetworkHWNeighbor::TDENetworkHWNeighbor() {
	valid = false;
}

TDENetworkHWNeighbor::~TDENetworkHWNeighbor() {
	//
}

/*================================================================================================*/
/* TDENetworkWiFiAPInfo                                                                      */
/*================================================================================================*/

TDENetworkWiFiAPInfo::TDENetworkWiFiAPInfo() : TDENetworkHWNeighbor() {
	//
}

TDENetworkWiFiAPInfo::~TDENetworkWiFiAPInfo() {
	//
}

TQString TDENetworkWiFiAPInfo::friendlySSID() const {
	TQString ret;

	ret = TQString(SSID);
	if (ret == "") {
		ret = "<" + i18n("hidden") + ">";
	}

	return ret;
}

/*================================================================================================*/
/* TDENetworkConnection                                                                           */
/*================================================================================================*/

TDENetworkConnection::TDENetworkConnection() {
	readOnly = false;
	autoConnect = false;
	fullDuplex = true;
	requireIPV4 = false;
	requireIPV6 = false;
	mtu = 0;
}

TDENetworkConnection::~TDENetworkConnection() {
	//
}

TDENetworkConnectionType::TDENetworkConnectionType TDENetworkConnection::type() {
	if (dynamic_cast<TDEWiredEthernetConnection*>(this)) {
		return TDENetworkConnectionType::WiredEthernet;
	}
	else if (dynamic_cast<TDEWiredInfinibandConnection*>(this)) {
		return TDENetworkConnectionType::Infiniband;
	}
	else if (dynamic_cast<TDEVPNConnection*>(this)) {
		return TDENetworkConnectionType::VPN;
	}
	else if (dynamic_cast<TDEWiMaxConnection*>(this)) {
		return TDENetworkConnectionType::WiMax;
	}
	else if (dynamic_cast<TDEVLANConnection*>(this)) {
		return TDENetworkConnectionType::VLAN;
	}
	else if (dynamic_cast<TDEOLPCMeshConnection*>(this)) {
		return TDENetworkConnectionType::OLPCMesh;
	}
	else if (dynamic_cast<TDEBluetoothConnection*>(this)) {
		return TDENetworkConnectionType::Bluetooth;
	}
	else if (dynamic_cast<TDEModemConnection*>(this)) {
		return TDENetworkConnectionType::Modem;
	}
	else if (dynamic_cast<TDEWiFiConnection*>(this)) {
		return TDENetworkConnectionType::WiFi;
	}
	else {
		return TDENetworkConnectionType::Other;
	}
}

/*================================================================================================*/
/* TDEWiredEthernetConnection                                                                     */
/*================================================================================================*/

TDEWiredEthernetConnection::TDEWiredEthernetConnection() : TDENetworkConnection() {
	//
}

TDEWiredEthernetConnection::~TDEWiredEthernetConnection() {
	//
}

/*================================================================================================*/
/* TDEWiredInfinibandConnection                                                                   */
/*================================================================================================*/

TDEWiredInfinibandConnection::TDEWiredInfinibandConnection() : TDENetworkConnection() {
	//
}

TDEWiredInfinibandConnection::~TDEWiredInfinibandConnection() {
	//
}

/*================================================================================================*/
/* TDEVPNConnection                                                                               */
/*================================================================================================*/

TDEVPNConnection::TDEVPNConnection() : TDENetworkConnection() {
	secretsValid = false;
}

TDEVPNConnection::~TDEVPNConnection() {
	//
}

/*================================================================================================*/
/* TDEWiMaxConnection                                                                             */
/*================================================================================================*/

TDEWiMaxConnection::TDEWiMaxConnection() : TDENetworkConnection() {
	//
}

TDEWiMaxConnection::~TDEWiMaxConnection() {
	//
}

/*================================================================================================*/
/* TDEVLANConnection                                                                              */
/*================================================================================================*/

TDEVLANConnection::TDEVLANConnection() : TDENetworkConnection() {
	//
}

TDEVLANConnection::~TDEVLANConnection() {
	//
}

/*================================================================================================*/
/* TDEOLPCMeshConnection                                                                          */
/*================================================================================================*/

TDEOLPCMeshConnection::TDEOLPCMeshConnection() : TDENetworkConnection() {
	channel = 0;
}

TDEOLPCMeshConnection::~TDEOLPCMeshConnection() {
	//
}

/*================================================================================================*/
/* TDEBluetoothConnection                                                                         */
/*================================================================================================*/

TDEBluetoothConnection::TDEBluetoothConnection() : TDENetworkConnection() {
	type = TDEBluetoothConnectionType::Other;
}

TDEBluetoothConnection::~TDEBluetoothConnection() {
	//
}

/*================================================================================================*/
/* TDEModemConnection                                                                             */
/*================================================================================================*/

TDEModemConnection::TDEModemConnection() : TDENetworkConnection() {
	type = TDEModemConnectionType::Other;
}

TDEModemConnection::~TDEModemConnection() {
	//
}

/*================================================================================================*/
/* TDEWiFiConnection                                                                              */
/*================================================================================================*/

TDEWiFiConnection::TDEWiFiConnection() : TDENetworkConnection() {
	operatingMode = TDEWiFiMode::Other;
	bandRestriction = TDEWiFiFrequencyBand::None;
	channelRestriction = -1;
	bitRateRestriction = -1;
	powerRestriction = -1;
	isHiddenNetwork = false;
	securityRequired = false;
}

TDEWiFiConnection::~TDEWiFiConnection() {
	//
}

/*================================================================================================*/
/* TDENetworkConnectionManager                                                                    */
/*================================================================================================*/

TDENetworkConnectionManager::TDENetworkConnectionManager(TDENetworkDevice *networkDevice) : TQObject(), m_connectionList(NULL), m_hwNeighborList(NULL), m_networkDevice(networkDevice), m_prevConnectionStatus(TDENetworkGlobalManagerFlags::Unknown) {
	m_emissionTimer = new TQTimer();
	connect(m_emissionTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(emitQueuedSignals()));
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, true);
}

TDENetworkConnectionManager::~TDENetworkConnectionManager() {
	m_emissionTimer->stop();
	delete m_emissionTimer;
}

TQString TDENetworkConnectionManager::deviceNode() {
	return m_networkDevice ? m_networkDevice->deviceNode() : TQString();
}

TQString TDENetworkConnectionManager::deviceMACAddress() {
	return m_networkDevice ? m_networkDevice->macAddress() : TQString();
}

TDENetworkConnectionList* TDENetworkConnectionManager::connections() {
	return m_connectionList;
}

TDENetworkConnection* TDENetworkConnectionManager::findConnectionByUUID(TQString uuid) {
	TDENetworkConnection *connection;
	for (connection = m_connectionList->first(); connection; connection = m_connectionList->next()) {
		if (connection->UUID == uuid) {
			return connection;
		}
	}
	return NULL;
}

TDENetworkDevice* TDENetworkConnectionManager::findDeviceByUUID(TQString uuid) {
	TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
	if (!hwdevices) return NULL;

	TDEGenericHardwareList devices = hwdevices->listByDeviceClass(TDEGenericDeviceType::Network);
	for (TDEGenericHardwareList::iterator it = devices.begin(); it != devices.end(); ++it)
	{
		TDENetworkDevice* dev = dynamic_cast<TDENetworkDevice*>(*it);
		if (dev) {
			TDENetworkConnectionManager* deviceConnectionManager = dev->connectionManager();
			if (deviceConnectionManager) {
				if (deviceConnectionManager->deviceInformation().UUID == uuid) {
					return dev;
				}
			}
		}
	}

	return NULL;
}

TDENetworkWiFiAPInfo* TDENetworkConnectionManager::findAccessPointByBSSID(TDEMACAddress bssid) {
	if (!bssid.isValid()) {
		return NULL;
	}

	TDENetworkHWNeighbor *neighbor;
	for (neighbor = m_hwNeighborList->first(); neighbor; neighbor = m_hwNeighborList->next()) {
		TDENetworkWiFiAPInfo* apInfo = dynamic_cast<TDENetworkWiFiAPInfo*>(neighbor);
		if (!apInfo) {
			continue;
		}
		if (apInfo->BSSID == bssid) {
			return apInfo;
		}
	}
	return NULL;
}

TQString TDENetworkConnectionManager::friendlyConnectionTypeName(TDENetworkConnectionType::TDENetworkConnectionType type) {
	if (type == TDENetworkConnectionType::WiredEthernet) {
		return i18n("Wired Ethernet");
	}
	else if (type == TDENetworkConnectionType::WiFi) {
		return i18n("802.11 WiFi");
	}
	else if (type == TDENetworkConnectionType::Bluetooth) {
		return i18n("Bluetooth");
	}
	else if (type == TDENetworkConnectionType::OLPCMesh) {
		return i18n("OLPC Mesh");
	}
	else if (type == TDENetworkConnectionType::WiMax) {
		return i18n("WiMax");
	}
	else if (type == TDENetworkConnectionType::Modem) {
		return i18n("Cellular Modem");
	}
	else if (type == TDENetworkConnectionType::Infiniband) {
		return i18n("Infiniband");
	}
	else if (type == TDENetworkConnectionType::Bond) {
		return i18n("Bond");
	}
	else if (type == TDENetworkConnectionType::VLAN) {
		return i18n("Virtual LAN");
	}
	else if (type == TDENetworkConnectionType::ADSL) {
		return i18n("ADSL");
	}
	else if (type == TDENetworkConnectionType::VPN) {
		return i18n("Virtual Private Network");
	}
	else if (type == TDENetworkConnectionType::Other) {
		return i18n("Other");
	}
	else {
		return TQString::null;
	}
}

bool TDENetworkConnectionManager::validateIPAddress(TQHostAddress address) {
	if (address.isIPv4Address()) {
		TQ_UINT32 rawaddress = address.toIPv4Address();
		if ((((rawaddress & 0xff000000) >> 24) == 0) || ((rawaddress & 0x000000ff) == 0) || ((rawaddress & 0x000000ff) == 255)) {
			return false;
		}
	}
	else if (address.isIPv6Address()) {
		Q_IPV6ADDR rawaddress = address.toIPv6Address();
		if (rawaddress.c[0] == 0xff) {
			return false;
		}
	}
	return true;
}

bool TDENetworkConnectionManager::validateIPNeworkMask(TQHostAddress netmask) {
	Q_UNUSED(netmask);
	return true;
}

void TDENetworkConnectionManager::clearTDENetworkConnectionList() {
	TDENetworkConnection *connection;
	for (connection = m_connectionList->first(); connection; connection = m_connectionList->next()) {
		delete connection;
	}
	m_connectionList->clear();
}

void TDENetworkConnectionManager::clearTDENetworkHWNeighborList() {
	TDENetworkHWNeighbor *neighbor;
	for (neighbor = m_hwNeighborList->first(); neighbor; neighbor = m_hwNeighborList->next()) {
		delete neighbor;
	}
	m_hwNeighborList->clear();
}

void TDENetworkConnectionManager::internalNetworkConnectionStateChanged(TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags newState) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 0;
	queuedEvent.newState = newState;
	queuedEvent.previousState = m_prevConnectionStatus;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, true);

	m_prevConnectionStatus = newState;
}

void TDENetworkConnectionManager::internalNetworkDeviceStateChanged(TDENetworkConnectionStatus::TDENetworkConnectionStatus newState, TQString deviceNodeName) {
	if (!m_prevDeviceStatus.contains(deviceNodeName)) {
		m_prevDeviceStatus[deviceNodeName] = TDENetworkConnectionStatus::Invalid;
	}

	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 1;
	queuedEvent.newConnStatus = newState;
	queuedEvent.previousConnStatus = m_prevDeviceStatus[deviceNodeName];
	queuedEvent.deviceNode = deviceNodeName;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, true);

	m_prevDeviceStatus[deviceNodeName] = newState;
}

void TDENetworkConnectionManager::internalAccessPointStatusChanged(TDEMACAddress BSSID, TDENetworkAPEventType::TDENetworkAPEventType event) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 2;
	queuedEvent.BSSID = BSSID;
	queuedEvent.apevent = event;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, true);
}

void TDENetworkConnectionManager::internalNetworkDeviceEvent(TDENetworkDeviceEventType::TDENetworkDeviceEventType event, TQString message) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 3;
	queuedEvent.ndevent = event;
	queuedEvent.message = message;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, true);
}

void TDENetworkConnectionManager::internalVpnEvent(TDENetworkVPNEventType::TDENetworkVPNEventType event, TQString message) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 4;
	queuedEvent.vpnevent = event;
	queuedEvent.message = message;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, true);
}

void TDENetworkConnectionManager::internalNetworkManagementEvent(TDENetworkGlobalEventType::TDENetworkGlobalEventType event) {
	TDENetworkEventQueueEvent_Private queuedEvent;
	queuedEvent.eventType = 5;
	queuedEvent.globalevent = event;
	m_globalEventQueueEventList.append(queuedEvent);
	if (!m_emissionTimer->isActive()) m_emissionTimer->start(0, true);
}

void TDENetworkConnectionManager::emitQueuedSignals() {
	if (!m_globalEventQueueEventList.isEmpty()) {
#ifdef DEBUG_SIGNAL_QUEUE
		kdDebug() << "TDENetworkConnectionManager::emitQueuedSignals: Going to dequeue " << m_globalEventQueueEventList.count() << " events..." << endl;
#endif // DEBUG_SIGNAL_QUEUE
		TDENetworkEventQueueEvent_PrivateList::Iterator it;
		it = m_globalEventQueueEventList.begin();
		while (it != m_globalEventQueueEventList.end()) {
			TDENetworkEventQueueEvent_Private event = (*it);
			it = m_globalEventQueueEventList.remove(it);
			if (event.eventType == 0) {
				emit(networkConnectionStateChanged(event.newState, event.previousState));
			}
			else if (event.eventType == 1) {
				emit(networkDeviceStateChanged(event.newConnStatus, event.previousConnStatus, event.deviceNode));
			}
			else if (event.eventType == 2) {
				emit(accessPointStatusChanged(event.BSSID, event.apevent));
			}
			else if (event.eventType == 3) {
				emit(networkDeviceEvent(event.ndevent, event.message));
			}
			else if (event.eventType == 4) {
				emit(vpnEvent(event.vpnevent, event.message));
			}
			else if (event.eventType == 5) {
				emit(networkManagementEvent(event.globalevent));
			}
		}
#ifdef DEBUG_SIGNAL_QUEUE
		kdDebug() << "TDENetworkConnectionManager::emitQueuedSignals: " << m_globalEventQueueEventList.count() << " events remain in queue" << endl;
#endif // DEBUG_SIGNAL_QUEUE
	}
}

/*================================================================================================*/
/* TDEGlobalNetworkManager                                                                        */
/*================================================================================================*/

TDEGlobalNetworkManager::TDEGlobalNetworkManager() : m_internalConnectionManager(NULL) {
#ifdef WITH_NETWORK_MANAGER_BACKEND
	m_internalConnectionManager = new TDENetworkConnectionManager_BackendNM(NULL);
#endif // WITH_NETWORK_MANAGER_BACKEND
	if (m_internalConnectionManager) {
		connect(m_internalConnectionManager, TQ_SIGNAL(networkConnectionStateChanged(TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags, TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags)), this, TQ_SIGNAL(networkConnectionStateChanged(TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags, TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags)));
		connect(m_internalConnectionManager, TQ_SIGNAL(vpnEvent(TDENetworkVPNEventType::TDENetworkVPNEventType, TQString)), this, TQ_SIGNAL(vpnEvent(TDENetworkVPNEventType::TDENetworkVPNEventType, TQString)));
		connect(m_internalConnectionManager, TQ_SIGNAL(networkManagementEvent(TDENetworkGlobalEventType::TDENetworkGlobalEventType)), this, TQ_SIGNAL(networkManagementEvent(TDENetworkGlobalEventType::TDENetworkGlobalEventType)));
	}
}

TDEGlobalNetworkManager::~TDEGlobalNetworkManager() {
	delete m_internalConnectionManager;
}

TQString TDEGlobalNetworkManager::backendName() {
	if (!m_internalConnectionManager) return TQString::null;
	return m_internalConnectionManager->backendName();
}

TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags TDEGlobalNetworkManager::backendStatus() {
	if (!m_internalConnectionManager) return TDENetworkGlobalManagerFlags::BackendUnavailable;
	return m_internalConnectionManager->backendStatus();
}

void TDEGlobalNetworkManager::loadConnectionInformation() {
	if (!m_internalConnectionManager) return;
	return m_internalConnectionManager->loadConnectionInformation();
}

void TDEGlobalNetworkManager::loadConnectionAllowedValues(TDENetworkConnection* connection) {
	if (!m_internalConnectionManager) return;
	return m_internalConnectionManager->loadConnectionAllowedValues(connection);
}

bool TDEGlobalNetworkManager::loadConnectionSecrets(TQString uuid) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->loadConnectionSecrets(uuid);
}

bool TDEGlobalNetworkManager::saveConnection(TDENetworkConnection* connection) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->saveConnection(connection);
}

bool TDEGlobalNetworkManager::deleteConnection(TQString uuid) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->deleteConnection(uuid);
}

bool TDEGlobalNetworkManager::verifyConnectionSettings(TDENetworkConnection* connection, TDENetworkConnectionErrorFlags::TDENetworkConnectionErrorFlags* type, TDENetworkErrorStringMap* reason) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->verifyConnectionSettings(connection, type, reason);
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDEGlobalNetworkManager::initiateConnection(TQString uuid) {
	if (!m_internalConnectionManager) return TDENetworkConnectionStatus::Invalid;
	return m_internalConnectionManager->initiateConnection(uuid);
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDEGlobalNetworkManager::checkConnectionStatus(TQString uuid) {
	if (!m_internalConnectionManager) return TDENetworkConnectionStatus::Invalid;
	return m_internalConnectionManager->checkConnectionStatus(uuid);
}

TDENetworkConnectionStatus::TDENetworkConnectionStatus TDEGlobalNetworkManager::deactivateConnection(TQString uuid) {
	if (!m_internalConnectionManager) return TDENetworkConnectionStatus::Invalid;
	return m_internalConnectionManager->deactivateConnection(uuid);
}

TQStringList TDEGlobalNetworkManager::validSettings() {
	if (!m_internalConnectionManager) return TQStringList();
	return m_internalConnectionManager->validSettings();
}

TDENetworkHWNeighborList* TDEGlobalNetworkManager::siteSurvey() {
	if (!m_internalConnectionManager) return NULL;
	return m_internalConnectionManager->siteSurvey();
}

TQStringList TDEGlobalNetworkManager::connectionPhysicalDeviceUUIDs(TQString uuid) {
	if (!m_internalConnectionManager) return TQStringList();
	return m_internalConnectionManager->connectionPhysicalDeviceUUIDs(uuid);
}

TDENetworkVPNTypeList TDEGlobalNetworkManager::availableVPNTypes() {
	if (!m_internalConnectionManager) return TDENetworkVPNTypeList();
	return m_internalConnectionManager->availableVPNTypes();
}

bool TDEGlobalNetworkManager::networkingEnabled() {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->networkingEnabled();
}

bool TDEGlobalNetworkManager::enableNetworking(bool enable) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->enableNetworking(enable);
}

bool TDEGlobalNetworkManager::wiFiHardwareEnabled() {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->wiFiHardwareEnabled();
}

bool TDEGlobalNetworkManager::enableWiFi(bool enable) {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->enableWiFi(enable);
}

bool TDEGlobalNetworkManager::wiFiEnabled() {
	if (!m_internalConnectionManager) return false;
	return m_internalConnectionManager->wiFiEnabled();
}

TQStringList TDEGlobalNetworkManager::defaultNetworkDevices() {
	if (!m_internalConnectionManager) return TQStringList();
	return m_internalConnectionManager->defaultNetworkDevices();
}

TDENetworkConnectionList* TDEGlobalNetworkManager::connections() {
	if (!m_internalConnectionManager) return NULL;
	return m_internalConnectionManager->connections();
}

TDENetworkConnection* TDEGlobalNetworkManager::findConnectionByUUID(TQString uuid) {
	if (!m_internalConnectionManager) return NULL;
	return m_internalConnectionManager->findConnectionByUUID(uuid);
}

TDENetworkDevice* TDEGlobalNetworkManager::findDeviceByUUID(TQString uuid) {
	if (!m_internalConnectionManager) return NULL;
	return m_internalConnectionManager->findDeviceByUUID(uuid);
}

TDENetworkWiFiAPInfo* TDEGlobalNetworkManager::findAccessPointByBSSID(TDEMACAddress bssid) {
	TDEHardwareDevices *hwdevices = TDEGlobal::hardwareDevices();
	if (!hwdevices) {
		return NULL;
	}

	TDENetworkWiFiAPInfo* ret = NULL;
	TDEGenericHardwareList devices = hwdevices->listByDeviceClass(TDEGenericDeviceType::Network);
	for (TDEGenericHardwareList::iterator it = devices.begin(); it != devices.end(); ++it) {
		TDENetworkDevice* dev = dynamic_cast<TDENetworkDevice*>(*it);
		if (dev) {
			TDENetworkConnectionManager* deviceConnMan = dev->connectionManager();
			if (deviceConnMan)
			{
				TDENetworkWiFiAPInfo* candidate = deviceConnMan->findAccessPointByBSSID(bssid);
				if (candidate)
				{
					ret = candidate;
					break;
				}
			}
		}
	}

	return ret;
}

/*================================================================================================*/
/* End                                                                                            */
/*================================================================================================*/

#include "tdenetworkconnections.moc"
