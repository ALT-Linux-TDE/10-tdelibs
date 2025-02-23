/* This file is part of the KDE libraries
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kxmlguiclient.h"
#include "kxmlguifactory.h"
#include "kxmlguibuilder.h"

#include <tqdir.h>
#include <tqfile.h>
#include <tqdom.h>
#include <tqtextstream.h>
#include <tqregexp.h>
#include <tqguardedptr.h>

#include <kinstance.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <tdeaction.h>
#include <tdeapplication.h>

#include <assert.h>

class KXMLGUIClientPrivate
{
public:
  KXMLGUIClientPrivate()
  {
    m_instance = TDEGlobal::instance();
    m_parent = 0L;
    m_builder = 0L;
    m_actionCollection = 0;
  }
  ~KXMLGUIClientPrivate()
  {
  }

  TDEInstance *m_instance;

  TQDomDocument m_doc;
  TDEActionCollection *m_actionCollection;
  TQDomDocument m_buildDocument;
  TQGuardedPtr<KXMLGUIFactory> m_factory;
  KXMLGUIClient *m_parent;
  //TQPtrList<KXMLGUIClient> m_supers;
  TQPtrList<KXMLGUIClient> m_children;
  KXMLGUIBuilder *m_builder;
  TQString m_xmlFile;
  TQString m_localXMLFile;
};

KXMLGUIClient::KXMLGUIClient()
{
  d = new KXMLGUIClientPrivate;
}

KXMLGUIClient::KXMLGUIClient( KXMLGUIClient *parent )
{
  d = new KXMLGUIClientPrivate;
  parent->insertChildClient( this );
}

KXMLGUIClient::~KXMLGUIClient()
{
  if ( d->m_parent )
    d->m_parent->removeChildClient( this );

  TQPtrListIterator<KXMLGUIClient> it( d->m_children );
  for ( ; it.current(); ++it ) {
      assert( it.current()->d->m_parent == this );
      it.current()->d->m_parent = 0;
  }

  delete d->m_actionCollection;
  delete d;
}

TDEAction *KXMLGUIClient::action( const char *name ) const
{
  TDEAction* act = actionCollection()->action( name );
  if ( !act ) {
    TQPtrListIterator<KXMLGUIClient> childIt( d->m_children );
    for (; childIt.current(); ++childIt ) {
      act = childIt.current()->actionCollection()->action( name );
      if ( act )
        break;
    }
  }
  return act;
}

TDEActionCollection *KXMLGUIClient::actionCollection() const
{
  if ( !d->m_actionCollection )
  {
    d->m_actionCollection = new TDEActionCollection(
      "KXMLGUIClient-TDEActionCollection", this );
  }
  return d->m_actionCollection;
}

TDEAction *KXMLGUIClient::action( const TQDomElement &element ) const
{
  static const TQString &attrName = TDEGlobal::staticQString( "name" );
  return actionCollection()->action( element.attribute( attrName ).latin1() );
}

TDEInstance *KXMLGUIClient::instance() const
{
  return d->m_instance;
}

TQDomDocument KXMLGUIClient::domDocument() const
{
  return d->m_doc;
}

TQString KXMLGUIClient::xmlFile() const
{
  return d->m_xmlFile;
}

TQString KXMLGUIClient::localXMLFile() const
{
  if ( !d->m_localXMLFile.isEmpty() )
    return d->m_localXMLFile;

  if ( !TQDir::isRelativePath(d->m_xmlFile) )
      return TQString::null; // can't save anything here

  return locateLocal( "data", TQString::fromLatin1( instance()->instanceName() + '/' ) + d->m_xmlFile );
}


void KXMLGUIClient::reloadXML()
{
    TQString file( xmlFile() );
    if ( !file.isEmpty() )
        setXMLFile( file );
}

void KXMLGUIClient::setInstance( TDEInstance *instance )
{
  d->m_instance = instance;
  actionCollection()->setInstance( instance );
  if ( d->m_builder )
    d->m_builder->setBuilderClient( this );
}

void KXMLGUIClient::setXMLFile( const TQString& _file, bool merge, bool setXMLDoc )
{
  // store our xml file name
  if ( !_file.isNull() ) {
    d->m_xmlFile = _file;
    actionCollection()->setXMLFile( _file );
  }

  if ( !setXMLDoc )
    return;

  TQString file = _file;
  if ( TQDir::isRelativePath(file) )
  {
    TQString doc;

    TQString filter = TQString::fromLatin1( instance()->instanceName() + '/' ) + _file;

    TQStringList allFiles = instance()->dirs()->findAllResources( "data", filter ) + instance()->dirs()->findAllResources( "data", _file );

    file = findMostRecentXMLFile( allFiles, doc );

    if ( file.isEmpty() )
    {
      // this might or might not be an error.  for the time being,
      // let's treat this as if it isn't a problem and the user just
      // wants the global standards file

      // however if a non-empty file gets passed and we can't find it we might
      // inform the developer using some debug output
      if ( !_file.isEmpty() )
          kdWarning() << "KXMLGUIClient::setXMLFile: cannot find .rc file " << _file << endl;

      setXML( TQString::null, true );
      return;
    }
    else if ( !doc.isEmpty() )
    {
      setXML( doc, merge );
      return;
    }
  }

  TQString xml = KXMLGUIFactory::readConfigFile( file );
  setXML( xml, merge );
}

void KXMLGUIClient::setLocalXMLFile( const TQString &file )
{
    d->m_localXMLFile = file;
}

void KXMLGUIClient::setXML( const TQString &document, bool merge )
{
  TQDomDocument doc;
  doc.setContent( document );
  setDOMDocument( doc, merge );
}

void KXMLGUIClient::setDOMDocument( const TQDomDocument &document, bool merge )
{
  if ( merge )
  {
    TQDomElement base = d->m_doc.documentElement();

    TQDomElement e = document.documentElement();

    // merge our original (global) xml with our new one
    mergeXML(base, e, actionCollection());

    // reassign our pointer as mergeXML might have done something
    // strange to it
    base = d->m_doc.documentElement();

    // we want some sort of failsafe.. just in case
    if ( base.isNull() )
      d->m_doc = document;
  }
  else
  {
    d->m_doc = document;
  }

  setXMLGUIBuildDocument( TQDomDocument() );
}

bool KXMLGUIClient::mergeXML( TQDomElement &base, const TQDomElement &additive, TDEActionCollection *actionCollection )
{
  static const TQString &tagAction = TDEGlobal::staticQString( "Action" );
  static const TQString &tagMerge = TDEGlobal::staticQString( "Merge" );
  static const TQString &tagSeparator = TDEGlobal::staticQString( "Separator" );
  static const TQString &attrName = TDEGlobal::staticQString( "name" );
  static const TQString &attrAppend = TDEGlobal::staticQString( "append" );
  static const TQString &attrWeakSeparator = TDEGlobal::staticQString( "weakSeparator" );
  static const TQString &tagMergeLocal = TDEGlobal::staticQString( "MergeLocal" );
  static const TQString &tagText = TDEGlobal::staticQString( "text" );
  static const TQString &attrAlreadyVisited = TDEGlobal::staticQString( "alreadyVisited" );
  static const TQString &attrNoMerge = TDEGlobal::staticQString( "noMerge" );
  static const TQString &attrOne = TDEGlobal::staticQString( "1" );

  // there is a possibility that we don't want to merge in the
  // additive.. rather, we might want to *replace* the base with the
  // additive.  this can be for any container.. either at a file wide
  // level or a simple container level.  we look for the 'noMerge'
  // tag, in any event and just replace the old with the new
  if ( additive.attribute(attrNoMerge) == attrOne ) // ### use toInt() instead? (Simon)
  {
    base.parentNode().replaceChild(additive, base);
    return true;
  }

  TQString tag;

  // iterate over all elements in the container (of the global DOM tree)
  TQDomNode n = base.firstChild();
  while ( !n.isNull() )
  {
    TQDomElement e = n.toElement();
    n = n.nextSibling(); // Advance now so that we can safely delete e
    if (e.isNull())
       continue;

    tag = e.tagName();

    // if there's an action tag in the global tree and the action is
    // not implemented, then we remove the element
    if ( tag == tagAction )
    {
      TQCString name =  e.attribute( attrName ).utf8(); // WABA
      if ( !actionCollection->action( name.data() ) ||
           (kapp && !kapp->authorizeTDEAction(name)))
      {
        // remove this child as we aren't using it
        base.removeChild( e );
        continue;
      }
    }

    // if there's a separator defined in the global tree, then add an
    // attribute, specifying that this is a "weak" separator
    else if ( tag == tagSeparator )
    {
      e.setAttribute( attrWeakSeparator, (uint)1 );

      // okay, hack time. if the last item was a weak separator OR
      // this is the first item in a container, then we nuke the
      // current one
      TQDomElement prev = e.previousSibling().toElement();
      if ( prev.isNull() ||
	 ( prev.tagName() == tagSeparator && !prev.attribute( attrWeakSeparator ).isNull() ) ||
	 ( prev.tagName() == tagText ) )
      {
        // the previous element was a weak separator or didn't exist
        base.removeChild( e );
        continue;
      }
    }

    // the MergeLocal tag lets us specify where non-standard elements
    // of the local tree shall be merged in.  After inserting the
    // elements we delete this element
    else if ( tag == tagMergeLocal )
    {
      TQDomNode it = additive.firstChild();
      while ( !it.isNull() )
      {
        TQDomElement newChild = it.toElement();
        it = it.nextSibling();
        if (newChild.isNull() )
          continue;

        if ( newChild.tagName() == tagText )
          continue;

        if ( newChild.attribute( attrAlreadyVisited ) == attrOne )
          continue;

        TQString itAppend( newChild.attribute( attrAppend ) );
        TQString elemName( e.attribute( attrName ) );

        if ( ( itAppend.isNull() && elemName.isEmpty() ) ||
             ( itAppend == elemName ) )
        {
          // first, see if this new element matches a standard one in
          // the global file.  if it does, then we skip it as it will
          // be merged in, later
          TQDomElement matchingElement = findMatchingElement( newChild, base );
          if ( matchingElement.isNull() || newChild.tagName() == tagSeparator )
            base.insertBefore( newChild, e );
        }
      }

      base.removeChild( e );
      continue;
    }

    // in this last case we check for a separator tag and, if not, we
    // can be sure that its a container --> proceed with child nodes
    // recursively and delete the just proceeded container item in
    // case its empty (if the recursive call returns true)
    else if ( tag != tagMerge )
    {
      // handle the text tag
      if ( tag == tagText )
        continue;

      TQDomElement matchingElement = findMatchingElement( e, additive );

      if ( !matchingElement.isNull() )
      {
        matchingElement.setAttribute( attrAlreadyVisited, (uint)1 );

        if ( mergeXML( e, matchingElement, actionCollection ) )
        {
          base.removeChild( e );
          continue;
        }

        // Merge attributes
        const TQDomNamedNodeMap attribs = matchingElement.attributes();
        const uint attribcount = attribs.count();

        for(uint i = 0; i < attribcount; ++i)
        {
          const TQDomNode node = attribs.item(i);
          e.setAttribute(node.nodeName(), node.nodeValue());
        }

        continue;
      }
      else
      {
        // this is an important case here! We reach this point if the
        // "local" tree does not contain a container definition for
        // this container. However we have to call mergeXML recursively
        // and make it check if there are actions implemented for this
        // container. *If* none, then we can remove this container now
        if ( mergeXML( e, TQDomElement(), actionCollection ) )
          base.removeChild( e );
        continue;
      }
    }
  }

  //here we append all child elements which were not inserted
  //previously via the LocalMerge tag
  n = additive.firstChild();
  while ( !n.isNull() )
  {
    TQDomElement e = n.toElement();
    n = n.nextSibling(); // Advance now so that we can safely delete e
    if (e.isNull())
       continue;

    TQDomElement matchingElement = findMatchingElement( e, base );

    if ( matchingElement.isNull() )
    {
      base.appendChild( e );
    }
  }

  // do one quick check to make sure that the last element was not
  // a weak separator
  TQDomElement last = base.lastChild().toElement();
  if ( (last.tagName() == tagSeparator) && (!last.attribute( attrWeakSeparator ).isNull()) )
  {
    base.removeChild( last );
  }

  // now we check if we are empty (in which case we return "true", to
  // indicate the caller that it can delete "us" (the base element
  // argument of "this" call)
  bool deleteMe = true;

  n = base.firstChild();
  while ( !n.isNull() )
  {
    TQDomElement e = n.toElement();
    n = n.nextSibling(); // Advance now so that we can safely delete e
    if (e.isNull())
       continue;

    tag = e.tagName();

    if ( tag == tagAction )
    {
      // if base contains an implemented action, then we must not get
      // deleted (note that the actionCollection contains both,
      // "global" and "local" actions
      if ( actionCollection->action( e.attribute( attrName ).utf8().data() ) )
      {
        deleteMe = false;
        break;
      }
    }
    else if ( tag == tagSeparator )
    {
      // if we have a separator which has *not* the weak attribute
      // set, then it must be owned by the "local" tree in which case
      // we must not get deleted either
      TQString weakAttr = e.attribute( attrWeakSeparator );
      if ( weakAttr.isEmpty() || weakAttr.toInt() != 1 )
      {
        deleteMe = false;
        break;
      }
    }

    // in case of a merge tag we have unlimited lives, too ;-)
    else if ( tag == tagMerge )
    {
//      deleteMe = false;
//      break;
        continue;
    }

    // a text tag is NOT enough to spare this container
    else if ( tag == tagText )
    {
      continue;
    }

    // what's left are non-empty containers! *don't* delete us in this
    // case (at this position we can be *sure* that the container is
    // *not* empty, as the recursive call for it was in the first loop
    // which deleted the element in case the call returned "true"
    else
    {
      deleteMe = false;
      break;
    }
  }

  return deleteMe;
}

TQDomElement KXMLGUIClient::findMatchingElement( const TQDomElement &base, const TQDomElement &additive )
{
  static const TQString &tagAction = TDEGlobal::staticQString( "Action" );
  static const TQString &tagMergeLocal = TDEGlobal::staticQString( "MergeLocal" );
  static const TQString &attrName = TDEGlobal::staticQString( "name" );

  TQDomNode n = additive.firstChild();
  while ( !n.isNull() )
  {
    TQDomElement e = n.toElement();
    n = n.nextSibling(); // Advance now so that we can safely delete e
    if (e.isNull())
       continue;

    // skip all action and merge tags as we will never use them
    if ( ( e.tagName() == tagAction ) || ( e.tagName() == tagMergeLocal ) )
    {
      continue;
    }

    // now see if our tags are equivalent
    if ( ( e.tagName() == base.tagName() ) &&
         ( e.attribute( attrName ) == base.attribute( attrName ) ) )
    {
        return e;
    }
  }

  // nope, return a (now) null element
  return TQDomElement();
}

void KXMLGUIClient::conserveMemory()
{
  d->m_doc = TQDomDocument();
  d->m_buildDocument = TQDomDocument();
}

void KXMLGUIClient::setXMLGUIBuildDocument( const TQDomDocument &doc )
{
  d->m_buildDocument = doc;
}

TQDomDocument KXMLGUIClient::xmlguiBuildDocument() const
{
  return d->m_buildDocument;
}

void KXMLGUIClient::setFactory( KXMLGUIFactory *factory )
{
  d->m_factory = factory;
}

KXMLGUIFactory *KXMLGUIClient::factory() const
{
  return d->m_factory;
}

KXMLGUIClient *KXMLGUIClient::parentClient() const
{
  return d->m_parent;
}

void KXMLGUIClient::insertChildClient( KXMLGUIClient *child )
{
  if (  child->d->m_parent )
    child->d->m_parent->removeChildClient( child );
   d->m_children.append( child );
   child->d->m_parent = this;
}

void KXMLGUIClient::removeChildClient( KXMLGUIClient *child )
{
  assert( d->m_children.containsRef( child ) );
  d->m_children.removeRef( child );
  child->d->m_parent = 0;
}

/*bool KXMLGUIClient::addSuperClient( KXMLGUIClient *super )
{
  if ( d->m_supers.contains( super ) )
    return false;
  d->m_supers.append( super );
  return true;
}*/

