/* This file is part of the KDE libraries
    Copyright
    (C) 2000 Reginald Stadlbauer (reggie@kde.org)
    (C) 1997, 1998 Stephan Kulow (coolo@kde.org)
    (C) 1997, 1998 Mark Donohoe (donohoe@kde.org)
    (C) 1997, 1998 Sven Radej (radej@kde.org)
    (C) 1997, 1998 Matthias Ettrich (ettrich@kde.org)
    (C) 1999 Chris Schlaeger (cs@kde.org)
    (C) 1999 Kurt Granroth (granroth@kde.org)

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

#include <config.h>

#ifdef KDE_USE_FINAL
#undef Always
#include <tqdockwindow.h>
#endif

#include <string.h>

#include <tqpainter.h>
#include <tqtooltip.h>
#include <tqdrawutil.h>
#include <tqstring.h>
#include <tqrect.h>
#include <tqobjectlist.h>
#include <tqtimer.h>
#include <tqstyle.h>
#include <tqlayout.h>

#include <tdetoolbar.h>
#include <tdemainwindow.h>
#include <klineedit.h>
#include <kseparator.h>
#include <tdelocale.h>
#include <tdeapplication.h>
#include <tdeaction.h>
#include <kstdaction.h>
#include <tdeglobal.h>
#include <tdeconfig.h>
#include <kiconloader.h>
#include <kcombobox.h>
#include <tdepopupmenu.h>
#include <kanimwidget.h>
#include <kedittoolbar.h>
#include <kipc.h>
#include <twin.h>
#include <kdebug.h>
#include <tdetoolbarbutton.h>

enum {
    CONTEXT_TOP = 0,
    CONTEXT_LEFT = 1,
    CONTEXT_RIGHT = 2,
    CONTEXT_BOTTOM = 3,
    CONTEXT_FLOAT = 4,
    CONTEXT_FLAT = 5,
    CONTEXT_ICONS = 6,
    CONTEXT_TEXT = 7,
    CONTEXT_TEXTRIGHT = 8,
    CONTEXT_TEXTUNDER = 9,
    CONTEXT_ICONSIZES = 50 // starting point for the icon size list, put everything else before
};

class TDEToolBarPrivate
{
public:
    TDEToolBarPrivate() {
        m_iconSize     = 0;
        m_iconText     = TDEToolBar::IconOnly;
        m_highlight    = true;
        m_transparent  = true;
        m_honorStyle   = false;

        m_enableContext  = true;

        m_parent         = NULL;
        m_xmlguiClient   = 0;

        oldPos = TQt::DockUnmanaged;

        modified = m_isHorizontal = positioned = false;

        IconSizeDefault = 0;
        IconTextDefault = "IconOnly";

        NewLineDefault = false;
        OffsetDefault = 0;
        PositionDefault = "Top";
	HiddenDefault = false;
        idleButtons.setAutoDelete(true);
    }

    int m_iconSize;
    TDEToolBar::IconText m_iconText;
    bool m_highlight : 1;
    bool m_transparent : 1;
    bool m_honorStyle : 1;
    bool m_isHorizontal : 1;
    bool m_enableContext : 1;
    bool modified : 1;
    bool positioned : 1;

    TQWidget *m_parent;

    TQMainWindow::ToolBarDock oldPos;

    KXMLGUIClient *m_xmlguiClient;

    struct ToolBarInfo
    {
        ToolBarInfo() : index( -1 ), offset( -1 ), newline( false ), dock( TQt::DockTop ) {}
        ToolBarInfo( TQt::Dock d, int i, bool n, int o ) : index( i ), offset( o ), newline( n ), dock( d ) {}
        int index, offset;
        bool newline;
        TQt::Dock dock;
    };

    ToolBarInfo toolBarInfo;
    TQValueList<int> iconSizes;
    TQTimer repaintTimer;

  // Default Values.
  bool HiddenDefault;
  int IconSizeDefault;
  TQString IconTextDefault;
  bool NewLineDefault;
  int OffsetDefault;
  TQString PositionDefault;

   TQPtrList<TQWidget> idleButtons;
};

TDEToolBarSeparator::TDEToolBarSeparator(Orientation o , bool l, TQToolBar *parent,
                                     const char* name )
    :TQFrame( parent, name ), line( l )
{
    connect( parent, TQ_SIGNAL(orientationChanged(Orientation)),
             this, TQ_SLOT(setOrientation(Orientation)) );
    setOrientation( o );
    setBackgroundMode( parent->backgroundMode() );
    setBackgroundOrigin( ParentOrigin );
}

void TDEToolBarSeparator::setOrientation( Orientation o )
{
    orient = o;
    setFrameStyle( NoFrame );
}

void TDEToolBarSeparator::drawContents( TQPainter* p )
{
    if ( line ) {
        TQStyle::SFlags flags = TQStyle::Style_Default;

        if ( orientation() == TQt::Horizontal )
            flags = flags | TQStyle::Style_Horizontal;

        style().drawPrimitive(TQStyle::PE_DockWindowSeparator, p,
                              contentsRect(), colorGroup(), flags);
    } else {
        TQFrame::drawContents(p);
    }
}

void TDEToolBarSeparator::styleChange( TQStyle& )
{
    setOrientation( orient );
}

TQSize TDEToolBarSeparator::sizeHint() const
{
    int dim = style().pixelMetric( TQStyle::PM_DockWindowSeparatorExtent, this );
    return orientation() == TQt::Vertical ? TQSize( 0, dim ) : TQSize( dim, 0 );
}

TQSizePolicy TDEToolBarSeparator::sizePolicy() const
{
    return TQSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Minimum );
}

TDEToolBar::TDEToolBar( TQWidget *parent, const char *name, bool honorStyle, bool readConfig )
    : TQToolBar( TQString::fromLatin1( name ),
      dynamic_cast<TQMainWindow*>(parent),
      parent, false,
      name ? name : "mainToolBar")
{
    init( readConfig, honorStyle );
}

TDEToolBar::TDEToolBar( TQMainWindow *parentWindow, TQMainWindow::ToolBarDock dock, bool newLine, const char *name, bool honorStyle, bool readConfig )
    : TQToolBar( TQString::fromLatin1( name ),
      parentWindow, dock, newLine,
      name ? name : "mainToolBar")
{
    init( readConfig, honorStyle );
}

TDEToolBar::TDEToolBar( TQMainWindow *parentWindow, TQWidget *dock, bool newLine, const char *name, bool honorStyle, bool readConfig )
    : TQToolBar( TQString::fromLatin1( name ),
      parentWindow, dock, newLine,
      name ? name : "mainToolBar")
{
    init( readConfig, honorStyle );
}

TDEToolBar::~TDEToolBar()
{
    emit toolbarDestroyed();
    delete d;
}

void TDEToolBar::init( bool readConfig, bool honorStyle )
{
    d = new TDEToolBarPrivate();

    setFullSize( true );
    d->m_honorStyle = honorStyle;
    context = 0;
    layoutTimer = new TQTimer( this );
    connect( layoutTimer, TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( rebuildLayout() ) );
    connect( &(d->repaintTimer), TQ_SIGNAL( timeout() ),
             this, TQ_SLOT( slotRepaint() ) );

    if ( kapp ) { // may be null when started inside designer
        connect(kapp, TQ_SIGNAL(toolbarAppearanceChanged(int)), this, TQ_SLOT(slotAppearanceChanged()));
        // request notification of changes in icon style
        kapp->addKipcEventMask(KIPC::IconChanged);
        connect(kapp, TQ_SIGNAL(iconChanged(int)), this, TQ_SLOT(slotIconChanged(int)));
    }

    // finally, read in our configurable settings
    if ( readConfig )
        slotReadConfig();

    if ( mainWindow() )
        connect( mainWindow(), TQ_SIGNAL( toolBarPositionChanged( TQToolBar * ) ),
                 this, TQ_SLOT( toolBarPosChanged( TQToolBar * ) ) );

    // Hack to make sure we recalculate our size when we dock.
    connect( this, TQ_SIGNAL(placeChanged(TQDockWindow::Place)), TQ_SLOT(rebuildLayout()) );
}

int TDEToolBar::insertButton(const TQString& icon, int id, bool enabled,
                            const TQString& text, int index, TDEInstance *_instance )
{
    TDEToolBarButton *button = new TDEToolBarButton( icon, id, this, 0, text, _instance );

    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    doConnections( button );
    return index;
}


int TDEToolBar::insertButton(const TQString& icon, int id, const char *signal,
                            const TQObject *receiver, const char *slot,
                            bool enabled, const TQString& text, int index, TDEInstance *_instance )
{
    TDEToolBarButton *button = new TDEToolBarButton( icon, id, this, 0, text, _instance);
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    connect( button, signal, receiver, slot );
    doConnections( button );
    return index;
}


int TDEToolBar::insertButton(const TQPixmap& pixmap, int id, bool enabled,
                            const TQString& text, int index )
{
    TDEToolBarButton *button = new TDEToolBarButton( pixmap, id, this, 0, text);
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    doConnections( button );
    return index;
}


int TDEToolBar::insertButton(const TQPixmap& pixmap, int id, const char *signal,
                            const TQObject *receiver, const char *slot,
                            bool enabled, const TQString& text,
                            int index )
{
    TDEToolBarButton *button = new TDEToolBarButton( pixmap, id, this, 0, text);
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    connect( button, signal, receiver, slot );
    doConnections( button );
    return index;
}


int TDEToolBar::insertButton(const TQString& icon, int id, TQPopupMenu *popup,
                            bool enabled, const TQString &text, int index )
{
    TDEToolBarButton *button = new TDEToolBarButton( icon, id, this, 0, text );
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    button->setPopup( popup );
    doConnections( button );
    return index;
}


int TDEToolBar::insertButton(const TQPixmap& pixmap, int id, TQPopupMenu *popup,
                            bool enabled, const TQString &text, int index )
{
    TDEToolBarButton *button = new TDEToolBarButton( pixmap, id, this, 0, text );
    insertWidgetInternal( button, index, id );
    button->setEnabled( enabled );
    button->setPopup( popup );
    doConnections( button );
    return index;
}


int TDEToolBar::insertLined (const TQString& text, int id,
                            const char *signal,
                            const TQObject *receiver, const char *slot,
                            bool enabled ,
                            const TQString& toolTipText,
                            int size, int index )
{
    KLineEdit *lined = new KLineEdit ( this, 0 );
    if ( !toolTipText.isEmpty() )
        TQToolTip::add( lined, toolTipText );
    if ( size > 0 )
        lined->setMinimumWidth( size );
    insertWidgetInternal( lined, index, id );
    connect( lined, signal, receiver, slot );
    lined->setText(text);
    lined->setEnabled( enabled );
    return index;
}

int TDEToolBar::insertCombo (const TQStringList &list, int id, bool writable,
                            const char *signal, const TQObject *receiver,
                            const char *slot, bool enabled,
                            const TQString& tooltiptext,
                            int size, int index,
                            TQComboBox::Policy policy )
{
    KComboBox *combo = new KComboBox ( writable, this );

    insertWidgetInternal( combo, index, id );
    combo->insertStringList (list);
    combo->setInsertionPolicy(policy);
    combo->setEnabled( enabled );
    if ( size > 0 )
        combo->setMinimumWidth( size );
    if (!tooltiptext.isNull())
        TQToolTip::add( combo, tooltiptext );

    if ( signal && receiver && slot )
        connect ( combo, signal, receiver, slot );
    return index;
}


int TDEToolBar::insertCombo (const TQString& text, int id, bool writable,
                            const char *signal, TQObject *receiver,
                            const char *slot, bool enabled,
                            const TQString& tooltiptext,
                            int size, int index,
                            TQComboBox::Policy policy )
{
    KComboBox *combo = new KComboBox ( writable, this );
    insertWidgetInternal( combo, index, id );
    combo->insertItem (text);
    combo->setInsertionPolicy(policy);
    combo->setEnabled( enabled );
    if ( size > 0 )
        combo->setMinimumWidth( size );
    if (!tooltiptext.isNull())
        TQToolTip::add( combo, tooltiptext );
    connect (combo, signal, receiver, slot);
    return index;
}

int TDEToolBar::insertSeparator(int index, int id)
{
    TQWidget *w = new TDEToolBarSeparator( orientation(), false, this, "tool bar separator" );
    insertWidgetInternal( w, index, id );
    return index;
}

int TDEToolBar::insertLineSeparator(int index, int id)
{
    TQWidget *w = new TDEToolBarSeparator( orientation(), true, this, "tool bar separator" );
    insertWidgetInternal( w, index, id );
    return index;
}


int TDEToolBar::insertWidget(int id, int /*width*/, TQWidget *widget, int index)
{
    removeWidgetInternal( widget ); // in case we already have it ?
    insertWidgetInternal( widget, index, id );
    return index;
}

