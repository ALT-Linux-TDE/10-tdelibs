/* This file is part of the KDE libraries
   Copyright (C) 1999,2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

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

#include "kxmlguifactory.h"
#include "kxmlguifactory_p.h"
#include "kxmlguiclient.h"
#include "kxmlguibuilder.h"

#include <assert.h>

#include <tqdir.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqwidget.h>
#include <tqdatetime.h>
#include <tqvariant.h>

#include <tdeaction.h>
#include <kdebug.h>
#include <kinstance.h>
#include <tdeglobal.h>
#include <tdeshortcut.h>
#include <kstandarddirs.h>
#include <kkeydialog.h>

using namespace KXMLGUI;

/*
 * TODO:     - make more use of TQValueList instead of QPtrList
 */

class KXMLGUIFactoryPrivate : public BuildState
{
public:
    KXMLGUIFactoryPrivate()
    {
        static const TQString &defaultMergingName = TDEGlobal::staticQString( "<default>" );
        static const TQString &actionList = TDEGlobal::staticQString( "actionlist" );
        static const TQString &name = TDEGlobal::staticQString( "name" );

        m_rootNode = new ContainerNode( 0L, TQString::null, 0L );
        m_defaultMergingName = defaultMergingName;
        tagActionList = actionList;
        attrName = name;
    }
    ~KXMLGUIFactoryPrivate()
    {
        delete m_rootNode;
    }

    void pushState()
    {
        m_stateStack.push( *this );
    }

    void popState()
    {
        BuildState::operator=( m_stateStack.pop() );
    }

    ContainerNode *m_rootNode;

    TQString m_defaultMergingName;

    /*
     * Contains the container which is searched for in ::container .
     */
    TQString m_containerName;

    /*
     * List of all clients
     */
    TQPtrList<KXMLGUIClient> m_clients;

    TQString tagActionList;

    TQString attrName;

    BuildStateStack m_stateStack;
};

TQString KXMLGUIFactory::readConfigFile( const TQString &filename, const TDEInstance *instance )
{
    return readConfigFile( filename, false, instance );
}

TQString KXMLGUIFactory::readConfigFile( const TQString &filename, bool never_null, const TDEInstance *_instance )
{
    const TDEInstance *instance = _instance ? _instance : TDEGlobal::instance();
    TQString xml_file;

    if (!TQDir::isRelativePath(filename))
        xml_file = filename;
    else
    {
        xml_file = locate("data", TQString::fromLatin1(instance->instanceName() + '/' ) + filename);
        if ( !TQFile::exists( xml_file ) )
          xml_file = locate( "data", filename );
    }

    TQFile file( xml_file );
    if ( !file.open( IO_ReadOnly ) )
    {
        kdError(240) << "No such XML file " << filename << endl;
        if ( never_null )
            return TQString::fromLatin1( "<!DOCTYPE kpartgui>\n<kpartgui name=\"empty\">\n</kpartgui>" );
        else
            return TQString::null;
    }

#if TQT_VERSION <= 0x030302
    // Work around bug in TQString::fromUtf8 (which calls strlen).
    TQByteArray buffer(file.size() + 1);
    buffer = file.readAll();
    if(!buffer.isEmpty())
        buffer[ buffer.size() - 1 ] = '\0';
    else
        return TQString::null;
#else
    TQByteArray buffer(file.readAll());
#endif
    return TQString::fromUtf8(buffer.data(), buffer.size());
}

bool KXMLGUIFactory::saveConfigFile( const TQDomDocument& doc,
                                     const TQString& filename, const TDEInstance *_instance )
{
    const TDEInstance *instance = _instance ? _instance : TDEGlobal::instance();
    TQString xml_file(filename);

    if (TQDir::isRelativePath(xml_file))
        xml_file = locateLocal("data", TQString::fromLatin1( instance->instanceName() + '/' )
                               + filename);

    TQFile file( xml_file );
    if ( !file.open( IO_WriteOnly ) )
    {
        kdError(240) << "Could not write to " << filename << endl;
        return false;
    }

    // write out our document
    TQTextStream ts(&file);
    ts.setEncoding( TQTextStream::UnicodeUTF8 );
    ts << doc;

    file.close();
    return true;
}

TQString KXMLGUIFactory::documentToXML( const TQDomDocument& doc )
{
    TQString str;
    TQTextStream ts(&str, IO_WriteOnly);
    ts.setEncoding( TQTextStream::UnicodeUTF8 );
    ts << doc;
    return str;
}

