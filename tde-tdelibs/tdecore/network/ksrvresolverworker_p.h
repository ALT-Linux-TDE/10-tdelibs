/*
 *  Copyright (C) 2005 Thiago Macieira <thiago@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#ifndef KSRVRESOLVERWORKER_P_H
#define KSRVRESOLVERWORKER_P_H

#include <tqobject.h>
#include <tqdns.h>
#include <tqsemaphore.h>
#include <tqvaluelist.h>
#include <tqdict.h>
#include "kresolver.h"
#include "kresolverworkerbase.h"

class TQCustomEvent;

namespace KNetwork
{
  namespace Internal
  {
    /**
     * @internal
     * This class implements SRV-based resolution
     */
    class KSrvResolverWorker: public TQObject,
			      public KNetwork::KResolverWorkerBase
    {
      TQ_OBJECT
      

    public:
      struct Entry
      {
	TQString name;
	TQ_UINT16 port;
	TQ_UINT16 weight;
	KNetwork::KResolver* resolver;
      };

      struct PriorityClass
      {
	PriorityClass() : totalWeight(0) { }

	TQValueList<Entry> entries;
	TQ_UINT16 totalWeight;
      };

    private:
      TQDns *dns;
      TQValueList<TQDns::Server> rawResults;
      TQCString encodedName;
      TQSemaphore *sem;

      typedef TQMap<TQ_UINT16, PriorityClass> Results;
      Results myResults;

    public:
      virtual bool preprocess();
      virtual bool run();
      virtual bool postprocess();

      virtual void customEvent(TQCustomEvent*);

    public slots:
      void dnsResultsReady();
    };
  }
}

#endif
