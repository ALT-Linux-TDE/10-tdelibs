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
#ifndef KACLEDITWIDGET_P_H
#define KACLEDITWIDGET_P_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef USE_POSIX_ACL
#include <tdelistview.h>
#include <sys/acl.h>
#include <kacl.h>
#include <tdefileitem.h>
#include <kdialogbase.h>
#include <tqpixmap.h>
#include <tqcombobox.h>

class KACLListViewItem;
class TQPushButton;
class TQVButtonGroup;
class KACLListView;
class TQWidgetStack;
class TQCheckBox;

/**
@author Sean Harmer
*/
class KACLListView : public TDEListView
{
    TQ_OBJECT
    
    friend class KACLListViewItem;
public:
    enum Types
    {
        OWNER_IDX = 0,
        GROUP_IDX,
        OTHERS_IDX,
        MASK_IDX,
        NAMED_USER_IDX,
        NAMED_GROUP_IDX,
        LAST_IDX
    };
    enum EntryType { User = 1,
                     Group = 2,
                     Others = 4,
                     Mask = 8,
                     NamedUser = 16,
                     NamedGroup = 32,
                     AllTypes = 63 };

    KACLListView( TQWidget* parent = 0, const char* name = 0 );
    ~KACLListView();

    bool hasMaskEntry() const { return m_hasMask; }
    bool hasDefaultEntries() const;
    bool allowDefaults() const { return m_allowDefaults; }
    void setAllowDefaults( bool v ) { m_allowDefaults = v; }
    unsigned short maskPermissions() const;
    void setMaskPermissions( unsigned short maskPerms );
    acl_perm_t maskPartialPermissions() const;
    void setMaskPartialPermissions( acl_perm_t maskPerms );

    bool maskCanBeDeleted() const;
    bool defaultMaskCanBeDeleted() const;

    const KACLListViewItem* findDefaultItemByType( EntryType type ) const;
    const KACLListViewItem* findItemByType( EntryType type,
                                            bool defaults = false ) const;
    unsigned short calculateMaskValue( bool defaults ) const;
    void calculateEffectiveRights();

    TQStringList allowedUsers( bool defaults, KACLListViewItem *allowedItem = 0 );
    TQStringList allowedGroups( bool defaults, KACLListViewItem *allowedItem = 0 );

    const KACL getACL() const { return getACL(); }
    KACL getACL();

    const KACL getDefaultACL() const { return getDefaultACL(); }
    KACL getDefaultACL();

    TQPixmap getYesPixmap() const { return *m_yesPixmap; }
    TQPixmap getYesPartialPixmap() const { return *m_yesPartialPixmap; }

public slots:
    void slotAddEntry();
    void slotEditEntry();
    void slotRemoveEntry();
    void setACL( const KACL &anACL );
    void setDefaultACL( const KACL &anACL );

protected slots:
    void entryClicked( TQListViewItem* pItem, const TQPoint& pt, int col );
protected:
    void contentsMousePressEvent( TQMouseEvent * e );

private:
    void fillItemsFromACL( const KACL &pACL, bool defaults = false );
    KACL itemsToACL( bool defaults ) const;

    KACL m_ACL;
    KACL m_defaultACL;
    unsigned short m_mask;
    bool m_hasMask;
    bool m_allowDefaults;
    TQStringList m_allUsers;
    TQStringList m_allGroups;
    TQPixmap* m_yesPixmap;
    TQPixmap* m_yesPartialPixmap;
};

class EditACLEntryDialog : public KDialogBase
{
    TQ_OBJECT
    
public:
    EditACLEntryDialog( KACLListView *listView, KACLListViewItem *item,
                        const TQStringList &users,
                        const TQStringList &groups,
                        const TQStringList &defaultUsers,
                        const TQStringList &defaultGroups,
                        int allowedTypes = KACLListView::AllTypes,
                        int allowedDefaultTypes = KACLListView::AllTypes,
                        bool allowDefault = false );
    KACLListViewItem* item() const { return m_item; }
public slots:
     void slotOk();
     void slotSelectionChanged( int id );
private slots:
     void slotUpdateAllowedUsersAndGroups();
     void slotUpdateAllowedTypes();
private:
     KACLListView *m_listView;
     KACLListViewItem *m_item;
     TQStringList m_users;
     TQStringList m_groups;
     TQStringList m_defaultUsers;
     TQStringList m_defaultGroups;
     int m_allowedTypes;
     int m_allowedDefaultTypes;
     TQVButtonGroup *m_buttonGroup;
     TQComboBox *m_usersCombo;
     TQComboBox *m_groupsCombo;
     TQWidgetStack *m_widgetStack;
     TQCheckBox *m_defaultCB;
};


class KACLListViewItem : public TDEListViewItem
{
public:
    KACLListViewItem( TQListView* parent, KACLListView::EntryType type,
                      unsigned short value,
                      bool defaultEntry,
                      const TQString& qualifier = TQString::null );
    virtual ~KACLListViewItem();
    virtual TQString key( int column, bool ascending ) const;

    void calcEffectiveRights();

    bool isDeletable() const;
    bool isAllowedToChangeType() const;

    void togglePerm( acl_perm_t perm );

    virtual void paintCell( TQPainter *p, const TQColorGroup &cg,
                            int column, int width, int alignment );

    void updatePermPixmaps();
    void repaint();

    KACLListView::EntryType type;
    unsigned short value;
    bool isDefault;
    TQString qualifier;
    bool isPartial;

private:
    KACLListView* m_pACLListView;
};


#endif
#endif
