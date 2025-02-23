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

#include "tdenetworkdevice.h"

// Network connection manager
#include "tdenetworkconnections.h"

#include "config.h"

#ifdef WITH_NETWORK_MANAGER_BACKEND
	#include "network-manager.h"
#endif // WITH_NETWORK_MANAGER_BACKEND


TDENetworkDevice::TDENetworkDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
	m_rxbytes = -1;
	m_txbytes = -1;
	m_rxpackets = -1;
	m_txpackets = -1;
	m_connectionManager = NULL;
}

TDENetworkDevice::~TDENetworkDevice() {
	if (m_connectionManager) {
		delete m_connectionManager;
	}
}

TQString TDENetworkDevice::macAddress() {
	return m_macAddress;
}

void TDENetworkDevice::internalSetMacAddress(TQString ma) {
	m_macAddress = ma;
}

TQString TDENetworkDevice::state() {
	return m_state;
}

void TDENetworkDevice::internalSetState(TQString st) {
	m_state = st;
}

bool TDENetworkDevice::carrierPresent() {
	return m_carrier;
}

void TDENetworkDevice::internalSetCarrierPresent(bool cp) {
	m_carrier = cp;
}

bool TDENetworkDevice::dormant() {
	return m_dormant;
}

void TDENetworkDevice::internalSetDormant(bool dm) {
	m_dormant = dm;
}

TQString TDENetworkDevice::ipV4Address() {
	return m_ipV4Address;
}

void TDENetworkDevice::internalSetIpV4Address(TQString ad) {
	m_ipV4Address = ad;
}

TQString TDENetworkDevice::ipV6Address() {
	return m_ipV6Address;
}

void TDENetworkDevice::internalSetIpV6Address(TQString ad) {
	m_ipV6Address = ad;
}

TQString TDENetworkDevice::ipV4Netmask() {
	return m_ipV4Netmask;
}

void TDENetworkDevice::internalSetIpV4Netmask(TQString nm) {
	m_ipV4Netmask = nm;
}

TQString TDENetworkDevice::ipV6Netmask() {
	return m_ipV6Netmask;
}

void TDENetworkDevice::internalSetIpV6Netmask(TQString nm) {
	m_ipV6Netmask = nm;
}

TQString TDENetworkDevice::ipV4Broadcast() {
	return m_ipV4Broadcast;
}

void TDENetworkDevice::internalSetIpV4Broadcast(TQString br) {
	m_ipV4Broadcast = br;
}

TQString TDENetworkDevice::ipV6Broadcast() {
	return m_ipV6Broadcast;
}

void TDENetworkDevice::internalSetIpV6Broadcast(TQString br) {
	m_ipV6Broadcast = br;
}

TQString TDENetworkDevice::ipV4Destination() {
	return m_ipV4Destination;
}

void TDENetworkDevice::internalSetIpV4Destination(TQString ds) {
	m_ipV4Destination = ds;
}

TQString TDENetworkDevice::ipV6Destination() {
	return m_ipV6Destination;
}

void TDENetworkDevice::internalSetIpV6Destination(TQString ds) {
	m_ipV6Destination = ds;
}

double TDENetworkDevice::rxBytes() {
	return m_rxbytes;
}

void TDENetworkDevice::internalSetRxBytes(double rx) {
	m_rxbytes = rx;
}

double TDENetworkDevice::txBytes() {
	return m_txbytes;
}

void TDENetworkDevice::internalSetTxBytes(double tx) {
	m_txbytes = tx;
}

double TDENetworkDevice::rxPackets() {
	return m_rxpackets;
}

void TDENetworkDevice::internalSetRxPackets(double rx) {
	m_rxpackets = rx;
}

double TDENetworkDevice::txPackets() {
	return m_txpackets;
}

void TDENetworkDevice::internalSetTxPackets(double tx) {
	m_txpackets = tx;
}

TDENetworkConnectionManager* TDENetworkDevice::connectionManager() {
#ifdef WITH_NETWORK_MANAGER_BACKEND
	if (!m_connectionManager) {
		m_connectionManager = new TDENetworkConnectionManager_BackendNM(this);
	}
#endif // WITH_NETWORK_MANAGER_BACKEND

	return m_connectionManager;
}

void TDENetworkDevice::internalSetConnectionManager(TDENetworkConnectionManager* mgr) {
	m_connectionManager = mgr;
}

#include "tdenetworkdevice.moc"
