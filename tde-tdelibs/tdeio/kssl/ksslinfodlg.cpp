/* This file is part of the KDE project
 *
 * Copyright (C) 2000,2001 George Staikos <staikos@kde.org>
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

#include "ksslinfodlg.h"

#include <kssl.h>

#include <tqlayout.h>
#include <kpushbutton.h>
#include <tqframe.h>
#include <tqlabel.h>
#include <tqscrollview.h>
#include <tqfile.h>

#include <tdeapplication.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kprocess.h>
#include <kiconloader.h>
#include <tdeglobalsettings.h>
#include <ksqueezedtextlabel.h>
#include <kurllabel.h>
#include <kstdguiitem.h>
//#include <kstandarddirs.h>
//#include <krun.h>
#include <kcombobox.h>
#include "ksslcertificate.h"
#include "ksslcertchain.h"
#include "ksslsigners.h"


class KSSLInfoDlg::KSSLInfoDlgPrivate {
    private:
        friend class KSSLInfoDlg;
        bool m_secCon;
        TQGridLayout *m_layout;
        KComboBox *_chain;
        KSSLCertificate *_cert;
        KSSLCertificate::KSSLValidationList _cert_ksvl;

        bool inQuestion;

        TQLabel *_serialNum;
        TQLabel *_csl;
        TQLabel *_validFrom;
        TQLabel *_validUntil;
        TQLabel *_digest;

        TQLabel *pixmap;
        TQLabel *info;

        KSSLCertBox *_subject, *_issuer;
};



KSSLInfoDlg::KSSLInfoDlg(bool secureConnection, TQWidget *parent, const char *name, bool modal)
    : KDialog(parent, name, modal, (WFlags)TQt::WDestructiveClose), d(new KSSLInfoDlgPrivate) {
        TQVBoxLayout *topLayout = new TQVBoxLayout(this, KDialog::marginHint(), KDialog::spacingHint());
        d->m_secCon = secureConnection;
        d->m_layout = new TQGridLayout(topLayout, 3, 3, KDialog::spacingHint());
        d->m_layout->setColStretch(1, 1);
        d->m_layout->setColStretch(2, 1);

        d->pixmap = new TQLabel(this);
        d->m_layout->addWidget(d->pixmap, 0, 0);

        d->info = new TQLabel(this);
        d->m_layout->addWidget(d->info, 0, 1);

        if (KSSL::doesSSLWork()) {
            if (d->m_secCon) {
                d->pixmap->setPixmap(BarIcon("encrypted"));
                d->info->setText(i18n("Current connection is secured with SSL."));
            } else {
                d->pixmap->setPixmap(BarIcon("decrypted"));
                d->info->setText(i18n("Current connection is not secured with SSL."));
            }
        } else {
            d->pixmap->setPixmap(BarIcon("decrypted"));
            d->info->setText(i18n("SSL support is not available in this build of TDE."));
        }
        d->m_layout->addRowSpacing( 0, 50 ); // give minimum height to look better

        TQHBoxLayout *buttonLayout = new TQHBoxLayout(topLayout, KDialog::spacingHint());
        buttonLayout->addStretch( 1 );

        KPushButton *button;

        if (KSSL::doesSSLWork()) {
            button = new KPushButton(KGuiItem(i18n("C&ryptography Configuration..."),"configure"), this);
            connect(button, TQ_SIGNAL(clicked()), TQ_SLOT(launchConfig()));
            buttonLayout->addWidget( button );
        }

        button = new KPushButton(KStdGuiItem::close(), this);
        connect(button, TQ_SIGNAL(clicked()), TQ_SLOT(close()));
        buttonLayout->addWidget( button );

        button->setFocus();

        setCaption(i18n("TDE SSL Information"));
        d->inQuestion = false;
    }


KSSLInfoDlg::~KSSLInfoDlg() {
    delete d;
}

void KSSLInfoDlg::launchConfig() {
    TDEProcess p;
    p << "tdecmshell" << "crypto";
    p.start(TDEProcess::DontCare);
}


void KSSLInfoDlg::setSecurityInQuestion(bool isIt) {
	d->inQuestion = isIt;
	if (KSSL::doesSSLWork()) {
		if (isIt) {
			d->pixmap->setPixmap(BarIcon("halfencrypted"));
			if (d->m_secCon) {
				d->info->setText(i18n("The main part of this document is secured with SSL, but some parts are not."));
			}
			else {
				d->info->setText(i18n("Some of this document is secured with SSL, but the main part is not."));
			}
		}
		else {
			if (d->m_secCon) {
				d->pixmap->setPixmap(BarIcon("encrypted"));
				d->info->setText(i18n("Current connection is secured with SSL."));
			}
			else {
				d->pixmap->setPixmap(BarIcon("decrypted"));
				d->info->setText(i18n("Current connection is not secured with SSL."));
			}
		}
	}
}


void KSSLInfoDlg::setup( KSSL & ssl, const TQString & ip, const TQString & url )
{
    setup(
            &ssl.peerInfo().getPeerCertificate(),
            ip,
            url,
            ssl.connectionInfo().getCipher(),
            ssl.connectionInfo().getCipherDescription(),
            ssl.connectionInfo().getCipherVersion(),
            ssl.connectionInfo().getCipherUsedBits(),
            ssl.connectionInfo().getCipherBits(),
            ssl.peerInfo().getPeerCertificate().validate()
         );
}

void KSSLInfoDlg::setup(KSSLCertificate *cert,
        const TQString& ip, const TQString& url,
        const TQString& cipher, const TQString& cipherdesc,
        const TQString& sslversion, int usedbits, int bits,
        KSSLCertificate::KSSLValidation /*certState*/) {
    // Needed to put the GUI stuff here to get the layouting right

    d->_cert = cert;

    TQGridLayout *layout = new TQGridLayout(4, 2, KDialog::spacingHint());

    layout->addWidget(new TQLabel(i18n("Chain:"), this), 0, 0);
    d->_chain = new KComboBox(this);
    layout->addMultiCellWidget(d->_chain, 1, 1, 0, 1);
    connect(d->_chain, TQ_SIGNAL(activated(int)), this, TQ_SLOT(slotChain(int)));

    d->_chain->clear();

    if (cert->chain().isValid() && cert->chain().depth() > 1) {
        d->_chain->setEnabled(true);
        d->_chain->insertItem(i18n("0 - Site Certificate"));
        int cnt = 0;
        TQPtrList<KSSLCertificate> cl = cert->chain().getChain();
        cl.setAutoDelete(true);
        for (KSSLCertificate *c = cl.first(); c != 0; c = cl.next()) {
            KSSLX509Map map(c->getSubject());
            TQString id;
            id = map.getValue("CN");
            if (id.length() == 0)
                id = map.getValue("O");
            if (id.length() == 0)
                id = map.getValue("OU");
            d->_chain->insertItem(TQString::number(++cnt)+" - "+id);
        }
        d->_chain->setCurrentItem(0);
    } else d->_chain->setEnabled(false);

    layout->addWidget(new TQLabel(i18n("Peer certificate:"), this), 2, 0);
    layout->addWidget(d->_subject = static_cast<KSSLCertBox*>(buildCertInfo(cert->getSubject())), 3, 0);
    layout->addWidget(new TQLabel(i18n("Issuer:"), this), 2, 1);
    layout->addWidget(d->_issuer = static_cast<KSSLCertBox*>(buildCertInfo(cert->getIssuer())), 3, 1);
    d->m_layout->addMultiCell(layout, 1, 1, 0, 2);

    layout = new TQGridLayout(11, 2, KDialog::spacingHint());
    layout->setColStretch(1, 1);
    TQLabel *ipl = new TQLabel(i18n("IP address:"), this);
    layout->addWidget(ipl, 0, 0);
    if (ip.isEmpty()) {
        ipl->hide();
    }
    layout->addWidget(ipl = new TQLabel(ip, this), 0, 1);
    if (ip.isEmpty()) {
        ipl->hide();
    }
    layout->addWidget(new TQLabel(i18n("URL:"), this), 1, 0);
    KSqueezedTextLabel *urlLabel = new KSqueezedTextLabel(url, this);
    layout->addWidget(urlLabel, 1, 1);
    layout->addWidget(new TQLabel(i18n("Certificate state:"), this), 2, 0);

    layout->addWidget(d->_csl = new TQLabel("", this), 2, 1);

    update();

    layout->addWidget(new TQLabel(i18n("Valid from:"), this), 3, 0);
    layout->addWidget(d->_validFrom = new TQLabel("", this), 3, 1);
    layout->addWidget(new TQLabel(i18n("Valid until:"), this), 4, 0);
    layout->addWidget(d->_validUntil = new TQLabel("", this), 4, 1);

    layout->addWidget(new TQLabel(i18n("Serial number:"), this), 5, 0);
    layout->addWidget(d->_serialNum = new TQLabel("", this), 5, 1);
    layout->addWidget(new TQLabel(i18n("MD5 digest:"), this), 6, 0);
    layout->addWidget(d->_digest = new TQLabel("", this), 6, 1);

    layout->addWidget(new TQLabel(i18n("Cipher in use:"), this), 7, 0);
    layout->addWidget(new TQLabel(cipher, this), 7, 1);
    layout->addWidget(new TQLabel(i18n("Details:"), this), 8, 0);
    layout->addWidget(new TQLabel(cipherdesc.simplifyWhiteSpace(), this), 8, 1);
    layout->addWidget(new TQLabel(i18n("SSL version:"), this), 9, 0);
    layout->addWidget(new TQLabel(sslversion, this), 9, 1);
    layout->addWidget(new TQLabel(i18n("Cipher strength:"), this), 10, 0);
    layout->addWidget(new TQLabel(i18n("%1 bits used of a %2 bit cipher").arg(usedbits).arg(bits), this), 10, 1);
    d->m_layout->addMultiCell(layout, 2, 2, 0, 2);

    ipl->setTextFormat(TQt::PlainText);
    urlLabel->setTextFormat(TQt::PlainText);
    d->_serialNum->setTextFormat(TQt::PlainText);
    d->_csl->setTextFormat(TQt::PlainText);
    d->_validFrom->setTextFormat(TQt::PlainText);
    d->_validUntil->setTextFormat(TQt::PlainText);
    d->_digest->setTextFormat(TQt::PlainText);

    displayCert(cert);
}

