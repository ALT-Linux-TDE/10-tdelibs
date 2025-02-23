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

#include "tdeeventdevice.h"

#include <unistd.h>
#include <linux/input.h>

#include <tqsocketnotifier.h>
#include <tqtimer.h>

#include "tdelocale.h"

#include "tdehardwaredevices.h"

#include "config.h"

#define BITS_PER_LONG (sizeof(long) * 8)
#define NUM_BITS(x) ((((x) - 1) / BITS_PER_LONG) + 1)
#define OFF(x)  ((x) % BITS_PER_LONG)
#define BIT(x)  (1UL << OFF(x))
#define LONG(x) ((x) / BITS_PER_LONG)
#define BIT_IS_SET(array, bit)  ((array[LONG(bit)] >> OFF(bit)) & 1)

#if defined(WITH_TDEHWLIB_DAEMONS)
#include <tqdbusconnection.h>
#include <tqdbusproxy.h>
#include <tqdbusmessage.h>
#include <tqdbusvariant.h>
#include <tqdbusdata.h>
#include <tqdbusdatalist.h>
#endif

TDEEventDevice::TDEEventDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
	m_fd = -1;
	m_watchTimer = 0;
	m_monitorActive = false;
}

TDEEventDevice::~TDEEventDevice() {
	if (m_fd >= 0) {
		close(m_fd);
	}
	if (m_watchTimer) {
		m_watchTimer->stop();
		delete m_watchTimer;
	}
}

TDEEventDeviceType::TDEEventDeviceType TDEEventDevice::eventType() {
	return m_eventType;
}

void TDEEventDevice::internalSetEventType(TDEEventDeviceType::TDEEventDeviceType et) {
	m_eventType = et;
}

TDESwitchType::TDESwitchType TDEEventDevice::providedSwitches() {
	if (!m_monitorActive) {
		internalReadProvidedSwitches();
	}
	return m_providedSwitches;
}

void TDEEventDevice::internalReadProvidedSwitches() {
	unsigned long switches[NUM_BITS(EV_CNT)];
	int r = 0;

	// Figure out which switch types are supported, if any
	TDESwitchType::TDESwitchType supportedSwitches = TDESwitchType::Null;
	if (m_fd >= 0) {
		r = ioctl(m_fd, EVIOCGBIT(EV_SW, EV_CNT), switches);
	}
#ifdef WITH_TDEHWLIB_DAEMONS
	if( r < 1 ) {
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy switchesProxy("org.trinitydesktop.hardwarecontrol",
				"/org/trinitydesktop/hardwarecontrol",
				"org.trinitydesktop.hardwarecontrol.InputEvents",
				dbusConn);
			if (switchesProxy.canSend()) {
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(deviceNode().ascii());
				TQT_DBusMessage reply = switchesProxy.sendWithReply("GetProvidedSwitches", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					TQValueList<TQ_UINT32> list = reply[0].toList().toUInt32List();
					TQValueList<TQ_UINT32>::const_iterator it = list.begin();
					for (r = 0; it != list.end(); ++it, r++) {
						switches[r] = (*it);
					}
				}
			}
		}
	}
