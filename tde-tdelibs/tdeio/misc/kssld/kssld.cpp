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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqtimer.h>

#include "kssld.h"
#include <tdeconfig.h>
#include <ksimpleconfig.h>
#include <ksslcertchain.h>
#include <ksslcertificate.h>
#include <ksslcertificatehome.h>
#include <ksslpkcs12.h>
#include <ksslx509map.h>
#include <tqptrlist.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <pwd.h>
#include <unistd.h>
#include <tqfile.h>
#include <tqsortedlist.h>
#include <tdeglobal.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <tqdatetime.h>

#include <kmdcodec.h>
#include <kopenssl.h>

// See design notes at end

extern "C" {
	TDE_EXPORT KDEDModule *create_kssld(const TQCString &name) {
		return new KSSLD(name);
	}

	TDE_EXPORT void *__kde_do_unload;
}


static void updatePoliciesConfig(TDEConfig *cfg) {
	TQStringList groups = cfg->groupList();

	for (TQStringList::Iterator i = groups.begin(); i != groups.end(); ++i) {
		if ((*i).isEmpty() || *i == "General") {
			continue;
		}

		cfg->setGroup(*i);

		bool permanent = cfg->readBoolEntry("Permanent");
		TQDateTime expires = cfg->readDateTimeEntry("Expires");
		kdDebug(7029) << "static void updatePoliciesConfig(TDEConfig *cfg) permanent: " << permanent << endl;
		kdDebug(7029) << "static void updatePoliciesConfig(TDEConfig *cfg) expires: " << expires.toString() << endl;

		// remove it if it has expired
		if ( !permanent || expires <= TQDateTime::currentDateTime() ) {
			cfg->deleteGroup(*i);
			continue;
		}

		TQString encodedCertStr = cfg->readEntry("Certificate");
		TQCString encodedCert = encodedCertStr.local8Bit();
	       	KSSLCertificate *newCert = KSSLCertificate::fromString(encodedCert);
		if (!newCert) {
			cfg->deleteGroup(*i);
			continue;
		}

		KSSLCertificateCache::KSSLCertificatePolicy policy = (KSSLCertificateCache::KSSLCertificatePolicy) cfg->readNumEntry("Policy");
		TQStringList hosts = cfg->readListEntry("Hosts");
		TQStringList chain = cfg->readListEntry("Chain");
		cfg->deleteGroup(*i);

		cfg->setGroup(newCert->getMD5Digest());
		cfg->writeEntry("Certificate", encodedCertStr);
		cfg->writeEntry("Policy", policy);
		cfg->writeEntry("Permanent", permanent);
		cfg->writeEntry("Expires", expires);
		cfg->writeEntry("Hosts", hosts);
		cfg->writeEntry("Chain", chain);
		delete newCert;
	}

	cfg->setGroup("General");
	cfg->writeEntry("policies version", 2);

	cfg->sync();
}


KSSLD::KSSLD(const TQCString &name) : KDEDModule(name)
{
// ----------------------- FOR THE CACHE ------------------------------------	
	cfg = new KSimpleConfig("ksslpolicies", false);
	cfg->setGroup("General");
	if (2 != cfg->readNumEntry("policies version", 0)) {
		::updatePoliciesConfig(cfg);
	}
	TDEGlobal::dirs()->addResourceType("kssl", TDEStandardDirs::kde_default("data") + "kssl");
	caVerifyUpdate();
	cacheLoadDefaultPolicies();
	certList.setAutoDelete(false);
	kossl = KOSSL::self();

// ----------------------- FOR THE HOME -------------------------------------
}
  

KSSLD::~KSSLD()
{
// ----------------------- FOR THE CACHE ------------------------------------	
	cacheClearList();
	delete cfg;

// ----------------------- FOR THE HOME -------------------------------------
}

  


// A node in the cache
class KSSLCNode {
	public:
		KSSLCertificate *cert;
		KSSLCertificateCache::KSSLCertificatePolicy policy;
		bool permanent;
		TQDateTime expires;
		TQStringList hosts;
		KSSLCNode() { cert = 0L;
				policy = KSSLCertificateCache::Unknown; 
				permanent = false;
			}
		~KSSLCNode() { delete cert; }
};