int TDEToolBar::insertAnimatedWidget(int id, TQObject *receiver, const char *slot,
                                    const TQString& icons, int index )
{
    KAnimWidget *anim = new KAnimWidget( icons, d->m_iconSize, this );
    insertWidgetInternal( anim, index, id );

    if ( receiver )
        connect( anim, TQ_SIGNAL(clicked()), receiver, slot);

    return index;
}

KAnimWidget *TDEToolBar::animatedWidget( int id )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return 0;
    KAnimWidget *aw = dynamic_cast<KAnimWidget *>(*it);
    if ( aw )
        return aw;
    TQObjectList *l = queryList( "KAnimWidget" );
    if ( !l || !l->first() ) {
        delete l;
        return 0;
    }

    for ( TQObject *o = l->first(); o; o = l->next() ) {
        KAnimWidget *aw = dynamic_cast<KAnimWidget *>(o);
        if ( aw )
        {
            delete l;
            return aw;
        }
    }

    delete l;
    return 0;
}


void TDEToolBar::addConnection (int id, const char *signal,
                               const TQObject *receiver, const char *slot)
{
    TQWidget* w = getWidget( id );
    if ( w )
        connect( w, signal, receiver, slot );
}

void TDEToolBar::setItemEnabled( int id, bool enabled )
{
    TQWidget* w = getWidget( id );
    if ( w )
        w->setEnabled( enabled );
}


void TDEToolBar::setButtonPixmap( int id, const TQPixmap& _pixmap )
{
    TDEToolBarButton * button = getButton( id );
    if ( button )
        button->setPixmap( _pixmap );
}


void TDEToolBar::setButtonIcon( int id, const TQString& _icon )
{
    TDEToolBarButton * button = getButton( id );
    if ( button )
        button->setIcon( _icon );
}

void TDEToolBar::setButtonIconSet( int id, const TQIconSet& iconset )
{
    TDEToolBarButton * button = getButton( id );
    if ( button )
        button->setIconSet( iconset );
}


void TDEToolBar::setDelayedPopup (int id , TQPopupMenu *_popup, bool toggle )
{
    TDEToolBarButton * button = getButton( id );
    if ( button )
        button->setDelayedPopup( _popup, toggle );
}


void TDEToolBar::setAutoRepeat (int id, bool flag)
{
    TDEToolBarButton * button = getButton( id );
    if ( button )
        button->setAutoRepeat( flag );
}


void TDEToolBar::setToggle (int id, bool flag )
{
    TDEToolBarButton * button = getButton( id );
    if ( button )
        button->setToggle( flag );
}


void TDEToolBar::toggleButton (int id)
{
    TDEToolBarButton * button = getButton( id );
    if ( button )
        button->toggle();
}


void TDEToolBar::setButton (int id, bool flag)
{
    TDEToolBarButton * button = getButton( id );
    if ( button )
        button->on( flag );
}


bool TDEToolBar::isButtonOn (int id) const
{
    TDEToolBarButton * button = const_cast<TDEToolBar*>( this )->getButton( id );
    return button ? button->isOn() : false;
}


void TDEToolBar::setLinedText (int id, const TQString& text)
{
    KLineEdit * lineEdit = getLined( id );
    if ( lineEdit )
        lineEdit->setText( text );
}


TQString TDEToolBar::getLinedText (int id) const
{
    KLineEdit * lineEdit = const_cast<TDEToolBar*>( this )->getLined( id );
    return lineEdit ? lineEdit->text() : TQString::null;
}


void TDEToolBar::insertComboItem (int id, const TQString& text, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->insertItem( text, index );
}

void TDEToolBar::insertComboList (int id, const TQStringList &list, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->insertStringList( list, index );
}


void TDEToolBar::removeComboItem (int id, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->removeItem( index );
}


void TDEToolBar::setCurrentComboItem (int id, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->setCurrentItem( index );
}


void TDEToolBar::changeComboItem  (int id, const TQString& text, int index)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->changeItem( text, index );
}


void TDEToolBar::clearCombo (int id)
{
    KComboBox * comboBox = getCombo( id );
    if (comboBox)
        comboBox->clear();
}


TQString TDEToolBar::getComboItem (int id, int index) const
{
    KComboBox * comboBox = const_cast<TDEToolBar*>( this )->getCombo( id );
    return comboBox ? comboBox->text( index ) : TQString::null;
}


KComboBox * TDEToolBar::getCombo(int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return 0;
    return dynamic_cast<KComboBox *>( *it );
}


KLineEdit * TDEToolBar::getLined (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return 0;
    return dynamic_cast<KLineEdit *>( *it );
}


TDEToolBarButton * TDEToolBar::getButton (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return 0;
    return dynamic_cast<TDEToolBarButton *>( *it );
}


void TDEToolBar::alignItemRight (int id, bool right )
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
        return;
    if ( rightAligned && !right && (*it) == rightAligned )
        rightAligned = 0;
    if ( (*it) && right )
        rightAligned = (*it);
}


TQWidget *TDEToolBar::getWidget (int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    return ( it == id2widget.end() ) ? 0 : (*it);
}


