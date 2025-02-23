/* This file is part of the KDE libraries
    Copyright (C) 2001,2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <unistd.h>

#include <tqapplication.h>
#include <tqcheckbox.h>
#include <tqdrawutil.h>
#include <tqfontmetrics.h>
#include <tqlabel.h>
#include <tqgrid.h>
#include <tqpainter.h>
#include <tqpopupmenu.h>
#include <tqstyle.h>
#include <tqvbox.h>
#include <tqwhatsthis.h>

#include <tdeaboutdata.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kicondialog.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <kmimetype.h>
#include <kprotocolinfo.h>
#include <kstringhandler.h>
#include <kurldrag.h>
#include <kurlrequester.h>
#include <tdeio/global.h>
#include <tdeio/netaccess.h>

#include "kurlbar.h"

/**
 * Handles tooltips in the KURLBar
 * @internal
 */
class KURLBarToolTip : public TQToolTip
{
public:
    KURLBarToolTip( TQListBox *view ) : TQToolTip( view ), m_view( view ) {}

protected:
    virtual void maybeTip( const TQPoint& point ) {
        TQListBoxItem *item = m_view->itemAt( point );
        if ( item ) {
            TQString text = static_cast<KURLBarItem*>( item )->toolTip();
            if ( !text.isEmpty() )
                tip( m_view->itemRect( item ), text );
        }
    }

private:
    TQListBox *m_view;
};


///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class KURLBarItem::KURLBarItemPrivate
{
public:
    KURLBarItemPrivate()
    {
        isPersistent = true;
    }

    bool isPersistent;
};

KURLBarItem::KURLBarItem( KURLBar *parent,
                          const KURL& url, bool persistent, const TQString& description,
                          const TQString& icon, TDEIcon::Group group )
    : TQListBoxPixmap( TDEIconLoader::unknown() /*, parent->listBox()*/ ),
      m_url( url ),
      m_pixmap( 0L ),
      m_parent( parent ),
      m_appLocal( false )
{
    init( icon, group, description, persistent );
}

KURLBarItem::KURLBarItem( KURLBar *parent,
                          const KURL& url, const TQString& description,
                          const TQString& icon, TDEIcon::Group group )
    : TQListBoxPixmap( TDEIconLoader::unknown() /*, parent->listBox()*/ ),
      m_url( url ),
      m_pixmap( 0L ),
      m_parent( parent ),
      m_appLocal( false )
{
    init( icon, group, description, true /*persistent*/ );
}

void KURLBarItem::init( const TQString& icon, TDEIcon::Group group,
                        const TQString& description, bool persistent )
{
    d = new KURLBarItemPrivate;
    d->isPersistent = persistent;

    setCustomHighlighting( true );
    setIcon( icon, group );
    setDescription( description );
}

KURLBarItem::~KURLBarItem()
{
    delete d;
}

void KURLBarItem::setURL( const KURL& url )
{
    m_url = url;
    if ( m_description.isEmpty() )
        setText( url.fileName() );
}

void KURLBarItem::setIcon( const TQString& icon, TDEIcon::Group group )
{
    m_icon  = icon;
    m_group = group;

    if ( icon.isEmpty() )
        m_pixmap = KMimeType::pixmapForURL( m_url, 0, group, iconSize() );
    else
        m_pixmap = TDEGlobal::iconLoader()->loadIcon( icon, group, iconSize(),
                                                    TDEIcon::DefaultState );
}

void KURLBarItem::setDescription( const TQString& desc )
{
    m_description = desc;
    setText( desc.isEmpty() ? m_url.fileName() : desc );
}

void KURLBarItem::setApplicationLocal( bool local )
{
    if ( !local && !isPersistent() )
    {
        kdWarning() << "KURLBar: dynamic (non-persistent) items can not be global." << endl;
        return;
    }

    m_appLocal = local;
}

void KURLBarItem::setToolTip( const TQString& tip )
{
    m_toolTip = tip;
}

TQString KURLBarItem::toolTip() const
{
    return m_toolTip.isEmpty() ? m_url.prettyURL() : m_toolTip;
}

