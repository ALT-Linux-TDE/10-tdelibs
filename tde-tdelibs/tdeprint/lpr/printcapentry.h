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

#ifndef PRINTCAPENTRY_H
#define PRINTCAPENTRY_H

#if !defined( _TDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a TDEPrint developer
#endif

#include <tqstring.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <tqtextstream.h>

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class Field
{
public:
	enum Type { String, Integer, Boolean };
	Field() : type(String) {}
	Field(const Field &f) : type(f.type), name(f.name), value(f.value) {}
	Field& operator= (const Field& f)
	{
		type = f.type;
		name = f.name;
		value = f.value;
		return (*this);
	}
	TQString toString() const;

	Type	type;
	TQString	name;
	TQString	value;
};

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class PrintcapEntry
{
public:
	TQString			name;
	TQStringList		aliases;
	TQString			comment;
	TQMap<TQString,Field>	fields;
	TQString			postcomment;

	bool has(const TQString& f) const	{ return fields.contains(f); }
	TQString field(const TQString& f) const	{ return fields[f].value; }
	bool writeEntry(TQTextStream&);
	void addField(const TQString& name, Field::Type type = Field::Boolean, const TQString& value = TQString::null);
};

#endif