void TDEToolBar::setItemAutoSized (int id, bool yes )
{
    TQWidget *w = getWidget(id);
    if ( w && yes )
        setStretchableWidget( w );
}


void TDEToolBar::clear ()
{
    /* Delete any idle buttons, so TQToolBar doesn't delete them itself, making a mess */
    for(TQWidget *w=d->idleButtons.first(); w; w=d->idleButtons.next())
       w->blockSignals(false);    
    d->idleButtons.clear();
     
    TQToolBar::clear();
    widget2id.clear();
    id2widget.clear();
}


void TDEToolBar::removeItem(int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
    {
        kdDebug(220) << name() << " TDEToolBar::removeItem item " << id << " not found" << endl;
        return;
    }
    TQWidget * w = (*it);
    id2widget.remove( id );
    widget2id.remove( w );
    widgets.removeRef( w );
    delete w;
}


void TDEToolBar::removeItemDelayed(int id)
{
    Id2WidgetMap::Iterator it = id2widget.find( id );
    if ( it == id2widget.end() )
    {
        kdDebug(220) << name() << " TDEToolBar::removeItem item " << id << " not found" << endl;
        return;
    }
    TQWidget * w = (*it);
    id2widget.remove( id );
    widget2id.remove( w );
    widgets.removeRef( w );

    w->blockSignals(true);
    d->idleButtons.append(w);
    layoutTimer->start( 50, true );
}


void TDEToolBar::hideItem (int id)
{
    TQWidget *w = getWidget(id);
    if ( w )
        w->hide();
}


void TDEToolBar::showItem (int id)
{
    TQWidget *w = getWidget(id);
    if ( w )
        w->show();
}


int TDEToolBar::itemIndex (int id)
{
    TQWidget *w = getWidget(id);
    return w ? widgets.findRef(w) : -1;
}

int TDEToolBar::idAt (int index)
{
    TQWidget *w = widgets.at(index);
    return widget2id[w];
}

void TDEToolBar::setFullSize(bool flag )
{
    setHorizontalStretchable( flag );
    setVerticalStretchable( flag );
}


bool TDEToolBar::fullSize() const
{
    return isHorizontalStretchable() || isVerticalStretchable();
}


void TDEToolBar::enableMoving(bool flag )
{
    setMovingEnabled(flag);
}


void TDEToolBar::setBarPos (BarPosition bpos)
{
    if ( !mainWindow() )
        return;
    mainWindow()->moveDockWindow( this, (Dock)bpos );
    //kdDebug(220) << name() << " setBarPos dockWindowIndex=" << dockWindowIndex() << endl;
}


TDEToolBar::BarPosition TDEToolBar::barPos() const
{
    if ( !this->mainWindow() )
        return place() == TQDockWindow::InDock ? TDEToolBar::Top : TDEToolBar::Floating;
    Dock dock;
    int dm1, dm2;
    bool dm3;
    this->mainWindow()->getLocation( (TQToolBar*)this, dock, dm1, dm3, dm2 );
    if ( dock == DockUnmanaged ) {
        return (TDEToolBar::BarPosition)DockTop;
    }
    return (BarPosition)dock;
}


bool TDEToolBar::enable(BarStatus stat)
{
    bool mystat = isVisible();

    if ( (stat == Toggle && mystat) || stat == Hide )
        hide();
    else
        show();

    return isVisible() == mystat;
}


void TDEToolBar::setMaxHeight ( int h )
{
    setMaximumHeight( h );
}

int TDEToolBar::maxHeight()
{
    return maximumHeight();
}


void TDEToolBar::setMaxWidth (int dw)
{
    setMaximumWidth( dw );
}


int TDEToolBar::maxWidth()
{
    return maximumWidth();
}


void TDEToolBar::setTitle (const TQString& _title)
{
    setLabel( _title );
}


void TDEToolBar::enableFloating (bool )
{
}


void TDEToolBar::setIconText(IconText it)
{
    setIconText( it, true );
}


void TDEToolBar::setIconText(IconText icontext, bool update)
{
    bool doUpdate=false;

    if (icontext != d->m_iconText) {
        d->m_iconText = icontext;
        doUpdate=true;
        //kdDebug(220) << name() << "  icontext has changed, doUpdate=true" << endl;
    }
    else {
        //kdDebug(220) << name() << "  icontext hasn't changed, doUpdate=false" << endl;
    }

    if (!update)
        return;

    if (doUpdate)
        doModeChange(); // tell buttons what happened

    // ugly hack to force a TQMainWindow::triggerLayout( true )
    TQMainWindow *mw = mainWindow();
    if ( mw ) {
        mw->setUpdatesEnabled( false );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setUpdatesEnabled( true );
    }
}


TDEToolBar::IconText TDEToolBar::iconText() const
{
    return d->m_iconText;
}


void TDEToolBar::setIconSize(int size)
{
    setIconSize( size, true );
}

void TDEToolBar::setIconSize(int size, bool update)
{
    bool doUpdate=false;

    if ( size != d->m_iconSize ) {
            d->m_iconSize = size;
            doUpdate=true;
    }

    if (!update)
        return;

    if (doUpdate)
        doModeChange(); // tell buttons what happened

    // ugly hack to force a TQMainWindow::triggerLayout( true )
    if ( mainWindow() ) {
        TQMainWindow *mw = mainWindow();
        mw->setUpdatesEnabled( false );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setToolBarsMovable( !mw->toolBarsMovable() );
        mw->setUpdatesEnabled( true );
    }
}

int TDEToolBar::iconSize() const
{
    if ( !d->m_iconSize ) // default value?
		return iconSizeDefault();

	return d->m_iconSize;
}

int TDEToolBar::iconSizeDefault() const
{
	if (!::qstrcmp(name(), "mainToolBar"))
		return TDEGlobal::iconLoader()->currentSize(TDEIcon::MainToolbar);

	return TDEGlobal::iconLoader()->currentSize(TDEIcon::Toolbar);
}

void TDEToolBar::setEnableContextMenu(bool enable )
{
    d->m_enableContext = enable;
}


bool TDEToolBar::contextMenuEnabled() const
{
    return d->m_enableContext;
}


void TDEToolBar::setItemNoStyle(int id, bool no_style )
{
    TDEToolBarButton * button = getButton( id );
    if (button)
        button->setNoStyle( no_style );
}


void TDEToolBar::setFlat (bool flag)
{
    if ( !mainWindow() )
        return;
    if ( flag )
        mainWindow()->moveDockWindow( this, DockMinimized );
    else
        mainWindow()->moveDockWindow( this, DockTop );
    // And remember to save the new look later
    TDEMainWindow *kmw = dynamic_cast<TDEMainWindow *>(mainWindow());
    if ( kmw )
        kmw->setSettingsDirty();
}


int TDEToolBar::count() const
{
    return id2widget.count();
}


void TDEToolBar::saveState()
{
    // first, try to save to the xml file
    if ( d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty() ) {
        //kdDebug(220) << name() << " saveState: saving to " << d->m_xmlguiClient->xmlFile() << endl;
        TQString barname(!::qstrcmp(name(), "unnamed") ? "mainToolBar" : name());
        // try to find our toolbar
        d->modified = false;
        // go down one level to get to the right tags
        TQDomElement current;
        for( TQDomNode n = d->m_xmlguiClient->domDocument().documentElement().firstChild();
             !n.isNull(); n = n.nextSibling()) {
            current = n.toElement();

            if ( current.tagName().lower() != "toolbar" )
                continue;

            TQString curname(current.attribute( "name" ));

            if ( curname == barname ) {
                saveState( current );
                break;
            }
        }
        // if we didn't make changes, then just return
        if ( !d->modified )
            return;

        // now we load in the (non-merged) local file
        TQString local_xml(KXMLGUIFactory::readConfigFile(d->m_xmlguiClient->xmlFile(), true, d->m_xmlguiClient->instance()));
        TQDomDocument local;
        local.setContent(local_xml);

        // make sure we don't append if this toolbar already exists locally
        bool just_append = true;

        for( TQDomNode n = local.documentElement().firstChild();
             !n.isNull(); n = n.nextSibling()) {
            TQDomElement elem = n.toElement();

            if ( elem.tagName().lower() != "toolbar" )
                continue;

            TQString curname(elem.attribute( "name" ));

            if ( curname == barname ) {
                just_append = false;
                local.documentElement().replaceChild( current, elem );
                break;
            }
        }

        if (just_append)
            local.documentElement().appendChild( current );

        KXMLGUIFactory::saveConfigFile(local, d->m_xmlguiClient->localXMLFile(), d->m_xmlguiClient->instance() );

        return;
    }

    // if that didn't work, we save to the config file
    TDEConfig *config = TDEGlobal::config();
    saveSettings(config, TQString::null);
    config->sync();
}