const TQPtrList<KXMLGUIClient> *KXMLGUIClient::childClients()
{
  return &d->m_children;
}

void KXMLGUIClient::setClientBuilder( KXMLGUIBuilder *builder )
{
  d->m_builder = builder;
  if ( builder )
    builder->setBuilderInstance( instance() );
}

KXMLGUIBuilder *KXMLGUIClient::clientBuilder() const
{
  return d->m_builder;
}

void KXMLGUIClient::plugActionList( const TQString &name, const TQPtrList<TDEAction> &actionList )
{
  if ( !d->m_factory )
    return;

  d->m_factory->plugActionList( this, name, actionList );
}

void KXMLGUIClient::unplugActionList( const TQString &name )
{
  if ( !d->m_factory )
    return;

  d->m_factory->unplugActionList( this, name );
}

TQString KXMLGUIClient::findMostRecentXMLFile( const TQStringList &files, TQString &doc )
{

  TQValueList<DocStruct> allDocuments;

  TQStringList::ConstIterator it = files.begin();
  TQStringList::ConstIterator end = files.end();
  for (; it != end; ++it )
  {
    //kdDebug() << "KXMLGUIClient::findMostRecentXMLFile " << *it << endl;
    TQString data = KXMLGUIFactory::readConfigFile( *it );
    DocStruct d;
    d.file = *it;
    d.data = data;
    allDocuments.append( d );
  }

  TQValueList<DocStruct>::Iterator best = allDocuments.end();
  uint bestVersion = 0;

  TQValueList<DocStruct>::Iterator docIt = allDocuments.begin();
  TQValueList<DocStruct>::Iterator docEnd = allDocuments.end();
  for (; docIt != docEnd; ++docIt )
  {
    TQString versionStr = findVersionNumber( (*docIt).data );
    if ( versionStr.isEmpty() )
      continue;

    bool ok = false;
    uint version = versionStr.toUInt( &ok );
    if ( !ok )
      continue;
    //kdDebug() << "FOUND VERSION " << version << endl;

    if ( version > bestVersion )
    {
      best = docIt;
      //kdDebug() << "best version is now " << version << endl;
      bestVersion = version;
    }
  }

  if ( best != docEnd )
  {
    if ( best != allDocuments.begin() )
    {
      TQValueList<DocStruct>::Iterator local = allDocuments.begin();

      // load the local document and extract the action properties
      TQDomDocument document;
      document.setContent( (*local).data );

      ActionPropertiesMap properties = extractActionProperties( document );

      // in case the document has a ActionProperties section
      // we must not delete it but copy over the global doc
      // to the local and insert the ActionProperties section
      if ( !properties.isEmpty() )
      {
          // now load the global one with the higher version number
          // into memory
          document.setContent( (*best).data );
          // and store the properties in there
          storeActionProperties( document, properties );

          (*local).data = document.toString();
          // make sure we pick up the new local doc, when we return later
          best = local;

          // write out the new version of the local document
          TQFile f( (*local).file );
          if ( f.open( IO_WriteOnly ) )
          {
            TQCString utf8data = (*local).data.utf8();
            f.writeBlock( utf8data.data(), utf8data.length() );
            f.close();
          }
      }
      else
      {
        TQString f = (*local).file;
        TQString backup = f + TQString::fromLatin1( ".backup" );
        TQDir dir;
        dir.rename( f, backup );
      }
    }
    doc = (*best).data;
    return (*best).file;
  }
  else if ( files.count() > 0 )
  {
    //kdDebug() << "returning first one..." << endl;
    doc = (*allDocuments.begin()).data;
    return (*allDocuments.begin()).file;
  }

  return TQString::null;
}



