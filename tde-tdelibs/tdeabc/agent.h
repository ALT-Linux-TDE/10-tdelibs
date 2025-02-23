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

#ifndef KABC_AGENT_H
#define KABC_AGENT_H

class TQDataStream;

#include <tqstring.h>

#include <tdelibs_export.h>

namespace TDEABC {

class Addressee;

/**
 * Important!!!
 *
 * At the moment the vcard format does not support saving and loading
 * this entity.
 */
class KABC_EXPORT Agent
{
  friend KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Agent & );
  friend KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Agent & );

public:

  /**
   * Consturctor. Creates an empty object.
   */
  Agent();

  /**
   * Consturctor.
   *
   * @param url  A URL that describes the position of the agent file.
   */
  Agent( const TQString &url );

  /**
   * Consturctor.
   *
   * @param addressee  The addressee object of the agent.
   */
  Agent( Addressee *addressee );

  /**
   * Destructor.
   */
  ~Agent();


  bool operator==( const Agent & ) const;
  bool operator!=( const Agent & ) const;
  Agent &operator=(  const Agent & );
  
  /**
   * Sets a URL for the location of the agent file. When using this
   * function, isIntern() will return 'false' until you use
   * setAddressee().
   *
   * @param url  The location URL of the agent file.
   */
  void setUrl( const TQString &url );

  /**
   * Sets the addressee of the agent. When using this function,
   * isIntern() will return 'true' until you use setUrl().
   *
   * @param addressee  The addressee object of the agent.
   */
  void setAddressee( Addressee *addressee );

  /**
   * Returns whether the agent is described by a URL (extern) or
   * by a addressee (intern).
   * When this method returns 'true' you can use addressee() to
   * get a Addressee object. Otherwise you can request the URL
   * of this agent by url() and load the data from that location.
   */
  bool isIntern() const;

  /**
   * Returns the location URL of this agent.
   */
  TQString url() const;

  /**
   * Returns the addressee object of this agent.
   */
  Addressee* addressee() const;

  /**
   * Returns string representation of the agent.
   */
  TQString asString() const;

private:
  Addressee *mAddressee;
  TQString mUrl;

  int mIntern;
};

KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Agent & );
KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Agent & );

}
#endif
