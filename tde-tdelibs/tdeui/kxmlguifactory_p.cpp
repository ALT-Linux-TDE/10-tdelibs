/* This file is part of the KDE libraries
   Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>

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

#include "kxmlguifactory_p.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

#include <tqwidget.h>

#include <tdeglobal.h>
#include <kdebug.h>

#include <assert.h>

using namespace KXMLGUI;

void ActionList::plug( TQWidget *container, int index ) const
{
    ActionListIt it( *this );
    for (; it.current(); ++it )
        it.current()->plug( container, index++ );
}

void ActionList::unplug( TQWidget *container ) const
{
    ActionListIt it( *this );
    for (; it.current(); ++it )
        it.current()->unplug( container );
}

ContainerNode::ContainerNode( TQWidget *_container, const TQString &_tagName,
                              const TQString &_name, ContainerNode *_parent,
                              KXMLGUIClient *_client, KXMLGUIBuilder *_builder,
                              int id, const TQString &_mergingName,
                              const TQString &_groupName, const TQStringList &customTags,
                              const TQStringList &containerTags )
    : parent( _parent ), client( _client ), builder( _builder ),
      builderCustomTags( customTags ), builderContainerTags( containerTags ),
      container( _container ), containerId( id ), tagName( _tagName ), name( _name ),
      groupName( _groupName ), index( 0 ), mergingName( _mergingName )
{
    children.setAutoDelete( true );
    clients.setAutoDelete( true );

    if ( parent )
        parent->children.append( this );
}

void ContainerNode::removeChild( ContainerNode *child )
{
    MergingIndexList::Iterator mergingIt = findIndex( child->mergingName );
    adjustMergingIndices( -1, mergingIt );
    children.removeRef( child );
}

/*
 * Find a merging index with the given name. Used to find an index defined by <Merge name="blah"/>
 * or by a <DefineGroup name="foo" /> tag.
 */
MergingIndexList::Iterator ContainerNode::findIndex( const TQString &name )
{
    MergingIndexList::Iterator it( mergingIndices.begin() );
    MergingIndexList::Iterator end( mergingIndices.end() );
    for (; it != end; ++it )
        if ( (*it).mergingName == name )
            return it;
    return it;
}

/*
 * Check if the given container widget is a child of this node and return the node structure
 * if found.
 */
ContainerNode *ContainerNode::findContainerNode( TQWidget *container )
{
    ContainerNodeListIt it( children );

    for (; it.current(); ++it )
        if ( it.current()->container == container )
            return it.current();

    return 0L;
}

/*
 * Find a container recursively with the given name. Either compares _name with the
 * container's tag name or the value of the container's name attribute. Specified by
 * the tag bool .
 */
ContainerNode *ContainerNode::findContainer( const TQString &_name, bool tag )
{
    if ( ( tag && tagName == _name ) ||
         ( !tag && name == _name ) )
        return this;

    ContainerNodeListIt it( children );
    for (; it.current(); ++it )
    {
        ContainerNode *res = it.current()->findContainer( _name, tag );
        if ( res )
            return res;
    }

    return 0;
}

/*
 * Finds a child container node (not recursively) with the given name and tagname. Explicitly
 * leaves out container widgets specified in the exludeList . Also ensures that the containers
 * belongs to currClient.
 */
ContainerNode *ContainerNode::findContainer( const TQString &name, const TQString &tagName,
                                             const TQPtrList<TQWidget> *excludeList,
                                             KXMLGUIClient * /*currClient*/ )
{
    ContainerNode *res = 0L;
    ContainerNodeListIt nIt( children );

    if ( !name.isEmpty() )
    {
        for (; nIt.current(); ++nIt )
            if ( nIt.current()->name == name &&
                 !excludeList->containsRef( nIt.current()->container ) )
            {
                res = nIt.current();
                break;
            }

        return res;
    }

    if ( !tagName.isEmpty() )
        for (; nIt.current(); ++nIt )
        {
            if ( nIt.current()->tagName == tagName &&
                 !excludeList->containsRef( nIt.current()->container )
                 /*
                  * It is a bad idea to also compare the client, because
                  * we don't want to do so in situations like these:
                  *
                  * <MenuBar>
                  *   <Menu>
                  *     ...
                  *
                  * other client:
                  * <MenuBar>
                  *   <Menu>
                  *    ...
                  *
                 && nIt.current()->client == currClient )
                 */
                )
            {
                res = nIt.current();
                break;
            }
        }

    return res;
}