void KSSLD::cacheSaveToDisk() {
KSSLCNode *node;

	cfg->setGroup("General");
	cfg->writeEntry("policies version", 2);

	for (node = certList.first(); node; node = certList.next()) {
			// First convert to a binary format and then write the
			// tdeconfig entry write the (CN, policy, cert) to
			// KSimpleConfig
			cfg->setGroup(node->cert->getMD5Digest());
			cfg->writeEntry("Certificate", node->cert->toString());
			cfg->writeEntry("Policy", node->policy);
			cfg->writeEntry("Expires", node->expires);
			cfg->writeEntry("Permanent", node->permanent);
			cfg->writeEntry("Hosts", node->hosts);

			// Also write the chain
			TQStringList qsl;
			TQPtrList<KSSLCertificate> cl =
						node->cert->chain().getChain();
			for (KSSLCertificate *c = cl.first();
							c != 0;
							c = cl.next()) {
				//kdDebug(7029) << "Certificate in chain: "
				//	    <<  c->toString() << endl;
				qsl << c->toString();
			}

			cl.setAutoDelete(true);
			cfg->writeEntry("Chain", qsl);
		}

	cfg->sync();

	// insure proper permissions -- contains sensitive data
	TQString cfgName(TDEGlobal::dirs()->findResource("config", "ksslpolicies"));

	if (!cfgName.isEmpty()) {
		::chmod(TQFile::encodeName(cfgName), 0600);
	}
}


void KSSLD::cacheReload() {
	cacheClearList();
	delete cfg;
	cfg = new KSimpleConfig("ksslpolicies", false);
	cacheLoadDefaultPolicies();
}


void KSSLD::cacheClearList() {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		certList.remove(node);
		delete node;
	}

	skEmail.clear();
	skMD5Digest.clear();
}


void KSSLD::cacheLoadDefaultPolicies() {
TQStringList groups = cfg->groupList();

	for (TQStringList::Iterator i = groups.begin();
				i != groups.end();
				++i) {
		if ((*i).isEmpty() || *i == "General")
			continue;

		cfg->setGroup(*i);

		bool permanent = cfg->readBoolEntry("Permanent");
		TQDateTime expires = cfg->readDateTimeEntry("Expires");
		kdDebug(7029) << "static void cacheLoadDefaultPolicies() permanent: " << permanent << endl;
		kdDebug(7029) << "static void cacheLoadDefaultPolicies() expires: " << expires.toString() << endl;
		
		// remove it if it has expired
		if ( !permanent || expires <= TQDateTime::currentDateTime()) {
			cfg->deleteGroup(*i);
			continue;
		}

		TQCString encodedCert;
		KSSLCertificate *newCert;

		encodedCert = cfg->readEntry("Certificate").local8Bit();
	       	newCert = KSSLCertificate::fromString(encodedCert);

		if (!newCert) {
		       continue;
		}

		KSSLCNode *n = new KSSLCNode;
		n->cert = newCert;
		n->policy = (KSSLCertificateCache::KSSLCertificatePolicy) cfg->readNumEntry("Policy");
		n->permanent = permanent;
		n->expires = expires;
		n->hosts = cfg->readListEntry("Hosts");
		newCert->chain().setCertChain(cfg->readListEntry("Chain"));
		certList.append(n); 
		searchAddCert(newCert);
	}
}


