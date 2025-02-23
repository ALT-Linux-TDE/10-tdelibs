/***************************************************************************
 *   Copyright (C) 2005 by Sean Harmer <sh@rama.homelinux.org>             *
 *                         Till Adam <adam@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Library General Public License as       *
 *   published by  the Free Software Foundation; either version 2 of the   *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.             *
 ***************************************************************************/


#include "kacleditwidget.h"
#include "kacleditwidget_p.h"

#ifdef USE_POSIX_ACL

#include <tqpainter.h>
#include <tqptrlist.h>
#include <tqvbox.h>
#include <tqhbox.h>
#include <tqpushbutton.h>
#include <tqvbuttongroup.h>
#include <tqradiobutton.h>
#include <tqcombobox.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqwidgetstack.h>
#include <tqheader.h>

#include <tdelocale.h>
#include <tdefileitem.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kdialogbase.h>

#ifdef HAVE_ACL_LIBACL_H
# include <acl/libacl.h>
#endif
extern "C" {
#include <pwd.h>
#include <grp.h>
}
#include <assert.h>

#include "images.h"

static struct {
    const char* label;
    const char* pixmapName;
    TQPixmap* pixmap;
} s_itemAttributes[] = {
    { I18N_NOOP( "Owner" ), "user-grey", 0 },
    { I18N_NOOP( "Owning Group" ), "group-grey", 0 },
    { I18N_NOOP( "Others" ), "others-grey", 0 },
    { I18N_NOOP( "Mask" ), "mask", 0 },
    { I18N_NOOP( "Named User" ), "user", 0 },
    { I18N_NOOP( "Named Group" ), "group", 0 },
};

KACLEditWidget::KACLEditWidget( TQWidget *parent, const char *name )
   :TQWidget( parent, name )
{
    TQHBox *hbox = new TQHBox( parent );
    hbox->setSpacing(  KDialog::spacingHint() );
    m_listView = new KACLListView( hbox, "acl_listview" );
    connect( m_listView, TQ_SIGNAL( selectionChanged() ),
            this, TQ_SLOT( slotUpdateButtons() ) );
    TQVBox *vbox = new TQVBox( hbox );
    vbox->setSpacing(  KDialog::spacingHint() );
    m_AddBtn = new TQPushButton( i18n( "Add Entry..." ), vbox, "add_entry_button" );
    connect( m_AddBtn, TQ_SIGNAL( clicked() ), m_listView, TQ_SLOT( slotAddEntry() ) );
    m_EditBtn = new TQPushButton( i18n( "Edit Entry..." ), vbox, "edit_entry_button" );
    connect( m_EditBtn, TQ_SIGNAL( clicked() ), m_listView, TQ_SLOT( slotEditEntry() ) );
    m_DelBtn = new TQPushButton( i18n( "Delete Entry" ), vbox, "delete_entry_button" );
    connect( m_DelBtn, TQ_SIGNAL( clicked() ), m_listView, TQ_SLOT( slotRemoveEntry() ) );
    TQWidget *spacer = new TQWidget( vbox );
    spacer->setSizePolicy( TQSizePolicy::Minimum, TQSizePolicy::Expanding );
    slotUpdateButtons();
}

void KACLEditWidget::slotUpdateButtons()
{
    bool atLeastOneIsNotDeletable = false;
    bool atLeastOneIsNotAllowedToChangeType = false;
    int selectedCount = 0;
    TQListViewItemIterator it( m_listView, TQListViewItemIterator::Selected );
    while ( KACLListViewItem *item = dynamic_cast<KACLListViewItem*>( it.current() ) ) {
        ++it; ++selectedCount;
        if ( !item->isDeletable() )
            atLeastOneIsNotDeletable = true;
        if ( !item->isAllowedToChangeType() )
            atLeastOneIsNotAllowedToChangeType = true;
    }
    m_EditBtn->setEnabled( selectedCount && !atLeastOneIsNotAllowedToChangeType );
    m_DelBtn->setEnabled( selectedCount && !atLeastOneIsNotDeletable );
}

KACL KACLEditWidget::getACL() const
{
  return m_listView->getACL();
}

KACL KACLEditWidget::getDefaultACL() const
{
  return m_listView->getDefaultACL();
}

void KACLEditWidget::setACL( const KACL &acl )
{
  return m_listView->setACL( acl );
}

void KACLEditWidget::setDefaultACL( const KACL &acl )
{
  return m_listView->setDefaultACL( acl );
}

void KACLEditWidget::setAllowDefaults( bool value )
{
    m_listView->setAllowDefaults( value );
}

void KACLEditWidget::setReadOnly( bool on )
{
    m_listView->setEnabled( !on );
    m_AddBtn->setEnabled( !on );
    if ( !on )
      slotUpdateButtons();
}

