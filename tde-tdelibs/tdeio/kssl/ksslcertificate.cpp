/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif



#include <unistd.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqfile.h>

#include "kssldefs.h"
#include "ksslcertificate.h"
#include "ksslcertchain.h"
#include "ksslutils.h"

#include <kstandarddirs.h>
#include <kmdcodec.h>
#include <tdelocale.h>
#include <tqdatetime.h>
#include <tdetempfile.h>

#include <sys/types.h>

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/ssl.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/x509_vfy.h>
#include <openssl/pem.h>
#undef crypt
#endif

#include <kopenssl.h>
#include <tqcstring.h>
#include <kdebug.h>
#include "ksslx509v3.h"



static char hv[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};


class KSSLCertificatePrivate {
public:
	KSSLCertificatePrivate() {
		kossl = KOSSL::self();
		_lastPurpose = KSSLCertificate::None;
	}

	~KSSLCertificatePrivate() {
	}

	KSSLCertificate::KSSLValidation m_stateCache;
	bool m_stateCached;
	#ifdef KSSL_HAVE_SSL
		X509 *m_cert;
		X509_CRL *m_cert_crl;
	#endif
	KOSSL *kossl;
	KSSLCertChain _chain;
	KSSLX509V3 _extensions;
	KSSLCertificate::KSSLPurpose _lastPurpose;
};

KSSLCertificate::KSSLCertificate() {
	d = new KSSLCertificatePrivate;
	d->m_stateCached = false;
	TDEGlobal::dirs()->addResourceType("kssl", TDEStandardDirs::kde_default("data") + "kssl");
	#ifdef KSSL_HAVE_SSL
		d->m_cert = NULL;
		d->m_cert_crl = NULL;
	#endif
}


KSSLCertificate::KSSLCertificate(const KSSLCertificate& x) {
	d = new KSSLCertificatePrivate;
	d->m_stateCached = false;
	TDEGlobal::dirs()->addResourceType("kssl", TDEStandardDirs::kde_default("data") + "kssl");
	#ifdef KSSL_HAVE_SSL
		d->m_cert = NULL;
		d->m_cert_crl = NULL;
		setCert(KOSSL::self()->X509_dup(const_cast<KSSLCertificate&>(x).getCert()));
		KSSLCertChain *c = x.d->_chain.replicate();
		setChain(c->rawChain());
		delete c;
	#endif
}



KSSLCertificate::~KSSLCertificate() {
#ifdef KSSL_HAVE_SSL
	if (d->m_cert) {
		d->kossl->X509_free(d->m_cert);
	}
	if (d->m_cert_crl) {
		d->kossl->X509_CRL_free(d->m_cert_crl);
	}
#endif
	delete d;
}


KSSLCertChain& KSSLCertificate::chain() {
	return d->_chain;
}


KSSLCertificate *KSSLCertificate::fromX509(X509 *x5) {
KSSLCertificate *n = NULL;
#ifdef KSSL_HAVE_SSL
	if (x5) {
		n = new KSSLCertificate;
		n->setCert(KOSSL::self()->X509_dup(x5));
	}
#endif
return n;
}


KSSLCertificate *KSSLCertificate::fromString(TQCString cert) {
KSSLCertificate *n = NULL;
#ifdef KSSL_HAVE_SSL
	if (cert.length() == 0)
		return NULL;

	TQByteArray qba, qbb = cert.copy();
	KCodecs::base64Decode(qbb, qba);
	unsigned char *qbap = reinterpret_cast<unsigned char *>(qba.data());
	X509 *x5c = KOSSL::self()->d2i_X509(NULL, &qbap, qba.size());
	if (!x5c) {
		return NULL;
	}

	n = new KSSLCertificate;
	n->setCert(x5c);
#endif
return n;
}

KSSLCertificate *KSSLCertificate::crlFromString(TQCString cert) {
KSSLCertificate *n = NULL;
#ifdef KSSL_HAVE_SSL
	if (cert.length() == 0)
		return NULL;

	TQByteArray qba, qbb = cert.copy();
	KCodecs::base64Decode(qbb, qba);
	unsigned char *qbap = reinterpret_cast<unsigned char *>(qba.data());
	X509_CRL *x5c = KOSSL::self()->d2i_X509_CRL(NULL, &qbap, qba.size());
	if (!x5c) {
		return NULL;
	}

	n = new KSSLCertificate;
	n->setCRL(x5c);
#endif
return n;
}



TQString KSSLCertificate::getSubject() const {
TQString rc = "";

#ifdef KSSL_HAVE_SSL
	char *t = d->kossl->X509_NAME_oneline(d->kossl->X509_get_subject_name(d->m_cert), 0, 0);
	if (!t)
		return rc;
	rc = t;
	d->kossl->CRYPTO_free(t);
#endif
return rc;
}


