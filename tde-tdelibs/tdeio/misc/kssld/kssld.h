/*
   This file is part of the KDE libraries

   Copyright (c) 2001-2005 George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/
#ifndef __KSSLD_H__
#define __KSSLD_H__

#include <kded/kdedmodule.h>
#include <ksslcertificate.h>
#include <ksslcertificatecache.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>
#include <tqmap.h>
#include <tqptrvector.h>


class KSimpleConfig;
class KSSLCNode;
class KOpenSSLProxy;

class KSSLD : public KDEDModule
{
  TQ_OBJECT
  K_DCOP

public:
  
  KSSLD(const TQCString &name);
  
  virtual ~KSSLD();

k_dcop:
  //
  //  Certificate Cache methods
  //
  void cacheAddCertificate(KSSLCertificate cert, 
		           KSSLCertificateCache::KSSLCertificatePolicy policy,
		           bool permanent = true);
  KSSLCertificateCache::KSSLCertificatePolicy cacheGetPolicyByCN(TQString cn);

  KSSLCertificateCache::KSSLCertificatePolicy cacheGetPolicyByCertificate(KSSLCertificate cert);

  bool cacheSeenCN(TQString cn);
  bool cacheSeenCertificate(KSSLCertificate cert);

  bool cacheRemoveByCN(TQString cn);
  bool cacheRemoveBySubject(TQString subject);
  bool cacheRemoveByCertificate(KSSLCertificate cert);
	       
  bool cacheIsPermanent(KSSLCertificate cert);

  void cacheReload();

  bool cacheModifyByCN(TQString cn,
                       KSSLCertificateCache::KSSLCertificatePolicy policy,
                       bool permanent,
                       TQDateTime expires);

  bool cacheModifyByCertificate(KSSLCertificate cert,
                           KSSLCertificateCache::KSSLCertificatePolicy policy,
                                bool permanent,
                                TQDateTime expires);

  TQStringList cacheGetHostList(KSSLCertificate cert);

  bool cacheAddHost(KSSLCertificate cert, TQString host);

  bool cacheRemoveHost(KSSLCertificate cert, TQString host);

  /* Certificate Authorities */
  void caVerifyUpdate();
  bool caRegenerate();

  TQStringList caList();

  bool caUseForSSL(TQString subject);

  bool caUseForEmail(TQString subject);
  
  bool caUseForCode(TQString subject);

  bool caAdd(TQString certificate, bool ssl, bool email, bool code);

  bool caAddFromFile(TQString filename, bool ssl, bool email, bool code);

  bool caRemove(TQString subject);

  bool caRemoveFromFile(TQString filename);

  TQString caGetCert(TQString subject);

  bool caSetUse(TQString subject, bool ssl, bool email, bool code);

  TQStringList getKDEKeyByEmail(const TQString &email);

  KSSLCertificate getCertByMD5Digest(const TQString &key);

  //
  //  Certificate Home methods
  //

  TQStringList getHomeCertificateList();

  bool addHomeCertificateFile(TQString filename, TQString password, bool storePass /*=false*/);

  bool addHomeCertificatePKCS12(TQString base64cert, TQString passToStore);

  bool deleteHomeCertificateByFile(TQString filename, TQString password);

  bool deleteHomeCertificateByPKCS12(TQString base64cert, TQString password);

  bool deleteHomeCertificateByName(TQString name);

private:

  void cacheClearList();
  void cacheSaveToDisk();
  void cacheLoadDefaultPolicies();

  // for the cache portion:
  KSimpleConfig *cfg;
  TQPtrList<KSSLCNode> certList;

  // Our pointer to OpenSSL
  KOpenSSLProxy *kossl;

  // 
  void searchAddCert(KSSLCertificate *cert);
  void searchRemoveCert(KSSLCertificate *cert);

  TQMap<TQString, TQPtrVector<KSSLCertificate> > skEmail;
  TQMap<TQString, KSSLCertificate *> skMD5Digest;
};


#endif
