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
#ifndef _TDENETWORKCONNECTIONS_H
#define _TDENETWORKCONNECTIONS_H

// TDE includes
#include <tqobject.h>
#include <tqstring.h>
#include <tqptrlist.h>
#include <tqdatetime.h>
#include <tqstringlist.h>
#include <tqhostaddress.h>
#include "kiconloader.h"
#include "tdelibs_export.h"

#define CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(x)														\
	inline x operator|(x a, x b)																\
	{																			\
		return static_cast<x>(static_cast<int>(a) | static_cast<int>(b));										\
	}																			\
																				\
	inline x operator&(x a, x b)																\
	{																			\
		return static_cast<x>(static_cast<int>(a) & static_cast<int>(b));										\
	}																			\
																				\
	inline x operator~(x a)																	\
	{																			\
		return static_cast<x>(~static_cast<int>(a));													\
	}																			\
																				\
	inline x &operator|=(x& a, const x& b)															\
	{																			\
		a = static_cast<x>(static_cast<int>(a) | static_cast<int>(b));											\
		return a;																	\
	}																			\
																				\
	inline x &operator&=(x& a, const x& b)															\
	{																			\
		a = static_cast<x>(static_cast<int>(a) & static_cast<int>(b));											\
		return a;																	\
	}

class TDENetworkDevice;

typedef TQValueList<TQ_UINT8> TDENetworkByteList;
typedef TQValueList<TQHostAddress> TDENetworkAddressList;
typedef TQMap<TQString, TQString> TDENetworkSettingsMap;
typedef TQMap<TQ_UINT32, TQ_UINT32> TDENetworkPriorityMap;

namespace TDENetworkDeviceType {
	enum TDENetworkDeviceType {
		BackendOnly,
		WiredEthernet,
		WiFi,
		Bluetooth,
		OLPCMesh,
		WiMax,
		Modem,
		Infiniband,
		Bond,
		VLAN,
		ADSL,
		Other,
		Last = Other
	};
};

namespace TDENetworkConnectionType {
	enum TDENetworkConnectionType {
		WiredEthernet,
		WiFi,
		Bluetooth,
		OLPCMesh,
		WiMax,
		Modem,
		Infiniband,
		Bond,
		VLAN,
		ADSL,
		VPN,
		Other,
		Last = Other
	};
};

namespace TDENetworkConnectionErrorFlags {
	enum TDENetworkConnectionErrorFlags {
		NoError				= 0x00000000,
		InvalidConnectionSetting	= 0x00000001,
		InvalidIPv4Setting		= 0x00000002,
		InvalidIPv6Setting		= 0x00000004,
		InvalidEAPSetting		= 0x00000008,
		InvalidEAPKey			= 0x00000010,
		InvalidWirelessSetting		= 0x00000020,
		InvalidWirelessKey		= 0x00000040
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkConnectionErrorFlags)
};

typedef TQMap<TDENetworkConnectionErrorFlags::TDENetworkConnectionErrorFlags, TQString> TDENetworkErrorStringMap;

namespace TDENetworkInfinibandTransportMode {
	enum TDENetworkInfinibandTransportMode {
		Datagram,
		Connected,
		Other,
		Last = Other
	};
};

namespace TDEWiFiMode {
	enum TDEWiFiMode {
		AdHoc,
		Infrastructure,
		Other,
		Last = Other
	};
};

namespace TDEWiFiFrequencyBand {
	enum TDEWiFiFrequencyBand {
		None,
		Band2_4GHz,
		Band5GHz,
		Other,
		Last = Other
	};
};

namespace TDEBluetoothConnectionType {
	enum TDEBluetoothConnectionType {
		DUN,
		PAN,
		Other,
		Last = Other
	};
};

namespace TDEModemConnectionType {
	enum TDEModemConnectionType {
		CDMA,
		GSM,
		Other,
		Last = Other
	};
};

namespace TDEGSMNetworkType {
	enum TDEGSMNetworkType {
		Any,
		Only3G,
		GPRSEdge,
		Prefer3G,
		Prefer2G,
		Other,
		Last = Other
	};
};

namespace TDENetworkParity {
	enum TDENetworkParity {
		None,
		Even,
		Odd,
		Other,
		Last = Other
	};
};

namespace TDENetworkSlaveDeviceType {
	enum TDENetworkSlaveDeviceType {
		None,
		Bond,
		Other,
		Last = Other
	};
};

namespace TDENetworkGlobalEventType {
	enum TDENetworkGlobalEventType {
		ConnectionListChanged,
		Other,
		Last = Other
	};
};

namespace TDENetworkVPNEventType {
	enum TDENetworkVPNEventType {
		LoginBanner,
		Failure,
		Other,
		Last = Other
	};
};

namespace TDENetworkDeviceEventType {
	enum TDENetworkDeviceEventType {
		BitRateChanged,
		Failure,
		Other,
		Last = Other
	};
};

namespace TDENetworkAPEventType {
	enum TDENetworkAPEventType {
		Discovered,
		Lost,
		SignalStrengthChanged,
		AccessPointChanged,
		Other,
		Last = Other
	};
};

namespace TDENetworkGlobalManagerFlags {
	enum TDENetworkGlobalManagerFlags {
		Unknown			= 0x00000000,
		Disconnected		= 0x00000001,
		Connected		= 0x00000002,
		EstablishingLink	= 0x00000004,
		DeactivatingLink	= 0x00000008,
		LinkLocalAccess		= 0x00000010,
		SiteLocalAccess		= 0x00000020,
		GlobalAccess		= 0x00000040,
		Sleeping		= 0x00000080,
		BackendUnavailable	= 0x00000100,
		VPNUnknown		= 0x00000200,
		VPNEstablishingLink	= 0x00000400,
		VPNNeedAuthorization	= 0x00000800,
		VPNConfiguringProtocols	= 0x00001000,
		VPNVerifyingProtocols	= 0x00002000,
		VPNConnected		= 0x00004000,
		VPNFailed		= 0x00008000,
		VPNDisconnected		= 0x00010000,
		GlobalMask		= 0x000001ff,
		VPNMask			= 0x0001fe00
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkGlobalManagerFlags)
};

