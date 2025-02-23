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

#ifndef NETWORKSCANNER_H
#define NETWORKSCANNER_H

#include <tqwidget.h>
#include <kdialogbase.h>
#include <tqptrlist.h>


class TDEPRINT_EXPORT NetworkScanner : public TQWidget
{
	TQ_OBJECT

public:
	NetworkScanner( int port = 9100, TQWidget *parent = 0, const char *name = 0 );
	~NetworkScanner();

	struct SocketInfo
	{
		TQString IP;
		TQString Name;
		int Port;
	};

	const TQPtrList<NetworkScanner::SocketInfo>* printerList();

	int timeout() const;
	void setTimeout( int to );
	TQString subnet() const;
	void setSubnet( const TQString& );
	int port() const;
	void setPort( int );
	bool checkPrinter( const TQString&, int );

signals:
	void scanStarted();
	void scanFinished();

protected slots:
	void slotConnectionSuccess();
	void slotConnectionFailed( int );
	void slotTimeout();
	void slotScanClicked();
	void slotSettingsClicked();
	void slotNext();

protected:
	void next();
	void finish();
	void start();

private:
	class NetworkScannerPrivate;
	NetworkScannerPrivate *d;
};

class TQLineEdit;
class TQComboBox;

class NetworkScannerConfig : public KDialogBase
{
	TQ_OBJECT

public:
	NetworkScannerConfig(NetworkScanner *scanner, const char *name = 0);
	~NetworkScannerConfig();

protected slots:
	void slotOk();

private:
	TQLineEdit	*mask_, *tout_;
	TQComboBox	*port_;
	NetworkScanner *scanner_;
};

#endif
