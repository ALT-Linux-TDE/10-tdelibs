/*
This file is part of KDE

  Copyright (C) 1998-2000 Waldo Bastian (bastian@kde.org)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
//----------------------------------------------------------------------------
//
// KDE Cookie Server
// $Id$

#define SAVE_DELAY 3 // Save after 3 minutes

#include <unistd.h>

#include <tqtimer.h>
#include <tqptrlist.h>
#include <tqfile.h>

#include <dcopclient.h>

#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <kstandarddirs.h>

#include "kcookiejar.h"
#include "kcookiewin.h"
#include "kcookieserver.h"

extern "C" {
    TDE_EXPORT KDEDModule *create_kcookiejar(const TQCString &name)
    {
       return new KCookieServer(name);
    }
}


// Cookie field indexes
enum CookieDetails { CF_DOMAIN=0, CF_PATH, CF_NAME, CF_HOST,
                     CF_VALUE, CF_EXPIRE, CF_PROVER, CF_SECURE };


class CookieRequest {
public:
   DCOPClient *client;
   DCOPClientTransaction *transaction;
   TQString url;
   bool DOM;
   long windowId;
};

template class  TQPtrList<CookieRequest>;

class RequestList : public TQPtrList<CookieRequest>
{
public:
   RequestList() : TQPtrList<CookieRequest>() { }
};

KCookieServer::KCookieServer(const TQCString &name)
              :KDEDModule(name)
{
   mOldCookieServer = new DCOPClient(); // backwards compatibility.
   mOldCookieServer->registerAs("kcookiejar", false);
   mOldCookieServer->setDaemonMode( true );
   mCookieJar = new KCookieJar;
   mPendingCookies = new KHttpCookieList;
   mPendingCookies->setAutoDelete(true);
   mRequestList = new RequestList;
   mAdvicePending = false;
   mTimer = new TQTimer();
   connect( mTimer, TQ_SIGNAL( timeout()), TQ_SLOT( slotSave()));
   mConfig = new TDEConfig("kcookiejarrc");
   mCookieJar->loadConfig( mConfig );

   TQString filename = locateLocal("data", "kcookiejar/cookies");

   // Stay backwards compatible!
   TQString filenameOld = locate("data", "kfm/cookies");
   if (!filenameOld.isEmpty())
   {
      mCookieJar->loadCookies( filenameOld );
      if (mCookieJar->saveCookies( filename))
      {
         unlink(TQFile::encodeName(filenameOld)); // Remove old kfm cookie file
      }
   }
   else
   {
      mCookieJar->loadCookies( filename);
   }
   connect(this, TQ_SIGNAL(windowUnregistered(long)),
           this, TQ_SLOT(slotDeleteSessionCookies(long)));
}

KCookieServer::~KCookieServer()
{
   if (mCookieJar->changed())
      slotSave();
   delete mOldCookieServer;
   delete mCookieJar;
   delete mTimer;
   delete mPendingCookies;
   delete mConfig;
}

bool KCookieServer::cookiesPending( const TQString &url, KHttpCookieList *cookieList )
{
  TQString fqdn;
  TQStringList domains;
  TQString path;
  // Check whether 'url' has cookies on the pending list
  if (mPendingCookies->isEmpty())
     return false;
  if (!KCookieJar::parseURL(url, fqdn, path))
     return false;

  mCookieJar->extractDomains( fqdn, domains );
  for( KHttpCookie *cookie = mPendingCookies->first();
       cookie != 0L;
       cookie = mPendingCookies->next())
  {
       if (cookie->match( fqdn, domains, path))
       {
          if (!cookieList)
             return true;
          cookieList->append(cookie);
       }
  }
  if (!cookieList)
     return false;
  return cookieList->isEmpty();
}

void KCookieServer::addCookies( const TQString &url, const TQCString &cookieHeader,
                               long windowId, bool useDOMFormat )
{
    KHttpCookieList cookieList;
    if (useDOMFormat)
       cookieList = mCookieJar->makeDOMCookies(url, cookieHeader, windowId);
    else
       cookieList = mCookieJar->makeCookies(url, cookieHeader, windowId);

    checkCookies(&cookieList);
    
    for(KHttpCookiePtr cookie = cookieList.first(); cookie; cookie = cookieList.first())
       mPendingCookies->append(cookieList.take());

    if (!mAdvicePending)
    {
       mAdvicePending = true;
       while (!mPendingCookies->isEmpty())
       {
          checkCookies(0);
       }
       mAdvicePending = false;
    }
}

void KCookieServer::checkCookies( KHttpCookieList *cookieList)
{
    KHttpCookieList *list;
    
    if (cookieList)
       list = cookieList;
    else
       list = mPendingCookies;

    KHttpCookiePtr cookie = list->first();
    while (cookie)
    {
        kdDebug(7104) << "checkCookies: Asking cookie advice for " << cookie->host() << endl;
        KCookieAdvice advice = mCookieJar->cookieAdvice(cookie);
        switch(advice)
        {
        case KCookieAccept:
            list->take();
            mCookieJar->addCookie(cookie);
            cookie = list->current();
            break;

        case KCookieReject:
            list->take();
            delete cookie;
            cookie = list->current();
            break;

        default:
            cookie = list->next();
            break;
        }
    }
    
    if (cookieList || list->isEmpty())
       return;
       
    KHttpCookiePtr currentCookie = mPendingCookies->first();
    
    KHttpCookieList currentList;
    currentList.append(currentCookie);
    TQString currentHost = currentCookie->host();

    cookie = mPendingCookies->next();
    while (cookie)
    {
        if (cookie->host() == currentHost)
        {
            currentList.append(cookie);
        }
        cookie = mPendingCookies->next();
    }

    KCookieWin *kw = new KCookieWin( 0L, currentList,
                                     mCookieJar->preferredDefaultPolicy(),
                                     mCookieJar->showCookieDetails() );
    KCookieAdvice userAdvice = kw->advice(mCookieJar, currentCookie);
    delete kw;
    // Save the cookie config if it has changed
    mCookieJar->saveConfig( mConfig );
    
    // Apply the user's choice to all cookies that are currently
    // queued for this host.
    cookie = mPendingCookies->first();
    while (cookie)
    {
        if (cookie->host() == currentHost)
        {
           switch(userAdvice)
           {
           case KCookieAccept:
               mPendingCookies->take();
               mCookieJar->addCookie(cookie);
               cookie = mPendingCookies->current();
               break;

           case KCookieReject:
               mPendingCookies->take();
               delete cookie;
               cookie = mPendingCookies->current();
               break;

           default:
               tqWarning(__FILE__":%d Problem!", __LINE__);
               cookie = mPendingCookies->next();
               break;
           }
        }
        else
        {
            cookie = mPendingCookies->next();
        }
    }


    // Check if we can handle any request
    for ( CookieRequest *request = mRequestList->first(); request;)
    {
        if (!cookiesPending( request->url ))
        {
           TQCString replyType;
           TQByteArray replyData;
           TQString res = mCookieJar->findCookies( request->url, request->DOM, request->windowId );

           TQDataStream stream2(replyData, IO_WriteOnly);
           stream2 << res;
           replyType = "TQString";
           request->client->endTransaction( request->transaction,
                                            replyType, replyData);
           CookieRequest *tmp = request;
           request = mRequestList->next();
           mRequestList->removeRef( tmp );
           delete tmp;
        }
        else
        {
          request = mRequestList->next();
        }
    }
    if (mCookieJar->changed())
        saveCookieJar();
}

void KCookieServer::slotSave()
{
   TQString filename = locateLocal("data", "kcookiejar/cookies");
   mCookieJar->saveCookies(filename);
}

void KCookieServer::saveCookieJar()
{
    if( mTimer->isActive() )
        return;

    mTimer->start( 1000*60*SAVE_DELAY, true );
}

void KCookieServer::putCookie( TQStringList& out, KHttpCookie *cookie,
                               const TQValueList<int>& fields )
{
    TQValueList<int>::ConstIterator i = fields.begin();
    for ( ; i != fields.end(); ++i )
    {
        switch(*i)
        {
         case CF_DOMAIN :
            out << cookie->domain();
            break;
         case CF_NAME :
            out << cookie->name();
            break;
         case CF_PATH :
            out << cookie->path();
            break;
         case CF_HOST :
            out << cookie->host();
            break;
         case CF_VALUE :
            out << cookie->value();
            break;
         case CF_EXPIRE :
            out << TQString::number(cookie->expireDate());
            break;
         case CF_PROVER :
            out << TQString::number(cookie->protocolVersion());
            break;
         case CF_SECURE :
            out << TQString::number( cookie->isSecure() ? 1 : 0 );
            break;
         default :
            out << TQString::null;
        }
    }
}

bool KCookieServer::cookieMatches( KHttpCookiePtr c,
                                   TQString domain, TQString fqdn,
                                   TQString path, TQString name )
{
    if( c )
    {
        bool hasDomain = !domain.isEmpty();
        return
       ((hasDomain && c->domain() == domain) ||
        fqdn == c->host()) &&
       (c->path()   == path) &&
       (c->name()   == name) &&
       (!c->isExpired(time(0)));
    }
    return false;
}

// DCOP function
TQString
KCookieServer::findCookies(TQString url)
{
  return findCookies(url, 0);
}

// DCOP function
TQString
KCookieServer::findCookies(TQString url, long windowId)
{
   if (cookiesPending(url))
   {
      CookieRequest *request = new CookieRequest;
      request->client = callingDcopClient();
      request->transaction = request->client->beginTransaction();
      request->url = url;
      request->DOM = false;
      request->windowId = windowId;
      mRequestList->append( request );
      return TQString::null; // Talk to you later :-)
   }

   TQString cookies = mCookieJar->findCookies(url, false, windowId);

   if (mCookieJar->changed())
      saveCookieJar();

   return cookies;
}

// DCOP function
TQStringList
KCookieServer::findDomains()
{
   TQStringList result;
   const TQStringList domains = mCookieJar->getDomainList();
   for ( TQStringList::ConstIterator domIt = domains.begin();
         domIt != domains.end(); ++domIt )
   {
       // Ignore domains that have policy set for but contain
       // no cookies whatsoever...
       const KHttpCookieList* list =  mCookieJar->getCookieList(*domIt, "");
       if ( list && !list->isEmpty() )
          result << *domIt;
   }
   return result;
}

// DCOP function
TQStringList
KCookieServer::findCookies(TQValueList<int> fields,
                           TQString domain,
                           TQString fqdn,
                           TQString path,
                           TQString name)
{
   TQStringList result;
   bool allDomCookies = name.isEmpty();

   const KHttpCookieList* list =  mCookieJar->getCookieList(domain, fqdn);
   if ( list && !list->isEmpty() )
   {
      TQPtrListIterator<KHttpCookie>it( *list );
      for ( ; it.current(); ++it )
      {
         if ( !allDomCookies )
         {
            if ( cookieMatches(it.current(), domain, fqdn, path, name) )
            {
               putCookie(result, it.current(), fields);
               break;
            }
         }
         else
            putCookie(result, it.current(), fields);
      }
   }
   return result;
}

// DCOP function
TQString
KCookieServer::findDOMCookies(TQString url)
{
   return findDOMCookies(url, 0);
}

// DCOP function
TQString
KCookieServer::findDOMCookies(TQString url, long windowId)
{
   // We don't wait for pending cookies because it locks up konqueror 
   // which can cause a deadlock if it happens to have a popup-menu up.
   // Instead we just return pending cookies as if they had been accepted already.
   KHttpCookieList pendingCookies;
   cookiesPending(url, &pendingCookies);

   return mCookieJar->findCookies(url, true, windowId, &pendingCookies);
}

// DCOP function
void
KCookieServer::addCookies(TQString arg1, TQCString arg2, long arg3)
{
   addCookies(arg1, arg2, arg3, false);
}

// DCOP function
void
KCookieServer::deleteCookie(TQString domain, TQString fqdn,
                            TQString path, TQString name)
{
   const KHttpCookieList* list = mCookieJar->getCookieList( domain, fqdn );
   if ( list && !list->isEmpty() )
   {
      TQPtrListIterator<KHttpCookie>it (*list);
      for ( ; it.current(); ++it )
      {
         if( cookieMatches(it.current(), domain, fqdn, path, name) )
         {
            mCookieJar->eatCookie( it.current() );
            saveCookieJar();
            break;
         }
      }
   }
}

// DCOP function
void
KCookieServer::deleteCookiesFromDomain(TQString domain)
{
   mCookieJar->eatCookiesForDomain(domain);
   saveCookieJar();
}


// Qt function
void
KCookieServer::slotDeleteSessionCookies( long windowId )
{
   deleteSessionCookies(windowId);
}

// DCOP function
void
KCookieServer::deleteSessionCookies( long windowId )
{
  mCookieJar->eatSessionCookies( windowId );
  saveCookieJar();
}

void
KCookieServer::deleteSessionCookiesFor(TQString fqdn, long windowId)
{
  mCookieJar->eatSessionCookies( fqdn, windowId );
  saveCookieJar();
}

// DCOP function
void
KCookieServer::deleteAllCookies()
{
   mCookieJar->eatAllCookies();
   saveCookieJar();
}

// DCOP function
void
KCookieServer::addDOMCookies(TQString arg1, TQCString arg2, long arg3)
{
   addCookies(arg1, arg2, arg3, true);
}

// DCOP function
void
KCookieServer::setDomainAdvice(TQString url, TQString advice)
{
   TQString fqdn;
   TQString dummy;
   if (KCookieJar::parseURL(url, fqdn, dummy))
   {
      TQStringList domains;
      mCookieJar->extractDomains(fqdn, domains);
      
      mCookieJar->setDomainAdvice(domains[domains.count() > 3 ? 3 : 0],
                                  KCookieJar::strToAdvice(advice));
      // Save the cookie config if it has changed
      mCookieJar->saveConfig( mConfig );
   }
}

// DCOP function
TQString
KCookieServer::getDomainAdvice(TQString url)
{
   KCookieAdvice advice = KCookieDunno;
   TQString fqdn;
   TQString dummy;
   if (KCookieJar::parseURL(url, fqdn, dummy))
   {
      TQStringList domains;
      mCookieJar->extractDomains(fqdn, domains);

      TQStringList::ConstIterator it = domains.begin();
      while ( (advice == KCookieDunno) && (it != domains.end()) )
      {
         // Always check advice in both ".domain" and "domain". Note
         // that we only want to check "domain" if it matches the
         // fqdn of the requested URL.
         if ( (*it)[0] == '.' || (*it) == fqdn )
            advice = mCookieJar->getDomainAdvice(*it);
         ++it;
      }
      if (advice == KCookieDunno)
         advice = mCookieJar->getGlobalAdvice();
   }
   return KCookieJar::adviceToStr(advice);
}

// DCOP function
void
KCookieServer::reloadPolicy()
{
   mCookieJar->loadConfig( mConfig, true );
}

// DCOP function
void
KCookieServer::shutdown()
{
   deleteLater();
}

#include "kcookieserver.moc"