namespace TDENetworkDeviceCapabilityFlags {
	enum TDENetworkDeviceCapabilityFlags {
		None			= 0x00000000,
		Supported		= 0x00000001,
		CanDetectLink		= 0x00000002
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkDeviceCapabilityFlags)
};

namespace TDENetworkPPPFlags {
	enum TDENetworkPPPFlags {
		None			= 0x00000000,
		DisableEAP		= 0x00000001,
		DisablePAP		= 0x00000002,
		DisableCHAP		= 0x00000004,
		DisableMSCHAP		= 0x00000008,
		DisableMSCHAPv2		= 0x00000010,
		AllowBSDCompression	= 0x00000020,
		AllowDeflateCompression	= 0x00000040,
		AllowVJCompression	= 0x00000080,
		RequireMPPE		= 0x00000100,
		RequireMPPE128		= 0x00000200,
		StatefulMPPE		= 0x00000400,
		UseHardwareFlowControl	= 0x00000800
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkPPPFlags)
};

namespace TDENetworkWiFiAPFlags {
	enum TDENetworkWiFiAPFlags {
		None			= 0x00000000,
		PrivacySupport		= 0x00000001,
		PairWEP40		= 0x00000002,
		PairWEP104		= 0x00000004,
		PairTKIP		= 0x00000008,
		PairCCMP		= 0x00000010,
		GroupWEP40		= 0x00000020,
		GroupWEP104		= 0x00000040,
		GroupTKIP		= 0x00000080,
		GroupCCMP		= 0x00000100,
		KeyManagementPSK	= 0x00000200,
		KeyManagement80211	= 0x00000400,
		GeneralFlagsMask	= 0x00000001,
		EncryptionFlagsMask	= 0xfffffffe
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkWiFiAPFlags)
};

namespace TDENetworkWiFiClientFlags {
	enum TDENetworkWiFiClientFlags {
		None			= 0x00000000,
		CipherWEP40		= 0x00000002,
		CipherWEP104		= 0x00000004,
		CipherTKIP		= 0x00000008,
		CipherCCMP		= 0x00000010,
		CipherWPA		= 0x00000020,
		CipherRSN		= 0x00000040
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkWiFiClientFlags)
};

namespace TDENetworkVPNType {
	enum TDENetworkVPNType {
		OpenVPN,
		PPTP,
		StrongSwan,
		VPNC,
		Other,
		Last = Other
	};
};

typedef TQValueList<TDENetworkVPNType::TDENetworkVPNType> TDENetworkVPNTypeList;

namespace TDENetworkWiFiConnectionCipher {
	enum TDENetworkWiFiConnectionCipher {
		None,
		CipherWEP40,
		CipherWEP104,
		CipherTKIP,
		CipherCCMP,
		CipherWPA,
		CipherRSN,
		Any
	};
};

typedef TQValueList<TDENetworkWiFiConnectionCipher::TDENetworkWiFiConnectionCipher> TDENetworkWiFiConnectionCipherList;

namespace TDENetworkWepKeyType {
	enum TDENetworkWepKeyType {
		Hexadecimal,
		Ascii,
		Passphrase
	};
};

namespace TDENetworkVLANFlags {
	enum TDENetworkVLANFlags {
		None			= 0x00000000,
		ReorderPacketHeaders	= 0x00000001,
		UseGVRP			= 0x00000002,
		LooseBinding		= 0x00000004
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkVLANFlags)
};

namespace TDENetworkWiFiKeyType {
	enum TDENetworkWiFiKeyType {
		WEP,
		DynamicWEP,
		WPAAdHoc,
		WPAInfrastructure,
		WPAEnterprise,
		Other,
		Last = Other
	};
};

namespace TDENetworkWiFiAuthType {
	enum TDENetworkWiFiAuthType {
		Open,
		Shared,
		LEAP,
		Other,
		Last = Other
	};
};

namespace TDENetworkIEEE8021xType {
	enum TDENetworkIEEE8021xType {
		None,
		LEAP,
		MD5,
		PAP,
		CHAP,
		MSCHAP,
		MSCHAPV2,
		Fast,
		PSK,
		PAX,
		SAKE,
		GPSK,
		TLS,
		PEAP,
		TTLS,
		SIM,
		GTC,
		OTP
	};
};

typedef TQValueList<TDENetworkIEEE8021xType::TDENetworkIEEE8021xType> TDENetworkIEEE8021xTypeList;

namespace TDENetworkIEEE8021xFastFlags {
	enum TDENetworkIEEE8021xFastFlags {
		None			= 0x00000000,
		AllowUnauthenticated	= 0x00000001,
		AllowAuthenticated	= 0x00000002
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkIEEE8021xFastFlags)
};

namespace TDENetworkWiFiWPAVersionFlags {
	enum TDENetworkWiFiWPAVersionFlags {
		None			= 0x00000000,
		WPA			= 0x00000001,
		RSN			= 0x00000002,
		Any			= 0x00000003
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkWiFiWPAVersionFlags)
};

namespace TDENetworkPasswordHandlingFlags {
	enum TDENetworkPasswordHandlingFlags {
		None			= 0x00000000,
		NoSave			= 0x00000001,
		NoPrompt		= 0x00000002,
		ExternalStorage		= 0x00000004
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkPasswordHandlingFlags)
};

namespace TDENetworkConnectionStatus {
	enum TDENetworkConnectionStatus {
		None			= 0x00000000,
		Invalid			= 0x00000001,
		Disconnected		= 0x00000002,
		Connected		= 0x00000004,
		LinkUnavailable		= 0x00000008,
		EstablishingLink	= 0x00000010,
		DeactivatingLink	= 0x00000020,
		ConfiguringProtocols	= 0x00000040,
		Reconnecting		= 0x00000080,
		LinkLost		= 0x00000100,
		LinkLocalAccess		= 0x00000200,
		SiteLocalAccess		= 0x00000400,
		GlobalAccess		= 0x00000800,
		UnManaged		= 0x00001000,
		NeedAuthorization	= 0x00002000,
		Failed			= 0x00004000,
		VerifyingProtocols	= 0x00008000,
		DependencyWait		= 0x00010000
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkConnectionStatus)
};