TQString TDEToolBar::settingsGroup() const
{
    TQString configGroup;
    if (!::qstrcmp(name(), "unnamed") || !::qstrcmp(name(), "mainToolBar"))
        configGroup = "Toolbar style";
    else
        configGroup = TQString(name()) + " Toolbar style";
    if ( this->mainWindow() )
    {
        configGroup.prepend(" ");
        configGroup.prepend( this->mainWindow()->name() );
    }
    return configGroup;
}

void TDEToolBar::saveSettings(TDEConfig *config, const TQString &_configGroup)
{
    TQString configGroup = _configGroup;
    if (configGroup.isEmpty())
        configGroup = settingsGroup();
    //kdDebug(220) << name() << " saveSettings() group=" << _configGroup << " -> " << configGroup << endl;

    TQString position, icontext;
    int index;
    getAttributes( position, icontext, index );

    //kdDebug(220) << name() << "                position=" << position << " index=" << index << " offset=" << offset() << " newLine=" << newLine() << endl;

    TDEConfigGroupSaver saver(config, configGroup);

    if(!config->hasDefault("Position") && position == d->PositionDefault )
      config->revertToDefault("Position");
    else
      config->writeEntry("Position", position);

    //kdDebug(220) << name() << "                icontext=" << icontext << " hasDefault:" << config->hasDefault( "IconText" ) << " d->IconTextDefault=" << d->IconTextDefault << endl;

    if(d->m_honorStyle && icontext == d->IconTextDefault && !config->hasDefault("IconText") )
    {
      //kdDebug(220) << name() << "                reverting icontext to default" << endl;
      config->revertToDefault("IconText");
    }
    else
    {
      //kdDebug(220) << name() << "                writing icontext " << icontext << endl;
      config->writeEntry("IconText", icontext);
    }

    if(!config->hasDefault("IconSize") && iconSize() == iconSizeDefault() )
      config->revertToDefault("IconSize");
    else
      config->writeEntry("IconSize", iconSize());

    if(!config->hasDefault("Hidden") && isHidden() == d->HiddenDefault )
      config->revertToDefault("Hidden");
    else
      config->writeEntry("Hidden", isHidden());

    // Note that index, unlike the other settings, depends on the other toolbars
    // So on the first run with a clean local config file, even the usual
    // hasDefault/==IndexDefault test would save the toolbar indexes
    // (IndexDefault was 0, whereas index is the real index in the GUI)
    //
    // Saving the whole set of indexes is necessary though. When moving only
    // one toolbar, if we only saved the changed indexes, the toolbars wouldn't
    // reappear at the same position the next time.
    // The whole set of indexes has to be saved.
    //kdDebug(220) << name() << "                writing index " << index << endl;
    TDEMainWindow *kmw = dynamic_cast<TDEMainWindow *>(mainWindow());
    // don't save if there's only one toolbar

    // Don't use kmw->toolBarIterator() because you might
    // mess up someone else's iterator.  Make the list on your own
    TQPtrList<TDEToolBar> toolbarList;
    TQPtrList<TQToolBar> lst;
    for ( int i = (int)TQMainWindow::DockUnmanaged; i <= (int)DockMinimized; ++i ) {
        lst = kmw->toolBars( (ToolBarDock)i );
        for ( TQToolBar *tb = lst.first(); tb; tb = lst.next() ) {
            if ( !tb->inherits( "TDEToolBar" ) )
                continue;
            toolbarList.append( (TDEToolBar*)tb );
        }
    }
    TQPtrListIterator<TDEToolBar> toolbarIterator( toolbarList );
    if ( !kmw || toolbarIterator.count() > 1 )
        config->writeEntry("Index", index);
    else
        config->revertToDefault("Index");

    if(!config->hasDefault("Offset") && offset() == d->OffsetDefault )
      config->revertToDefault("Offset");
    else
      config->writeEntry("Offset", offset());

    if(!config->hasDefault("NewLine") && newLine() == d->NewLineDefault )
      config->revertToDefault("NewLine");
    else
      config->writeEntry("NewLine", newLine());
}


void TDEToolBar::setXMLGUIClient( KXMLGUIClient *client )
{
    d->m_xmlguiClient = client;
}

void TDEToolBar::setText( const TQString & txt )
{
    setLabel( txt + " (" + kapp->caption() + ") " );
}


TQString TDEToolBar::text() const
{
    return label();
}


void TDEToolBar::doConnections( TDEToolBarButton *button )
{
    connect(button, TQ_SIGNAL(clicked(int)), this, TQ_SIGNAL( clicked( int ) ) );
    connect(button, TQ_SIGNAL(doubleClicked(int)), this, TQ_SIGNAL( doubleClicked( int ) ) );
    connect(button, TQ_SIGNAL(released(int)), this, TQ_SIGNAL( released( int ) ) );
    connect(button, TQ_SIGNAL(pressed(int)), this, TQ_SIGNAL( pressed( int ) ) );
    connect(button, TQ_SIGNAL(toggled(int)), this, TQ_SIGNAL( toggled( int ) ) );
    connect(button, TQ_SIGNAL(highlighted(int, bool)), this, TQ_SIGNAL( highlighted( int, bool ) ) );
}

void TDEToolBar::mousePressEvent ( TQMouseEvent *m )
{
    if ( !mainWindow() )
        return;
    TQMainWindow *mw = mainWindow();
    if ( mw->toolBarsMovable() && d->m_enableContext ) {
        if ( m->button() == TQt::RightButton ) {
	    TQGuardedPtr<TDEToolBar> guard( this );
            int i = contextMenu()->exec( m->globalPos(), 0 );
	    // "Configure Toolbars" recreates toolbars, so we might not exist anymore.
	    if ( guard )
                slotContextAboutToHide();
            switch ( i ) {
            case -1:
                return; // popup canceled
            case CONTEXT_LEFT:
                mw->moveDockWindow( this, DockLeft );
                break;
            case CONTEXT_RIGHT:
                mw->moveDockWindow( this, DockRight );
                break;
            case CONTEXT_TOP:
                mw->moveDockWindow( this, DockTop );
                break;
            case CONTEXT_BOTTOM:
                mw->moveDockWindow( this, DockBottom );
                break;
            case CONTEXT_FLOAT:
                mw->moveDockWindow( this, DockTornOff );
                break;
            case CONTEXT_FLAT:
                mw->moveDockWindow( this, DockMinimized );
                break;
            case CONTEXT_ICONS:
                setIconText( IconOnly );
                break;
            case CONTEXT_TEXTRIGHT:
                setIconText( IconTextRight );
                break;
            case CONTEXT_TEXT:
                setIconText( TextOnly );
                break;
            case CONTEXT_TEXTUNDER:
                setIconText( IconTextBottom );
                break;
            default:
                if ( i >= CONTEXT_ICONSIZES )
					setIconSize( i - CONTEXT_ICONSIZES );
                else
                    return; // assume this was an action handled elsewhere, no need for setSettingsDirty()
            }
            TDEMainWindow *kmw = dynamic_cast<TDEMainWindow *>(mw);
            if ( kmw )
                kmw->setSettingsDirty();
        }
    }
}

void TDEToolBar::doModeChange()
{
    for(TQWidget *w=d->idleButtons.first(); w; w=d->idleButtons.next())
       w->blockSignals(false);
    d->idleButtons.clear();

    emit modechange();
}

void TDEToolBar::rebuildLayout()
{ 
    for(TQWidget *w=d->idleButtons.first(); w; w=d->idleButtons.next())
       w->blockSignals(false);
    d->idleButtons.clear();

    layoutTimer->stop();
    TQApplication::sendPostedEvents( this, TQEvent::ChildInserted );
    TQBoxLayout *l = boxLayout();

    // clear the old layout
    TQLayoutIterator it = l->iterator();
    while ( it.current() )
        it.deleteCurrent();

    for ( TQWidget *w = widgets.first(); w; w = widgets.next() ) {
        if ( w == rightAligned )
            continue;
        TDEToolBarSeparator *ktbs = dynamic_cast<TDEToolBarSeparator *>(w);
        if ( ktbs && !ktbs->showLine() ) {
            l->addSpacing( orientation() == TQt::Vertical ? w->sizeHint().height() : w->sizeHint().width() );
            w->hide();
            continue;
        }
        if ( dynamic_cast<TQPopupMenu *>(w) ) // w is a QPopupMenu?
            continue;
        l->addWidget( w );
        w->show();
        if ((orientation() == TQt::Horizontal) && dynamic_cast<TQLineEdit *>(w)) // w is TQLineEdit ?
            l->addSpacing(2); // A little bit extra spacing behind it.
    }
    if ( rightAligned ) {
        l->addStretch();
        l->addWidget( rightAligned );
        rightAligned->show();
    }

    if ( fullSize() ) {
        if ( !rightAligned )
            l->addStretch();
        if ( stretchableWidget )
            l->setStretchFactor( stretchableWidget, 10 );
    }
    l->invalidate();
    TQApplication::postEvent( this, new TQEvent( TQEvent::LayoutHint ) );
}

