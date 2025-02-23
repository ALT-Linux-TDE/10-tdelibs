/* This file is part of the KDE libraries
   Copyright (C) 1999,2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

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


#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <knotifyclient.h>
#include <tdeglobal.h>

#include <tqptrvector.h>

#include "kcompletion.h"
#include "kcompletion_private.h"


class TDECompletionPrivate
{
public:
    // not a member to avoid #including kcompletion_private.h from kcompletion.h
    // list used for nextMatch() and previousMatch()
    TDECompletionMatchesWrapper matches;
};

TDECompletion::TDECompletion()
{
    d = new TDECompletionPrivate;

    myCompletionMode = TDEGlobalSettings::completionMode();
    myTreeRoot = new TDECompTreeNode;
    myBeep       = true;
    myIgnoreCase = false;
    myHasMultipleMatches = false;
    myRotationIndex = 0;
    setOrder( Insertion );
}

TDECompletion::~TDECompletion()
{
    delete d;
    delete myTreeRoot;
}

void TDECompletion::setOrder( CompOrder order )
{
    myOrder = order;
    d->matches.setSorting( order == Weighted );
}

void TDECompletion::setIgnoreCase( bool ignoreCase )
{
    myIgnoreCase = ignoreCase;
}

void TDECompletion::setItems( const TQStringList& items )
{
    clear();
    insertItems( items );
}


void TDECompletion::insertItems( const TQStringList& items )
{
    bool weighted = (myOrder == Weighted);
    TQStringList::ConstIterator it;
    if ( weighted ) { // determine weight
        for ( it = items.begin(); it != items.end(); ++it )
            addWeightedItem( *it );
    }
    else {
        for ( it = items.begin(); it != items.end(); ++it )
            addItem( *it, 0 );
    }
}

TQStringList TDECompletion::items() const
{
    TDECompletionMatchesWrapper list; // unsorted
    bool addWeight = (myOrder == Weighted);
    extractStringsFromNode( myTreeRoot, TQString::null, &list, addWeight );

    return list.list();
}

bool TDECompletion::isEmpty() const
{
  return (myTreeRoot->childrenCount() == 0);
}

void TDECompletion::addItem( const TQString& item )
{
    d->matches.clear();
    myRotationIndex = 0;
    myLastString = TQString::null;

    addItem( item, 0 );
}

void TDECompletion::addItem( const TQString& item, uint weight )
{
    if ( item.isEmpty() )
        return;

    TDECompTreeNode *node = myTreeRoot;
    uint len = item.length();

    bool sorted = (myOrder == Sorted);
    bool weighted = ((myOrder == Weighted) && weight > 1);

    // knowing the weight of an item, we simply add this weight to all of its
    // nodes.

    for ( uint i = 0; i < len; i++ ) {
        node = node->insert( item.at(i), sorted );
        if ( weighted )
            node->confirm( weight -1 ); // node->insert() sets weighting to 1
    }

    // add 0x0-item as delimiter with evtl. weight
    node = node->insert( 0x0, true );
    if ( weighted )
        node->confirm( weight -1 );
//     tqDebug("*** added: %s (%i)", item.latin1(), node->weight());
}

void TDECompletion::addWeightedItem( const TQString& item )
{
    if ( myOrder != Weighted ) {
        addItem( item, 0 );
        return;
    }

    uint len = item.length();
    uint weight = 0;

    // find out the weighting of this item (appended to the string as ":num")
    int index = item.findRev(':');
    if ( index > 0 ) {
        bool ok;
        weight = item.mid( index + 1 ).toUInt( &ok );
        if ( !ok )
            weight = 0;

        len = index; // only insert until the ':'
    }

    addItem( item.left( len ), weight );
    return;
}


void TDECompletion::removeItem( const TQString& item )
{
    d->matches.clear();
    myRotationIndex = 0;
    myLastString = TQString::null;

    myTreeRoot->remove( item );
}


void TDECompletion::clear()
{
    d->matches.clear();
    myRotationIndex = 0;
    myLastString = TQString::null;

    delete myTreeRoot;
    myTreeRoot = new TDECompTreeNode;
}


TQString TDECompletion::makeCompletion( const TQString& string )
{
    if ( myCompletionMode == TDEGlobalSettings::CompletionNone )
        return TQString::null;

    //kdDebug(0) << "TDECompletion: completing: " << string << endl;

    d->matches.clear();
    myRotationIndex = 0;
    myHasMultipleMatches = false;
    myLastMatch = myCurrentMatch;

    // in Shell-completion-mode, emit all matches when we get the same
    // complete-string twice
    if ( myCompletionMode == TDEGlobalSettings::CompletionShell &&
         string == myLastString ) {
        // Don't use d->matches since calling postProcessMatches()
        // on d->matches here would interfere with call to
        // postProcessMatch() during rotation
    
        findAllCompletions( string, &d->matches, myHasMultipleMatches );
        TQStringList l = d->matches.list();
        postProcessMatches( &l );
        emit matches( l );

        if ( l.isEmpty() )
            doBeep( NoMatch );
    
        return TQString::null;
    }

    TQString completion;
    // in case-insensitive popup mode, we search all completions at once
    if ( myCompletionMode == TDEGlobalSettings::CompletionPopup ||
         myCompletionMode == TDEGlobalSettings::CompletionPopupAuto ) {
        findAllCompletions( string, &d->matches, myHasMultipleMatches );
        if ( !d->matches.isEmpty() )
            completion = d->matches.first();
    }
    else
        completion = findCompletion( string );

    if ( myHasMultipleMatches )
        emit multipleMatches();

    myLastString = string;
    myCurrentMatch = completion;

    postProcessMatch( &completion );

    if ( !string.isEmpty() ) { // only emit match when string is not empty
        //kdDebug(0) << "TDECompletion: Match: " << completion << endl;
        emit match( completion );
    }

    if ( completion.isNull() )
        doBeep( NoMatch );

    return completion;
}


TQStringList TDECompletion::substringCompletion( const TQString& string ) const
{
    // get all items in the tree, possibly in sorted order
    bool sorted = (myOrder == Weighted);
    TDECompletionMatchesWrapper allItems( sorted );
    extractStringsFromNode( myTreeRoot, TQString::null, &allItems, false );

    TQStringList list = allItems.list();

    // subStringMatches is invoked manually, via a shortcut, so we should
    // beep here, if necessary.
    if ( list.isEmpty() ) {
        doBeep( NoMatch );
        return list;
    }

    if ( string.isEmpty() ) { // shortcut
        postProcessMatches( &list );
        return list;
    }

    TQStringList matches;
    TQStringList::ConstIterator it = list.begin();

    for( ; it != list.end(); ++it ) {
        TQString item = *it;
        if ( item.find( string, 0, false ) != -1 ) { // always case insensitive
            matches.append( item );
        }
    }

    postProcessMatches( &matches );

    if ( matches.isEmpty() )
        doBeep( NoMatch );

    return matches;
}


void TDECompletion::setCompletionMode( TDEGlobalSettings::Completion mode )
{
    myCompletionMode = mode;
}

TQStringList TDECompletion::allMatches()
{
    // Don't use d->matches since calling postProcessMatches()
    // on d->matches here would interfere with call to
    // postProcessMatch() during rotation
    TDECompletionMatchesWrapper matches( myOrder == Weighted );
    bool dummy;
    findAllCompletions( myLastString, &matches, dummy );
    TQStringList l = matches.list();
    postProcessMatches( &l );
    return l;
}

TDECompletionMatches TDECompletion::allWeightedMatches()
{
    // Don't use d->matches since calling postProcessMatches()
    // on d->matches here would interfere with call to
    // postProcessMatch() during rotation
    TDECompletionMatchesWrapper matches( myOrder == Weighted );
    bool dummy;
    findAllCompletions( myLastString, &matches, dummy );
    TDECompletionMatches ret( matches );
    postProcessMatches( &ret );
    return ret;
}

TQStringList TDECompletion::allMatches( const TQString &string )
{
    TDECompletionMatchesWrapper matches( myOrder == Weighted );
    bool dummy;
    findAllCompletions( string, &matches, dummy );
    TQStringList l = matches.list();
    postProcessMatches( &l );
    return l;
}

TDECompletionMatches TDECompletion::allWeightedMatches( const TQString &string )
{
    TDECompletionMatchesWrapper matches( myOrder == Weighted );
    bool dummy;
    findAllCompletions( string, &matches, dummy );
    TDECompletionMatches ret( matches );
    postProcessMatches( &ret );
    return ret;
}

/////////////////////////////////////////////////////
///////////////// tree operations ///////////////////


TQString TDECompletion::nextMatch()
{
    TQString completion;
    myLastMatch = myCurrentMatch;

    if ( d->matches.isEmpty() ) {
        findAllCompletions( myLastString, &d->matches, myHasMultipleMatches );
        completion = d->matches.first();
        myCurrentMatch = completion;
        myRotationIndex = 0;
        postProcessMatch( &completion );
        emit match( completion );
        return completion;
    }

    TQStringList matches = d->matches.list();
    myLastMatch = matches[ myRotationIndex++ ];

    if ( myRotationIndex == matches.count() -1 )
        doBeep( Rotation ); // indicate last matching item -> rotating

    else if ( myRotationIndex == matches.count() )
        myRotationIndex = 0;

    completion = matches[ myRotationIndex ];
    myCurrentMatch = completion;
    postProcessMatch( &completion );
    emit match( completion );
    return completion;
}



TQString TDECompletion::previousMatch()
{
    TQString completion;
    myLastMatch = myCurrentMatch;

    if ( d->matches.isEmpty() ) {
        findAllCompletions( myLastString, &d->matches, myHasMultipleMatches );
        completion = d->matches.last();
        myCurrentMatch = completion;
        myRotationIndex = 0;
        postProcessMatch( &completion );
        emit match( completion );
        return completion;
    }

    TQStringList matches = d->matches.list();
    myLastMatch = matches[ myRotationIndex ];
    if ( myRotationIndex == 1 )
        doBeep( Rotation ); // indicate first item -> rotating

    else if ( myRotationIndex == 0 )
        myRotationIndex = matches.count();

    myRotationIndex--;

    completion = matches[ myRotationIndex ];
    myCurrentMatch = completion;
    postProcessMatch( &completion );
    emit match( completion );
    return completion;
}



// tries to complete "string" from the tree-root
TQString TDECompletion::findCompletion( const TQString& string )
{
    TQChar ch;
    TQString completion;
    const TDECompTreeNode *node = myTreeRoot;

    // start at the tree-root and try to find the search-string
    for( uint i = 0; i < string.length(); i++ ) {
        ch = string.at( i );
        node = node->find( ch );

        if ( node )
            completion += ch;
        else
            return TQString::null; // no completion
    }

    // Now we have the last node of the to be completed string.
    // Follow it as long as it has exactly one child (= longest possible
    // completion)

    while ( node->childrenCount() == 1 ) {
        node = node->firstChild();
        if ( !node->isNull() )
            completion += *node;
    }
    // if multiple matches and auto-completion mode
    // -> find the first complete match
    if ( node && node->childrenCount() > 1 ) {
        myHasMultipleMatches = true;
    
        if ( myCompletionMode == TDEGlobalSettings::CompletionAuto ) {
            myRotationIndex = 1;
            if (myOrder != Weighted) {
                while ( (node = node->firstChild()) ) {
                    if ( !node->isNull() )
                        completion += *node;
                    else
                        break;
                }
            }
            else {
                // don't just find the "first" match, but the one with the
                // highest priority
        
                const TDECompTreeNode* temp_node = 0L;
                while(1) {
                    int count = node->childrenCount();
                    temp_node = node->firstChild();
                    uint weight = temp_node->weight();
                    const TDECompTreeNode* hit = temp_node;
                    for( int i = 1; i < count; i++ ) {
                        temp_node = node->childAt(i);
                        if( temp_node->weight() > weight ) {
                            hit = temp_node;
                            weight = hit->weight();
                        }
                    }
                    // 0x0 has the highest priority -> we have the best match
                    if ( hit->isNull() )
                        break;

                    node = hit;
                    completion += *node;
                }
            }
        }

        else
            doBeep( PartialMatch ); // partial match -> beep
    }

    return completion;
}


void TDECompletion::findAllCompletions(const TQString& string,
                                     TDECompletionMatchesWrapper *matches,
                                     bool& hasMultipleMatches) const
{
    //kdDebug(0) << "*** finding all completions for " << string << endl;

    if ( string.isEmpty() )
        return;

    if ( myIgnoreCase ) { // case insensitive completion
        extractStringsFromNodeCI( myTreeRoot, TQString::null, string, matches );
        hasMultipleMatches = (matches->count() > 1);
        return;
    }

    TQChar ch;
    TQString completion;
    const TDECompTreeNode *node = myTreeRoot;

    // start at the tree-root and try to find the search-string
    for( uint i = 0; i < string.length(); i++ ) {
        ch = string.at( i );
        node = node->find( ch );

        if ( node )
            completion += ch;
        else
            return; // no completion -> return empty list
    }
    
    // Now we have the last node of the to be completed string.
    // Follow it as long as it has exactly one child (= longest possible
    // completion)

    while ( node->childrenCount() == 1 ) {
        node = node->firstChild();
        if ( !node->isNull() )
            completion += *node;
        // kdDebug() << completion << node->latin1();
    }


    // there is just one single match)
    if ( node->childrenCount() == 0 )
        matches->append( node->weight(), completion );

    else {
        // node has more than one child
        // -> recursively find all remaining completions
        hasMultipleMatches = true;
        extractStringsFromNode( node, completion, matches );
    }
}


void TDECompletion::extractStringsFromNode( const TDECompTreeNode *node,
                                          const TQString& beginning,
                                          TDECompletionMatchesWrapper *matches,
                                          bool addWeight ) const
{
    if ( !node || !matches )
        return;

    // kDebug() << "Beginning: " << beginning << endl;
    const TDECompTreeChildren *list = node->children();
    TQString string;
    TQString w;

    // loop thru all children
    for ( TDECompTreeNode *cur = list->begin(); cur ; cur = cur->next) {
        string = beginning;
        node = cur;
        if ( !node->isNull() )
            string += *node;

        while ( node && node->childrenCount() == 1 ) {
            node = node->firstChild();
            if ( node->isNull() )
                break;
            string += *node;
        }

        if ( node && node->isNull() ) { // we found a leaf
            if ( addWeight ) {
                // add ":num" to the string to store the weighting
                string += ':';
                w.setNum( node->weight() );
                string.append( w );
            }
            matches->append( node->weight(), string );
        }

        // recursively find all other strings.
        if ( node && node->childrenCount() > 1 )
            extractStringsFromNode( node, string, matches, addWeight );
    }
}

void TDECompletion::extractStringsFromNodeCI( const TDECompTreeNode *node,
                                            const TQString& beginning,
                                            const TQString& restString,
                                            TDECompletionMatchesWrapper *matches ) const
{
    if ( restString.isEmpty() ) {
        extractStringsFromNode( node, beginning, matches, false /*noweight*/ );
        return;
    }

    TQChar ch1 = restString.at(0);
    TQString newRest = restString.mid(1);
    TDECompTreeNode *child1, *child2;

    child1 = node->find( ch1 ); // the correct match
    if ( child1 )
        extractStringsFromNodeCI( child1, beginning + *child1, newRest,
                                  matches );

    // append the case insensitive matches, if available
    if ( ch1.isLetter() ) {
        // find out if we have to lower or upper it. Is there a better way?
        TQChar ch2 = ch1.lower();
        if ( ch1 == ch2 )
            ch2 = ch1.upper();
        if ( ch1 != ch2 ) {
            child2 = node->find( ch2 );
            if ( child2 )
                extractStringsFromNodeCI( child2, beginning + *child2, newRest,
                                          matches );
        }
    }
}