KACLListViewItem::KACLListViewItem( TQListView* parent,
                                    KACLListView::EntryType _type,
                                    unsigned short _value, bool defaults,
                                    const TQString& _qualifier )
 : TDEListViewItem( parent, parent->lastItem() ), // we want to append
   type( _type ), value( _value ), isDefault( defaults ),
   qualifier( _qualifier ), isPartial( false )
{
    m_pACLListView = dynamic_cast<KACLListView*>( parent );
    repaint();
}


KACLListViewItem::~ KACLListViewItem()
{

}

TQString KACLListViewItem::key( int, bool ) const
{
    TQString key;
    if ( !isDefault )
        key = "A";
    else
        key = "B";
    switch ( type )
    {
        case KACLListView::User:
            key += "A";
            break;
        case KACLListView::Group:
            key += "B";
            break;
        case KACLListView::Others:
            key += "C";
            break;
        case KACLListView::Mask:
            key += "D";
            break;
        case KACLListView::NamedUser:
            key += "E" + text( 1 );
            break;
        case KACLListView::NamedGroup:
            key += "F" + text( 1 );
            break;
        default:
            key += text( 0 );
            break;
    }
    return key;
}

void KACLListViewItem::paintCell( TQPainter* p, const TQColorGroup &cg,
                                  int column, int width, int alignment )
{
    TQColorGroup mycg = cg;
    if ( isDefault ) {
        mycg.setColor( TQColorGroup::Text, TQColor( 0, 0, 255 ) );
    }
    if ( isPartial ) {
        TQFont font = p->font();
        font.setItalic( true );
        mycg.setColor( TQColorGroup::Text, TQColor( 100, 100, 100 ) );
        p->setFont( font );
    }
    TDEListViewItem::paintCell( p, mycg, column, width, alignment );

    KACLListViewItem *below =0;
    if ( itemBelow() )
        below = static_cast<KACLListViewItem*>( itemBelow() );
    const bool lastUser = type == KACLListView::NamedUser && below && below->type == KACLListView::NamedGroup;
    const bool lastNonDefault = !isDefault && below && below->isDefault;
    if ( type == KACLListView::Mask || lastUser || lastNonDefault )
    {
        p->setPen( TQPen( TQt::gray, 0, TQPen::DotLine ) );
        if ( type == KACLListView::Mask )
            p->drawLine( 0, 0, width - 1, 0 );
        p->drawLine( 0, height() - 1, width - 1, height() - 1 );
    }
}


void KACLListViewItem::updatePermPixmaps()
{
    unsigned int partialPerms = value;

    if ( value & ACL_READ )
        setPixmap( 2, m_pACLListView->getYesPixmap() );
    else if ( partialPerms & ACL_READ )
        setPixmap( 2, m_pACLListView->getYesPartialPixmap() );
    else
        setPixmap( 2, TQPixmap() );

    if ( value & ACL_WRITE )
        setPixmap( 3, m_pACLListView->getYesPixmap() );
    else if ( partialPerms & ACL_WRITE )
        setPixmap( 3, m_pACLListView->getYesPartialPixmap() );
    else
        setPixmap( 3, TQPixmap() );

    if ( value & ACL_EXECUTE )
        setPixmap( 4, m_pACLListView->getYesPixmap() );
    else if ( partialPerms & ACL_EXECUTE )
        setPixmap( 4, m_pACLListView->getYesPartialPixmap() );
    else
        setPixmap( 4, TQPixmap() );
}

void KACLListViewItem::repaint()
{
    int idx = 0;
    switch ( type )
    {
      case KACLListView::User:
          idx = KACLListView::OWNER_IDX;
            break;
        case KACLListView::Group:
          idx = KACLListView::GROUP_IDX;
            break;
        case KACLListView::Others:
          idx = KACLListView::OTHERS_IDX;
            break;
        case KACLListView::Mask:
          idx = KACLListView::MASK_IDX;
            break;
        case KACLListView::NamedUser:
          idx = KACLListView::NAMED_USER_IDX;
            break;
        case KACLListView::NamedGroup:
          idx = KACLListView::NAMED_GROUP_IDX;
            break;
        default:
          idx = KACLListView::OWNER_IDX;
            break;
    }
    setText( 0, i18n(s_itemAttributes[idx].label) );
    setPixmap( 0, *s_itemAttributes[idx].pixmap );
    if ( isDefault )
        setText( 0, text( 0 ) + i18n( " (Default)" ) );
    setText( 1, qualifier );
    // Set the pixmaps for which of the perms are set
    updatePermPixmaps();
}