TQString KSSLCertificate::getSerialNumber() const {
TQString rc = "";

#ifdef KSSL_HAVE_SSL
	ASN1_INTEGER *aint = d->kossl->X509_get_serialNumber(d->m_cert);
	if (aint) {
		rc = ASN1_INTEGER_QString(aint);
		// d->kossl->ASN1_INTEGER_free(aint);   this makes the sig test fail
	}
#endif
return rc;
}


TQString KSSLCertificate::getSignatureText() const {
TQString rc = "";

#ifdef KSSL_HAVE_SSL
char *s;
int n, i;

	const ASN1_BIT_STRING *signature = 0L;
	const X509_ALGOR *sig_alg = 0L;
	d->kossl->X509_get0_signature(&signature, &sig_alg, d->m_cert);
	i = d->kossl->OBJ_obj2nid(sig_alg->algorithm);
	rc = i18n("Signature Algorithm: ");
	rc += (i == NID_undef)?i18n("Unknown"):TQString(d->kossl->OBJ_nid2ln(i));

	rc += "\n";
	rc += i18n("Signature Contents:");
	n = signature->length;
	s = (char *)signature->data;
	for (i = 0; i < n; i++) {
		if (i%20 != 0) rc += ":";
		else rc += "\n";
		rc.append(hv[(s[i]&0xf0)>>4]);
		rc.append(hv[s[i]&0x0f]);
	}

#endif

return rc;
}


void KSSLCertificate::getEmails(TQStringList &to) const {
	to.clear();
#ifdef KSSL_HAVE_SSL
	if (!d->m_cert)
		return;
	
	STACK *s = d->kossl->X509_get1_email(d->m_cert);
	if (s) {
		for(int n=0; n < d->kossl->OPENSSL_sk_num(s); n++) {
			to.append(d->kossl->OPENSSL_sk_value(s,n));
		}
		d->kossl->X509_email_free(s);
	}
#endif	
}	


TQString KSSLCertificate::getKDEKey() const {
	return getSubject() + " (" + getMD5DigestText() + ")";
}


TQString KSSLCertificate::getMD5DigestFromKDEKey(const TQString &k) {
	TQString rc;
	int pos = k.findRev('(');
	if (pos != -1) {
		unsigned int len = k.length();
		if (k.at(len-1) == ')') {
			rc = k.mid(pos+1, len-pos-2);
		}
	}
	return rc;
}


TQString KSSLCertificate::getMD5DigestText() const {
TQString rc = "";

#ifdef KSSL_HAVE_SSL
	unsigned int n;
	unsigned char md[EVP_MAX_MD_SIZE];

	if (!d->kossl->X509_digest(d->m_cert, d->kossl->EVP_md5(), md, &n)) {
		return rc;
	}

	for (unsigned int j = 0; j < n; j++) {
		if (j > 0)
			rc += ":";
		rc.append(hv[(md[j]&0xf0)>>4]);
		rc.append(hv[md[j]&0x0f]);
	}

#endif

return rc;
}



TQString KSSLCertificate::getMD5Digest() const {
TQString rc = "";

#ifdef KSSL_HAVE_SSL
	unsigned int n;
	unsigned char md[EVP_MAX_MD_SIZE];

	if (!d->kossl->X509_digest(d->m_cert, d->kossl->EVP_md5(), md, &n)) {
		return rc;
	}

	for (unsigned int j = 0; j < n; j++) {
		rc.append(hv[(md[j]&0xf0)>>4]);
		rc.append(hv[md[j]&0x0f]);
	}

#endif

return rc;
}



TQString KSSLCertificate::getKeyType() const {
TQString rc = "";

#ifdef KSSL_HAVE_SSL
	EVP_PKEY *pkey = d->kossl->X509_get_pubkey(d->m_cert);
	if (pkey) {
		#ifndef NO_RSA
			if (d->kossl->EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA)
				rc = "RSA";
			else
		#endif
		#ifndef NO_DSA
			if (d->kossl->EVP_PKEY_base_id(pkey) == EVP_PKEY_DSA)
				rc = "DSA";
			else
		#endif
				rc = "Unknown";
		d->kossl->EVP_PKEY_free(pkey);
	}
#endif

return rc;
}



