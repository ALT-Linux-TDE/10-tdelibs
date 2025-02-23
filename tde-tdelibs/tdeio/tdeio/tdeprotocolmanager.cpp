/* This file is part of the KDE libraries
   Copyright (C) 1999 Torben Weis <weis@kde.org>
   Copyright (C) 2000- Waldo Bastain <bastain@kde.org>
   Copyright (C) 2000- Dawit Alemayehu <adawit@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <string.h>
#include <sys/utsname.h>

#include <dcopref.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <kstandarddirs.h>
#include <klibloader.h>
#include <kstringhandler.h>
#include <kstaticdeleter.h>
#include <tdeio/slaveconfig.h>
#include <tdeio/ioslave_defaults.h>
#include <tdeio/http_slave_defaults.h>

#include "tdeprotocolmanager.h"

class
KProtocolManagerPrivate
{
public:
   KProtocolManagerPrivate();

   ~KProtocolManagerPrivate();

   TDEConfig *config;
   TDEConfig *http_config;
   bool init_busy;
   KURL url;
   TQString protocol;
   TQString proxy;
   TQString modifiers;
   TQString useragent;
};

static KProtocolManagerPrivate* d = 0;
static KStaticDeleter<KProtocolManagerPrivate> kpmpksd;

KProtocolManagerPrivate::KProtocolManagerPrivate()
                        :config(0), http_config(0), init_busy(false)
{
   kpmpksd.setObject(d, this);
}

KProtocolManagerPrivate::~KProtocolManagerPrivate()
{
   delete config;
   delete http_config;
}


// DEFAULT USERAGENT STRING
#define CFG_DEFAULT_UAGENT(X) \
TQString("Mozilla/5.0 (compatible; Konqueror/%1.%2%3) KHTML/TDEHTML/%4.%5.%6 (like Gecko)") \
        .arg(TDE_VERSION_MAJOR).arg(TDE_VERSION_MINOR).arg(X).arg(TDE_VERSION_MAJOR).arg(TDE_VERSION_MINOR).arg(TDE_VERSION_RELEASE)

void KProtocolManager::reparseConfiguration()
{
  kpmpksd.destructObject();

  // Force the slave config to re-read its config...
  TDEIO::SlaveConfig::self()->reset ();
}

TDEConfig *KProtocolManager::config()
{
  if (!d)
     d = new KProtocolManagerPrivate;

  if (!d->config)
  {
     d->config = new TDEConfig("tdeioslaverc", true, false);
  }
  return d->config;
}

TDEConfig *KProtocolManager::http_config()
{
  if (!d)
     d = new KProtocolManagerPrivate;

  if (!d->http_config)
  {
     d->http_config = new TDEConfig("tdeio_httprc", false, false);
  }
  return d->http_config;
}

/*=============================== TIMEOUT SETTINGS ==========================*/

int KProtocolManager::readTimeout()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  int val = cfg->readNumEntry( "ReadTimeout", DEFAULT_READ_TIMEOUT );
  return TQMAX(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::connectTimeout()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  int val = cfg->readNumEntry( "ConnectTimeout", DEFAULT_CONNECT_TIMEOUT );
  return TQMAX(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::proxyConnectTimeout()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  int val = cfg->readNumEntry( "ProxyConnectTimeout", DEFAULT_PROXY_CONNECT_TIMEOUT );
  return TQMAX(MIN_TIMEOUT_VALUE, val);
}

int KProtocolManager::responseTimeout()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  int val = cfg->readNumEntry( "ResponseTimeout", DEFAULT_RESPONSE_TIMEOUT );
  return TQMAX(MIN_TIMEOUT_VALUE, val);
}

/*========================== PROXY SETTINGS =================================*/

bool KProtocolManager::useProxy()
{
  return proxyType() != NoProxy;
}

bool KProtocolManager::useReverseProxy()
{
  TDEConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return cfg->readBoolEntry("ReversedException", false);
}

KProtocolManager::ProxyType KProtocolManager::proxyType()
{
  TDEConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return static_cast<ProxyType>(cfg->readNumEntry( "ProxyType" ));
}

KProtocolManager::ProxyAuthMode KProtocolManager::proxyAuthMode()
{
  TDEConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return static_cast<ProxyAuthMode>(cfg->readNumEntry( "AuthMode" ));
}

/*========================== CACHING =====================================*/

bool KProtocolManager::useCache()
{
  TDEConfig *cfg = http_config();
  return cfg->readBoolEntry( "UseCache", true );
}

TDEIO::CacheControl KProtocolManager::cacheControl()
{
  TDEConfig *cfg = http_config();
  TQString tmp = cfg->readEntry("cache");
  if (tmp.isEmpty())
    return DEFAULT_CACHE_CONTROL;
  return TDEIO::parseCacheControl(tmp);
}

TQString KProtocolManager::cacheDir()
{
  TDEConfig *cfg = http_config();
  return cfg->readPathEntry("CacheDir", TDEGlobal::dirs()->saveLocation("cache","http"));
}

