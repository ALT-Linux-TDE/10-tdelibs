/*
    This file is part of libtderesources.

    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef TDERESOURCES_KCMTDERESOURCES_H
#define TDERESOURCES_KCMTDERESOURCES_H

#include <tdecmodule.h>

class TDERESOURCES_EXPORT KCMTDEResources : public TDECModule
{
  TQ_OBJECT

  public:
    KCMTDEResources( TQWidget *parent, const char *name, const TQStringList& );

    void load();
    void save();
    void defaults();

  private:
    KRES::ConfigPage *mConfigPage;
};

#endif