TQString KSSLCertificate::getPublicKeyText() const {
TQString rc = "";
char *x = NULL;

#ifdef KSSL_HAVE_SSL
	EVP_PKEY *pkey = d->kossl->X509_get_pubkey(d->m_cert);
	if (pkey) {
		rc = i18n("Unknown", "Unknown key algorithm");
		#ifndef NO_RSA
			if (d->kossl->EVP_PKEY_base_id(pkey) == EVP_PKEY_RSA) {
				rc = i18n("Key type: RSA (%1 bit)") + "\n";

				RSA *pkey_rsa = d->kossl->EVP_PKEY_get0_RSA(pkey);
				const BIGNUM *bn_n = 0L;
				const BIGNUM *bn_e = 0L;
				d->kossl->RSA_get0_key(pkey_rsa, &bn_n, &bn_e, NULL);
				x = d->kossl->BN_bn2hex(bn_n);
				rc += i18n("Modulus: ");
				rc = rc.arg(strlen(x)*4);
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->CRYPTO_free(x);

				x = d->kossl->BN_bn2hex(bn_e);
				rc += i18n("Exponent: 0x") + x + "\n";
				d->kossl->CRYPTO_free(x);
			}
		#endif
		#ifndef NO_DSA
			if (d->kossl->EVP_PKEY_base_id(pkey) == EVP_PKEY_DSA) {
				rc = i18n("Key type: DSA (%1 bit)") + "\n";

				DSA *pkey_dsa = d->kossl->EVP_PKEY_get0_DSA(pkey);
				const BIGNUM *bn_p = 0L;
				const BIGNUM *bn_q = 0L;
				const BIGNUM *bn_g = 0L;
				const BIGNUM *bn_pub_key = 0L;
				d->kossl->DSA_get0_pqg(pkey_dsa, &bn_p, &bn_q, &bn_g);
				d->kossl->DSA_get0_key(pkey_dsa, &bn_pub_key, NULL);

				x = d->kossl->BN_bn2hex(bn_p);
				rc += i18n("Prime: ");
				// hack - this may not be always accurate
				rc = rc.arg(strlen(x)*4) ;
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->CRYPTO_free(x);

				x = d->kossl->BN_bn2hex(bn_q);
				rc += i18n("160 bit prime factor: ");
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->CRYPTO_free(x);
	
				x = d->kossl->BN_bn2hex(bn_g);
				rc += TQString("g: ");
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->CRYPTO_free(x);
	
				x = d->kossl->BN_bn2hex(bn_pub_key);
				rc += i18n("Public key: ");
				for (unsigned int i = 0; i < strlen(x); i++) {
					if (i%40 != 0 && i%2 == 0)
						rc += ":";
					else if (i%40 == 0)
						rc += "\n";
					rc += x[i];
				}
				rc += "\n";
				d->kossl->CRYPTO_free(x);
			}
		#endif
		d->kossl->EVP_PKEY_free(pkey);
	}
#endif

return rc;
}



TQString KSSLCertificate::getIssuer() const {
TQString rc = "";

#ifdef KSSL_HAVE_SSL
	char *t = d->kossl->X509_NAME_oneline(d->kossl->X509_get_issuer_name(d->m_cert), 0, 0);

	if (!t)
		return rc;

	rc = t;
	d->kossl->CRYPTO_free(t);
#endif

return rc;
}

void KSSLCertificate::setChain(void *c) {
#ifdef KSSL_HAVE_SSL
	d->_chain.setChain(c);
#endif
	d->m_stateCached = false;
	d->m_stateCache = KSSLCertificate::Unknown;
}

