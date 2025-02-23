/* This file is part of the KDE project
 *
 * Copyright (C) 2000, 2001 George Staikos <staikos@kde.org>
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


#include "ksslcertificatecache.h"
#include "ksslcertchain.h"
#include "ksslcertificate.h"

#include <stdlib.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kdatastream.h>


class KSSLCertificateCache::KSSLCertificateCachePrivate {
  public:
  DCOPClient *dcc;

  KSSLCertificateCachePrivate()  { dcc = new DCOPClient; dcc->attach(); }
  ~KSSLCertificateCachePrivate() { delete dcc;}

};



KSSLCertificateCache::KSSLCertificateCache() {
  d = new KSSLCertificateCachePrivate;
}


KSSLCertificateCache::~KSSLCertificateCache() {
  delete d;
}


void KSSLCertificateCache::saveToDisk() {
   kdDebug() << "Deprecated function KSSLCertificateCache::saveToDisk() called" << endl;
}


void KSSLCertificateCache::clearList() {
   kdDebug() << "Deprecated function KSSLCertificateCache::clearList() called" << endl;
}


void KSSLCertificateCache::loadDefaultPolicies() {
   kdDebug() << "Deprecated function KSSLCertificateCache::loadDefaultPolicies() called" << endl;
}


void KSSLCertificateCache::reload() {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     d->dcc->call("kded", "kssld",
                  "cacheReload()",
                  data, rettype, retval);
}


void KSSLCertificateCache::addCertificate(KSSLCertificate& cert, 
                       KSSLCertificatePolicy policy, bool permanent) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert;
     arg << policy;
     arg << permanent;
     d->dcc->call("kded", "kssld",
                  "cacheAddCertificate(KSSLCertificate,KSSLCertificateCache::KSSLCertificatePolicy,bool)",
                  data, rettype, retval);
}


// KDE 4: Make it const TQString &
KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCN(TQString& cn) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheGetPolicyByCN(TQString)",
                            data, rettype, retval);

     if (rc && rettype == "KSSLCertificateCache::KSSLCertificatePolicy") {
        TQDataStream retStream(retval, IO_ReadOnly);
        KSSLCertificateCache::KSSLCertificatePolicy drc;
        retStream >> drc;
	return drc;
     }
return KSSLCertificateCache::Ambiguous;
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLCertificateCache::getPolicyByCertificate(KSSLCertificate& cert) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheGetPolicyByCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "KSSLCertificateCache::KSSLCertificatePolicy") {
        TQDataStream retStream(retval, IO_ReadOnly);
        KSSLCertificateCache::KSSLCertificatePolicy drc;
        retStream >> drc;
	return drc;
     }
return KSSLCertificateCache::Ambiguous;
}


// KDE 4: Make it const TQString &
bool KSSLCertificateCache::seenCN(TQString& cn) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheSeenCN(TQString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::seenCertificate(KSSLCertificate& cert) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheSeenCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::isPermanent(KSSLCertificate& cert) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheIsPermanent(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


// KDE 4: Make it const TQString &
bool KSSLCertificateCache::removeByCN(TQString& cn) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cn;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheRemoveByCN(TQString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


bool KSSLCertificateCache::removeByCertificate(KSSLCertificate& cert) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheRemoveByCertificate(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
	return drc;
     }

return false;
}


// KDE 4: Make it const TQString &
bool KSSLCertificateCache::modifyByCN(TQString& cn,
                  KSSLCertificateCache::KSSLCertificatePolicy policy,
                  bool permanent,
                  TQDateTime& expires) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cn << policy << permanent << expires;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheModifyByCN(TQString,KSSLCertificateCache::KSSLCertificatePolicy,bool,TQDateTime)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLCertificateCache::modifyByCertificate(KSSLCertificate& cert,
                           KSSLCertificateCache::KSSLCertificatePolicy policy,
                           bool permanent,
                           TQDateTime& expires) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert << policy << permanent << expires;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheModifyByCertificate(KSSLCertificate,KSSLCertificateCache::KSSLCertificatePolicy,bool,TQDateTime)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


TQStringList KSSLCertificateCache::getHostList(KSSLCertificate& cert) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheGetHostList(KSSLCertificate)",
                            data, rettype, retval);

     if (rc && rettype == "TQStringList") {
        TQDataStream retStream(retval, IO_ReadOnly);
        TQStringList drc;
        retStream >> drc;
	return drc;
     }
return TQStringList();
}


// KDE 4: Make it const TQString &
bool KSSLCertificateCache::addHost(KSSLCertificate& cert, TQString& host) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert << host;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheAddHost(KSSLCertificate,TQString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


// KDE 4: Make it const TQString &
bool KSSLCertificateCache::removeHost(KSSLCertificate& cert, TQString& host) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert << host;
     bool rc = d->dcc->call("kded", "kssld",
                            "cacheRemoveHost(KSSLCertificate,TQString)",
                            data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


TQStringList KSSLCertificateCache::getKDEKeyByEmail(const TQString &email) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << email;
     bool rc = d->dcc->call("kded", "kssld",
                            "getKDEKeyByEmail(TQString)",
                            data, rettype, retval);

     if (rc && rettype == "TQStringList") {
        TQDataStream retStream(retval, IO_ReadOnly);
        TQStringList drc;
        retStream >> drc;
        return drc;
     }

     return TQStringList();
}     


KSSLCertificate *KSSLCertificateCache::getCertByMD5Digest(const TQString &key) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << key;
     bool rc = d->dcc->call("kded", "kssld",
                            "getCertByMD5Digest(TQString)",
                            data, rettype, retval);

     if (rc && rettype == "KSSLCertificate") {
        TQDataStream retStream(retval, IO_ReadOnly);
        KSSLCertificate *drc = new KSSLCertificate;
        retStream >> *drc;
	if (drc->getCert())
             return drc; 
	delete drc; // should not happen too often if used in conjunction with getKDEKeyByEmail
     }

     return 0L;
}     


TQDataStream& operator<<(TQDataStream& s, const KSSLCertificateCache::KSSLCertificatePolicy& p) {
  s << (TQ_UINT32)p;
return s;
}


TQDataStream& operator>>(TQDataStream& s, KSSLCertificateCache::KSSLCertificatePolicy& p) {
  TQ_UINT32 pd;
  s >> pd;
  p = (KSSLCertificateCache::KSSLCertificatePolicy) pd;
  return s;
}





