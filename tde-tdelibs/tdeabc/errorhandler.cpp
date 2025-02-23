/*
    This file is part of libtdeabc.

    Copyright (c) 2002 Tobias Koenig <tokoe@kde.org>
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

#include <kdebug.h>
#include <tdelocale.h>
#include <tdemessagebox.h>

#include <tqapplication.h>

#include "errorhandler.h"

using namespace TDEABC;

void ConsoleErrorHandler::error( const TQString &msg )
{
  // no debug area is ok here
  kdError() << msg << endl;
}


void GUIErrorHandler::error( const TQString &msg )
{
  KMessageBox::error( 0, msg, i18n( "Error in libtdeabc" ) );
}


GuiErrorHandler::GuiErrorHandler( TQWidget *parent )
  : mParent( parent )
{
}

void GuiErrorHandler::error( const TQString &msg )
{
  if (tqApp)
    KMessageBox::error( mParent, msg );
}