void KSSLCertificate::setCert(X509 *c) {
#ifdef KSSL_HAVE_SSL
d->m_cert = c;
if (c) {
  	d->_extensions.flags = 0;
	d->kossl->X509_check_purpose(c, -1, 0);    // setup the fields (!!)

#if 0
	kdDebug(7029) << "---------------- Certificate ------------------" 
		      << endl;
	kdDebug(7029) << getSubject() << endl;
#endif

	for (int j = 0; j < d->kossl->X509_PURPOSE_get_count(); j++) {
		X509_PURPOSE *ptmp = d->kossl->X509_PURPOSE_get0(j);
		int id = d->kossl->X509_PURPOSE_get_id(ptmp);
		for (int ca = 0; ca < 2; ca++) {
			int idret = d->kossl->X509_check_purpose(c, id, ca);
			if (idret == 1 || idret == 2) {   // have it
//				kdDebug() << "PURPOSE: " << id << (ca?" CA":"") << endl;
				if (!ca)
					d->_extensions.flags |= (1L <<(id-1));
				else d->_extensions.flags |= (1L <<(16+id-1));
			} else {
				if (!ca)
					d->_extensions.flags &= ~(1L <<(id-1));
				else d->_extensions.flags &= ~(1L <<(16+id-1));
			}
		}
	}

#if 0
	kdDebug(7029) << "flags: " << TQString::number(c->ex_flags, 2)
		      << "\nkeyusage: " << TQString::number(c->ex_kusage, 2)
		      << "\nxkeyusage: " << TQString::number(c->ex_xkusage, 2)
		      << "\nnscert: " << TQString::number(c->ex_nscert, 2)
		      << endl;
	if (c->ex_flags & EXFLAG_KUSAGE)
		kdDebug(7029) << "     --- Key Usage extensions found" << endl;
        else kdDebug(7029) << "     --- Key Usage extensions NOT found" << endl;

	if (c->ex_flags & EXFLAG_XKUSAGE)
		kdDebug(7029) << "     --- Extended key usage extensions found" << endl;
        else kdDebug(7029) << "     --- Extended key usage extensions NOT found" << endl;

	if (c->ex_flags & EXFLAG_NSCERT)
		kdDebug(7029) << "     --- NS extensions found" << endl;
        else kdDebug(7029) << "     --- NS extensions NOT found" << endl;

        if (d->_extensions.certTypeSSLCA())
                kdDebug(7029) << "NOTE: this is an SSL CA file." << endl;
        else kdDebug(7029) << "NOTE: this is NOT an SSL CA file." << endl;

        if (d->_extensions.certTypeEmailCA())
                kdDebug(7029) << "NOTE: this is an EMAIL CA file." << endl;
        else kdDebug(7029) << "NOTE: this is NOT an EMAIL CA file." << endl;

        if (d->_extensions.certTypeCodeCA())
                kdDebug(7029) << "NOTE: this is a CODE CA file." << endl;
        else kdDebug(7029) << "NOTE: this is NOT a CODE CA file." << endl;

        if (d->_extensions.certTypeSSLClient())
                kdDebug(7029) << "NOTE: this is an SSL client." << endl;
        else kdDebug(7029) << "NOTE: this is NOT an SSL client." << endl;

        if (d->_extensions.certTypeSSLServer())
                kdDebug(7029) << "NOTE: this is an SSL server." << endl;
        else kdDebug(7029) << "NOTE: this is NOT an SSL server." << endl;

        if (d->_extensions.certTypeNSSSLServer())
                kdDebug(7029) << "NOTE: this is a NETSCAPE SSL server." << endl;
        else kdDebug(7029) << "NOTE: this is NOT a NETSCAPE SSL server." << endl;

        if (d->_extensions.certTypeSMIME())
                kdDebug(7029) << "NOTE: this is an SMIME certificate." << endl;
        else kdDebug(7029) << "NOTE: this is NOT an SMIME certificate." << endl;

        if (d->_extensions.certTypeSMIMEEncrypt())
                kdDebug(7029) << "NOTE: this is an SMIME encrypt cert." << endl;
        else kdDebug(7029) << "NOTE: this is NOT an SMIME encrypt cert." << endl;

        if (d->_extensions.certTypeSMIMESign())
                kdDebug(7029) << "NOTE: this is an SMIME sign cert." << endl;
        else kdDebug(7029) << "NOTE: this is NOT an SMIME sign cert." << endl;

        if (d->_extensions.certTypeCRLSign())
                kdDebug(7029) << "NOTE: this is a CRL signer." << endl;
        else kdDebug(7029) << "NOTE: this is NOT a CRL signer." << endl;

	kdDebug(7029) << "-----------------------------------------------" 
		      << endl;
#endif
}
#endif
d->m_stateCached = false;
d->m_stateCache = KSSLCertificate::Unknown;
}

void KSSLCertificate::setCRL(X509_CRL *c) {
#ifdef KSSL_HAVE_SSL
d->m_cert_crl = c;
if (c) {
  	d->_extensions.flags = 0;
}
#endif
d->m_stateCached = false;
d->m_stateCache = KSSLCertificate::Unknown;
}

X509 *KSSLCertificate::getCert() {
#ifdef KSSL_HAVE_SSL
	return d->m_cert;
#endif
return 0;
}

// pull in the callback.  It's common across multiple files but we want
// it to be hidden.

#include "ksslcallback.c"


bool KSSLCertificate::isValid(KSSLCertificate::KSSLPurpose p) {
	return (validate(p) == KSSLCertificate::Ok);
}


bool KSSLCertificate::isValid() {
	return isValid(KSSLCertificate::SSLServer);
}


int KSSLCertificate::purposeToOpenSSL(KSSLCertificate::KSSLPurpose p) const {
int rc = 0;
#ifdef KSSL_HAVE_SSL
	if (p == KSSLCertificate::SSLServer) {
		rc = X509_PURPOSE_SSL_SERVER;
	} else if (p == KSSLCertificate::SSLClient) {
		rc = X509_PURPOSE_SSL_CLIENT;
	} else if (p == KSSLCertificate::SMIMEEncrypt) {
		rc = X509_PURPOSE_SMIME_ENCRYPT;
	} else if (p == KSSLCertificate::SMIMESign) {
		rc = X509_PURPOSE_SMIME_SIGN;
	} else if (p == KSSLCertificate::Any) {
		rc = X509_PURPOSE_ANY;
	}
#endif
return rc;	
}


// For backward compatibility
KSSLCertificate::KSSLValidation KSSLCertificate::validate() {
	return validate(KSSLCertificate::SSLServer);
}

KSSLCertificate::KSSLValidation KSSLCertificate::validate(KSSLCertificate::KSSLPurpose purpose)
{
	KSSLValidationList result = validateVerbose(purpose);
	if (result.isEmpty())
		return KSSLCertificate::Ok;
	else
		return result.first();
} 

//
// See apps/verify.c in OpenSSL for the source of most of this logic.
//

