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

#include "tdesensordevice.h"

#include "config.h"

TDESensorCluster::TDESensorCluster() {
	label = TQString::null;
	current = -1;
	minimum = -1;
	maximum = -1;
	warning = -1;
	critical = -1;
}

TDESensorDevice::TDESensorDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn) {
}

TDESensorDevice::~TDESensorDevice() {
}

TDESensorClusterMap TDESensorDevice::values() {
	return m_sensorValues;
}

void TDESensorDevice::internalSetValues(TDESensorClusterMap cl) {
	m_sensorValues = cl;
}

#include "tdesensordevice.moc"
