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

#ifndef KABC_DISTRIBUTIONLISTDIALOG_H
#define KABC_DISTRIBUTIONLISTDIALOG_H

#include <tqwidget.h>

#include <kdialogbase.h>

class TQListView;
class TQComboBox;
class TQButtonGroup;

namespace TDEABC {

class AddressBook;
class DistributionListEditorWidget;
class DistributionListManager;

/**
  @short Frontend to create distribution lists
 
  Creating a new DistributionListDialog does automatically
  load all addressees and distribution lists from the config
  files. The changes will be saved when clicking the 'OK'
  button.
 
  Example:
 
  \code
  TDEABC::DistributionListDialog *dlg = new
          TDEABC::DistributionListDialog( TDEABC::StdAddressBook::self(), this );
 
  dlg->exec();
  \endcode
*/
class KABC_EXPORT DistributionListDialog : public KDialogBase
{
    TQ_OBJECT

  public:
    /**
      Constructor.

      @param ab     The addressbook, the addressees should be used from
      @param parent The parent widget
    */
    DistributionListDialog( AddressBook *ab, TQWidget *parent );

    /**
      Destructor.
    */
    virtual ~DistributionListDialog();

  private:
    DistributionListEditorWidget *mEditor;

    struct Data;
    Data *d;
};

/**
  @short Helper class
*/
class KABC_EXPORT EmailSelector : public KDialogBase
{
  public:
    EmailSelector( const TQStringList &emails, const TQString &current,
        TQWidget *parent );

    TQString selected();

    static TQString getEmail( const TQStringList &emails, const TQString &current,
        TQWidget *parent );

  private:
    TQButtonGroup *mButtonGroup;
};

/**
  @short Helper class
*/
class KABC_EXPORT DistributionListEditorWidget : public TQWidget
{
    TQ_OBJECT

  public:
    DistributionListEditorWidget( AddressBook *, TQWidget *parent );
    virtual ~DistributionListEditorWidget();

  private slots:
    void newList();
    void editList();
    void removeList();
    void addEntry();
    void removeEntry();
    void changeEmail();
    void updateEntryView();
    void updateAddresseeView();
    void updateNameCombo();
    void slotSelectionEntryViewChanged();
    void slotSelectionAddresseeViewChanged();
    void save();

  private:
    TQComboBox *mNameCombo;  
    TQLabel *mListLabel;
    TQListView *mEntryView;
    TQListView *mAddresseeView;

    AddressBook *mAddressBook;
    DistributionListManager *mManager;
    TQPushButton *mNewButton, *mEditButton, *mRemoveButton;
    TQPushButton *mChangeEmailButton, *mRemoveEntryButton, *mAddEntryButton;

    struct Data;
    Data *d;
};

}
#endif