void TDECompletion::doBeep( BeepMode mode ) const
{
    if ( !myBeep )
        return;

    TQString text, event;

    switch ( mode ) {
        case Rotation:
            event = TQString::fromLatin1("Textcompletion: rotation");
            text = i18n("You reached the end of the list\nof matching items.\n");
            break;
        case PartialMatch:
            if ( myCompletionMode == TDEGlobalSettings::CompletionShell ||
                 myCompletionMode == TDEGlobalSettings::CompletionMan ) {
                event = TQString::fromLatin1("Textcompletion: partial match");
                text = i18n("The completion is ambiguous, more than one\nmatch is available.\n");
            }
            break;
        case NoMatch:
            if ( myCompletionMode == TDEGlobalSettings::CompletionShell ) {
                event = TQString::fromLatin1("Textcompletion: no match");
                text = i18n("There is no matching item available.\n");
            }
            break;
    }

    if ( !text.isEmpty() )
        KNotifyClient::event( event, text );
}


/////////////////////////////////
/////////


// Implements the tree. Every node is a TQChar and has a list of children, which
// are Nodes as well.
// TQChar( 0x0 ) is used as the delimiter of a string; the last child of each
// inserted string is 0x0.

TDECompTreeNode::~TDECompTreeNode()
{
    // delete all children
    TDECompTreeNode *cur = myChildren.begin();
    while (cur) {
        TDECompTreeNode * next = cur->next;
        delete myChildren.remove(cur);
        cur = next;
    }
}


