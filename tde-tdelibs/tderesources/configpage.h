/*
    This file is part of libtderesources.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>
    Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef TDERESOURCES_CONFIGPAGE_H
#define TDERESOURCES_CONFIGPAGE_H

#include <ksharedptr.h>
#include <tqstringlist.h>
#include <tqwidget.h>

#include "manager.h"

class KComboBox;
class TDEListView;

class TQListViewItem;
class TQPushButton;

namespace KRES {

class TDERESOURCES_EXPORT ResourcePageInfo : public TDEShared
{
  public:
    ResourcePageInfo();
    ~ResourcePageInfo();
    Manager<Resource> *mManager;
    TDEConfig *mConfig;
};

class Resource;
class ConfigViewItem;

class TDERESOURCES_EXPORT ConfigPage : public TQWidget, public ManagerObserver<Resource>
{
  TQ_OBJECT

  public:
    ConfigPage( TQWidget *parent = 0, const char *name = 0 );
    virtual ~ConfigPage();

    void load();
    void save();
    virtual void defaults();

  public slots:
    void slotFamilyChanged( int );
    void slotAdd();
    void slotRemove();
    void slotEdit();
    void slotStandard();
    void slotSelectionChanged();

  public:
    // From ManagerObserver<Resource>
    virtual void resourceAdded( Resource *resource );
    virtual void resourceModified( Resource *resource );
    virtual void resourceDeleted( Resource *resource );

  protected:
    ConfigViewItem *findItem( Resource *resource );

  protected slots:
    void slotItemClicked( TQListViewItem * );

  signals:
    void changed( bool );

  private:
    void loadManager( const TQString& family );
    void saveResourceSettings();

    Manager<Resource>* mCurrentManager;
    TDEConfig* mCurrentConfig;
    TDEConfig* mConfig;
    TQString mFamily;
    TQStringList mFamilyMap;
    TQValueList<TDESharedPtr<ResourcePageInfo> > mInfoMap;

    KComboBox* mFamilyCombo;
    TDEListView* mListView;
    TQPushButton* mAddButton;
    TQPushButton* mRemoveButton;
    TQPushButton* mEditButton;
    TQPushButton* mStandardButton;

    TQListViewItem* mLastItem;
};

}

#endif
