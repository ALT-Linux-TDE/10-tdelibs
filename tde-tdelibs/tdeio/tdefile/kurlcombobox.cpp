/* This file is part of the KDE libraries
    Copyright (C) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2, as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqdir.h>
#include <tqlistbox.h>

#include <kdebug.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kmimetype.h>

#include <kurlcombobox.h>

class KURLComboBox::KURLComboBoxPrivate
{
public:
    KURLComboBoxPrivate() {
	dirpix = SmallIcon(TQString::fromLatin1("folder"));
    }

    TQPixmap dirpix;
};


KURLComboBox::KURLComboBox( Mode mode, TQWidget *parent, const char *name )
    : KComboBox( parent, name )
{
    init( mode );
}


KURLComboBox::KURLComboBox( Mode mode, bool rw, TQWidget *parent,
                            const char *name )
    : KComboBox( rw, parent, name )
{
    init( mode );
}


KURLComboBox::~KURLComboBox()
{
    delete d;
}


void KURLComboBox::init( Mode mode )
{
    d = new KURLComboBoxPrivate();

    myMode    = mode;
    urlAdded  = false;
    myMaximum = 10; // default
    itemList.setAutoDelete( true );
    defaultList.setAutoDelete( true );
    setInsertionPolicy( NoInsertion );
    setTrapReturnKey( true );
    setSizePolicy( TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed ));

    opendirPix = SmallIcon(TQString::fromLatin1("folder_open"));

    connect( this, TQ_SIGNAL( activated( int )), TQ_SLOT( slotActivated( int )));
}


TQStringList KURLComboBox::urls() const
{
    kdDebug(250) << "::urls()" << endl;
    //static const TQString &fileProt = TDEGlobal::staticQString("file:");
    TQStringList list;
    TQString url;
    for ( int i = defaultList.count(); i < count(); i++ ) {
        url = text( i );
        if ( !url.isEmpty() ) {
            //if ( url.at(0) == '/' )
            //    list.append( url.prepend( fileProt ) );
            //else
                list.append( url );
        }
    }

    return list;
}


void KURLComboBox::addDefaultURL( const KURL& url, const TQString& text )
{
    addDefaultURL( url, getPixmap( url ), text );
}


void KURLComboBox::addDefaultURL( const KURL& url, const TQPixmap& pix,
                                  const TQString& text )
{
    KURLComboItem *item = new KURLComboItem;
    item->url = url;
    item->pixmap = pix;
    if ( text.isEmpty() )
        if ( url.isLocalFile() )
          item->text = url.path( myMode );
        else
          item->text = url.prettyURL( myMode );
    else
        item->text = text;

    defaultList.append( item );
}


void KURLComboBox::setDefaults()
{
    clear();
    itemMapper.clear();

    KURLComboItem *item;
    for ( unsigned int id = 0; id < defaultList.count(); id++ ) {
        item = defaultList.at( id );
        insertURLItem( item );
    }
}

void KURLComboBox::setURLs( TQStringList urls )
{
    setURLs( urls, RemoveBottom );
}

void KURLComboBox::setURLs( TQStringList urls, OverLoadResolving remove )
{
    setDefaults();
    itemList.clear();

    if ( urls.isEmpty() )
        return;

    TQStringList::Iterator it = urls.begin();

    // kill duplicates
    TQString text;
    while ( it != urls.end() ) {
        while ( urls.contains( *it ) > 1 ) {
            it = urls.remove( it );
            continue;
        }
        ++it;
    }

    // limit to myMaximum items
    /* Note: overload is an (old) C++ keyword, some compilers (KCC) choke
       on that, so call it Overload (capital 'O').  (matz) */
    int Overload = urls.count() - myMaximum + defaultList.count();
    while ( Overload > 0 ) {
        urls.remove((remove == RemoveBottom) ? urls.fromLast() : urls.begin());
        Overload--;
    }

    it = urls.begin();

    KURLComboItem *item = 0L;
    KURL u;

    while ( it != urls.end() ) {
        if ( (*it).isEmpty() ) {
            ++it;
            continue;
        }
        u = KURL::fromPathOrURL( *it );

        // Don't restore if file doesn't exist anymore
        if (u.isLocalFile() && !TQFile::exists(u.path())) {
            ++it; 
            continue;
        }

        item = new KURLComboItem;
        item->url = u;
        item->pixmap = getPixmap( u );

        if ( u.isLocalFile() )
            item->text = u.path( myMode ); // don't show file:/
        else
            item->text = *it;

        insertURLItem( item );
        itemList.append( item );
        ++it;
    }
}