void KACLListViewItem::calcEffectiveRights()
{
    TQString strEffective = TQString( "---" );

    // Do we need to worry about the mask entry? It applies to named users,
    // owning group, and named groups
    if ( m_pACLListView->hasMaskEntry()
            && ( type == KACLListView::NamedUser
              || type == KACLListView::Group
              || type == KACLListView::NamedGroup ) 
            && !isDefault )
    {

        strEffective[0] = ( m_pACLListView->maskPermissions() & value & ACL_READ ) ? 'r' : '-';
        strEffective[1] = ( m_pACLListView->maskPermissions() & value & ACL_WRITE ) ? 'w' : '-';
        strEffective[2] = ( m_pACLListView->maskPermissions() & value & ACL_EXECUTE ) ? 'x' : '-';
/*
        // What about any partial perms?
        if ( maskPerms & partialPerms & ACL_READ || // Partial perms on entry
             maskPartialPerms & perms & ACL_READ || // Partial perms on mask
             maskPartialPerms & partialPerms & ACL_READ ) // Partial perms on mask and entry
            strEffective[0] = 'R';
        if ( maskPerms & partialPerms & ACL_WRITE || // Partial perms on entry
             maskPartialPerms & perms & ACL_WRITE || // Partial perms on mask
             maskPartialPerms & partialPerms & ACL_WRITE ) // Partial perms on mask and entry
            strEffective[1] = 'W';
        if ( maskPerms & partialPerms & ACL_EXECUTE || // Partial perms on entry
             maskPartialPerms & perms & ACL_EXECUTE || // Partial perms on mask
             maskPartialPerms & partialPerms & ACL_EXECUTE ) // Partial perms on mask and entry
            strEffective[2] = 'X';
*/
    }
    else
    {
        // No, the effective value are just the value in this entry
        strEffective[0] = ( value & ACL_READ ) ? 'r' : '-';
        strEffective[1] = ( value & ACL_WRITE ) ? 'w' : '-';
        strEffective[2] = ( value & ACL_EXECUTE ) ? 'x' : '-';

        /*
        // What about any partial perms?
        if ( partialPerms & ACL_READ )
            strEffective[0] = 'R';
        if ( partialPerms & ACL_WRITE )
            strEffective[1] = 'W';
        if ( partialPerms & ACL_EXECUTE )
            strEffective[2] = 'X';
            */
    }
    setText( 5, strEffective );
}

bool KACLListViewItem::isDeletable() const
{
    bool isMaskAndDeletable = false;
    if (type == KACLListView::Mask ) {
        if ( !isDefault &&  m_pACLListView->maskCanBeDeleted() )
            isMaskAndDeletable = true;
        else if ( isDefault &&  m_pACLListView->defaultMaskCanBeDeleted() )
            isMaskAndDeletable = true;
    }
    return type != KACLListView::User &&
           type != KACLListView::Group &&
           type != KACLListView::Others &&
           ( type != KACLListView::Mask || isMaskAndDeletable );
}

bool KACLListViewItem::isAllowedToChangeType() const
{
    return type != KACLListView::User &&
           type != KACLListView::Group &&
           type != KACLListView::Others &&
           type != KACLListView::Mask;
}

void KACLListViewItem::togglePerm( acl_perm_t perm )
{
    value ^= perm; // Toggle the perm
    if ( type == KACLListView::Mask && !isDefault ) {
        m_pACLListView->setMaskPermissions( value );
    }
    calcEffectiveRights();
    updatePermPixmaps();
/*
    // If the perm is in the partial perms then remove it. i.e. Once
    // a user changes a partial perm it then applies to all selected files.
    if ( m_pEntry->m_partialPerms & perm )
        m_pEntry->m_partialPerms ^= perm;

    m_pEntry->setPartialEntry( false );
    // Make sure that all entries have their effective rights calculated if
    // we are changing the ACL_MASK entry.
    if ( type == Mask )
    {
        m_pACLListView->setMaskPartialPermissions( m_pEntry->m_partialPerms );
        m_pACLListView->setMaskPermissions( value );
        m_pACLListView->calculateEffectiveRights();
    }
*/
}



