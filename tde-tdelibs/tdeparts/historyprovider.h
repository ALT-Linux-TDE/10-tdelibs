/* This file is part of the KDE project
   Copyright (C) 2001 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KHISTORYPROVIDER_H
#define KHISTORYPROVIDER_H

#include <tqdict.h>
#include <tqobject.h>

#include <tdelibs_export.h>

namespace KParts {

/**
 * Basic class to manage a history of "items". This class is only meant
 * for fast lookup, if an item is in the history or not.
 *
 * May be subclassed to implement a persistent history for example.
 * For usage with tdehtml, just create your provider and call the
 * HistoryProvider constructor _before_ you do any tdehtml stuff. That way,
 * tdehtml, using the self()-method, will use your subclassed provider.
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEPARTS_EXPORT HistoryProvider : public TQObject
{
    TQ_OBJECT

public:
    static HistoryProvider * self();

    /**
     * Creates a KHistoryProvider with an optional parent and name
     */
    HistoryProvider( TQObject *parent = 0L, const char *name = 0 );

    /**
     * Destroys the provider.
     */
    virtual ~HistoryProvider();

    /**
     * @returns true if @p item is present in the history.
     */
    virtual bool contains( const TQString& item ) const;

    /**
     * Inserts @p item into the history.
     */
    virtual void insert( const TQString& item );

    /**
     * Removes @p item from the history.
     */
    virtual void remove( const TQString& item );

    /**
     * Clears the history. The cleared() signal is emitted after clearing.
     */
    virtual void clear();

signals:
    /**
     * Emitted after the history has been cleared.
     */
    void cleared();

    /**
     * This signal is never emitted from this class, it is only meant as an
     * interface for subclasses. Emit this signal to notify others that the
     * history has changed. Put those items that were added or removed from the
     * history into @p items.
     */
    void updated( const TQStringList& items );

    /**
     * Emitted after the item has been inserted
     */
    void inserted( const TQString& item );

private:
    static HistoryProvider *s_self;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class HistoryProviderPrivate;
    HistoryProviderPrivate *d;
};

}

#endif // KHISTORYPROVIDER_H