TQString KXMLGUIFactory::elementToXML( const TQDomElement& elem )
{
    TQString str;
    TQTextStream ts(&str, IO_WriteOnly);
    ts.setEncoding( TQTextStream::UnicodeUTF8 );
    ts << elem;
    return str;
}

void KXMLGUIFactory::removeDOMComments( TQDomNode &node )
{
    TQDomNode n = node.firstChild();
    while ( !n.isNull() )
    {
        if ( n.nodeType() == TQDomNode::CommentNode )
        {
            TQDomNode tmp = n;
            n = n.nextSibling();
            node.removeChild( tmp );
        }
        else
        {
            TQDomNode tmp = n;
            n = n.nextSibling();
            removeDOMComments( tmp );
        }
    }
}

KXMLGUIFactory::KXMLGUIFactory( KXMLGUIBuilder *builder, TQObject *parent, const char *name )
    : TQObject( parent, name )
{
    d = new KXMLGUIFactoryPrivate;
    d->builder = builder;
    d->guiClient = 0;
    if ( d->builder )
    {
        d->builderContainerTags = d->builder->containerTags();
        d->builderCustomTags = d->builder->customTags();
    }
}

KXMLGUIFactory::~KXMLGUIFactory()
{
    delete d;
}

void KXMLGUIFactory::addClient( KXMLGUIClient *client )
{
    kdDebug(1002) << "KXMLGUIFactory::addClient( " << client << " )" << endl; // ellis
    static const TQString &actionPropElementName = TDEGlobal::staticQString( "ActionProperties" );

    if ( client->factory() ) {
        if ( client->factory() == this )
            return;
        else
            client->factory()->removeClient( client ); //just in case someone does stupid things ;-)
    }

    d->pushState();

//    TQTime dt; dt.start();

    d->guiClient = client;

    // add this client to our client list
    if ( !d->m_clients.containsRef( client ) )
        d->m_clients.append( client );
    else
        kdDebug(1002) << "XMLGUI client already added " << client << endl;

    // Tell the client that plugging in is process and
    //  let it know what builder widget its mainwindow shortcuts
    //  should be attached to.
    client->beginXMLPlug( d->builder->widget() );

    // try to use the build document for building the client's GUI, as the build document
    // contains the correct container state information (like toolbar positions, sizes, etc.) .
    // if there is non available, then use the "real" document.
    TQDomDocument doc = client->xmlguiBuildDocument();
    if ( doc.documentElement().isNull() )
        doc = client->domDocument();

    TQDomElement docElement = doc.documentElement();

    d->m_rootNode->index = -1;

    // cache some variables

    d->clientName = docElement.attribute( d->attrName );
    d->clientBuilder = client->clientBuilder();

    if ( d->clientBuilder )
    {
        d->clientBuilderContainerTags = d->clientBuilder->containerTags();
        d->clientBuilderCustomTags = d->clientBuilder->customTags();
    }
    else
    {
        d->clientBuilderContainerTags.clear();
        d->clientBuilderCustomTags.clear();
    }

    // process a possibly existing actionproperties section

    TQDomElement actionPropElement = docElement.namedItem( actionPropElementName ).toElement();
    if ( actionPropElement.isNull() )
        actionPropElement = docElement.namedItem( actionPropElementName.lower() ).toElement();

    if ( !actionPropElement.isNull() )
        applyActionProperties( actionPropElement );

    BuildHelper( *d, d->m_rootNode ).build( docElement );

    // let the client know that we built its GUI.
    client->setFactory( this );

    // call the finalizeGUI method, to fix up the positions of toolbars for example.
    // ### FIXME : obey client builder
    // --- Well, toolbars have a bool "positioned", so it doesn't really matter,
    // if we call positionYourself on all of them each time. (David)
    d->builder->finalizeGUI( d->guiClient );

    // reset some variables, for safety
    d->BuildState::reset();

    client->endXMLPlug();

    d->popState();

    emit clientAdded( client );

    // build child clients
    if ( client->childClients()->count() > 0 )
    {
        const TQPtrList<KXMLGUIClient> *children = client->childClients();
        TQPtrListIterator<KXMLGUIClient> childIt( *children );
        for (; childIt.current(); ++childIt )
            addClient( childIt.current() );
    }

//    kdDebug() << "addClient took " << dt.elapsed() << endl;
}

