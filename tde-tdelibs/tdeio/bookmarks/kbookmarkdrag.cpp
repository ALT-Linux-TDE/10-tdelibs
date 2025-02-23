/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "kbookmarkdrag.h"
#include <kurldrag.h>
#include <kdebug.h>

KBookmarkDrag * KBookmarkDrag::newDrag( const TQValueList<KBookmark> & bookmarks, TQWidget * dragSource, const char * name )
{
    KURL::List urls;

    for ( TQValueListConstIterator<KBookmark> it = bookmarks.begin(); it != bookmarks.end(); ++it ) {
       urls.append( (*it).url() );
    }

    // See KURLDrag::newDrag
    TQStrList uris;
    KURL::List::ConstIterator uit = urls.begin();
    KURL::List::ConstIterator uEnd = urls.end();
    // Get each URL encoded in utf8 - and since we get it in escaped
    // form on top of that, .latin1() is fine.
    for ( ; uit != uEnd ; ++uit )
        uris.append( KURLDrag::urlToString(*uit).latin1() );

    return new KBookmarkDrag( bookmarks, uris, dragSource, name );
}

KBookmarkDrag * KBookmarkDrag::newDrag( const KBookmark & bookmark, TQWidget * dragSource, const char * name )
{
    TQValueList<KBookmark> bookmarks;
    bookmarks.append( KBookmark(bookmark) );
    return newDrag(bookmarks, dragSource, name);
}

KBookmarkDrag::KBookmarkDrag( const TQValueList<KBookmark> & bookmarks, const TQStrList & urls,
                  TQWidget * dragSource, const char * name )
    : TQUriDrag( urls, dragSource, name ), m_bookmarks( bookmarks ), m_doc("xbel")
{
    // We need to create the XML for this drag right now and not
    // in encodedData because when cutting a folder, the children
    // wouldn't be part of the bookmarks anymore, when encodedData
    // is requested.
    TQDomElement elem = m_doc.createElement("xbel");
    m_doc.appendChild( elem );
    for ( TQValueListConstIterator<KBookmark> it = bookmarks.begin(); it != bookmarks.end(); ++it ) {
       elem.appendChild( (*it).internalElement().cloneNode( true /* deep */ ) );
    }
    //kdDebug(7043) << "KBookmarkDrag::KBookmarkDrag " << m_doc.toString() << endl;
}

const char* KBookmarkDrag::format( int i ) const
{
    if ( i == 0 )
        return "application/x-xbel";
    else if ( i == 1 )
	return "text/uri-list";
    else if ( i == 2 )
	return "text/plain";
    else return 0;
}

TQByteArray KBookmarkDrag::encodedData( const char* mime ) const
{
    TQByteArray a;
    TQCString mimetype( mime );
    if ( mimetype == "text/uri-list" )
        return TQUriDrag::encodedData( mime );
    else if ( mimetype == "application/x-xbel" )
    {
        a = m_doc.toCString();
        //kdDebug(7043) << "KBookmarkDrag::encodedData " << m_doc.toCString() << endl;
    }
    else if ( mimetype == "text/plain" )
    {
        KURL::List m_lstDragURLs;
        if ( KURLDrag::decode( this, m_lstDragURLs ) )
        {
            TQStringList uris;
            KURL::List::ConstIterator uit = m_lstDragURLs.begin();
            KURL::List::ConstIterator uEnd = m_lstDragURLs.end();
            for ( ; uit != uEnd ; ++uit )
                uris.append( (*uit).prettyURL() );

            TQCString s = uris.join( "\n" ).local8Bit();
            a.resize( s.length() + 1 ); // trailing zero
            memcpy( a.data(), s.data(), s.length() + 1 );
        }
    }
    return a;
}

bool KBookmarkDrag::canDecode( const TQMimeSource * e )
{
    return e->provides("text/uri-list") || e->provides("application/x-xbel") ||
           e->provides("text/plain");
}

TQValueList<KBookmark> KBookmarkDrag::decode( const TQMimeSource * e )
{
    TQValueList<KBookmark> bookmarks;
    if ( e->provides("application/x-xbel") )
    {
        TQByteArray s( e->encodedData("application/x-xbel") );
        //kdDebug(7043) << "KBookmarkDrag::decode s=" << TQCString(s) << endl;
        TQDomDocument doc;
        doc.setContent( s );
        TQDomElement elem = doc.documentElement();
        TQDomNodeList children = elem.childNodes();
        for ( uint childno = 0; childno < children.count(); childno++) 
        {
           bookmarks.append( KBookmark( children.item(childno).cloneNode(true).toElement() ));
        }
        return bookmarks;
    }
    if ( e->provides("text/uri-list") )
    {
        KURL::List m_lstDragURLs;
        //kdDebug(7043) << "KBookmarkDrag::decode uri-list" << endl;
        if ( KURLDrag::decode( e, m_lstDragURLs ) )
        {
            KURL::List::ConstIterator uit = m_lstDragURLs.begin();
            KURL::List::ConstIterator uEnd = m_lstDragURLs.end();
            for ( ; uit != uEnd ; ++uit )
            {
                //kdDebug(7043) << "KBookmarkDrag::decode url=" << (*uit).url() << endl;
                bookmarks.append( KBookmark::standaloneBookmark( 
                                        (*uit).prettyURL(), (*uit) ));
            }
            return bookmarks;
        }
    }
    if( e->provides("text/plain") )
    {        
        //kdDebug(7043) << "KBookmarkDrag::decode text/plain" << endl;
        TQString s;
        if(TQTextDrag::decode( e, s ))
        {
            
            TQStringList listDragURLs = TQStringList::split(TQChar('\n'), s);
            TQStringList::ConstIterator it = listDragURLs.begin();
            TQStringList::ConstIterator end = listDragURLs.end();
            for( ; it!=end; ++it)
            {
                //kdDebug(7043)<<"KBookmarkDrag::decode string"<<(*it)<<endl;
                bookmarks.append( KBookmark::standaloneBookmark( KURL(*it).prettyURL(), KURL(*it)));
            }
            return bookmarks;
        }
    }
    bookmarks.append( KBookmark() );
    return bookmarks;
}