void TDEToolBar::childEvent( TQChildEvent *e )
{
    if ( e->child()->isWidgetType() ) {
        TQWidget * w = dynamic_cast<TQWidget *>(e->child());
        if (!w || !(::qstrcmp( "qt_dockwidget_internal", w->name())))
        {
           TQToolBar::childEvent( e );
           return;
        }
        if ( e->type() == TQEvent::ChildInserted ) {
            if ( !dynamic_cast<TQPopupMenu *>(w)) { // e->child() is not a QPopupMenu
                // prevent items that have been explicitly inserted by insert*() from
                // being inserted again
                if ( !widget2id.contains( w ) )
                {
                    int dummy = -1;
                    insertWidgetInternal( w, dummy, -1 );
                }
            }
        } else {
            removeWidgetInternal( w );
        }
        if ( isVisibleTo( 0 ) )
        {
            layoutTimer->start( 50, true );
            TQBoxLayout *l = boxLayout();

            // clear the old layout so that we don't get unnecessary layout
            // changes until we have rebuilt the thing
            TQLayoutIterator it = l->iterator();
            while ( it.current() )
               it.deleteCurrent();
        }
    }
    TQToolBar::childEvent( e );
}

void TDEToolBar::insertWidgetInternal( TQWidget *w, int &index, int id )
{
    // we can't have it in widgets, or something is really wrong
    //widgets.removeRef( w );

    connect( w, TQ_SIGNAL( destroyed() ),
             this, TQ_SLOT( widgetDestroyed() ) );
    if ( index == -1 || index > (int)widgets.count() ) {
        index = (int)widgets.count();
        widgets.append( w );
    }
    else
        widgets.insert( index, w );
    if ( id == -1 )
        id = id2widget.count();
    id2widget.insert( id, w );
    widget2id.insert( w, id );
}

void TDEToolBar::showEvent( TQShowEvent *e )
{
    TQToolBar::showEvent( e );
    rebuildLayout();
}

void TDEToolBar::setStretchableWidget( TQWidget *w )
{
    TQToolBar::setStretchableWidget( w );
    stretchableWidget = w;
}

TQSizePolicy TDEToolBar::sizePolicy() const
{
    if ( orientation() == TQt::Horizontal )
        return TQSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Fixed );
    else
        return TQSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Expanding );
}

TQSize TDEToolBar::sizeHint() const
{
    TQSize minSize(0,0);
    TDEToolBar *ncThis = const_cast<TDEToolBar *>(this);

    ncThis->polish();

    int margin = ncThis->layout()->margin() + frameWidth();
    switch( barPos() )
    {
     case TDEToolBar::Top:
     case TDEToolBar::Bottom:
       for ( TQWidget *w = ncThis->widgets.first(); w; w = ncThis->widgets.next() )
       {
          TQSize sh = w->sizeHint();
          if ( w->sizePolicy().horData() == TQSizePolicy::Ignored )
             sh.setWidth( 1 );
          if ( w->sizePolicy().verData() == TQSizePolicy::Ignored )
             sh.setHeight( 1 );
          sh = sh.boundedTo( w->maximumSize() )
                 .expandedTo( w->minimumSize() ).expandedTo( TQSize(1, 1) );

          minSize = minSize.expandedTo(TQSize(0, sh.height()));
          minSize += TQSize(sh.width()+1, 0);
          if (dynamic_cast<TQLineEdit *>(w)) // w is a TQLineEdit ?
             minSize += TQSize(2, 0); // A little bit extra spacing behind it.
       }

       minSize += TQSize(TQApplication::style().pixelMetric( TQStyle::PM_DockWindowHandleExtent ), 0);
       minSize += TQSize(margin*2, margin*2);
       break;

     case TDEToolBar::Left:
     case TDEToolBar::Right:
       for ( TQWidget *w = ncThis->widgets.first(); w; w = ncThis->widgets.next() )
       {
          TQSize sh = w->sizeHint();
          if ( w->sizePolicy().horData() == TQSizePolicy::Ignored )
             sh.setWidth( 1 );
          if ( w->sizePolicy().verData() == TQSizePolicy::Ignored )
             sh.setHeight( 1 );
          sh = sh.boundedTo( w->maximumSize() )
                 .expandedTo( w->minimumSize() ).expandedTo( TQSize(1, 1) );

          minSize = minSize.expandedTo(TQSize(sh.width(), 0));
          minSize += TQSize(0, sh.height()+1);
       }
       minSize += TQSize(0, TQApplication::style().pixelMetric( TQStyle::PM_DockWindowHandleExtent ));
       minSize += TQSize(margin*2, margin*2);
       break;

     default:
       minSize = TQToolBar::sizeHint();
       break;
    }
    return minSize;
}

TQSize TDEToolBar::minimumSize() const
{
    return minimumSizeHint();
}

TQSize TDEToolBar::minimumSizeHint() const
{
    return sizeHint();
}

bool TDEToolBar::highlight() const
{
    return d->m_highlight;
}

void TDEToolBar::hide()
{
    TQToolBar::hide();
}

void TDEToolBar::show()
{
    TQToolBar::show();
}

void TDEToolBar::resizeEvent( TQResizeEvent *e )
{
    bool b = isUpdatesEnabled();
    setUpdatesEnabled( false );
    TQToolBar::resizeEvent( e );
    if (b)
    {
      if (layoutTimer->isActive())
      {
         // Wait with repainting till layout is complete.
         d->repaintTimer.start( 100, true );
      }
      else
      {
         // Repaint now
         slotRepaint();
      }
    }
//     else {
//         printf("[WARNING] In TDEToolBar::resizeEvent, but this code block should not be executing.  Preventing toolbar lockup.  [Code 0045]\n");
//         setUpdatesEnabled( true );
//     }
}

void TDEToolBar::slotIconChanged(int group)
{
    if ((group != TDEIcon::Toolbar) && (group != TDEIcon::MainToolbar))
        return;
    if ((group == TDEIcon::MainToolbar) != !::qstrcmp(name(), "mainToolBar"))
        return;

    doModeChange();

    if (isVisible())
        updateGeometry();
}

void TDEToolBar::slotReadConfig()
{
    //kdDebug(220) << name() << " slotReadConfig" << endl;
    // Read appearance settings (hmm, we used to do both here,
    // but a well behaved application will call applyMainWindowSettings
    // anyway, right ?)
    applyAppearanceSettings(TDEGlobal::config(), TQString::null );
}

void TDEToolBar::slotAppearanceChanged()
{
    // Read appearance settings from global file.
    applyAppearanceSettings(TDEGlobal::config(), TQString::null, true /* lose local settings */ );

    // And remember to save the new look later
    TDEMainWindow *kmw = dynamic_cast<TDEMainWindow *>(mainWindow());
    if ( kmw )
        kmw->setSettingsDirty();
}

//static
bool TDEToolBar::highlightSetting()
{
    TQString grpToolbar(TQString::fromLatin1("Toolbar style"));
    TDEConfigGroupSaver saver(TDEGlobal::config(), grpToolbar);
    return TDEGlobal::config()->readBoolEntry(TQString::fromLatin1("Highlighting"),true);
}

//static
bool TDEToolBar::transparentSetting()
{
    TQString grpToolbar(TQString::fromLatin1("Toolbar style"));
    TDEConfigGroupSaver saver(TDEGlobal::config(), grpToolbar);
    return TDEGlobal::config()->readBoolEntry(TQString::fromLatin1("TransparentMoving"),true);
}

//static
TDEToolBar::IconText TDEToolBar::iconTextSetting()
{
    TQString grpToolbar(TQString::fromLatin1("Toolbar style"));
    TDEConfigGroupSaver saver(TDEGlobal::config(), grpToolbar);
    TQString icontext = TDEGlobal::config()->readEntry(TQString::fromLatin1("IconText"),TQString::fromLatin1("IconOnly"));
    if ( icontext == "IconTextRight" )
        return IconTextRight;
    else if ( icontext == "IconTextBottom" )
        return IconTextBottom;
    else if ( icontext == "TextOnly" )
        return TextOnly;
    else
        return IconOnly;
}