// Adds a child-node "ch" to this node. If such a node is already existant,
// it will not be created. Returns the new/existing node.
TDECompTreeNode * TDECompTreeNode::insert( const TQChar& ch, bool sorted )
{
    TDECompTreeNode *child = find( ch );
    if ( !child ) {
        child = new TDECompTreeNode( ch );

        // FIXME, first (slow) sorted insertion implementation
        if ( sorted ) {
            TDECompTreeNode * prev = 0;
            TDECompTreeNode * cur = myChildren.begin();
            while ( cur ) {
                if ( ch > *cur ) {
                    prev = cur;
                    cur = cur->next;
                } else
                    break;
            }
            if (prev)
                myChildren.insert( prev, child );
            else
                myChildren.prepend(child);
        }

        else
            myChildren.append( child );
    }

    // implicit weighting: the more often an item is inserted, the higher
    // priority it gets.
    child->confirm();

    return child;
}


// Iteratively removes a string from the tree. The nicer recursive
// version apparently was a little memory hungry (see #56757)
void TDECompTreeNode::remove( const TQString& str )
{
    TQString string = str;
    string += TQChar(0x0);

    TQPtrVector<TDECompTreeNode> deletables( string.length() + 1 );

    TDECompTreeNode *child = 0L;
    TDECompTreeNode *parent = this;
    deletables.insert( 0, parent );
    
    uint i = 0;
    for ( ; i < string.length(); i++ )
    {
        child = parent->find( string.at( i ) );
        if ( child )
            deletables.insert( i + 1, child );
        else
            break;

        parent = child;
    }

    for ( ; i >= 1; i-- )
    {
        parent = deletables.at( i - 1 );
        child = deletables.at( i );
        if ( child->myChildren.count() == 0 )
            delete parent->myChildren.remove( child );
    }
}

