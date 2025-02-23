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
#ifndef __kdcoppropertyproxy_h__
#define __kdcoppropertyproxy_h__

#include <tqobject.h>
#include <tqcstring.h>
#include <tqvaluelist.h>
#include "tdelibs_export.h"

class KDCOPPropertyProxyPrivate;
/**
 * The KDCOPPropertyProxy class provides an easy way to publish Qt properties of a
 * TQObject through DCOP.
 *
 * The class provides DCOP equivalents for the Qt property methods setProperty() ,
 * property() and propertyNames() and also provides automatic set/get methods for
 * the properties of a TQObject. That means for example if your object provides a
 * TQString property called foo , then KDCOPPropertyProxy translates DCOP calls
 * "setFoo( TQString )" and "TQString foo()" automatically into the corresponding
 * setProperty/property calls.
 */
class TDECORE_EXPORT KDCOPPropertyProxy
{
public:
  /**
   * Convenience constructor. Use it if you want to use this class as object, in contrary
   * to using the static methods of this class and providing a TQObject argument for each
   * call.
   */
  KDCOPPropertyProxy( TQObject *object );
  /**
   * Destructor.
   */
  ~KDCOPPropertyProxy();

  /**
   * Convenience method, when using this class as object. See documentation of the constructor and
   * static isPropertyRequest method.
   */
  bool isPropertyRequest( const TQCString &fun );

  /**
   * Convenience method, when using this class as object. See documentation of the constructor and
   * static processPropertyRequest method.
   */
  bool processPropertyRequest( const TQCString &fun, const TQByteArray &data, TQCString &replyType,
                               TQByteArray &replyData );

  /**
   * Convenience method, when using this class as object. See documentation of the constructor and
   * static functions method.
   */
  TQValueList<TQCString> functions();

  /**
   * Returns a semicolon-separated list of functions understood by the PropertyProxy for the given
   * TQObject argument.
   *
   * Returns "property(TQCString);setProperty(TQCString,TQVariant);propertyNames();" plus set/get
   * methods for the properties of the given object argument.
   *
   * @see DCOPObject::functions()
   */
  static TQValueList<TQCString> functions( TQObject *object );

  /**
   * Returns true if the method request in the fun argument matches the signature of the three standard
   * property methods or set/get methods for the properties of the object argument.
   *
   * Use this method in your own DCOPObject dispatcher to check if the DCOP request is a property
   * request which can be handled by this class.
   */
  static bool isPropertyRequest( const TQCString &fun, TQObject *object );

  /**
   * Processes the given DCOP method request by translating the request into a setProperty/property call
   * on the given TQObject argument.
   */
  static bool processPropertyRequest( const TQCString &fun, const TQByteArray &data, TQCString &replyType,
                                      TQByteArray &replyData, TQObject *object );

private:
  static bool decodePropertyRequestInternal( const TQCString &fun, TQObject *object, bool &set,
                                             TQCString &propName, TQCString &arg );

  KDCOPPropertyProxyPrivate *d;
};

#endif