// CRL files?  we don't do that yet
KSSLCertificate::KSSLValidationList KSSLCertificate::validateVerbose(KSSLCertificate::KSSLPurpose purpose) 
{
	return validateVerbose(purpose, 0);
}

KSSLCertificate::KSSLValidationList KSSLCertificate::validateVerbose(KSSLCertificate::KSSLPurpose purpose, KSSLCertificate *ca)
{
	KSSLValidationList errors;
	if (ca || (d->_lastPurpose != purpose)) {
		d->m_stateCached = false;
	}

	if (!d->m_stateCached)
		d->_lastPurpose = purpose;

#ifdef KSSL_HAVE_SSL
	X509_STORE *certStore;
	X509_LOOKUP *certLookup;
	X509_STORE_CTX *certStoreCTX;

	if (!d->m_cert)
	{
		errors << KSSLCertificate::Unknown;
		return errors;
	}

	if (d->m_stateCached) {
		errors << d->m_stateCache;
		return errors;
	}

	TQStringList qsl = TDEGlobal::dirs()->resourceDirs("kssl");

	if (qsl.isEmpty()) {
		errors << KSSLCertificate::NoCARoot;
		return errors;
	}

	KSSLCertificate::KSSLValidation ksslv = Unknown;

	for (TQStringList::Iterator j = qsl.begin(); j != qsl.end(); ++j) {
		struct stat sb;
		TQString _j = (*j) + "ca-bundle.crt";
		if (-1 == stat(_j.ascii(), &sb)) {
			continue;
		}

		certStore = d->kossl->X509_STORE_new();
		if (!certStore) {
			errors << KSSLCertificate::Unknown;
			return errors;
		}

		d->kossl->X509_STORE_set_verify_cb(certStore, X509Callback);

		certLookup = d->kossl->X509_STORE_add_lookup(certStore, d->kossl->X509_LOOKUP_file());
		if (!certLookup) {
			ksslv = KSSLCertificate::Unknown;
			d->kossl->X509_STORE_free(certStore);
			continue;
		}

		if (!d->kossl->X509_LOOKUP_load_file(certLookup, _j.ascii(), X509_FILETYPE_PEM)) {
			// error accessing directory and loading pems
			kdDebug(7029) << "KSSL couldn't read CA root: " 
					<< _j << endl;
			ksslv = KSSLCertificate::ErrorReadingRoot;
			d->kossl->X509_STORE_free(certStore);
			continue;
		}

		// This is the checking code
		certStoreCTX = d->kossl->X509_STORE_CTX_new();

		// this is a bad error - could mean no free memory.
		// This may be the wrong thing to do here
		if (!certStoreCTX) {
			kdDebug(7029) << "KSSL couldn't create an X509 store context." << endl;
			d->kossl->X509_STORE_free(certStore);
			continue;
		}

		d->kossl->X509_STORE_CTX_init(certStoreCTX, certStore, d->m_cert, NULL);
		if (d->_chain.isValid()) {
			d->kossl->X509_STORE_CTX_set0_untrusted(certStoreCTX, (STACK_OF(X509)*)d->_chain.rawChain());
		}

		//kdDebug(7029) << "KSSL setting CRL.............." << endl;
		// int X509_STORE_add_crl(X509_STORE *ctx, X509_CRL *x);

		d->kossl->X509_STORE_CTX_set_purpose(certStoreCTX, purposeToOpenSSL(purpose));

		KSSL_X509CallBack_ca = ca ? ca->d->m_cert : 0;
		KSSL_X509CallBack_ca_found = false;

		d->kossl->X509_STORE_CTX_set_error(certStoreCTX, X509_V_OK);
		d->kossl->X509_verify_cert(certStoreCTX);
		int errcode = d->kossl->X509_STORE_CTX_get_error(certStoreCTX);
		if (ca && !KSSL_X509CallBack_ca_found) {
			ksslv = KSSLCertificate::Irrelevant;
		} else {
			ksslv = processError(errcode);
		}
		// For servers, we can try NS_SSL_SERVER too
		if (	(ksslv != KSSLCertificate::Ok) &&
			(ksslv != KSSLCertificate::Irrelevant) &&
			purpose == KSSLCertificate::SSLServer) {
			d->kossl->X509_STORE_CTX_set_purpose(certStoreCTX,
						X509_PURPOSE_NS_SSL_SERVER);

			d->kossl->X509_STORE_CTX_set_error(certStoreCTX, X509_V_OK);
			d->kossl->X509_verify_cert(certStoreCTX);
			errcode = d->kossl->X509_STORE_CTX_get_error(certStoreCTX);
			ksslv = processError(errcode);
		}
		d->kossl->X509_STORE_CTX_free(certStoreCTX);
		d->kossl->X509_STORE_free(certStore);
		// end of checking code
		//

		//kdDebug(7029) << "KSSL Validation procedure RC: " 
		//		<< rc << endl;
		//kdDebug(7029) << "KSSL Validation procedure errcode: "
		//		<< errcode << endl;
		//kdDebug(7029) << "KSSL Validation procedure RESULTS: "
		//		<< ksslv << endl;

		if (ksslv != NoCARoot && ksslv != InvalidCA) {
			d->m_stateCached = true;
			d->m_stateCache = ksslv;
		}
		break;
	}
	
	if (ksslv != KSSLCertificate::Ok)
		errors << ksslv;
#else
	errors << KSSLCertificate::NoSSL;
#endif
	return errors;
}