EditACLEntryDialog::EditACLEntryDialog( KACLListView *listView, KACLListViewItem *item,
                                        const TQStringList &users,
                                        const TQStringList &groups,
                                        const TQStringList &defaultUsers,
                                        const TQStringList &defaultGroups,
                                        int allowedTypes, int allowedDefaultTypes,
                                        bool allowDefaults )
      : KDialogBase( listView, "edit_entry_dialog", true,
              i18n( "Edit ACL Entry" ), KDialogBase::Ok|KDialogBase::Cancel,
              KDialogBase::Ok, false ), 
        m_listView( listView ), m_item( item ), m_users( users ), m_groups( groups ),
        m_defaultUsers( defaultUsers ), m_defaultGroups( defaultGroups ),
        m_allowedTypes( allowedTypes ), m_allowedDefaultTypes( allowedDefaultTypes ),
        m_defaultCB( 0 )
{
    TQWidget *page = new TQWidget(  this );
    setMainWidget( page );
    TQVBoxLayout *mainLayout = new TQVBoxLayout( page, 0, spacingHint(), "mainLayout" );
    m_buttonGroup = new TQVButtonGroup( i18n("Entry Type"), page, "bg" );

    if ( allowDefaults ) {
        m_defaultCB = new TQCheckBox( i18n("Default for new files in this folder"), page, "defaultCB" );
        mainLayout->addWidget( m_defaultCB );
        connect( m_defaultCB, TQ_SIGNAL( toggled( bool ) ),
                 this, TQ_SLOT( slotUpdateAllowedUsersAndGroups() ) );
        connect( m_defaultCB, TQ_SIGNAL( toggled( bool ) ),
                 this, TQ_SLOT( slotUpdateAllowedTypes() ) );

    }

    mainLayout->addWidget( m_buttonGroup );

    TQRadioButton *ownerType = new TQRadioButton( i18n("Owner"), m_buttonGroup, "ownerType" );
    m_buttonGroup->insert( ownerType, KACLListView::User );
    TQRadioButton *owningGroupType = new TQRadioButton( i18n("Owning Group"), m_buttonGroup, "owningGroupType" );
    m_buttonGroup->insert( owningGroupType, KACLListView::Group );
    TQRadioButton *othersType = new TQRadioButton( i18n("Others"), m_buttonGroup, "othersType" );
    m_buttonGroup->insert( othersType, KACLListView::Others );
    TQRadioButton *maskType = new TQRadioButton( i18n("Mask"), m_buttonGroup, "maskType" );
    m_buttonGroup->insert( maskType, KACLListView::Mask );
    TQRadioButton *namedUserType = new TQRadioButton( i18n("Named User"), m_buttonGroup, "namesUserType" );
    m_buttonGroup->insert( namedUserType, KACLListView::NamedUser );
    TQRadioButton *namedGroupType = new TQRadioButton( i18n("Named Group"), m_buttonGroup, "namedGroupType" );
    m_buttonGroup->insert( namedGroupType, KACLListView::NamedGroup );

    connect( m_buttonGroup, TQ_SIGNAL( clicked( int ) ),
             this, TQ_SLOT( slotSelectionChanged( int ) ) );

    m_widgetStack = new TQWidgetStack( page );
    mainLayout->addWidget( m_widgetStack );

    TQHBox *usersBox = new TQHBox( m_widgetStack );
    m_widgetStack->addWidget( usersBox, KACLListView::NamedUser );

    TQHBox *groupsBox = new TQHBox( m_widgetStack );
    m_widgetStack->addWidget( groupsBox, KACLListView::NamedGroup );

    TQLabel *usersLabel = new TQLabel( i18n( "User: " ), usersBox );
    m_usersCombo = new TQComboBox( false, usersBox, "users" );
    usersLabel->setBuddy( m_usersCombo );

    TQLabel *groupsLabel = new TQLabel( i18n( "Group: " ), groupsBox );
    m_groupsCombo = new TQComboBox( false, groupsBox, "groups" );
    groupsLabel->setBuddy( m_groupsCombo );

    if ( m_item ) {
        m_buttonGroup->setButton( m_item->type );
        if ( m_defaultCB )
            m_defaultCB->setChecked( m_item->isDefault );
        slotUpdateAllowedTypes();
        slotSelectionChanged( m_item->type );
        slotUpdateAllowedUsersAndGroups();
        if ( m_item->type == KACLListView::NamedUser ) {
            m_usersCombo->setCurrentText( m_item->qualifier );
        } else if ( m_item->type == KACLListView::NamedGroup ) {
            m_groupsCombo->setCurrentText( m_item->qualifier );
        }
    } else {
        // new entry, preselect "named user", arguably the most common one
        m_buttonGroup->setButton( KACLListView::NamedUser );
        slotUpdateAllowedTypes();
        slotSelectionChanged( KACLListView::NamedUser );
        slotUpdateAllowedUsersAndGroups();
    }
    incInitialSize(  TQSize( 100, 0 ) );
}

void EditACLEntryDialog::slotUpdateAllowedTypes()
{
    int allowedTypes = m_allowedTypes;
    if ( m_defaultCB && m_defaultCB->isChecked() ) {
        allowedTypes = m_allowedDefaultTypes;
    }
    for ( int i=1; i < KACLListView::AllTypes; i=i*2 ) {
        if ( allowedTypes & i )
            m_buttonGroup->find( i )->show();
        else
            m_buttonGroup->find( i )->hide();
    }
}

