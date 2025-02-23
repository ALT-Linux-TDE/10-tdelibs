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

#include "tdebacklightdevice.h"

#include <unistd.h>

#include <tqfile.h>

#include "config.h"

// uPower
#if defined(WITH_TDEHWLIB_DAEMONS)
	#include <tqdbusdata.h>
	#include <tqdbusmessage.h>
	#include <tqdbusproxy.h>
	#include <tqdbusvariant.h>
	#include <tqdbusconnection.h>
#endif // defined(WITH_TDEHWLIB_DAEMONS)

TDEBacklightDevice::TDEBacklightDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDEBacklightDevice::~TDEBacklightDevice() {
}

TDEDisplayPowerLevel::TDEDisplayPowerLevel TDEBacklightDevice::powerLevel() {
	return m_powerLevel;
}

void TDEBacklightDevice::internalSetPowerLevel(TDEDisplayPowerLevel::TDEDisplayPowerLevel pl) {
	m_powerLevel = pl;
}

void TDEBacklightDevice::internalSetMaximumRawBrightness(int br) {
	m_maximumBrightness = br;
}

void TDEBacklightDevice::internalSetCurrentRawBrightness(int br) {
	m_currentBrightness = br;
}

int TDEBacklightDevice::brightnessSteps() {
	return m_maximumBrightness + 1;
}

double TDEBacklightDevice::brightnessPercent() {
	return (((m_currentBrightness*1.0)/m_maximumBrightness)*100.0);
}

bool TDEBacklightDevice::canSetBrightness() {
	TQString brightnessnode = systemPath() + "/brightness";
	int rval = access (brightnessnode.ascii(), W_OK);
	if (rval == 0) {
		return true;
	}

#ifdef WITH_TDEHWLIB_DAEMONS
	{
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy hardwareControl("org.trinitydesktop.hardwarecontrol", "/org/trinitydesktop/hardwarecontrol", "org.trinitydesktop.hardwarecontrol.Brightness", dbusConn);
			if (hardwareControl.canSend()) {
				// can set brightness?
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(brightnessnode);
				TQT_DBusMessage reply = hardwareControl.sendWithReply("CanSetBrightness", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					return reply[0].toBool();
				}
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

	return false;
}

int TDEBacklightDevice::rawBrightness() {
	return m_currentBrightness;
}

void TDEBacklightDevice::setRawBrightness(int br) {
	bool setRawBrightnessDone = false;

	TQString brightnessnode = systemPath() + "/brightness";
	TQString brightnessCommand = TQString("%1").arg(br);
	TQFile file( brightnessnode );
	if ( file.open( IO_WriteOnly ) ) {
		TQTextStream stream( &file );
		stream << brightnessCommand;
		file.close();
		setRawBrightnessDone = true;
	}

#ifdef WITH_TDEHWLIB_DAEMONS
	if ( !setRawBrightnessDone ) {
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy hardwareControl("org.trinitydesktop.hardwarecontrol", "/org/trinitydesktop/hardwarecontrol", "org.trinitydesktop.hardwarecontrol.Brightness", dbusConn);
			if (hardwareControl.canSend()) {
				// set brightness
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(brightnessnode) << TQT_DBusData::fromString(brightnessCommand);
				TQT_DBusMessage reply = hardwareControl.sendWithReply("SetBrightness", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage) {
					setRawBrightnessDone = true;
				}
			}
		}
	}
#endif // WITH_TDEHWLIB_DAEMONS

}

#include "tdebacklightdevice.moc"
