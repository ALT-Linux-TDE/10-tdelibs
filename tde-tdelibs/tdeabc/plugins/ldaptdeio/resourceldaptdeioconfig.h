/*
    This file is part of libtdeabc.
    Copyright (c) 2002 - 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef RESOURCELDAPCONFIG_H
#define RESOURCELDAPCONFIG_H

#include <tqmap.h>
#include <tqradiobutton.h>
#include <tqcombobox.h>
#include <tqdict.h>

#include <kdialogbase.h>
#include <tderesources/configwidget.h>
#include <tdeabc/ldif.h>
#include <tdeabc/ldapconfigwidget.h>


class TQCheckBox;
class TQPushButton;
class TQSpinBox;
class TQString;

class KComboBox;
class KLineEdit;

namespace TDEABC {

class KABC_EXPORT ResourceLDAPTDEIOConfig : public KRES::ConfigWidget
{ 
  TQ_OBJECT

  public:
    ResourceLDAPTDEIOConfig( TQWidget* parent = 0, const char* name = 0 );

  public slots:
    void loadSettings( KRES::Resource* );
    void saveSettings( KRES::Resource* );

  private slots:
    void editAttributes();
    void editCache();
  private:
    TQPushButton *mEditButton, *mCacheButton;
    LdapConfigWidget *cfg;
    TQCheckBox *mSubTree;
    TQMap<TQString, TQString> mAttributes;
    int mRDNPrefix, mCachePolicy;
    bool mAutoCache;
    TQString mCacheDst;
};

class AttributesDialog : public KDialogBase
{
  TQ_OBJECT

  public:
    AttributesDialog( const TQMap<TQString, TQString> &attributes, int rdnprefix,
                      TQWidget *parent, const char *name = 0 );
    ~AttributesDialog();

    TQMap<TQString, TQString> attributes() const;
    int rdnprefix() const;

  private slots:
    void mapChanged( int pos );

  private:
    enum { UserMap, KolabMap, NetscapeMap, EvolutionMap, OutlookMap };

    KComboBox *mMapCombo, *mRDNCombo;
    TQValueList< TQMap<TQString, TQString> > mMapList;
    TQMap<TQString, TQString> mDefaultMap;
    TQDict<KLineEdit> mLineEditDict;
    TQDict<TQString> mNameDict;
};

class OfflineDialog : public KDialogBase
{
  TQ_OBJECT

  public:
    OfflineDialog( bool autoCache, int cachePolicy, const KURL &src, 
      const TQString &dst, TQWidget *parent, const char *name = 0 );
    ~OfflineDialog();

    int cachePolicy() const;
    bool autoCache() const;

  private slots:
    void loadCache();

  private:
    KURL mSrc;
    TQString mDst;
    TQButtonGroup *mCacheGroup;
    TQCheckBox *mAutoCache;
};

}

#endif
