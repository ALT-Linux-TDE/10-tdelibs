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

#ifndef _TDESENSORDEVICE_H
#define _TDESENSORDEVICE_H

#include "tdegenericdevice.h"

class TDECORE_EXPORT TDESensorCluster
{
	public:
		/**
		*  Constructor.
		*/
		TDESensorCluster();

		TQString label;
		double current;
		double minimum;
		double maximum;
		double warning;
		double critical;
};

typedef TQMap<TQString, TDESensorCluster> TDESensorClusterMap;

class TDECORE_EXPORT TDESensorDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDESensorDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDESensorDevice();

		/**
		* @return a TDESensorClusterMap with the current sensor values
		*/
		TDESensorClusterMap values();

	protected:
		/**
		* @param a TDESensorClusterMap with the current sensor values
		* @internal
		*/
		void internalSetValues(TDESensorClusterMap cl);

	private:
		TDESensorClusterMap m_sensorValues;

	friend class TDEHardwareDevices;
};

#endif // _TDESENSORDEVICE_H
