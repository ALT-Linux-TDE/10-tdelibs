/* This file is part of the KDE libraries
    Copyright (C) 2000 Carsten Pfeiffer <pfeiffer@kde.org>

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

#ifndef KURLCOMBOBOX_H
#define KURLCOMBOBOX_H

#include <tqevent.h>
#include <tqptrlist.h>
#include <tqmap.h>
#include <tqpixmap.h>
#include <tqstringlist.h>

#include <kcombobox.h>
#include <kurl.h>

/**
 * This combobox shows a number of recent URLs/directories, as well as some
 * default directories.
 * It will manage the default dirs root-directory, home-directory and
 * Desktop-directory, as well as a number of URLs set via setURLs()
 * and one additional entry to be set via setURL().
 *
 * @short A combo box showing a number of recent URLs/directories
 * @author Carsten Pfeiffer <pfeiffer@kde.org>
 */
class TDEIO_EXPORT KURLComboBox : public KComboBox
{
    TQ_OBJECT
    TQ_PROPERTY(TQStringList urls READ urls WRITE setURLs DESIGNABLE true)
    TQ_PROPERTY(int maxItems READ maxItems WRITE setMaxItems DESIGNABLE true)

public:
    /**
     * This enum describes which kind of items is shown in the combo box.
     */
    enum Mode { Files = -1, Directories = 1, Both = 0 };
    /**
     * This Enumeration is used in setURL() to determine which items
     * will be removed when the given list is larger than maxItems().
     *
     * @li RemoveTop means that items will be removed from top
     * @li RemoveBottom means, that items will be removed from the bottom
     */
    enum OverLoadResolving { RemoveTop, RemoveBottom };

    /**
     * Constructs a KURLComboBox.
     * @param mode is either Files, Directories or Both and controls the
     * following behavior:
     * @li Files  all inserted URLs will be treated as files, therefore the
     *            url shown in the combo will never show a trailing /
     *            the icon will be the one associated with the file's mimetype.
     * @li Directories  all inserted URLs will be treated as directories, will
     *                  have a trailing slash in the combobox. The current
     *                  directory will show the "open folder" icon, other
     *                  directories the "folder" icon.
     * @li Both  Don't mess with anything, just show the url as given.
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     */
    KURLComboBox( Mode mode, TQWidget *parent=0, const char *name=0 );
    KURLComboBox( Mode mode, bool rw, TQWidget *parent=0, const char *name=0 );
    /**
     * Destructs the combo box.
     */
    ~KURLComboBox();

    /**
     * Sets the current url. This combo handles exactly one url additionally
     * to the default items and those set via setURLs(). So you can call
     * setURL() as often as you want, it will always replace the previous one
     * set via setURL().
     * If @p url is already in the combo, the last item will stay there
     * and the existing item becomes the current item.
     * The current item will always have the open-directory-pixmap as icon.
     *
     * Note that you won't receive any signals, e.g. textChanged(),
     * returnPressed() or activated() upon calling this method.
     */
    void setURL( const KURL& url );

    /**
     * Inserts @p urls into the combobox below the "default urls" (see
     * addDefaultURL).
     *
     * If the list of urls contains more items than maxItems, the first items
     * will be stripped.
     */
    void setURLs( TQStringList urls );

    /**
     * Inserts @p urls into the combobox below the "default urls" (see
     * addDefaultURL).
     *
     * If the list of urls contains more items than maxItems, the @p remove
     * parameter determines whether the first or last items will be stripped.
     */
    void setURLs( TQStringList urls, OverLoadResolving remove );

    /**
     * @returns a list of all urls currently handled. The list contains at most
     * maxItems() items.
     * Use this to save the list of urls in a config-file and reinsert them
     * via setURLs() next time.
     * Note that all default urls set via addDefaultURL() are not
     * returned, they will automatically be set via setURLs() or setURL().
     * You will always get fully qualified urls, i.e. with protocol like
     * file:/
     */
    TQStringList urls() const;

    /**
     * Sets how many items should be handled and displayed by the combobox.
     * @see maxItems
     */
    void setMaxItems( int );

    /**
     * @returns the maximum of items the combobox handles.
     * @see setMaxItems
     */
    int maxItems() const { return myMaximum; }

    /**
     * Adds a url that will always be shown in the combobox, it can't be
     * "rotated away". Default urls won't be returned in urls() and don't
     * have to be set via setURLs().
     * If you want to specify a special pixmap, use the overloaded method with
     * the pixmap parameter.
     * Default URLs will be inserted into the combobox by setDefaults()
     */
    void addDefaultURL( const KURL& url, const TQString& text = TQString::null );

    /**
     * Adds a url that will always be shown in the combobox, it can't be
     * "rotated away". Default urls won't be returned in urls() and don't
     * have to be set via setURLs().
     * If you don't need to specify a pixmap, use the overloaded method without
     * the pixmap parameter.
     * Default URLs will be inserted into the combobox by setDefaults()
     */
    void addDefaultURL( const KURL& url, const TQPixmap& pix,
			const TQString& text = TQString::null );

    /**
     * Clears all items and inserts the default urls into the combo. Will be
     * called implicitly upon the first call to setURLs() or setURL()
     * @see addDefaultURL
     */
    void setDefaults();

    /**
     * Removes any occurrence of @p url. If @p checkDefaultURLs is false
     * default-urls won't be removed.
     */
    void removeURL( const KURL& url, bool checkDefaultURLs = true );

signals:
    /**
     * Emitted when an item was clicked at.
     * @param url is the url of the now current item. If it is a local url,
     * it won't have a protocol (file:/), otherwise it will.
     */
    void urlActivated( const KURL& url );


protected slots:
    void slotActivated( int );


protected:
    struct _KURLComboItem {
	TQString text;
	KURL url;
	TQPixmap pixmap;
    };
    typedef _KURLComboItem KURLComboItem;
    TQPtrList<KURLComboItem> itemList;
    TQPtrList<KURLComboItem> defaultList;
    TQMap<int,const KURLComboItem*> itemMapper;

    void init( Mode mode );
    void insertURLItem( const KURLComboItem * );

    /**
     * Uses KMimeType::pixmapForURL() to return a proper pixmap for @p url.
     * In directory mode, a folder icon is always returned.
     */
    TQPixmap getPixmap( const KURL& url ) const;

    /**
     * Updates @p item with @p pixmap and sets the url instead of the text
     * of the KURLComboItem.
     * Also works around a Qt bug.
     */
    void updateItem( const KURLComboItem *item, int index, const TQPixmap& pix);

    TQPixmap opendirPix;
    int firstItemIndex;


private:
    bool urlAdded;
    int myMaximum;
    Mode myMode; // can be used as parameter to KUR::path( int ) or url( int )
                 // to specify if we want a trailing slash or not

private:
    class KURLComboBoxPrivate;
    KURLComboBoxPrivate *d;
};


#endif // KURLCOMBOBOX_H
