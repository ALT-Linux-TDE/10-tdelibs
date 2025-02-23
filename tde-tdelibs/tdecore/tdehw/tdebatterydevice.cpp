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

#include "tdebatterydevice.h"

#include "config.h"

TDEBatteryDevice::TDEBatteryDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) :
 m_currentVoltage(0), m_minimumVoltage(0), m_maximumVoltage(0), m_maximumDesignVoltage(0),
 m_alarmEnergy(0), m_currentEnergy(0), m_maximumEnergy(0), m_maximumDesignEnergy(0),
 m_dischargeRate(0), m_timeRemaining(0),
 m_technology(TQString::null),
 m_status(TDEBatteryStatus::Unknown),
 m_installed(0),
 TDEGenericDevice(dt, dn) {
}

TDEBatteryDevice::~TDEBatteryDevice() {
}

double TDEBatteryDevice::voltage() {
	return m_currentVoltage;
}

void TDEBatteryDevice::internalSetVoltage(double vt) {
	m_currentVoltage = vt;
}

double TDEBatteryDevice::maximumVoltage() {
	return m_maximumVoltage;
}

void TDEBatteryDevice::internalSetMaximumVoltage(double vt) {
	m_maximumVoltage = vt;
}

double TDEBatteryDevice::minimumVoltage() {
	return m_minimumVoltage;
}

void TDEBatteryDevice::internalSetMinimumVoltage(double vt) {
	m_minimumVoltage = vt;
}

double TDEBatteryDevice::maximumDesignVoltage() {
	return m_maximumDesignVoltage;
}

void TDEBatteryDevice::internalSetMaximumDesignVoltage(double vt) {
	m_maximumDesignVoltage = vt;
}

double TDEBatteryDevice::energy() {
	return m_currentEnergy;
}

void TDEBatteryDevice::internalSetEnergy(double vt) {
	m_currentEnergy = vt;
}

double TDEBatteryDevice::alarmEnergy() {
	return m_alarmEnergy;
}

void TDEBatteryDevice::internalSetAlarmEnergy(double vt) {
	m_alarmEnergy = vt;
}

double TDEBatteryDevice::maximumEnergy() {
	return m_maximumEnergy;
}

void TDEBatteryDevice::internalSetMaximumEnergy(double vt) {
	m_maximumEnergy = vt;
}

double TDEBatteryDevice::maximumDesignEnergy() {
	return m_maximumDesignEnergy;
}

void TDEBatteryDevice::internalSetMaximumDesignEnergy(double vt) {
	m_maximumDesignEnergy = vt;
}

double TDEBatteryDevice::dischargeRate() {
	return m_dischargeRate;
}

void TDEBatteryDevice::internalSetDischargeRate(double vt) {
	m_dischargeRate = vt;
}

double TDEBatteryDevice::timeRemaining() {
	return m_timeRemaining;
}

void TDEBatteryDevice::internalSetTimeRemaining(double tr) {
	m_timeRemaining = tr;
}

TQString TDEBatteryDevice::technology() {
	return m_technology;
}

void TDEBatteryDevice::internalSetTechnology(TQString tc) {
	m_technology = tc;
}

TDEBatteryStatus::TDEBatteryStatus TDEBatteryDevice::status() {
	return m_status;
}

void TDEBatteryDevice::internalSetStatus(TQString tc) {
	tc = tc.lower();

	if (tc == "charging") {
		m_status = TDEBatteryStatus::Charging;
	}
	else if (tc == "discharging") {
		m_status = TDEBatteryStatus::Discharging;
	}
	else if (tc == "full") {
		m_status = TDEBatteryStatus::Full;
	}
	else {
		m_status = TDEBatteryStatus::Unknown;
	}
}

bool TDEBatteryDevice::installed() {
	return m_installed;
}

void TDEBatteryDevice::internalSetInstalled(bool tc) {
	m_installed = tc;
}

double TDEBatteryDevice::chargePercent() {
	return (m_currentEnergy/m_maximumEnergy)*100.0;
}

#include "tdebatterydevice.moc"
