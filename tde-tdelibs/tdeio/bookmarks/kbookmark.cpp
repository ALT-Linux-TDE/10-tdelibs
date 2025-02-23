/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#include "kbookmark.h"
#include <tqvaluestack.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kstringhandler.h>
#include <kinputdialog.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <assert.h>
#include <tdeapplication.h>
#include <dcopclient.h>
#include <kbookmarkmanager.h>

KBookmarkGroup::KBookmarkGroup()
 : KBookmark( TQDomElement() )
{
}

KBookmarkGroup::KBookmarkGroup( TQDomElement elem )
 : KBookmark(elem)
{
}

TQString KBookmarkGroup::groupAddress() const
{
    if (m_address.isEmpty())
        m_address = address();
    return m_address;
}

bool KBookmarkGroup::isOpen() const
{
    return element.attribute("folded") == "no"; // default is: folded
}

// Returns first element node equal to or after node n
static TQDomElement firstElement(TQDomNode n)
{
    while(!n.isNull() && !n.isElement())
        n = n.nextSibling();
    return n.toElement();
}

// Returns first element node equal to or before node n
static TQDomElement lastElement(TQDomNode n)
{
    while(!n.isNull() && !n.isElement())
        n = n.previousSibling();
    return n.toElement();
}

KBookmark KBookmarkGroup::first() const
{
    return KBookmark( nextKnownTag( firstElement(element.firstChild()), true ) );
}

KBookmark KBookmarkGroup::previous( const KBookmark & current ) const
{
    return KBookmark( nextKnownTag( lastElement(current.element.previousSibling()), false ) );
}

KBookmark KBookmarkGroup::next( const KBookmark & current ) const
{
    return KBookmark( nextKnownTag( firstElement(current.element.nextSibling()), true ) );
}

// KDE4: Change TQDomElement to TQDomNode so that we can get rid of
// firstElement() and lastElement()
TQDomElement KBookmarkGroup::nextKnownTag( TQDomElement start, bool goNext ) const
{
    static const TQString & bookmark = TDEGlobal::staticQString("bookmark");
    static const TQString & folder = TDEGlobal::staticQString("folder");
    static const TQString & separator = TDEGlobal::staticQString("separator");

    for( TQDomNode n = start; !n.isNull(); )
    {
        TQDomElement elem = n.toElement();
        TQString tag = elem.tagName();
        if (tag == folder || tag == bookmark || tag == separator)
            return elem;
        if (goNext)
            n = n.nextSibling();
        else
            n = n.previousSibling();
    }
    return TQDomElement();
}

KBookmarkGroup KBookmarkGroup::createNewFolder( KBookmarkManager* mgr, const TQString & text, bool emitSignal )
{
    TQString txt( text );
    if ( text.isEmpty() )
    {
        bool ok;
        TQString caption = parentGroup().fullText().isEmpty() ?
                      i18n( "Create New Bookmark Folder" ) :
                      i18n( "Create New Bookmark Folder in %1" )
                      .arg( parentGroup().text() );
        txt = KInputDialog::getText( caption, i18n( "New folder:" ),
                      TQString::null, &ok );
        if ( !ok )
            return KBookmarkGroup();
    }

    Q_ASSERT(!element.isNull());
    TQDomDocument doc = element.ownerDocument();
    TQDomElement groupElem = doc.createElement( "folder" );
    element.appendChild( groupElem );
    TQDomElement textElem = doc.createElement( "title" );
    groupElem.appendChild( textElem );
    textElem.appendChild( doc.createTextNode( txt ) );

    KBookmarkGroup grp(groupElem);

    if (emitSignal) 
        emit mgr->notifier().createdNewFolder(
                                mgr->path(), grp.fullText(), 
                                grp.address() );

    return grp;

}