namespace TDENetworkIPConfigurationFlags {
	enum TDENetworkIPConfigurationFlags {
		None					= 0x00000000,
		IPV4DHCPIP				= 0x00000001,
		IPV4DHCPDNS				= 0x00000002,
		IPV4DHCPRoutes				= 0x00000004,
		IPV4LocalOnly				= 0x00000008,
		IPV4StartConnectionSharingServer	= 0x00000010,
		IPV4Disabled				= 0x00000020,
		IPV4MayUseAsDefaultRoute		= 0x00000040,
		IPV6DHCPIP				= 0x00000080,
		IPV6DHCPDNS				= 0x00000100,
		IPV6DHCPRoutes				= 0x00000200,
		IPV6LocalOnly				= 0x00000400,
		IPV6StartConnectionSharingServer	= 0x00000800,
		IPV6Disabled				= 0x00001000,
		IPV6MayUseAsDefaultRoute		= 0x00002000
	};

	CREATE_FLAG_BITWISE_MANIPULATION_FUNCTIONS(TDENetworkIPConfigurationFlags)
};

class TDECORE_EXPORT TDENetworkSearchDomain
{
	public:
		TDENetworkSearchDomain();
		TDENetworkSearchDomain(TQString domain, bool ipv6=false);
		~TDENetworkSearchDomain();

		TQString searchDomain();
		void setSearchDomain(TQString domain, bool ipv6=false);

		bool isIPv4();
		bool isIPv6();

	private:
		TQString m_domain;
		bool m_isIPV6;
};

typedef TQValueList<TDENetworkSearchDomain> TDENetworkSearchDomainList;

class TDECORE_EXPORT TDENetMask
{
	public:
		TDENetMask();
		TDENetMask(TQ_UINT32 netmask);
		TDENetMask(TQ_UINT8* netmask);
		~TDENetMask();

		void fromCIDRMask(unsigned char mask, bool ipv6=false);
		unsigned char toCIDRMask();
		void fromString(TQString mask);
		TQString toString();

		bool isIPv4();
		bool isIPv6();

	private:
		TQ_UINT32 m_ipv4NetMask;
		TQHostAddress m_ipv6NetMask;
		bool m_isIPV6;
};

class TDECORE_EXPORT TDEMACAddress
{
	public:
		TDEMACAddress();
		TDEMACAddress(TDENetworkByteList address);
		~TDEMACAddress();

		TDENetworkByteList address();
		void setAddress(TDENetworkByteList address);
		TQString toString();
		void fromString(TQString address);
		bool isValid();

	private:
		bool m_isValid;
		TDENetworkByteList m_macAddress;

		friend bool operator==(const TDEMACAddress &a1, const TDEMACAddress &a2);
		friend bool operator<(const TDEMACAddress &a1, const TDEMACAddress &a2);
};

TDECORE_EXPORT bool operator==(const TDEMACAddress &a1, const TDEMACAddress &a2);
TDECORE_EXPORT bool operator<(const TDEMACAddress &a1, const TDEMACAddress &a2);

typedef TQValueList<TDEMACAddress> TDEMACAddressList;

class TDECORE_EXPORT TDENetworkSingleIPConfiguration
{
	public:
		TDENetworkSingleIPConfiguration();
		~TDENetworkSingleIPConfiguration();

		bool isIPv4();
		bool isIPv6();

	public:
		bool valid;
		TQHostAddress ipAddress;
		TDENetMask networkMask;
		TQHostAddress gateway;
};

typedef TQValueList<TDENetworkSingleIPConfiguration> TDENetworkSingleIPConfigurationList;

class TDECORE_EXPORT TDENetworkSingleRouteConfiguration
{
	public:
		TDENetworkSingleRouteConfiguration();
		~TDENetworkSingleRouteConfiguration();

		bool isIPv4();
		bool isIPv6();

	public:
		bool valid;
		TQHostAddress ipAddress;
		TDENetMask networkMask;
		TQHostAddress gateway;
		TQ_UINT32 metric;
};

typedef TQValueList<TDENetworkSingleRouteConfiguration> TDENetworkSingleRouteConfigurationList;

class TDECORE_EXPORT TDENetworkIEEE8021xConfiguration
{
	public:
		TDENetworkIEEE8021xConfiguration();
		~TDENetworkIEEE8021xConfiguration();

	public:
		bool valid;
		bool allowedValid;
		bool secretsValid;
		TDENetworkIEEE8021xType::TDENetworkIEEE8021xType type;
		TQString userName;
		TQString anonymousUserName;
		TQString pacFileName;
		TQByteArray caCertificate;
		TQString additionalCAFilesPath;
		TQString authServerCertSubjectMatch;
		TQStringList alternateAuthServerCertSubjectMatch;
		TQByteArray clientCertificate;
		TQString forcePEAPVersion;
		TQString forcePEAPLabel;
		TDENetworkIEEE8021xFastFlags::TDENetworkIEEE8021xFastFlags fastProvisioningFlags;
		TDENetworkIEEE8021xType::TDENetworkIEEE8021xType phase2NonEAPAuthMethod;
		TDENetworkIEEE8021xType::TDENetworkIEEE8021xType phase2EAPAuthMethod;
		TDENetworkIEEE8021xTypeList allowedPhase2NonEAPMethods;
		TDENetworkIEEE8021xTypeList allowedPhase2EAPMethods;
		TQByteArray phase2CaCertificate;
		TQString phase2CaFilesPath;
		TQString phase2AuthServerCertSubjectMatch;
		TQStringList phase2AlternateAuthServerCertSubjectMatch;
		TQByteArray phase2ClientCertificate;
		TQString password;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags passwordFlags;
		TQByteArray binaryPassword;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags binaryPasswordFlags;
		TQByteArray privateKey;
		TQString privateKeyPassword;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags privateKeyPasswordFlags;
		TQByteArray phase2PrivateKey;
		TQString phase2PrivateKeyPassword;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags phase2PrivateKeyPasswordFlags;
		bool forceSystemCaCertificates;
};

class TDECORE_EXPORT TDENetworkPPPConfiguration
{
	public:
		TDENetworkPPPConfiguration();
		virtual ~TDENetworkPPPConfiguration();