void TDEToolBar::applyAppearanceSettings(TDEConfig *config, const TQString &_configGroup, bool forceGlobal)
{
    TQString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;
    //kdDebug(220) << name() << " applyAppearanceSettings: configGroup=" << configGroup << " forceGlobal=" << forceGlobal << endl;

    // If we have application-specific settings in the XML file,
    // and nothing in the application's config file, then
    // we don't apply the global defaults, the XML ones are preferred
    // (see applySettings for a full explanation)
    // This is the reason for the xmlgui tests below.
    bool xmlgui = d->m_xmlguiClient && !d->m_xmlguiClient->xmlFile().isEmpty();

    TDEConfig *gconfig = TDEGlobal::config();

    static const TQString &attrIconText  = TDEGlobal::staticQString("IconText");
    static const TQString &attrHighlight = TDEGlobal::staticQString("Highlighting");
    static const TQString &attrTrans     = TDEGlobal::staticQString("TransparentMoving");
    static const TQString &attrIconSize  = TDEGlobal::staticQString("IconSize");

    // we actually do this in two steps.
    // First, we read in the global styles [Toolbar style] (from the KControl module).
    // Then, if the toolbar is NOT 'mainToolBar', we will also try to read in [barname Toolbar style]
    bool highlight;
    int transparent;
    bool applyIconText = !xmlgui; // if xmlgui is used, global defaults won't apply
    bool applyIconSize = !xmlgui;

    int iconSize = d->IconSizeDefault;
    TQString iconText = d->IconTextDefault;

    // this is the first iteration
    TQString grpToolbar(TQString::fromLatin1("Toolbar style"));
    { // start block for TDEConfigGroupSaver
        TDEConfigGroupSaver saver(gconfig, grpToolbar);

        // first, get the generic settings
        highlight   = gconfig->readBoolEntry(attrHighlight, true);
        transparent = gconfig->readBoolEntry(attrTrans, true);

        // we read in the IconText property *only* if we intend on actually
        // honoring it
        if (d->m_honorStyle)
            d->IconTextDefault = gconfig->readEntry(attrIconText, d->IconTextDefault);
        else
            d->IconTextDefault = "IconOnly";

        // Use the default icon size for toolbar icons.
        d->IconSizeDefault = gconfig->readNumEntry(attrIconSize, d->IconSizeDefault);

        iconSize = d->IconSizeDefault;
        iconText = d->IconTextDefault;

        if ( !forceGlobal && config->hasGroup(configGroup) )
        {
            config->setGroup(configGroup);

            // first, get the generic settings
            highlight   = config->readBoolEntry(attrHighlight, highlight);
            transparent = config->readBoolEntry(attrTrans, transparent);

            // read in the IconText property
            if ( config->hasKey( attrIconText ) ) {
                iconText = config->readEntry(attrIconText);
                applyIconText = true;
                //kdDebug(220) << name() << " read icontext=" << d->IconTextDefault << ", that will be the default" << endl;
            }

            // now get the size
            if ( config->hasKey( attrIconSize ) ) {
                iconSize = config->readNumEntry(attrIconSize);
                applyIconSize = true;
            }
        }

        // revert back to the old group
    } // end block for TDEConfigGroupSaver

    bool doUpdate = false;

    IconText icon_text;
    if ( iconText == "IconTextRight" )
        icon_text = IconTextRight;
    else if ( iconText == "IconTextBottom" )
        icon_text = IconTextBottom;
    else if ( iconText == "TextOnly" )
        icon_text = TextOnly;
    else
        icon_text = IconOnly;

    // check if the icon/text has changed
    if (icon_text != d->m_iconText && applyIconText) {
        //kdDebug(220) << name() << " applyAppearanceSettings setIconText " << icon_text << endl;
        setIconText(icon_text, false);
        doUpdate = true;
    }

    // ...and check if the icon size has changed
    if (iconSize != d->m_iconSize && applyIconSize) {
        setIconSize(iconSize, false);
        doUpdate = true;
    }

    TQMainWindow *mw = mainWindow();

    // ...and if we should highlight
    if ( highlight != d->m_highlight ) {
        d->m_highlight = highlight;
        doUpdate = true;
    }

    // ...and if we should move transparently
    if ( mw && transparent != (!mw->opaqueMoving()) ) {
        mw->setOpaqueMoving( !transparent );
    }

    if (doUpdate)
        doModeChange(); // tell buttons what happened

    if (isVisible ())
        updateGeometry();
}

void TDEToolBar::applySettings(TDEConfig *config, const TQString &_configGroup)
{
    return applySettings(config,_configGroup,false);
}

void TDEToolBar::applySettings(TDEConfig *config, const TQString &_configGroup, bool force)
{
    //kdDebug(220) << name() << " applySettings group=" << _configGroup << endl;

    TQString configGroup = _configGroup.isEmpty() ? settingsGroup() : _configGroup;

    /*
      Let's explain this a bit more in details.
      The order in which we apply settings is :
       Global config / <appnamerc> user settings                        if no XMLGUI is used
       Global config / App-XML attributes / <appnamerc> user settings   if XMLGUI is used

      So in the first case, we simply read everything from TDEConfig as below,
      but in the second case we don't do anything here if there is no app-specific config,
      and the XMLGUI-related code (loadState()) uses the static methods of this class
      to get the global defaults.

      Global config doesn't include position (index, offset, newline and hidden/shown).
    */

    // First the appearance stuff - the one which has a global config
    applyAppearanceSettings( config, configGroup );

    // ...and now the position stuff
    if ( config->hasGroup(configGroup) || force )
    {
        TDEConfigGroupSaver cgs(config, configGroup);

        static const TQString &attrPosition  = TDEGlobal::staticQString("Position");
        static const TQString &attrIndex  = TDEGlobal::staticQString("Index");
        static const TQString &attrOffset  = TDEGlobal::staticQString("Offset");
        static const TQString &attrNewLine  = TDEGlobal::staticQString("NewLine");
        static const TQString &attrHidden  = TDEGlobal::staticQString("Hidden");

        TQString position = config->readEntry(attrPosition, d->PositionDefault);
        int index = config->readNumEntry(attrIndex, -1);
        int offset = config->readNumEntry(attrOffset, d->OffsetDefault);
        bool newLine = config->readBoolEntry(attrNewLine, d->NewLineDefault);
        bool hidden = config->readBoolEntry(attrHidden, d->HiddenDefault);

        Dock pos(DockTop);
        if ( position == "Top" )
            pos = DockTop;
        else if ( position == "Bottom" )
            pos = DockBottom;
        else if ( position == "Left" )
            pos = DockLeft;
        else if ( position == "Right" )
            pos = DockRight;
        else if ( position == "Floating" )
            pos = DockTornOff;
        else if ( position == "Flat" )
            pos = DockMinimized;

        //kdDebug(220) << name() << " applySettings hidden=" << hidden << endl;
        if (hidden)
            hide();
        else
            show();

        if ( mainWindow() )
        {
            //kdDebug(220) << name() << " applySettings updating ToolbarInfo" << endl;
            d->toolBarInfo = TDEToolBarPrivate::ToolBarInfo( pos, index, newLine, offset );
            positionYourself( true );
        }
        if (isVisible ())
            updateGeometry();
    }
}

bool TDEToolBar::event( TQEvent *e )
{
    if ( (e->type() == TQEvent::LayoutHint) && isUpdatesEnabled() )
       d->repaintTimer.start( 100, true );

    if (e->type() == TQEvent::ChildInserted )
    {
       // Bypass TQToolBar::event,
       // it will show() the inserted child and we don't want to
       // do that until we have rebuilt the layout.
       childEvent((TQChildEvent *)e);
       return true;
    }

    return TQToolBar::event( e );
}

void TDEToolBar::slotRepaint()
{
    setUpdatesEnabled( false );
    // Send a resizeEvent to update the "toolbar extension arrow"
    // (The button you get when your toolbar-items don't fit in
    // the available space)
    TQResizeEvent ev(size(), size());
    resizeEvent(&ev);
    TQApplication::sendPostedEvents( this, TQEvent::LayoutHint );
    setUpdatesEnabled( true );
    repaint( true );
}

void TDEToolBar::toolBarPosChanged( TQToolBar *tb )
{
    if ( tb != this )
        return;
    if ( d->oldPos == DockMinimized )
        rebuildLayout();
    d->oldPos = (TQMainWindow::ToolBarDock)barPos();
    TDEMainWindow *kmw = dynamic_cast<TDEMainWindow *>(mainWindow());
    if ( kmw )
        kmw->setSettingsDirty();
}

static TDEToolBar::Dock stringToDock( const TQString& attrPosition )
{
    TDEToolBar::Dock dock = TDEToolBar::DockTop;
    if ( !attrPosition.isEmpty() ) {
        if ( attrPosition == "top" )
            dock = TDEToolBar::DockTop;
        else if ( attrPosition == "left" )
            dock = TDEToolBar::DockLeft;
        else if ( attrPosition == "right" )
            dock = TDEToolBar::DockRight;
        else if ( attrPosition == "bottom" )
            dock = TDEToolBar::DockBottom;
        else if ( attrPosition == "floating" )
            dock = TDEToolBar::DockTornOff;
        else if ( attrPosition == "flat" )
            dock = TDEToolBar::DockMinimized;
    }
    return dock;
}