KBookmark KBookmarkGroup::createNewSeparator()
{
    Q_ASSERT(!element.isNull());
    TQDomDocument doc = element.ownerDocument();
    Q_ASSERT(!doc.isNull());
    TQDomElement sepElem = doc.createElement( "separator" );
    element.appendChild( sepElem );
    return KBookmark(sepElem);
}

bool KBookmarkGroup::moveItem( const KBookmark & item, const KBookmark & after )
{
    TQDomNode n;
    if ( !after.isNull() )
        n = element.insertAfter( item.element, after.element );
    else // first child
    {
        if ( element.firstChild().isNull() ) // Empty element -> set as real first child
            n = element.insertBefore( item.element, TQDomElement() );

        // we have to skip everything up to the first valid child
        TQDomElement firstChild = nextKnownTag(element.firstChild().toElement(), true);
        if ( !firstChild.isNull() )
            n = element.insertBefore( item.element, firstChild );
        else
        {
            // No real first child -> append after the <title> etc.
            n = element.appendChild( item.element );
        }
    }
    return (!n.isNull());
}

KBookmark KBookmarkGroup::addBookmark( KBookmarkManager* mgr, const KBookmark &bm, bool emitSignal )
{
    element.appendChild( bm.internalElement() );

    if (emitSignal) {
        if ( bm.hasMetaData() ) {
            mgr->notifyCompleteChange( "" );
        } else {
            emit mgr->notifier().addedBookmark(
                                     mgr->path(), bm.url().url(),
                                     bm.fullText(), bm.address(), bm.icon() );
        }
    }

    return bm;
}

KBookmark KBookmarkGroup::addBookmark( KBookmarkManager* mgr, const TQString & text, const KURL & url, const TQString & icon, bool emitSignal )
{
    //kdDebug(7043) << "KBookmarkGroup::addBookmark " << text << " into " << m_address << endl;
    TQDomDocument doc = element.ownerDocument();
    TQDomElement elem = doc.createElement( "bookmark" );
    elem.setAttribute( "href", url.url( 0, 106 ) ); // write utf8 URL (106 is mib enum for utf8)
    TQString _icon = icon;
    if ( _icon.isEmpty() )
        _icon = KMimeType::iconForURL( url );
    elem.setAttribute( "icon", _icon );

    TQDomElement textElem = doc.createElement( "title" );
    elem.appendChild( textElem );
    textElem.appendChild( doc.createTextNode( text ) );

    return addBookmark( mgr, KBookmark( elem ), emitSignal );
}

void KBookmarkGroup::deleteBookmark( KBookmark bk )
{
    element.removeChild( bk.element );
}

bool KBookmarkGroup::isToolbarGroup() const
{
    return ( element.attribute("toolbar") == "yes" );
}

TQDomElement KBookmarkGroup::findToolbar() const
{
    if ( element.attribute("toolbar") == "yes" )
        return element;
    for (TQDomNode n = element.firstChild(); !n.isNull() ; n = n.nextSibling() )
    {
        TQDomElement e = n.toElement();
        // Search among the "folder" children only
        if ( e.tagName() == "folder" )
        {
            if ( e.attribute("toolbar") == "yes" )
                return e;
            else
            {
                TQDomElement result = KBookmarkGroup(e).findToolbar();
                if (!result.isNull())
                    return result;
            }
        }
    }
    return TQDomElement();
}

TQValueList<KURL> KBookmarkGroup::groupUrlList() const
{
    TQValueList<KURL> urlList;
    for ( KBookmark bm = first(); !bm.isNull(); bm = next(bm) )
    {
        if ( bm.isSeparator() || bm.isGroup() )
           continue;
        urlList << bm.url();
    }
    return urlList;
}

//////

bool KBookmark::isGroup() const
{
    TQString tag = element.tagName();
    return ( tag == "folder"
             || tag == "xbel" ); // don't forget the toplevel group
}

bool KBookmark::isSeparator() const
{
    return (element.tagName() == "separator");
}

bool KBookmark::hasParent() const
{
    TQDomElement parent = element.parentNode().toElement();
    return !parent.isNull();
}

