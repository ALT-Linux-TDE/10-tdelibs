/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Stefan Rompf <sux@loplof.de>
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


#include <tqptrlist.h>
#include <tqcstring.h>
#include <tqstring.h>
#include <kdebug.h>

#include "kopenssl.h"
#include "ksslcertificate.h"
#include "ksslpkcs12.h"
#include "ksmimecrypto.h"

// this hack provided by Malte Starostik to avoid glibc/openssl bug
// on some systems
#ifdef KSSL_HAVE_SSL
#define crypt _openssl_crypt
#include <openssl/err.h>
#undef crypt
#endif


#ifdef KSSL_HAVE_SSL
static const char eot = 0;

class KSMIMECryptoPrivate {
    KOpenSSLProxy *kossl;

public:
    KSMIMECryptoPrivate(KOpenSSLProxy *kossl);


    STACK_OF(X509) *certsToX509(TQPtrList<KSSLCertificate> &certs);

    KSMIMECrypto::rc signMessage(BIO *clearText,
				 BIO *cipherText,
				 KSSLPKCS12 &privKey, TQPtrList<KSSLCertificate> &certs,
				 bool detached);

    KSMIMECrypto::rc encryptMessage(BIO *clearText,
				    BIO *cipherText, KSMIMECrypto::algo algorithm,
				    TQPtrList<KSSLCertificate> &recip);

    KSMIMECrypto::rc checkSignature(BIO *clearText,
				    BIO *signature, bool detached,
				    TQPtrList<KSSLCertificate> &recip);
    
    KSMIMECrypto::rc decryptMessage(BIO *cipherText,
				    BIO *clearText,
				    KSSLPKCS12 &privKey);
    
    void MemBIOToQByteArray(BIO *src, TQByteArray &dest);

    KSMIMECrypto::rc sslErrToRc(void);
};


KSMIMECryptoPrivate::KSMIMECryptoPrivate(KOpenSSLProxy *kossl): kossl(kossl) {
}


STACK_OF(X509) *KSMIMECryptoPrivate::certsToX509(TQPtrList<KSSLCertificate> &certs) {
    STACK_OF(X509) *x509 = reinterpret_cast<STACK_OF(X509)*>(kossl->OPENSSL_sk_new(NULL));
    KSSLCertificate *cert = certs.first();
    while(cert) {
	kossl->OPENSSL_sk_push(x509, cert->getCert());
	cert = certs.next();
    }
    return x509;
}


KSMIMECrypto::rc KSMIMECryptoPrivate::signMessage(BIO *clearText,
						  BIO *cipherText,
						  KSSLPKCS12 &privKey, TQPtrList<KSSLCertificate> &certs,
						  bool detached) {

    STACK_OF(X509) *other = NULL;
    KSMIMECrypto::rc rc;
    int flags = detached?PKCS7_DETACHED:0;

    if (certs.count()) other = certsToX509(certs);

    PKCS7 *p7 = kossl->PKCS7_sign(privKey.getCertificate()->getCert(), privKey.getPrivateKey(),
				  other, clearText, flags);

    if (other) kossl->OPENSSL_sk_free(other);

    if (!p7) return sslErrToRc();

    if (kossl->i2d_PKCS7_bio(cipherText, p7)) {
	rc = KSMIMECrypto::KSC_R_OK;
    } else {
	rc = sslErrToRc();
    }

    kossl->PKCS7_free(p7);

    return rc;
}

KSMIMECrypto::rc KSMIMECryptoPrivate::encryptMessage(BIO *clearText,
						     BIO *cipherText, KSMIMECrypto::algo algorithm,
						     TQPtrList<KSSLCertificate> &recip) {
    EVP_CIPHER *cipher = NULL;
    KSMIMECrypto::rc rc;
    switch(algorithm) {
	case KSMIMECrypto::KSC_C_DES3_CBC:
	    cipher = kossl->EVP_des_ede3_cbc();
	    break;
	case KSMIMECrypto::KSC_C_RC2_CBC_128:
	    cipher = kossl->EVP_rc2_cbc();
	    break;
	case KSMIMECrypto::KSC_C_RC2_CBC_64:
	    cipher = kossl->EVP_rc2_64_cbc();
	    break;
	case KSMIMECrypto::KSC_C_DES_CBC:
	    cipher = kossl->EVP_des_cbc();
	    break;
	case KSMIMECrypto::KSC_C_RC2_CBC_40:
	    cipher = kossl->EVP_rc2_40_cbc();
	    break;
    }
    if (!cipher) return KSMIMECrypto::KSC_R_NOCIPHER;

    STACK_OF(X509) *certs = certsToX509(recip);

    PKCS7 *p7 = kossl->PKCS7_encrypt(certs, clearText, cipher, 0);

    kossl->OPENSSL_sk_free(certs);

    if (!p7) return sslErrToRc();

    if (kossl->i2d_PKCS7_bio(cipherText, p7)) {
	rc = KSMIMECrypto::KSC_R_OK;
    } else {
	rc = sslErrToRc();
    }

    kossl->PKCS7_free(p7);

    return rc;
}


