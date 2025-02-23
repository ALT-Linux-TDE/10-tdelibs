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
#ifndef __kxmlguifactory_p_h__
#define __kxmlguifactory_p_h__

#include <tqstringlist.h>
#include <tqmap.h>
#include <tqdom.h>
#include <tqvaluestack.h>

#include <tdeaction.h>

class TQWidget;
class KXMLGUIClient;
class KXMLGUIBuilder;
class KXMLGUIFactory;

namespace KXMLGUI
{

struct BuildState;

class TDEUI_EXPORT ActionList : public TQPtrList<TDEAction>
{
public:
    ActionList() {}
    ActionList( const TQPtrList<TDEAction> &rhs )
        : TQPtrList<TDEAction>( rhs )
    {}
    ActionList &operator=( const TQPtrList<TDEAction> &rhs )
    { TQPtrList<TDEAction>::operator=( rhs ); return *this; }

    void plug( TQWidget *container, int index ) const;
    void unplug( TQWidget *container ) const;
};

typedef TQPtrListIterator<TDEAction> ActionListIt;
typedef TQMap< TQString, ActionList > ActionListMap;

/*
 * This structure is used to know to which client certain actions and custom elements
 * (i.e. menu separators) belong.
 * We do not only use a ContainerClient per GUIClient but also per merging group.
 *
 * groupName : Used for grouped merging. Specifies the group name to which these actions/elements
 * belong to.
 * actionLists : maps from action list name to action list.
 * mergingName : The (named) merging point.
 *
 * A ContainerClient always belongs to a ContainerNode.
 */
struct ContainerClient
{
    KXMLGUIClient *client;
    ActionList actions;
    TQValueList<int> customElements;
    TQString groupName; //is empty if no group client
    ActionListMap actionLists;
    TQString mergingName;
};
typedef TQPtrList<ContainerClient> ContainerClientList;
typedef TQPtrListIterator<ContainerClient> ContainerClientListIt;

struct ContainerNode;

struct MergingIndex
{
    int value; // the actual index value, used as index for plug() or createContainer() calls
    TQString mergingName; // the name of the merging index (i.e. the name attribute of the
                         // Merge or DefineGroup tag)
    TQString clientName; // the name of the client that defined this index
};
typedef TQValueList<MergingIndex> MergingIndexList;

/*
 * Here we store detailed information about a container, its clients (client=a guiclient having actions
 * plugged into the container), child nodes, naming information (tagname and name attribute) and
 * merging index information, to plug/insert new actions/items a the correct position.
 *
 * The builder variable is needed for using the proper GUIBuilder for destruction ( to use the same for
 * con- and destruction ). The builderCustomTags and builderContainerTags variables are cached values
 * of what the corresponding methods of the GUIBuilder which built the container return. The stringlists
 * is shared all over the place, so there's no need to worry about memory consumption for these
 * variables :-)
 *
 * The mergingIndices list contains the merging indices ;-) , as defined by <Merge>, <DefineGroup>
 * or by <ActionList> tags. The order of these index structures within the mergingIndices list
 * is (and has to be) identical with the order in the DOM tree.
 *
 * Beside the merging indices we have the "real" index of the container. It points to the next free
 * position.
 * (used when no merging index is used for a certain action, custom element or sub-container)
 */
struct TDEUI_EXPORT ContainerNode
{
    ContainerNode( TQWidget *_container, const TQString &_tagName, const TQString &_name,
                   ContainerNode *_parent = 0L, KXMLGUIClient *_client = 0L,
                   KXMLGUIBuilder *_builder = 0L, int id = -1,
                   const TQString &_mergingName = TQString::null,
                   const TQString &groupName = TQString::null,
                   const TQStringList &customTags = TQStringList(),
                   const TQStringList &containerTags = TQStringList() );

    ContainerNode *parent;
    KXMLGUIClient *client;
    KXMLGUIBuilder *builder;
    TQStringList builderCustomTags;
    TQStringList builderContainerTags;
    TQWidget *container;
    int containerId;