TQString KBookmark::text() const
{
    return KStringHandler::csqueeze( fullText() );
}

TQString KBookmark::fullText() const
{
    if (isSeparator())
        return i18n("--- separator ---");

    return element.namedItem("title").toElement().text();
}

KURL KBookmark::url() const
{
    return KURL(element.attribute("href"), 106); // Decode it from utf8 (106 is mib enum for utf8)
}

TQString KBookmark::icon() const
{
    TQString icon = element.attribute("icon");
    if ( icon.isEmpty() ) {
        // Default icon depends on URL for bookmarks, and is default directory
        // icon for groups.
        if ( isGroup() ) {
            icon = "bookmark_folder";
	}
        else {
            if ( isSeparator() ) {
                icon = "eraser"; // whatever
	    }
            else {
                icon = KMimeType::iconForURL( url() );
	    }
	}
    }
    return icon;
}

KBookmarkGroup KBookmark::parentGroup() const
{
    return KBookmarkGroup( element.parentNode().toElement() );
}

KBookmarkGroup KBookmark::toGroup() const
{
    Q_ASSERT( isGroup() );
    return KBookmarkGroup(element);
}

TQString KBookmark::address() const
{
    if ( element.tagName() == "xbel" )
        return ""; // not TQString::null !
    else
    {
        // Use keditbookmarks's DEBUG_ADDRESSES flag to debug this code :)
        if (!hasParent())
        {
            Q_ASSERT(hasParent());
            return "ERROR"; // Avoid an infinite loop
        }
        KBookmarkGroup group = parentGroup();
        TQString parentAddress = group.address();
        uint counter = 0;
        // Implementation note: we don't use QDomNode's childNode list because we
        // would have to skip "TEXT", which KBookmarkGroup already does for us.
        for ( KBookmark bk = group.first() ; !bk.isNull() ; bk = group.next(bk), ++counter )
        {
            if ( bk.element == element )
                return parentAddress + "/" + TQString::number(counter);
        }
        kdWarning() << "KBookmark::address : this can't happen!  " << parentAddress << endl;
        return "ERROR";
    }
}

KBookmark KBookmark::standaloneBookmark( const TQString & text, const KURL & url, const TQString & icon )
{
    TQDomDocument doc("xbel");
    TQDomElement elem = doc.createElement("xbel");
    doc.appendChild( elem );
    KBookmarkGroup grp( elem );
    grp.addBookmark( 0L, text, url, icon, false );
    return grp.first();
}

// For some strange reason TQString("").left(0) returns TQString::null;
// That breaks commonParent()
TQString KBookmark::left(const TQString & str, uint len)
{
    //kdDebug()<<"********"<<TQString("").left(0).isNull()<<endl;
    if(len == 0)
        return TQString("");
    else
        return str.left(len);
}

TQString KBookmark::commonParent(TQString A, TQString B)
{
    TQString error("ERROR");
    if(A == error || B == error)
        return error;

    A += "/";
    B += "/";

    uint lastCommonSlash = 0;
    uint lastPos = A.length() < B.length() ? A.length() : B.length();
    for(uint i=0; i < lastPos; ++i)
    {
        if(A[i] != B[i])
            return left(A, lastCommonSlash);
        if(A[i] == '/')
            lastCommonSlash = i;
    }
    return left(A, lastCommonSlash);
}

static TQDomNode cd_or_create(TQDomNode node, TQString name)
{
    TQDomNode subnode = node.namedItem(name);
    if (subnode.isNull()) 
    {
        subnode = node.ownerDocument().createElement(name);
        node.appendChild(subnode);
    }
    return subnode;
}

static TQDomText get_or_create_text(TQDomNode node)
{
    TQDomNode subnode = node.firstChild();
    if (subnode.isNull()) 
    {
        subnode = node.ownerDocument().createTextNode("");
        node.appendChild(subnode);
    }
    return subnode.toText();
}

