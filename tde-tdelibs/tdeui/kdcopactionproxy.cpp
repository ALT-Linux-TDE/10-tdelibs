/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>

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

#include "kdcopactionproxy.h"

#include <dcopclient.h>
#include <tdeapplication.h>
#include <tdeaction.h>
#include <kdebug.h>
#include <kdcoppropertyproxy.h>

#include <ctype.h>

class KDCOPActionProxy::KDCOPActionProxyPrivate
{
public:
  KDCOPActionProxyPrivate()
  {
  }
  ~KDCOPActionProxyPrivate()
  {
  }

  TDEActionCollection *m_actionCollection;
  DCOPObject *m_parent;
  TQCString m_prefix;
  int m_pos;
};

KDCOPActionProxy::KDCOPActionProxy( TDEActionCollection *actionCollection, DCOPObject *parent )
{
  init( actionCollection, parent );
}

KDCOPActionProxy::KDCOPActionProxy( DCOPObject *parent )
{
  init( 0, parent );
}

void KDCOPActionProxy::init( TDEActionCollection *collection, DCOPObject *parent )
{
  d = new KDCOPActionProxyPrivate;
  d->m_actionCollection = collection;
  d->m_parent = parent;
  d->m_prefix = parent->objId() + "/action/";
  d->m_pos = d->m_prefix.length();
}

KDCOPActionProxy::~KDCOPActionProxy()
{
  delete d;
}

TQValueList<TDEAction *>KDCOPActionProxy::actions() const
{
  if ( !d->m_actionCollection )
    return TQValueList<TDEAction *>();

  return d->m_actionCollection->actions();
}

TDEAction *KDCOPActionProxy::action( const char *name ) const
{
  if ( !d->m_actionCollection )
    return 0;

  return d->m_actionCollection->action( name );
}

TQCString KDCOPActionProxy::actionObjectId( const TQCString &name ) const
{
  return d->m_prefix + name;
}

TQMap<TQCString,DCOPRef> KDCOPActionProxy::actionMap( const TQCString &appId ) const
{
  TQMap<TQCString,DCOPRef> res;

  TQCString id = appId;
  if ( id.isEmpty() )
    id = kapp->dcopClient()->appId();

  TQValueList<TDEAction *> lst = actions();
  TQValueList<TDEAction *>::ConstIterator it = lst.begin();
  TQValueList<TDEAction *>::ConstIterator end = lst.end();
  for (; it != end; ++it )
    res.insert( (*it)->name(), DCOPRef( id, actionObjectId( (*it)->name() ) ) );

  return res;
}

bool KDCOPActionProxy::process( const TQCString &obj, const TQCString &fun, const TQByteArray &data,
                                TQCString &replyType, TQByteArray &replyData )
{
  if ( obj.left( d->m_pos ) != d->m_prefix )
    return false;

  TDEAction *act = action( obj.mid( d->m_pos ) );
  if ( !act )
    return false;

  return processAction( obj, fun, data, replyType, replyData, act );
}

bool KDCOPActionProxy::processAction( const TQCString &, const TQCString &fun, const TQByteArray &data,
                                      TQCString &replyType, TQByteArray &replyData, TDEAction *action )
{
  if ( fun == "activate()" )
  {
    replyType = "void";
    action->activate();
    return true;
  }

  if ( fun == "isPlugged()" )
  {
    replyType = "bool";
    TQDataStream reply( replyData, IO_WriteOnly );
    reply << (TQ_INT8)action->isPlugged();
    return true;
  }

  if ( fun == "functions()" )
  {
    TQValueList<TQCString> res;
    res << "QCStringList functions()";
    res << "void activate()";
    res << "bool isPlugged()";

    res += KDCOPPropertyProxy::functions( action );

    replyType = "QCStringList";
    TQDataStream reply( replyData, IO_WriteOnly );
    reply << res;
    return true;
  }

  return KDCOPPropertyProxy::processPropertyRequest( fun, data, replyType, replyData, action );
}

void KDCOPActionProxy::virtual_hook( int id, void* data )
{ DCOPObjectProxy::virtual_hook( id, data ); }