int KProtocolManager::maxCacheAge()
{
  TDEConfig *cfg = http_config();
  return cfg->readNumEntry( "MaxCacheAge", DEFAULT_MAX_CACHE_AGE ); // 14 days
}

int KProtocolManager::maxCacheSize()
{
  TDEConfig *cfg = http_config();
  return cfg->readNumEntry( "MaxCacheSize", DEFAULT_MAX_CACHE_SIZE ); // 5 MB
}

TQString KProtocolManager::noProxyForRaw()
{
  TDEConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );

  return cfg->readEntry( "NoProxyFor" );
}

TQString KProtocolManager::noProxyFor()
{
  TQString noProxy = noProxyForRaw();
  if (proxyType() == EnvVarProxy)
    noProxy = TQString::fromLocal8Bit(getenv(noProxy.local8Bit()));

  return noProxy;
}

TQString KProtocolManager::proxyFor( const TQString& protocol )
{
  TQString scheme = protocol.lower();

  if (scheme == "webdav")
    scheme = "http";
  else if (scheme == "webdavs")
    scheme = "https";

  TDEConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return cfg->readEntry( scheme + "Proxy" );
}

TQString KProtocolManager::proxyForURL( const KURL &url )
{
  TQString proxy;
  ProxyType pt = proxyType();

  switch (pt)
  {
      case PACProxy:
      case WPADProxy:
          if (!url.host().isEmpty())
          {
            KURL u (url);
            TQString p = u.protocol().lower();

            // webdav is a KDE specific protocol. Look up proxy
            // information using HTTP instead...
            if ( p == "webdav" )
            {
              p = "http";
              u.setProtocol( p );
            }
            else if ( p == "webdavs" )
            {
              p = "https";
              u.setProtocol( p );
            }

            if ( p.startsWith("http") || p == "ftp" || p == "gopher" )
              DCOPRef( "kded", "proxyscout" ).call( "proxyForURL", u ).get( proxy );
          }
          break;
      case EnvVarProxy:
          proxy = TQString(TQString::fromLocal8Bit(getenv(proxyFor(url.protocol()).local8Bit()))).stripWhiteSpace();
          break;
      case ManualProxy:
          proxy = proxyFor( url.protocol() );
          break;
      case NoProxy:
      default:
          break;
  }

  return (proxy.isEmpty() ? TQString::fromLatin1("DIRECT") : proxy);
}

void KProtocolManager::badProxy( const TQString &proxy )
{
  DCOPRef( "kded", "proxyscout" ).send( "blackListProxy", proxy );
}

/*
    Domain suffix match. E.g. return true if host is "cuzco.inka.de" and
    nplist is "inka.de,hadiko.de" or if host is "localhost" and nplist is
    "localhost".
*/
static bool revmatch(const char *host, const char *nplist)
{
  if (host == 0)
    return false;

  const char *hptr = host + strlen( host ) - 1;
  const char *nptr = nplist + strlen( nplist ) - 1;
  const char *shptr = hptr;

  while ( nptr >= nplist )
  {
    if ( *hptr != *nptr )
    {
      hptr = shptr;

      // Try to find another domain or host in the list
      while(--nptr>=nplist && *nptr!=',' && *nptr!=' ') ;

      // Strip out multiple spaces and commas
      while(--nptr>=nplist && (*nptr==',' || *nptr==' ')) ;
    }
    else
    {
      if ( nptr==nplist || nptr[-1]==',' || nptr[-1]==' ')
        return true;
      if ( hptr == host ) // e.g. revmatch("bugs.kde.org","mybugs.kde.org")
        return false;

      hptr--;
      nptr--;
    }
  }

  return false;
}