void KXMLGUIFactory::removeClient( KXMLGUIClient *client )
{
    kdDebug(1002) << "KXMLGUIFactory::removeClient( " << client << " )" << endl; // ellis

    // don't try to remove the client's GUI if we didn't build it
    if ( !client || client->factory() != this )
        return;

    // remove this client from our client list
    d->m_clients.removeRef( client );

    // remove child clients first
    if ( client->childClients()->count() > 0 )
    {
        const TQPtrList<KXMLGUIClient> *children = client->childClients();
        TQPtrListIterator<KXMLGUIClient> childIt( *children );
        childIt.toLast();
        for (; childIt.current(); --childIt )
            removeClient( childIt.current() );
    }

    kdDebug(1002) << "KXMLGUIFactory::removeServant, calling removeRecursive" << endl;

    d->pushState();

    // cache some variables

    d->guiClient = client;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );
    d->clientBuilder = client->clientBuilder();

    client->setFactory( 0L );

    // if we don't have a build document for that client, yet, then create one by
    // cloning the original document, so that saving container information in the
    // DOM tree does not touch the original document.
    TQDomDocument doc = client->xmlguiBuildDocument();
    if ( doc.documentElement().isNull() )
    {
        doc = client->domDocument().cloneNode( true ).toDocument();
        client->setXMLGUIBuildDocument( doc );
    }

    d->m_rootNode->destruct( doc.documentElement(), *d );

    d->builder->finalizeGUI( d->guiClient ); //JoWenn

    // reset some variables
    d->BuildState::reset();

    // This will destruct the TDEAccel object built around the given widget.
    client->prepareXMLUnplug( d->builder->widget() );

    d->popState();

    emit clientRemoved( client );
}

TQPtrList<KXMLGUIClient> KXMLGUIFactory::clients() const
{
    return d->m_clients;
}

TQWidget *KXMLGUIFactory::container( const TQString &containerName, KXMLGUIClient *client,
                                    bool useTagName )
{
    d->pushState();
    d->m_containerName = containerName;
    d->guiClient = client;

    TQWidget *result = findRecursive( d->m_rootNode, useTagName );

    d->guiClient = 0L;
    d->m_containerName = TQString::null;

    d->popState();

    return result;
}

TQPtrList<TQWidget> KXMLGUIFactory::containers( const TQString &tagName )
{
    return findRecursive( d->m_rootNode, tagName );
}

void KXMLGUIFactory::reset()
{
    d->m_rootNode->reset();

    d->m_rootNode->clearChildren();
}

void KXMLGUIFactory::resetContainer( const TQString &containerName, bool useTagName )
{
    if ( containerName.isEmpty() )
        return;

    ContainerNode *container = d->m_rootNode->findContainer( containerName, useTagName );

    if ( !container )
        return;

    ContainerNode *parent = container->parent;
    if ( !parent )
        return;

    //  resetInternal( container );

    parent->removeChild( container );
}

TQWidget *KXMLGUIFactory::findRecursive( KXMLGUI::ContainerNode *node, bool tag )
{
    if ( ( ( !tag && node->name == d->m_containerName ) ||
           ( tag && node->tagName == d->m_containerName ) ) &&
         ( !d->guiClient || node->client == d->guiClient ) )
        return node->container;

    TQPtrListIterator<ContainerNode> it( node->children );
    for (; it.current(); ++it )
    {
        TQWidget *cont = findRecursive( it.current(), tag );
        if ( cont )
            return cont;
    }

    return 0L;
}

TQPtrList<TQWidget> KXMLGUIFactory::findRecursive( KXMLGUI::ContainerNode *node,
                                                 const TQString &tagName )
{
    TQPtrList<TQWidget> res;

    if ( node->tagName == tagName.lower() )
        res.append( node->container );

    TQPtrListIterator<KXMLGUI::ContainerNode> it( node->children );
    for (; it.current(); ++it )
    {
        TQPtrList<TQWidget> lst = findRecursive( it.current(), tagName );
        TQPtrListIterator<TQWidget> wit( lst );
        for (; wit.current(); ++wit )
            res.append( wit.current() );
    }

    return res;
}

