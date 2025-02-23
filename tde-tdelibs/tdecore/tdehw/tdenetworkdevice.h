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

#ifndef _TDENETWORKDEVICE_H
#define _TDENETWORKDEVICE_H

#include "tdegenericdevice.h"

class TDENetworkConnectionManager;

class TDECORE_EXPORT TDENetworkDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDENetworkDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDENetworkDevice();

		/**
		* @return a TQString containing the network device's MAC address
		*/
		TQString macAddress();

		/**
		* @return a TQString containing the network device's operational state
		*/
		TQString state();

		/**
		* @return true if carrier is present, false if not
		*/
		bool carrierPresent();

		/**
		* @return true if device is dormant, false if not
		*/
		bool dormant();

		/**
		* @return a TQString containing the network device's IPv4 address
		*/
		TQString ipV4Address();

		/**
		* @return a TQString containing the network device's IPv6 address
		*/
		TQString ipV6Address();

		/**
		* @return a TQString containing the network device's IPv4 netmask
		*/
		TQString ipV4Netmask();

		/**
		* @return a TQString containing the network device's IPv6 netmask
		*/
		TQString ipV6Netmask();

		/**
		* @return a TQString containing the network device's IPv4 broadcast
		*/
		TQString ipV4Broadcast();

		/**
		* @return a TQString containing the network device's IPv6 broadcast
		*/
		TQString ipV6Broadcast();

		/**
		* @return a TQString containing the network device's IPv4 destination
		*/
		TQString ipV4Destination();

		/**
		* @return a TQString containing the network device's IPv6 destination
		*/
		TQString ipV6Destination();

		/**
		* @return a double with the number of received bytes, if available
		*/
		double rxBytes();

		/**
		* @return a double with the number of transmitted bytes, if available
		*/
		double txBytes();

		/**
		* @return a double with the number of received packets, if available
		*/
		double rxPackets();

		/**
		* @return a double with the number of transmitted packets, if available
		*/
		double txPackets();

		/**
		* @return a pointer to a TDENetworkConnectionManager object, if available
		*/
		TDENetworkConnectionManager* connectionManager();

	protected:
		/**
		* @param ma a TQString containing the network device's MAC address
		* @internal
		*/
		void internalSetMacAddress(TQString ma);

		/**
		* @param st a TQString containing the network device's operational state
		* @internal
		*/
		void internalSetState(TQString st);

		/**
		* @param true if carrier is present, false if not
		* @internal
		*/
		void internalSetCarrierPresent(bool cp);

		/**
		* @param true if device is dormant, false if not
		* @internal
		*/
		void internalSetDormant(bool dm);

		/**
		* @param ad a TQString containing the network device's IPv4 address
		* @internal
		*/
		void internalSetIpV4Address(TQString ad);

		/**
		* @param ad a TQString containing the network device's IPv6 address
		* @internal
		*/
		void internalSetIpV6Address(TQString ad);

		/**
		* @param nm a TQString containing the network device's IPv4 netmask
		* @internal
		*/
		void internalSetIpV4Netmask(TQString nm);

		/**
		* @param nm a TQString containing the network device's IPv6 netmask
		* @internal
		*/
		void internalSetIpV6Netmask(TQString nm);

		/**
		* @param br a TQString containing the network device's IPv4 broadcast
		* @internal
		*/
		void internalSetIpV4Broadcast(TQString br);

		/**
		* @param br a TQString containing the network device's IPv6 broadcast
		* @internal
		*/
		void internalSetIpV6Broadcast(TQString br);

		/**
		* @param ds a TQString containing the network device's IPv4 destination
		* @internal
		*/
		void internalSetIpV4Destination(TQString ds);

		/**
		* @param ds a TQString containing the network device's IPv6 destination
		* @internal
		*/
		void internalSetIpV6Destination(TQString ds);

		/**
		* @param rx a double with the number of received bytes, if available
		* @internal
		*/
		void internalSetRxBytes(double rx);

		/**
		* @param tx a double with the number of transmitted bytes, if available
		* @internal
		*/
		void internalSetTxBytes(double tx);

		/**
		* @param rx a double with the number of received packets, if available
		* @internal
		*/
		void internalSetRxPackets(double rx);

		/**
		* @param tx a double with the number of transmitted packets, if available
		* @internal
		*/
		void internalSetTxPackets(double tx);

		/**
		* @param mgr a pointer to a TDENetworkConnectionManager object, if available
		*/
		void internalSetConnectionManager(TDENetworkConnectionManager* mgr);

	private:
		TQString m_macAddress;
		TQString m_state;
		bool m_carrier;
		bool m_dormant;
		TQString m_ipV4Address;
		TQString m_ipV6Address;
		TQString m_ipV4Netmask;
		TQString m_ipV6Netmask;
		TQString m_ipV4Broadcast;
		TQString m_ipV6Broadcast;
		TQString m_ipV4Destination;
		TQString m_ipV6Destination;
		double m_rxbytes;
		double m_txbytes;
		double m_rxpackets;
		double m_txpackets;
		TDENetworkConnectionManager* m_connectionManager;

	friend class TDEHardwareDevices;
};

#endif // _TDENETWORKDEVICE_H
