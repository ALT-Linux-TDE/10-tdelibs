/* This file is part of the KDE project
 *
 * Copyright (C) 2000 George Staikos <staikos@kde.org>
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

#include "ksslx509map.h"
#include <tqstringlist.h>
#include <tqregexp.h>

KSSLX509Map::KSSLX509Map(const TQString& name) {
  parse(name);
}


KSSLX509Map::~KSSLX509Map() {

}


void KSSLX509Map::setValue(const TQString& key, const TQString& value) {
	m_pairs.replace(key, value);
}


TQString KSSLX509Map::getValue(const TQString& key) const {
	if (!m_pairs.contains(key)) {
		return TQString::null;
	}

	return m_pairs[key];
}

static TQStringList tokenizeBy(const TQString& str, const TQRegExp& tok, bool keepEmpties = false) {
TQStringList tokens;
unsigned int head, tail;
const char *chstr = str.ascii();
unsigned int length = str.length();
 
	if (length < 1) {
		return tokens;
	}

	if (length == 1) {
		tokens.append(str);
		return tokens;
	}

	for(head = 0, tail = 0; tail < length-1; head = tail+1) {
		TQString thisline;

		tail = str.find(tok, head);

		if (tail > length)           // last token - none at end
			tail = length;

		if (tail-head > 0 || keepEmpties) {    // it has to be at least 1 long!
			thisline = &(chstr[head]);
			thisline.truncate(tail-head);
			tokens.append(thisline);
		}
	}
	return tokens;
}


void KSSLX509Map::parse(const TQString& name) {
TQStringList vl = tokenizeBy(name, TQRegExp("/[A-Za-z]+="), false);

	m_pairs.clear();

	for (TQStringList::Iterator j = vl.begin(); j != vl.end(); ++j) {
		TQStringList apair = tokenizeBy(*j, TQRegExp("="), false);
		if (m_pairs.contains(apair[0])) {
			TQString oldValue = m_pairs[apair[0]];
			oldValue += "\n";
			oldValue += apair[1];
			m_pairs.replace(apair[0], oldValue);
		} else {
			m_pairs.insert(apair[0], apair[1]);
		}
	}
}


void KSSLX509Map::reset(const TQString& name) {
	parse(name);
}