TQString KXMLGUIClient::findVersionNumber( const TQString &xml )
{
  enum { ST_START, ST_AFTER_OPEN, ST_AFTER_GUI,
               ST_EXPECT_VERSION, ST_VERSION_NUM} state = ST_START;
  for (unsigned int pos = 0; pos < xml.length(); pos++)
  {
    switch (state)
    {
      case ST_START:
        if (xml[pos] == '<')
          state = ST_AFTER_OPEN;
        break;
      case ST_AFTER_OPEN:
      {
        //Jump to gui..
        int guipos = xml.find("gui", pos, false /*case-insensitive*/);
        if (guipos == -1)
          return TQString::null; //Reject

        pos = guipos + 2; //Position at i, so we're moved ahead to the next character by the ++;
        state = ST_AFTER_GUI;
        break;
      }
      case ST_AFTER_GUI:
        state = ST_EXPECT_VERSION;
        break;
      case ST_EXPECT_VERSION:
      {
        int verpos =  xml.find("version=\"", pos, false /*case-insensitive*/);
        if (verpos == -1)
          return TQString::null; //Reject

        pos = verpos +  8; //v = 0, e = +1, r = +2, s = +3 , i = +4, o = +5, n = +6, = = +7, " = + 8
        state = ST_VERSION_NUM;
        break;
      }
      case ST_VERSION_NUM:
      {
        unsigned int endpos;
        for (endpos = pos; endpos <  xml.length(); endpos++)
        {
          if (xml[endpos].unicode() >= '0' && xml[endpos].unicode() <= '9')
            continue; //Number..
          if (xml[endpos].unicode() == '"') //End of parameter
            break;
          else //This shouldn't be here..
          {
            endpos = xml.length();
          }
        }

        if (endpos != pos && endpos < xml.length() )
        {
          TQString matchCandidate = xml.mid(pos, endpos - pos); //Don't include " ".
          return matchCandidate;
        }

        state = ST_EXPECT_VERSION; //Try to match a well-formed version..
        break;
      } //case..
    } //switch
  } //for

  return TQString::null;
}

