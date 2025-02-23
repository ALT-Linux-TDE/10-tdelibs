/*
    Copyright (C) 2004, Arend van Beelen jr. <arend@auton.nl>
    This file is part of the KDE project

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KFINDTEST_H
#define KFINDTEST_H

#include <tqobject.h>
#include <tqstringlist.h>

class KFind;

class KFindTest : public TQObject
{
	TQ_OBJECT

	public:
		KFindTest(const TQStringList &text) :
		  TQObject(0),
		  m_find(0),
		  m_text(text),
		  m_line(0)
		{}

		void find(const TQString &pattern, long options = 0);
		void findNext(const TQString &pattern = TQString::null);

		void changeText(uint line, const TQString &text);

		const TQStringList &hits() const { return m_hits; }
		void clearHits() { m_hits.clear(); }

	public slots:
		void slotHighlight(const TQString &text, int index, int matchedLength);
		void slotHighlight(int id, int index, int matchedLengthlength);

	private:
		KFind                 *m_find;
		TQStringList            m_text;
		uint                   m_line;
		TQStringList            m_hits;
};

#endif
