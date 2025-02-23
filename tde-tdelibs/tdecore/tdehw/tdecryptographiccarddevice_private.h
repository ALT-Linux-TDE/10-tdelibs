
/* This file is part of the TDE libraries
   Copyright (C) 2015 Timothy Pearson <kb9vqf@pearsoncomputing.net>

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

#ifndef _TDECRYPTOGRAPHICCARDDEVICE_PRIVATE_H
#define _TDECRYPTOGRAPHICCARDDEVICE_PRIVATE_H

#include "tdegenericdevice.h"

#ifdef WITH_PCSC
	#include <pcsclite.h>
	#include <wintypes.h>
	#include <winscard.h>
	#include <reader.h>
#endif

#ifdef WITH_PKCS
	#include <pkcs11-helper-1.0/pkcs11h-certificate.h>
	#include <pkcs11-helper-1.0/pkcs11h-openssl.h>
	#include <openssl/x509v3.h>
	#define PKCS11H_PROMPT_MASK_ALLOW_NONE (PKCS11H_PROMPT_MASK_ALLOW_ALL & ~PKCS11H_PROMPT_MASK_ALLOW_ALL)
#endif

class TDECryptographicCardDevice;

class CryptoCardDeviceWatcher : public TQObject
{
	TQ_OBJECT

	public:
		CryptoCardDeviceWatcher();
		~CryptoCardDeviceWatcher();

	public slots:
		void run();
		void requestTermination();
		TQString getCardATR(TQString readerName);
		int retrieveCardCertificates(TQString readerName);

	signals:
		void statusChanged(TQString, TQString);
		void pinRequested(TQString);

	public:
		int initializePkcs();
		TQString doPinRequest(TQString prompt);
		void setProvidedPin(TQString pin);
		void retrySamePin(bool enable);
		void enablePINEntryCallbacks(bool enable);

	public:
		TDECryptographicCardDevice *cardDevice;

	private:
		void deleteAllCertificatesFromCache();

	private:
		bool m_terminationRequested;
		bool m_pinCallbacksEnabled;
		TQString m_cardPIN;
		bool m_cardPINPromptDone;
		bool m_cardReusePIN;
#ifdef WITH_PCSC
		SCARDCONTEXT m_cardContext;
		SCARD_READERSTATE *m_readerStates;
#endif
};

#endif // _TDECRYPTOGRAPHICCARDDEVICE_PRIVATE_H