KSMIMECrypto::rc KSMIMECryptoPrivate::checkSignature(BIO *clearText,
						     BIO *signature, bool detached,
						     TQPtrList<KSSLCertificate> &recip) {
    
    PKCS7 *p7 = kossl->d2i_PKCS7_bio(signature, NULL);
    KSMIMECrypto::rc rc = KSMIMECrypto::KSC_R_OTHER;

    if (!p7) return sslErrToRc();

    BIO *in;
    BIO *out;
    if (detached) {
	in = clearText;
	out = NULL;
    } else {
	in = NULL;
	out = clearText;
    }

    X509_STORE *dummystore = kossl->X509_STORE_new();
    if (kossl->PKCS7_verify(p7, NULL, dummystore, in, out, PKCS7_NOVERIFY)) {
	STACK_OF(X509) *signers = kossl->PKCS7_get0_signers(p7, 0, PKCS7_NOVERIFY);
	int num = kossl->OPENSSL_sk_num(signers);

	for(int n=0; n<num; n++) {
	    KSSLCertificate *signer = KSSLCertificate::fromX509(reinterpret_cast<X509*>(kossl->OPENSSL_sk_value(signers, n)));
	    recip.append(signer);
	}

	kossl->OPENSSL_sk_free(signers);
	rc = KSMIMECrypto::KSC_R_OK;
    } else {
	rc = sslErrToRc();
    }

    kossl->X509_STORE_free(dummystore);
    kossl->PKCS7_free(p7);

    return rc;
}


KSMIMECrypto::rc KSMIMECryptoPrivate::decryptMessage(BIO *cipherText,
						     BIO *clearText,
						     KSSLPKCS12 &privKey) {
    
    PKCS7 *p7 = kossl->d2i_PKCS7_bio(cipherText, NULL);
    KSMIMECrypto::rc rc;

    if (!p7) return sslErrToRc();

    if (kossl->PKCS7_decrypt(p7, privKey.getPrivateKey(), privKey.getCertificate()->getCert(), 
			     clearText, 0)) {
	rc = KSMIMECrypto::KSC_R_OK;
    } else {
	rc = sslErrToRc();
    }

    kossl->PKCS7_free(p7);

    return rc;
}


void KSMIMECryptoPrivate::MemBIOToQByteArray(BIO *src, TQByteArray &dest) {
    char *buf;
    long len = kossl->BIO_get_mem_data(src, &buf);
    dest.assign(buf, len);
    /* Now this goes quite a bit into openssl internals.
       We assume that openssl uses malloc() (it does in
       default config) and rip out the buffer.
    */
    void *ptr = kossl->BIO_get_data(src);
    reinterpret_cast<BUF_MEM *>(ptr)->data = NULL;
}

    
KSMIMECrypto::rc KSMIMECryptoPrivate::sslErrToRc(void) {
    unsigned long cerr = kossl->ERR_get_error();

    // To be completed and possibly fixed

    switch(ERR_GET_REASON(cerr)) {
	case ERR_R_MALLOC_FAILURE:
	    return KSMIMECrypto::KSC_R_NOMEM;
    }

    switch(ERR_GET_LIB(cerr)) {
	case ERR_LIB_PKCS7:
	    switch(ERR_GET_REASON(cerr)) {	
		case PKCS7_R_WRONG_CONTENT_TYPE:
		case PKCS7_R_NO_CONTENT:
		case PKCS7_R_NO_SIGNATURES_ON_DATA:
		    return KSMIMECrypto::KSC_R_FORMAT;
		    break;
		case PKCS7_R_PRIVATE_KEY_DOES_NOT_MATCH_CERTIFICATE:
		case PKCS7_R_DECRYPT_ERROR: // Hmm?
		    return KSMIMECrypto::KSC_R_WRONGKEY;
		    break;
		case PKCS7_R_DIGEST_FAILURE:
		    return KSMIMECrypto::KSC_R_VERIFY;
		default:
		    break;
	    }
	    break;
	default:
	    break;
    }

    kdDebug(7029) <<"KSMIMECrypto: uncaught error " <<ERR_GET_LIB(cerr)
		  <<" " <<ERR_GET_REASON(cerr) <<endl;
    return KSMIMECrypto::KSC_R_OTHER;
}    
#endif


