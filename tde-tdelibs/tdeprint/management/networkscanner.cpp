/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#define USE_QSOCKET

#include "networkscanner.h"

#include <tqprogressbar.h>
#include <kpushbutton.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <tqlabel.h>
#include <tqcombobox.h>
#include <tqlineedit.h>
#include <tqregexp.h>
#include <tqsocket.h>
#include <tdelocale.h>
#include <kextendedsocket.h>
#include <tdemessagebox.h>
#include <knumvalidator.h>
#include <kdebug.h>
#include <unistd.h>

class NetworkScanner::NetworkScannerPrivate
{
public:
	int port;
	TQString prefixaddress;
	int currentaddress;
	int timeout;
	bool scanning;
	TQPtrList<NetworkScanner::SocketInfo> printers;

	TQProgressBar *bar;
	KPushButton *scan, *settings;
	TQLabel *subnetlab;
	TQTimer *timer;
#ifdef USE_QSOCKET
	TQSocket *socket;
#else
	KExtendedSocket *socket;
#endif

	NetworkScannerPrivate( int portvalue ) : port( portvalue )
	{
		prefixaddress = localPrefix();
		currentaddress = 1;
		timeout = 50;
		scanning = false;
		printers.setAutoDelete( true );
	}
	TQString localPrefix();
	TQString scanString();
};

TQString NetworkScanner::NetworkScannerPrivate::localPrefix()
{
	char	buf[256];
	buf[0] = '\0';
	if (!gethostname(buf, sizeof(buf)))
		buf[sizeof(buf)-1] = '\0';
	TQPtrList<KAddressInfo>	infos = KExtendedSocket::lookup(buf, TQString::null);
	infos.setAutoDelete(true);
	if (infos.count() > 0)
	{
		TQString	IPstr = infos.first()->address()->nodeName();
		int	p = IPstr.findRev('.');
		IPstr.truncate(p);
		return IPstr;
	}
	return TQString::null;
}

TQString NetworkScanner::NetworkScannerPrivate::scanString()
{
	TQString s = prefixaddress + ".*";
	if ( port != -1 )
		s.append( ":" ).append( TQString::number( port ) );
	return s;
}

NetworkScanner::NetworkScanner( int port, TQWidget *parent, const char *name )
	: TQWidget( parent, name )
{
	d = new NetworkScannerPrivate( port );
	d->bar = new TQProgressBar( 256, this );
	d->settings = new KPushButton( KGuiItem( i18n( "&Settings" ), "configure" ), this );
	d->scan = new KPushButton( KGuiItem( i18n( "Sc&an" ), "viewmag" ), this );
	d->timer = new TQTimer( this );
#ifdef USE_QSOCKET
	d->socket = new TQSocket( this );
#else
	d->socket = new KExtendedSocket();
#endif
	TQLabel *label = new TQLabel( i18n( "Network scan:" ), this );
	d->subnetlab = new TQLabel( i18n( "Subnet: %1" ).arg( d->scanString() ), this );

	TQGridLayout *l0 = new TQGridLayout( this, 4, 2, 0, 10 );
	l0->addMultiCellWidget( label, 0, 0, 0, 1 );
	l0->addMultiCellWidget( d->bar, 1, 1, 0, 1 );
	l0->addMultiCellWidget( d->subnetlab, 2, 2, 0, 1 );
	l0->addWidget( d->settings, 3, 0 );
	l0->addWidget( d->scan, 3, 1 );

	connect( d->timer, TQ_SIGNAL( timeout() ), TQ_SLOT( slotTimeout() ) );
	connect( d->settings, TQ_SIGNAL( clicked() ), TQ_SLOT( slotSettingsClicked() ) );
	connect( d->scan, TQ_SIGNAL( clicked() ), TQ_SLOT( slotScanClicked() ) );
#ifdef USE_QSOCKET
	connect( d->socket, TQ_SIGNAL( connected() ), TQ_SLOT( slotConnectionSuccess() ) );
	connect( d->socket, TQ_SIGNAL( error( int ) ), TQ_SLOT( slotConnectionFailed( int ) ) );
#else
	connect( d->socket, TQ_SIGNAL( connectionSuccess() ), TQ_SLOT( slotConnectionSuccess() ) );
	connect( d->socket, TQ_SIGNAL( connectionFailed( int ) ), TQ_SLOT( slotConnectionFailed( int ) ) );
#endif
}

NetworkScanner::~NetworkScanner()
{
#ifndef USE_QSOCKET
	delete d->socket;
#endif
	delete d;
}