TQStringList TDECompletionMatchesWrapper::list() const 
{
    if ( sortedList && dirty ) {
        sortedList->sort();
        dirty = false;

        stringList.clear();

        // high weight == sorted last -> reverse the sorting here
        TQValueListConstIterator<KSortableItem<TQString> > it;
        for ( it = sortedList->begin(); it != sortedList->end(); ++it )
            stringList.prepend( (*it).value() );
    }

    return stringList;
}

TDECompletionMatches::TDECompletionMatches( bool sort_P )
    : _sorting( sort_P )
{
}

TDECompletionMatches::TDECompletionMatches( const TDECompletionMatchesWrapper& matches )
    : _sorting( matches.sorting())
{
    if( matches.sortedList != 0L )
        TDECompletionMatchesList::operator=( *matches.sortedList );
    else {
        TQStringList l = matches.list();
        for( TQStringList::ConstIterator it = l.begin();
             it != l.end();
             ++it )
            prepend( KSortableItem<TQString, int>( 1, *it ) );
    }
}

TDECompletionMatches::~TDECompletionMatches()
{
}

TQStringList TDECompletionMatches::list( bool sort_P ) const
{
    if( _sorting && sort_P )
        const_cast< TDECompletionMatches* >( this )->sort();
    TQStringList stringList;
    // high weight == sorted last -> reverse the sorting here
    for ( ConstIterator it = begin(); it != end(); ++it )
        stringList.prepend( (*it).value() );
    return stringList;
}