KXMLGUIClient::ActionPropertiesMap KXMLGUIClient::extractActionProperties( const TQDomDocument &doc )
{
  ActionPropertiesMap properties;

  TQDomElement actionPropElement = doc.documentElement().namedItem( "ActionProperties" ).toElement();

  if ( actionPropElement.isNull() )
    return properties;

  TQDomNode n = actionPropElement.firstChild();
  while(!n.isNull())
  {
    TQDomElement e = n.toElement();
    n = n.nextSibling(); // Advance now so that we can safely delete e
    if ( e.isNull() )
      continue;

    if ( e.tagName().lower() != "action" )
      continue;

    TQString actionName = e.attribute( "name" );

    if ( actionName.isEmpty() )
      continue;

    TQMap<TQString, TQMap<TQString, TQString> >::Iterator propIt = properties.find( actionName );
    if ( propIt == properties.end() )
      propIt = properties.insert( actionName, TQMap<TQString, TQString>() );

    const TQDomNamedNodeMap attributes = e.attributes();
    const uint attributeslength = attributes.length();

    for ( uint i = 0; i < attributeslength; ++i )
    {
      const TQDomAttr attr = attributes.item( i ).toAttr();

      if ( attr.isNull() )
        continue;

      const TQString name = attr.name();

      if ( name == "name" || name.isEmpty() )
        continue;

      (*propIt)[ name ] = attr.value();
    }

  }

  return properties;
}