void NetworkScanner::start()
{
	if ( d->scanning )
		return;

	d->printers.clear();
	emit scanStarted();
	d->settings->setEnabled( false );
	d->scan->setGuiItem( KGuiItem( i18n( "&Abort" ), "process-stop" ) );
	d->currentaddress = -1;
	d->scanning = true;
	next();
}

void NetworkScanner::slotScanClicked()
{
	if ( !d->scanning )
	{
		if ( d->localPrefix() == d->prefixaddress ||
				KMessageBox::warningContinueCancel( this->parentWidget(),
					i18n( "You are about to scan a subnet (%1.*) that does not "
						  "correspond to the current subnet of this computer (%2.*). Do you want "
						  "to scan the specified subnet anyway?" ).arg( d->prefixaddress ).arg( d->localPrefix() ),
					TQString::null, KGuiItem( i18n( "&Scan" ), "viewmag" ), "askForScan" ) == KMessageBox::Continue )
			start();
	}
	else
	{
#ifdef USE_QSOCKET
		d->socket->close();
#else
		d->socket->cancelAsyncConnect();
#endif
		finish();
	}
}

void NetworkScanner::finish()
{
	if ( !d->scanning )
		return;

	d->settings->setEnabled( true );
	d->scan->setGuiItem( KGuiItem( i18n( "Sc&an" ), "viewmag" ) );
	d->bar->reset();
	d->scanning = false;
	emit scanFinished();
}

void NetworkScanner::slotSettingsClicked()
{
	NetworkScannerConfig dlg( this );
	dlg.exec();
}

void NetworkScanner::slotNext()
{
	if ( !d->scanning )
		return;

	d->timer->stop();
#ifdef USE_QSOCKET
	d->socket->connectToHost( d->prefixaddress + "." + TQString::number( d->currentaddress ), d->port );
	kdDebug() << "Address: " << d->socket->peerName() << ", Port: " << d->socket->peerPort() << endl;
#else
	d->socket->setAddress( d->prefixaddress + "." + TQString::number( d->currentaddress ), d->port );
	d->socket->startAsyncLookup();
	kdDebug() << "Address: " << d->socket->host() << ", Port: " << d->socket->port() << endl;
#endif
	d->timer->start( d->timeout, true );
}

void NetworkScanner::next()
{
	//kdDebug() << "Next" << endl;
	d->currentaddress++;
	if ( d->currentaddress >= 256 )
		finish();
	else
	{
		d->bar->setProgress( d->currentaddress );
		TQTimer::singleShot( 0, this, TQ_SLOT( slotNext() ) );
	}
}

void NetworkScanner::slotTimeout()
{
	kdDebug() << "Timeout" << endl;
	if ( !d->scanning )
		return;

#ifdef USE_QSOCKET
	d->socket->close();
#else
	d->socket->cancelAsyncConnect();
#endif
	next();
}

void NetworkScanner::slotConnectionSuccess()
{
	kdDebug() << "Success" << endl;
#ifdef USE_QSOCKET
	TDESocketAddress *addr = KExtendedSocket::peerAddress( d->socket->socket() );
#else
	TDESocketAddress *addr = const_cast<TDESocketAddress*>( d->socket->peerAddress() );
#endif
	kdDebug() << "Connection success: " << ( addr ? addr->pretty() : TQString( "ERROR" ) ) << endl;
	kdDebug() << "Socket: " << d->socket->socket() << endl;
	if ( addr )
	{
		SocketInfo *info = new SocketInfo;
#ifdef USE_QSOCKET
		info->IP = d->socket->peerName();
#else
		info->IP = d->socket->host();
#endif
		info->Port = d->port;
		TQString portname;
		KExtendedSocket::resolve( addr, info->Name, portname );
		d->printers.append( info );
		d->socket->close();
		delete addr;
	}
	else
		kdDebug() << "Unconnected socket, skipping" << endl;
	next();
}

void NetworkScanner::slotConnectionFailed( int )
{
	kdDebug() << "Failure" << endl;
	next();
}

const TQPtrList<NetworkScanner::SocketInfo>* NetworkScanner::printerList()
{
	return &( d->printers );
}

int NetworkScanner::timeout() const
{
	return d->timeout;
}

void NetworkScanner::setTimeout( int to )
{
	d->timeout = to;
}

TQString NetworkScanner::subnet() const
{
	return d->prefixaddress;
}

void NetworkScanner::setSubnet( const TQString& sn )
{
	d->prefixaddress = sn;
	d->subnetlab->setText( i18n( "Subnet: %1" ).arg( d->scanString() ) );
}

int NetworkScanner::port() const
{
	return d->port;
}

void NetworkScanner::setPort( int p )
{
	d->port = p;
	d->subnetlab->setText( i18n( "Subnet: %1" ).arg( d->scanString() ) );
}

bool NetworkScanner::checkPrinter( const TQString& host, int port )
{
	// try first to find it in the SocketInfo list
	TQPtrListIterator<NetworkScanner::SocketInfo> it( d->printers );
	for ( ; it.current(); ++it )
	{
		if ( port == it.current()->Port && ( host == it.current()->IP ||
					host == it.current()->Name ) )
			return true;
	}

	// not found in SocketInfo list, try to establish connection
	KExtendedSocket extsock( host, port );
	extsock.setBlockingMode( false );
	extsock.setTimeout( 0, d->timeout * 1000 );
	return ( extsock.connect() == 0 );
}

NetworkScannerConfig::NetworkScannerConfig(NetworkScanner *scanner, const char *name)
	: KDialogBase(scanner, name, true, TQString::null, Ok|Cancel, Ok, true)
{
	scanner_ = scanner;
	TQWidget	*dummy = new TQWidget(this);
	setMainWidget(dummy);
        KIntValidator *val = new KIntValidator( this );
	TQLabel	*masklabel = new TQLabel(i18n("&Subnetwork:"),dummy);
	TQLabel	*portlabel = new TQLabel(i18n("&Port:"),dummy);
	TQLabel	*toutlabel = new TQLabel(i18n("&Timeout (ms):"),dummy);
	TQLineEdit	*mm = new TQLineEdit(dummy);
	mm->setText(TQString::fromLatin1(".[0-255]"));
	mm->setReadOnly(true);
	mm->setFixedWidth(fontMetrics().width(mm->text())+10);

	mask_ = new TQLineEdit(dummy);
	mask_->setAlignment(TQt::AlignRight);
	port_ = new TQComboBox(true,dummy);
        if ( port_->lineEdit() )
            port_->lineEdit()->setValidator( val );
	tout_ = new TQLineEdit(dummy);
        tout_->setValidator( val );

	masklabel->setBuddy(mask_);
	portlabel->setBuddy(port_);
	toutlabel->setBuddy(tout_);

	mask_->setText(scanner_->subnet());
	port_->insertItem("631");
	port_->insertItem("9100");
	port_->insertItem("9101");
	port_->insertItem("9102");
	port_->setEditText(TQString::number(scanner_->port()));
	tout_->setText(TQString::number(scanner_->timeout()));

	TQGridLayout	*main_ = new TQGridLayout(dummy, 3, 2, 0, 10);
	TQHBoxLayout	*lay1 = new TQHBoxLayout(0, 0, 5);
	main_->addWidget(masklabel, 0, 0);
	main_->addWidget(portlabel, 1, 0);
	main_->addWidget(toutlabel, 2, 0);
	main_->addLayout(lay1, 0, 1);
	main_->addWidget(port_, 1, 1);
	main_->addWidget(tout_, 2, 1);
	lay1->addWidget(mask_,1);
	lay1->addWidget(mm,0);

	resize(250,130);
	setCaption(i18n("Scan Configuration"));
}

NetworkScannerConfig::~NetworkScannerConfig()
{
}

void NetworkScannerConfig::slotOk()
{
	TQString	msg;
	TQRegExp	re("(\\d{1,3})\\.(\\d{1,3})\\.(\\d{1,3})");
	if (!re.exactMatch(mask_->text()))
		msg = i18n("Wrong subnetwork specification.");
	else
	{
		for (int i=1; i<=3; i++)
			if (re.cap(i).toInt() >= 255)
			{
				msg = i18n("Wrong subnetwork specification.");
				break;
			}
	}

	bool 	ok(false);
	int 	v = tout_->text().toInt(&ok);
	if (!ok || v <= 0)
		msg = i18n("Wrong timeout specification.");
	v = port_->currentText().toInt(&ok);
	if (!ok || v <= 0)
		msg = i18n("Wrong port specification.");
	if (!msg.isEmpty())
	{
		KMessageBox::error(this,msg);
		return;
	}

	scanner_->setTimeout( tout_->text().toInt() );
	scanner_->setSubnet( mask_->text() );
	scanner_->setPort( port_->currentText().toInt() );

	KDialogBase::slotOk();
}

#include "networkscanner.moc"