KSMIMECrypto::KSMIMECrypto() {
#ifdef KSSL_HAVE_SSL
    kossl = KOpenSSLProxy::self();
    priv = new KSMIMECryptoPrivate(kossl);
    if (!kossl->hasLibCrypto()) kossl = 0L;
#else
    kossl = 0L;
#endif
}


KSMIMECrypto::~KSMIMECrypto() {
#ifdef KSSL_HAVE_SSL
    delete priv;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::signMessage(const TQCString &clearText,
					   TQByteArray &cipherText,
					   const KSSLPKCS12 &privKey,
					   const TQPtrList<KSSLCertificate> &certs,
					   bool detached) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;
    BIO *in = kossl->BIO_new_mem_buf((char *)clearText.data(), clearText.size());
    BIO *out = kossl->BIO_new(kossl->BIO_s_mem());

    rc rc = priv->signMessage(in, out,
			      const_cast<KSSLPKCS12 &>(privKey),
			      const_cast<TQPtrList<KSSLCertificate> &>(certs),
			      detached);

    if (!rc) priv->MemBIOToQByteArray(out, cipherText);

    kossl->BIO_free(out);
    kossl->BIO_free(in);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::checkDetachedSignature(const TQCString &clearText,
						      const TQByteArray &signature,
						      TQPtrList<KSSLCertificate> &foundCerts) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;
    BIO *txt = kossl->BIO_new_mem_buf((char *)clearText.data(), clearText.length());
    BIO *sig = kossl->BIO_new_mem_buf((char *)signature.data(), signature.size());

    rc rc = priv->checkSignature(txt, sig, true, foundCerts);

    kossl->BIO_free(sig);
    kossl->BIO_free(txt);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::checkOpaqueSignature(const TQByteArray &signedText,
						    TQCString &clearText,
						    TQPtrList<KSSLCertificate> &foundCerts) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;

    BIO *in = kossl->BIO_new_mem_buf((char *)signedText.data(), signedText.size());
    BIO *out = kossl->BIO_new(kossl->BIO_s_mem());
   
    rc rc = priv->checkSignature(out, in, false, foundCerts);

    kossl->BIO_write(out, &eot, 1);
    priv->MemBIOToQByteArray(out, clearText);

    kossl->BIO_free(out);
    kossl->BIO_free(in);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::encryptMessage(const TQCString &clearText,
					      TQByteArray &cipherText,
					      algo algorithm,
					      const TQPtrList<KSSLCertificate> &recip) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;

    BIO *in = kossl->BIO_new_mem_buf((char *)clearText.data(), clearText.size());
    BIO *out = kossl->BIO_new(kossl->BIO_s_mem());

    rc rc = priv->encryptMessage(in,out,algorithm,
				 const_cast< TQPtrList<KSSLCertificate> &>(recip));

    if (!rc) priv->MemBIOToQByteArray(out, cipherText);

    kossl->BIO_free(out);
    kossl->BIO_free(in);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}


KSMIMECrypto::rc KSMIMECrypto::decryptMessage(const TQByteArray &cipherText,
					      TQCString &clearText,
					      const KSSLPKCS12 &privKey) {
#ifdef KSSL_HAVE_SSL
    if (!kossl) return KSC_R_NO_SSL;

    BIO *in = kossl->BIO_new_mem_buf((char *)cipherText.data(), cipherText.size());
    BIO *out = kossl->BIO_new(kossl->BIO_s_mem());

    rc rc = priv->decryptMessage(in,out,
				 const_cast<KSSLPKCS12 &>(privKey));

    kossl->BIO_write(out, &eot, 1);
    priv->MemBIOToQByteArray(out, clearText);

    kossl->BIO_free(out);
    kossl->BIO_free(in);

    return rc;
#else
    return KSC_R_NO_SSL;
#endif
}

