/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
 * Copyright (C) 2000 Malte Starostik <malte@kde.org>
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

#ifndef _KSSLINFODLG_H
#define _KSSLINFODLG_H

#include <kdialog.h>

#include "ksslx509map.h"
#include "ksslcertificate.h"
#include "kssl.h"
#include <tqscrollview.h>

class TQWidget;
class KSSLCertBox;
class KSSLCertChain;


/**
 * KDE SSL Information Dialog
 *
 * This class creates a dialog that can be used to display information about
 * an SSL session.
 *
 * There are NO GUARANTEES that KSSLInfoDlg will remain binary compatible/
 * Contact staikos@kde.org for details if needed.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL
 * @short KDE SSL Information Dialog
 */
class TDEIO_EXPORT KSSLInfoDlg : public KDialog {
	TQ_OBJECT
public:
	/**
	 *  Construct a KSSL Information Dialog
	 *
	 *  @param secureConnection true if the connection is secured with SSL
	 *  @param parent the parent widget
	 *  @param name the internal name of this instance
	 *  @param modal true if the dialog should be modal
	 */
	KSSLInfoDlg(bool secureConnection, TQWidget *parent=0L, const char *name=0L, bool modal=false);

	/**
	 *  Destroy this dialog
	 */
	virtual ~KSSLInfoDlg();

	/**
	 *  Tell the dialog if the connection has portions that may not be
	 *  secure (ie. a mixture of secure and insecure frames)
	 *
	 *  @param isIt true if security is in question
	 */
	void setSecurityInQuestion(bool isIt);

	/**
	 *  Setup the dialog before showing it.
	 *
	 *  @param cert the certificate presented by the site
	 *  @param ip the ip of the remote host
	 *  @param url the url being accessed
	 *  @param cipher the cipher in use
	 *  @param cipherdesc text description of the cipher in use
	 *  @param sslversion the version of SSL in use (SSLv2, SSLv3, TLSv1, etc)
	 *  @param usedbits the number of bits in the cipher key being used
	 *  @param bits the bit-size of the cipher in use
	 *  @param certState the certificate state (valid, invalid, etc)
	 */
	void setup(KSSLCertificate *cert,
			const TQString& ip, const TQString& url,
			const TQString& cipher, const TQString& cipherdesc,
			const TQString& sslversion, int usedbits, int bits,
			KSSLCertificate::KSSLValidation certState);

	/**
	 *  Setup the dialog before showing it.  This is a convenience version
	 *  of the above method, and obtains the same information using the
	 *  @param ssl parameter instead.
	 *
	 *  @param ssl the ssl connection
	 *  @param ip the ip of the remote host
	 *  @param url the url being accessed
	 */
	void setup( KSSL & ssl, const TQString & ip, const TQString & url );

        /**
         *  Set the errors that were encountered while validating the site 
         *  certificate.
         */
        void setCertState(const TQString &errorNrs);

	/**
	 *  Utility function to generate the widget which displays the detailed
	 *  information about an X.509 certificate.
	 *
	 *  @param parent the parent widget
	 *  @param certName the name (subject) of the certificate
	 *  @param mailCatcher the class which catches click events on e-mail
	 *         addresses
	 */
	static KSSLCertBox *certInfoWidget(TQWidget *parent, const TQString &certName, TQWidget *mailCatcher=0);

private:
	TQScrollView *buildCertInfo(const TQString &certName);
	void displayCert(KSSLCertificate *x);

	class KSSLInfoDlgPrivate;
	KSSLInfoDlgPrivate *d;

private slots:
	void launchConfig();
	void urlClicked(const TQString &url);
	void mailClicked(const TQString &url);
	void slotChain(int x);
};


/**
 * KDE SSL Certificate Box
 *
 * This class creates a widget which formats and displays the contents of an
 * SSL X.509 certificate.  That is, it takes the "subject" of the certificate
 * and displays everything contained therein.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSLInfoDlg
 * @short KDE SSL Certificate Box
 */
class TDEIO_EXPORT KSSLCertBox : public TQScrollView {
public:
	/**
	 *  Construct a certificate box
	 *
	 *  @param parent the parent widget
	 *  @param name the internal name of this instance
	 *  @param f widget flags for the object
	 */
	KSSLCertBox(TQWidget *parent=0L, const char *name=0L, WFlags f=0);

	/**
	 *  Change the contents of the widget
	 *
	 *  @param certName the name ("subject") of the certificate
	 *  @param mailCatcher the widget which catches the url open events
	 */
	void setValues(TQString certName, TQWidget *mailCatcher=0L);

private:
	TQFrame *_frame;
};

#endif

