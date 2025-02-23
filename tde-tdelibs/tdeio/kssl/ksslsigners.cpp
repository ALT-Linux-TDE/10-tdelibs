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


#include <tqstring.h>
#include <tqstringlist.h>
#include "ksslcertificate.h"
#include "ksslsigners.h"
#include <stdlib.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kdatastream.h>


KSSLSigners::KSSLSigners() {
	dcc = new DCOPClient;
	dcc->attach();
}


KSSLSigners::~KSSLSigners() {
	delete dcc;
}

bool KSSLSigners::addCA(KSSLCertificate& cert,
                        bool ssl,
                        bool email,
                        bool code) {
	return addCA(cert.toString(), ssl, email, code);
}


bool KSSLSigners::addCA(TQString cert,
                        bool ssl,
                        bool email,
                        bool code) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << cert;
     arg << ssl << email << code;
     bool rc = dcc->call("kded", "kssld",
                         "caAdd(TQString,bool,bool,bool)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::regenerate() {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     bool rc = dcc->call("kded", "kssld",
                         "caRegenerate()",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::useForSSL(KSSLCertificate& cert) {
	return useForSSL(cert.getSubject());
}


bool KSSLSigners::useForSSL(TQString subject) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caUseForSSL(TQString)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::useForEmail(KSSLCertificate& cert) {
	return useForEmail(cert.getSubject());
}


bool KSSLSigners::useForEmail(TQString subject) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caUseForEmail(TQString)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::useForCode(KSSLCertificate& cert) {
	return useForCode(cert.getSubject());
}


bool KSSLSigners::useForCode(TQString subject) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caUseForCode(TQString)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


bool KSSLSigners::remove(KSSLCertificate& cert) {
	return remove(cert.getSubject());
}


bool KSSLSigners::remove(TQString subject) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caRemove(TQString)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}


TQStringList KSSLSigners::list() {
     TQStringList drc;
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     bool rc = dcc->call("kded", "kssld",
                         "caList()",
                         data, rettype, retval);

     if (rc && rettype == "TQStringList") {
        TQDataStream retStream(retval, IO_ReadOnly);
        retStream >> drc;
     }

return drc;
}


TQString KSSLSigners::getCert(TQString subject) {
     TQString drc;
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << subject;
     bool rc = dcc->call("kded", "kssld",
                         "caGetCert(TQString)",
                         data, rettype, retval);

     if (rc && rettype == "TQString") {
        TQDataStream retStream(retval, IO_ReadOnly);
        retStream >> drc;
     }

return drc;
}


bool KSSLSigners::setUse(TQString subject, bool ssl, bool email, bool code) {
     TQByteArray data, retval;
     TQCString rettype;
     TQDataStream arg(data, IO_WriteOnly);
     arg << subject << ssl << email << code;
     bool rc = dcc->call("kded", "kssld",
                         "caSetUse(TQString,bool,bool,bool)",
                         data, rettype, retval);

     if (rc && rettype == "bool") {
        TQDataStream retStream(retval, IO_ReadOnly);
        bool drc;
        retStream >> drc;
        return drc;
     }

return false;
}