void EditACLEntryDialog::slotUpdateAllowedUsersAndGroups()
{
    const TQString oldUser = m_usersCombo->currentText();
    const TQString oldGroup = m_groupsCombo->currentText();
    m_usersCombo->clear();
    m_groupsCombo->clear();
    if ( m_defaultCB && m_defaultCB->isChecked() ) {
        m_usersCombo->insertStringList( m_defaultUsers );
        if ( m_defaultUsers.find( oldUser ) != m_defaultUsers.end() )
            m_usersCombo->setCurrentText( oldUser );
        m_groupsCombo->insertStringList( m_defaultGroups );
        if ( m_defaultGroups.find( oldGroup ) != m_defaultGroups.end() )
            m_groupsCombo->setCurrentText( oldGroup );
    } else {
        m_usersCombo->insertStringList( m_users );
        if ( m_users.find( oldUser ) != m_users.end() )
            m_usersCombo->setCurrentText( oldUser );
        m_groupsCombo->insertStringList( m_groups );
        if ( m_groups.find( oldGroup ) != m_groups.end() )
            m_groupsCombo->setCurrentText( oldGroup );
    }
}
void EditACLEntryDialog::slotOk()
{
    KACLListView::EntryType type = static_cast<KACLListView::EntryType>( m_buttonGroup->selectedId() );

    TQString qualifier;
    if ( type == KACLListView::NamedUser )
      qualifier = m_usersCombo->currentText();
    if ( type == KACLListView::NamedGroup )
      qualifier = m_groupsCombo->currentText();

    if ( !m_item ) {
        m_item = new KACLListViewItem( m_listView, type, ACL_READ | ACL_WRITE | ACL_EXECUTE, false, qualifier );
    } else {
        m_item->type = type;
        m_item->qualifier = qualifier;
    }
    if ( m_defaultCB )
        m_item->isDefault = m_defaultCB->isChecked();
    m_item->repaint();

    KDialogBase::slotOk();
}

void EditACLEntryDialog::slotSelectionChanged( int id )
{
    switch ( id ) {
        case KACLListView::User:
        case KACLListView::Group:
        case KACLListView::Others:
        case KACLListView::Mask:
            m_widgetStack->setEnabled( false );
            break;
        case KACLListView::NamedUser:
            m_widgetStack->setEnabled( true );
            m_widgetStack->raiseWidget( KACLListView::NamedUser );
            break;
        case KACLListView::NamedGroup:
            m_widgetStack->setEnabled( true );
            m_widgetStack->raiseWidget( KACLListView::NamedGroup );
            break;
        default:
            break;
    }
}


KACLListView::KACLListView( TQWidget* parent, const char* name )
 : TDEListView( parent, name ),
   m_hasMask( false ), m_allowDefaults( false )
{
    // Add the columns
    addColumn( i18n( "Type" ) );
    addColumn( i18n( "Name" ) );
    addColumn( i18n( "read permission", "r" ) );
    addColumn( i18n( "write permission", "w" ) );
    addColumn( i18n( "execute permission", "x" ) );
    addColumn( i18n( "Effective" ) );

    header()->setClickEnabled( false );

    // Load the avatars
    for ( int i=0; i < LAST_IDX; ++i ) {
        s_itemAttributes[i].pixmap = new TQPixmap( qembed_findImage( s_itemAttributes[i].pixmapName ) );
    }
    m_yesPixmap = new TQPixmap( qembed_findImage( "yes" ) );
    m_yesPartialPixmap = new TQPixmap( qembed_findImage( "yespartial" ) );

    setSelectionMode( TQListView::Extended );

    // fill the lists of all legal users and groups
    struct passwd *user = 0;
    setpwent();
    while ( ( user = getpwent() ) != 0 ) {
       m_allUsers << TQString::fromLatin1( user->pw_name );
    }
    endpwent();

    struct group *gr = 0;
    setgrent();
    while ( ( gr = getgrent() ) != 0 ) {
       m_allGroups << TQString::fromLatin1( gr->gr_name );
    }
    endgrent();
    m_allUsers.sort();
    m_allGroups.sort();
}


KACLListView::~KACLListView()
{
    for ( int i=0; i < LAST_IDX; ++i ) {
       delete s_itemAttributes[i].pixmap;
    }
    delete m_yesPixmap;
    delete m_yesPartialPixmap;
}

TQStringList KACLListView::allowedUsers( bool defaults, KACLListViewItem *allowedItem )
{
    TQStringList allowedUsers = m_allUsers;
    TQListViewItemIterator it( this );
    while ( it.current() ) {
        const KACLListViewItem *item = static_cast<const KACLListViewItem*>( *it );
        ++it;
        if ( !item->type == NamedUser || item->isDefault != defaults ) continue;
        if ( allowedItem && item == allowedItem && allowedItem->isDefault == defaults ) continue;
        allowedUsers.remove( item->qualifier );
    }
    return allowedUsers;
}