TQString KProtocolManager::slaveProtocol(const KURL &url, TQString &proxy)
{
  if (url.hasSubURL()) // We don't want the suburl's protocol
  {
     KURL::List list = KURL::split(url);
     KURL::List::Iterator it = list.fromLast();
     return slaveProtocol(*it, proxy);
  }

  if (!d)
    d = new KProtocolManagerPrivate;

  if (d->url == url)
  {
     proxy = d->proxy;
     return d->protocol;
  }

  if (useProxy())
  {
     proxy = proxyForURL(url);
     if ((proxy != "DIRECT") && (!proxy.isEmpty()))
     {
        bool isRevMatch = false;
        KProtocolManager::ProxyType type = proxyType();
        bool useRevProxy = ((type == ManualProxy) && useReverseProxy());

        TQString noProxy;
        // Check no proxy information iff the proxy type is either
        // ManualProxy or EnvVarProxy
        if ( (type == ManualProxy) || (type == EnvVarProxy) )
          noProxy = noProxyFor();

        if (!noProxy.isEmpty())
        {
           TQString qhost = url.host().lower();
           const char *host = qhost.latin1();
           TQString qno_proxy = noProxy.stripWhiteSpace().lower();
           const char *no_proxy = qno_proxy.latin1();
           isRevMatch = revmatch(host, no_proxy);

           // If no match is found and the request url has a port
           // number, try the combination of "host:port". This allows
           // users to enter host:port in the No-proxy-For list.
           if (!isRevMatch && url.port() > 0)
           {
              qhost += ':' + TQString::number (url.port());
              host = qhost.latin1();
              isRevMatch = revmatch (host, no_proxy);
           }

           // If the hostname does not contain a dot, check if
           // <local> is part of noProxy.
           if (!isRevMatch && host && (strchr(host, '.') == NULL))
              isRevMatch = revmatch("<local>", no_proxy);
        }

        if ( (!useRevProxy && !isRevMatch) || (useRevProxy && isRevMatch) )
        {
           d->url = proxy;
           if ( d->url.isValid() )
           {
              // The idea behind slave protocols is not applicable to http
              // and webdav protocols.
              TQString protocol = url.protocol().lower();
              if (protocol.startsWith("http") || protocol.startsWith("webdav"))
                d->protocol = protocol;
              else
              {
                d->protocol = d->url.protocol();
                kdDebug () << "slaveProtocol: " << d->protocol << endl;
              }

              d->url = url;
              d->proxy = proxy;
              return d->protocol;
           }
        }
     }
  }

  d->url = url;
  d->proxy = proxy = TQString::null;
  d->protocol = url.protocol();
  return d->protocol;
}

/*================================= USER-AGENT SETTINGS =====================*/

TQString KProtocolManager::userAgentForHost( const TQString& hostname )
{
  TQString sendUserAgent = TDEIO::SlaveConfig::self()->configData("http", hostname.lower(), "SendUserAgent").lower();
  if (sendUserAgent == "false")
     return TQString::null;

  TQString useragent = TDEIO::SlaveConfig::self()->configData("http", hostname.lower(), "UserAgent");

  // Return the default user-agent if none is specified
  // for the requested host.
  if (useragent.isEmpty())
    return defaultUserAgent();

  return useragent;
}

TQString KProtocolManager::defaultUserAgent( )
{
  TQString modifiers = TDEIO::SlaveConfig::self()->configData("http", TQString::null, "UserAgentKeys");
  return defaultUserAgent(modifiers);
}

TQString KProtocolManager::defaultUserAgent( const TQString &_modifiers )
{
  if (!d)
     d = new KProtocolManagerPrivate;

  TQString modifiers = _modifiers.lower();
  if (modifiers.isEmpty())
     modifiers = DEFAULT_USER_AGENT_KEYS;

  if (d->modifiers == modifiers)
     return d->useragent;

  TQString supp;
  struct utsname nam;
  if( uname(&nam) >= 0 )
  {
    if( modifiers.contains('o') )
    {
      supp += TQString("; %1").arg(nam.sysname);
      if ( modifiers.contains('v') )
        supp += TQString(" %1").arg(nam.release);
    }
    if( modifiers.contains('p') )
    {
      // TODO: determine this value instead of hardcoding it...
      supp += TQString::fromLatin1("; X11");
    }
    if( modifiers.contains('m') )
    {
      supp += TQString("; %1").arg(nam.machine);
    }
    if( modifiers.contains('l') )
    {
      TQStringList languageList = TDEGlobal::locale()->languageList();
      TQStringList::Iterator it = languageList.find( TQString::fromLatin1("C") );
      if( it != languageList.end() )
      {
        if( languageList.contains( TQString::fromLatin1("en") ) > 0 )
          languageList.remove( it );
        else
          (*it) = TQString::fromLatin1("en");
      }
      if( languageList.count() )
        supp += TQString("; %1").arg(languageList.join(", "));
    }
  }
  d->modifiers = modifiers;
  d->useragent = CFG_DEFAULT_UAGENT(supp);
  return d->useragent;
}

/*==================================== OTHERS ===============================*/

bool KProtocolManager::markPartial()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  return cfg->readBoolEntry( "MarkPartial", true );
}

int KProtocolManager::minimumKeepSize()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  return cfg->readNumEntry( "MinimumKeepSize",
                            DEFAULT_MINIMUM_KEEP_SIZE ); // 5000 byte
}

bool KProtocolManager::autoResume()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  return cfg->readBoolEntry( "AutoResume", false );
}

bool KProtocolManager::persistentConnections()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  return cfg->readBoolEntry( "PersistentConnections", true );
}

bool KProtocolManager::persistentProxyConnection()
{
  TDEConfig *cfg = config();
  cfg->setGroup( TQString::null );
  return cfg->readBoolEntry( "PersistentProxyConnection", false );
}

TQString KProtocolManager::proxyConfigScript()
{
  TDEConfig *cfg = config();
  cfg->setGroup( "Proxy Settings" );
  return cfg->readEntry( "Proxy Config Script" );
}