ContainerClient *ContainerNode::findChildContainerClient( KXMLGUIClient *currentGUIClient,
                                                          const TQString &groupName,
                                                          const MergingIndexList::Iterator &mergingIdx )
{
    if ( !clients.isEmpty() )
    {
        ContainerClientListIt clientIt( clients );

        for (; clientIt.current(); ++clientIt )
            if ( clientIt.current()->client == currentGUIClient )
            {
                if ( groupName.isEmpty() )
                    return clientIt.current();

                if ( groupName == clientIt.current()->groupName )
                    return clientIt.current();
            }
    }

    ContainerClient *client = new ContainerClient;
    client->client = currentGUIClient;
    client->groupName = groupName;

    if ( mergingIdx != mergingIndices.end() )
        client->mergingName = (*mergingIdx).mergingName;

    clients.append( client );

    return client;
}

void ContainerNode::plugActionList( BuildState &state )
{
    MergingIndexList::Iterator mIt( mergingIndices.begin() );
    MergingIndexList::Iterator mEnd( mergingIndices.end() );
    for (; mIt != mEnd; ++mIt )
        plugActionList( state, mIt );

    TQPtrListIterator<ContainerNode> childIt( children );
    for (; childIt.current(); ++childIt )
        childIt.current()->plugActionList( state );
}

void ContainerNode::plugActionList( BuildState &state, const MergingIndexList::Iterator &mergingIdxIt )
{
    static const TQString &tagActionList = TDEGlobal::staticQString( "actionlist" );

    MergingIndex mergingIdx = *mergingIdxIt;

    TQString k( mergingIdx.mergingName );

    if ( k.find( tagActionList ) == -1 )
        return;

    k = k.mid( tagActionList.length() );

    if ( mergingIdx.clientName != state.clientName )
        return;

    if ( k != state.actionListName )
        return;

    ContainerClient *client = findChildContainerClient( state.guiClient,
                                                        TQString(),
                                                        mergingIndices.end() );

    client->actionLists.insert( k, state.actionList );

    state.actionList.plug( container, mergingIdx.value );

    adjustMergingIndices( state.actionList.count(), mergingIdxIt );
}

void ContainerNode::unplugActionList( BuildState &state )
{
    MergingIndexList::Iterator mIt( mergingIndices.begin() );
    MergingIndexList::Iterator mEnd( mergingIndices.end() );
    for (; mIt != mEnd; ++mIt )
        unplugActionList( state, mIt );

    TQPtrListIterator<ContainerNode> childIt( children );
    for (; childIt.current(); ++childIt )
        childIt.current()->unplugActionList( state );
}

void ContainerNode::unplugActionList( BuildState &state, const MergingIndexList::Iterator &mergingIdxIt )
{
    static const TQString &tagActionList = TDEGlobal::staticQString( "actionlist" );

    MergingIndex mergingIdx = *mergingIdxIt;

    TQString k = mergingIdx.mergingName;

    if ( k.find( tagActionList ) == -1 )
        return;

    k = k.mid( tagActionList.length() );

    if ( mergingIdx.clientName != state.clientName )
        return;

    if ( k != state.actionListName )
        return;

    ContainerClient *client = findChildContainerClient( state.guiClient,
                                                        TQString(),
                                                        mergingIndices.end() );

    ActionListMap::Iterator lIt( client->actionLists.find( k ) );
    if ( lIt == client->actionLists.end() )
        return;

    lIt.data().unplug( container );

    adjustMergingIndices( -int(lIt.data().count()), mergingIdxIt );

    client->actionLists.remove( lIt );
}

void ContainerNode::adjustMergingIndices( int offset,
                                          const MergingIndexList::Iterator &it )
{
    MergingIndexList::Iterator mergingIt = it;
    MergingIndexList::Iterator mergingEnd = mergingIndices.end();

    for (; mergingIt != mergingEnd; ++mergingIt )
        (*mergingIt).value += offset;

    index += offset;
}