TQStringList KACLListView::allowedGroups( bool defaults, KACLListViewItem *allowedItem )
{
    TQStringList allowedGroups = m_allGroups;
    TQListViewItemIterator it( this );
    while ( it.current() ) {
        const KACLListViewItem *item = static_cast<const KACLListViewItem*>( *it );
        ++it;
        if ( !item->type == NamedGroup || item->isDefault != defaults ) continue;
        if ( allowedItem && item == allowedItem && allowedItem->isDefault == defaults ) continue;
        allowedGroups.remove( item->qualifier );
    }
    return allowedGroups;
}

void KACLListView::fillItemsFromACL( const KACL &pACL, bool defaults )
{
    // clear out old entries of that ilk
    TQListViewItemIterator it( this );
    while ( KACLListViewItem *item = static_cast<KACLListViewItem*>( it.current() ) ) {
        ++it;
        if ( item->isDefault == defaults )
            delete item;
    }
    KACLListViewItem *item =
        new KACLListViewItem( this, User, pACL.ownerPermissions(), defaults );

    item = new KACLListViewItem( this, Group, pACL.owningGroupPermissions(), defaults );

    item = new KACLListViewItem( this, Others, pACL.othersPermissions(), defaults );

    bool hasMask = false;
    unsigned short mask = pACL.maskPermissions( hasMask );
    if ( hasMask ) {
        item = new KACLListViewItem( this, Mask, mask, defaults );
    }

    // read all named user entries
    const ACLUserPermissionsList &userList =  pACL.allUserPermissions();
    ACLUserPermissionsConstIterator itu = userList.begin();
    while ( itu != userList.end() ) {
        new KACLListViewItem( this, NamedUser, (*itu).second, defaults, (*itu).first );
        ++itu;
    }

    // and now all named groups
    const ACLUserPermissionsList &groupList =  pACL.allGroupPermissions();
    ACLUserPermissionsConstIterator itg = groupList.begin();
    while ( itg != groupList.end() ) {
        new KACLListViewItem( this, NamedGroup, (*itg).second, defaults, (*itg).first );
        ++itg;
    }
}

void KACLListView::setACL( const KACL &acl )
{
    if ( !acl.isValid() ) return;
    // Remove any entries left over from displaying a previous ACL
    m_ACL = acl;
    fillItemsFromACL( m_ACL );

    m_mask = acl.maskPermissions( m_hasMask );
    calculateEffectiveRights();
}

void KACLListView::setDefaultACL( const KACL &acl )
{
    if ( !acl.isValid() ) return;
    m_defaultACL = acl;
    fillItemsFromACL( m_defaultACL, true );
    calculateEffectiveRights();
}

KACL KACLListView::itemsToACL( bool defaults ) const
{
    KACL newACL( 0 );
    bool atLeastOneEntry = false;
    ACLUserPermissionsList users;
    ACLGroupPermissionsList groups;
    TQListViewItemIterator it( const_cast<KACLListView*>( this ) );
    while ( TQListViewItem* qlvi = it.current() ) {
        ++it;
        const KACLListViewItem* item = static_cast<KACLListViewItem*>( qlvi );
        if ( item->isDefault != defaults ) continue;
        atLeastOneEntry = true;
        switch ( item->type ) {
            case User:
                newACL.setOwnerPermissions( item->value );
                break;
            case Group:
                newACL.setOwningGroupPermissions( item->value );
                break;
            case Others:
                newACL.setOthersPermissions( item->value );
                break;
            case Mask:
                newACL.setMaskPermissions( item->value );
                break;
            case NamedUser:
                users.append( qMakePair( item->text( 1 ), item->value ) );
                break;
            case NamedGroup:
                groups.append( qMakePair( item->text( 1 ), item->value ) );
                break;
            default:
                break;
        }
    }
    if ( atLeastOneEntry ) {
        newACL.setAllUserPermissions( users );
        newACL.setAllGroupPermissions( groups );
        if ( newACL.isValid() )
            return newACL;
    }
    return KACL();
}

KACL KACLListView::getACL()
{
    return itemsToACL( false );
}


KACL KACLListView::getDefaultACL()
{
    return itemsToACL( true );
}