	public:
		bool valid;
		bool requireServerAuthentication;
		TDENetworkPPPFlags::TDENetworkPPPFlags flags;
		TQ_UINT32 baudRate;
		TQ_UINT32 mru;
		TQ_UINT32 mtu;
		TQ_UINT32 lcpEchoPingInterval;
		TQ_UINT32 lcpEchoFailureThreshold;
};

class TDECORE_EXPORT TDENetworkPPPOEConfiguration
{
	public:
		TDENetworkPPPOEConfiguration();
		virtual ~TDENetworkPPPOEConfiguration();

	public:
		bool valid;
		bool secretsValid;
		TQString networkServiceProvider;
		TQString username;
		TQString password;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags passwordFlags;
};

class TDECORE_EXPORT TDENetworkSerialConfiguration
{
	public:
		TDENetworkSerialConfiguration();
		virtual ~TDENetworkSerialConfiguration();

	public:
		bool valid;
		TQ_UINT32 baudRate;
		TQ_UINT32 byteWidth;
		TDENetworkParity::TDENetworkParity parity;
		TQ_UINT32 stopBits;
		TQ_UINT64 txDelay;
};

class TDECORE_EXPORT TDENetworkCDMAConfiguration
{
	public:
		TDENetworkCDMAConfiguration();
		virtual ~TDENetworkCDMAConfiguration();

	public:
		bool valid;
		bool secretsValid;
		TQString providerDataNumber;
		TQString username;
		TQString password;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags passwordFlags;
};

class TDECORE_EXPORT TDENetworkGSMConfiguration
{
	public:
		TDENetworkGSMConfiguration();
		virtual ~TDENetworkGSMConfiguration();

	public:
		bool valid;
		bool secretsValid;
		TQString providerDataNumber;
		TQString username;
		TQString password;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags passwordFlags;
		TQString accessPointName;
		TQString networkID;
		TDEGSMNetworkType::TDEGSMNetworkType networkType;
		TQString pin;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags pinFlags;
		TQ_UINT32 allowedFrequencyBands;
		bool allowRoaming;
};
		

class TDENetworkWiFiSecurityConfiguration
{
	public:
		TDENetworkWiFiSecurityConfiguration();
		~TDENetworkWiFiSecurityConfiguration();

	public:
		bool valid;
		bool secretsValid;
		TDENetworkWiFiKeyType::TDENetworkWiFiKeyType keyType;
		TDENetworkWiFiAuthType::TDENetworkWiFiAuthType authType;
		TDENetworkWiFiWPAVersionFlags::TDENetworkWiFiWPAVersionFlags wpaVersion;
		TDENetworkWiFiConnectionCipher::TDENetworkWiFiConnectionCipher cipher;
		TQString wepKey0;
		TQString wepKey1;
		TQString wepKey2;
		TQString wepKey3;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags wepKeyFlags;
		TQ_UINT32 wepKeyIndex;
		TDENetworkWepKeyType::TDENetworkWepKeyType wepKeyType;
		TDENetworkWiFiConnectionCipherList allowedPairWiseCiphers;
		TDENetworkWiFiConnectionCipherList allowedGroupWiseCiphers;
		TQString psk;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags pskFlags;
		TQString leapUsername;
		TQString leapPassword;
		TDENetworkPasswordHandlingFlags::TDENetworkPasswordHandlingFlags leapPasswordFlags;
};

class TDECORE_EXPORT TDENetworkIPConfiguration
{
	public:
		TDENetworkIPConfiguration();
		~TDENetworkIPConfiguration();

	public:
		bool valid;
		TDENetworkIPConfigurationFlags::TDENetworkIPConfigurationFlags connectionFlags;
		TDENetworkSingleIPConfigurationList ipConfigurations;
		TDENetworkSingleRouteConfigurationList routeConfigurations;
		TQHostAddress broadcast;
		TQHostAddress destination;
		TDENetworkAddressList resolvers;
		TDENetworkSearchDomainList searchDomains;
		TQString dhcpClientIdentifier;
};

class TDECORE_EXPORT TDENetworkWiFiDeviceInfo
{
	public:
		TDENetworkWiFiDeviceInfo();
		~TDENetworkWiFiDeviceInfo();

	public:
		bool valid;
		TDEMACAddress hwAddress;
		TDEMACAddress permanentHWAddress;
		TDEWiFiMode::TDEWiFiMode operatingMode;
		unsigned int bitrate;
		TDEMACAddress activeAccessPointBSSID;
		TDENetworkWiFiClientFlags::TDENetworkWiFiClientFlags wirelessFlags;
};

class TDECORE_EXPORT TDENetworkDeviceInformation
{
	public:
		TDENetworkDeviceInformation();
		~TDENetworkDeviceInformation();

	public:
		bool valid;
		TDENetworkConnectionStatus::TDENetworkConnectionStatus statusFlags;
		TQString UUID;
		TQString backendDriver;
		TQString backendDriverVersion;
		TQString firmwareVersion;
		TDENetworkDeviceCapabilityFlags::TDENetworkDeviceCapabilityFlags capabilityFlags;
		TDENetworkIPConfiguration ipConfiguration;
		bool managed;
		bool autoConnect;
		bool firmwareMissing;
		TDENetworkDeviceType::TDENetworkDeviceType deviceType;
		TQString activeConnectionUUID;
		TDENetworkWiFiDeviceInfo wiFiInfo;
};

class TDECORE_EXPORT TDENetworkHWNeighbor
{
	public:
		TDENetworkHWNeighbor();
		virtual ~TDENetworkHWNeighbor();

	public:
		bool valid;
};

class TDECORE_EXPORT TDENetworkWiFiAPInfo : public TDENetworkHWNeighbor
{
	public:
		TDENetworkWiFiAPInfo();
		virtual ~TDENetworkWiFiAPInfo();

	public:
		TQString friendlySSID() const;

	public:
		TQByteArray SSID;
		TDENetworkWiFiAPFlags::TDENetworkWiFiAPFlags wpaFlags;
		TDENetworkWiFiAPFlags::TDENetworkWiFiAPFlags rsnFlags;
		unsigned int frequency;
		TDEMACAddress BSSID;
		unsigned int maxBitrate;
		double signalQuality;
};

typedef TQPtrList< TDENetworkHWNeighbor > TDENetworkHWNeighborList;

