/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
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

#include <kopenssl.h>

#include <tqstring.h>
#include <tqfile.h>
#include <ksslall.h>
#include <kdebug.h>
#include <tdetempfile.h>
#include <kmdcodec.h>

#include <assert.h>


KSSLPKCS12::KSSLPKCS12() {
   _pkcs = NULL;
   _pkey = NULL;
   _cert = NULL;
   _caStack = NULL;
   kossl = KOSSL::self();
}



KSSLPKCS12::~KSSLPKCS12() {
#ifdef KSSL_HAVE_SSL
   if (_pkey) kossl->EVP_PKEY_free(_pkey);
   if (_caStack) {
      for (;;) {
         X509* x5 = reinterpret_cast<X509*>(kossl->OPENSSL_sk_pop(_caStack));
         if (!x5) break;
         kossl->X509_free(x5);
      }
      kossl->OPENSSL_sk_free(_caStack);
   }
   if (_pkcs) kossl->PKCS12_free(_pkcs);
#endif
   if (_cert) delete _cert;
}


KSSLPKCS12* KSSLPKCS12::fromString(TQString base64, TQString password) {
#ifdef KSSL_HAVE_SSL
KTempFile ktf;

    if (base64.isEmpty()) return NULL;
    TQByteArray qba, qbb = TQCString(base64.latin1()).copy();
    KCodecs::base64Decode(qbb, qba);
    ktf.file()->writeBlock(qba);
    ktf.close();
    KSSLPKCS12* rc = loadCertFile(ktf.name(), password);
    ktf.unlink();
    return rc;
#endif
return NULL;
}



KSSLPKCS12* KSSLPKCS12::loadCertFile(TQString filename, TQString password) {
#ifdef KSSL_HAVE_SSL
TQFile qf(filename);
PKCS12 *newpkcs = NULL;

  if (!qf.open(IO_ReadOnly))
    return NULL;

  FILE *fp = fdopen(qf.handle(), "r");
  if (!fp) return NULL;

  newpkcs = KOSSL::self()->d2i_PKCS12_fp(fp, &newpkcs);

  fclose(fp);
  if (!newpkcs) {
	KOSSL::self()->ERR_clear_error();
	return NULL;
  }

  KSSLPKCS12 *c = new KSSLPKCS12;
  c->setCert(newpkcs);

  // Now we parse it to see if we can decrypt it and interpret it
  if (!c->parse(password)) {
        delete c;  c = NULL;
  }

  return c;
#endif
return NULL;
}


void KSSLPKCS12::setCert(PKCS12 *c) {
#ifdef KSSL_HAVE_SSL
   _pkcs = c;
#endif
}


bool KSSLPKCS12::changePassword(TQString pold, TQString pnew) {
#ifdef KSSL_HAVE_SSL
   // OpenSSL makes me cast away the const here.  argh
   return (0 == kossl->PKCS12_newpass(_pkcs, 
                           pold.isNull() ? (char *)"" : (char *)pold.latin1(), 
                           pnew.isNull() ? (char *)"" : (char *)pnew.latin1()));
#endif
return false;
}


bool KSSLPKCS12::parse(TQString pass) {
#ifdef KSSL_HAVE_SSL
X509 *x = NULL;

  assert(_pkcs);   // if you're calling this before pkcs gets set, it's a BUG!

   if (_cert) delete _cert;
   if (_pkey) kossl->EVP_PKEY_free(_pkey);
   if (_caStack) {
      for (;;) {
         X509* x5 = reinterpret_cast<X509*>(kossl->OPENSSL_sk_pop(_caStack));
         if (!x5) break;
         kossl->X509_free(x5);
      }
      kossl->OPENSSL_sk_free(_caStack);
   }
   _pkey = NULL;
   _caStack = NULL;
   _cert = NULL;

  int rc = kossl->PKCS12_parse(_pkcs, pass.latin1(), &_pkey, &x, &_caStack);

  if (rc == 1) {
     // kdDebug(7029) << "PKCS12_parse success" << endl;
     if (x) {
        _cert = new KSSLCertificate;
        _cert->setCert(x);
        if (_caStack) {
           _cert->setChain(_caStack);
        }
        return true;
     }
  } else {
    _caStack = NULL;
    _pkey = NULL;
    kossl->ERR_clear_error();
  }
#endif
return false;  
}


EVP_PKEY *KSSLPKCS12::getPrivateKey() {
   return _pkey;
}


KSSLCertificate *KSSLPKCS12::getCertificate() {
   return _cert;
}


TQString KSSLPKCS12::toString() {
TQString base64;
#ifdef KSSL_HAVE_SSL
unsigned char *p;
int len;

   len = kossl->i2d_PKCS12(_pkcs, NULL);
   if (len >= 0) {
       char *buf = new char[len];
       p = (unsigned char *)buf;
       kossl->i2d_PKCS12(_pkcs, &p);
       TQByteArray qba;
       qba.setRawData(buf, len);
       base64 = KCodecs::base64Encode(qba);
       qba.resetRawData(buf, len);
       delete[] buf;
   }
#endif
return base64;
}



bool KSSLPKCS12::toFile(TQString filename) {
#ifdef KSSL_HAVE_SSL
TQFile out(filename);

   if (!out.open(IO_WriteOnly)) return false;

   int fd = out.handle();
   FILE *fp = fdopen(fd, "w");

   if (!fp) {
      unlink(filename.latin1());
      return false;
   }

   kossl->i2d_PKCS12_fp(fp, _pkcs);

   fclose(fp);
   return true;
#endif
return false;
}


KSSLCertificate::KSSLValidation KSSLPKCS12::validate() {
	return validate(KSSLCertificate::SSLServer);
}


KSSLCertificate::KSSLValidation KSSLPKCS12::validate(KSSLCertificate::KSSLPurpose p) {
#ifdef KSSL_HAVE_SSL
KSSLCertificate::KSSLValidation xx = _cert->validate(p);
   if (1 != kossl->X509_check_private_key(_cert->getCert(), _pkey)) {
      xx = KSSLCertificate::PrivateKeyFailed;
   }

return xx;
#else
return KSSLCertificate::NoSSL;
#endif
}


KSSLCertificate::KSSLValidation KSSLPKCS12::revalidate() {
   return revalidate(KSSLCertificate::SSLServer);
}


KSSLCertificate::KSSLValidation KSSLPKCS12::revalidate(KSSLCertificate::KSSLPurpose p) {
   return _cert->revalidate(p);
}


bool KSSLPKCS12::isValid() {
return isValid(KSSLCertificate::SSLServer);
}


bool KSSLPKCS12::isValid(KSSLCertificate::KSSLPurpose p) {
return (validate(p) == KSSLCertificate::Ok);
}


TQString KSSLPKCS12::name() {
   return _cert->getSubject();
}

