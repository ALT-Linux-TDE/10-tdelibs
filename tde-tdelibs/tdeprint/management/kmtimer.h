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

#ifndef KMTIMER_H
#define KMTIMER_H

#if !defined( _TDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a TDEPrint developer
#endif

#include <tqtimer.h>
#include <tdelibs_export.h>

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT KMTimer : public TQTimer
{
	TQ_OBJECT
public:
	void delay(int t);
	~KMTimer();

public slots:
	void hold();
	void release();
	void release(bool do_emit);

	static KMTimer* self();

private slots:
	void slotTimeout();

private:
	static KMTimer	*m_self;
	int	m_count;
	KMTimer(TQObject *parent = 0, const char *name = 0);
	void releaseTimer(bool do_emit = false);
	void startTimer(int delay = -1);
};

#endif
