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

#include "tdecpudevice.h"

#include <unistd.h>

#include <tqfile.h>

#include "tdeglobal.h"

#include "tdehardwaredevices.h"

#include "config.h"

// uPower
#if defined(WITH_TDEHWLIB_DAEMONS)
 	#include <tqdbusdata.h>
 	#include <tqdbusmessage.h>
 	#include <tqdbusproxy.h>
 	#include <tqdbusvariant.h>
 	#include <tqdbusconnection.h>
#endif


TDECPUDevice::TDECPUDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
	m_frequency = -1;
	m_minfrequency = -1;
	m_maxfrequency = -1;
	m_corenumber = -1;
	m_transitionlatency = -1;
}

TDECPUDevice::~TDECPUDevice() {
}

double TDECPUDevice::frequency() {
	return m_frequency;
}

void TDECPUDevice::internalSetFrequency(double fr) {
	m_frequency = fr;
}

double TDECPUDevice::minFrequency() {
	return m_minfrequency;
}

void TDECPUDevice::internalSetMinFrequency(double fr) {
	m_minfrequency = fr;
}

double TDECPUDevice::maxFrequency() {
	return m_maxfrequency;
}

void TDECPUDevice::internalSetMaxFrequency(double fr) {
	m_maxfrequency = fr;
}

double TDECPUDevice::transitionLatency() {
	return m_transitionlatency;
}

void TDECPUDevice::internalSetTransitionLatency(double tl) {
	m_transitionlatency = tl;
}

TQString TDECPUDevice::governor() {
	return m_governor;
}

void TDECPUDevice::internalSetGovernor(TQString gr) {
	m_governor = gr;
}

TQString TDECPUDevice::scalingDriver() {
	return m_scalingdriver;
}

void TDECPUDevice::internalSetScalingDriver(TQString dr) {
	m_scalingdriver = dr;
}

TQStringList TDECPUDevice::dependentProcessors() {
	return m_tiedprocs;
}

void TDECPUDevice::internalSetDependentProcessors(TQStringList dp) {
	m_tiedprocs = dp;
}

TQStringList TDECPUDevice::availableFrequencies() {
	return m_frequencies;
}

void TDECPUDevice::internalSetAvailableFrequencies(TQStringList af) {
	m_frequencies = af;
}

TQStringList TDECPUDevice::availableGovernors() {
	return m_governers;
}

void TDECPUDevice::internalSetAvailableGovernors(TQStringList gp) {
	m_governers = gp;
}

void TDECPUDevice::internalSetCoreNumber(int cn) {
	m_corenumber = cn;
}

bool TDECPUDevice::canSetGovernor() {
	TQString governornode = systemPath() + "/cpufreq/scaling_governor";
	int rval = access (governornode.ascii(), W_OK);
	if (rval == 0) {
		return true;
	}

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy hardwareControl("org.trinitydesktop.hardwarecontrol", "/org/trinitydesktop/hardwarecontrol", "org.trinitydesktop.hardwarecontrol.CPUGovernor", dbusConn);
			if (hardwareControl.canSend()) {
				// can set CPU governor?
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromInt32(coreNumber());
				TQT_DBusMessage reply = hardwareControl.sendWithReply("CanSetCPUGovernor", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toBool();
				}
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

	return false;
}

void TDECPUDevice::setGovernor(TQString gv) {
	bool setGovernorDone = false;

	TQString governornode = systemPath() + "/cpufreq/scaling_governor";
	TQFile file( governornode );
	if ( file.open( IO_WriteOnly ) ) {
		TQTextStream stream( &file );
		stream << gv.lower();
		file.close();
		setGovernorDone = true;
	}

#ifdef WITH_TDEHWLIB_DAEMONS
	if ( !setGovernorDone ) {
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy hardwareControl("org.trinitydesktop.hardwarecontrol", "/org/trinitydesktop/hardwarecontrol", "org.trinitydesktop.hardwarecontrol.CPUGovernor", dbusConn);
			if (hardwareControl.canSend()) {
				// set CPU governor
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromInt32(coreNumber()) << TQT_DBusData::fromString(gv.lower());
				TQT_DBusMessage reply = hardwareControl.sendWithReply("SetCPUGovernor", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage) {
					setGovernorDone = true;
				}
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

	// Force update of the device information object
	if ( setGovernorDone ) {
		TDEGlobal::hardwareDevices()->processModifiedCPUs();
	}
}

bool TDECPUDevice::canSetMaximumScalingFrequency() {
	TQString freqnode = systemPath() + "/cpufreq/scaling_max_freq";
	int rval = access (freqnode.ascii(), W_OK);
	if (rval == 0) {
		return true;
	}
	else {
		return false;
	}
}

void TDECPUDevice::setMaximumScalingFrequency(double fr) {
	TQString freqnode = systemPath() + "/cpufreq/scaling_max_freq";
	TQFile file( freqnode );
	if ( file.open( IO_WriteOnly ) ) {
		TQTextStream stream( &file );
		stream << TQString("%1").arg(fr*1000000.0, 0, 'f', 0);
		file.close();
	}

	// Force update of the device information object
	TDEGlobal::hardwareDevices()->processModifiedCPUs();
}

int TDECPUDevice::coreNumber() {
	return m_corenumber;
}

#include "tdecpudevice.moc"
