/*
    This file is part of libtderesources.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2002 Jan-Pascal van Best <janpascal@vanbest.org>

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

#ifndef TDERESOURCES_CONFIGDIALOG_H
#define TDERESOURCES_CONFIGDIALOG_H

#include <kdialogbase.h>

class KLineEdit;
class TQCheckBox;
class KButtonBox;

namespace KRES {
  class Resource;
  class ConfigWidget;

class TDERESOURCES_EXPORT ConfigDialog : public KDialogBase
{
    TQ_OBJECT
  public:
    // Resource=0: create new resource
    ConfigDialog( TQWidget *parent, const TQString& resourceFamily,
	          Resource* resource, const char *name = 0);

    void setInEditMode( bool value );

  protected slots:
    void accept();
    void setReadOnly( bool value );
    void slotNameChanged( const TQString &text);

  private:
    ConfigWidget *mConfigWidget;
    Resource* mResource;

    KLineEdit *mName;
    TQCheckBox *mReadOnly;
};

}

#endif