void KXMLGUIClient::storeActionProperties( TQDomDocument &doc, const ActionPropertiesMap &properties )
{
  TQDomElement actionPropElement = doc.documentElement().namedItem( "ActionProperties" ).toElement();

  if ( actionPropElement.isNull() )
  {
    actionPropElement = doc.createElement( "ActionProperties" );
    doc.documentElement().appendChild( actionPropElement );
  }

  while ( !actionPropElement.firstChild().isNull() )
    actionPropElement.removeChild( actionPropElement.firstChild() );

  ActionPropertiesMap::ConstIterator it = properties.begin();
  ActionPropertiesMap::ConstIterator end = properties.end();
  for (; it != end; ++it )
  {
    TQDomElement action = doc.createElement( "Action" );
    action.setAttribute( "name", it.key() );
    actionPropElement.appendChild( action );

    TQMap<TQString, TQString> attributes = (*it);
    TQMap<TQString, TQString>::ConstIterator attrIt = attributes.begin();
    TQMap<TQString, TQString>::ConstIterator attrEnd = attributes.end();
    for (; attrIt != attrEnd; ++attrIt )
      action.setAttribute( attrIt.key(), attrIt.data() );
  }
}

void KXMLGUIClient::addStateActionEnabled(const TQString& state,
                                          const TQString& action)
{
  StateChange stateChange = getActionsToChangeForState(state);

  stateChange.actionsToEnable.append( action );
  //kdDebug() << "KXMLGUIClient::addStateActionEnabled( " << state << ", " << action << ")" << endl;

  m_actionsStateMap.replace( state, stateChange );
}


