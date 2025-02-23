/*
    This file is part of the KDE File Manager

    Copyright (C) 1998 Waldo Bastian (bastian@kde.org)

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
// KDE Cookie Server
// $Id$

#ifndef KCOOKIESERVER_H
#define KCOOKIESERVER_H

#include <tqstringlist.h>
#include <kded/kdedmodule.h>

class KHttpCookieList;
class KCookieJar;
class KHttpCookie;
class TQTimer;
class RequestList;
class DCOPClient;
class TDEConfig;

class KCookieServer : public KDEDModule
{
  TQ_OBJECT
  K_DCOP
public:
  KCookieServer(const TQCString &);
  ~KCookieServer();

k_dcop:
  TQString findCookies(TQString);
  TQString findCookies(TQString, long);
  TQStringList findDomains();
  TQStringList findCookies(TQValueList<int>,TQString,TQString,TQString,TQString);
  TQString findDOMCookies(TQString);
  TQString findDOMCookies(TQString, long);
  void addCookies(TQString, TQCString, long);
  void deleteCookie(TQString, TQString, TQString, TQString);
  void deleteCookiesFromDomain(TQString);
  void deleteSessionCookies(long);
  void deleteSessionCookiesFor(TQString, long);
  void deleteAllCookies();
  void addDOMCookies(TQString, TQCString, long);
  /**
   * Sets the cookie policy for the domain associated with the specified URL.
   */
  void setDomainAdvice(TQString url, TQString advice);
  /** 
   * Returns the cookie policy in effect for the specified URL.
   */
  TQString getDomainAdvice(TQString url);
  void reloadPolicy();
  void shutdown();

public:
  bool cookiesPending(const TQString &url, KHttpCookieList *cookieList=0);
  void addCookies(const TQString &url, const TQCString &cookieHeader,
                  long windowId, bool useDOMFormat);
  void checkCookies(KHttpCookieList *cookieList);

public slots:
  void slotSave();
  void slotDeleteSessionCookies(long);

protected:
  KCookieJar *mCookieJar;
  KHttpCookieList *mPendingCookies;
  RequestList *mRequestList;
  TQTimer *mTimer;
  bool mAdvicePending;
  DCOPClient *mOldCookieServer;
  TDEConfig *mConfig;

private:
  virtual int newInstance(TQValueList<TQCString>) { return 0; }
  bool cookieMatches(KHttpCookie*, TQString, TQString, TQString, TQString);
  void putCookie(TQStringList&, KHttpCookie*, const TQValueList<int>&);
  void saveCookieJar();
};

#endif