class TDECORE_EXPORT TDENetworkConnection
{
	public:
		TDENetworkConnection();
		virtual ~TDENetworkConnection();

	public:
		TDENetworkConnectionType::TDENetworkConnectionType type();

	public:
		TQString UUID;
		TQString friendlyName;
		TDENetworkIPConfiguration ipConfig;
		TDEMACAddress lockedHWAddress;
		TDEMACAddress manualHWAddress;
		bool readOnly;
		bool autoConnect;
		bool fullDuplex;
		bool requireIPV4;
		bool requireIPV6;
		TQ_UINT32 mtu;
		TDENetworkIEEE8021xConfiguration eapConfig;
		TDENetworkPPPConfiguration pppConfig;
		TDENetworkPPPOEConfiguration pppoeConfig;
		TDENetworkSerialConfiguration serialConfig;
		TQStringList authorizedUsers;
		TQString masterConnectionUUID;
		TDENetworkSlaveDeviceType::TDENetworkSlaveDeviceType slaveType;
		TQDateTime lastKnownConnection;
};

class TDECORE_EXPORT TDEWiredEthernetConnection : public TDENetworkConnection
{
	public:
		TDEWiredEthernetConnection();
		virtual ~TDEWiredEthernetConnection();
};

class TDECORE_EXPORT TDEWiFiConnection : public TDENetworkConnection
{
	public:
		TDEWiFiConnection();
		virtual ~TDEWiFiConnection();

	public:
		TQByteArray SSID;
		TDEWiFiMode::TDEWiFiMode operatingMode;
		TDEWiFiFrequencyBand::TDEWiFiFrequencyBand bandRestriction;
		TQ_INT32 channelRestriction;
		TQ_INT32 bitRateRestriction;
		TQ_INT32 powerRestriction;
		TDEMACAddress accessPointRestriction;
		TDEMACAddressList blacklistedBSSIDs;
		TDEMACAddressList heardBSSIDs;
		bool isHiddenNetwork;
		bool securityRequired;
		TDENetworkWiFiSecurityConfiguration securitySettings;
};

class TDECORE_EXPORT TDEWiredInfinibandConnection : public TDENetworkConnection
{
	public:
		TDEWiredInfinibandConnection();
		virtual ~TDEWiredInfinibandConnection();

	public:
		TDENetworkInfinibandTransportMode::TDENetworkInfinibandTransportMode transportMode;
};

class TDECORE_EXPORT TDEVPNConnection : public TDENetworkConnection
{
	public:
		TDEVPNConnection();
		virtual ~TDEVPNConnection();

	public:
		TQString vpnPluginID;
		TQString lockedUserName;
		TDENetworkSettingsMap pluginData;
		bool secretsValid;
		TDENetworkSettingsMap pluginSecrets;
};

class TDECORE_EXPORT TDEWiMaxConnection : public TDENetworkConnection
{
	public:
		TDEWiMaxConnection();
		virtual ~TDEWiMaxConnection();

	public:
		TQString networkServiceProvider;
};

class TDECORE_EXPORT TDEVLANConnection : public TDENetworkConnection
{
	public:
		TDEVLANConnection();
		virtual ~TDEVLANConnection();

	public:
		TQString kernelName;
		TQString parentConnectionUUID;
		TQ_UINT32 vlanID;
		TDENetworkVLANFlags::TDENetworkVLANFlags vlanFlags;
		TDENetworkPriorityMap ingressPriorityMap;
		TDENetworkPriorityMap egressPriorityMap;
};

class TDECORE_EXPORT TDEOLPCMeshConnection : public TDENetworkConnection
{
	public:
		TDEOLPCMeshConnection();
		virtual ~TDEOLPCMeshConnection();

	public:
		TQByteArray SSID;
		TQ_INT32 channel;
		TQByteArray anycastDHCPHWAddress;
};

class TDECORE_EXPORT TDEBluetoothConnection : public TDENetworkConnection
{
	public:
		TDEBluetoothConnection();
		virtual ~TDEBluetoothConnection();

	public:
		TDEBluetoothConnectionType::TDEBluetoothConnectionType type;
};

class TDECORE_EXPORT TDEModemConnection : public TDENetworkConnection
{
	public:
		TDEModemConnection();
		virtual ~TDEModemConnection();

	public:
		TDEModemConnectionType::TDEModemConnectionType type;
		TDENetworkCDMAConfiguration cdmaConfig;
		TDENetworkGSMConfiguration gsmConfig;
};

typedef TQPtrList< TDENetworkConnection > TDENetworkConnectionList;

/**
* INTERNAL CLASS
*/
class TDENetworkEventQueueEvent_Private
{
	public:
		int eventType;
		TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags newState;
		TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags previousState;
		TDENetworkConnectionStatus::TDENetworkConnectionStatus newConnStatus;
		TDENetworkConnectionStatus::TDENetworkConnectionStatus previousConnStatus;
		TDEMACAddress BSSID;
		TQString message;
		TQString deviceNode;
		TDENetworkAPEventType::TDENetworkAPEventType apevent;
		TDENetworkDeviceEventType::TDENetworkDeviceEventType ndevent;
		TDENetworkVPNEventType::TDENetworkVPNEventType vpnevent;
		TDENetworkGlobalEventType::TDENetworkGlobalEventType globalevent;
};
typedef TQValueList<TDENetworkEventQueueEvent_Private> TDENetworkEventQueueEvent_PrivateList;

class TQTimer;

class TDECORE_EXPORT TDENetworkConnectionManager : public TQObject
{
	TQ_OBJECT

	public:
		/**
		*  Constructor.
		*  @param networkDevice Pointer to holder network device
		*  If the network device is passed null, this object will make global networking backend
		*  methods available exclusively (TDENetworkDeviceType::BackendOnly).
		*/
		TDENetworkConnectionManager(TDENetworkDevice *networkDevice);

		/**
		* Destructor.
		*/
		~TDENetworkConnectionManager();

		/**
		* @return a TQString containing the name of the backend in use
		*/
		virtual TQString backendName() = 0;

		/**
		* @return the type of connection supported by this device
		*/
		virtual TDENetworkDeviceType::TDENetworkDeviceType deviceType() = 0;