    TQString tagName;
    TQString name;

    TQString groupName; //is empty if the container is in no group

    ContainerClientList clients;
    TQPtrList<ContainerNode> children;

    int index;
    MergingIndexList mergingIndices;

    TQString mergingName;

    void clearChildren() { children.clear(); }
    void removeChild( ContainerNode *child );

    MergingIndexList::Iterator findIndex( const TQString &name );
    ContainerNode *findContainerNode( TQWidget *container );
    ContainerNode *findContainer( const TQString &_name, bool tag );
    ContainerNode *findContainer( const TQString &name, const TQString &tagName,
                                  const TQPtrList<TQWidget> *excludeList,
                                  KXMLGUIClient *currClient );

    ContainerClient *findChildContainerClient( KXMLGUIClient *currentGUIClient, 
                                               const TQString &groupName, 
                                               const MergingIndexList::Iterator &mergingIdx );

    void plugActionList( BuildState &state );
    void plugActionList( BuildState &state, const MergingIndexList::Iterator &mergingIdxIt );

    void unplugActionList( BuildState &state );
    void unplugActionList( BuildState &state, const MergingIndexList::Iterator &mergingIdxIt );

    void adjustMergingIndices( int offset, const MergingIndexList::Iterator &it );

    bool destruct( TQDomElement element, BuildState &state );
    void destructChildren( const TQDomElement &element, BuildState &state );
    static TQDomElement findElementForChild( const TQDomElement &baseElement, 
                                            ContainerNode *childNode );
    void unplugActions( BuildState &state );
    void unplugClient( ContainerClient *client );

    void reset();

    int calcMergingIndex( const TQString &mergingName,
                          MergingIndexList::Iterator &it,
                          BuildState &state,
                          bool ignoreDefaultMergingIndex );
};

typedef TQPtrList<ContainerNode> ContainerNodeList;
typedef TQPtrListIterator<ContainerNode> ContainerNodeListIt;

class TDEUI_EXPORT BuildHelper
{
public:
    BuildHelper( BuildState &state, 
                 ContainerNode *node );

    void build( const TQDomElement &element );

private:
    void processElement( const TQDomElement &element );

    void processActionOrCustomElement( const TQDomElement &e, bool isActionTag );
    bool processActionElement( const TQDomElement &e, int idx );
    bool processCustomElement( const TQDomElement &e, int idx );

    void processStateElement( const TQDomElement &element );

    void processMergeElement( const TQString &tag, const TQString &name, const TQDomElement &e );

    void processContainerElement( const TQDomElement &e, const TQString &tag,
                                  const TQString &name );


    TQWidget *createContainer( TQWidget *parent, int index, const TQDomElement &element,
                              int &id, KXMLGUIBuilder **builder );

    int calcMergingIndex( const TQDomElement &element, MergingIndexList::Iterator &it, TQString &group );

    TQStringList customTags;
    TQStringList containerTags;

    TQPtrList<TQWidget> containerList;

    ContainerClient *containerClient;

    bool ignoreDefaultMergingIndex;

    BuildState &m_state;

    ContainerNode *parentNode;
};

struct TDEUI_EXPORT BuildState
{
    BuildState() : guiClient( 0 ), builder( 0 ), clientBuilder( 0 ) {}

    void reset();

    TQString clientName;

    TQString actionListName;
    ActionList actionList;

    KXMLGUIClient *guiClient;

    MergingIndexList::Iterator currentDefaultMergingIt;
    MergingIndexList::Iterator currentClientMergingIt;

    KXMLGUIBuilder *builder;
    TQStringList builderCustomTags;
    TQStringList builderContainerTags;

    KXMLGUIBuilder *clientBuilder;
    TQStringList clientBuilderCustomTags;
    TQStringList clientBuilderContainerTags;
};

typedef TQValueStack<BuildState> BuildStateStack;

}

#endif