KSSLCertificate::KSSLValidation KSSLCertificate::revalidate() {
	return revalidate(KSSLCertificate::SSLServer);
}


KSSLCertificate::KSSLValidation KSSLCertificate::revalidate(KSSLCertificate::KSSLPurpose p) {
	d->m_stateCached = false;
	return validate(p);
}


KSSLCertificate::KSSLValidation KSSLCertificate::processError(int ec) {
KSSLCertificate::KSSLValidation rc;

rc = KSSLCertificate::Unknown;
#ifdef KSSL_HAVE_SSL
	switch (ec) {
	case X509_V_OK:       // OK
		rc = KSSLCertificate::Ok;
	break;


	case X509_V_ERR_CERT_REJECTED:
		rc = KSSLCertificate::Rejected;
	break;


	case X509_V_ERR_CERT_UNTRUSTED:
		rc = KSSLCertificate::Untrusted;
	break;


	case X509_V_ERR_UNABLE_TO_VERIFY_LEAF_SIGNATURE:
	case X509_V_ERR_CERT_SIGNATURE_FAILURE:
	case X509_V_ERR_CRL_SIGNATURE_FAILURE:
	case X509_V_ERR_UNABLE_TO_DECRYPT_CERT_SIGNATURE:
	case X509_V_ERR_UNABLE_TO_DECRYPT_CRL_SIGNATURE:
		rc = KSSLCertificate::SignatureFailed;
	break;

	case X509_V_ERR_INVALID_CA:
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT:
	case X509_V_ERR_UNABLE_TO_DECODE_ISSUER_PUBLIC_KEY:
	case X509_V_ERR_UNABLE_TO_GET_ISSUER_CERT_LOCALLY:
		rc = KSSLCertificate::InvalidCA;
	break;


	case X509_V_ERR_INVALID_PURPOSE:
		rc = KSSLCertificate::InvalidPurpose;
	break;


	case X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT:
		rc = KSSLCertificate::SelfSigned;
	break;

	case X509_V_ERR_SELF_SIGNED_CERT_IN_CHAIN:
		rc = KSSLCertificate::SelfSignedChain;
	break;

	case X509_V_ERR_CERT_REVOKED:
		rc = KSSLCertificate::Revoked;
	break;

	case X509_V_ERR_PATH_LENGTH_EXCEEDED:
		rc = KSSLCertificate::PathLengthExceeded;
	break;

	case X509_V_ERR_CERT_NOT_YET_VALID:
	case X509_V_ERR_CERT_HAS_EXPIRED:
	case X509_V_ERR_CRL_NOT_YET_VALID:
	case X509_V_ERR_CRL_HAS_EXPIRED:
	case X509_V_ERR_ERROR_IN_CERT_NOT_BEFORE_FIELD:
	case X509_V_ERR_ERROR_IN_CERT_NOT_AFTER_FIELD:
	case X509_V_ERR_ERROR_IN_CRL_LAST_UPDATE_FIELD:
	case X509_V_ERR_ERROR_IN_CRL_NEXT_UPDATE_FIELD:
		rc = KSSLCertificate::Expired;
		kdDebug(7029) << "KSSL apparently this is expired.  Not after: "
				<< getNotAfter() << endl;
	break;

	//case 1:
	case X509_V_ERR_APPLICATION_VERIFICATION:
	case X509_V_ERR_OUT_OF_MEM:
	case X509_V_ERR_UNABLE_TO_GET_CRL:
	case X509_V_ERR_CERT_CHAIN_TOO_LONG:
	default:
		rc = KSSLCertificate::Unknown;
	break;
}

d->m_stateCache = rc;
d->m_stateCached = true;
#endif
return rc;
}


TQString KSSLCertificate::getNotBefore() const {
#ifdef KSSL_HAVE_SSL
return ASN1_UTCTIME_QString(d->kossl->X509_getm_notBefore(d->m_cert));
#else
return TQString::null;
#endif
}


TQString KSSLCertificate::getNotAfter() const {
#ifdef KSSL_HAVE_SSL
return ASN1_UTCTIME_QString(d->kossl->X509_getm_notAfter(d->m_cert));
#else
return TQString::null;
#endif
}


TQDateTime KSSLCertificate::getQDTNotBefore() const {
#ifdef KSSL_HAVE_SSL
return ASN1_UTCTIME_QDateTime(d->kossl->X509_getm_notBefore(d->m_cert), NULL);
#else
return TQDateTime::currentDateTime();
#endif
}