		/**
		* @return A TDENetworkGlobalManagerFlags enum value with the current status of the networking backend.
		*/
		virtual TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags backendStatus() = 0;

		/**
		* @return A TDENetworkDeviceInformation object containing the current configuration and status of the network device.
		*/
		virtual TDENetworkDeviceInformation deviceInformation() = 0;

		/**
		* @return A TDENetworkDeviceInformation object containing a (limited) current status of the network device.
		* Only the following object fields are populated:
		* statusFlags
		* UUID
		* activeConnectionUUID
		* valid
		*/
		virtual TDENetworkDeviceInformation deviceStatus() = 0;

		/**
		* Loads all connection information from the configuration backend
		* Secret information must be loaded separately via a call to
		* loadConnectionSecrets(TQString uuid) after this method has been
		* executed at least once.
		*/
		virtual void loadConnectionInformation() = 0;

		/**
		* @param connection a pointer to a TDENetworkConnection object containing a
		* connection in which to load the values allowed by the backend.
		* This is normally called as part of loadConnectionInformation(), but should
		* manually be called immediately after creation of a new TDENetworkConnection object.
		*/
		virtual void loadConnectionAllowedValues(TDENetworkConnection* connection) = 0;

		/**
		* @param uuid a TQString conntaining the UUID of a connection for which to
		* load secrets from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool loadConnectionSecrets(TQString uuid) = 0;

		/**
		* @param connection a pointer to a TDENetworkConnection object containing a
		* connection to save to the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool saveConnection(TDENetworkConnection* connection) = 0;

		/**
		* @param uuid a TQString conntaining the UUID of a connection to
		* delete from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool deleteConnection(TQString uuid) = 0;

		/**
		* @param connection a pointer to a TDENetworkConnection object containing a
		* connection for which to verify integrity of all settings.
		* @param type a pointer to an TDENetworkConnectionErrorFlags::TDENetworkConnectionErrorFlags
		* which will be filled with the generic error type code if provided.
		* @param reason a pointer to a TDENetworkErrorStringMap which will be filled with translated
		* strings containing the reason for the failure if provided.
		* @return true on success, false if invalid settings are detected.
		*/
		virtual bool verifyConnectionSettings(TDENetworkConnection* connection, TDENetworkConnectionErrorFlags::TDENetworkConnectionErrorFlags* type=NULL, TDENetworkErrorStringMap* reason=NULL) = 0;

		/**
		* Initiates a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		* Note that if this manager's type is not TDENetworkDeviceType::BackendOnly, the connection
		* will be initiated on the internal device specified when this object was created
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus initiateConnection(TQString uuid) = 0;

		/**
		* Checks the status of a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus checkConnectionStatus(TQString uuid) = 0;

		/**
		* Disconnects a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus deactivateConnection(TQString uuid) = 0;

		/**
		* @return a TQStringList object containing all valid network settings
		* Each string has the form "TDENetworkConfigObject::member"
		* If a setting is not in this list, it is not supported by the backend in use
		*/
		virtual TQStringList validSettings() = 0;

		/**
		* @return a TDENetworkHWNeighborList object containing the result of a site survey;
		* i.e. all nearby access points or devices. This function only returns valid information
		* if the underlying network device supports site surveys.
		*
		* Note that the returned list is internally managed and must not be deleted!
		* Also note that pointers in the list may become invalid on subsequent calls to
		* siteSurvey().
		*/
		virtual TDENetworkHWNeighborList* siteSurvey() = 0;

		/**
		* @return a TQStringList containing the UUIDs of all physical devices used by the connection
		* with UUID @param uuid.
		* This function may return an empty list if the connection is inactive, this behaviour is
		* dependend on the specific network backend in use.
		*/
		virtual TQStringList connectionPhysicalDeviceUUIDs(TQString uuid) = 0;

		/**
		* @return a TDENetworkVPNTypeList object containing all supported VPN types
		* If a type is not in this list, it is not supported by the backend in use
		*/
		virtual TDENetworkVPNTypeList availableVPNTypes() = 0;

		/**
		* @return true if networking is enabled, false if not.
		*/
		virtual bool networkingEnabled() = 0;

		/**
		* @param enable true to enable networking, false to disable it.
		* @return true on success, false on failure.
		*/
		virtual bool enableNetworking(bool enable) = 0;

		/**
		* @return true if WiFi hardware is enabled, false if not.
		*/
		virtual bool wiFiHardwareEnabled() = 0;

		/**
		* @param enable true to enable WiFi, false to disable it.
		* @return true on success, false on failure.
		*/
		virtual bool enableWiFi(bool enable) = 0;

		/**
		* @return true if WiFi is enabled, false if not.
		*/
		virtual bool wiFiEnabled() = 0;

		/**
		* @return a list of UUIDs of the default network devices, or an empty list if no such devices exist.
		* The default network devices are normally the devices holding the highest priority default route.
		*/
		virtual TQStringList defaultNetworkDevices() = 0;

	signals:
		/**
		* Emitted whenever the state of the system's connection changes
		* If previous state data was unavailable, @param previousState will contain TDENetworkConnectionStatus::Invalid
		*/
		void networkConnectionStateChanged(TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags newState, TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags previousState);

		/**
		* Emitted whenever the state of a device changes
		* If previous state data was unavailable, @param previousState will contain TDENetworkConnectionStatus::Invalid
		* If the global connection state has changed, @param hwAddress will be empty, otherwise it will contain the MAC address
		* of the networking hardware that has changed state.
		*/
		void networkDeviceStateChanged(TDENetworkConnectionStatus::TDENetworkConnectionStatus newState, TDENetworkConnectionStatus::TDENetworkConnectionStatus previousState, TQString deviceNode);

		/**
		* Emitted whenever the status of a wireless access point changes
		* The event type that caused the signal is available in @param event
		*/
		void accessPointStatusChanged(TDEMACAddress BSSID, TDENetworkAPEventType::TDENetworkAPEventType event);

		/**
		* Emitted whenever a network device event occurs
		* The event type that caused the signal is available in @param event
		* @param message contains additional information if available
		*/
		void networkDeviceEvent(TDENetworkDeviceEventType::TDENetworkDeviceEventType event, TQString message);