bool ContainerNode::destruct( TQDomElement element, BuildState &state )
{
    destructChildren( element, state );

    unplugActions( state );

    // remove all merging indices the client defined
    MergingIndexList::Iterator cmIt = mergingIndices.begin();
    while ( cmIt != mergingIndices.end() )
        if ( (*cmIt).clientName == state.clientName )
            cmIt = mergingIndices.remove( cmIt );
        else
            ++cmIt;

    // ### check for merging index count, too?
    if ( clients.count() == 0 && children.count() == 0 && container &&
         client == state.guiClient )
    {
        TQWidget *parentContainer = 0L;

        if ( parent && parent->container )
            parentContainer = parent->container;

        assert( builder );

        builder->removeContainer( container, parentContainer, element, containerId );

        client = 0L;

        return true;
    }

    if ( client == state.guiClient )
        client = 0L;

    return false;

}

void ContainerNode::destructChildren( const TQDomElement &element, BuildState &state )
{
    TQPtrListIterator<ContainerNode> childIt( children );
    while ( childIt.current() )
    {
        ContainerNode *childNode = childIt.current();

        TQDomElement childElement = findElementForChild( element, childNode );

        // destruct returns true in case the container really got deleted
        if ( childNode->destruct( childElement, state ) )
            removeChild( childNode );
        else
            ++childIt;
    }
}

TQDomElement ContainerNode::findElementForChild( const TQDomElement &baseElement,
                                                ContainerNode *childNode )
{
    static const TQString &attrName = TDEGlobal::staticQString( "name" );

    // ### slow
    for ( TQDomNode n = baseElement.firstChild(); !n.isNull();
          n = n.nextSibling() )
    {
        TQDomElement e = n.toElement();
        if ( e.tagName().lower() == childNode->tagName &&
             e.attribute( attrName ) == childNode->name )
            return e;
    }

    return TQDomElement();
}

void ContainerNode::unplugActions( BuildState &state )
{
    if ( !container )
        return;

    ContainerClientListIt clientIt( clients );

    /*
        Disabled because it means in TDEToolBar::saveState isHidden is always true then,
        which is clearly wrong.

    if ( clients.count() == 1 && clientIt.current()->client == client &&
         client == state.guiClient )
        container->hide(); // this container is going to die, that's for sure.
                           // in this case let's just hide it, which makes the
                           // destruction faster
     */

    while ( clientIt.current() )
        //only unplug the actions of the client we want to remove, as the container might be owned
        //by a different client
        if ( clientIt.current()->client == state.guiClient )
        {
            unplugClient( clientIt.current() );
            clients.removeRef( clientIt.current() );
        }
        else
            ++clientIt;
}

void ContainerNode::unplugClient( ContainerClient *client )
{
    static const TQString &tagActionList = TDEGlobal::staticQString( "actionlist" );

    assert( builder );

    // now quickly remove all custom elements (i.e. separators) and unplug all actions

    TQValueList<int>::ConstIterator custIt = client->customElements.begin();
    TQValueList<int>::ConstIterator custEnd = client->customElements.end();
    for (; custIt != custEnd; ++custIt )
        builder->removeCustomElement( container, *custIt );

    client->actions.unplug( container );

    // now adjust all merging indices

    MergingIndexList::Iterator mergingIt = findIndex( client->mergingName );

    adjustMergingIndices( - int( client->actions.count()
                          + client->customElements.count() ),
                          mergingIt );

    // unplug all actionslists

    ActionListMap::ConstIterator alIt = client->actionLists.begin();
    ActionListMap::ConstIterator alEnd = client->actionLists.end();
    for (; alIt != alEnd; ++alIt )
    {
        alIt.data().unplug( container );

        // construct the merging index key (i.e. like named merging) , find the
        // corresponding merging index and adjust all indices
        TQString mergingKey = alIt.key();
        mergingKey.prepend( tagActionList );

        MergingIndexList::Iterator mIt = findIndex( mergingKey );
        if ( mIt == mergingIndices.end() )
            continue;

        adjustMergingIndices( -int(alIt.data().count()), mIt );

        // remove the actionlists' merging index
        // ### still needed? we clean up below anyway?
        mergingIndices.remove( mIt );
    }
}

