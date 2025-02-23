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
#ifndef TDERESOURCES_MANAGERIMPL_H
#define TDERESOURCES_MANAGERIMPL_H

#include <tqstring.h>
#include <tqptrlist.h>
#include <tqdict.h>

#include "manageriface.h"
#include <tderesources/resource.h>

class TDEConfig;

namespace KRES {

class Resource;
class Factory;
class ManagerNotifier;

/**
  @internal

  Do not use this class directly. Use ResourceManager instead
*/
class TDERESOURCES_EXPORT ManagerImpl : virtual public ManagerIface
{
  public:
    ManagerImpl( ManagerNotifier *, const TQString &family );
    ~ManagerImpl();

    void readConfig( TDEConfig * );
    void writeConfig( TDEConfig * );

    void add( Resource *resource );
    void remove( Resource *resource );
    void change( Resource *resource );

    Resource *standardResource();
    void setStandardResource( Resource *resource );

    void setActive( Resource *resource, bool active );

    Resource::List *resourceList();

    TQPtrList<Resource> resources();

    // Get only active or passive resources
    TQPtrList<Resource> resources( bool active );

    TQStringList resourceNames();

    static TQString defaultConfigFile( const TQString &family );

  private:
    // dcop calls
    void dcopKResourceAdded( TQString managerId, TQString resourceId );
    void dcopKResourceModified( TQString managerId, TQString resourceId );
    void dcopKResourceDeleted( TQString managerId, TQString resourceId );

  private:
    void createStandardConfig();

    Resource *readResourceConfig( const TQString& identifier, bool checkActive );
    void writeResourceConfig( Resource *resource, bool checkActive );

    void removeResource( Resource *resource );
    Resource *getResource( Resource *resource );
    Resource *getResource( const TQString& identifier );

    ManagerNotifier *mNotifier;
    TQString mFamily;
    TDEConfig *mConfig;
    TDEConfig *mStdConfig;
    Resource *mStandard;
    Factory *mFactory;
    Resource::List mResources;
    TQString mId;
    bool mConfigRead;

    class ManagerImplPrivate;
    ManagerImplPrivate *d;
};

}

#endif
