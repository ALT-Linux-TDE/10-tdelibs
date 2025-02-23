/* This file is part of the KDE libraries

   Copyright (c) 2000,2001 Dawit Alemayehu <adawit@kde.org>
   Copyright (c) 2000,2001 Carsten Pfeiffer <pfeiffer@kde.org>
   Copyright (c) 2000 Stefan Schimanski <1Stein@gmx.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <tqclipboard.h>
#include <tqlistbox.h>
#include <tqpopupmenu.h>
#include <tqapplication.h>

#include <tdecompletionbox.h>
#include <kcursor.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <tdelistviewsearchline.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <knotifyclient.h>
#include <kpixmapprovider.h>
#include <tdestdaccel.h>
#include <kurl.h>
#include <kurldrag.h>

#include <kdebug.h>

#include "kcombobox.h"

#include <stdlib.h> // getenv

class KComboBox::KComboBoxPrivate
{
public:
    KComboBoxPrivate() : klineEdit(0L)
    {
    }
    ~KComboBoxPrivate()
    {
    }

    KLineEdit *klineEdit;
};

KComboBox::KComboBox( TQWidget *parent, const char *name )
    : TQComboBox( parent, name ), d(new KComboBoxPrivate)
{
    init();
}

KComboBox::KComboBox( bool rw, TQWidget *parent, const char *name )
    : TQComboBox( rw, parent, name ), d(new KComboBoxPrivate)
{
    init();

    if ( rw )
    {
      KLineEdit *edit = new KLineEdit( this, "combo lineedit" );
      setLineEdit( edit );
    }
}

KComboBox::~KComboBox()
{
    delete d;
}

void KComboBox::init()
{
    // Permanently set some parameters in the parent object.
    TQComboBox::setAutoCompletion( false );

    // Enable context menu by default if widget
    // is editable.
    setContextMenuEnabled( true );
}


bool KComboBox::contains( const TQString& _text ) const
{
    if ( _text.isEmpty() )
        return false;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i )
    {
        if ( text(i) == _text )
            return true;
    }
    return false;
}

void KComboBox::setAutoCompletion( bool autocomplete )
{
    if ( d->klineEdit )
    {
        if ( autocomplete )
        {
            d->klineEdit->setCompletionMode( TDEGlobalSettings::CompletionAuto );
            setCompletionMode( TDEGlobalSettings::CompletionAuto );
        }
        else
        {
            d->klineEdit->setCompletionMode( TDEGlobalSettings::completionMode() );
            setCompletionMode( TDEGlobalSettings::completionMode() );
        }
    }
}

void KComboBox::setContextMenuEnabled( bool showMenu )
{
    if( d->klineEdit )
        d->klineEdit->setContextMenuEnabled( showMenu );
}


void KComboBox::setURLDropsEnabled( bool enable )
{
    if ( d->klineEdit )
        d->klineEdit->setURLDropsEnabled( enable );
}

bool KComboBox::isURLDropsEnabled() const
{
    return d->klineEdit && d->klineEdit->isURLDropsEnabled();
}


void KComboBox::setCompletedText( const TQString& text, bool marked )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedText( text, marked );
}

void KComboBox::setCompletedText( const TQString& text )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedText( text );
}

void KComboBox::makeCompletion( const TQString& text )
{
    if( d->klineEdit )
        d->klineEdit->makeCompletion( text );

    else // read-only combo completion
    {
        if( text.isNull() || !listBox() )
            return;

        const int index = listBox()->index( listBox()->findItem( text ) );
        if( index >= 0 )
            setCurrentItem( index );
    }
}

void KComboBox::rotateText( TDECompletionBase::KeyBindingType type )
{
    if ( d->klineEdit )
        d->klineEdit->rotateText( type );
}

// not needed anymore
bool KComboBox::eventFilter( TQObject* o, TQEvent* ev )
{
    return TQComboBox::eventFilter( o, ev );
}

void KComboBox::setTrapReturnKey( bool grab )
{
    if ( d->klineEdit )
        d->klineEdit->setTrapReturnKey( grab );
    else
        tqWarning("KComboBox::setTrapReturnKey not supported with a non-KLineEdit.");
}

bool KComboBox::trapReturnKey() const
{
    return d->klineEdit && d->klineEdit->trapReturnKey();
}


void KComboBox::setEditURL( const KURL& url )
{
    TQComboBox::setEditText( url.prettyURL() );
}

void KComboBox::insertURL( const KURL& url, int index )
{
    TQComboBox::insertItem( url.prettyURL(), index );
}

void KComboBox::insertURL( const TQPixmap& pixmap, const KURL& url, int index )
{
    TQComboBox::insertItem( pixmap, url.prettyURL(), index );
}

void KComboBox::changeURL( const KURL& url, int index )
{
    TQComboBox::changeItem( url.prettyURL(), index );
}

void KComboBox::changeURL( const TQPixmap& pixmap, const KURL& url, int index )
{
    TQComboBox::changeItem( pixmap, url.prettyURL(), index );
}

void KComboBox::setCompletedItems( const TQStringList& items )
{
    if ( d->klineEdit )
        d->klineEdit->setCompletedItems( items );
}

TDECompletionBox * KComboBox::completionBox( bool create )
{
    if ( d->klineEdit )
        return d->klineEdit->completionBox( create );
    return 0;
}

// TQWidget::create() turns off mouse-Tracking which would break auto-hiding
void KComboBox::create( WId id, bool initializeWindow, bool destroyOldWindow )
{
    TQComboBox::create( id, initializeWindow, destroyOldWindow );
    KCursor::setAutoHideCursor( lineEdit(), true, true );
}

void KComboBox::wheelEvent( TQWheelEvent *ev )
{
    // Not necessary anymore
    TQComboBox::wheelEvent( ev );
}

void KComboBox::setLineEdit( TQLineEdit *edit )
{
    if ( !editable() && edit &&
         !qstrcmp( edit->className(), "TQLineEdit" ) )
    {
        // uic generates code that creates a read-only KComboBox and then
        // calls combo->setEditable( true ), which causes TQComboBox to set up
        // a dumb TQLineEdit instead of our nice KLineEdit.
        // As some KComboBox features rely on the KLineEdit, we reject
        // this order here.
        delete edit;
        edit = new KLineEdit( this, "combo edit" );
    }

    TQComboBox::setLineEdit( edit );
    d->klineEdit = dynamic_cast<KLineEdit*>( edit );
    setDelegate( d->klineEdit );

    // Connect the returnPressed signal for both Q[K]LineEdits'
    if (edit)
        connect( edit, TQ_SIGNAL( returnPressed() ), TQ_SIGNAL( returnPressed() ));

    if ( d->klineEdit )
    {
        // someone calling KComboBox::setEditable( false ) destroys our
        // lineedit without us noticing. And TDECompletionBase::delegate would
        // be a dangling pointer then, so prevent that. Note: only do this
        // when it is a KLineEdit!
        connect( edit, TQ_SIGNAL( destroyed() ), TQ_SLOT( lineEditDeleted() ));

        connect( d->klineEdit, TQ_SIGNAL( returnPressed( const TQString& )),
                 TQ_SIGNAL( returnPressed( const TQString& ) ));

        connect( d->klineEdit, TQ_SIGNAL( completion( const TQString& )),
                 TQ_SIGNAL( completion( const TQString& )) );

        connect( d->klineEdit, TQ_SIGNAL( substringCompletion( const TQString& )),
                 TQ_SIGNAL( substringCompletion( const TQString& )) );

        connect( d->klineEdit,
                 TQ_SIGNAL( textRotation( TDECompletionBase::KeyBindingType )),
                 TQ_SIGNAL( textRotation( TDECompletionBase::KeyBindingType )) );

        connect( d->klineEdit,
                 TQ_SIGNAL( completionModeChanged( TDEGlobalSettings::Completion )),
                 TQ_SIGNAL( completionModeChanged( TDEGlobalSettings::Completion)));

        connect( d->klineEdit,
                 TQ_SIGNAL( aboutToShowContextMenu( TQPopupMenu * )),
                 TQ_SIGNAL( aboutToShowContextMenu( TQPopupMenu * )) );

        connect( d->klineEdit,
                 TQ_SIGNAL( completionBoxActivated( const TQString& )),
                 TQ_SIGNAL( activated( const TQString& )) );
    }
}

void KComboBox::setCurrentItem( const TQString& item, bool insert, int index )
{
    int sel = -1;

    const int itemCount = count();
    for (int i = 0; i < itemCount; ++i)
    {
        if (text(i) == item)
        {
            sel = i;
            break;
        }
    }

    if (sel == -1 && insert)
    {
        insertItem(item, index);
        if (index >= 0)
            sel = index;
        else
            sel = count() - 1;
    }
    setCurrentItem(sel);
}

void KComboBox::lineEditDeleted()
{
    // yes, we need those ugly casts due to the multiple inheritance
    // sender() is guaranteed to be a KLineEdit (see the connect() to the
    // destroyed() signal
    const TDECompletionBase *base = static_cast<const TDECompletionBase*>( static_cast<const KLineEdit*>( sender() ));

    // is it our delegate, that is destroyed?
    if ( base == delegate() )
        setDelegate( 0L );
}


// *********************************************************************
// *********************************************************************

class KHistoryCombo::KHistoryComboPrivate
{
public:
    KHistoryComboPrivate() : bHistoryEditorEnabled(false)
    {
    }
    ~KHistoryComboPrivate()
    {
    }

    bool bHistoryEditorEnabled;
};

// we are always read-write
KHistoryCombo::KHistoryCombo( TQWidget *parent, const char *name )
    : KComboBox( true, parent, name ), d(new KHistoryComboPrivate)
{
    init( true ); // using completion
}

// we are always read-write
KHistoryCombo::KHistoryCombo( bool useCompletion,
                              TQWidget *parent, const char *name )
    : KComboBox( true, parent, name ), d(new KHistoryComboPrivate)
{
    init( useCompletion );
}

void KHistoryCombo::init( bool useCompletion )
{
    // Set a default history size to something reasonable, Qt sets it to INT_MAX by default
    setMaxCount( 50 );

    if ( useCompletion )
        completionObject()->setOrder( TDECompletion::Weighted );

    setInsertionPolicy( NoInsertion );
    myIterateIndex = -1;
    myRotated = false;
    myPixProvider = 0L;

    // obey HISTCONTROL setting
    TQCString histControl = getenv("HISTCONTROL");
    if ( histControl == "ignoredups" || histControl == "ignoreboth" )
        setDuplicatesEnabled( false );

    connect( this, TQ_SIGNAL(aboutToShowContextMenu(TQPopupMenu*)),
             TQ_SLOT(addContextMenuItems(TQPopupMenu*)) );
    connect( this, TQ_SIGNAL( activated(int) ), TQ_SLOT( slotReset() ));
    connect( this, TQ_SIGNAL( returnPressed(const TQString&) ), TQ_SLOT(slotReset()));
}

KHistoryCombo::~KHistoryCombo()
{
    delete myPixProvider;
}

void KHistoryCombo::setHistoryItems( TQStringList items,
                                     bool setCompletionList )
{
    KComboBox::clear();

    // limit to maxCount()
    const int itemCount = items.count(); 
    const int toRemove = itemCount - maxCount();

    if (toRemove >= itemCount) {
        items.clear();
    } else {
        for (int i = 0; i < toRemove; ++i) 
            items.pop_front();
    }

    insertItems( items );

    if ( setCompletionList && useCompletion() ) {
        // we don't have any weighting information here ;(
        TDECompletion *comp = completionObject();
        comp->setOrder( TDECompletion::Insertion );
        comp->setItems( items );
        comp->setOrder( TDECompletion::Weighted );
    }

    clearEdit();
}

TQStringList KHistoryCombo::historyItems() const
{
    TQStringList list;
    const int itemCount = count();
    for ( int i = 0; i < itemCount; ++i )
        list.append( text( i ) );

    return list;
}

void KHistoryCombo::clearHistory()
{
    const TQString temp = currentText();
    KComboBox::clear();
    if ( useCompletion() )
        completionObject()->clear();
    setEditText( temp );
}

void KHistoryCombo::addContextMenuItems( TQPopupMenu* menu )
{
    if ( menu )
    {
        menu->insertSeparator();
        if (d->bHistoryEditorEnabled) {
           int idedit = menu->insertItem( SmallIconSet("edit"), i18n("&Edit History..."), this, TQ_SLOT( slotEdit()) );
           menu->setItemEnabled(idedit, count());
        }
        int id = menu->insertItem( SmallIconSet("history_clear"), i18n("Clear &History"), this, TQ_SLOT( slotClear()));
        if (!count())
           menu->setItemEnabled(id, false);
    }
}

void KHistoryCombo::addToHistory( const TQString& item )
{
    if ( item.isEmpty() || (count() > 0 && item == text(0) )) {
        return;
    }

    bool wasCurrent = false;
    // remove all existing items before adding
    if ( !duplicatesEnabled() ) {
        int i = 0;
        int itemCount = count();
        while ( i < itemCount ) {
            if ( text( i ) == item ) {
                if ( !wasCurrent )
                  wasCurrent = ( i == currentItem() );
                removeItem( i );
                --itemCount;
            } else {
                ++i;
            }
        }
    }

    // now add the item
    if ( myPixProvider )
        insertItem( myPixProvider->pixmapFor(item, TDEIcon::SizeSmall), item, 0);
    else
        insertItem( item, 0 );

    if ( wasCurrent )
        setCurrentItem( 0 );

    const bool useComp = useCompletion();

    const int last = count() - 1; // last valid index
    const int mc = maxCount();
    const int stopAt = TQMAX(mc, 0);

    for (int rmIndex = last; rmIndex >= stopAt; --rmIndex) {
        // remove the last item, as long as we are longer than maxCount()
        // remove the removed item from the completionObject if it isn't
        // anymore available at all in the combobox.
        const TQString rmItem = text( rmIndex );
        removeItem( rmIndex );
        if ( useComp && !contains( rmItem ) )
            completionObject()->removeItem( rmItem );
    }

    if ( useComp )
        completionObject()->addItem( item );
}

bool KHistoryCombo::removeFromHistory( const TQString& item )
{
    if ( item.isEmpty() )
        return false;

    bool removed = false;
    const TQString temp = currentText();
    int i = 0;
    int itemCount = count();
    while ( i < itemCount ) {
        if ( item == text( i ) ) {
            removed = true;
            removeItem( i );
            --itemCount;
        } else {
            ++i;
        }
    }

    if ( removed && useCompletion() )
        completionObject()->removeItem( item );

    setEditText( temp );
    return removed;
}

void KHistoryCombo::rotateUp()
{
    // save the current text in the lineedit
    if ( myIterateIndex == -1 )
        myText = currentText();

    ++myIterateIndex;

    // skip duplicates/empty items
    const int last = count() - 1; // last valid index
    const TQString currText = currentText();

    while ( myIterateIndex < last &&
            (currText == text( myIterateIndex ) ||
             text( myIterateIndex ).isEmpty()) )
        ++myIterateIndex;

    if ( myIterateIndex >= count() ) {
        myRotated = true;
        myIterateIndex = -1;

        // if the typed text is the same as the first item, skip the first
        if ( count() > 0 && myText == text(0) )
            myIterateIndex = 0;

        setEditText( myText );
    }
    else
        setEditText( text( myIterateIndex ));
}

void KHistoryCombo::rotateDown()
{
    // save the current text in the lineedit
    if ( myIterateIndex == -1 )
        myText = currentText();

    --myIterateIndex;

    const TQString currText = currentText();
    // skip duplicates/empty items
    while ( myIterateIndex >= 0 &&
            (currText == text( myIterateIndex ) ||
             text( myIterateIndex ).isEmpty()) )
        --myIterateIndex;


    if ( myIterateIndex < 0 ) {
        if ( myRotated && myIterateIndex == -2 ) {
            myRotated = false;
            myIterateIndex = count() - 1;
            setEditText( text(myIterateIndex) );
        }
        else { // bottom of history
            if ( myIterateIndex == -2 ) {
                KNotifyClient::event( (int)winId(), KNotifyClient::notification,
                                      i18n("No further item in the history."));
            }

            myIterateIndex = -1;
            if ( currentText() != myText )
                setEditText( myText );
        }
    }
    else
        setEditText( text( myIterateIndex ));

}

void KHistoryCombo::keyPressEvent( TQKeyEvent *e )
{
    KKey event_key( e );

    // going up in the history, rotating when reaching TQListBox::count()
    if ( TDEStdAccel::rotateUp().contains(event_key) )
        rotateUp();

    // going down in the history, no rotation possible. Last item will be
    // the text that was in the lineedit before Up was called.
    else if ( TDEStdAccel::rotateDown().contains(event_key) )
        rotateDown();
    else
        KComboBox::keyPressEvent( e );
}

void KHistoryCombo::wheelEvent( TQWheelEvent *ev )
{
    // Pass to poppable listbox if it's up
    TQListBox* const lb = listBox();
    if ( lb && lb->isVisible() )
    {
        TQApplication::sendEvent( lb, ev );
        return;
    }
    // Otherwise make it change the text without emitting activated
    if ( ev->delta() > 0 ) {
        rotateUp();
    } else {
        rotateDown();
    }
    ev->accept();
}

void KHistoryCombo::slotReset()
{
    myIterateIndex = -1;
    myRotated = false;
}


void KHistoryCombo::setPixmapProvider( KPixmapProvider *prov )
{
    if ( myPixProvider == prov )
        return;

    delete myPixProvider;
    myPixProvider = prov;

    // re-insert all the items with/without pixmap
    // I would prefer to use changeItem(), but that doesn't honor the pixmap
    // when using an editable combobox (what we do)
    if ( count() > 0 ) {
        TQStringList items( historyItems() );
        clear();
        insertItems( items );
    }
}

void KHistoryCombo::insertItems( const TQStringList& items )
{
    TQStringList::ConstIterator it = items.constBegin();
    const TQStringList::ConstIterator itEnd = items.constEnd();

    while ( it != itEnd ) {
        const TQString item = *it;
        if ( !item.isEmpty() ) { // only insert non-empty items
            if ( myPixProvider )
                insertItem( myPixProvider->pixmapFor(item, TDEIcon::SizeSmall),
                            item );
            else
                insertItem( item );
        }
        ++it;
    }
}

void KHistoryCombo::slotClear()
{
    clearHistory();
    emit cleared();
}

void KHistoryCombo::slotEdit()
{
    KHistoryComboEditor dlg( historyItems(), this );
    connect( &dlg, TQ_SIGNAL( removeFromHistory(const TQString&) ), TQ_SLOT( slotRemoveFromHistory(const TQString&)) );
    dlg.exec();
}

void KHistoryCombo::slotRemoveFromHistory(const TQString &entry)
{
    removeFromHistory(entry);
    emit removed(entry);
}

void KHistoryCombo::setHistoryEditorEnabled( bool enable )
{
    d->bHistoryEditorEnabled = enable;
}

bool KHistoryCombo::isHistoryEditorEnabled() const
{
    return d->bHistoryEditorEnabled;
}

void KComboBox::virtual_hook( int id, void* data )
{ TDECompletionBase::virtual_hook( id, data ); }

void KHistoryCombo::virtual_hook( int id, void* data )
{ KComboBox::virtual_hook( id, data ); }

void KHistoryComboEditor::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

KHistoryComboEditor::KHistoryComboEditor( const TQStringList& entries, TQWidget *parent )
: KDialogBase( parent, "khistorycomboeditor", true, i18n( "History Editor" ),
    KDialogBase::Close | KDialogBase::User1, KDialogBase::User1, true,
    KGuiItem( i18n( "&Delete Entry" ), "edit-delete") ), d(0)
{
    TQVBox* box = new TQVBox( this );
    box->setSpacing( KDialog::spacingHint() );
    setMainWidget( box );

    new TQLabel( i18n( "This dialog allows you to delete unwanted history items." ), box );

    // Add searchline
    TQHBox* searchbox = new TQHBox( box );
    searchbox->setSpacing( KDialog::spacingHint() );

    TQToolButton *clearSearch = new TQToolButton(searchbox);
    clearSearch->setTextLabel(i18n("Clear Search"), true);
    clearSearch->setIconSet(SmallIconSet(TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase"));
    TQLabel* slbl = new TQLabel(i18n("&Search:"), searchbox);
    TDEListViewSearchLine* listViewSearch = new TDEListViewSearchLine(searchbox);
    slbl->setBuddy(listViewSearch);
    connect(clearSearch, TQ_SIGNAL(pressed()), listViewSearch, TQ_SLOT(clear()));

    // Add ListView
    m_pListView = new TDEListView( box );
    listViewSearch->setListView(m_pListView);
    m_pListView->setAllColumnsShowFocus(true);
    m_pListView->header()->hide();
    m_pListView->addColumn("");
    m_pListView->setRenameable( 0 );

    box->setStretchFactor( m_pListView, 1 );

    TQStringList newlist = entries;
    for ( TQStringList::Iterator it = newlist.begin(); it != newlist.end(); ++it ) {
        new TQListViewItem( m_pListView, *it );
    }

    m_pListView->setMinimumSize( m_pListView->sizeHint() );

    connect( m_pListView, TQ_SIGNAL( selectionChanged( TQListViewItem * ) ),
             this, TQ_SLOT( slotSelectionChanged( TQListViewItem * ) ) );

    enableButton( KDialogBase::User1, false );

    resize( sizeHint() );
}

KHistoryComboEditor::~KHistoryComboEditor()
{
}

void KHistoryComboEditor::slotUser1() // Delete button
{
    TQListViewItem *item = m_pListView->selectedItem();

    if ( item ) {
       emit removeFromHistory( item->text(0) );
       m_pListView->takeItem( item );
       enableButton( KDialogBase::User1, false );
    }
}

void KHistoryComboEditor::slotSelectionChanged( TQListViewItem * item )
{
    enableButton( KDialogBase::User1, item );
}

#include "kcombobox.moc"