void ContainerNode::reset()
{
    TQPtrListIterator<ContainerNode> childIt( children );
    for (; childIt.current(); ++childIt )
        childIt.current()->reset();

    if ( client )
        client->setFactory( 0L );
}

int ContainerNode::calcMergingIndex( const TQString &mergingName,
                                     MergingIndexList::Iterator &it,
                                     BuildState &state,
                                     bool ignoreDefaultMergingIndex )
{
    MergingIndexList::Iterator mergingIt;

    if ( mergingName.isEmpty() )
        mergingIt = findIndex( state.clientName );
    else
        mergingIt = findIndex( mergingName );

    MergingIndexList::Iterator mergingEnd = mergingIndices.end();
    it = mergingEnd;

    if ( ( mergingIt == mergingEnd && state.currentDefaultMergingIt == mergingEnd ) ||
         ignoreDefaultMergingIndex )
        return index;

    if ( mergingIt != mergingEnd )
        it = mergingIt;
    else
        it = state.currentDefaultMergingIt;

    return (*it).value;
}

int BuildHelper::calcMergingIndex( const TQDomElement &element, MergingIndexList::Iterator &it, TQString &group )
{
    static const TQString &attrGroup = TDEGlobal::staticQString( "group" );

    bool haveGroup = false;
    group = element.attribute( attrGroup );
    if ( !group.isEmpty() ) {
        group.prepend( attrGroup );
        haveGroup = true;
    }

    int idx;
    if ( haveGroup )
        idx = parentNode->calcMergingIndex( group, it, m_state, ignoreDefaultMergingIndex );
    else if ( m_state.currentClientMergingIt == parentNode->mergingIndices.end() )
        idx = parentNode->index;
    else
        idx = (*m_state.currentClientMergingIt).value;

    return idx;
}

BuildHelper::BuildHelper( BuildState &state, ContainerNode *node )
    : containerClient( 0 ), ignoreDefaultMergingIndex( false ), m_state( state ),
      parentNode( node )
{
    static const TQString &defaultMergingName = TDEGlobal::staticQString( "<default>" );

    // create a list of supported container and custom tags
    customTags = m_state.builderCustomTags;
    containerTags = m_state.builderContainerTags;

    if ( parentNode->builder != m_state.builder )
    {
        customTags += parentNode->builderCustomTags;
        containerTags += parentNode->builderContainerTags;
    }

    if ( m_state.clientBuilder ) {
        customTags = m_state.clientBuilderCustomTags + customTags;
        containerTags = m_state.clientBuilderContainerTags + containerTags;
    }

    m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
    parentNode->calcMergingIndex( TQString(), m_state.currentClientMergingIt,
                                  m_state, /*ignoreDefaultMergingIndex*/ false );
}

void BuildHelper::build( const TQDomElement &element )
{
    for (TQDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        TQDomElement e = n.toElement();
        if (e.isNull()) continue;
        processElement( e );
    }
}

void BuildHelper::processElement( const TQDomElement &e )
{
    // some often used QStrings
    static const TQString &tagAction = TDEGlobal::staticQString( "action" );
    static const TQString &tagMerge = TDEGlobal::staticQString( "merge" );
    static const TQString &tagState = TDEGlobal::staticQString( "state" );
    static const TQString &tagDefineGroup = TDEGlobal::staticQString( "definegroup" );
    static const TQString &tagActionList = TDEGlobal::staticQString( "actionlist" );
    static const TQString &attrName = TDEGlobal::staticQString( "name" );

    TQString tag( e.tagName().lower() );
    TQString currName( e.attribute( attrName ) );

    bool isActionTag = ( tag == tagAction );

    if ( isActionTag || customTags.findIndex( tag ) != -1 )
        processActionOrCustomElement( e, isActionTag );
    else if ( containerTags.findIndex( tag ) != -1 )
        processContainerElement( e, tag, currName );
    else if ( tag == tagMerge || tag == tagDefineGroup || tag == tagActionList )
        processMergeElement( tag, currName, e );
    else if ( tag == tagState )
        processStateElement( e );
}