#endif
	if (r > 0) {
		if (BIT_IS_SET(switches, SW_LID)) {
			supportedSwitches = supportedSwitches | TDESwitchType::Lid;
		}
		if (BIT_IS_SET(switches, SW_TABLET_MODE)) {
			supportedSwitches = supportedSwitches | TDESwitchType::TabletMode;
		}
		if (BIT_IS_SET(switches, SW_RFKILL_ALL)) {
			supportedSwitches = supportedSwitches | TDESwitchType::RFKill;
		}
#		if SW_RADIO != SW_RFKILL_ALL // SW_RADIO is a deprecated name for SW_RFKILL_ALL
		if (BIT_IS_SET(switches, SW_RADIO)) {
			supportedSwitches = supportedSwitches | TDESwitchType::Radio;
		}
#		endif
		if (BIT_IS_SET(switches, SW_MICROPHONE_INSERT)) {
			supportedSwitches = supportedSwitches | TDESwitchType::MicrophoneInsert;
		}
		if (BIT_IS_SET(switches, SW_DOCK)) {
			supportedSwitches = supportedSwitches | TDESwitchType::Dock;
		}
		if (BIT_IS_SET(switches, SW_LINEOUT_INSERT)) {
			supportedSwitches = supportedSwitches | TDESwitchType::LineOutInsert;
		}
		if (BIT_IS_SET(switches, SW_JACK_PHYSICAL_INSERT)) {
			supportedSwitches = supportedSwitches | TDESwitchType::JackPhysicalInsert;
		}
		if (BIT_IS_SET(switches, SW_VIDEOOUT_INSERT)) {
			supportedSwitches = supportedSwitches | TDESwitchType::VideoOutInsert;
		}
#		ifdef SW_CAMERA_LENS_COVER
		if (BIT_IS_SET(switches, SW_CAMERA_LENS_COVER)) {
			supportedSwitches = supportedSwitches | TDESwitchType::CameraLensCover;
		}
#		endif
#		ifdef SW_KEYPAD_SLIDE
		if (BIT_IS_SET(switches, SW_KEYPAD_SLIDE)) {
			supportedSwitches = supportedSwitches | TDESwitchType::KeypadSlide;
		}
#		endif
#		ifdef SW_FRONT_PROXIMITY
		if (BIT_IS_SET(switches, SW_FRONT_PROXIMITY)) {
			supportedSwitches = supportedSwitches | TDESwitchType::FrontProximity;
		}
#		endif
#		ifdef SW_ROTATE_LOCK
		if (BIT_IS_SET(switches, SW_ROTATE_LOCK)) {
			supportedSwitches = supportedSwitches | TDESwitchType::RotateLock;
		}
#		endif
#		ifdef SW_LINEIN_INSERT
		if (BIT_IS_SET(switches, SW_LINEIN_INSERT)) {
			supportedSwitches = supportedSwitches | TDESwitchType::LineInInsert;
		}
#		endif
		// Keep in sync with ACPI Event/Input identification routines above
		if (systemPath().contains("PNP0C0D")) {
			supportedSwitches = supportedSwitches | TDESwitchType::Lid;
		}
		if (systemPath().contains("PNP0C0E") || systemPath().contains("/LNXSLPBN")) {
			supportedSwitches = supportedSwitches | TDESwitchType::SleepButton;
		}
		if (systemPath().contains("PNP0C0C") || systemPath().contains("/LNXPWRBN")) {
			supportedSwitches = supportedSwitches | TDESwitchType::PowerButton;
		}
	}
	m_providedSwitches = supportedSwitches;
}

void TDEEventDevice::internalSetProvidedSwitches(TDESwitchType::TDESwitchType sl) {
	m_providedSwitches = sl;
}

TDESwitchType::TDESwitchType TDEEventDevice::activeSwitches() {
	if (!m_monitorActive) {
		internalReadActiveSwitches();
	}
	return m_switchActive;
}

