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
#ifndef _INCLUDE_TDECERTPART_H
#define _INCLUDE_TDECERTPART_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tdeparts/part.h>
#include <tqptrlist.h>
#include <tqlistview.h>
#include <tdelistview.h>

class KSSLCertBox;
class TQFrame;
class TQLabel;
class KCertPartPrivate;
class KComboBox;
class TQButton;
class KSSLSigners;
class KSSLPKCS12;
class KSSLCertificate;
class TQTabWidget;
class TQMultiLineEdit;
class TDEAboutData;
class TQGridLayout;


class KX509Item : public TDEListViewItem {
	public:
		KX509Item(TDEListViewItem *parent, KSSLCertificate *x);
		KX509Item(TDEListView *parent, KSSLCertificate *x);
		void setup(KSSLCertificate *x);
		~KX509Item();
		virtual int rtti() const { return 1; }
	KSSLCertificate *cert;
	TQString _prettyName;
};


class KPKCS12Item : public TDEListViewItem {
	public:
		KPKCS12Item(TDEListViewItem *parent, KSSLPKCS12 *x);
		~KPKCS12Item();
	KSSLPKCS12 *cert;
	TQString _prettyName;
};


class KCertPart : public KParts::ReadWritePart {
TQ_OBJECT
public:
  KCertPart(TQWidget *parentWidget, const char *widgetName,
            TQObject *parent = 0L, const char *name = 0L,
	    const TQStringList &args = TQStringList() );
  virtual ~KCertPart();

  virtual void setReadWrite(bool readwrite);

  static TDEAboutData *createAboutData();

protected slots:
  void slotChain(int c);
  void slotImport();
  void slotSave();
  void slotDone();
  void slotLaunch();
  void slotSelectionChanged(TQListViewItem *x);
  void slotImportAll();

protected:

  virtual bool openFile();
  virtual bool saveFile();

  void displayPKCS12Cert(KSSLCertificate *c);
  void displayCACert(KSSLCertificate *c);

  TDEListView *_sideList;
  TDEListViewItem *_parentCA, *_parentP12;
  TQFrame *_pkcsFrame, *_blankFrame, *_x509Frame, *_frame;

  // for the PKCS12 widget
  TQLabel *_p12_filenameLabel, *_p12_validFrom, *_p12_validUntil, 
         *_p12_serialNum, *_p12_certState;
  TQLabel *_p12_digest;
  KComboBox *_p12_chain;
  TQMultiLineEdit *_p12_pubkey, *_p12_sig;
  KSSLCertBox *_p12_subject, *_p12_issuer;

  // for the CA widget
  TQLabel *_ca_filenameLabel, *_ca_validFrom, *_ca_validUntil, 
         *_ca_serialNum, *_ca_certState;
  TQLabel *_ca_digest;
  TQMultiLineEdit *_ca_pubkey, *_ca_sig;
  KSSLCertBox *_ca_subject, *_ca_issuer;


  // The rest
  TDEInstance *_instance;
  TQButton *_import, *_save, *_done, *_launch, *_importAll;
  // Store the pointer to the current item
  KSSLPKCS12 *_p12;
  KSSLCertificate *_ca;
  TQTabWidget *_tabs;
  TQGridLayout *_baseGrid;
  KSSLSigners *_signers;
  bool _silentImport;
  TQString _curName;

private:
  KCertPartPrivate *d;
  void displayPKCS12();
};




#endif


