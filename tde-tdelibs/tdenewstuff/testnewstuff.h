/*
    This file is part of KOrganizer.

    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef TESTNEWSTUFF_H
#define TESTNEWSTUFF_H

#include <stdlib.h>

#include <tqpushbutton.h>

#include <tdeapplication.h>
#include <kdebug.h>

#include "knewstuff.h"

class TestNewStuff : public TDENewStuff
{
  public:
    TestNewStuff() : TDENewStuff( "korganizer/calendar" ) {}
    
    bool install( const TQString &fileName );
    
    bool createUploadFile( const TQString &fileName );
};

class MyWidget : public TQWidget
{
    TQ_OBJECT
  public:
    MyWidget();
    ~MyWidget();
    
  public slots:
    void upload();
    void download();

  private:
    TDENewStuff *mNewStuff;
};

#endif