int KURLBarItem::iconSize() const
{
    return m_parent->iconSize();
}

void KURLBarItem::paint( TQPainter *p )
{
    TQListBox *box = listBox();
    int w = width( box );
    static const int margin = KDialog::spacingHint();

    // draw sunken selection
    if ( isCurrent() || isSelected() ) {
        int h = height( box );

        TQBrush brush = box->colorGroup().brush( TQColorGroup::Highlight );
        p->fillRect( 0, 0, w, h, brush );
        TQPen pen = p->pen();
        TQPen oldPen = pen;
        pen.setColor( box->colorGroup().mid() );
        p->setPen( pen );

        p->drawPoint( 0, 0 );
        p->drawPoint( 0, h - 1 );
        p->drawPoint( w - 1, 0 );
        p->drawPoint( w - 1, h - 1 );

        p->setPen( oldPen );
    }

    if ( m_parent->iconSize() < TDEIcon::SizeMedium ) {
        // small icon -> draw icon next to text

        // ### mostly cut & paste of TQListBoxPixmap::paint() until Qt 3.1
        // (where it will properly use pixmap() instead of the internal pixmap)
        const TQPixmap *pm = pixmap();
        int yPos = TQMAX( 0, (height(box) - pm->height())/2 );

        p->drawPixmap( margin, yPos, *pm );
        if ( !text().isEmpty() ) {
            TQFontMetrics fm = p->fontMetrics();
            if ( pm->height() < fm.height() ) {
                yPos = fm.ascent() + fm.leading()/2;
            }
            else {
                yPos = height(box)/2 - fm.height()/2 + fm.ascent() - margin;
            }

            yPos += margin;
            int stringWidth = box->width() - pm->width() - 2 - (margin * 2);
            TQString visibleText = KStringHandler::rPixelSqueeze( text(), fm, stringWidth );
            int xPos = pm->width() + margin + 2;

            if ( isCurrent() || isSelected() ) {
                p->setPen( box->colorGroup().highlight().dark(115) );
                p->drawText( xPos + ( TQApplication::reverseLayout() ? -1 : 1),
                             yPos + 1, visibleText );
                p->setPen( box->colorGroup().highlightedText() );
            }

            p->drawText( xPos, yPos, visibleText );
        }
        // end cut & paste (modulo pixmap centering)
    }

    else {
        // big icons -> draw text below icon
        int y = margin;
        const TQPixmap *pm = pixmap();

        if ( !pm->isNull() ) {
            int x = (w - pm->width()) / 2;
            x = TQMAX( x, margin );
            p->drawPixmap( x, y, *pm );
        }

        if ( !text().isEmpty() ) {
            TQFontMetrics fm = p->fontMetrics();
            y += pm->height() + fm.height() - fm.descent();

            int stringWidth = box->width() - (margin * 2);
            TQString visibleText = KStringHandler::rPixelSqueeze( text(), fm, stringWidth );
            int x = (w - fm.width( visibleText )) / 2;
            x = TQMAX( x, margin );

            if ( isCurrent() || isSelected() ) {
                p->setPen( box->colorGroup().highlight().dark(115) );
                p->drawText( x + ( TQApplication::reverseLayout() ? -1 : 1),
                             y + 1, visibleText );
                p->setPen( box->colorGroup().highlightedText() );
            }

            p->drawText( x, y, visibleText );
        }
    }
}

TQSize KURLBarItem::sizeHint() const
{
    int wmin = 0;
    int hmin = 0;
    const KURLBarListBox *lb =static_cast<const KURLBarListBox*>(listBox());

    if ( m_parent->iconSize() < TDEIcon::SizeMedium ) {
        wmin = TQListBoxPixmap::width( lb ) + KDialog::spacingHint() * 2;
        hmin = TQListBoxPixmap::height( lb ) + KDialog::spacingHint() * 2;
    }
    else {
        wmin = TQMAX(lb->fontMetrics().width(text()), pixmap()->width()) + KDialog::spacingHint() * 2;
        hmin = lb->fontMetrics().lineSpacing() + pixmap()->height() + KDialog::spacingHint() * 2;
    }

    if ( lb->isVertical() )
        wmin = TQMIN( wmin, lb->viewport()->sizeHint().width() );
    else
        hmin = TQMIN( hmin, lb->viewport()->sizeHint().height() );

    return TQSize( wmin, hmin );
}

