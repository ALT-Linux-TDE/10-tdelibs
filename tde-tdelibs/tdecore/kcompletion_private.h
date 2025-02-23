/* This file is part of the KDE libraries
    Copyright (C) 1999 Carsten Pfeiffer <pfeiffer@kde.org>

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


#ifndef KCOMPLETION_PRIVATE_H
#define KCOMPLETION_PRIVATE_H

#include <tqstring.h>
#include <ksortablevaluelist.h>

class TDECompTreeNode;

#include <kallocator.h>

/**
 * @internal
 */
class TDECORE_EXPORT TDECompTreeNodeList
{
public:
    TDECompTreeNodeList() : first(0), last(0), m_count(0) {}
    TDECompTreeNode *begin() const { return first; }
    TDECompTreeNode *end() const { return last; }

    TDECompTreeNode *at(uint index) const;
    void append(TDECompTreeNode *item); 
    void prepend(TDECompTreeNode *item); 
    void insert(TDECompTreeNode *after, TDECompTreeNode *item);
    TDECompTreeNode *remove(TDECompTreeNode *item);
    uint count() const { return m_count; }

private:
    TDECompTreeNode *first, *last;
    uint m_count;
};

typedef TDECompTreeNodeList TDECompTreeChildren;

/**
 * A helper class for TDECompletion. Implements a tree of TQChar.
 *
 * The tree looks like this (containing the items "kde", "kde-ui",
 * "kde-core" and "pfeiffer". Every item is delimited with TQChar( 0x0 )
 *
 *              some_root_node
 *                  /     \
 *                 k       p
 *                 |       |
 *                 d       f
 *                 |       |
 *                 e       e
 *                /|       |
 *             0x0 -       i
 *                / \      |
 *               u   c     f
 *               |   |     |
 *               i   o     f
 *               |   |     |
 *              0x0  r     e
 *                   |     |
 *                   e     r
 *                   |     |
 *                  0x0   0x0
 *
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 * @internal
 */
class TDECORE_EXPORT TDECompTreeNode : public TQChar
{
public:
    TDECompTreeNode() : TQChar(), myWeight(0) {}
    TDECompTreeNode( const TQChar& ch, uint weight = 0 )
        : TQChar( ch ),
          myWeight( weight ) {}
    ~TDECompTreeNode();

    void * operator new( size_t s ) {
      return alloc.allocate( s );
    }
    void operator delete( void * s ) {
      alloc.deallocate( s );
    }

    // Returns a child of this node matching ch, if available.
    // Otherwise, returns 0L
    inline TDECompTreeNode * find( const TQChar& ch ) const {
      TDECompTreeNode * cur = myChildren.begin();
      while (cur && (*cur != ch)) cur = cur->next;
      return cur;
    }
    TDECompTreeNode *	insert( const TQChar&, bool sorted );
    void 		remove( const TQString& );

    inline int		childrenCount() const { return myChildren.count(); }

    // weighting
    inline void confirm() 	{ myWeight++; 		}
    inline void confirm(uint w) { myWeight += w;	}
    inline void decline() 	{ myWeight--; 		}
    inline uint weight() const 	{ return myWeight; 	}

    inline const TDECompTreeChildren * children() const {
	return &myChildren;
    }
    inline const TDECompTreeNode * childAt(int index) const {
	return myChildren.at(index);
    }
    inline const TDECompTreeNode * firstChild() const {
	return myChildren.begin();
    }
    inline const TDECompTreeNode * lastChild()  const {
	return myChildren.end();
    }

    /* We want to handle a list of TDECompTreeNodes on our own, to not
       need to use TQValueList<>.  And to make it even more fast we don't
       use an accessor, but just a public member.  */
    TDECompTreeNode *next;
private:
    uint myWeight;
    TDECompTreeNodeList	myChildren;
    static TDEZoneAllocator alloc;
};



// some more helper stuff
typedef KSortableValueList<TQString> TDECompletionMatchesList;

/**
 * @internal
 */
class TDECORE_EXPORT TDECompletionMatchesWrapper
{
public:
    TDECompletionMatchesWrapper( bool sort = false )
        : sortedList( sort ? new TDECompletionMatchesList : 0L ),
          dirty( false )
    {}
    ~TDECompletionMatchesWrapper() {
        delete sortedList;
    }

    void setSorting( bool sort ) {
        if ( sort && !sortedList )
            sortedList = new TDECompletionMatchesList;
        else if ( !sort ) {
            delete sortedList;
            sortedList = 0L;
        }
        stringList.clear();
        dirty = false;
    }

    bool sorting() const {
        return sortedList != 0L;
    }

    void append( int i, const TQString& string ) {
        if ( sortedList )
            sortedList->insert( i, string );
        else
            stringList.append( string );
        dirty = true;
    }

    void clear() {
        if ( sortedList )
            sortedList->clear();
        stringList.clear();
        dirty = false;
    }

    uint count() const {
        if ( sortedList )
            return sortedList->count();
        return stringList.count();
    }

    bool isEmpty() const {
        return count() == 0;
    }

    TQString first() const {
        return list().first();
    }

    TQString last() const {
        return list().last();
    }

    TQStringList list() const;

    mutable TQStringList stringList;
    TDECompletionMatchesList *sortedList;
    mutable bool dirty;
};


#endif // KCOMPLETION_PRIVATE_H
