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

#ifndef _TDECPUDEVICE_H
#define _TDECPUDEVICE_H

#include "tdegenericdevice.h"

class TDECORE_EXPORT TDECPUDevice : public TDEGenericDevice
{
	public:
		/**
		*  Constructor.
		*  @param Device type
		*/
		TDECPUDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);
		
		/**
		* Destructor.
		*/
		~TDECPUDevice();

		/**
		* @return a double with the current CPU frequency in MHz, if available
		*/
		double frequency();

		/**
		* @return a double with the minimum CPU frequency in MHz, if available
		*/
		double minFrequency();

		/**
		* @return a double with the maximum CPU frequency in MHz, if available
		*/
		double maxFrequency();

		/**
		* @return a double with the transition latency in ns, if available
		*/
		double transitionLatency();

		/**
		* @return a TQString with the current CPU governor policy, if available
		*/
		TQString governor();

		/**
		* @return a TQString with the current CPU scaling driver, if available
		*/
		TQString scalingDriver();

		/**
		* @return a TQStringList with the IDs of all processors that are dependent on the frequency/power settings of this one, if available
		*/
		TQStringList dependentProcessors();

		/**
		* @return a TQStringList with all valid scaling frequencies in Hz, if available
		*/
		TQStringList availableFrequencies();

		/**
		* @return a TQStringList with all available governor policies, if available
		*/
		TQStringList availableGovernors();

		/**
		* @return true if permissions allow the CPU governor to be set, false if not
		*/
		bool canSetGovernor();

		/**
		* @param gv a TQString with the new CPU governor policy name
		*/
		void setGovernor(TQString gv);

		/**
		* @return true if permissions allow the CPU maximum frequency to be set, false if not
		*/
		bool canSetMaximumScalingFrequency();

		/**
		* @param gv a double with the new CPU maximum frequency in MHz
		*/
		void setMaximumScalingFrequency(double fr);

		/**
		* @return an integer with the core number, starting at 0
		*/
		int coreNumber();

	protected:
		/**
		* @param fr a double with the current CPU frequency in MHz, if available
		* @internal
		*/
		void internalSetFrequency(double fr);

		/**
		* @param fr a double with the minimum CPU frequency in MHz, if available
		* @internal
		*/
		void internalSetMinFrequency(double fr);

		/**
		* @param fr a double with the maximum CPU frequency in MHz, if available
		* @internal
		*/
		void internalSetMaxFrequency(double fr);

		/**
		* @param tl a double with the transition latency in ns, if available
		* @internal
		*/
		void internalSetTransitionLatency(double tl);

		/**
		* @param gr a TQString with the current CPU governor policy, if available
		* @internal
		*/
		void internalSetGovernor(TQString gr);

		/**
		* @param dr a TQString with the current CPU scaling driver, if available
		* @internal
		*/
		void internalSetScalingDriver(TQString dr);

		/**
		* @param dp a TQStringList with the IDs of all processors that are dependent on the frequency/power settings of this one, if available
		* @internal
		*/
		void internalSetDependentProcessors(TQStringList dp);

		/**
		* @param af a TQStringList with all valid scaling frequencies in Hz, if available
		* @internal
		*/
		void internalSetAvailableFrequencies(TQStringList af);

		/**
		* @param gp a TQStringList with all available governor policies, if available
		* @internal
		*/
		void internalSetAvailableGovernors(TQStringList gp);

		/**
		* @param cn an integer with the core number, starting at 0
		* @internal
		*/
		void internalSetCoreNumber(int cn);

	private:
		double m_frequency;
		double m_minfrequency;
		double m_maxfrequency;
		double m_transitionlatency;
		TQString m_governor;
		TQString m_scalingdriver;
		TQStringList m_tiedprocs;
		TQStringList m_frequencies;
		TQStringList m_governers;
		int m_corenumber;

	friend class TDEHardwareDevices;
};


#endif // _TDECPUDEVICE_H
