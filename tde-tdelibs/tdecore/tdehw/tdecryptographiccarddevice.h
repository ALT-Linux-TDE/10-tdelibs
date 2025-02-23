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

#ifndef _TDECRYPTOGRAPHICCARDDEVICE_H
#define _TDECRYPTOGRAPHICCARDDEVICE_H

#include "ksslconfig.h"
#include "tdegenericdevice.h"

#ifndef _TDECRYPTOGRAPHICCARDDEVICE_INTERNAL
	#ifdef KSSL_HAVE_SSL
		typedef struct x509_st X509;
	#else
		struct X509;
	#endif
#endif

class TQEventLoopThread;
class CryptoCardDeviceWatcher;

typedef TQValueList<X509*> X509CertificatePtrList;
typedef TQValueListIterator<X509*> X509CertificatePtrListIterator;

class TDECORE_EXPORT TDECryptographicCardDevice : public TDEGenericDevice
{
	TQ_OBJECT

	public:
		/**
		 *  Constructor.
		 *  @param Device type
		 */
		TDECryptographicCardDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn=TQString::null);

		/**
		 * Destructor.
		 */
		~TDECryptographicCardDevice();

		/**
		 * Enable / disable monitoring of insert / remove events.
		 * @param enable true to enable, false to disable.
		 */
		void enableCardMonitoring(bool enable);

		/**
		 * Enable / disable PIN entry.
		 *
		 * @note You must connect to pinRequested and call setProvidedPin with
		 * the provided PIN, otherwise the TDECryptographicCardDevice object
		 * will hang waiting for input.
		 *
		 * @param enable true to enable, false to disable.
		 *
		 * @see setProvidedPin(TQString pin)
		 * @see pinRequested
		 */
		void enablePINEntryCallbacks(bool enable);

		/**
		 * If monitoring of insert / remove events is enabled,
		 * return whether or not a card is present.
		 * @return -1 if status unknown, 0 if card not present,
		 * 1 if card is present.
		 */
		int cardPresent();

		/**
		 * If monitoring of insert / remove events is enabled,
		 * and a card has been inserted, @return the card's ATR.
		 * @return TQString::null if no card or card status unknown.
		 */
		TQString cardATR();

		/**
		 * If monitoring of insert / remove events is enabled,
		 * and a card has been inserted, @return a list of all
		 * X509 certificates on the card.
		 * @return an empty list if no card or card contents unknown.
		 *
		 * @example KSSLCertificate* tdeCert = KSSLCertificate::fromX509(cardX509Certificates().first());
		 */
		X509CertificatePtrList cardX509Certificates();

		/**
		 * Sets the user-provided PIN from within the pinRequested callback.
		 * This method must not be called from anywhere else in user code.
		 * @param pin the user-provided PIN, TQString::null to abort
		 *
		 * @see pinRequested(TQString prompt)
		 */
		void setProvidedPin(TQString pin);

		/**
		 * If the inserted card and system configuration provides a PIN for automatic
		 * pin-less operation, @returns the PIN to use when unlocking the card, otherwise
		 * @returns TQString::null.
		 *
		 * @see pinRequested(TQString prompt)
		 * @see setProvidedPin(TQString pin)
		 */
		TQString autoPIN();

		/**
		 * If monitoring of insert / remove events is enabled, and a card has been inserted,
		 * decrypt data originally encrypted using a public key from one of the certificates
		 * stored on the card.
		 * This operation takes place on the card, and in most cases will require PIN entry.
		 * This method decrypts one data object only
		 * @param ciphertext Encrypted data
		 * @param plaintext Decrypted data
		 * @param errstr Pointer to TQString to be loaded with error description on failure
		 * @return 0 on success, -1 on general failure, -2 on encryption failure, -3 on user cancel
		 */
		int decryptDataEncryptedWithCertPublicKey(TQByteArray &ciphertext, TQByteArray &plaintext, TQString *errstr=NULL);

		/**
		 * If monitoring of insert / remove events is enabled, and a card has been inserted,
		 * decrypt data originally encrypted using a public key from one of the certificates
		 * stored on the card.
		 * This operation takes place on the card, and in most cases will require PIN entry.
		 * This method is used to decrypt multiple data objects in one pass.
		 * @param cipherTextList Encrypted data object list
		 * @param plainTextList Decrypted data object list
		 * @param retcodes Return code for each data object
		 * @param errstr Pointer to TQString to be loaded with error description on failure
		 * @return 0 on success, -1 on general failure, -2 on encryption failure, -3 on user cancel
		 */
		int decryptDataEncryptedWithCertPublicKey(TQValueList<TQByteArray> &cipherTextList, TQValueList<TQByteArray> &plainTextList, TQValueList<int> &retcodes, TQString *errstr);

		/**
		 * Create a new random key and encrypt with the public key
		 * contained in the given certificate.
		 * @param plaintext Generated (decrypted) random key
		 * @param ciphertext Encrypted key
		 * @param certificate X509 certificate containing the public key to use
		 * @return 0 on success, -1 on general failure, -2 on encryption failure
		 */
		static int createNewSecretRSAKeyFromCertificate(TQByteArray &plaintext, TQByteArray &ciphertext, X509* certificate);

		/**
		 * @return The built-in PKCS provider library file name, including the full path
		 */
		static TQString pkcsProviderLibrary();

	public slots:
		void cardStatusChanged(TQString status, TQString atr);
		void workerRequestedPin(TQString prompt);

	signals:
		void cardInserted(TDECryptographicCardDevice*);
		void cardRemoved(TDECryptographicCardDevice*);
		void certificateListAvailable(TDECryptographicCardDevice*);
		void pinRequested(TQString prompt, TDECryptographicCardDevice* cdevice);

	private:
		TQEventLoopThread *m_watcherThread;
		CryptoCardDeviceWatcher *m_watcherObject;

		bool m_cardPresent;
		TQString m_cardATR;
		X509CertificatePtrList m_cardCertificates;

	friend class TDEHardwareDevices;
	friend class CryptoCardDeviceWatcher;
};

#endif // _TDECRYPTOGRAPHICCARDDEVICE_H