void KSSLD::cacheAddCertificate(KSSLCertificate cert, 
			KSSLCertificateCache::KSSLCertificatePolicy policy,
			bool permanent) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			node->policy = policy;
		
			if (!permanent) // check if we've set it to permanent before
			   node->permanent = cacheIsPermanent(cert);
			else
			   node->permanent = true;

			if ( !node->expires.isValid() ) {
			if ( !node->permanent ) {
				node->expires = TQDateTime::currentDateTime();
				// FIXME: make this configurable
					node->expires = node->expires.addSecs(5);
			} else {
					node->expires = node->cert->getQDTNotAfter(); // set to certs expiry date
			}
			}

			kdDebug(7029) << "KSSLD::cacheAddCertificate(...) node permanent: " << node->permanent << endl;
			kdDebug(7029) << "KSSLD::cacheAddCertificate(...) node expires: " << node->expires.toString() << endl;
			cacheSaveToDisk();
			return;
		}
	}

	KSSLCNode *n = new KSSLCNode;
	n->cert = cert.replicate();
	n->policy = policy;
	n->permanent = permanent;
	// remove the old one
	cacheRemoveByCertificate(*(n->cert));
	certList.prepend(n); 

	if (!permanent) {
		n->expires = TQDateTime::currentDateTime();
		n->expires = n->expires.addSecs(5);
	} else {
		if ( !n->expires.isValid() )
			n->expires = n->cert->getQDTNotAfter(); // set to certs expiry date
	}
	kdDebug(7029) << "KSSLD::cacheAddCertificate(...) new node permanent: " << n->permanent << endl;
	kdDebug(7029) << "KSSLD::cacheAddCertificate(...) new node expires: " << n->expires.toString() << endl;

	searchAddCert(n->cert);
	cacheSaveToDisk();
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLD::cacheGetPolicyByCN(TQString cn) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			certList.remove(node);
			certList.prepend(node);
			return node->policy;
		}
	}

return KSSLCertificateCache::Unknown;
}


KSSLCertificateCache::KSSLCertificatePolicy KSSLD::cacheGetPolicyByCertificate(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {  
			certList.remove(node);
			certList.prepend(node);
			return node->policy;
		}
	}

return KSSLCertificateCache::Unknown;
}


bool KSSLD::cacheSeenCN(TQString cn) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			certList.remove(node);
			certList.prepend(node);
			return true;
		}
	}

return false;
}


bool KSSLD::cacheSeenCertificate(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			certList.remove(node);
			certList.prepend(node);
			return true;
		}
	}

return false;
}


bool KSSLD::cacheIsPermanent(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			certList.remove(node);
			certList.prepend(node);
			return node->permanent;
		}
	}

return false;
}


bool KSSLD::cacheRemoveBySubject(TQString subject) {
KSSLCNode *node;
bool gotOne = false;

	for (node = certList.first(); node; node = certList.next()) {
		if (node->cert->getSubject() == subject) {
			certList.remove(node);
			cfg->deleteGroup(node->cert->getMD5Digest());
			searchRemoveCert(node->cert);
			delete node;
			gotOne = true;
		}
	}

	cacheSaveToDisk();
return gotOne;
}


bool KSSLD::cacheRemoveByCN(TQString cn) {
KSSLCNode *node;
bool gotOne = false;

	for (node = certList.first(); node; node = certList.next()) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			certList.remove(node);
			cfg->deleteGroup(node->cert->getMD5Digest());
			searchRemoveCert(node->cert);
			delete node;
			gotOne = true;
		}
	}

	cacheSaveToDisk();
return gotOne;
}


