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
#ifndef __kdcopactionproxy_h__
#define __kdcopactionproxy_h__

#include <dcopobject.h>
#include <dcopref.h>
#include <tdelibs_export.h>

class TDEActionCollection;
class TDEAction;

/**
 * @short A proxy class publishing a DCOP interface for actions.
 *
 * The KDCOPActionProxy class provides an easy way to publish a collection of TDEAction objects
 * through DCOP. For the DCOP client the exported actions behave like full-fledged DCOP objects,
 * providing full access to the TDEAction object functionality in the server.
 *
 * This class can generate DCOP object ids for given action objects, which it automatically
 * processes, as being a DCOPObjectProxy .
 */
class TDEUI_EXPORT KDCOPActionProxy : public DCOPObjectProxy
{
public:
  /**
   * Constructs a dcop action proxy, being able to export the actions of the provided
   * TDEActionCollection through DCOP, using the parent DCOPObject's object id to
   * generate unique object ids for the actions.
   */
  KDCOPActionProxy( TDEActionCollection *actionCollection, DCOPObject *parent );
  /**
   * Use this constructor if do not want to provide the exportable actions through a
   * TDEActionCollection . You have to reimplement the virtual actions() and
   * action() methods if you use this constructor.
   */
  KDCOPActionProxy( DCOPObject *parent );
  /**
   * Destructor.
   */
  ~KDCOPActionProxy();

  /**
   * Returns a list of exportable actions. The default implementation returns a list of actions
   * provided by a TDEActionCollection, if the first constructor has been used.
   */
  virtual TQValueList<TDEAction *> actions() const;
  /**
   * Returns an action object with the given name. The default implementation queries the action object
   * from the TDEActionCollection, if the first constructor has been used.
   */
  virtual TDEAction *action( const char *name ) const;

  /**
   * Use this method to retrieve a DCOP object id for an action with the given name.
   * This class automatically takes care of processing DCOP object requests for the returned
   * object id.
   *
   * You can construct a global DCOP object referenence using DCOPRef. For example like
   * DCOPRef( kapp->dcopClient()->appId, actionProxy->actionObjectId( actionName ) );
   *
   * The action with the given name has to be available through the #action method.
   */
  virtual TQCString actionObjectId( const TQCString &name ) const;

  /**
   * Returns a map of all exported actions, with the action name as keys and a global DCOP reference
   * as data entries.
   * The appId argument is used to specify the appid component of the DCOP reference. By default the
   * global application id is used ( kapp->dcopClient()->appId() ) .
   */
  virtual TQMap<TQCString,DCOPRef> actionMap( const TQCString &appId = TQCString() ) const;

  /**
   * Internal reimplementation of DCOPObjectProxy::process .
   */
  virtual bool process( const TQCString &obj, const TQCString &fun, const TQByteArray &data,
                        TQCString &replyType, TQByteArray &replyData );

  /**
   * Called by the #process method and takes care of processing the object request for an
   * action object.
   */
  virtual bool processAction( const TQCString &obj, const TQCString &fun, const TQByteArray &data,
                              TQCString &replyType, TQByteArray &replyData, TDEAction *action );
private:
  void init( TDEActionCollection *collection, DCOPObject *parent );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KDCOPActionProxyPrivate;
  KDCOPActionProxyPrivate *d;
};

#endif