void KACLListView::contentsMousePressEvent( TQMouseEvent * e )
{
    TQListViewItem *clickedItem = itemAt( contentsToViewport(  e->pos() ) );
    if ( !clickedItem ) return;
    // if the click is on an as yet unselected item, select it first
    if ( !clickedItem->isSelected() )
        TDEListView::contentsMousePressEvent( e );

    if ( !currentItem() ) return;
    int column = header()->sectionAt( e->x() );
    acl_perm_t perm;
    switch ( column )
    {
        case 2:
            perm = ACL_READ;
            break;
        case 3:
            perm = ACL_WRITE;
            break;
        case 4:
            perm = ACL_EXECUTE;
            break;
        default:
            return TDEListView::contentsMousePressEvent( e );
    }
    KACLListViewItem* referenceItem = static_cast<KACLListViewItem*>( clickedItem );
    unsigned short referenceHadItSet = referenceItem->value & perm;
    TQListViewItemIterator it( this );
    while ( KACLListViewItem* item = static_cast<KACLListViewItem*>( it.current() ) ) {
        ++it;
        if ( !item->isSelected() ) continue;
        // toggle those with the same value as the clicked item, leave the others
        if ( referenceHadItSet == ( item->value & perm ) )
            item->togglePerm( perm );
    }
}

void KACLListView::entryClicked( TQListViewItem* pItem, const TQPoint& /*pt*/, int col )
{
    if ( !pItem ) return;

    TQListViewItemIterator it( this );
    while ( KACLListViewItem* item = static_cast<KACLListViewItem*>( it.current() ) ) {
        ++it;
        if ( !item->isSelected() ) continue;
        switch ( col )
        {
            case 2:
                item->togglePerm( ACL_READ );
                break;
            case 3:
                item->togglePerm( ACL_WRITE );
                break;
            case 4:
                item->togglePerm( ACL_EXECUTE );
                break;

            default:
                ; // Do nothing
        }
    }
    /*
    // Has the user changed one of the required entries in a default ACL?
    if ( m_pACL->aclType() == ACL_TYPE_DEFAULT &&
    ( col == 2 || col == 3 || col == 4 ) &&
    ( pACLItem->entryType() == ACL_USER_OBJ ||
    pACLItem->entryType() == ACL_GROUP_OBJ ||
    pACLItem->entryType() == ACL_OTHER ) )
    {
    // Mark the required entries as no longer being partial entries.
    // That is, they will get applied to all selected directories.
    KACLListViewItem* pUserObj = findACLEntryByType( this, ACL_USER_OBJ );
    pUserObj->entry()->setPartialEntry( false );

    KACLListViewItem* pGroupObj = findACLEntryByType( this, ACL_GROUP_OBJ );
    pGroupObj->entry()->setPartialEntry( false );

    KACLListViewItem* pOther = findACLEntryByType( this, ACL_OTHER );
    pOther->entry()->setPartialEntry( false );

    update();
    }
     */
}


void KACLListView::calculateEffectiveRights()
{
    TQListViewItemIterator it( this );
    KACLListViewItem* pItem;
    while ( ( pItem = dynamic_cast<KACLListViewItem*>( it.current() ) ) != 0 )
    {
        ++it;
        pItem->calcEffectiveRights();
    }
}


unsigned short KACLListView::maskPermissions() const
{
  return m_mask;
}


void KACLListView::setMaskPermissions( unsigned short maskPerms )
{
    m_mask = maskPerms;
    calculateEffectiveRights();
}


acl_perm_t KACLListView::maskPartialPermissions() const
{
  //  return m_pMaskEntry->m_partialPerms;
  return 0;
}


void KACLListView::setMaskPartialPermissions( acl_perm_t /*maskPartialPerms*/ )
{
    //m_pMaskEntry->m_partialPerms = maskPartialPerms;
    calculateEffectiveRights();
}

bool KACLListView::hasDefaultEntries() const
{
    TQListViewItemIterator it( const_cast<KACLListView*>( this ) );
    while ( it.current() ) {
        const KACLListViewItem *item = static_cast<const KACLListViewItem*>( it.current() );
        ++it;
        if ( item->isDefault ) return true;
    }
    return false;
}

const KACLListViewItem* KACLListView::findDefaultItemByType( EntryType type ) const
{
    return findItemByType( type, true );
}

const KACLListViewItem* KACLListView::findItemByType( EntryType type, bool defaults ) const
{
    TQListViewItemIterator it( const_cast<KACLListView*>( this ) );
    while ( it.current() ) {
        const KACLListViewItem *item = static_cast<const KACLListViewItem*>( it.current() );
        ++it;
        if ( item->isDefault == defaults && item->type == type ) {
            return item;
        }
    }
    return 0;
}


unsigned short KACLListView::calculateMaskValue( bool defaults ) const
{
    // KACL auto-adds the relevant maks entries, so we can simply query
    bool dummy;
    return itemsToACL( defaults ).maskPermissions( dummy );
}

