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

#ifndef JOBITEM_H
#define JOBITEM_H

#include <tqlistview.h>
#include "kmobject.h"

class KMJob;

class JobItem : public TQListViewItem, public KMObject
{
public:
	JobItem(TQListView *parent, KMJob *job = 0);
	~JobItem();
	void init(KMJob *job);
	int compare(TQListViewItem*, int, bool) const;

	int jobID() const;
	TQString jobUri() const;
	KMJob* job() const;

private:
	KMJob	*m_job;
	int	m_ID;
	TQString	m_uri;
};

inline int JobItem::jobID() const
{ return m_ID; }

inline KMJob* JobItem::job() const
{ return m_job; }

inline TQString JobItem::jobUri() const
{ return m_uri; }

#endif
