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

#include "tdemonitordevice.h"

#include "config.h"

TDEMonitorDevice::TDEMonitorDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDEMonitorDevice::~TDEMonitorDevice() {
}

bool TDEMonitorDevice::connected() {
	return m_connected;
}

void TDEMonitorDevice::internalSetConnected(bool cn) {
	m_connected = cn;
}

bool TDEMonitorDevice::enabled() {
	return m_enabled;
}

void TDEMonitorDevice::internalSetEnabled(bool en) {
	m_enabled = en;
}

TQByteArray TDEMonitorDevice::edid() {
	return m_edid;
}

void TDEMonitorDevice::internalSetEdid(TQByteArray ed) {
	m_edid = ed;
}

TDEResolutionList TDEMonitorDevice::resolutions() {
	return m_resolutions;
}

void TDEMonitorDevice::internalSetResolutions(TDEResolutionList rs) {
	m_resolutions = rs;
}

TQString TDEMonitorDevice::portType() {
	return m_portType;
}

void TDEMonitorDevice::internalSetPortType(TQString pt) {
	m_portType = pt;
}

TDEDisplayPowerLevel::TDEDisplayPowerLevel TDEMonitorDevice::powerLevel() {
	return m_powerLevel;
}

void TDEMonitorDevice::internalSetPowerLevel(TDEDisplayPowerLevel::TDEDisplayPowerLevel pl) {
	m_powerLevel = pl;
}

#include "tdemonitordevice.moc"
