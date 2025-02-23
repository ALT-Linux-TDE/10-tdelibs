/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
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

#ifndef ESCPWIDGET_H
#define ESCPWIDGET_H

#include <tqwidget.h>
#include <kprocess.h>
#include <kurl.h>

class TQLabel;
class TQCheckBox;

class EscpWidget : public TQWidget
{
	TQ_OBJECT

public:
	EscpWidget(TQWidget *parent = 0, const char *name = 0);
	void setDevice(const TQString&);
	void setPrinterName(const TQString&);

protected slots:
	void slotReceivedStdout(TDEProcess*, char*, int);
	void slotReceivedStderr(TDEProcess*, char*, int);
	void slotProcessExited(TDEProcess*);
	void slotButtonClicked();

protected:
	void startCommand(const TQString& arg);

private:
	TDEProcess	m_proc;
	KURL		m_deviceURL;
	TQString		m_errorbuffer, m_outbuffer;
	TQLabel		*m_printer, *m_device;
	TQCheckBox	*m_useraw;
	bool		m_hasoutput;
};

#endif