void BuildHelper::processActionOrCustomElement( const TQDomElement &e, bool isActionTag )
{
    if ( !parentNode->container )
        return;

    MergingIndexList::Iterator it( m_state.currentClientMergingIt );

    TQString group;
    int idx = calcMergingIndex( e, it, group );

    containerClient = parentNode->findChildContainerClient( m_state.guiClient, group, it );

    bool guiElementCreated = false;
    if ( isActionTag )
        guiElementCreated = processActionElement( e, idx );
    else
        guiElementCreated = processCustomElement( e, idx );

    if ( guiElementCreated )
        // adjust any following merging indices and the current running index for the container
        parentNode->adjustMergingIndices( 1, it );
}

bool BuildHelper::processActionElement( const TQDomElement &e, int idx )
{
    assert( m_state.guiClient );

    // look up the action and plug it in
    TDEAction *action = m_state.guiClient->action( e );

    //kdDebug() << "BuildHelper::processActionElement " << e.attribute( "name" ) << " -> " << action << " (in " << m_state.guiClient->actionCollection() << ")" << endl;
    if ( !action )
        return false;

    action->plug( parentNode->container, idx );

    // save a reference to the plugged action, in order to properly unplug it afterwards.
    containerClient->actions.append( action );

    return true;
}

bool BuildHelper::processCustomElement( const TQDomElement &e, int idx )
{
    assert( parentNode->builder );

    int id = parentNode->builder->createCustomElement( parentNode->container, idx, e );
    if ( id == 0 )
        return false;

    containerClient->customElements.append( id );
    return true;
}

void BuildHelper::processStateElement( const TQDomElement &element )
{
    TQString stateName = element.attribute( "name" );

    if ( !stateName || !stateName.length() ) return;

    for (TQDomNode n = element.firstChild(); !n.isNull(); n = n.nextSibling() )
    {
        TQDomElement e = n.toElement();
        if (e.isNull()) continue;

        TQString tagName = e.tagName().lower();

        if ( tagName != "enable" && tagName != "disable" )
            continue;

        bool processingActionsToEnable = (tagName == "enable");

        // process action names
        for (TQDomNode n2 = n.firstChild(); !n2.isNull(); n2 = n2.nextSibling() )
        {
            TQDomElement actionEl = n2.toElement();
            if ( actionEl.tagName().lower() != "action" ) continue;

            TQString actionName = actionEl.attribute( "name" );
            if ( !actionName || !actionName.length() ) return;

            if ( processingActionsToEnable )
                m_state.guiClient->addStateActionEnabled( stateName, actionName );
            else
                m_state.guiClient->addStateActionDisabled( stateName, actionName );

        }
    }
}

void BuildHelper::processMergeElement( const TQString &tag, const TQString &name, const TQDomElement &e )
{
    static const TQString &tagDefineGroup = TDEGlobal::staticQString( "definegroup" );
    static const TQString &tagActionList = TDEGlobal::staticQString( "actionlist" );
    static const TQString &defaultMergingName = TDEGlobal::staticQString( "<default>" );
    static const TQString &attrGroup = TDEGlobal::staticQString( "group" );

    TQString mergingName( name );
    if ( mergingName.isEmpty() )
    {
        if ( tag == tagDefineGroup )
        {
            kdError(1000) << "cannot define group without name!" << endl;
            return;
        }
        if ( tag == tagActionList )
        {
            kdError(1000) << "cannot define actionlist without name!" << endl;
            return;
        }
        mergingName = defaultMergingName;
    }

    if ( tag == tagDefineGroup )
        mergingName.prepend( attrGroup ); //avoid possible name clashes by prepending
                                              // "group" to group definitions
    else if ( tag == tagActionList )
        mergingName.prepend( tagActionList );

    if ( parentNode->findIndex( mergingName ) != parentNode->mergingIndices.end() )
        return; //do not allow the redefinition of merging indices!

    MergingIndexList::Iterator mIt( parentNode->mergingIndices.end() );

    TQString group( e.attribute( attrGroup ) );
    if ( !group.isEmpty() )
        group.prepend( attrGroup );

    // calculate the index of the new merging index. Usually this does not need any calculation,
    // we just want the last available index (i.e. append) . But in case the <Merge> tag appears
    // "inside" another <Merge> tag from a previously build client, then we have to use the
    // "parent's" index. That's why we call calcMergingIndex here.
    MergingIndex newIdx;
    newIdx.value = parentNode->calcMergingIndex( group, mIt, m_state, ignoreDefaultMergingIndex );
    newIdx.mergingName = mergingName;
    newIdx.clientName = m_state.clientName;

    // if that merging index is "inside" another one, then append it right after the "parent" .
    if ( mIt != parentNode->mergingIndices.end() )
        parentNode->mergingIndices.insert( ++mIt, newIdx );
    else
        parentNode->mergingIndices.append( newIdx );

    if ( mergingName == defaultMergingName )

        ignoreDefaultMergingIndex = true;

    // re-calculate the running default and client merging indices.
    m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
    parentNode->calcMergingIndex( TQString(), m_state.currentClientMergingIt,
                                  m_state, ignoreDefaultMergingIndex );
}

