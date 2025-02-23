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

#ifdef WITH_PKCS
	#define _TDECRYPTOGRAPHICCARDDEVICE_INTERNAL 1
#endif

#include "tdecryptographiccarddevice_private.h"
#include "tdecryptographiccarddevice.h"

#include <tqpixmap.h>
#include <tqtimer.h>
#include <tqthread.h>
#include <tqeventloop.h>
#include <tqapplication.h>

#include "tdeglobal.h"
#include "tdelocale.h"
#include "tdeapplication.h"

#include "tdehardwaredevices.h"

#include "config.h"

// 1 second
#define PCSC_POLL_TIMEOUT_S 1000

#define CARD_MAX_LOGIN_RETRY_COUNT 3

/* FIXME
 * This is incomplete
 */
#ifdef WITH_PCSC
static TQString pcsc_error_code_to_string(long errcode) {
	if (errcode == SCARD_W_UNPOWERED_CARD) {
		return i18n("card not powered on");
	}
	else if (errcode == SCARD_E_PROTO_MISMATCH) {
		return i18n("protocol mismatch");
	}
	else {
		return TQString::null;
	}
}
#endif

CryptoCardDeviceWatcher::CryptoCardDeviceWatcher() {
#ifdef WITH_PCSC
	m_readerStates = NULL;
#endif
	m_cardPINPromptDone = true;
	m_pinCallbacksEnabled = false;
	m_cardReusePIN = false;
}

CryptoCardDeviceWatcher::~CryptoCardDeviceWatcher() {
#ifdef WITH_PCSC
	free(m_readerStates);
#endif
}

