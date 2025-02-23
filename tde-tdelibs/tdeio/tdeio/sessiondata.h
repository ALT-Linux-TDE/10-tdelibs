/* This file is part of the KDE project
   Copyright (C) 2000 Dawit Alemayehu <adawit@kde.org

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License (LGPL) as published by the Free Software Foundation;
   either version 2 of the License, or (at your option) any
   later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 51 Franklin Street,
   Fifth Floor, Boston, MA 02110-1301, USA.
*/

#ifndef __TDEIO_SESSIONDATA_H
#define __TDEIO_SESSIONDATA_H

#include <tqobject.h>
#include <tdeio/global.h>

namespace TDEIO  {

class SlaveConfig;


/**
 * @internal
 */
class TDEIO_EXPORT SessionData : public TQObject
{
    TQ_OBJECT

public:
    SessionData();
    ~SessionData();

    virtual void configDataFor( TDEIO::MetaData &configData, const TQString &proto,
                                const TQString &host );
    virtual void reset();

    /// @since 3.1
    struct AuthData;
public slots:
    void slotAuthData( const TQCString&, const TQCString&, bool );
    void slotDelAuthData( const TQCString& );

private:
    class AuthDataList;
    friend class AuthDataList;
    AuthDataList* authData;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SessionDataPrivate;
    SessionDataPrivate* d;
};

} // namespace

#endif