void TDEToolBar::loadState( const TQDomElement &element )
{
    TQMainWindow *mw = mainWindow();

    if ( !mw )
        return;

    {
        TQCString text = element.namedItem( "text" ).toElement().text().utf8();
        if ( text.isEmpty() )
            text = element.namedItem( "Text" ).toElement().text().utf8();
        if ( !text.isEmpty() )
            setText( i18n( text ) );
    }

    {
        TQCString attrFullWidth = element.attribute( "fullWidth" ).lower().latin1();
        if ( !attrFullWidth.isEmpty() )
            setFullSize( attrFullWidth == "true" );
    }

    /*
      This method is called in order to load toolbar settings from XML.
      However this can be used in two rather different cases:
      - for the initial loading of the app's XML. In that case the settings
        are only the defaults, the user's TDEConfig settings will override them
        (KDE4 TODO: how about saving those user settings into the local XML file instead?
        Then this whole thing would be simpler, no TDEConfig settings to apply afterwards.
        OTOH we'd have to migrate those settings when the .rc version increases,
        like we do for shortcuts)

      - for later re-loading when switching between parts in KXMLGUIFactory.
        In that case the XML contains the final settings, not the defaults.
        We do need the defaults, and the toolbar might have been completely
        deleted and recreated meanwhile. So we store the app-default settings
        into the XML.
     */
    bool loadingAppDefaults = true;
    if ( element.hasAttribute( "offsetDefault" ) )
    {
        // this isn't the first time, so the defaults have been saved into the (in-memory) XML
        loadingAppDefaults = false;
        d->OffsetDefault = element.attribute( "offsetDefault" ).toInt();
        d->NewLineDefault = element.attribute( "newlineDefault" ) == "true";
        d->HiddenDefault = element.attribute( "hiddenDefault" ) == "true";
        d->IconSizeDefault = element.attribute( "iconSizeDefault" ).toInt();
        d->PositionDefault = element.attribute( "positionDefault" );
        d->IconTextDefault = element.attribute( "iconTextDefault" );
    }
    //kdDebug(220) << name() << " loadState loadingAppDefaults=" << loadingAppDefaults << endl;

    Dock dock = stringToDock( element.attribute( "position" ).lower() );

    {
        TQCString attrIconText = element.attribute( "iconText" ).lower().latin1();
        if ( !attrIconText.isEmpty() ) {
            //kdDebug(220) << name() << " loadState attrIconText=" << attrIconText << endl;
            if ( attrIconText == "icontextright" )
                setIconText( TDEToolBar::IconTextRight );
            else if ( attrIconText == "textonly" )
                setIconText( TDEToolBar::TextOnly );
            else if ( attrIconText == "icontextbottom" )
                setIconText( TDEToolBar::IconTextBottom );
            else if ( attrIconText == "icononly" )
                setIconText( TDEToolBar::IconOnly );
        } else
	{
	    //kdDebug(220) << name() << " loadState no iconText attribute in XML, using iconTextSetting=" << iconTextSetting() << endl;
            // Use global setting
            if (d->m_honorStyle)
                setIconText( iconTextSetting() );
            else
                setIconText( d->IconTextDefault );
	}
    }

    TQString attrIconSize = element.attribute( "iconSize" ).lower();
    int iconSize = d->IconSizeDefault;
    if ( !attrIconSize.isEmpty() )
        iconSize = attrIconSize.toInt();
    setIconSize( iconSize );

    int index = -1; // append by default. This is very important, otherwise
    // with all 0 indexes, we keep reversing the toolbars.
    {
        TQString attrIndex = element.attribute( "index" ).lower();
        if ( !attrIndex.isEmpty() )
            index = attrIndex.toInt();
    }

    int offset = d->OffsetDefault;
    bool newLine = d->NewLineDefault;
    bool hidden = d->HiddenDefault;

    {
        TQString attrOffset = element.attribute( "offset" );
        if ( !attrOffset.isEmpty() )
            offset = attrOffset.toInt();
    }

    {
        TQString attrNewLine = element.attribute( "newline" ).lower();
        if ( !attrNewLine.isEmpty() )
            newLine = attrNewLine == "true";
    }

    {
        TQString attrHidden = element.attribute( "hidden" ).lower();
        if ( !attrHidden.isEmpty() ) {
            hidden = attrHidden  == "true";
        }
    }

    d->toolBarInfo = TDEToolBarPrivate::ToolBarInfo( dock, index, newLine, offset );
    mw->addDockWindow( this, dock, newLine );
    mw->moveDockWindow( this, dock, newLine, index, offset );

    // Apply the highlight button setting
    d->m_highlight = highlightSetting();

    if ( hidden )
        hide();
    else
        show();

    if ( loadingAppDefaults )
    {
        getAttributes( d->PositionDefault, d->IconTextDefault, index );
        //kdDebug(220) << name() << " loadState IconTextDefault=" << d->IconTextDefault << endl;
        d->OffsetDefault = offset;
        d->NewLineDefault = newLine;
        d->HiddenDefault = hidden;
        d->IconSizeDefault = iconSize;
    }
    //kdDebug(220) << name() << " loadState hidden=" << hidden << endl;

    // Apply transparent-toolbar-moving setting (ok, this is global to the mainwindow,
    // but we do it only if there are toolbars...)
    // KDE4: move to TDEMainWindow
    if ( transparentSetting() != !mw->opaqueMoving() ) {
        mw->setOpaqueMoving( !transparentSetting() );
    }
}

int TDEToolBar::dockWindowIndex()
{
    int index = 0;
    Q_ASSERT( mainWindow() );
    if ( mainWindow() ) {
        TQMainWindow::ToolBarDock dock;
        bool newLine;
        int offset;
        mainWindow()->getLocation( this, dock, index, newLine, offset );
    }
    return index;
}

void TDEToolBar::getAttributes( TQString &position, TQString &icontext, int &index )
{
    // get all of the stuff to save
    switch ( barPos() ) {
    case TDEToolBar::Flat:
        position = "Flat";
        break;
    case TDEToolBar::Bottom:
        position = "Bottom";
        break;
    case TDEToolBar::Left:
        position = "Left";
        break;
    case TDEToolBar::Right:
        position = "Right";
        break;
    case TDEToolBar::Floating:
        position = "Floating";
        break;
    case TDEToolBar::Top:
    default:
        position = "Top";
        break;
    }

    index = dockWindowIndex();

    switch (d->m_iconText) {
    case TDEToolBar::IconTextRight:
        icontext = "IconTextRight";
        break;
    case TDEToolBar::IconTextBottom:
        icontext = "IconTextBottom";
        break;
    case TDEToolBar::TextOnly:
        icontext = "TextOnly";
        break;
    case TDEToolBar::IconOnly:
    default:
        icontext = "IconOnly";
        break;
    }
    //kdDebug(220) << name() << " getAttributes: icontext=" << icontext << endl;
}

void TDEToolBar::saveState( TQDomElement &current )
{
    Q_ASSERT( !current.isNull() );
    TQString position, icontext;
    int index = -1;
    getAttributes( position, icontext, index );

    current.setAttribute( "noMerge", "1" );
    current.setAttribute( "position", position );
    current.setAttribute( "iconText", icontext );
    current.setAttribute( "index", index );
    current.setAttribute( "offset", offset() );
    current.setAttribute( "newline", newLine() );
    if ( isHidden() )
        current.setAttribute( "hidden", "true" );
    d->modified = true;

    // TODO if this method is used by more than KXMLGUIBuilder, e.g. to save XML settings to *disk*,
    // then the stuff below shouldn't always be done.
    current.setAttribute( "offsetDefault", d->OffsetDefault );
    current.setAttribute( "newlineDefault", d->NewLineDefault );
    current.setAttribute( "hiddenDefault", d->HiddenDefault ? "true" : "false" );
    current.setAttribute( "iconSizeDefault", d->IconSizeDefault );
    current.setAttribute( "positionDefault", d->PositionDefault );
    current.setAttribute( "iconTextDefault", d->IconTextDefault );

    //kdDebug(220) << name() << " saveState: saving index=" << index << " iconText=" << icontext << " hidden=" << isHidden() << endl;
}

// Called by TDEMainWindow::finalizeGUI
void TDEToolBar::positionYourself( bool force )
{
    if (force)
        d->positioned = false;

    if ( d->positioned || !mainWindow() )
    {
        //kdDebug(220) << name() << " positionYourself d->positioned=true  ALREADY DONE" << endl;
        return;
    }
    // we can't test for ForceHide after moveDockWindow because QDockArea
    // does a reparent() with showIt == true
    bool hidden = isHidden();
    //kdDebug(220) << name() << " positionYourself  dock=" << d->toolBarInfo.dock << " newLine=" << d->toolBarInfo.newline << " index=" << d->toolBarInfo.index << " offset=" << d->toolBarInfo.offset << endl;
    mainWindow()->moveDockWindow( this, d->toolBarInfo.dock,
                                  d->toolBarInfo.newline,
                                  d->toolBarInfo.index,
                                  d->toolBarInfo.offset );

    //kdDebug(220) << name() << " positionYourself dockWindowIndex=" << dockWindowIndex() << endl;
    if ( hidden )
        hide();
    else
        show();
    // This method can only have an effect once - unless force is set
    d->positioned = true;
}