		/**
		* Emitted whenever a VPN-related event occurs
		* The event type that caused the signal is available in @param event
		* @param message contains additional information if available
		*/
		void vpnEvent(TDENetworkVPNEventType::TDENetworkVPNEventType event, TQString message);

		/**
		* Emitted whenever a global network management event occurs
		* The event type that caused the signal is available in @param event
		*/
		void networkManagementEvent(TDENetworkGlobalEventType::TDENetworkGlobalEventType event);

	public:
		/**
		* @return a TDENetworkConnectionList object containing a list of all
		* possible connections this connection manager is aware of, regardless
		* of current state or availability.
		*
		* loadConnectionInformation() should be called at least once before calling
		* this method, in order to update internal connection information from the
		* configuration backend.
		*
		* Note that the returned list is internally managed and must not be deleted!
		* Also note that pointers in the list may become invalid on subsequent calls to
		* loadConnectionInformation(), saveConnection(), deleteConnection(), or connections().
		*/
		virtual TDENetworkConnectionList* connections();

		/**
		* @return the interface name of this device
		*/
		TQString deviceNode();

		/**
		* @return the MAC address of this device
		*/
		TQString deviceMACAddress();

		/**
		* @return a pointer to a TDENetworkConnection object with the specified @param uuid,
		* or a NULL pointer if no such connection exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		TDENetworkConnection* findConnectionByUUID(TQString uuid);

		/**
		* @return a pointer to a TDENetworkDevice object with the specified @param uuid,
		* or a NULL pointer if no such device exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		TDENetworkDevice* findDeviceByUUID(TQString uuid);

		/**
		* @return a pointer to a TDENetworkWiFiAPInfo object with the specified @param bssid,
		* or a NULL pointer if no such access point exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		TDENetworkWiFiAPInfo* findAccessPointByBSSID(TDEMACAddress bssid);

		/**
		* @return a string containing the friendly name of the connection type @param type given
		*/
		static TQString friendlyConnectionTypeName(TDENetworkConnectionType::TDENetworkConnectionType type);

		/**
		* @return true if @param address is valid, false if not
		*/
		static bool validateIPAddress(TQHostAddress address);

		/**
		* @return true if @param netmask is valid, false if not
		*/
		static bool validateIPNeworkMask(TQHostAddress netmask);

	protected:
		/**
		* @internal Safely clears out the master connection list and deletes all member objects
		*/
		void clearTDENetworkConnectionList();
		
		/**
		* @internal Safely clears out the neighboring devices list and deletes all member objects
		*/
		void clearTDENetworkHWNeighborList();

		/**
		* @internal This method must be called by the network backend whenever a connection changes state
		* It emits the appropriate signals to notify client applications of the state change
		*/
		void internalNetworkConnectionStateChanged(TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags newState);

		/**
		* @internal This method must be called by the network backend whenever a device changes state
		* It emits the appropriate signals to notify client applications of the state change
		*/
		void internalNetworkDeviceStateChanged(TDENetworkConnectionStatus::TDENetworkConnectionStatus newState, TQString deviceNode=TQString::null);

		/**
		* @internal This method must be called by the network backend whenever a wireless access point changes state
		* It emits the appropriate signals to notify client applications of the state change
		*/
		void internalAccessPointStatusChanged(TDEMACAddress BSSID, TDENetworkAPEventType::TDENetworkAPEventType event);

		/**
		* @internal This method must be called by the network backend whenever a device event occurs
		* It emits the appropriate signals to notify client applications of the network device event
		*/
		void internalNetworkDeviceEvent(TDENetworkDeviceEventType::TDENetworkDeviceEventType event, TQString message);

		/**
		* @internal This method must be called by the network backend whenever a VPN event occurs
		* It emits the appropriate signals to notify client applications of the network device event
		*/
		void internalVpnEvent(TDENetworkVPNEventType::TDENetworkVPNEventType event, TQString message);

		/**
		* @internal This method must be called by the network backend whenever it changes state
		* It emits the appropriate signals to notify client applications of the state change
		*/
		void internalNetworkManagementEvent(TDENetworkGlobalEventType::TDENetworkGlobalEventType event);

	protected slots:
		void emitQueuedSignals();

	protected:
		TDENetworkConnectionList* m_connectionList;
		TDENetworkHWNeighborList* m_hwNeighborList;
		TDENetworkDevice* m_networkDevice;
		TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags m_prevConnectionStatus;
		TQMap<TQString, TDENetworkConnectionStatus::TDENetworkConnectionStatus> m_prevDeviceStatus;
		TQTimer* m_emissionTimer;
		TDENetworkEventQueueEvent_PrivateList m_globalEventQueueEventList;
};

class TDECORE_EXPORT TDEGlobalNetworkManager : public TQObject
{
	TQ_OBJECT

	public:
		/**
		*  Constructor.
		*/
		TDEGlobalNetworkManager();
		
		/**
		* Destructor.
		*/
		~TDEGlobalNetworkManager();

		/**
		* @return a TQString containing the name of the backend in use
		*/
		virtual TQString backendName();

		/**
		* @return A TDENetworkGlobalManagerFlags enum value with the current status of the networking backend.
		*/
		virtual TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags backendStatus();

		/**
		* Loads all connection information from the configuration backend
		* Secret information must be loaded separately via a call to
		* loadConnectionSecrets(TQString uuid) after this method has been
		* executed at least once.
		*/
		virtual void loadConnectionInformation();

		/**
		* @param connection a pointer to a TDENetworkConnection object containing a
		* connection in which to load the values allowed by the backend.
		* This is normally called as part of loadConnectionInformation(), but should
		* manually be called immediately after creation of a new TDENetworkConnection object.
		*/
		virtual void loadConnectionAllowedValues(TDENetworkConnection* connection);

		/**
		* @param uuid a TQString conntaining the UUID of a connection for which to
		* load secrets from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool loadConnectionSecrets(TQString uuid);

		/**
		* @param connection a pointer to a TDENetworkConnection object containing a
		* connection to save to the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool saveConnection(TDENetworkConnection* connection);

		/**
		* @param uuid a TQString conntaining the UUID of a connection to
		* delete from the configuration backend.
		* @return true on success, false on failure.
		*/
		virtual bool deleteConnection(TQString uuid);

