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
#ifndef KABC_DISTRIBUTIONLISTEDITOR_H
#define KABC_DISTRIBUTIONLISTEDITOR_H

#include <tqwidget.h>

#include <kdialogbase.h>

class TQListView;
class TQComboBox;
class TQButtonGroup;

namespace TDEABC {

class AddressBook;
class DistributionListManager;

class KABC_EXPORT EmailSelectDialog : public KDialogBase
{
  public:
    EmailSelectDialog( const TQStringList &emails, const TQString &current,
                       TQWidget *parent );
    
    TQString selected();

    static TQString getEmail( const TQStringList &emails, const TQString &current,
                             TQWidget *parent );

  private:
    TQButtonGroup *mButtonGroup;
};

/**
  @obsolete
*/
class KABC_EXPORT DistributionListEditor : public TQWidget
{
    TQ_OBJECT
  public:
    DistributionListEditor( AddressBook *, TQWidget *parent );
    virtual ~DistributionListEditor();

  private slots:
    void newList();
    void removeList();
    void addEntry();
    void removeEntry();
    void changeEmail();
    void updateEntryView();
    void updateAddresseeView();
    void updateNameCombo();
    void slotSelectionEntryViewChanged();
    void slotSelectionAddresseeViewChanged();

  private:
    TQComboBox *mNameCombo;  
    TQListView *mEntryView;
    TQListView *mAddresseeView;

    AddressBook *mAddressBook;
    DistributionListManager *mManager;
    TQPushButton *newButton, *removeButton;
    TQPushButton *changeEmailButton,*removeEntryButton,*addEntryButton;
};

}

#endif