void CryptoCardDeviceWatcher::run() {
#ifdef WITH_PCSC
	bool first_loop;
	unsigned int i;
	long ret;

	DWORD dword_readers;
	LPSTR lpstring_readers = NULL;

	TQStringList readers;

	first_loop = true;
	m_terminationRequested = false;

	TQEventLoop* eventLoop = TQApplication::eventLoop();
	if (!eventLoop) return;

	ret = SCardEstablishContext(SCARD_SCOPE_SYSTEM, NULL, NULL, &m_cardContext);
	if (ret != SCARD_S_SUCCESS) {
		printf("TDECryptographicCardDevice: PCSC SCardEstablishContext cannot connect to resource manager (%lX)", ret);
		eventLoop->exit(0);
		return;
	}

	ret = SCardListReaders(m_cardContext, NULL, NULL, &dword_readers);
	if (ret == SCARD_S_SUCCESS) {
		lpstring_readers = (LPSTR)malloc(sizeof(char)*dword_readers);
		if (lpstring_readers == NULL) {
			printf("TDECryptographicCardDevice: insufficient memory, aborting");
			eventLoop->exit(0);
			return;
		}

		ret = SCardListReaders(m_cardContext, NULL, lpstring_readers, &dword_readers);
		if (ret == SCARD_S_SUCCESS) {
			/* Extract reader names from the null separated string */
			char *ptr = lpstring_readers;
			while (*ptr != '\0') {
				readers.append(ptr);
				ptr += strlen(ptr)+1;
			}

			free(lpstring_readers);

			m_readerStates = (SCARD_READERSTATE*)calloc(readers.count(), sizeof(*m_readerStates));
			if (m_readerStates == NULL) {
				printf("TDECryptographicCardDevice: insufficient memory, aborting");
				free(lpstring_readers);
				eventLoop->exit(0);
				return;
			}

			for (i=0; i<readers.count(); i++) {
				m_readerStates[i].szReader = strdup(readers[i].ascii());
				m_readerStates[i].dwCurrentState = SCARD_STATE_UNAWARE;
			}

			ret = SCardGetStatusChange(m_cardContext, PCSC_POLL_TIMEOUT_S, m_readerStates, readers.count());
			while ((ret == SCARD_S_SUCCESS) || (ret == SCARD_E_TIMEOUT)) {
				if (m_terminationRequested) {
					for (i=0; i<readers.count(); i++) {
						free((char*)m_readerStates[i].szReader);
						m_readerStates[i].szReader = NULL;
					}
					eventLoop->exit(0);
					return;
				}

				for (i=0; i<readers.count(); i++) {
					/* FIXME
					 * Find a better / more reliable way to match the card low level device to the PCSC name
					 */
					SCARDHANDLE hCard = 0;
					DWORD dwActiveProtocol = 0;
					DWORD cByte = 0;
					TQString reader_vendor_name;
					TQString reader_interface_type;
					
					ret = SCardConnect(m_cardContext, readers[i].ascii(), SCARD_SHARE_DIRECT, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
					if (ret == SCARD_S_SUCCESS) {
						ret = SCardGetAttrib(hCard, SCARD_ATTR_VENDOR_NAME, NULL, &cByte);
						if (ret == SCARD_S_SUCCESS) {
							char* data = new char[cByte];
							ret = SCardGetAttrib(hCard, SCARD_ATTR_VENDOR_NAME, (LPBYTE)data, &cByte);
							reader_vendor_name = data;
							delete [] data;
						}
						ret = SCardGetAttrib(hCard, SCARD_ATTR_VENDOR_IFD_TYPE, NULL, &cByte);
						if (ret == SCARD_S_SUCCESS) {
							char* data = new char[cByte];
							ret = SCardGetAttrib(hCard, SCARD_ATTR_VENDOR_IFD_TYPE, (LPBYTE)data, &cByte);
							reader_interface_type = data;
							delete [] data;
						}
						SCardDisconnect(hCard, SCARD_LEAVE_CARD);
					}

					/* FIXME
					 * If only one reader was detected by PCSC, assume it corresponds to the current device node.
					 * This is fragile, but avoids corner cases with common systems failing to work due to
					 * mismatched udev / PCSC card reader vendor names...
					 */
					if (readers.count() > 1) {
						if (!readers[i].contains(cardDevice->friendlyName())) {
							if (!cardDevice->friendlyName().contains(reader_vendor_name) ||
								((reader_interface_type != "") && !cardDevice->friendlyName().contains(reader_vendor_name))) {
								continue;
							}
						}
					}

					if (first_loop) {
						if (m_readerStates[i].dwEventState & SCARD_STATE_PRESENT) {
							// sleep(1);	// Allow the card to settle
							TQString atr = getCardATR(readers[i]);
							retrieveCardCertificates(readers[i]);
							statusChanged("PRESENT", atr);
						}
						else {
							deleteAllCertificatesFromCache();
						}
						first_loop = false;
					}
					if (m_readerStates[i].dwEventState & SCARD_STATE_CHANGED) {
						if ((m_readerStates[i].dwCurrentState & SCARD_STATE_PRESENT)
							&& (m_readerStates[i].dwEventState & SCARD_STATE_EMPTY)) {
							deleteAllCertificatesFromCache();
							statusChanged("REMOVED", TQString::null);
						}
						else if ((m_readerStates[i].dwCurrentState & SCARD_STATE_EMPTY)
							&& (m_readerStates[i].dwEventState & SCARD_STATE_PRESENT)) {
							// sleep(1);	// Allow the card to settle
							TQString atr = getCardATR(readers[i]);
							retrieveCardCertificates(readers[i]);
							statusChanged("INSERTED", atr);
						}
						m_readerStates[i].dwCurrentState = m_readerStates[i].dwEventState;
					}
					else {
						continue;
					}
				}
				ret = SCardGetStatusChange(m_cardContext, PCSC_POLL_TIMEOUT_S, m_readerStates, readers.count());
			}
		}
	}

	eventLoop->exit(0);
#endif
}

void CryptoCardDeviceWatcher::requestTermination() {
	m_terminationRequested = true;
}

void CryptoCardDeviceWatcher::setProvidedPin(TQString pin) {
	m_cardPIN = pin;
	m_cardPINPromptDone = true;
}

void CryptoCardDeviceWatcher::retrySamePin(bool enable) {
	m_cardReusePIN = enable;
	if (!enable) {
		m_cardPIN = "SHREDDINGTHEPINISMOSTSECURE";
		m_cardPIN = TQString::null;
	}
}

TQString CryptoCardDeviceWatcher::getCardATR(TQString readerName) {
#ifdef WITH_PCSC
	unsigned int i;
	long ret;
	TQString atr_formatted;
	SCARDHANDLE hCard = 0;
	DWORD dwActiveProtocol = 0;
	DWORD cByte = 0;

	ret = SCardConnect(m_cardContext, readerName.ascii(), SCARD_SHARE_SHARED, SCARD_PROTOCOL_T0 | SCARD_PROTOCOL_T1, &hCard, &dwActiveProtocol);
	if (ret == SCARD_S_SUCCESS) {
		ret = SCardGetAttrib(hCard, SCARD_ATTR_ATR_STRING, NULL, &cByte);
		if (ret == SCARD_S_SUCCESS) {
			char* data = new char[cByte];
			ret = SCardGetAttrib(hCard, SCARD_ATTR_ATR_STRING, (LPBYTE)data, &cByte);
			atr_formatted = TQString::null;
			for (i=0; i<cByte; i++) {
				TQString formatted;
				formatted.sprintf("%02x ", ((uint8_t)(*(data+i))));
				atr_formatted.append(formatted.upper());
			}
			atr_formatted = atr_formatted.stripWhiteSpace();
			delete [] data;
			SCardDisconnect(hCard, SCARD_LEAVE_CARD);
		}
	}
	else {
		TQString errstring = pcsc_error_code_to_string(ret);
		if (errstring != "") {
			atr_formatted = i18n("Unknown (%1)").arg(errstring);
		}
		else {
			atr_formatted = TQString("CARD_CONNECT_FAIL (%1)").arg(ret, 0, 16);
		}
	}

	return atr_formatted;
#else
	return TQString::null;
#endif
}

void CryptoCardDeviceWatcher::enablePINEntryCallbacks(bool enable) {
	m_pinCallbacksEnabled = enable;
}

TQString CryptoCardDeviceWatcher::doPinRequest(TQString prompt) {
	if (!m_pinCallbacksEnabled) {
		return TQString::null;
	}

	if (m_cardReusePIN) {
		return m_cardPIN;
	}

	m_cardPINPromptDone = false;
	emit(pinRequested(prompt));
	while (!m_cardPINPromptDone) {
		usleep(100);
	}

	if (m_cardPIN.length() > 0) {
		return m_cardPIN;
	}
	else {
		return TQString::null;
	}
}

#ifdef WITH_PKCS
static void pkcs_log_hook(IN void * const global_data, IN unsigned flags, IN const char * const format, IN va_list args) {
	vprintf(format, args);
	printf("\n");
}

static PKCS11H_BOOL pkcs_pin_hook(IN void * const global_data, IN void * const user_data, IN const pkcs11h_token_id_t token, IN const unsigned retry, OUT char * const pin, IN const size_t pin_max) {
	CryptoCardDeviceWatcher* watcher = (CryptoCardDeviceWatcher*)global_data;

	TQString providedPin = watcher->doPinRequest(i18n("Please enter the PIN for '%1'").arg(token->display));
	if (providedPin.length() > 0) {
		snprintf(pin, pin_max, "%s", providedPin.ascii());

		// Success
		return 1;
	}
	else {
		// Abort
		return 0;
	}
}
#endif

int CryptoCardDeviceWatcher::initializePkcs() {
#if defined(WITH_PKCS)
	CK_RV rv;
	printf("Initializing pkcs11-helper\n");
	if ((rv = pkcs11h_initialize()) != CKR_OK) {
		printf("pkcs11h_initialize failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}

	printf("Registering pkcs11-helper hooks\n");
	if ((rv = pkcs11h_setLogHook(pkcs_log_hook, this)) != CKR_OK) {
		printf("pkcs11h_setLogHook failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}
	pkcs11h_setLogLevel(PKCS11H_LOG_WARN);
	// pkcs11h_setLogLevel(PKCS11H_LOG_DEBUG2);

#if 0
	if ((rv = pkcs11h_setTokenPromptHook(_pkcs11h_hooks_token_prompt, NULL)) != CKR_OK) {
		printf("pkcs11h_setTokenPromptHook failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}
#endif

	if ((rv = pkcs11h_setMaxLoginRetries(CARD_MAX_LOGIN_RETRY_COUNT)) != CKR_OK) {
		printf("pkcs11h_setMaxLoginRetries failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}

	if ((rv = pkcs11h_setPINPromptHook(pkcs_pin_hook, this)) != CKR_OK) {
		printf("pkcs11h_setPINPromptHook failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}

	printf("Adding provider '%s'\n", OPENSC_PKCS11_PROVIDER_LIBRARY);
		if ((rv = pkcs11h_addProvider(OPENSC_PKCS11_PROVIDER_LIBRARY, OPENSC_PKCS11_PROVIDER_LIBRARY, false, PKCS11H_PRIVATEMODE_MASK_AUTO, PKCS11H_SLOTEVENT_METHOD_AUTO, 0, false)) != CKR_OK) {
		printf("pkcs11h_addProvider failed: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}

	return 0;
#else
	return -1;
#endif
}

int CryptoCardDeviceWatcher::retrieveCardCertificates(TQString readerName) {
#if defined(WITH_PKCS)
	int ret = -1;

	CK_RV rv;
	pkcs11h_certificate_id_list_t issuers;
	pkcs11h_certificate_id_list_t certs;

	if (initializePkcs() < 0) {
		printf("Unable to initialize PKCS\n");
		return -1;
	}

	rv = pkcs11h_certificate_enumCertificateIds(PKCS11H_ENUM_METHOD_CACHE, NULL, PKCS11H_PROMPT_MASK_ALLOW_PIN_PROMPT, &issuers, &certs);
	if ((rv != CKR_OK) || (certs == NULL)) {
		printf("Cannot enumerate certificates: %s\n", pkcs11h_getMessage(rv));
		return -1;
	}
	printf("Successfully enumerated certificates\n");

	int i = 0;
	for (pkcs11h_certificate_id_list_t cert = certs; cert != NULL; cert = cert->next) {
		TQString label = cert->certificate_id->displayName;
		printf("Certificate %d name: '%s'\n", i, label.ascii());

		pkcs11h_certificate_t certificate;
		rv = pkcs11h_certificate_create(certs->certificate_id, NULL, PKCS11H_PROMPT_MASK_ALLOW_PIN_PROMPT, PKCS11H_PIN_CACHE_INFINITE, &certificate);
		if (rv != CKR_OK) {
			printf("Cannot read certificate: %s\n", pkcs11h_getMessage(rv));
			pkcs11h_certificate_freeCertificateId(certs->certificate_id);
			ret = -1;
			break;
		}

		pkcs11h_certificate_freeCertificateId(certs->certificate_id);

		pkcs11h_openssl_session_t openssl_session = NULL;
		if ((openssl_session = pkcs11h_openssl_createSession(certificate)) == NULL) {
			printf("Cannot initialize openssl session to retrieve cryptographic objects\n");
			pkcs11h_certificate_freeCertificate(certificate);
			ret = -1;
			break;
		}
		certificate = NULL;	// the certificate object is managed by openssl_session

		X509* x509_local;
		x509_local = pkcs11h_openssl_session_getX509(openssl_session);
		if (x509_local) {
			printf("Successfully retrieved X509 certificate\n");
		}
		else {
			printf("Cannot get X509 object\n");
			ret = -1;
		}
#if 0
		RSA* rsa_local;
		rsa_local = pkcs11h_openssl_session_getRSA(openssl_session);
		if (rsa_local) {
			printf("Successfully retrieved RSA public key\n");
		}
		else {
			printf("Cannot get RSA object\n");
			ret = -1;
		}
#endif

		X509* x509_copy = X509_dup(x509_local);
		if (x509_copy) {
			cardDevice->m_cardCertificates.append(x509_copy);
		}
		else {
			printf("Unable to copy X509 certificate\n");
		}

		pkcs11h_openssl_freeSession(openssl_session);
		i++;
	}

	pkcs11h_certificate_freeCertificateIdList(issuers);

	return ret;
#else
	return -1;
#endif
}

void CryptoCardDeviceWatcher::deleteAllCertificatesFromCache() {
#ifdef WITH_PKCS
	X509 *x509_cert;

	X509CertificatePtrListIterator it;
	for (it = cardDevice->m_cardCertificates.begin(); it != cardDevice->m_cardCertificates.end(); ++it) {
		x509_cert = *it;
		X509_free(x509_cert);
	}

	cardDevice->m_cardCertificates.clear();
#endif
}

TDECryptographicCardDevice::TDECryptographicCardDevice(TDEGenericDeviceType::TDEGenericDeviceType dt, TQString dn) : TDEGenericDevice(dt, dn),
	m_watcherThread(NULL),
	m_watcherObject(NULL),
	m_cardPresent(false) {
}

TDECryptographicCardDevice::~TDECryptographicCardDevice() {
	enableCardMonitoring(false);
}

void TDECryptographicCardDevice::enableCardMonitoring(bool enable) {
#ifdef WITH_PCSC
	if (enable) {
		if (m_watcherObject && m_watcherThread) {
			// Monitoring thread already active
			if ((cardPresent() == 1) && (cardX509Certificates().count() > 0)) {
				// Card was already inserted and initialized
				emit(certificateListAvailable(this));
			}

			// Abort!
			return;
		}

		m_watcherThread = new TQEventLoopThread();
		m_watcherObject = new CryptoCardDeviceWatcher();

		m_watcherObject->cardDevice = this;
		m_watcherObject->moveToThread(m_watcherThread);
		TQObject::connect(m_watcherObject, TQ_SIGNAL(statusChanged(TQString,TQString)), this, TQ_SLOT(cardStatusChanged(TQString,TQString)));
		TQObject::connect(m_watcherObject, TQ_SIGNAL(pinRequested(TQString)), this, TQ_SLOT(workerRequestedPin(TQString)));
		TQTimer::singleShot(0, m_watcherObject, TQ_SLOT(run()));

		m_watcherThread->start();
	}
	else {
		if (m_watcherObject) {
			m_watcherObject->requestTermination();
		}
		if (m_watcherThread) {
			m_watcherThread->wait();
			delete m_watcherThread;
			m_watcherThread = NULL;
		}
		if (m_watcherObject) {
			delete m_watcherObject;
			m_watcherObject = NULL;
		}
	}
#endif
}

void TDECryptographicCardDevice::enablePINEntryCallbacks(bool enable) {
	if (m_watcherObject) {
		m_watcherObject->enablePINEntryCallbacks(enable);
	}
}

int TDECryptographicCardDevice::cardPresent() {
	if (m_watcherObject && m_watcherThread) {
		if (m_cardPresent)
			return 1;
		else
			return 0;
	}
	else {
		return -1;
	}
}

TQString TDECryptographicCardDevice::cardATR() {
	if (m_watcherObject && m_watcherThread) {
		if (m_cardPresent)
			return m_cardATR;
		else
			return TQString::null;
	}
	else {
		return TQString::null;
	}
}

X509CertificatePtrList TDECryptographicCardDevice::cardX509Certificates() {
	if (m_watcherObject && m_watcherThread) {
		if (m_cardPresent) {
			return m_cardCertificates;
		}
		else {
			return X509CertificatePtrList();
		}
	}
	else {
		return X509CertificatePtrList();
	}
}

void TDECryptographicCardDevice::cardStatusChanged(TQString status, TQString atr) {
	if (status == "INSERTED") {
		m_cardPresent = true;
		m_cardATR = atr;
		emit(cardInserted(this));
		if (m_cardCertificates.count() > 0) {
			emit(certificateListAvailable(this));
		}
	}
	else if (status == "REMOVED") {
		m_cardPresent = false;
		m_cardATR = atr;
		emit(cardRemoved(this));
	}
	else if (status == "PRESENT") {
		m_cardATR = atr;
		m_cardPresent = true;
		if (m_cardCertificates.count() > 0) {
			emit(certificateListAvailable(this));
		}
	}
}

void TDECryptographicCardDevice::setProvidedPin(TQString pin) {
	if (m_watcherObject) {
		m_watcherObject->setProvidedPin(pin);
	}
}

TQString TDECryptographicCardDevice::autoPIN() {
#if defined(WITH_PKCS)
	TQString retString = TQString::null;

	// Use subjAltName field in card certificate to provide the card's PIN,
	// in order to support optional pin-less operation.
	// Parse the TDE autologin extension
	// Structure:
	// OID 1.3.6.1.4.1.40364.1.2.1
	// 	SEQUENCE
	// 		ASN1_CONSTRUCTED [index: 0] (field name: pin)
	// 			GeneralString

	// Register custom OID type for TDE autopin data
	ASN1_OBJECT* tde_autopin_data_object = OBJ_txt2obj("1.3.6.1.4.1.40364.1.2.1", 0);

	int i;
	X509CertificatePtrListIterator it;
	for (it = m_cardCertificates.begin(); it != m_cardCertificates.end(); ++it) {
		X509* x509_cert = *it;
		GENERAL_NAMES* subjectAltNames = (GENERAL_NAMES*)X509_get_ext_d2i(x509_cert, NID_subject_alt_name, NULL, NULL);
		int altNameCount = sk_GENERAL_NAME_num(subjectAltNames);
		for (i=0; i < altNameCount; i++) {
			GENERAL_NAME* generalName = sk_GENERAL_NAME_value(subjectAltNames, i);
			if (generalName->type == GEN_OTHERNAME) {
				OTHERNAME* otherName = generalName->d.otherName;
				if (!OBJ_cmp(otherName->type_id, tde_autopin_data_object)) {
					ASN1_TYPE* asnValue = otherName->value;
					if (asnValue) {
						// Found autopin structure
						ASN1_TYPE* asnSeqValue = NULL;
						ASN1_GENERALSTRING* asnGeneralString = NULL;
						STACK_OF(ASN1_TYPE) *asnSeqValueStack = NULL;
						long asn1SeqValueObjectLength;
						int asn1SeqValueObjectTag;
						int asn1SeqValueObjectClass;
						int returnCode;
						int index = 0;	// Search for the PIN field

#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
						const uint8_t* asnSeqValueString = ASN1_STRING_get0_data(asnValue->value.sequence);
						asnSeqValueStack = d2i_ASN1_SEQUENCE_ANY(NULL, &asnSeqValueString, ASN1_STRING_length(asnValue->value.sequence));
#else
						uint8_t* asnSeqValueString = ASN1_STRING_data(asnValue->value.sequence);
						asnSeqValueStack = ASN1_seq_unpack_ASN1_TYPE(asnSeqValueString, ASN1_STRING_length(asnValue->value.sequence), d2i_ASN1_TYPE, ASN1_TYPE_free);
#endif
						asnSeqValue = sk_ASN1_TYPE_value(asnSeqValueStack, index);
						if (asnSeqValue) {
							if (asnSeqValue->value.octet_string->data[0] == ((V_ASN1_CONSTRUCTED | V_ASN1_CONTEXT_SPECIFIC) + index)) {
								const unsigned char* asn1SeqValueObjectData = asnSeqValue->value.sequence->data;
								returnCode = ASN1_get_object(&asn1SeqValueObjectData, &asn1SeqValueObjectLength, &asn1SeqValueObjectTag, &asn1SeqValueObjectClass, asnSeqValue->value.sequence->length);
								if (!(returnCode & 0x80)) {
									if (returnCode == (V_ASN1_CONSTRUCTED + index)) {
										if (d2i_ASN1_GENERALSTRING(&asnGeneralString, &asn1SeqValueObjectData, asn1SeqValueObjectLength) != NULL) {
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
											retString = TQString((const char *)ASN1_STRING_get0_data(asnGeneralString));
#else
											retString = TQString((const char *)ASN1_STRING_data(asnGeneralString));
#endif
										}
									}
								}
							}
						}
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
						sk_ASN1_TYPE_pop_free(asnSeqValueStack, ASN1_TYPE_free);
#endif
					}
				}
			}
		}
	}

	// Clean up
	OBJ_cleanup();

	return retString;
#else
	return TQString::null;
#endif
}

void TDECryptographicCardDevice::workerRequestedPin(TQString prompt) {
	emit(pinRequested(prompt, this));
}

int TDECryptographicCardDevice::decryptDataEncryptedWithCertPublicKey(TQByteArray &ciphertext, TQByteArray &plaintext, TQString *errstr) {
	TQValueList<TQByteArray> cipherTextList;
	TQValueList<TQByteArray> plainTextList;
	TQValueList<int> retCodeList;

	cipherTextList.append(ciphertext);

	this->decryptDataEncryptedWithCertPublicKey(cipherTextList, plainTextList, retCodeList, errstr);

	plaintext = plainTextList[0];
	return retCodeList[0];
}

int TDECryptographicCardDevice::decryptDataEncryptedWithCertPublicKey(TQValueList<TQByteArray> &cipherTextList, TQValueList<TQByteArray> &plainTextList, TQValueList<int> &retcodes, TQString *errstr) {
#if defined(WITH_PKCS)
	int ret = -1;

	if (!m_watcherObject) {
		if (errstr) *errstr = i18n("Card watcher object not available");
		return -1;
	}

	CK_RV rv;
	pkcs11h_certificate_id_list_t issuers;
	pkcs11h_certificate_id_list_t certs;

	if (m_watcherObject->initializePkcs() < 0) {
		if (errstr) *errstr = i18n("Unable to initialize PKCS");
		return -1;
	}

	rv = pkcs11h_certificate_enumCertificateIds(PKCS11H_ENUM_METHOD_CACHE, NULL, PKCS11H_PROMPT_MASK_ALLOW_PIN_PROMPT, &issuers, &certs);
	if ((rv != CKR_OK) || (certs == NULL)) {
		if (errstr) *errstr = i18n("Cannot enumerate certificates: %1").arg(pkcs11h_getMessage(rv));
		return -1;
	}

	int i = 0;
	for (pkcs11h_certificate_id_list_t cert = certs; cert != NULL; cert = cert->next) {
		TQString label = cert->certificate_id->displayName;

		pkcs11h_certificate_t certificate;
		rv = pkcs11h_certificate_create(certs->certificate_id, NULL, PKCS11H_PROMPT_MASK_ALLOW_PIN_PROMPT, PKCS11H_PIN_CACHE_INFINITE, &certificate);
		if (rv != CKR_OK) {
			if (errstr) *errstr = i18n("Cannot read certificate: %1").arg(pkcs11h_getMessage(rv));
			pkcs11h_certificate_freeCertificateId(certs->certificate_id);
			ret = -1;
			break;
		}

		pkcs11h_certificate_freeCertificateId(certs->certificate_id);

		pkcs11h_openssl_session_t openssl_session = NULL;
		if ((openssl_session = pkcs11h_openssl_createSession(certificate)) == NULL) {
			if (errstr) *errstr = i18n("Cannot initialize openssl session to retrieve cryptographic objects");
			pkcs11h_certificate_freeCertificate(certificate);
			ret = -1;
			break;
		}

		// Get certificate data
		X509* x509_local;
		x509_local = pkcs11h_openssl_session_getX509(openssl_session);
		if (!x509_local) {
			if (errstr) *errstr = i18n("Cannot get X509 object");
			ret = -1;
		}

		// Extract public key from X509 certificate
		EVP_PKEY* x509_pubkey = NULL;
		RSA* rsa_pubkey = NULL;
		x509_pubkey = X509_get_pubkey(x509_local);
		if (x509_pubkey) {
			rsa_pubkey = EVP_PKEY_get1_RSA(x509_pubkey);
		}

		// Check PIN
		rv = pkcs11h_certificate_ensureKeyAccess(certificate);
		if (rv != CKR_OK) {
			if (rv == CKR_CANCEL) {
				ret = -3;
				break;
			}
			else if ((rv == CKR_PIN_INCORRECT) || (rv == CKR_USER_NOT_LOGGED_IN)) {
				ret = -2;
				break;
			}
			else {
				ret = -2;
				break;
			}
		}

		// We know the cached PIN is correct; disable any further login prompts
		m_watcherObject->retrySamePin(true);

		TQValueList<TQByteArray>::iterator it;
		TQValueList<TQByteArray>::iterator it2;
		TQValueList<int>::iterator it3;
		plainTextList.clear();
		retcodes.clear();
		for (it = cipherTextList.begin(); it != cipherTextList.end(); ++it) {
			plainTextList.append(TQByteArray());
			retcodes.append(-1);
		}
		for (it = cipherTextList.begin(), it2 = plainTextList.begin(), it3 = retcodes.begin(); it != cipherTextList.end(); ++it, ++it2, ++it3) {
			TQByteArray& ciphertext = *it;
			TQByteArray& plaintext = *it2;
			int& retcode = *it3;

			// Verify minimum size
			if (ciphertext.size() < 16) {
				if (errstr) *errstr = i18n("Cannot decrypt: %1").arg(i18n("Ciphertext too small"));
				ret = -2;
				retcode = -2;
				continue;
			}

			// Try to get RSA parameters and verify maximum size
			if (rsa_pubkey) {
				unsigned int rsa_length = RSA_size(rsa_pubkey);
				if (ciphertext.size() > rsa_length) {
					if (errstr) *errstr = i18n("Cannot decrypt: %1").arg(i18n("Ciphertext too large"));
					ret = -2;
					retcode = -2;
					continue;
				}
			}

			size_t size = 0;
			// Determine output buffer size
			rv = pkcs11h_certificate_decryptAny(certificate, CKM_RSA_PKCS, (unsigned char*)ciphertext.data(), ciphertext.size(), NULL, &size);
			if (rv != CKR_OK) {
				if (errstr) *errstr = i18n("Cannot determine decrypted message length: %1 (%2)").arg(pkcs11h_getMessage(rv)).arg(rv);
				if (rv == CKR_CANCEL) {
					ret = -3;
					retcode = -3;
					break;
				}
				else if ((rv == CKR_PIN_INCORRECT) || (rv == CKR_USER_NOT_LOGGED_IN)) {
					ret = -2;
					retcode = -2;
					break;
				}
				else {
					ret = -2;
					retcode = -2;
				}
			}
			else {
				// Decrypt data
				plaintext.resize(size);
				rv = pkcs11h_certificate_decryptAny(certificate, CKM_RSA_PKCS, (unsigned char*)ciphertext.data(), ciphertext.size(), (unsigned char*)plaintext.data(), &size);
				if (rv != CKR_OK) {
					if (errstr) *errstr = i18n("Cannot decrypt: %1 (%2)").arg(pkcs11h_getMessage(rv)).arg(rv);
					if (rv == CKR_CANCEL) {
						ret = -3;
						retcode = -3;
						break;
					}
					else if ((rv == CKR_PIN_INCORRECT) || (rv == CKR_USER_NOT_LOGGED_IN)) {
						ret = -2;
						retcode = -2;
						break;
					}
					else {
						ret = -2;
						retcode = -2;
					}
				}
				else {
					if (errstr) *errstr = TQString::null;
					ret = 0;
					retcode = 0;
				}
			}
		}

		pkcs11h_openssl_freeSession(openssl_session);

		// Only interested in first certificate for now
		// FIXME
		// If cards with multiple certificates are used this should be modified to try decryption
		// using each certificate in turn...
		break;

		i++;
	}
	pkcs11h_certificate_freeCertificateIdList(issuers);

	// Restore normal login attempt method
	m_watcherObject->retrySamePin(false);

	return ret;
#else
	return -1;
#endif
}

int TDECryptographicCardDevice::createNewSecretRSAKeyFromCertificate(TQByteArray &plaintext, TQByteArray &ciphertext, X509* certificate) {
#if defined(WITH_PKCS)
	unsigned int i;
	int retcode = -1;

	// Extract public key from X509 certificate
	EVP_PKEY* x509_pubkey = NULL;
	RSA* rsa_pubkey = NULL;
	x509_pubkey = X509_get_pubkey(certificate);
	if (x509_pubkey) {
		rsa_pubkey = EVP_PKEY_get1_RSA(x509_pubkey);
	}

	if (rsa_pubkey) {
		// Determine encryption parameters
		// NOTE
		// RSA_PKCS1_OAEP_PADDING is preferred but cannot be decoded from
		// the command line via openssl at this time of this writing.
		int rsa_padding_style = RSA_PKCS1_PADDING;
		unsigned int rsa_length = RSA_size(rsa_pubkey);
		unsigned int max_key_length = rsa_length - 41;

		// Create a new random key as the plaintext
		plaintext.resize(max_key_length);
		for (i=0; i < max_key_length; i++) {
			plaintext[i] = TDEApplication::random();
		}

		// Encrypt data
		ciphertext.resize(rsa_length);
		if (RSA_public_encrypt(plaintext.size(), (unsigned char *)plaintext.data(), (unsigned char *)ciphertext.data(), rsa_pubkey, rsa_padding_style) < 0) {
			retcode = -2;
		}

		// Success!
		retcode = 0;
	}

	// Clean up
	if (rsa_pubkey) {
		RSA_free(rsa_pubkey);
	}
	if (x509_pubkey) {
		EVP_PKEY_free(x509_pubkey);
	}

	return retcode;
#else
	return -1;
#endif
}

TQString TDECryptographicCardDevice::pkcsProviderLibrary() {
#if defined(WITH_PKCS)
	return OPENSC_PKCS11_PROVIDER_LIBRARY;
#else
	return TQString::null;
#endif
}

#include "tdecryptographiccarddevice.moc"
#include "tdecryptographiccarddevice_private.moc"
