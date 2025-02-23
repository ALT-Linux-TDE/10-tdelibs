/*
   This file is part of the KDE libraries

   Copyright (c) 2003 George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include "ktimeout.h"

KTimeout::KTimeout(int size)
: TQObject(), _timers(size) {
	_timers.setAutoDelete(true);
}


KTimeout::~KTimeout() {
	clear();
}


void KTimeout::clear() {
	_timers.clear();
}


void KTimeout::removeTimer(int id) {
	TQTimer *t = _timers.find(id);
	if (t != 0L) {
		_timers.remove(id); // autodeletes
	}
}


void KTimeout::addTimer(int id, int timeout) {
	if (_timers.find(id) != 0L) {
		return;
	}

	TQTimer *t = new TQTimer;
	connect(t, TQ_SIGNAL(timeout()), this, TQ_SLOT(timeout()));
	t->start(timeout);
	_timers.insert(id, t);
}


void KTimeout::resetTimer(int id, int timeout) {
	TQTimer *t = _timers.find(id);
	if (t) {
		t->changeInterval(timeout);
	}
}


void KTimeout::timeout() {
	const TQTimer *t = static_cast<const TQTimer*>(sender());
	if (t) {
		TQIntDictIterator<TQTimer> it(_timers);
		for (; it.current(); ++it) {
			if (it.current() == t) {
				emit timedOut(it.currentKey());
				return;
			}
		}
	}
}


#include "ktimeout.moc"