TDEPopupMenu *TDEToolBar::contextMenu()
{
  if ( context )
    return context;
  // Construct our context popup menu. Name it qt_dockwidget_internal so it
  // won't be deleted by TQToolBar::clear().
  context = new TDEPopupMenu( this, "qt_dockwidget_internal" );
  context->insertTitle(i18n("Toolbar Menu"));

  TDEPopupMenu *orient = new TDEPopupMenu( context, "orient" );
  orient->insertItem( i18n("toolbar position string","Top"),  CONTEXT_TOP );
  orient->insertItem( i18n("toolbar position string","Left"), CONTEXT_LEFT );
  orient->insertItem( i18n("toolbar position string","Right"), CONTEXT_RIGHT );
  orient->insertItem( i18n("toolbar position string","Bottom"), CONTEXT_BOTTOM );
  orient->insertSeparator(-1);
  orient->insertItem( i18n("toolbar position string","Floating"), CONTEXT_FLOAT );
  orient->insertItem( i18n("min toolbar", "Flat"), CONTEXT_FLAT );

  TDEPopupMenu *mode = new TDEPopupMenu( context, "mode" );
  mode->insertItem( i18n("Icons Only"), CONTEXT_ICONS );
  mode->insertItem( i18n("Text Only"), CONTEXT_TEXT );
  mode->insertItem( i18n("Text Alongside Icons"), CONTEXT_TEXTRIGHT );
  mode->insertItem( i18n("Text Under Icons"), CONTEXT_TEXTUNDER );

  TDEPopupMenu *size = new TDEPopupMenu( context, "size" );
  size->insertItem( i18n("Default"), CONTEXT_ICONSIZES );
  // Query the current theme for available sizes
  TDEIconTheme *theme = TDEGlobal::instance()->iconLoader()->theme();
  TQValueList<int> avSizes;
  if (theme)
  {
      if (!::qstrcmp(name(), "mainToolBar"))
          avSizes = theme->querySizes( TDEIcon::MainToolbar);
      else
          avSizes = theme->querySizes( TDEIcon::Toolbar);
  }

  d->iconSizes = avSizes;
  qHeapSort(avSizes);

  TQValueList<int>::Iterator it;
  if (avSizes.count() < 10) {
      // Fixed or threshold type icons
	  TQValueList<int>::Iterator end(avSizes.end());
      for (it=avSizes.begin(); it!=end; ++it) {
          TQString text;
          if ( *it < 19 )
              text = i18n("Small (%1x%2)").arg(*it).arg(*it);
          else if (*it < 25)
              text = i18n("Medium (%1x%2)").arg(*it).arg(*it);
          else if (*it < 35)
              text = i18n("Large (%1x%2)").arg(*it).arg(*it);
          else
              text = i18n("Huge (%1x%2)").arg(*it).arg(*it);
          //we use the size as an id, with an offset
          size->insertItem( text, CONTEXT_ICONSIZES + *it );
      }
  }
  else {
      // Scalable icons.
      const int progression[] = {16, 22, 32, 48, 64, 96, 128, 192, 256};

      it = avSizes.begin();
      for (uint i = 0; i < 9; i++) {
          while (it++ != avSizes.end()) {
              if (*it >= progression[i]) {
                  TQString text;
                  if ( *it < 19 )
                      text = i18n("Small (%1x%2)").arg(*it).arg(*it);
                  else if (*it < 25)
                      text = i18n("Medium (%1x%2)").arg(*it).arg(*it);
                  else if (*it < 35)
                      text = i18n("Large (%1x%2)").arg(*it).arg(*it);
                  else
                      text = i18n("Huge (%1x%2)").arg(*it).arg(*it);
                  //we use the size as an id, with an offset
                  size->insertItem( text, CONTEXT_ICONSIZES + *it );
                  break;
              }
          }
      }
  }

  context->insertItem( i18n("Orientation"), orient );
  orient->setItemChecked(CONTEXT_TOP, true);
  context->insertItem( i18n("Text Position"), mode );
  context->setItemChecked(CONTEXT_ICONS, true);
  context->insertItem( i18n("Icon Size"), size );

  connect( context, TQ_SIGNAL( aboutToShow() ), this, TQ_SLOT( slotContextAboutToShow() ) );
  // Unplugging a submenu from abouttohide leads to the popupmenu floating around
  // So better simply call that code from after exec() returns (DF)
  //connect( context, TQ_SIGNAL( aboutToHide() ), this, TQ_SLOT( slotContextAboutToHide() ) );
  return context;
}

void TDEToolBar::slotContextAboutToShow()
{
  // The idea here is to reuse the "static" part of the menu to save time.
  // But the "Toolbars" action is dynamic (can be a single action or a submenu)
  // and ToolBarHandler::setupActions() deletes it, so better not keep it around.
  // So we currently plug/unplug the last two actions of the menu.
  // Another way would be to keep around the actions and plug them all into a (new each time) popupmenu.
  TDEMainWindow *kmw = dynamic_cast<TDEMainWindow *>(mainWindow());
  if ( kmw ) {
      kmw->setupToolbarMenuActions();
      // Only allow hiding a toolbar if the action is also plugged somewhere else (e.g. menubar)
      TDEAction *tbAction = kmw->toolBarMenuAction();
      if ( tbAction && tbAction->containerCount() > 0 )
          tbAction->plug(context);
  }

  // try to find "configure toolbars" action
  TDEAction *configureAction = 0;
  const char* actionName = KStdAction::name(KStdAction::ConfigureToolbars);
  if ( d->m_xmlguiClient )
    configureAction = d->m_xmlguiClient->actionCollection()->action(actionName);
  if ( !configureAction && kmw )
    configureAction = kmw->actionCollection()->action(actionName);
  if ( configureAction )
    configureAction->plug(context);
  KEditToolbar::setDefaultToolbar(name());

  for(int i = CONTEXT_ICONS; i <= CONTEXT_TEXTUNDER; ++i)
    context->setItemChecked(i, false);

  switch( d->m_iconText )
  {
        case IconOnly:
        default:
            context->setItemChecked(CONTEXT_ICONS, true);
            break;
        case IconTextRight:
            context->setItemChecked(CONTEXT_TEXTRIGHT, true);
            break;
        case TextOnly:
            context->setItemChecked(CONTEXT_TEXT, true);
            break;
        case IconTextBottom:
            context->setItemChecked(CONTEXT_TEXTUNDER, true);
            break;
  }

  TQValueList<int>::ConstIterator iIt = d->iconSizes.begin();
  TQValueList<int>::ConstIterator iEnd = d->iconSizes.end();
  for (; iIt != iEnd; ++iIt )
      context->setItemChecked( CONTEXT_ICONSIZES + *iIt, false );

  context->setItemChecked( CONTEXT_ICONSIZES, false );

  context->setItemChecked( CONTEXT_ICONSIZES + d->m_iconSize, true );

  for ( int i = CONTEXT_TOP; i <= CONTEXT_FLAT; ++i )
      context->setItemChecked( i, false );

  switch ( barPos() )
  {
  case TDEToolBar::Flat:
      context->setItemChecked( CONTEXT_FLAT, true );
      break;
  case TDEToolBar::Bottom:
      context->setItemChecked( CONTEXT_BOTTOM, true );
      break;
  case TDEToolBar::Left:
      context->setItemChecked( CONTEXT_LEFT, true );
      break;
  case TDEToolBar::Right:
      context->setItemChecked( CONTEXT_RIGHT, true );
      break;
  case TDEToolBar::Floating:
      context->setItemChecked( CONTEXT_FLOAT, true );
      break;
  case TDEToolBar::Top:
      context->setItemChecked( CONTEXT_TOP, true );
      break;
  default: break;
  }
}

void TDEToolBar::slotContextAboutToHide()
{
  // We have to unplug whatever slotContextAboutToShow plugged into the menu.
  // Unplug the toolbar menu action
  TDEMainWindow *kmw = dynamic_cast<TDEMainWindow *>(mainWindow());
  if ( kmw && kmw->toolBarMenuAction() )
    if ( kmw->toolBarMenuAction()->containerCount() > 1 )
      kmw->toolBarMenuAction()->unplug(context);

  // Unplug the configure toolbars action too, since it's afterwards anyway
  TDEAction *configureAction = 0;
  const char* actionName = KStdAction::name(KStdAction::ConfigureToolbars);
  if ( d->m_xmlguiClient )
    configureAction = d->m_xmlguiClient->actionCollection()->action(actionName);
  if ( !configureAction && kmw )
    configureAction = kmw->actionCollection()->action(actionName);
  if ( configureAction )
    configureAction->unplug(context);

  TQPtrListIterator<TQWidget> it( widgets );
  TQWidget *wdg;
  while ( ( wdg = it.current() ) != 0 ) {
    if ( wdg->inherits( "TQToolButton" ) )
      static_cast<TQToolButton*>( wdg )->setDown( false );
    ++it;
  }
}

void TDEToolBar::widgetDestroyed()
{
    removeWidgetInternal( (TQWidget*)sender() );
}

void TDEToolBar::removeWidgetInternal( TQWidget * w )
{
    widgets.removeRef( w );
    TQMap< TQWidget*, int >::Iterator it = widget2id.find( w );
    if ( it == widget2id.end() )
        return;
    id2widget.remove( *it );
    widget2id.remove( it );
}

void TDEToolBar::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

#include "tdetoolbar.moc"