TQDateTime KSSLCertificate::getQDTNotAfter() const {
#ifdef KSSL_HAVE_SSL
return ASN1_UTCTIME_QDateTime(d->kossl->X509_getm_notAfter(d->m_cert), NULL);
#else
return TQDateTime::currentDateTime();
#endif
}


TQDateTime KSSLCertificate::getQDTLastUpdate() const {
#ifdef KSSL_HAVE_SSL
return ASN1_UTCTIME_QDateTime((ASN1_UTCTIME*)d->kossl->X509_CRL_get0_lastUpdate(d->m_cert_crl), NULL);
#else
return TQDateTime::currentDateTime();
#endif
}


TQDateTime KSSLCertificate::getQDTNextUpdate() const {
#ifdef KSSL_HAVE_SSL
return ASN1_UTCTIME_QDateTime((ASN1_UTCTIME*)d->kossl->X509_CRL_get0_nextUpdate(d->m_cert_crl), NULL);
#else
return TQDateTime::currentDateTime();
#endif
}


int operator==(KSSLCertificate &x, KSSLCertificate &y) {
#ifndef KSSL_HAVE_SSL
  return 1;
#else
  if (!KOSSL::self()->X509_cmp(x.getCert(), y.getCert())) return 1;
  return 0;
#endif
}


KSSLCertificate *KSSLCertificate::replicate() {
// The new certificate doesn't have the cached value.  It's probably
// better this way.  We can't anticipate every reason for doing this.
KSSLCertificate *newOne = new KSSLCertificate();
#ifdef KSSL_HAVE_SSL
	newOne->setCert(d->kossl->X509_dup(getCert()));
	KSSLCertChain *c = d->_chain.replicate();
	newOne->setChain(c->rawChain());
	delete c;
#endif
return newOne;
}


TQString KSSLCertificate::toString() {
return KCodecs::base64Encode(toDer());
}


TQString KSSLCertificate::verifyText(KSSLValidation x) {
switch (x) {
case KSSLCertificate::Ok:
	return i18n("The certificate is valid.");
case KSSLCertificate::PathLengthExceeded:
case KSSLCertificate::ErrorReadingRoot:
case KSSLCertificate::NoCARoot:
	return i18n("Certificate signing authority root files could not be found so the certificate is not verified.");
case KSSLCertificate::SelfSignedChain:
case KSSLCertificate::InvalidCA:
	return i18n("Certificate signing authority is unknown or invalid.");
case KSSLCertificate::SelfSigned:
	return i18n("Certificate is self-signed and thus may not be trustworthy.");
case KSSLCertificate::Expired:
	return i18n("Certificate has expired.");
case KSSLCertificate::Revoked:
	return i18n("Certificate has been revoked.");
case KSSLCertificate::NoSSL:
	return i18n("SSL support was not found.");
case KSSLCertificate::Untrusted:
	return i18n("Signature is untrusted.");
case KSSLCertificate::SignatureFailed:
	return i18n("Signature test failed.");
case KSSLCertificate::Rejected:
case KSSLCertificate::InvalidPurpose:
	return i18n("Rejected, possibly due to an invalid purpose.");
case KSSLCertificate::PrivateKeyFailed:
	return i18n("Private key test failed.");
case KSSLCertificate::InvalidHost:
	return i18n("The certificate has not been issued for this host.");
case KSSLCertificate::Irrelevant:
	return i18n("This certificate is not relevant.");
default:
break;
}

return i18n("The certificate is invalid.");
}


TQByteArray KSSLCertificate::toDer() {
TQByteArray qba;
#ifdef KSSL_HAVE_SSL
unsigned int certlen = d->kossl->i2d_X509(getCert(), NULL);
unsigned char *cert = new unsigned char[certlen];
unsigned char *p = cert;
	// FIXME: return code!
	d->kossl->i2d_X509(getCert(), &p);

	// encode it into a TQString
	qba.duplicate((const char*)cert, certlen);
	delete[] cert;
#endif
return qba;
}



TQByteArray KSSLCertificate::toPem() {
TQByteArray qba;
TQString thecert = toString();
const char *header = "-----BEGIN CERTIFICATE-----\n";
const char *footer = "-----END CERTIFICATE-----\n";

	// We just do base64 on the ASN1
	//  64 character lines  (unpadded)
	unsigned int xx = thecert.length() - 1;
	for (unsigned int i = 0; i < xx/64; i++) {
		thecert.insert(64*(i+1)+i, '\n');
	}

	thecert.prepend(header);

	if (thecert[thecert.length()-1] != '\n')
		thecert += "\n";

	thecert.append(footer);

	qba.duplicate(thecert.local8Bit(), thecert.length());
return qba;
}