void KSSLInfoDlg::setCertState(const TQString &errorNrs)
{
    d->_cert_ksvl.clear();
    TQStringList errors = TQStringList::split(':', errorNrs);
    for(TQStringList::ConstIterator it = errors.begin();
            it != errors.end(); ++it)
    {
        d->_cert_ksvl << (KSSLCertificate::KSSLValidation) (*it).toInt();
    }
}

void KSSLInfoDlg::displayCert(KSSLCertificate *x) {
    TQPalette cspl;

    d->_serialNum->setText(x->getSerialNumber());

    cspl = d->_validFrom->palette();
    if (x->getQDTNotBefore() > TQDateTime::currentDateTime(TQt::UTC))
        cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
    else cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
    d->_validFrom->setPalette(cspl);
    d->_validFrom->setText(x->getNotBefore());

    cspl = d->_validUntil->palette();
    if (x->getQDTNotAfter() < TQDateTime::currentDateTime(TQt::UTC))
        cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
    else cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
    d->_validUntil->setPalette(cspl);
    d->_validUntil->setText(x->getNotAfter());

    cspl = palette();

    KSSLCertificate::KSSLValidation ksv;
    KSSLCertificate::KSSLValidationList ksvl;
    if ((x == d->_cert) && !d->_cert_ksvl.isEmpty()) {
        ksvl = d->_cert_ksvl;
        ksv = ksvl.first();
    } else {
        if (x == d->_cert)
            ksvl = d->_cert->validateVerbose(KSSLCertificate::SSLServer);
        else
            ksvl = d->_cert->validateVerbose(KSSLCertificate::SSLServer, x);

        if (ksvl.isEmpty())
            ksvl << KSSLCertificate::Ok;

        ksv = ksvl.first();

        if (ksv == KSSLCertificate::SelfSigned) {
            if (x->getQDTNotAfter() > TQDateTime::currentDateTime(TQt::UTC) &&
                    x->getQDTNotBefore() < TQDateTime::currentDateTime(TQt::UTC)) {
                if (KSSLSigners().useForSSL(*x))
                    ksv = KSSLCertificate::Ok;
            } else {
                ksv = KSSLCertificate::Expired;
            }
        }
    }

    if (ksv == KSSLCertificate::Ok) {
        cspl.setColor(TQColorGroup::Foreground, TQColor(42,153,59));
    } else if (ksv != KSSLCertificate::Irrelevant) {
        cspl.setColor(TQColorGroup::Foreground, TQColor(196,33,21));
    }
    d->_csl->setPalette(cspl);

    TQString errorStr;
    for(KSSLCertificate::KSSLValidationList::ConstIterator it = ksvl.begin();
            it != ksvl.end(); ++it) {
        if (!errorStr.isEmpty())
            errorStr.append('\n');
        errorStr += KSSLCertificate::verifyText(*it);
    }

    d->_csl->setText(errorStr);
    d->_csl->setMinimumSize(d->_csl->sizeHint());

    d->_subject->setValues(x->getSubject());
    d->_issuer->setValues(x->getIssuer());

    d->_digest->setText(x->getMD5DigestText());
}