		/**
		* @param connection a pointer to a TDENetworkConnection object containing a
		* connection for which to verify integrity of all settings.
		* @param type a pointer to an TDENetworkConnectionErrorFlags::TDENetworkConnectionErrorFlags
		* which will be filled with the generic error type code if provided.
		* @param reason a pointer to a TDENetworkErrorStringMap which will be filled with translated
		* strings containing the reason for the failure if provided.
		* @return true on success, false if invalid settings are detected.
		*/
		virtual bool verifyConnectionSettings(TDENetworkConnection* connection, TDENetworkConnectionErrorFlags::TDENetworkConnectionErrorFlags* type=NULL, TDENetworkErrorStringMap* reason=NULL);

		/**
		* Initiates a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus initiateConnection(TQString uuid);

		/**
		* Checks the status of a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus checkConnectionStatus(TQString uuid);

		/**
		* Disconnects a connection with UUID @param uuid.
		* @return A TDENetworkConnectionStatus enum value with the current connection status
		* The client application should poll for status updates using checkConnectionStatus()
		*/
		virtual TDENetworkConnectionStatus::TDENetworkConnectionStatus deactivateConnection(TQString uuid);

		/**
		* @return a TQStringList object containing all valid network settings
		* Each string has the form "TDENetworkConfigObject::member"
		* If a setting is not in this list, it is not supported by the backend in use
		*/
		virtual TQStringList validSettings();

		/**
		* @return a TDENetworkHWNeighborList object containing the result of a site survey;
		* i.e. all nearby access points or devices. This function only returns valid information
		* if the underlying network device supports site surveys.
		*
		* Note that the returned list is internally managed and must not be deleted!
		* Also note that pointers in the list may become invalid on subsequent calls to
		* siteSurvey().
		*/
		virtual TDENetworkHWNeighborList* siteSurvey();

		/**
		* @return a TQStringList containing the UUIDs of all physical devices used by the connection
		* with UUID @param uuid.
		* This function may return an empty list if the connection is inactive, this behaviour is
		* dependend on the specific network backend in use.
		*/
		virtual TQStringList connectionPhysicalDeviceUUIDs(TQString uuid);

		/**
		* @return a TDENetworkVPNTypeList object containing all supported VPN types
		* If a type is not in this list, it is not supported by the backend in use
		*/
		virtual TDENetworkVPNTypeList availableVPNTypes();

		/**
		* @return true if networking is enabled, false if not.
		*/
		virtual bool networkingEnabled();

		/**
		* @param enable true to enable networking, false to disable it.
		* @return true on success, false on failure.
		*/
		virtual bool enableNetworking(bool enable);

		/**
		* @return true if WiFi hardware is enabled, false if not.
		*/
		virtual bool wiFiHardwareEnabled();

		/**
		* @param enable true to enable WiFi, false to disable it.
		* @return true on success, false on failure.
		*/
		virtual bool enableWiFi(bool enable);

		/**
		* @return true if WiFi is enabled, false if not.
		*/
		virtual bool wiFiEnabled();

		/**
		* @return a list of UUIDs of the default network devices, or an empty list if no such devices exist.
		* The default network devices are normally the devices holding the highest priority default route.
		*/
		virtual TQStringList defaultNetworkDevices();

	signals:
		/**
		* Emitted whenever the state of the system's connection changes
		* If previous state data was unavailable, @param previousState will contain TDENetworkConnectionStatus::Invalid
		*/
		void networkConnectionStateChanged(TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags newState, TDENetworkGlobalManagerFlags::TDENetworkGlobalManagerFlags previousState);

		/**
		* Emitted whenever the state of a device changes
		* If previous state data was unavailable, @param previousState will contain TDENetworkConnectionStatus::Invalid
		* If the global connection state has changed, @param hwAddress will be empty, otherwise it will contain the MAC address
		* of the networking hardware that has changed state.
		*/
		void networkDeviceStateChanged(TDENetworkConnectionStatus::TDENetworkConnectionStatus newState, TDENetworkConnectionStatus::TDENetworkConnectionStatus previousState, TQString deviceNode);

		/**
		* Emitted whenever the status of a wireless access point changes
		* The event type that caused the signal is available in @param event
		*/
		void accessPointStatusChanged(TDEMACAddress BSSID, TDENetworkAPEventType::TDENetworkAPEventType event);

		/**
		* Emitted whenever a VPN-related event occurs
		* The event type that caused the signal is available in @param event
		* @param message contains additional information if available
		*/
		void vpnEvent(TDENetworkVPNEventType::TDENetworkVPNEventType event, TQString message);

		/**
		* Emitted whenever a global network management event occurs
		* The event type that caused the signal is available in @param event
		*/
		void networkManagementEvent(TDENetworkGlobalEventType::TDENetworkGlobalEventType event);

	public:
		/**
		* @return a TDENetworkConnectionList object containing a list of all
		* possible connections this connection manager is aware of, regardless
		* of current state or availability.
		*
		* loadConnectionInformation() should be called at least once before calling
		* this method, in order to update internal connection information from the
		* configuration backend.
		*
		* Note that the returned list is internally managed and must not be deleted!
		* Also note that pointers in the list may become invalid on subsequent calls to
		* loadConnectionInformation(), saveConnection(), deleteConnection(), or connections().
		*/
		virtual TDENetworkConnectionList* connections();

		/**
		* @return a pointer to a TDENetworkConnection object with the specified @param uuid,
		* or a NULL pointer if no such connection exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		TDENetworkConnection* findConnectionByUUID(TQString uuid);

		/**
		* @return a pointer to a TDENetworkDevice object with the specified @param uuid,
		* or a NULL pointer if no such device exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		TDENetworkDevice* findDeviceByUUID(TQString uuid);

		/**
		* @return a pointer to a TDENetworkWiFiAPInfo object with the specified @param bssid,
		* or a NULL pointer if no such access point exists.
		*
		* Note that the returned object is internally managed and must not be deleted!
		*/
		TDENetworkWiFiAPInfo* findAccessPointByBSSID(TDEMACAddress bssid);

	private:
		TDENetworkConnectionManager* m_internalConnectionManager;
};

#endif // _TDENETWORKCONNECTIONS_H