// Look for a metadata with owner="http://www.kde.org" or without any owner (for compatibility)
static TQDomNode findOrCreateMetadata( TQDomNode& parent )
{
    static const char kdeOwner[] = "http://www.kde.org";
    TQDomElement metadataElement;
    for ( TQDomNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) {
        TQDomElement elem = _node.toElement();
        if ( !elem.isNull() && elem.tagName() == "metadata" ) {
            const TQString owner = elem.attribute( "owner" );
            if ( owner == kdeOwner )
                return elem;
            if ( owner.isEmpty() )
                metadataElement = elem;
        }
    }
    if ( metadataElement.isNull() ) {
        metadataElement = parent.ownerDocument().createElement( "metadata" );
        parent.appendChild(metadataElement);
    }
    metadataElement.setAttribute( "owner", kdeOwner );
    return metadataElement;
}

bool KBookmark::hasMetaData() const
{
    // ### NOTE: this code creates <info> and <metadata>, despite its name and the const.
    // It doesn't matter much in practice since it's only called for newly-created bookmarks,
    // which will get metadata soon after anyway.
    TQDomNode n = cd_or_create( internalElement(), "info" );
    return findOrCreateMetadata( n ).hasChildNodes();
}

void KBookmark::updateAccessMetadata()
{
    kdDebug(7043) << "KBookmark::updateAccessMetadata " << address() << " " << url().prettyURL() << endl;

    const uint timet = TQDateTime::currentDateTime().toTime_t();
    setMetaDataItem( "time_added", TQString::number( timet ), DontOverwriteMetaData );
    setMetaDataItem( "time_visited", TQString::number( timet ) );

    TQString countStr = metaDataItem( "visit_count" ); // TODO use spec'ed name
    bool ok;
    int currentCount = countStr.toInt(&ok);
    if (!ok)
        currentCount = 0;
    currentCount++;
    setMetaDataItem( "visit_count", TQString::number( currentCount ) );

    // TODO - for 4.0 - time_modified
}

TQString KBookmark::metaDataItem( const TQString &key ) const
{
    TQDomNode infoNode = cd_or_create( internalElement(), "info" );
    infoNode = findOrCreateMetadata( infoNode );
    for ( TQDomNode n = infoNode.firstChild(); !n.isNull(); n = n.nextSibling() ) {
        if ( !n.isElement() ) {
            continue;
        }
        const TQDomElement e = n.toElement();
        if ( e.tagName() == key ) {
            return e.text();
        }
    }
    return TQString::null;
}

void KBookmark::setMetaDataItem( const TQString &key, const TQString &value, MetaDataOverwriteMode mode )
{
    TQDomNode infoNode = cd_or_create( internalElement(), "info" );
    infoNode = findOrCreateMetadata( infoNode );

    TQDomNode item = cd_or_create( infoNode, key );
    TQDomText text = get_or_create_text( item );
    if ( mode == DontOverwriteMetaData && !text.data().isEmpty() ) {
        return;
    }

    text.setData( value );
}

void KBookmarkGroupTraverser::traverse(const KBookmarkGroup &root)
{
    // non-recursive bookmark iterator
    TQValueStack<KBookmarkGroup> stack;
    stack.push(root);
    KBookmark bk = stack.top().first();
    for (;;) {
        if (bk.isNull())
        {
            if (stack.isEmpty()) 
                return;
            if (stack.count() > 1)
                visitLeave(stack.top());
            bk = stack.pop();
            bk = stack.top().next(bk);
            if (bk.isNull())
                continue;
        } 

        if (bk.isGroup()) 
        {
            KBookmarkGroup gp = bk.toGroup();
            visitEnter(gp);
            if (!gp.first().isNull()) 
            {
                stack.push(gp);
                bk = gp.first();
                continue;
            }
            // empty group
            visitLeave(gp);
        } 
        else 
            visit(bk);

        bk = stack.top().next(bk);
    }

    // never reached
}