void KXMLGUIClient::addStateActionDisabled(const TQString& state,
                                           const TQString& action)
{
  StateChange stateChange = getActionsToChangeForState(state);

  stateChange.actionsToDisable.append( action );
  //kdDebug() << "KXMLGUIClient::addStateActionDisabled( " << state << ", " << action << ")" << endl;

  m_actionsStateMap.replace( state, stateChange );
}


KXMLGUIClient::StateChange KXMLGUIClient::getActionsToChangeForState(const TQString& state)
{
  return m_actionsStateMap[state];
}


void KXMLGUIClient::stateChanged(const TQString &newstate, KXMLGUIClient::ReverseStateChange reverse)
{
  StateChange stateChange = getActionsToChangeForState(newstate);

  bool setTrue = (reverse == StateNoReverse);
  bool setFalse = !setTrue;

  // Enable actions which need to be enabled...
  //
  for ( TQStringList::Iterator it = stateChange.actionsToEnable.begin();
        it != stateChange.actionsToEnable.end(); ++it ) {

    TDEAction *action = actionCollection()->action((*it).latin1());
    if (action) action->setEnabled(setTrue);
  }

  // and disable actions which need to be disabled...
  //
  for ( TQStringList::Iterator it = stateChange.actionsToDisable.begin();
        it != stateChange.actionsToDisable.end(); ++it ) {

    TDEAction *action = actionCollection()->action((*it).latin1());
    if (action) action->setEnabled(setFalse);
  }

}

void KXMLGUIClient::beginXMLPlug( TQWidget *w )
{
  actionCollection()->beginXMLPlug( w );
  TQPtrListIterator<KXMLGUIClient> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    childIt.current()->actionCollection()->beginXMLPlug( w );
}

void KXMLGUIClient::endXMLPlug()
{
  actionCollection()->endXMLPlug();
  TQPtrListIterator<KXMLGUIClient> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    childIt.current()->actionCollection()->endXMLPlug();
}

void KXMLGUIClient::prepareXMLUnplug( TQWidget * )
{
  actionCollection()->prepareXMLUnplug();
  TQPtrListIterator<KXMLGUIClient> childIt( d->m_children );
  for (; childIt.current(); ++childIt )
    childIt.current()->actionCollection()->prepareXMLUnplug();
}

void KXMLGUIClient::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }
