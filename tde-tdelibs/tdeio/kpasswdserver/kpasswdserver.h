/*
    This file is part of the KDE Password Server

    Copyright (C) 2002 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    version 2 as published by the Free Software Foundation.

    This software is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this library; see the file COPYING. If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/
//----------------------------------------------------------------------------
//
// KDE Password Server
// $Id$

#ifndef KPASSWDSERVER_H
#define KPASSWDSERVER_H

#include <tqdict.h>
#include <tqintdict.h>

#include <dcopclient.h>
#include <tdeio/authinfo.h>
#include <kded/kdedmodule.h>

namespace TDEWallet {
    class Wallet;
}

class KPasswdServer : public KDEDModule
{
  TQ_OBJECT
  K_DCOP
public:
  KPasswdServer(const TQCString &);
  ~KPasswdServer();

k_dcop:
  // KDE4 merge
  TDEIO::AuthInfo checkAuthInfo(TDEIO::AuthInfo, long, unsigned long);
  TDEIO::AuthInfo checkAuthInfo(TDEIO::AuthInfo, long);
  TDEIO::AuthInfo queryAuthInfo(TDEIO::AuthInfo, TQString, long, long, unsigned long);
  TDEIO::AuthInfo queryAuthInfo(TDEIO::AuthInfo, TQString, long, long);
  void addAuthInfo(TDEIO::AuthInfo, long);

public slots:
  void processRequest();
  // Remove all authentication info associated with windowId
  void removeAuthForWindowId(long windowId);

protected:
  struct AuthInfo;

  TQString createCacheKey( const TDEIO::AuthInfo &info );
  const AuthInfo *findAuthInfoItem(const TQString &key, const TDEIO::AuthInfo &info);
  void removeAuthInfoItem(const TQString &key, const TDEIO::AuthInfo &info);
  void addAuthInfoItem(const TQString &key, const TDEIO::AuthInfo &info, long windowId, long seqNr, bool canceled);
  TDEIO::AuthInfo copyAuthInfo(const AuthInfo *);
  void updateAuthExpire(const TQString &key, const AuthInfo *, long windowId, bool keep);
  int findWalletEntry( const TQMap<TQString,TQString>& map, const TQString& username );
  bool openWallet( WId windowId );

  struct AuthInfo {
    AuthInfo() { expire = expNever; isCanceled = false; seqNr = 0; }

    KURL url;
    TQString directory;
    TQString username;
    TQString password;
    TQString realmValue;
    TQString digestInfo;

    enum { expNever, expWindowClose, expTime } expire;
    TQValueList<long> windowList;
    unsigned long expireTime;
    long seqNr;

    bool isCanceled;
  };

  class AuthInfoList : public TQPtrList<AuthInfo>
  {
    public:
      AuthInfoList() { setAutoDelete(true); }
      int compareItems(TQPtrCollection::Item n1, TQPtrCollection::Item n2);
  };

  TQDict< AuthInfoList > m_authDict;

  struct Request {
     DCOPClient *client;
     DCOPClientTransaction *transaction;
     TQString key;
     TDEIO::AuthInfo info;
     TQString errorMsg;
     long windowId;
     long seqNr;
     bool prompt;
  };

  TQPtrList< Request > m_authPending;
  TQPtrList< Request > m_authWait;
  TQIntDict<TQStringList> mWindowIdList;
  DCOPClient *m_dcopClient;
  TDEWallet::Wallet* m_wallet;
  long m_seqNr;
};

#endif