void KURLComboBox::setURL( const KURL& url )
{
    if ( url.isEmpty() )
        return;

    blockSignals( true );

    // check for duplicates
    TQMap<int,const KURLComboItem*>::ConstIterator mit = itemMapper.begin();
    TQString urlToInsert = url.url(-1);
    while ( mit != itemMapper.end() ) {
        if ( urlToInsert == mit.data()->url.url(-1) ) {
            setCurrentItem( mit.key() );

            if ( myMode == Directories )
                updateItem( mit.data(), mit.key(), opendirPix );

            blockSignals( false );
            return;
        }
        ++mit;
    }

    // not in the combo yet -> create a new item and insert it

    // first remove the old item
    if ( urlAdded ) {
        itemList.removeLast();
        urlAdded = false;
    }

    setDefaults();

    TQPtrListIterator<KURLComboItem> it( itemList );
    for( ; it.current(); ++it )
        insertURLItem( it.current() );

    KURLComboItem *item = new KURLComboItem;
    item->url = url;
    item->pixmap = getPixmap( url );
    if ( url.isLocalFile() )
        item->text = url.path( myMode );
    else
        item->text = url.prettyURL( myMode );
     kdDebug(250) << "setURL: text=" << item->text << endl;

    int id = count();
    TQString text = /*isEditable() ? item->url.prettyURL( myMode ) : */ item->text;

    if ( myMode == Directories )
        KComboBox::insertItem( opendirPix, text, id );
    else
        KComboBox::insertItem( item->pixmap, text, id );
    itemMapper.insert( id, item );
    itemList.append( item );

    setCurrentItem( id );
    urlAdded = true;
    blockSignals( false );
}


void KURLComboBox::slotActivated( int index )
{
    const KURLComboItem *item = itemMapper[ index ];

    if ( item ) {
        setURL( item->url );
        emit urlActivated( item->url );
    }
}


void KURLComboBox::insertURLItem( const KURLComboItem *item )
{
// kdDebug(250) << "insertURLItem " << item->text << endl;
    int id = count();
    KComboBox::insertItem( item->pixmap, item->text, id );
    itemMapper.insert( id, item );
}


void KURLComboBox::setMaxItems( int max )
{
    myMaximum = max;

    if ( count() > myMaximum ) {
        int oldCurrent = currentItem();

        setDefaults();

        TQPtrListIterator<KURLComboItem> it( itemList );
        int Overload = itemList.count() - myMaximum + defaultList.count();
        for ( int i = 0; i <= Overload; i++ )
            ++it;

        for( ; it.current(); ++it )
            insertURLItem( it.current() );

        if ( count() > 0 ) { // restore the previous currentItem
            if ( oldCurrent >= count() )
                oldCurrent = count() -1;
            setCurrentItem( oldCurrent );
        }
    }
}


void KURLComboBox::removeURL( const KURL& url, bool checkDefaultURLs )
{
    TQMap<int,const KURLComboItem*>::ConstIterator mit = itemMapper.begin();
    while ( mit != itemMapper.end() ) {
        if ( url.url(-1) == mit.data()->url.url(-1) ) {
            if ( !itemList.remove( mit.data() ) && checkDefaultURLs )
                defaultList.remove( mit.data() );
        }
        ++mit;
    }

    blockSignals( true );
    setDefaults();
    TQPtrListIterator<KURLComboItem> it( itemList );
    while ( it.current() ) {
        insertURLItem( *it );
        ++it;
    }
    blockSignals( false );
}


TQPixmap KURLComboBox::getPixmap( const KURL& url ) const
{
    if ( myMode == Directories )
        return d->dirpix;
    else
        return KMimeType::pixmapForURL( url, 0, TDEIcon::Small );
}


// updates "item" with pixmap "pixmap" and sets the URL instead of text
// works around a Qt bug.
void KURLComboBox::updateItem( const KURLComboItem *item,
                               int index, const TQPixmap& pixmap )
{
    // TQComboBox::changeItem() doesn't honor the pixmap when
    // using an editable combobox, so we just remove and insert
    if ( editable() ) {
	removeItem( index );
	insertItem( pixmap,
		    item->url.isLocalFile() ? item->url.path( myMode ) :
		                              item->url.prettyURL( myMode ),
		    index );
    }
    else
        changeItem( pixmap, item->text, index );
}


#include "kurlcombobox.moc"
