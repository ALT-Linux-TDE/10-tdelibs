/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <tdeprint@swing.be>
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

#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <tqwidget.h>
#include <tqptrdict.h>

#include <tdelibs_export.h>

class TQLabel;

class TDEPRINT_EXPORT MessageWindow : public TQWidget
{
	TQ_OBJECT

public:
	~MessageWindow();

	static void add( TQWidget *parent, const TQString& txt, int delay = 500 );
	static void change( TQWidget *parent, const TQString& txt );
	static void remove( TQWidget *parent );
	static void removeAll();

protected slots:
	void slotTimer();

protected:
	MessageWindow( const TQString& txt, int delay = 500, TQWidget *parent = 0, const char *name = 0 );
	void setText( const TQString& txt );
	TQString text() const;

private:
	TQLabel *m_text;
	static TQPtrDict<MessageWindow> m_windows;
};

#endif