int KURLBarItem::width( const TQListBox *lb ) const
{
    if ( static_cast<const KURLBarListBox *>( lb )->isVertical() )
        return TQMAX( sizeHint().width(), lb->viewport()->width() );
    else
        return sizeHint().width();
}

int KURLBarItem::height( const TQListBox *lb ) const
{
    if ( static_cast<const KURLBarListBox *>( lb )->isVertical() )
        return sizeHint().height();
    else
        return TQMAX( sizeHint().height(), lb->viewport()->height() );
}

bool KURLBarItem::isPersistent() const
{
    return d->isPersistent;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////

class KURLBar::KURLBarPrivate
{
public:
    KURLBarPrivate()
    {
        currentURL.setPath( TQDir::homeDirPath() );
        defaultIconSize = 0;
    }

    int defaultIconSize;
    KURL currentURL;
};


KURLBar::KURLBar( bool useGlobalItems, TQWidget *parent, const char *name, WFlags f )
    : TQFrame( parent, name, f ),
      m_activeItem( 0L ),
      m_useGlobal( useGlobalItems ),
      m_isModified( false ),
      m_isImmutable( false ),
      m_listBox( 0L ),
      m_iconSize( TDEIcon::SizeMedium )
{
    d = new KURLBarPrivate();

    setListBox( 0L );
    setSizePolicy( TQSizePolicy( isVertical() ?
                                TQSizePolicy::Maximum :
                                TQSizePolicy::Preferred,
                                isVertical() ?
                                TQSizePolicy::Preferred :
                                TQSizePolicy::Maximum ));
    TQWhatsThis::add(this, i18n("<qt>The <b>Quick Access</b> panel provides easy access to commonly used file locations.<p>"
                               "Clicking on one of the shortcut entries will take you to that location.<p>"
                               "By right clicking on an entry you can add, edit and remove shortcuts.</qt>"));
}

KURLBar::~KURLBar()
{
    delete d;
}

KURLBarItem * KURLBar::insertItem(const KURL& url, const TQString& description,
                                  bool applicationLocal,
                                  const TQString& icon, TDEIcon::Group group )
{
    KURLBarItem *item = new KURLBarItem(this, url, description, icon, group);
    item->setApplicationLocal( applicationLocal );
    m_listBox->insertItem( item );
    return item;
}

KURLBarItem * KURLBar::insertDynamicItem(const KURL& url, const TQString& description,
                                         const TQString& icon, TDEIcon::Group group )
{
    KURLBarItem *item = new KURLBarItem(this, url, false, description, icon, group);
    m_listBox->insertItem( item );
    return item;
}

void KURLBar::setOrientation( TQt::Orientation orient )
{
    m_listBox->setOrientation( orient );
    setSizePolicy( TQSizePolicy( isVertical() ?
                                TQSizePolicy::Maximum :
                                TQSizePolicy::Preferred,
                                isVertical() ?
                                TQSizePolicy::Preferred :
                                TQSizePolicy::Maximum ));
}

TQt::Orientation KURLBar::orientation() const
{
    return m_listBox->orientation();
}

void KURLBar::setListBox( KURLBarListBox *view )
{
    delete m_listBox;

    if ( !view ) {
        m_listBox = new KURLBarListBox( this, "urlbar listbox" );
        setOrientation( TQt::Vertical );
    }
    else {
        m_listBox = view;
        if ( m_listBox->parentWidget() != this )
            m_listBox->reparent( this, TQPoint(0,0) );
        m_listBox->resize( width(), height() );
    }

    m_listBox->setSelectionMode( TDEListBox::Single );
    paletteChange( palette() );
    m_listBox->setFocusPolicy( TQWidget::TabFocus );

    connect( m_listBox, TQ_SIGNAL( mouseButtonClicked( int, TQListBoxItem *, const TQPoint & ) ),
             TQ_SLOT( slotSelected( int, TQListBoxItem * )));
    connect( m_listBox, TQ_SIGNAL( dropped( TQDropEvent * )),
             this, TQ_SLOT( slotDropped( TQDropEvent * )));
    connect( m_listBox, TQ_SIGNAL( contextMenuRequested( TQListBoxItem *,
                                                      const TQPoint& )),
             TQ_SLOT( slotContextMenuRequested( TQListBoxItem *, const TQPoint& )));
    connect( m_listBox, TQ_SIGNAL( returnPressed( TQListBoxItem * ) ),
             TQ_SLOT( slotSelected( TQListBoxItem * ) ));
}

void KURLBar::setIconSize( int size )
{
    if ( size == m_iconSize )
        return;

    m_iconSize = size;

    // reload the icons with the new size
    KURLBarItem *item = static_cast<KURLBarItem*>( m_listBox->firstItem() );
    while ( item ) {
        item->setIcon( item->icon(), item->iconGroup() );
        item = static_cast<KURLBarItem*>( item->next() );
    }

    resize( sizeHint() );
    updateGeometry();
}

void KURLBar::clear()
{
    m_listBox->clear();
}

void KURLBar::resizeEvent( TQResizeEvent *e )
{
    TQFrame::resizeEvent( e );
    m_listBox->resize( width(), height() );
}

void KURLBar::paletteChange( const TQPalette & )
{
    TQPalette pal = palette();
    TQColor gray = pal.color( TQPalette::Normal, TQColorGroup::Background );
    TQColor selectedTextColor = pal.color( TQPalette::Normal, TQColorGroup::BrightText );
    TQColor foreground = pal.color( TQPalette::Normal, TQColorGroup::Foreground );
    pal.setColor( TQPalette::Normal,   TQColorGroup::Base, gray );
    pal.setColor( TQPalette::Normal,   TQColorGroup::HighlightedText, selectedTextColor );
    pal.setColor( TQPalette::Normal,   TQColorGroup::Text, foreground );
    pal.setColor( TQPalette::Inactive, TQColorGroup::Base, gray );
    pal.setColor( TQPalette::Inactive, TQColorGroup::HighlightedText, selectedTextColor );
    pal.setColor( TQPalette::Inactive, TQColorGroup::Text, foreground );

    setPalette( pal );
}

TQSize KURLBar::sizeHint() const
{
    return m_listBox->sizeHint();

#if 0
    // this code causes vertical and or horizontal scrollbars appearing
    // depending on the text, font, moonphase and earth rotation. Just using
    // m_listBox->sizeHint() fixes this (although the widget can then be
    // resized to a smaller size so that scrollbars appear).
    int w = 0;
    int h = 0;
    KURLBarItem *item;
    bool vertical = isVertical();

    for ( item = static_cast<KURLBarItem*>( m_listBox->firstItem() );
          item;
          item = static_cast<KURLBarItem*>( item->next() ) ) {

        TQSize sh = item->sizeHint();

        if ( vertical ) {
            w = TQMAX( w, sh.width() );
            h += sh.height();
        }
        else {
            w += sh.width();
            h = TQMAX( h, sh.height() );
        }
    }

//     if ( vertical && m_listBox->verticalScrollBar()->isVisible() )
//         w += m_listBox->verticalScrollBar()->width();
//     else if ( !vertical && m_listBox->horizontalScrollBar()->isVisible() )
//         h += m_listBox->horizontalScrollBar()->height();

    if ( w == 0 && h == 0 )
        return TQSize( 100, 200 );
    else
        return TQSize( 6 + w, h );
#endif
}

TQSize KURLBar::minimumSizeHint() const
{
    TQSize s = sizeHint(); // ###
    int w = s.width()  + m_listBox->verticalScrollBar()->width();
    int h = s.height() + m_listBox->horizontalScrollBar()->height();
    return TQSize( w, h );
}

void KURLBar::slotSelected( int button, TQListBoxItem *item )
{
    if ( button != TQt::LeftButton )
        return;

    slotSelected( item );
}

void KURLBar::slotSelected( TQListBoxItem *item )
{
    if ( item && item != m_activeItem )
        m_activeItem = static_cast<KURLBarItem*>( item );

    if ( m_activeItem ) {
        m_listBox->setCurrentItem( m_activeItem );
        emit activated( m_activeItem->url() );
    }
}

void KURLBar::setCurrentItem( const KURL& url )
{
    d->currentURL = url;

    TQString u = url.url(-1);

    if ( m_activeItem && m_activeItem->url().url(-1) == u )
        return;

    bool hasURL = false;
    TQListBoxItem *item = m_listBox->firstItem();
    while ( item ) {
        if ( static_cast<KURLBarItem*>( item )->url().url(-1) == u ) {
            m_activeItem = static_cast<KURLBarItem*>( item );
            m_listBox->setCurrentItem( item );
            m_listBox->setSelected( item, true );
            hasURL = true;
            break;
        }
        item = item->next();
    }

    if ( !hasURL ) {
        m_activeItem = 0L;
        m_listBox->clearSelection();
    }
}

KURLBarItem * KURLBar::currentItem() const
{
    TQListBoxItem *item = m_listBox->item( m_listBox->currentItem() );
    if ( item )
        return static_cast<KURLBarItem *>( item );
    return 0L;
}

KURL KURLBar::currentURL() const
{
    KURLBarItem *item = currentItem();
    return item ? item->url() : KURL();
}

void KURLBar::readConfig( TDEConfig *appConfig, const TQString& itemGroup )
{
    m_isImmutable = appConfig->groupIsImmutable( itemGroup );
    TDEConfigGroupSaver cs( appConfig, itemGroup );
    d->defaultIconSize = m_iconSize;
    m_iconSize = appConfig->readNumEntry( "Speedbar IconSize", m_iconSize );

    if ( m_useGlobal ) { // read global items
        TDEConfig *globalConfig = TDEGlobal::config();
        TDEConfigGroupSaver cs( globalConfig, (TQString)(itemGroup +" (Global)"));
        int num = globalConfig->readNumEntry( "Number of Entries" );
        for ( int i = 0; i < num; i++ ) {
            readItem( i, globalConfig, false );
        }
    }

    // read application local items
    int num = appConfig->readNumEntry( "Number of Entries" );
    for ( int i = 0; i < num; i++ ) {
        readItem( i, appConfig, true );
    }
}

void KURLBar::readItem( int i, TDEConfig *config, bool applicationLocal )
{
    TQString number = TQString::number( i );
    KURL url = KURL::fromPathOrURL( config->readPathEntry( TQString("URL_") + number ));
    if ( !url.isValid() || !KProtocolInfo::isKnownProtocol( url ))
        return; // nothing we could do.

    TQString description = config->readEntry( TQString("Description_") + number ); 

    insertItem( url,
                description,
                applicationLocal,
                config->readEntry( TQString("Icon_") + number ),
                static_cast<TDEIcon::Group>(
                    config->readNumEntry( TQString("IconGroup_") + number )) );
}

void KURLBar::writeConfig( TDEConfig *config, const TQString& itemGroup )
{
    TDEConfigGroupSaver cs1( config, itemGroup );
    if(!config->hasDefault("Speedbar IconSize") && m_iconSize == d->defaultIconSize )
        config->revertToDefault("Speedbar IconSize");
    else
        config->writeEntry( "Speedbar IconSize", m_iconSize );

    if ( !m_isModified )
        return;

    int i = 0;
    int numLocal = 0;
    KURLBarItem *item = static_cast<KURLBarItem*>( m_listBox->firstItem() );

    while ( item )
    {
        if ( item->isPersistent() ) // we only save persistent items
        {
            if ( item->applicationLocal() )
            {
                writeItem( item, numLocal, config, false );
                numLocal++;
            }

            i++;
        }
        item = static_cast<KURLBarItem*>( item->next() );
    }
    config->writeEntry("Number of Entries", numLocal);


    // write the global entries to kdeglobals, if any
    bool haveGlobalEntries = (i > numLocal);
    if ( m_useGlobal && haveGlobalEntries ) {
        config->setGroup( itemGroup + " (Global)" );

        int numGlobals = 0;
        item = static_cast<KURLBarItem*>( m_listBox->firstItem() );

        while ( item )
        {
            if ( item->isPersistent() ) // we only save persistent items
            {
                if ( !item->applicationLocal() )
                {
                    writeItem( item, numGlobals, config, true );
                    numGlobals++;
                }
            }

            item = static_cast<KURLBarItem*>( item->next() );
        }
        config->writeEntry("Number of Entries", numGlobals, true, true);
    }

    m_isModified = false;
}

void KURLBar::writeItem( KURLBarItem *item, int i, TDEConfig *config,
                         bool global )
{
    if ( !item->isPersistent() )
        return;

    TQString Description = "Description_";
    TQString URL = "URL_";
    TQString Icon = "Icon_";
    TQString IconGroup = "IconGroup_";

    TQString number = TQString::number( i );
    config->writePathEntry( URL + number, item->url().prettyURL(), true, global );

    config->writeEntry( Description + number, item->description(),true,global);
    config->writeEntry( Icon + number, item->icon(), true, global );
    config->writeEntry( IconGroup + number, item->iconGroup(), true, global );
}


void KURLBar::slotDropped( TQDropEvent *e )
{
    KURL::List urls;
    if ( KURLDrag::decode( e, urls ) ) {
        KURL url;
        TQString description;
        TQString icon;
        bool appLocal = false;

        KURL::List::Iterator it = urls.begin();
        for ( ; it != urls.end(); ++it ) {
            (void) insertItem( *it, description, appLocal, icon );
            m_isModified = true;
            updateGeometry();
        }
    }
}

void KURLBar::slotContextMenuRequested( TQListBoxItem *_item, const TQPoint& pos )
{
    if (m_isImmutable)
        return;

    KURLBarItem *item = dynamic_cast<KURLBarItem*>( _item );

    static const int IconSize   = 10;
    static const int AddItem    = 20;
    static const int EditItem   = 30;
    static const int RemoveItem = 40;

    KURL lastURL = m_activeItem ? m_activeItem->url() : KURL();

    bool smallIcons = m_iconSize < TDEIcon::SizeMedium;
    TQPopupMenu *popup = new TQPopupMenu();
    popup->insertItem( smallIcons ?
                       i18n("&Large Icons") : i18n("&Small Icons"),
                       IconSize );
    popup->insertSeparator();

    if (item != 0L && item->isPersistent())
    {
        popup->insertItem(SmallIconSet("edit"), i18n("&Edit Entry..."), EditItem);
        popup->insertSeparator();
    }

    popup->insertItem(SmallIconSet("document-new"), i18n("&Add Entry..."), AddItem);

    if (item != 0L && item->isPersistent())
    {
        popup->insertItem( SmallIconSet("edit-delete"), i18n("&Remove Entry"),
                          RemoveItem );
    }

    int result = popup->exec( pos );
    switch ( result ) {
        case IconSize:
            setIconSize( smallIcons ? TDEIcon::SizeMedium : TDEIcon::SizeSmallMedium );
            m_listBox->triggerUpdate( true );
            break;
        case AddItem:
            addNewItem();
            break;
        case EditItem:
            editItem( static_cast<KURLBarItem *>( item ) );
            break;
        case RemoveItem:
            delete item;
            m_isModified = true;
            break;
        default: // abort
            break;
    }

    // reset current item
    m_activeItem = 0L;
    setCurrentItem( lastURL );
}

bool KURLBar::addNewItem()
{
    KURLBarItem *item = new KURLBarItem( this, d->currentURL,
                                         i18n("Enter a description") );
    if ( editItem( item ) ) {
        m_listBox->insertItem( item );
        return true;
    }

    delete item;
    return false;
}

bool KURLBar::editItem( KURLBarItem *item )
{
    if ( !item || !item->isPersistent() ) // should never happen tho
        return false;

    KURL url            = item->url();
    TQString description = item->description();
    TQString icon        = item->icon();
    bool appLocal       = item->applicationLocal();

    if ( KURLBarItemDialog::getInformation( m_useGlobal,
                                            url, description,
                                            icon, appLocal,
                                            m_iconSize, this ))
    {
        item->setURL( url );
        item->setDescription( description );
        item->setIcon( icon );
        item->setApplicationLocal( appLocal );
        m_listBox->triggerUpdate( true );
        m_isModified = true;
        updateGeometry();
        return true;
    }

    return false;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


KURLBarListBox::KURLBarListBox( TQWidget *parent, const char *name )
    : TDEListBox( parent, name )
{
    m_toolTip = new KURLBarToolTip( this );
    setAcceptDrops( true );
    viewport()->setAcceptDrops( true );
}

KURLBarListBox::~KURLBarListBox()
{
    delete m_toolTip;
}

void KURLBarListBox::paintEvent( TQPaintEvent* )
{
    TQPainter p(this);
    p.setPen( colorGroup().mid() );
    p.drawRect( 0, 0, width(), height() );
}

TQDragObject * KURLBarListBox::dragObject()
{
    KURL::List urls;
    KURLBarItem *item = static_cast<KURLBarItem*>( firstItem() );

    while ( item ) {
        if ( item->isSelected() )
            urls.append( item->url() );
        item = static_cast<KURLBarItem*>( item->next() );
    }

    if ( !urls.isEmpty() ) // ### use custom drag-object with description etc.?
        return new KURLDrag( urls, this, "urlbar drag" );

    return 0L;
}

void KURLBarListBox::contentsDragEnterEvent( TQDragEnterEvent *e )
{
    e->accept( KURLDrag::canDecode( e ));
}

void KURLBarListBox::contentsDropEvent( TQDropEvent *e )
{
    emit dropped( e );
}

void KURLBarListBox::contextMenuEvent( TQContextMenuEvent *e )
{
    if (e)
    {
        emit contextMenuRequested( itemAt( e->globalPos() ), e->globalPos() );
        e->consume(); // Consume the event to avoid multiple contextMenuEvent calls...
    }
}

void KURLBarListBox::setOrientation( TQt::Orientation orient )
{
    if ( orient == TQt::Vertical ) {
        setColumnMode( 1 );
        setRowMode( Variable );
    }
    else {
        setRowMode( 1 );
        setColumnMode( Variable );
    }

    m_orientation = orient;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////


bool KURLBarItemDialog::getInformation( bool allowGlobal, KURL& url,
                                        TQString& description, TQString& icon,
                                        bool& appLocal, int iconSize,
                                        TQWidget *parent )
{
    KURLBarItemDialog *dialog = new KURLBarItemDialog( allowGlobal, url,
                                                       description, icon,
                                                       appLocal,
                                                       iconSize, parent );
    if ( dialog->exec() == TQDialog::Accepted ) {
        // set the return parameters
        url         = dialog->url();
        description = dialog->description();
        icon        = dialog->icon();
        appLocal    = dialog->applicationLocal();

        delete dialog;
        return true;
    }

    delete dialog;
    return false;
}

KURLBarItemDialog::KURLBarItemDialog( bool allowGlobal, const KURL& url,
                                      const TQString& description,
                                      TQString icon, bool appLocal,
                                      int iconSize,
                                      TQWidget *parent, const char *name )
    : KDialogBase( parent, name, true,
                   i18n("Edit Quick Access Entry"), Ok | Cancel, Ok, true )
{
    TQVBox *box = new TQVBox( this );
    TQString text = i18n("<qt><b>Please provide a description, URL and icon for this Quick Access entry.</b></br></qt>");
    TQLabel *label = new TQLabel( text, box );
    box->setSpacing( spacingHint() );

    TQGrid *grid = new TQGrid( 2, box );
    grid->setSpacing( spacingHint() );

    TQString whatsThisText = i18n("<qt>This is the text that will appear in the Quick Access panel.<p>"
                                 "The description should consist of one or two words "
                                 "that will help you remember what this entry refers to.</qt>");
    label = new TQLabel( i18n("&Description:"), grid );
    m_edit = new KLineEdit( grid, "description edit" );
    m_edit->setText( description.isEmpty() ? url.fileName() : description );
    label->setBuddy( m_edit );
    TQWhatsThis::add( label, whatsThisText );
    TQWhatsThis::add( m_edit, whatsThisText );

    whatsThisText = i18n("<qt>This is the location associated with the entry. Any valid URL may be used. For example:<p>"
                         "%1<br>http://www.trinitydesktop.org<p>"
                         "By clicking on the button next to the text edit box you can browse to an "
                         "appropriate URL.</qt>").arg(TQDir::homeDirPath());
    label = new TQLabel( i18n("&URL:"), grid );
    m_urlEdit = new KURLRequester( url.prettyURL(), grid );
    m_urlEdit->setMode( KFile::Directory );
    label->setBuddy( m_urlEdit );
    TQWhatsThis::add( label, whatsThisText );
    TQWhatsThis::add( m_urlEdit, whatsThisText );

    whatsThisText = i18n("<qt>This is the icon that will appear in the Quick Access panel.<p>"
                         "Click on the button to select a different icon.</qt>");
    label = new TQLabel( i18n("Choose an &icon:"), grid );
    m_iconButton = new TDEIconButton( grid, "icon button" );
    m_iconButton->setIconSize( iconSize );
    if ( icon.isEmpty() )
        icon = KMimeType::iconForURL( url );
    m_iconButton->setIcon( icon );
    label->setBuddy( m_iconButton );
    TQWhatsThis::add( label, whatsThisText );
    TQWhatsThis::add( m_iconButton, whatsThisText );

    if ( allowGlobal ) {
        TQString appName;
        if ( TDEGlobal::instance()->aboutData() )
            appName = TDEGlobal::instance()->aboutData()->programName();
        if ( appName.isEmpty() )
            appName = TQString::fromLatin1( TDEGlobal::instance()->instanceName() );
        m_appLocal = new TQCheckBox( i18n("&Only show when using this application (%1)").arg( appName ), box );
        m_appLocal->setChecked( appLocal );
        TQWhatsThis::add( m_appLocal,
                         i18n("<qt>Select this setting if you want this "
                              "entry to show only when using the current application (%1).<p>"
                              "If this setting is not selected, the entry will be available in all "
                              "applications.</qt>")
                              .arg(appName));
    }
    else
        m_appLocal = 0L;
    connect(m_urlEdit->lineEdit(),TQ_SIGNAL(textChanged ( const TQString & )),this,TQ_SLOT(urlChanged(const TQString & )));
    m_edit->setFocus();
    setMainWidget( box );
}

KURLBarItemDialog::~KURLBarItemDialog()
{
}

void KURLBarItemDialog::urlChanged(const TQString & text )
{
    enableButtonOK( !text.isEmpty() );
}

KURL KURLBarItemDialog::url() const
{
    TQString text = m_urlEdit->url();
    KURL u;
    if ( text.at(0) == '/' )
        u.setPath( text );
    else
        u = text;

    return u;
}

TQString KURLBarItemDialog::description() const
{
    return m_edit->text();
}

TQString KURLBarItemDialog::icon() const
{
    return m_iconButton->icon();
}

bool KURLBarItemDialog::applicationLocal() const
{
    if ( !m_appLocal )
        return true;

    return m_appLocal->isChecked();
}

void KURLBarItem::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KURLBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KURLBarListBox::virtual_hook( int id, void* data )
{ TDEListBox::virtual_hook( id, data ); }


#include "kurlbar.moc"
