/*
    This file is part of libtdeabc.
    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>

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

#ifndef RESOURCESQLCONFIG_H
#define RESOURCESQLCONFIG_H

#include "resourceconfigwidget.h"

class KLineEdit;
class TQSpinBox;

namespace TDEABC {

class ResourceSqlConfig : public ResourceConfigWidget
{ 
  TQ_OBJECT

public:
  ResourceSqlConfig( TQWidget* parent = 0, const char* name = 0 );

public slots:
  void loadSettings( TDEConfig *config );
  void saveSettings( TDEConfig *config );

private:
  KLineEdit* mUser;
  KLineEdit* mPassword;
  KLineEdit* mDbName;
  KLineEdit* mHost;
  TQSpinBox* mPort;
};

}
#endif