void KSSLInfoDlg::slotChain(int x) {
    if (x == 0) {
        displayCert(d->_cert);
    } else {
        TQPtrList<KSSLCertificate> cl = d->_cert->chain().getChain();
        cl.setAutoDelete(true);
        for (int i = 0; i < x-1; i++)
            cl.remove((unsigned int)0);
        KSSLCertificate thisCert = *(cl.at(0));
        cl.remove((unsigned int)0);
        thisCert.chain().setChain(cl);
        displayCert(&thisCert);
    }
}


KSSLCertBox *KSSLInfoDlg::certInfoWidget(TQWidget *parent, const TQString &certName, TQWidget *mailCatcher) {
    KSSLCertBox *result = new KSSLCertBox(parent);
    if (!certName.isEmpty()) {
        result->setValues(certName, mailCatcher);
    }
    return result;
}


KSSLCertBox::KSSLCertBox(TQWidget *parent, const char *name, WFlags f)
: TQScrollView(parent, name, f)
{
    _frame = 0L;
    setBackgroundMode(TQWidget::PaletteButton);
    setValues(TQString::null, 0L);
}


void KSSLCertBox::setValues(TQString certName, TQWidget *mailCatcher) {
    if (_frame) {
        removeChild(_frame);
        delete _frame;
    }

    if (certName.isEmpty()) {
        _frame = new TQFrame(this);
        addChild(_frame);
        viewport()->setBackgroundMode(_frame->backgroundMode());
        _frame->show();
        updateScrollBars();
        show();
        return;
    }

    KSSLX509Map cert(certName);
    TQString tmp;
    viewport()->setBackgroundMode(TQWidget::PaletteButton);
    _frame = new TQFrame(this);
    TQGridLayout *grid = new TQGridLayout(_frame, 1, 2, KDialog::marginHint(), KDialog::spacingHint());
    grid->setAutoAdd(true);
    TQLabel *label = 0L;
    if (!(tmp = cert.getValue("O")).isEmpty()) {
        label = new TQLabel(i18n("Organization:"), _frame);
        label->setAlignment(TQt::AlignLeft | TQt::AlignTop);
        (new TQLabel(tmp, _frame))->setTextFormat(TQt::PlainText);
    }
    if (!(tmp = cert.getValue("OU")).isEmpty()) {
        label = new TQLabel(i18n("Organizational unit:"), _frame);
        label->setAlignment(TQt::AlignLeft | TQt::AlignTop);
        (new TQLabel(tmp, _frame))->setTextFormat(TQt::PlainText);
    }
    if (!(tmp = cert.getValue("L")).isEmpty()) {
        label = new TQLabel(i18n("Locality:"), _frame);
        label->setAlignment(TQt::AlignLeft | TQt::AlignTop);
        (new TQLabel(tmp, _frame))->setTextFormat(TQt::PlainText);
    }
    if (!(tmp = cert.getValue("ST")).isEmpty()) {
        label = new TQLabel(i18n("Federal State","State:"), _frame);
        label->setAlignment(TQt::AlignLeft | TQt::AlignTop);
        (new TQLabel(tmp, _frame))->setTextFormat(TQt::PlainText);
    }
    if (!(tmp = cert.getValue("C")).isEmpty()) {
        label = new TQLabel(i18n("Country:"), _frame);
        label->setAlignment(TQt::AlignLeft | TQt::AlignTop);
        (new TQLabel(tmp, _frame))->setTextFormat(TQt::PlainText);
    }
    if (!(tmp = cert.getValue("CN")).isEmpty()) {
        label = new TQLabel(i18n("Common name:"), _frame);
        label->setAlignment(TQt::AlignLeft | TQt::AlignTop);
        (new TQLabel(tmp, _frame))->setTextFormat(TQt::PlainText);
    }
    if (!(tmp = cert.getValue("Email")).isEmpty()) {
        label = new TQLabel(i18n("Email:"), _frame);
        label->setAlignment(TQt::AlignLeft | TQt::AlignTop);
        if (mailCatcher) {
            KURLLabel *mail = new KURLLabel(tmp, tmp, _frame);
            connect(mail, TQ_SIGNAL(leftClickedURL(const TQString &)), mailCatcher, TQ_SLOT(mailClicked(const TQString &)));
        } else {
            label = new TQLabel(tmp, _frame);
            label->setTextFormat(TQt::PlainText);
        }
    }
    if (label && viewport()) {
        viewport()->setBackgroundMode(label->backgroundMode());
    }
    addChild(_frame);
    updateScrollBars();
    _frame->show();
    show();
}


TQScrollView *KSSLInfoDlg::buildCertInfo(const TQString &certName) {
    return KSSLInfoDlg::certInfoWidget(this, certName, this);
}

void KSSLInfoDlg::urlClicked(const TQString &url) {
    kapp->invokeBrowser(url);
}

void KSSLInfoDlg::mailClicked(const TQString &url) {
    kapp->invokeMailer(url, TQString::null);
}

#include "ksslinfodlg.moc"
