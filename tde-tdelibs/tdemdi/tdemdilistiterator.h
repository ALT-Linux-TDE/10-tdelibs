//----------------------------------------------------------------------------
//    filename             : tdemdilistiterator.h
//----------------------------------------------------------------------------
//    Project              : KDE MDI extension
//
//    begin                : 02/2000       by Massimo Morin
//    changes              : 02/2000       by Falk Brettschneider to create an
//                           - 06/2000     stand-alone Qt extension set of
//                                         classes and a Qt-based library
//                           2000-2003     maintained by the KDevelop project
//
//    copyright            : (C) 1999-2003 by Massimo Morin (mmorin@schedsys.com)
//                                         and
//                                         Falk Brettschneider
//    email                :  falkbr@kdevelop.org (Falk Brettschneider)
//----------------------------------------------------------------------------
//
//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _TDEMDILISTITERATOR_H_
#define _TDEMDILISTITERATOR_H_

#include <tdemdiiterator.h>

template <class I>
class TQPtrList;
template <class I>
class TQPtrListIterator;

template <class Item>
class KMdiListIterator : public KMdiIterator<Item*>
{
public:
	KMdiListIterator( TQPtrList<Item>& list )
	{
		m_iterator = new TQPtrListIterator<Item>( list );
	}

	virtual void first() { m_iterator->toFirst(); }
	virtual void last() { m_iterator->toLast(); }
	virtual void next() { ++( *m_iterator ); }
	virtual void prev() { --( *m_iterator ); }
	virtual bool isDone() const { return m_iterator->current() == 0; }
	virtual Item* currentItem() const { return m_iterator->current(); }

	virtual ~KMdiListIterator() { delete m_iterator; }

private:
	TQPtrListIterator<Item> *m_iterator;
};

#endif // _TDEMDILISTITERATOR_H_ 