void BuildHelper::processContainerElement( const TQDomElement &e, const TQString &tag,
                                           const TQString &name )
{
    static const TQString &defaultMergingName = TDEGlobal::staticQString( "<default>" );

    ContainerNode *containerNode = parentNode->findContainer( name, tag,
                                                              &containerList,
                                                              m_state.guiClient );

    if ( !containerNode )
    {
        MergingIndexList::Iterator it( m_state.currentClientMergingIt );
        TQString group;

        int idx = calcMergingIndex( e, it, group );

        int id;

        KXMLGUIBuilder *builder;

        TQWidget *container = createContainer( parentNode->container, idx, e, id, &builder );

        // no container? (probably some <text> tag or so ;-)
        if ( !container )
            return;

        parentNode->adjustMergingIndices( 1, it );

        assert( !parentNode->findContainerNode( container ) );

        containerList.append( container );

        TQString mergingName;
        if ( it != parentNode->mergingIndices.end() )
            mergingName = (*it).mergingName;

        TQStringList cusTags = m_state.builderCustomTags;
        TQStringList conTags = m_state.builderContainerTags;
        if ( builder != m_state.builder )
        {
            cusTags = m_state.clientBuilderCustomTags;
            conTags = m_state.clientBuilderContainerTags;
        }

        containerNode = new ContainerNode( container, tag, name, parentNode,
                                           m_state.guiClient, builder, id,
                                           mergingName, group, cusTags, conTags );
    }

    BuildHelper( m_state, containerNode ).build( e );

    // and re-calculate running values, for better performance
    m_state.currentDefaultMergingIt = parentNode->findIndex( defaultMergingName );
    parentNode->calcMergingIndex( TQString(), m_state.currentClientMergingIt,
                                  m_state, ignoreDefaultMergingIndex );
}

TQWidget *BuildHelper::createContainer( TQWidget *parent, int index,
                                       const TQDomElement &element, int &id,
                                       KXMLGUIBuilder **builder )
{
    TQWidget *res = 0L;

    if ( m_state.clientBuilder )
    {
        res = m_state.clientBuilder->createContainer( parent, index, element, id );

        if ( res )
        {
            *builder = m_state.clientBuilder;
            return res;
        }
    }

    TDEInstance *oldInstance = m_state.builder->builderInstance();
    KXMLGUIClient *oldClient = m_state.builder->builderClient();

    m_state.builder->setBuilderClient( m_state.guiClient );

    res = m_state.builder->createContainer( parent, index, element, id );

    m_state.builder->setBuilderInstance( oldInstance );
    m_state.builder->setBuilderClient( oldClient );

    if ( res )
        *builder = m_state.builder;

    return res;
}

void BuildState::reset()
{
    clientName = TQString();
    actionListName = TQString();
    actionList.clear();
    guiClient = 0;
    clientBuilder = 0;

    currentDefaultMergingIt = currentClientMergingIt = MergingIndexList::Iterator();
}