#define NETSCAPE_CERT_HDR     "certificate"
#ifdef KSSL_HAVE_SSL
#if OPENSSL_VERSION_NUMBER >= 0x10100000L && !defined(LIBRESSL_VERSION_NUMBER)
typedef struct NETSCAPE_X509_st
{
	ASN1_OCTET_STRING *header;
	X509 *cert;
} NETSCAPE_X509;
#endif
#endif

// what a piece of crap this is
TQByteArray KSSLCertificate::toNetscape() {
TQByteArray qba;
#ifdef KSSL_HAVE_SSL
#if OPENSSL_VERSION_NUMBER >= 0x10000000L
	NETSCAPE_X509 nx;
	ASN1_OCTET_STRING hdr;
#else
  ASN1_HEADER ah;
  ASN1_OCTET_STRING os;
#endif
	KTempFile ktf;

#if OPENSSL_VERSION_NUMBER >= 0x10000000L
	hdr.data = (unsigned char *)NETSCAPE_CERT_HDR;
	hdr.length = strlen(NETSCAPE_CERT_HDR);
	nx.header = &hdr;
	nx.cert = getCert();

	d->kossl->ASN1_i2d_fp(ktf.fstream(),(unsigned char *)&nx);
#else
   os.data = (unsigned char *)NETSCAPE_CERT_HDR;
   os.length = strlen(NETSCAPE_CERT_HDR);
   ah.header = &os;
   ah.data = (char *)getCert();
   ah.meth = d->kossl->X509_asn1_meth();

   d->kossl->ASN1_i2d_fp(ktf.fstream(),(unsigned char *)&ah);
#endif

	ktf.close();

	TQFile qf(ktf.name());
	qf.open(IO_ReadOnly);
	char *buf = new char[qf.size()];
	qf.readBlock(buf, qf.size());
	qba.duplicate(buf, qf.size());
	qf.close();
	delete[] buf;

	ktf.unlink();

#endif
return qba;
}



TQString KSSLCertificate::toText() {
TQString text;
#ifdef KSSL_HAVE_SSL
KTempFile ktf;

	d->kossl->X509_print(ktf.fstream(), getCert());
	ktf.close();

	TQFile qf(ktf.name());
	qf.open(IO_ReadOnly);
	char *buf = new char[qf.size()+1];
	qf.readBlock(buf, qf.size());
	buf[qf.size()] = 0;
	text = buf;
	delete[] buf;
	qf.close();
	ktf.unlink();
#endif
return text;
}

// KDE 4: Make it const TQString &
bool KSSLCertificate::setCert(TQString& cert) {
#ifdef KSSL_HAVE_SSL
TQByteArray qba, qbb = cert.local8Bit().copy();
	KCodecs::base64Decode(qbb, qba);
	unsigned char *qbap = reinterpret_cast<unsigned char *>(qba.data());
	X509 *x5c = KOSSL::self()->d2i_X509(NULL, &qbap, qba.size());
	if (x5c) {
		setCert(x5c);
		return true;
	}
#endif
return false;
}


KSSLX509V3& KSSLCertificate::x509V3Extensions() {
return d->_extensions;
}


bool KSSLCertificate::isSigner() {
return d->_extensions.certTypeCA();
}


TQStringList KSSLCertificate::subjAltNames() const {
	TQStringList rc;
#ifdef KSSL_HAVE_SSL
	STACK_OF(GENERAL_NAME) *names;
	names = (STACK_OF(GENERAL_NAME)*)d->kossl->X509_get_ext_d2i(d->m_cert, NID_subject_alt_name, 0, 0);

	if (!names) {
		return rc;
	}

	int cnt = d->kossl->OPENSSL_sk_num(names);

	for (int i = 0; i < cnt; i++) {
		const GENERAL_NAME *val = (const GENERAL_NAME *)d->kossl->OPENSSL_sk_value(names, i);
		if (val->type != GEN_DNS) {
			continue;
		}

		TQString s = (const char *)d->kossl->ASN1_STRING_data(val->d.ia5);
		if (!s.isEmpty()  &&
				/* skip subjectAltNames with embedded NULs */
				s.length() == (unsigned int)d->kossl->ASN1_STRING_length(val->d.ia5)) {
			rc += s;
		}
	}
	d->kossl->OPENSSL_sk_free(names);
#endif
	return rc;
}


TQDataStream& operator<<(TQDataStream& s, const KSSLCertificate& r) {
TQStringList qsl;
TQPtrList<KSSLCertificate> cl = const_cast<KSSLCertificate&>(r).chain().getChain();

	for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
		qsl << c->toString();
	}

	cl.setAutoDelete(true);

	s << const_cast<KSSLCertificate&>(r).toString() << qsl;

return s;
}


TQDataStream& operator>>(TQDataStream& s, KSSLCertificate& r) {
TQStringList qsl;
TQString cert;

s >> cert >> qsl;

	if (r.setCert(cert) && !qsl.isEmpty())
		r.chain().setCertChain(qsl);

return s;
}