void KXMLGUIFactory::plugActionList( KXMLGUIClient *client, const TQString &name,
                                     const TQPtrList<TDEAction> &actionList )
{
    d->pushState();
    d->guiClient = client;
    d->actionListName = name;
    d->actionList = actionList;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );

    d->m_rootNode->plugActionList( *d );

    d->BuildState::reset();
    d->popState();
}

void KXMLGUIFactory::unplugActionList( KXMLGUIClient *client, const TQString &name )
{
    d->pushState();
    d->guiClient = client;
    d->actionListName = name;
    d->clientName = client->domDocument().documentElement().attribute( d->attrName );

    d->m_rootNode->unplugActionList( *d );

    d->BuildState::reset();
    d->popState();
}

void KXMLGUIFactory::applyActionProperties( const TQDomElement &actionPropElement )
{
    static const TQString &tagAction = TDEGlobal::staticQString( "action" );

    for (TQDomNode n = actionPropElement.firstChild();
         !n.isNull(); n = n.nextSibling() )
    {
        TQDomElement e = n.toElement();
        if ( e.tagName().lower() != tagAction )
            continue;

        TDEAction *action = d->guiClient->action( e );
        if ( !action )
            continue;

        configureAction( action, e.attributes() );
    }
}

void KXMLGUIFactory::configureAction( TDEAction *action, const TQDomNamedNodeMap &attributes )
{
    for ( uint i = 0; i < attributes.length(); i++ )
    {
        TQDomAttr attr = attributes.item( i ).toAttr();
        if ( attr.isNull() )
            continue;

        configureAction( action, attr );
    }
}

void KXMLGUIFactory::configureAction( TDEAction *action, const TQDomAttr &attribute )
{
    static const TQString &attrShortcut = TDEGlobal::staticQString( "shortcut" );

    TQString attrName = attribute.name();
    // If the attribute is a deprecated "accel", change to "shortcut".
    if ( attrName.lower() == "accel" )
        attrName = attrShortcut;

    TQVariant propertyValue;

    TQVariant::Type propertyType = action->property( attrName.latin1() ).type();

    if ( propertyType == TQVariant::Int )
        propertyValue = TQVariant( attribute.value().toInt() );
    else if ( propertyType == TQVariant::UInt )
        propertyValue = TQVariant( attribute.value().toUInt() );
    else
        propertyValue = TQVariant( attribute.value() );

    action->setProperty( attrName.latin1(), propertyValue );
}


int KXMLGUIFactory::configureShortcuts(bool bAllowLetterShortcuts , bool bSaveSettings )
{
	KKeyDialog dlg( bAllowLetterShortcuts, dynamic_cast<TQWidget*>(parent()) );
	TQPtrListIterator<KXMLGUIClient> it( d->m_clients );
	KXMLGUIClient *client;
	while( (client=it.current()) !=0 )
	{
		++it;
		if(!client->xmlFile().isEmpty())
			dlg.insert( client->actionCollection() );
	}
	return dlg.configure(bSaveSettings);
}

TQDomElement KXMLGUIFactory::actionPropertiesElement( TQDomDocument& doc )
{
	const TQString tagActionProp = TQString::fromLatin1("ActionProperties");
	// first, lets see if we have existing properties
	TQDomElement elem;
	TQDomNode it = doc.documentElement().firstChild();
	for( ; !it.isNull(); it = it.nextSibling() ) {
		TQDomElement e = it.toElement();
		if( e.tagName() == tagActionProp ) {
			elem = e;
			break;
		}
	}

	// if there was none, create one
	if( elem.isNull() ) {
		elem = doc.createElement( tagActionProp );
		doc.documentElement().appendChild( elem );
	}
	return elem;
}

TQDomElement KXMLGUIFactory::findActionByName( TQDomElement& elem, const TQString& sName, bool create )
{
        static const TQString& attrName = TDEGlobal::staticQString( "name" );
	static const TQString& tagAction = TDEGlobal::staticQString( "Action" );
	for( TQDomNode it = elem.firstChild(); !it.isNull(); it = it.nextSibling() ) {
		TQDomElement e = it.toElement();
		if( e.attribute( attrName ) == sName )
			return e;
	}

	if( create ) {
		TQDomElement act_elem = elem.ownerDocument().createElement( tagAction );
		act_elem.setAttribute( attrName, sName );
                elem.appendChild( act_elem );
                return act_elem;
	}
        return TQDomElement();
}

void KXMLGUIFactory::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "kxmlguifactory.moc"