void TDECompletionMatches::removeDuplicates()
{
    Iterator it1, it2;
    for ( it1 = begin(); it1 != end(); ++it1 ) {
        for ( (it2 = it1), ++it2; it2 != end();) {
            if( (*it1).value() == (*it2).value()) {
                // use the max height
                (*it1).first = kMax( (*it1).index(), (*it2).index());
                it2 = remove( it2 );
                continue;
            }
            ++it2;
        }
    }
}

void TDECompTreeNodeList::append(TDECompTreeNode *item)
{
    m_count++;
    if (!last) {
        last = item;
        last->next = 0;
        first = item;
        return;
    }
    last->next = item;
    item->next = 0;
    last = item;
}

void TDECompTreeNodeList::prepend(TDECompTreeNode *item)
{
    m_count++;
    if (!last) {
        last = item;
        last->next = 0;
        first = item;
        return;
    }
    item->next = first;
    first = item;
}

void TDECompTreeNodeList::insert(TDECompTreeNode *after, TDECompTreeNode *item)
{
    if (!after) {
        append(item);
        return;
    }

    m_count++;

    item->next = after->next;
    after->next = item;

    if (after == last)
        last = item;
}

TDECompTreeNode *TDECompTreeNodeList::remove(TDECompTreeNode *item)
{
    if (!first || !item)
        return 0;
    TDECompTreeNode *cur = 0;

    if (item == first)
        first = first->next;
    else {
        cur = first;
        while (cur && cur->next != item) cur = cur->next;
        if (!cur)
            return 0;
        cur->next = item->next;
    }
    if (item == last)
        last = cur;
    m_count--;
    return item;
}

TDECompTreeNode *TDECompTreeNodeList::at(uint index) const
{
    TDECompTreeNode *cur = first;
    while (index-- && cur) cur = cur->next;
    return cur;
}

TDEZoneAllocator TDECompTreeNode::alloc(8192);

void TDECompletion::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void TDECompletionBase::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kcompletion.moc"