void TDEEventDevice::internalReadActiveSwitches() {
	unsigned long switches[NUM_BITS(EV_CNT)];
	int r = 0;

	TDESwitchType::TDESwitchType activeSwitches = TDESwitchType::Null;
	if (m_fd >= 0) {
		r = ioctl(m_fd, EVIOCGSW(sizeof(switches)), switches);
	}
#ifdef WITH_TDEHWLIB_DAEMONS
	if( r < 1 ) {
		TQT_DBusConnection dbusConn = TQT_DBusConnection::addConnection(TQT_DBusConnection::SystemBus);
		if (dbusConn.isConnected()) {
			TQT_DBusProxy switchesProxy("org.trinitydesktop.hardwarecontrol",
				"/org/trinitydesktop/hardwarecontrol",
				"org.trinitydesktop.hardwarecontrol.InputEvents",
				dbusConn);
			if (switchesProxy.canSend()) {
				TQValueList<TQT_DBusData> params;
				params << TQT_DBusData::fromString(deviceNode().ascii());
				TQT_DBusMessage reply = switchesProxy.sendWithReply("GetActiveSwitches", params);
				if (reply.type() == TQT_DBusMessage::ReplyMessage && reply.count() == 1) {
					TQValueList<TQ_UINT32> list = reply[0].toList().toUInt32List();
					TQValueList<TQ_UINT32>::const_iterator it = list.begin();
					for (r = 0; it != list.end(); ++it, r++) {
						switches[r] = (*it);
					}
				}
			}
		}
	}
#endif
	if (r > 0) {
		if (BIT_IS_SET(switches, SW_LID)) {
			activeSwitches = activeSwitches | TDESwitchType::Lid;
		}
		if (BIT_IS_SET(switches, SW_TABLET_MODE)) {
			activeSwitches = activeSwitches | TDESwitchType::TabletMode;
		}
		if (BIT_IS_SET(switches, SW_RFKILL_ALL)) {
			activeSwitches = activeSwitches | TDESwitchType::RFKill;
		}
#		if SW_RADIO != SW_RFKILL_ALL // SW_RADIO is a deprecated name for SW_RFKILL_ALL
		if (BIT_IS_SET(switches, SW_RADIO)) {
			activeSwitches = activeSwitches | TDESwitchType::Radio;
		}
#		endif
		if (BIT_IS_SET(switches, SW_MICROPHONE_INSERT)) {
			activeSwitches = activeSwitches | TDESwitchType::MicrophoneInsert;
		}
		if (BIT_IS_SET(switches, SW_DOCK)) {
			activeSwitches = activeSwitches | TDESwitchType::Dock;
		}
		if (BIT_IS_SET(switches, SW_LINEOUT_INSERT)) {
			activeSwitches = activeSwitches | TDESwitchType::LineOutInsert;
		}
		if (BIT_IS_SET(switches, SW_JACK_PHYSICAL_INSERT)) {
			activeSwitches = activeSwitches | TDESwitchType::JackPhysicalInsert;
		}
		if (BIT_IS_SET(switches, SW_VIDEOOUT_INSERT)) {
			activeSwitches = activeSwitches | TDESwitchType::VideoOutInsert;
		}
#		ifdef SW_CAMERA_LENS_COVER
		if (BIT_IS_SET(switches, SW_CAMERA_LENS_COVER)) {
			activeSwitches = activeSwitches | TDESwitchType::CameraLensCover;
		}
#		endif
#		ifdef SW_KEYPAD_SLIDE
		if (BIT_IS_SET(switches, SW_KEYPAD_SLIDE)) {
			activeSwitches = activeSwitches | TDESwitchType::KeypadSlide;
		}
#		endif
#		ifdef SW_FRONT_PROXIMITY
		if (BIT_IS_SET(switches, SW_FRONT_PROXIMITY)) {
			activeSwitches = activeSwitches | TDESwitchType::FrontProximity;
		}
#		endif
#		ifdef SW_ROTATE_LOCK
		if (BIT_IS_SET(switches, SW_ROTATE_LOCK)) {
			activeSwitches = activeSwitches | TDESwitchType::RotateLock;
		}
#		endif
#		ifdef SW_LINEIN_INSERT
		if (BIT_IS_SET(switches, SW_LINEIN_INSERT)) {
			activeSwitches = activeSwitches | TDESwitchType::LineInInsert;
		}
#		endif
	}
	m_switchActive = activeSwitches;
}

void TDEEventDevice::internalSetActiveSwitches(TDESwitchType::TDESwitchType sl) {
	m_switchActive = sl;
}