void KACLListView::slotAddEntry()
{
    int allowedTypes = NamedUser | NamedGroup;
    if ( !m_hasMask )
        allowedTypes |= Mask;
    int allowedDefaultTypes = NamedUser | NamedGroup;
    if ( !findDefaultItemByType( Mask ) )
        allowedDefaultTypes |=  Mask;
    if ( !hasDefaultEntries() )
        allowedDefaultTypes |= User | Group;
    EditACLEntryDialog dlg( this, 0,
                            allowedUsers( false ), allowedGroups( false ),
                            allowedUsers( true ), allowedGroups( true ),
                            allowedTypes, allowedDefaultTypes, m_allowDefaults );
    dlg.exec();
    KACLListViewItem *item = dlg.item();
    if ( !item ) return; // canceled
    if ( item->type == Mask && !item->isDefault ) {
        m_hasMask = true;
        m_mask = item->value;
    }
    if ( item->isDefault && !hasDefaultEntries() ) {
        // first default entry, fill in what is needed
        if ( item->type != User ) {
            unsigned short v = findDefaultItemByType( User )->value;
            new KACLListViewItem( this, User, v, true );
        }
        if ( item->type != Group ) {
            unsigned short v = findDefaultItemByType( Group )->value;
            new KACLListViewItem( this, Group, v, true );
        }
        if ( item->type != Others ) {
            unsigned short v = findDefaultItemByType( Others )->value;
            new KACLListViewItem( this, Others, v, true );
        }
    }
    const KACLListViewItem *defaultMaskItem = findDefaultItemByType( Mask );
    if ( item->isDefault && !defaultMaskItem ) {
        unsigned short v = calculateMaskValue( true );
        new KACLListViewItem( this, Mask, v, true );
    }
    if ( !item->isDefault && !m_hasMask &&
            ( item->type == Group
              || item->type == NamedUser
              || item->type == NamedGroup ) ) {
        // auto-add a mask entry
        unsigned short v = calculateMaskValue( false );
        new KACLListViewItem( this, Mask, v, false );
        m_hasMask = true;
        m_mask = v;
    }
    calculateEffectiveRights();
    sort();
    setCurrentItem( item );
    // TQListView doesn't seem to emit, in this case, and we need to update 
    // the buttons...
    if ( childCount() == 1 ) 
        emit currentChanged( item );
}

void KACLListView::slotEditEntry()
{
    TQListViewItem * current = currentItem();
    if ( !current ) return;
    KACLListViewItem *item = static_cast<KACLListViewItem*>( current );
    int allowedTypes = item->type | NamedUser | NamedGroup;
    bool itemWasMask = item->type == Mask;
    if ( !m_hasMask || itemWasMask )
        allowedTypes |= Mask;
    int allowedDefaultTypes = item->type | NamedUser | NamedGroup;
    if ( !findDefaultItemByType( Mask ) )
        allowedDefaultTypes |=  Mask;
    if ( !hasDefaultEntries() )
        allowedDefaultTypes |= User | Group;

    EditACLEntryDialog dlg( this, item,
                            allowedUsers( false, item ), allowedGroups( false, item ),
                            allowedUsers( true, item ), allowedGroups( true, item ),
                            allowedTypes, allowedDefaultTypes, m_allowDefaults );
    dlg.exec();
    if ( itemWasMask && item->type != Mask ) {
        m_hasMask = false;
        m_mask = 0;
    }
    if ( !itemWasMask && item->type == Mask ) {
        m_mask = item->value;
        m_hasMask = true;
    }
    calculateEffectiveRights();
    sort();
}

void KACLListView::slotRemoveEntry()
{
    TQListViewItemIterator it( this, TQListViewItemIterator::Selected );
    while ( it.current() ) {
        KACLListViewItem *item = static_cast<KACLListViewItem*>( it.current() );
        ++it;
        /* First check if it's a mask entry and if so, make sure that there is
         * either no name user or group entry, which means the mask can be 
         * removed, or don't remove it, but reset it. That is allowed. */
        if ( item->type == Mask ) {
            bool itemWasDefault = item->isDefault;
            if ( !itemWasDefault && maskCanBeDeleted() ) {
                m_hasMask= false;
                m_mask = 0;
                delete item;
            } else if ( itemWasDefault && defaultMaskCanBeDeleted() ) {
                delete item;
            } else {
                item->value = 0;
                item->repaint();
            }
            if ( !itemWasDefault )
                calculateEffectiveRights();
        } else {
            // for the base permissions, disable them, which is what libacl does
            if ( !item->isDefault &&
                    ( item->type == User
                      || item->type == Group
                      || item->type == Others ) ) {
                item->value = 0;
                item->repaint();
            } else {
                delete item;
            }
        }
    }
}

bool KACLListView::maskCanBeDeleted() const
{
   return !findItemByType( NamedUser ) && !findItemByType( NamedGroup );
}

bool KACLListView::defaultMaskCanBeDeleted() const
{
    return !findDefaultItemByType( NamedUser ) && !findDefaultItemByType( NamedGroup );
}

#include "kacleditwidget.moc"
#include "kacleditwidget_p.moc"
#endif
