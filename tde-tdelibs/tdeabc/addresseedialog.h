/*
    This file is part of libtdeabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef KABC_ADDRESSEEDIALOG_H
#define KABC_ADDRESSEEDIALOG_H

#include <tqdict.h>

#include <kdialogbase.h>
#include <klineedit.h>
#include <tdelistview.h>

#include "addressbook.h"

namespace TDEABC {

/**
  @short Special ListViewItem, that is used by the AddresseeDialog.
*/
class KABC_EXPORT AddresseeItem : public TQListViewItem
{
  public:

    /**
      Type of column
      @li @p Name -  Name in Addressee
      @li @p Email - Email in Addressee
    */
    enum columns { Name = 0, Email = 1 };

    /**
      Constructor.

      @param parent    The parent listview.
      @param addressee The associated addressee.
    */
    AddresseeItem( TQListView *parent, const Addressee &addressee );

    /**
      Returns the addressee.
    */
    Addressee addressee() const { return mAddressee; }

    /**
      Method used by TQListView to sort the items.
    */
    virtual TQString key( int column, bool ascending ) const;

  private:
    Addressee mAddressee;
};

/**
  @short Dialog for selecting address book entries.

  This class provides a dialog for selecting entries from the standard KDE
  address book. Use the getAddressee() function to open a modal dialog,
  returning an address book entry.

  In the dialog you can select an entry from the list with the mouse or type in
  the first letters of the name or email address you are searching for. The
  entry matching best is automatically selected. Use double click, pressing
  return or pressing the ok button to return the selected addressee to the
  application.
*/
class KABC_EXPORT AddresseeDialog : public KDialogBase
{
    TQ_OBJECT

  public:
    /**
      Construct addressbook entry select dialog.

      @param parent parent widget
      @param multiple if true, indicates a multiple selection.
    */
    AddresseeDialog( TQWidget *parent=0, bool multiple=false );

    /**
      Destructor.
    */
    virtual ~AddresseeDialog();

    /**
      Return the address chosen.

      If it is a multiple select, this will return only the first address chosen
    */
    Addressee addressee();

    /**
      Return the list of addresses chosen
    */
    Addressee::List addressees();

    /**
      Select a single address book entry.

      Open addressee select dialog and return the entry selected by the user.
      If the user doesn't select an entry or presses cancel, the returned
      addressee is empty.
    */
    static Addressee getAddressee( TQWidget *parent );

    /**
      Select multiple address book entries.

      Open addressee select dialog and return the entries selected by the user.
      If the user doesn't select an entry or presses cancel, the returned
      addressee list is empty.
    */
    static Addressee::List getAddressees( TQWidget *parent );

  private slots:
    void selectItem( const TQString & );
    void updateEdit( TQListViewItem *item );
    void addSelected( TQListViewItem *item );
    void removeSelected();

  protected slots:
    void addressBookChanged();

  private:
    void loadAddressBook();
    void addCompletionItem( const TQString &str, TQListViewItem *item );

    bool mMultiple;

    TDEListView *mAddresseeList;
    KLineEdit *mAddresseeEdit;

    TDEListView *mSelectedList;

    AddressBook *mAddressBook;

    TQDict<TQListViewItem> mItemDict;
    TQDict<TQListViewItem> mSelectedDict;

    class AddresseeDialogPrivate;
    AddresseeDialogPrivate *d;
};

}
#endif
