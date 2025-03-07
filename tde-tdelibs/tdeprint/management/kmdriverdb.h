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

#ifndef KMDRIVERDB_H
#define KMDRIVERDB_H

#include <tqobject.h>
#include <tqptrlist.h>
#include <tqdict.h>
#include <tqstring.h>

#include "kmdbentry.h"

class KMDBCreator;

class KMDriverDB : public TQObject
{
	TQ_OBJECT
public:
	static KMDriverDB* self();

	KMDriverDB(TQObject *parent = 0, const char *name = 0);
	~KMDriverDB();

	void init(TQWidget *parent = 0);
	KMDBEntryList* findEntry(const TQString& manu, const TQString& model);
	KMDBEntryList* findPnpEntry(const TQString& manu, const TQString& model);
	TQDict<KMDBEntryList>* findModels(const TQString& manu);
	const TQDict< TQDict<KMDBEntryList> >& manufacturers() const	{ return m_entries; }

protected:
	void loadDbFile();
	void insertEntry(KMDBEntry *entry);
	TQString dbFile();

protected slots:
	void slotDbCreated();

signals:
	void dbLoaded(bool reloaded);
	void error(const TQString&);

private:
	KMDBCreator			*m_creator;
	TQDict< TQDict<KMDBEntryList> >	m_entries;
	TQDict< TQDict<KMDBEntryList> >	m_pnpentries;

	static KMDriverDB	*m_self;
};

#endif