// Keep this in sync with the TDESwitchType definition in the header
TQStringList TDEEventDevice::friendlySwitchList(TDESwitchType::TDESwitchType switches) {
	TQStringList ret;

	if (switches & TDESwitchType::Lid) {
		ret.append(i18n("Lid Switch"));
	}
	if (switches & TDESwitchType::TabletMode) {
		ret.append(i18n("Tablet Mode"));
	}
	if (switches & TDESwitchType::HeadphoneInsert) {
		ret.append(i18n("Headphone Inserted"));
	}
	if (switches & TDESwitchType::RFKill) {
		ret.append(i18n("Radio Frequency Device Kill Switch"));
	}
	if (switches & TDESwitchType::Radio) {
		ret.append(i18n("Enable Radio"));
	}
	if (switches & TDESwitchType::MicrophoneInsert) {
		ret.append(i18n("Microphone Inserted"));
	}
	if (switches & TDESwitchType::Dock) {
		ret.append(i18n("Docked"));
	}
	if (switches & TDESwitchType::LineOutInsert) {
		ret.append(i18n("Line Out Inserted"));
	}
	if (switches & TDESwitchType::JackPhysicalInsert) {
		ret.append(i18n("Physical Jack Inserted"));
	}
	if (switches & TDESwitchType::VideoOutInsert) {
		ret.append(i18n("Video Out Inserted"));
	}
	if (switches & TDESwitchType::CameraLensCover) {
		ret.append(i18n("Camera Lens Cover"));
	}
	if (switches & TDESwitchType::KeypadSlide) {
		ret.append(i18n("Keypad Slide"));
	}
	if (switches & TDESwitchType::FrontProximity) {
		ret.append(i18n("Front Proximity"));
	}
	if (switches & TDESwitchType::RotateLock) {
		ret.append(i18n("Rotate Lock"));
	}
	if (switches & TDESwitchType::LineInInsert) {
		ret.append(i18n("Line In Inserted"));
	}
	if (switches & TDESwitchType::PowerButton) {
		ret.append(i18n("Power Button"));
	}
	if (switches & TDESwitchType::SleepButton) {
		ret.append(i18n("Sleep Button"));
	}

	return ret;
}

void TDEEventDevice::internalStartMonitoring(TDEHardwareDevices* hwmanager) {
	if (!m_monitorActive) {
		// For security and performance reasons, only monitor known ACPI buttons
		if (eventType() != TDEEventDeviceType::Unknown) {
			if (m_fd >= 0) {
				m_eventNotifier = new TQSocketNotifier(m_fd, TQSocketNotifier::Read, this);
				connect( m_eventNotifier, TQ_SIGNAL(activated(int)), this, TQ_SLOT(eventReceived()) );
				m_monitorActive = true;
			}
		}
		if (m_monitorActive == true) {
			// get initial state of switches
			internalReadProvidedSwitches();
			internalReadActiveSwitches();
			connect( this, TQ_SIGNAL(keyPressed(unsigned int, TDEEventDevice*)), hwmanager, TQ_SLOT(processEventDeviceKeyPressed(unsigned int, TDEEventDevice*)) );
		}
	}
}

void TDEEventDevice::eventReceived() {
	struct input_event ev;
	int r;
	r = read(m_fd, &ev, sizeof(struct input_event));
	if (r > 0) {
		if ((ev.type == EV_KEY) && (ev.value == 1)) {	// Only detect keypress events (value == 1)
			emit keyPressed(ev.code, this);
		}
		if (ev.type == EV_SW) {
			internalReadActiveSwitches();
			emit switchChanged();
		}
	}
}

void TDEEventDevice::processActiveSwitches() {
	TDESwitchType::TDESwitchType previousSwitches = m_switchActive;
	internalReadActiveSwitches();

	if (previousSwitches != m_switchActive) {
		emit switchChanged();
	}
}

void TDEEventDevice::connectNotify( const char* signal ) {
	if( !m_monitorActive && qstrcmp( signal, TQ_SIGNAL(switchChanged())) == 0 ) {
		m_watchTimer = new TQTimer(this);
		connect( m_watchTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(processActiveSwitches()) );
		m_watchTimer->start( 2500, false );
		m_monitorActive = true;

		// get initial state of switches
		internalReadProvidedSwitches();
		internalReadActiveSwitches();
	}
	TQObject::connectNotify( signal );
}

#include "tdeeventdevice.moc"
