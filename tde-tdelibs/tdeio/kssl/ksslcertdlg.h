/* This file is part of the KDE project
 *
 * Copyright (C) 2001-2003 George Staikos <staikos@kde.org>
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

#ifndef _KSSLCERTDLG_H
#define _KSSLCERTDLG_H

#include <tqstringlist.h>
#include <kdialog.h>

class TQWidget;
class TQCheckBox;
class TQRadioButton;
class TQListView;
class TQPushButton;

/**
 * KDE X.509 Certificate Dialog
 *
 * This class is used to create and display a dialog which contains the user's
 * X.509 certificates and allows the user to present it during SSL sessions.
 *
 * @author George Staikos <staikos@kde.org>
 * @see KSSL
 * @short KDE X.509 Certificate Dialog
 */
class TDEIO_EXPORT KSSLCertDlg : public KDialog {
	TQ_OBJECT
public:
	/**
	 *  Construct a KSSL certificate dialog
	 *
	 *  @param parent the parent widget
	 *  @param name the internal name of this instance
	 *  @param modal create a modal dialog if set to true
	 */
	KSSLCertDlg(TQWidget *parent=0L, const char *name=0L, bool modal=false);

	/**
	 *  Destroy this object and close the dialog
	 */
	virtual ~KSSLCertDlg();

	/**
	 *  Setup the dialog. Call this before you display the dialog.
	 *
	 *  @param certs the list of possible certificates
	 *  @param saveChecked save the checked item for the future
	 *  @param sendChecked send the checked item to the remote host
	 *  @deprecated
	 */
	void setup(TQStringList certs, bool saveChecked = false, bool sendChecked = true) TDE_DEPRECATED;

	/**
	 *  Setup the dialog. Call this before you display the dialog.
	 *
	 *  @param certs the list of possible certificates
	 *  @param saveChecked save the checked item for the future
	 *  @param sendChecked send the checked item to the remote host
	 */
	void setupDialog(const TQStringList& certs, bool saveChecked = false, bool sendChecked = true);

	/**
	 *  Obtain the name of the certificate the user wants to send
	 *
	 *  @return the name of the certificate
	 */
	TQString getChoice();

	/**
	 *  Determine if the user wants to send a certificate.
	 *
	 *  @return true if the user wants to send a certificate
	 */
	bool wantsToSend();

	/**
	 *  Determine if the user wants to save the choice for the future.
	 *
	 *  @return true if the user wants to save the choice.
	 */
	bool saveChoice();

	/**
	 *  Set the hostname that we are connecting to.
	 *
	 *  @param host the hostname
	 */
	void setHost(const TQString& host);

private slots:
	void slotSend();
	void slotDont();

private:
	class KSSLCertDlgPrivate;
	KSSLCertDlgPrivate *d;
	TQCheckBox *_save;
	TQRadioButton *_send, *_dont;
	TQListView *_certs;
	TQPushButton *_ok;
	TQString _host;
};


class TDEIO_EXPORT KSSLCertDlgRet {
public:
   bool ok;
   TQString choice;
   bool send;
   bool save;

protected:
   class KSSLCertDlgRetPrivate;
   KSSLCertDlgRetPrivate *d;
};

TDEIO_EXPORT TQDataStream& operator<<(TQDataStream& s, const KSSLCertDlgRet& r);
TDEIO_EXPORT TQDataStream& operator>>(TQDataStream& s, KSSLCertDlgRet& r);

#endif