bool KSSLD::cacheRemoveByCertificate(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			certList.remove(node);
			cfg->deleteGroup(node->cert->getMD5Digest());
			searchRemoveCert(node->cert);
			delete node;
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


bool KSSLD::cacheModifyByCN(TQString cn,
                            KSSLCertificateCache::KSSLCertificatePolicy policy,
                            bool permanent,
                            TQDateTime expires) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (KSSLX509Map(node->cert->getSubject()).getValue("CN") == cn) {
			node->permanent = permanent;
			node->expires = expires;
			node->policy = policy;
			certList.remove(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


bool KSSLD::cacheModifyByCertificate(KSSLCertificate cert,
                             KSSLCertificateCache::KSSLCertificatePolicy policy,
			     bool permanent,
			     TQDateTime expires) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			node->permanent = permanent;
			node->expires = expires;
			node->policy = policy;
			certList.remove(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


TQStringList KSSLD::cacheGetHostList(KSSLCertificate cert) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			certList.remove(node);
			certList.prepend(node);
			return node->hosts;
		}
	}

return TQStringList();
}


bool KSSLD::cacheAddHost(KSSLCertificate cert, TQString host) {
KSSLCNode *node;

	if (host.isEmpty())
		return true;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			if (!node->hosts.contains(host))
				node->hosts << host;

			certList.remove(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}


bool KSSLD::cacheRemoveHost(KSSLCertificate cert, TQString host) {
KSSLCNode *node;

	for (node = certList.first(); node; node = certList.next()) {
		if (cert == *(node->cert)) {
			node->hosts.remove(host);
			certList.remove(node);
			certList.prepend(node);
			cacheSaveToDisk();
			return true;
		}
	}

return false;
}




///////////////////////////////////////////////////////////////////////////

void KSSLD::caVerifyUpdate() {
	TQString path = TDEGlobal::dirs()->saveLocation("kssl") + "/ca-bundle.crt";
	if (!TQFile::exists(path))
		return;
	
	cfg->setGroup(TQString::null);
	TQ_UINT32 newStamp = TDEGlobal::dirs()->calcResourceHash("config", "ksslcalist", true);
	TQ_UINT32 oldStamp = cfg->readUnsignedNumEntry("ksslcalistStamp");
	if (oldStamp != newStamp)
	{
		caRegenerate();
		cfg->writeEntry("ksslcalistStamp", newStamp);
		cfg->sync();
	}
}

bool KSSLD::caRegenerate() {
TQString path = TDEGlobal::dirs()->saveLocation("kssl") + "/ca-bundle.crt";

TQFile out(path);

	if (!out.open(IO_WriteOnly))
		return false;

TDEConfig cfg("ksslcalist", true, false);

TQStringList x = cfg.groupList();

	for (TQStringList::Iterator i = x.begin();
				   i != x.end();
				   ++i) {
		if ((*i).isEmpty() || *i == "<default>") continue;

		cfg.setGroup(*i);

		if (!cfg.readBoolEntry("site", false)) continue;

		TQString cert = cfg.readEntry("x509", "");
		if (cert.length() <= 0) continue;

		unsigned int xx = cert.length() - 1;
		for (unsigned int j = 0; j < xx/64; j++) {
			cert.insert(64*(j+1)+j, '\n');
		}
		out.writeBlock("-----BEGIN CERTIFICATE-----\n", 28);
		out.writeBlock(cert.latin1(), cert.length());
		out.writeBlock("\n-----END CERTIFICATE-----\n\n", 28);
		out.flush();
	}

return true;
}



bool KSSLD::caAdd(TQString certificate, bool ssl, bool email, bool code) {
KSSLCertificate *x = KSSLCertificate::fromString(certificate.local8Bit());

	if (!x) return false;

TDEConfig cfg("ksslcalist", false, false);

	cfg.setGroup(x->getSubject());
	cfg.writeEntry("x509", certificate);
	cfg.writeEntry("site", ssl);
	cfg.writeEntry("email", email);
	cfg.writeEntry("code", code);

	cfg.sync();
	delete x;

return true;
}


/**
  * @internal
  * Returns a list of certificates as QStrings read from the given file
  */
static TQStringList caReadCerticatesFromFile(TQString filename) {

	TQStringList certificates;
	TQString certificate, temp;
	TQFile file(filename);

	if (!file.open(IO_ReadOnly))
		return certificates;

	while (!file.atEnd()) {
		file.readLine(temp, 999);
		if (temp.startsWith("-----BEGIN CERTIFICATE-----")) {
			certificate = TQString::null;
			continue;
		}

		if (temp.startsWith("-----END CERTIFICATE-----")) {
			certificates.append(certificate);
			certificate = TQString::null;
			continue;
		}

		certificate += temp.stripWhiteSpace();
	}

	file.close();

	return certificates;
}

bool KSSLD::caAddFromFile(TQString filename, bool ssl, bool email, bool code) {

	TQStringList certificates;
	certificates = caReadCerticatesFromFile(filename);
	if (certificates.isEmpty())
		return false;

	bool ok = true;

	for (TQStringList::Iterator it = certificates.begin();
					it != certificates.end(); ++it ) {
		ok &= caAdd(*it, ssl, email, code);
	}

	return ok;
}

bool KSSLD::caRemoveFromFile(TQString filename) {

	TQStringList certificates;
	certificates = caReadCerticatesFromFile(filename);
	if (certificates.isEmpty())
		return false;

	bool ok = true;

	for (TQStringList::Iterator it = certificates.begin();
					it != certificates.end(); ++it ) {
		TQString certificate = *it;
		KSSLCertificate *x = KSSLCertificate::fromString(certificate.local8Bit());
		ok &= x && caRemove(x->getSubject());
		delete x;
	}

	return ok;
}


TQStringList KSSLD::caList() {
TQStringList x;
TDEConfig cfg("ksslcalist", true, false);

	x = cfg.groupList();
	x.remove("<default>");

return x;
}


bool KSSLD::caUseForSSL(TQString subject) {
TDEConfig cfg("ksslcalist", true, false);

	if (!cfg.hasGroup(subject))
		return false;

	cfg.setGroup(subject);
return cfg.readBoolEntry("site", false);
}



bool KSSLD::caUseForEmail(TQString subject) {
TDEConfig cfg("ksslcalist", true, false);

	if (!cfg.hasGroup(subject))
		return false;

	cfg.setGroup(subject);
return cfg.readBoolEntry("email", false);
}



bool KSSLD::caUseForCode(TQString subject) {
TDEConfig cfg("ksslcalist", true, false);

	if (!cfg.hasGroup(subject))
		return false;

	cfg.setGroup(subject);
return cfg.readBoolEntry("code", false);
}


bool KSSLD::caRemove(TQString subject) {
TDEConfig cfg("ksslcalist", false, false);
	if (!cfg.hasGroup(subject))
		return false;

	cfg.deleteGroup(subject);
	cfg.sync();

return true;
}


TQString KSSLD::caGetCert(TQString subject) {
TDEConfig cfg("ksslcalist", true, false);
	if (!cfg.hasGroup(subject))
		return TQString::null;

	cfg.setGroup(subject);

return cfg.readEntry("x509", TQString::null);
}


bool KSSLD::caSetUse(TQString subject, bool ssl, bool email, bool code) {
TDEConfig cfg("ksslcalist", false, false);
	if (!cfg.hasGroup(subject))
		return false;

	cfg.setGroup(subject);

	cfg.writeEntry("site", ssl);
	cfg.writeEntry("email", email);
	cfg.writeEntry("code", code);
	cfg.sync();

return true;
}

///////////////////////////////////////////////////////////////////////////

void KSSLD::searchAddCert(KSSLCertificate *cert) {
	skMD5Digest.insert(cert->getMD5Digest(), cert, true);

	TQStringList mails;
	cert->getEmails(mails);
	for(TQStringList::const_iterator iter = mails.begin(); iter != mails.end(); ++iter) {
		TQString email = static_cast<const TQString &>(*iter).lower();
		TQMap<TQString, TQPtrVector<KSSLCertificate> >::iterator it = skEmail.find(email);

		if (it == skEmail.end())
			it = skEmail.insert(email, TQPtrVector<KSSLCertificate>());

		TQPtrVector<KSSLCertificate> &elem = *it;
		
		if (elem.findRef(cert) == -1) {
			unsigned int n = 0;
			for(; n < elem.size(); n++) {
				if (!elem.at(n)) {
					elem.insert(n, cert);
					break;
				}
			}
			if (n == elem.size()) {
				elem.resize(n+1);
				elem.insert(n, cert);
			}
		}
	}	
}


void KSSLD::searchRemoveCert(KSSLCertificate *cert) {
	skMD5Digest.remove(cert->getMD5Digest());

	TQStringList mails;
	cert->getEmails(mails);
	for(TQStringList::const_iterator iter = mails.begin(); iter != mails.end(); ++iter) {
		TQMap<TQString, TQPtrVector<KSSLCertificate> >::iterator it = skEmail.find(static_cast<const TQString &>(*iter).lower());

		if (it == skEmail.end())
		       break;

		TQPtrVector<KSSLCertificate> &elem = *it;

		int n = elem.findRef(cert);
		if (n != -1)
			elem.remove(n);
	}
}	


TQStringList KSSLD::getKDEKeyByEmail(const TQString &email) {
	TQStringList rc;
	TQMap<TQString, TQPtrVector<KSSLCertificate> >::iterator it = skEmail.find(email.lower());

	kdDebug(7029) << "GETKDEKey " << email.latin1() << endl;

	if (it == skEmail.end())
		return rc;

	TQPtrVector<KSSLCertificate> &elem = *it;
	for (unsigned int n = 0; n < elem.size(); n++) {
		KSSLCertificate *cert = elem.at(n);
		if (cert) {
			rc.append(cert->getKDEKey());
		}
	}

	kdDebug(7029) << "results: " << rc.size() << " " << elem.size() << endl;
	return rc;
}


KSSLCertificate KSSLD::getCertByMD5Digest(const TQString &key) {
	TQMap<TQString, KSSLCertificate *>::iterator iter = skMD5Digest.find(key);
	
	kdDebug(7029) << "Searching cert for " << key.latin1() << endl;

	if (iter != skMD5Digest.end())
		return **iter;
	
	KSSLCertificate rc; // FIXME: Better way to return a not found condition?
	kdDebug(7029) << "Not found: " << rc.toString().latin1() << endl;
	return rc;
}	


///////////////////////////////////////////////////////////////////////////

//
//  Certificate Home methods
//

TQStringList KSSLD::getHomeCertificateList() {
	return KSSLCertificateHome::getCertificateList();
}

bool KSSLD::addHomeCertificateFile(TQString filename, TQString password, bool storePass) {
	return KSSLCertificateHome::addCertificate(filename, password, storePass);
}

bool KSSLD::addHomeCertificatePKCS12(TQString base64cert, TQString passToStore) {
	bool ok;
	KSSLPKCS12 *pkcs12 = KSSLPKCS12::fromString(base64cert, passToStore);
	ok = KSSLCertificateHome::addCertificate(pkcs12, passToStore);
	delete pkcs12;
	return ok;
}

bool KSSLD::deleteHomeCertificateByFile(TQString filename, TQString password) {
	return KSSLCertificateHome::deleteCertificate(filename, password);
}

bool KSSLD::deleteHomeCertificateByPKCS12(TQString base64cert, TQString password) {
	bool ok;
	KSSLPKCS12 *pkcs12 = KSSLPKCS12::fromString(base64cert, password);
	ok = KSSLCertificateHome::deleteCertificate(pkcs12);
	delete pkcs12;
	return ok;
}

bool KSSLD::deleteHomeCertificateByName(TQString name) {
	return KSSLCertificateHome::deleteCertificateByName(name);
}



///////////////////////////////////////////////////////////////////////////

#include "kssld.moc"


/*

  DESIGN     - KSSLCertificateCache
  ------

  This is the first implementation and I think this cache actually needs
  experimentation to determine which implementation works best.  My current
  options are:

   (1) Store copies of the X509 certificates in a TQPtrList using a self
       organizing heuristic as described by Munro and Suwanda.
   (2) Store copies of the X509 certificates in a tree structure, perhaps
       a redblack tree, avl tree, or even just a simple binary tree.
   (3) Store the CN's in a tree or list and use them as a hash to retrieve
       the X509 certificates.
   (4) Create "nodes" containing the X509 certificate and place them in
       two structures concurrently, one organized by CN, the other by
       X509 serial number.

  This implementation uses (1).  (4) is definitely attractive, but I don't
  think it will be necessary to go so crazy with performance, and perhaps
  end up performing poorly in situations where there are very few entries in
  the cache (which is most likely the case most of the time).  The style of
  heuristic is move-to-front, not swap-forward.  This seems to make more
  sense because the typical user will hit a site at least a few times in a
  row before moving to a new one.

  What I worry about most with respect to performance is that cryptographic
  routines are expensive and if we have to perform them on each X509
  certificate until the right one is found, we will perform poorly.

  All in all, this code is actually quite crucial for performance on SSL
  website, especially those with many image files loaded via SSL.  If a
  site loads 15 images, we will have to run through this code 15 times.
  A heuristic for self organization will make each successive lookup faster.
  Sounds good, doesn't it?

  DO NOT ATTEMPT TO GUESS WHICH CERTIFICATES ARE ACCEPTIBLE IN YOUR CODE!!
  ALWAYS USE THE CACHE.  IT MAY CHECK THINGS THAT YOU DON'T THINK OF, AND
  ALSO IF THERE IS A BUG IN THE CHECKING CODE, IF IT IS ALL CONTAINED IN
  THIS LIBRARY, A MINOR FIX WILL FIX ALL APPLICATIONS.
 */

