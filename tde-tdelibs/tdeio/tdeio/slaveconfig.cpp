/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Waldo Bastian <bastian@kde.org>
 *
 * $Id$
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <assert.h>

#include <tqdict.h>

#include <tdeconfig.h>
#include <kstaticdeleter.h>
#include <kprotocolinfo.h>
#include <tdeprotocolmanager.h>

#include "slaveconfig.h"

using namespace TDEIO;

namespace TDEIO {

class SlaveConfigProtocol
{
public:
  SlaveConfigProtocol() { host.setAutoDelete(true); }
  ~SlaveConfigProtocol()
  {
     delete configFile;
  }

public:
  MetaData global;
  TQDict<MetaData> host;
  TDEConfig *configFile;
};

static void readConfig(TDEConfig *config, const TQString & group, MetaData *metaData)
{
   *metaData += config->entryMap(group);
}

class SlaveConfigPrivate
{
  public:
     void readGlobalConfig();
     SlaveConfigProtocol *readProtocolConfig(const TQString &_protocol);
     SlaveConfigProtocol *findProtocolConfig(const TQString &_protocol);
     void readConfigProtocolHost(const TQString &_protocol, SlaveConfigProtocol *scp, const TQString &host);
  public:
     MetaData global;
     TQDict<SlaveConfigProtocol> protocol;
};

void SlaveConfigPrivate::readGlobalConfig()
{
   global.clear();
   // Read stuff...
   TDEConfig *config = KProtocolManager::config();
   readConfig(TDEGlobal::config(), "Socks", &global); // Socks settings.
   if ( config )
       readConfig(config, "<default>", &global);
}

SlaveConfigProtocol* SlaveConfigPrivate::readProtocolConfig(const TQString &_protocol)
{
   SlaveConfigProtocol *scp = protocol.find(_protocol);
   if (!scp)
   {
      TQString filename = KProtocolInfo::config(_protocol);
      scp = new SlaveConfigProtocol;
      scp->configFile = new TDEConfig(filename, true, false);
      protocol.insert(_protocol, scp);
   }
   // Read global stuff...
   readConfig(scp->configFile, "<default>", &(scp->global));
   return scp;
}

SlaveConfigProtocol* SlaveConfigPrivate::findProtocolConfig(const TQString &_protocol)
{
   SlaveConfigProtocol *scp = protocol.find(_protocol);
   if (!scp)
      scp = readProtocolConfig(_protocol);
   return scp;
}

void SlaveConfigPrivate::readConfigProtocolHost(const TQString &, SlaveConfigProtocol *scp, const TQString &host)
{
   MetaData *metaData = new MetaData;
   scp->host.replace(host, metaData);

   // Read stuff
   // Break host into domains
   TQString domain = host;

   if (!domain.contains('.'))
   {
      // Host without domain.
      if (scp->configFile->hasGroup("<local>"))
         readConfig(scp->configFile, "<local>", metaData);
   }

   int pos = 0;
   do
   {
      pos = host.findRev('.', pos-1);

      if (pos < 0)
        domain = host;
      else
        domain = host.mid(pos+1);

      if (scp->configFile->hasGroup(domain))
         readConfig(scp->configFile, domain.lower(), metaData);
   }
   while (pos > 0);
}


SlaveConfig *SlaveConfig::_self = 0;
static KStaticDeleter<SlaveConfig> slaveconfigsd;

SlaveConfig *SlaveConfig::self()
{
   if (!_self)
      _self = slaveconfigsd.setObject(_self, new SlaveConfig);
   return _self;
}

SlaveConfig::SlaveConfig()
{
  d = new SlaveConfigPrivate;
  d->protocol.setAutoDelete(true);
  d->readGlobalConfig();
}

SlaveConfig::~SlaveConfig()
{
   delete d; d = 0;
   _self = 0;
}

void SlaveConfig::setConfigData(const TQString &protocol,
                                const TQString &host,
                                const TQString &key,
                                const TQString &value )
{
   MetaData config;
   config.insert(key, value);
   setConfigData(protocol, host, config);
}

void SlaveConfig::setConfigData(const TQString &protocol, const TQString &host, const MetaData &config )
{
   if (protocol.isEmpty())
      d->global += config;
   else {
      SlaveConfigProtocol *scp = d->findProtocolConfig(protocol);
      if (host.isEmpty())
      {
         scp->global += config;
      }
      else
      {
         MetaData *hostConfig = scp->host.find(host);
         if (!hostConfig)
         {
            d->readConfigProtocolHost(protocol, scp, host);
            hostConfig = scp->host.find(host);
            assert(hostConfig);
         }
         *hostConfig += config;
      }
   }
}

MetaData SlaveConfig::configData(const TQString &protocol, const TQString &host)
{
   MetaData config = d->global;
   SlaveConfigProtocol *scp = d->findProtocolConfig(protocol);
   config += scp->global;
   if (host.isEmpty())
      return config;
   MetaData *hostConfig = scp->host.find(host);
   if (!hostConfig)
   {
      d->readConfigProtocolHost(protocol, scp, host);
      emit configNeeded(protocol, host);
      hostConfig = scp->host.find(host);
      assert(hostConfig);
   }
   config += *hostConfig;
   return config;
}

TQString SlaveConfig::configData(const TQString &protocol, const TQString &host, const TQString &key)
{
   return configData(protocol, host)[key];
}

void SlaveConfig::reset()
{
   d->protocol.clear();
   d->readGlobalConfig();
}

}

#include "slaveconfig.moc"
