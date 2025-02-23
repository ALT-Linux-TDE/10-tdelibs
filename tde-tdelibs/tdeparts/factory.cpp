/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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

#include "factory.h"
#include "part.h"

#include <tqwidget.h>

#include <tdelocale.h>
#include <tdeglobal.h>
#include <kinstance.h>
#include <assert.h>

using namespace KParts;

Factory::Factory( TQObject *parent, const char *name )
: KLibFactory( parent, name )
{
}

Factory::~Factory()
{
}

Part *Factory::createPart( TQWidget *parentWidget, const char *widgetName, TQObject *parent, const char *name, const char *classname, const TQStringList &args )
{
    Part* part = createPartObject( parentWidget, widgetName, parent, name, classname, args );
    if ( part )
	emit objectCreated( part );
    return part;
}

const TDEInstance *Factory::partInstance()
{
    QueryInstanceParams params;
    params.instance = 0;
    virtual_hook( VIRTUAL_QUERY_INSTANCE_PARAMS, &params );
    return params.instance;
}

const TDEInstance *Factory::partInstanceFromLibrary( const TQCString &libraryName )
{
    KLibrary *library = KLibLoader::self()->library( libraryName );
    if ( !library )
        return 0;
    KLibFactory *factory = library->factory();
    if ( !factory )
        return 0;
    KParts::Factory *pfactory = dynamic_cast<KParts::Factory *>( factory );
    if ( !pfactory )
        return 0;
    return pfactory->partInstance();
}

Part *Factory::createPartObject( TQWidget *, const char *, TQObject *, const char *, const char *, const TQStringList & )
{
    return 0;
}

TQObject *Factory::createObject( TQObject *parent, const char *name, const char *classname, const TQStringList &args )
{
  assert( !parent || parent->isWidgetType() );
  return createPart( static_cast<TQWidget*>( parent ), name, parent, name, classname, args );
}
#include "factory.moc"
